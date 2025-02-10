// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/types.h>
#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/debugfs.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/mm.h>
#include <linux/major.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/spinlock_types.h>
#include <linux/extcon-provider.h>
#include "hdmitx_hdcp_type.h"
#include "hdmitx_ddc.h"
#include "../hdmitx_module.h"
#include <linux/uaccess.h>
#include "hdmitx_common.h"
#include "hdmitx_hdcp.h"

#include <drm/drmP.h>
#include <drm/amlogic/meson_connector_dev.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_common.h>
#include <linux/amlogic/media/vout/hdmitx_common/hdmitx_types.h>
#include <linux/workqueue.h>
#include <linux/timer.h>
#include <linux/workqueue.h>

/*
 * hdmi_tx_hdcp.c
 * version 1.1
 */

static int hdmi_authenticated;

static DEFINE_MUTEX(mutex);
/* obs_cur, record current obs */
static struct hdcp_obs_val obs_cur;
/* if obs_cur is differ than last time, then save to obs_last */
static struct hdcp_obs_val obs_last;
static int write_buff(const char *fmt, ...);

static struct hdcplog_buf hdcplog_buf;

unsigned int hdcp_get_downstream_ver(struct hdmitx_dev *hdev)
{
	unsigned int ret = 14;

	/* if TX don't have HDCP22 key, skip RX hdcp22 ver */
	if (hdmitx_hw_cntl_ddc(&hdev->hw_comm, DDC_HDCP_22_LSTORE, 0) == 0)
		if (hdcp_rd_hdcp22_ver())
			ret = 22;
		else
			ret = 14;
	else
		ret = 14;
	return ret;
}

/* Notice: the HDCP key setting has been moved to uboot
 * On MBX project, it is too late for HDCP get from
 * other devices
 */

/* verify ksv, 20 ones and 20 zeroes */
int hdcp_ksv_valid(unsigned char *dat)
{
	int i, j, one_num = 0;

	for (i = 0; i < 5; i++) {
		for (j = 0; j < 8; j++) {
			if ((dat[i] >> j) & 0x1)
				one_num++;
		}
	}
	return one_num == 20;
}

static int save_obs_val(struct hdmitx_dev *hdev, struct hdcp_obs_val *obs)
{
	return hdmitx_hw_cntl_ddc(hdev->tx20_hw.base,
		DDC_HDCP14_SAVE_OBS, (unsigned long)obs);
}

static void pr_obs(struct hdcp_obs_val *obst0, struct hdcp_obs_val *obst1,
	unsigned int mask)
{
	/* if idx > HDCP_LOG_SIZE, then set idx as 0 for wrapping log */
#define GETBITS(val, start, len) (((val) >> (start)) & ((1 << (len)) - 1))
#define DIFF_ST(_v1, v0, _s, _l, str) \
	do { \
		typeof(_v1) v1 = (_v1); \
		typeof(_s) s = (_s); \
		typeof(_l) l = (_l); \
		if (GETBITS(v1, s, l) != (GETBITS(v0, s, l))) { \
			write_buff("%s: %x\n", str, GETBITS(v1, s, l)); \
		} \
	} while (0)

	if (mask & (1 << 0)) {
		if (GETBITS(obst1->obs0, 1, 7) != GETBITS(obst0->obs0, 1, 7)) {
			write_buff("StateA: %x  SubStateA: %x\n",
				GETBITS(obst1->obs0, 4, 4),
				GETBITS(obst1->obs0, 1, 3));
		}
		DIFF_ST(obst1->obs0, obst0->obs0, 0, 1, "hdcpengaged");
	}
	if (mask & (1 << 1)) {
		DIFF_ST(obst1->obs1, obst0->obs1, 4, 3, "StateOEG");
		DIFF_ST(obst1->obs1, obst0->obs1, 0, 4, "StateR");
	}
	if (mask & (1 << 2)) {
		DIFF_ST(obst1->obs2, obst0->obs2, 3, 3, "StateE");
		DIFF_ST(obst1->obs2, obst0->obs2, 0, 3, "StateEEG");
	}
	if (mask & (1 << 3)) {
		DIFF_ST(obst1->obs3, obst0->obs3, 7, 1, "RSVD");
		DIFF_ST(obst1->obs3, obst0->obs3, 6, 1, "Repeater");
		DIFF_ST(obst1->obs3, obst0->obs3, 5, 1, "KSV_FIFO_Ready");
		DIFF_ST(obst1->obs3, obst0->obs3, 4, 1, "Fast_i2c");
		DIFF_ST(obst1->obs3, obst0->obs3, 3, 1, "RSVD2");
		DIFF_ST(obst1->obs3, obst0->obs3, 2, 1, "Hdmi_mode");
		DIFF_ST(obst1->obs3, obst0->obs3, 1, 1, "Features1.1");
		DIFF_ST(obst1->obs3, obst0->obs3, 0, 1, "Fast_Reauth");
	}
	if (mask & (1 << 4)) {
		DIFF_ST(obst1->intstat, obst0->intstat, 7, 1, "hdcp_engaged");
		DIFF_ST(obst1->intstat, obst0->intstat, 6, 1, "hdcp_failed");
		DIFF_ST(obst1->intstat, obst0->intstat, 4, 1, "i2cnack");
		DIFF_ST(obst1->intstat, obst0->intstat, 3, 1,
			"lostarbitration");
		DIFF_ST(obst1->intstat, obst0->intstat, 2, 1,
			"keepouterrorint");
		DIFF_ST(obst1->intstat, obst0->intstat, 1, 1, "KSVsha1calcint");
	}
}

static void _hdcp_do_work(struct work_struct *work)
{
	int ret = 0;
	struct hdmitx_dev *hdev =
		container_of(work, struct hdmitx_dev, work_do_hdcp.work);
	struct hdmitx_common *tx_comm = &hdev->tx_comm;

	switch (tx_comm->hdcp_mode) {
	case 2:
		/* hdev->HWOp.CntlMisc(hdev, MISC_HDCP_CLKDIS, 1); */
		/* schedule_delayed_work(&hdev->work_do_hdcp, HZ / 50); */
		break;
	case 1:
		mutex_lock(&mutex);
		ret = save_obs_val(hdev, &obs_cur);
		/* ret is NZ, then update obs_last */
		if (ret) {
			pr_obs(&obs_last, &obs_cur, ret);
			obs_last = obs_cur;
		}
		mutex_unlock(&mutex);
		/* log time frequency */
		schedule_delayed_work(&hdev->work_do_hdcp, HZ / 20);
		break;
	case 0:
	default:
		mutex_lock(&mutex);
		memset(&obs_cur, 0, sizeof(obs_cur));
		memset(&obs_last, 0, sizeof(obs_last));
		mutex_unlock(&mutex);
		hdmitx_hw_cntl_misc(tx_comm->tx_hw, MISC_HDCP_CLKDIS, 0);
		break;
	}
}

void hdmitx_hdcp_do_work(struct hdmitx_common *tx_comm)
{
	struct hdmitx_dev *hdev =
		container_of(tx_comm, struct hdmitx_dev, tx_comm);

	_hdcp_do_work(&hdev->work_do_hdcp.work);
}

void hdmitx_hdcp_status(struct hdmitx_dev *hdev, int hdmi_authenticated)
{
	hdmitx_set_uevent(&hdev->tx_comm, HDMITX_HDCP_EVENT, hdmi_authenticated);
	if (hdev->drm_hdcp_cb.callback)
		hdev->drm_hdcp_cb.callback(hdev->drm_hdcp_cb.data,
			hdmi_authenticated);
}

static int hdmitx_hdcp_task(void *data)
{
	static int auth_trigger;
	struct hdmitx_dev *hdev = (struct hdmitx_dev *)data;

	INIT_DELAYED_WORK(&hdev->work_do_hdcp, _hdcp_do_work);
	while (hdev->tx_comm.hpd_event != 0xff) {
		hdmi_authenticated = hdmitx_hw_cntl_ddc(&hdev->hw_comm,
			DDC_HDCP_GET_AUTH, 0);
		hdmitx_hdcp_status(hdev, hdmi_authenticated);
		if (auth_trigger != hdmi_authenticated) {
			auth_trigger = hdmi_authenticated;
			HDMITX_INFO("hdcptx: %d  auth: %d\n", hdev->tx_comm.hdcp_mode,
			auth_trigger);
			// Only collect the metric when hdmi is plugged in.
			if (hdev->tx_comm.hpd_state == 1) {
				hdmitx_current_status(auth_trigger
					? HDMITX_HDCP_AUTH_SUCCESS
					: HDMITX_HDCP_AUTH_FAILURE);
			}
		}
		msleep_interruptible(200);
	}

	return 0;
}

static int read_buff(char *p)
{
	if (hdcplog_buf.rd_pos == hdcplog_buf.wr_pos)
		return 0;

	*p = hdcplog_buf.buf[hdcplog_buf.rd_pos];
	hdcplog_buf.rd_pos = (hdcplog_buf.rd_pos + 1) % HDCP_LOG_SIZE;
	return 1;
}

static void _write_buff(char c)
{
	hdcplog_buf.buf[hdcplog_buf.wr_pos] = c;
	hdcplog_buf.wr_pos = (hdcplog_buf.wr_pos + 1) % HDCP_LOG_SIZE;
	if (hdcplog_buf.wr_pos == hdcplog_buf.rd_pos)
		hdcplog_buf.rd_pos = (hdcplog_buf.rd_pos + 1) % HDCP_LOG_SIZE;
	wake_up_interruptible(&hdcplog_buf.wait);
}

static int write_buff(const char *fmt, ...)
{
	va_list args;
	int i, len;
	static char temp[64];

	va_start(args, fmt);
	len = vsnprintf(temp, sizeof(temp), fmt, args);
	va_end(args);

	for (i = 0; i < len; i++)
		_write_buff(temp[i]);

	return len;
}

static int hdcplog_open(struct inode *inode, struct file *file)
{
	return 0;
}

static ssize_t hdcplog_read(struct file *file, char __user *buf,
			size_t count, loff_t *ppos)
{
	ssize_t error = -EINVAL;
	int i = 0;
	char c;

	if ((file->f_flags & O_NONBLOCK) && hdcplog_buf.rd_pos == hdcplog_buf.wr_pos)
		return  -EAGAIN;

	if (!buf || !count)
		goto out;

	error = wait_event_interruptible(hdcplog_buf.wait,
			(hdcplog_buf.wr_pos != hdcplog_buf.rd_pos));
	if (error)
		goto out;
	while (!error && (read_buff(&c)) && i < count) {
		error = __put_user(c, buf);
		buf++;
		i++;
	}

	if (!error)
		error = i;
out:
	return error;
}

static void hdmitx20_set_hdcp_mode(struct hdmitx_common *tx_comm, const char *buf)
{
	enum hdmi_vic vic =
		hdmitx_hw_get_state(tx_comm->tx_hw, STAT_VIDEO_VIC, 0);

	if (hdmitx_hw_cntl_misc(tx_comm->tx_hw, MISC_TMDS_RXSENSE, 0) == 0)
		hdmitx_current_status(HDMITX_HDCP_DEVICE_NOT_READY_ERROR);
	/*
	 * there's risk:
	 * hdcp2.2 start auth-->enter early suspend, stop hdcp-->
	 * hdcp2.2 auth fail & timeout-->fall back to hdcp1.4, so
	 * hdcp running even no hdmi output-->resume, read EDID.
	 * EDID may read fail as hdcp may also access DDC simultaneously.
	 */
	mutex_lock(&tx_comm->hdmimode_mutex);
	if (!tx_comm->ready) {
		HDMITX_INFO("hdmi signal not ready, should not set hdcp mode %s\n", buf);
		mutex_unlock(&tx_comm->hdmimode_mutex);
		return;
	}
	HDMITX_INFO(SYS "hdcp: set mode as %s\n", buf);
	hdmitx_hw_cntl_ddc(tx_comm->tx_hw, DDC_HDCP_MUX_INIT, 1);
	hdmitx_hw_cntl_ddc(tx_comm->tx_hw, DDC_HDCP_GET_AUTH, 0);
	if (strncmp(buf, "0", 1) == 0) {
		tx_comm->hdcp_mode = 0;
		hdmitx_hw_cntl_ddc(tx_comm->tx_hw,
			DDC_HDCP_OP, HDCP14_OFF);
		hdmitx_hdcp_do_work(tx_comm);
		hdmitx_current_status(HDMITX_HDCP_NOT_ENABLED);
	}
	if (strncmp(buf, "1", 1) == 0) {
		if (vic == HDMI_17_720x576p50_4x3 || vic == HDMI_18_720x576p50_16x9)
			usleep_range(500000, 500010);
		tx_comm->hdcp_mode = 1;
		hdmitx_hdcp_do_work(tx_comm);
		hdmitx_hw_cntl_ddc(tx_comm->tx_hw,
			DDC_HDCP_OP, HDCP14_ON);
		hdmitx_current_status(HDMITX_HDCP_HDCP_1_ENABLED);
	}
	if (strncmp(buf, "2", 1) == 0) {
		if (tx_comm->efuse_dis_hdcp_tx22) {
			HDMITX_ERROR("warning, efuse disable hdcptx22\n");
			mutex_unlock(&tx_comm->hdmimode_mutex);
			return;
		}
		tx_comm->hdcp_mode = 2;
		hdmitx_hdcp_do_work(tx_comm);
		hdmitx_hw_cntl_ddc(tx_comm->tx_hw,
			DDC_HDCP_MUX_INIT, 2);
		hdmitx_current_status(HDMITX_HDCP_HDCP_2_ENABLED);
	}
	mutex_unlock(&tx_comm->hdmimode_mutex);
}

static int hdmitx20_get_hdcp_ver(struct hdmitx_common *tx_comm, char *buf, int len)
{
	int pos = 0;
	u32 ver = meson_hdcp_get_rx_cap();

	if (ver == 0x3)
		pos += snprintf(buf + pos, len - pos, "22\n\r");
	pos += snprintf(buf + pos, len - pos, "14\n\r");
	return pos;
}

/***** Move the code in meson_hdcp.c here *****/
static struct meson_hdmitx_hdcp meson_hdcp;

static unsigned int get_hdcp_downstream_ver(void)
{
	unsigned int hdcp_downstream_type = meson_hdcp_get_rx_cap();

	HDMITX_INFO("downstream support hdcp14: %d\n",
		 hdcp_downstream_type & 0x1);
	HDMITX_INFO("downstream support hdcp22: %d\n",
		 (hdcp_downstream_type & 0x2) >> 1);

	return hdcp_downstream_type;
}

static unsigned int meson_hdcp_get_tx_key_version(void)
{
	struct hdmitx_dev *hdev = get_hdmitx_device();

	if (!hdev || hdev->tx_comm.hdmi_init != HDMITX20)
		return 0;

	if (hdev->hw_comm.lstore < 0x10) {
		hdev->hw_comm.lstore = 0;
		if (hdmitx_hw_cntl_ddc(&hdev->hw_comm, DDC_HDCP_14_LSTORE, 0))
			hdev->hw_comm.lstore += 1;
		if (hdmitx_hw_cntl_ddc(&hdev->hw_comm, DDC_HDCP_22_LSTORE, 0))
			hdev->hw_comm.lstore += 2;
	}

	HDMITX_INFO("hdmitx support hdcp14: %d, hdcp22: %d\n",
		hdev->hw_comm.lstore & 0x1, (hdev->hw_comm.lstore & 0x2) >> 1);
	return hdev->hw_comm.lstore & 0x3;
}

int meson_hdcp_get_valid_type(int request_type_mask)
{
	int type;
	unsigned int hdcp_tx_type = meson_hdcp_get_tx_cap();

	HDMITX_INFO("%s usr_type: %d, key_store: %d\n",
		 __func__, request_type_mask, hdcp_tx_type);
	if (hdcp_tx_type)
		meson_hdcp.hdcp_downstream_type = get_hdcp_downstream_ver();
	switch (hdcp_tx_type & 0x3) {
	case 0x3:
		if ((meson_hdcp.hdcp_downstream_type & 0x2) &&
			(request_type_mask & 0x2))
			type = HDCP_MODE22;
		else if ((meson_hdcp.hdcp_downstream_type & 0x1) &&
			 (request_type_mask & 0x1))
			type = HDCP_MODE14;
		else
			type = HDCP_NULL;
		break;
	case 0x2:
		if ((meson_hdcp.hdcp_downstream_type & 0x2) &&
			(request_type_mask & 0x2))
			type = HDCP_MODE22;
		else
			type = HDCP_NULL;
		break;
	case 0x1:
		if ((meson_hdcp.hdcp_downstream_type & 0x1) &&
			(request_type_mask & 0x1))
			type = HDCP_MODE14;
		else
			type = HDCP_NULL;
		break;
	default:
		type = HDCP_NULL;
		HDMITX_INFO("[%s]: TX no hdcp key\n", __func__);
		break;
	}
	return type;
}

static int am_get_hdcp_exe_type(void)
{
	return meson_hdcp_get_valid_type(meson_hdcp.hdcp_debug_type);
}

void meson_hdcp_disable(void)
{
	if (!meson_hdcp.hdcp_en)
		return;

	HDMITX_INFO("[%s]: %d\n", __func__, meson_hdcp.hdcp_execute_type);
	/*
	 * when switch mode under hdcp1.4, should not
	 * trigger hdcp_tx22 daemon to change status.
	 * otherwise, it may exit idle state and
	 * enter polling(driver hdcp2.2 start) per 5ms
	 * which cause high cpu CPU utilization
	 */
	if (meson_hdcp.hdcp_execute_type == HDCP_MODE22) {
		if (meson_hdcp.hdcp_report == HDCP_TX22_START) {
			/* notify hdcp_tx22 to stop hdcp22 */
			meson_hdcp.hdcp_report = HDCP_TX22_STOP;
			wake_up(&meson_hdcp.hdcp_comm_queue);
			/* wait for hdcp_tx22 stop hdcp22 done */
			msleep_interruptible(200);
		}
		drm_hdmitx_disable_hdcp_mode(HDCP_MODE22);
		/* notify hdcp_tx22 to enter hdcp22 init state(DISCONNECT) */
		meson_hdcp.hdcp_report = HDCP_TX22_DISCONNECT;
		wake_up(&meson_hdcp.hdcp_comm_queue);
	} else if (meson_hdcp.hdcp_execute_type == HDCP_MODE14) {
		drm_hdmitx_disable_hdcp_mode(HDCP_MODE14);
	}
	meson_hdcp.hdcp_execute_type = HDCP_NULL;
	meson_hdcp.hdcp_auth_result = HDCP_AUTH_UNKNOWN;
	meson_hdcp.hdcp_en = 0;
	meson_hdcp.hdcp_fail_cnt = 0;
}

bool is_hdcp22_stop_state(void)
{
	return meson_hdcp.hdcp_report == HDCP_TX22_STOP;
}

void meson_hdcp_disconnect(void)
{
	/* if (meson_hdcp.hdcp_execute_type == HDCP_MODE22) */
		/* drm_hdmitx_disable_hdcp_mode(HDCP_MODE22); */
	/* else if (meson_hdcp.hdcp_execute_type  == HDCP_MODE14) */
		/* drm_hdmitx_disable_hdcp_mode(HDCP_MODE14); */
	meson_hdcp.hdcp_report = HDCP_TX22_DISCONNECT;
	meson_hdcp.hdcp_downstream_type = 0;
	/*
	 * TODO: for suspend/resume, need to stop/start hdcp22
	 * need to keep exe type
	 */
	meson_hdcp.hdcp_execute_type = HDCP_NULL;
	meson_hdcp.hdcp_auth_result = HDCP_AUTH_UNKNOWN;
	meson_hdcp.hdcp_en = 0;
	meson_hdcp.hdcp_fail_cnt = 0;
	HDMITX_INFO("[%s]: HDCP_TX22_DISCONNECT\n", __func__);
	wake_up(&meson_hdcp.hdcp_comm_queue);
}

static void meson_hdmitx_hdcp_cb(void *data, int auth)
{
	struct meson_hdmitx_hdcp *hdcp_data = (struct meson_hdmitx_hdcp *)data;
	int hdcp_auth_result = HDCP_AUTH_UNKNOWN;

	if (hdcp_data->hdcp_en &&
		hdcp_data->hdcp_auth_result == HDCP_AUTH_UNKNOWN) {
		if (auth == 1) {
			hdcp_auth_result = HDCP_AUTH_OK;
		} else if (auth == 0) {
			hdcp_data->hdcp_fail_cnt++;

			if (hdcp_data->hdcp_fail_cnt > HDCP_AUTH_TIMEOUT)
				hdcp_auth_result = HDCP_AUTH_FAIL;
		}

		HDMITX_INFO("HDCP cb %d vs %d\n", hdcp_data->hdcp_auth_result, auth);

		if (hdcp_auth_result != hdcp_data->hdcp_auth_result) {
			hdcp_data->hdcp_auth_result = hdcp_auth_result;
			if (meson_hdcp.drm_hdcp_cb.hdcp_notify)
				meson_hdcp.drm_hdcp_cb.hdcp_notify
					(meson_hdcp.drm_hdcp_cb.data,
					hdcp_data->hdcp_execute_type,
					hdcp_data->hdcp_auth_result);
		}
	}
}

void meson_hdcp_reg_result_notify(struct connector_hdcp_cb *cb)
{
	if (meson_hdcp.drm_hdcp_cb.hdcp_notify)
		HDMITX_INFO("Register hdcp notify again!?\n");

	meson_hdcp.drm_hdcp_cb.hdcp_notify = cb->hdcp_notify;
	meson_hdcp.drm_hdcp_cb.data = cb->data;
}

void meson_hdcp_enable(int hdcp_type)
{
	if (hdcp_type == HDCP_NULL)
		return;

	/* hdcp enabled, but may not start auth as key not ready */
	meson_hdcp.hdcp_en = 1;
	meson_hdcp.hdcp_fail_cnt = 0;
	meson_hdcp.hdcp_auth_result = HDCP_AUTH_UNKNOWN;
	meson_hdcp.hdcp_execute_type = hdcp_type;

	if (meson_hdcp.hdcp_execute_type == HDCP_MODE22) {
		if (meson_hdcp.hdcp22_daemon_state != HDCP22_DAEMON_DONE) {
			HDMITX_INFO("[%s]: hdcp_tx22 not ready, delay hdcp auth\n", __func__);
			return;
		}
		drm_hdmitx_enable_hdcp_mode(2);
		meson_hdcp.hdcp_report = HDCP_TX22_START;
		msleep(50);
		wake_up(&meson_hdcp.hdcp_comm_queue);
	} else if (meson_hdcp.hdcp_execute_type == HDCP_MODE14) {
		drm_hdmitx_enable_hdcp_mode(1);
	}
	HDMITX_INFO("[%s]: report=%d, use_type=%u, execute=%u\n",
		 __func__, meson_hdcp.hdcp_report,
		 meson_hdcp.hdcp_debug_type, meson_hdcp.hdcp_execute_type);
}

static long hdcp_comm_ioctl(struct file *file,
	unsigned int cmd,
	unsigned long arg)
{
	int rtn_val;
	unsigned int out_size;
	int hdcp_type = HDCP_NULL;

	switch (cmd) {
	case TEE_HDCP_IOC_START:
		/* notify by TEE, hdcp key ready, echo 2 > hdcp_mode */
		rtn_val = 0;
		meson_hdcp.hdcp_tx_type = meson_hdcp_get_tx_key_version();
		if (meson_hdcp.hdcp_tx_type & 0x2) {
			/*
			 * when bootup, if hdcp22 init after hdcp14 auth,
			 * hdcp path will switch to hdcp22. need to delay
			 * hdcp auth to cover this issue.
			 */
			drm_hdmitx_hdcp22_init();
		}
		HDMITX_INFO("hdcp key load ready\n");
		break;
	case TEE_HDCP_IOC_END:
		rtn_val = 0;
		break;
	case HDCP_DAEMON_IOC_LOAD_END:
		/* hdcp_tx22 load ready (after TEE key ready) */
		HDMITX_INFO("IOC_LOAD_END %d, %d\n",
			 meson_hdcp.hdcp_report, meson_hdcp.hdcp_poll_report);
		if (meson_hdcp.hdcp22_daemon_state == HDCP22_DAEMON_TIMEOUT)
			HDMITX_INFO("hdcp22 key load late than TIMEOUT.\n");
		meson_hdcp.hdcp22_daemon_state = HDCP22_DAEMON_DONE;
		meson_hdcp.hdcp_poll_report = meson_hdcp.hdcp_report;
		rtn_val = 0;
		break;
	case HDCP_DAEMON_IOC_REPORT:
		rtn_val = copy_to_user((void __user *)arg,
				       (void *)&meson_hdcp.hdcp_report,
				       sizeof(meson_hdcp.hdcp_report));
		if (rtn_val != 0) {
			out_size = sizeof(meson_hdcp.hdcp_report);
			HDMITX_INFO("out_size: %u, left_size: %u\n",
				 out_size, rtn_val);
			rtn_val = -1;
		}
		break;
	case HDCP_EXE_VER_IOC_SET:
		if (arg > 2) {
			rtn_val = -1;
		} else {
			meson_hdcp.hdcp_debug_type = arg;
			rtn_val = 0;
			if (hdmitx_hpd_hw_op(HPD_READ_HPD_GPIO)) {
				meson_hdcp_disable();
				hdcp_type = am_get_hdcp_exe_type();
				meson_hdcp_enable(hdcp_type);
			}
		}
		break;
	case HDCP_TX_VER_IOC_REPORT:
		rtn_val = copy_to_user((void __user *)arg,
				       (void *)&meson_hdcp.hdcp_tx_type,
				       sizeof(meson_hdcp.hdcp_tx_type));
		if (rtn_val != 0) {
			out_size = sizeof(meson_hdcp.hdcp_tx_type);
			HDMITX_INFO("out_size: %u, left_size: %u\n",
				 out_size, rtn_val);
			rtn_val = -1;
		}
		break;
	case HDCP_DOWNSTR_VER_IOC_REPORT:
		rtn_val = copy_to_user((void __user *)arg,
				       (void *)&meson_hdcp.hdcp_downstream_type,
				       sizeof(meson_hdcp.hdcp_downstream_type));
		if (rtn_val != 0) {
			out_size = sizeof(meson_hdcp.hdcp_downstream_type);
			HDMITX_INFO("out_size: %u, left_size: %u\n",
				 out_size, rtn_val);
			rtn_val = -1;
		}
		break;
	case HDCP_EXE_VER_IOC_REPORT:
		rtn_val = copy_to_user((void __user *)arg,
				       (void *)&meson_hdcp.hdcp_execute_type,
				       sizeof(meson_hdcp.hdcp_execute_type));
		if (rtn_val != 0) {
			out_size = sizeof(meson_hdcp.hdcp_execute_type);
			HDMITX_INFO("out_size: %u, left_size: %u\n",
				 out_size, rtn_val);
			rtn_val = -1;
		}
		break;
	default:
		rtn_val = -EPERM;
	}
	return rtn_val;
}

static unsigned int hdcp_comm_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;

	HDMITX_INFO("hdcp_poll %d, %d\n", meson_hdcp.hdcp_report, meson_hdcp.hdcp_poll_report);
	poll_wait(file, &meson_hdcp.hdcp_comm_queue, wait);
	if (meson_hdcp.hdcp_report != meson_hdcp.hdcp_poll_report) {
		mask = POLLIN | POLLRDNORM;
		meson_hdcp.hdcp_poll_report = meson_hdcp.hdcp_report;
	}
	return mask;
}

static const struct file_operations hdcp_comm_file_operations = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = hdcp_comm_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = hdcp_comm_ioctl,
#endif
	.poll = hdcp_comm_poll,
};

/* debug interface begin */
static void am_hdmitx_set_hdmi_mode(void)
{
	enum vmode_e vmode = get_current_vmode();

	if (vmode == VMODE_HDMI) {
		HDMITX_INFO("set_hdmi_mode manually\n");
	} else {
		HDMITX_ERROR("set_hdmi_mode manually fail! vmode:%d\n", vmode);
		return;
	}

	/* TODO: sync meson_hdmi.c meson_hdmitx_encoder_atomic_enable() */
	set_vout_mode_pre_process(vmode);
	set_vout_vmode(vmode);
	set_vout_mode_post_process(vmode);
}

/* set hdmi+hdcp mode */
static void am_hdmitx_set_out_mode(void)
{
	enum vmode_e vmode = get_current_vmode();
	struct hdmitx_dev *hdmitx_dev = get_hdmitx_device();
	int last_hdcp_mode = HDCP_NULL;

	if (vmode == VMODE_HDMI) {
		HDMITX_INFO("set_out_mode\n");
	} else {
		HDMITX_ERROR("set_out_mode fail! vmode:%d\n", vmode);
		return;
	}

	if (hdmitx_dev->tx_comm.hdcp_ctl_lvl > 0) {
		hdmitx_common_avmute_locked(&hdmitx_dev->tx_comm,
			SET_AVMUTE, AVMUTE_PATH_HDMITX);
		last_hdcp_mode = meson_hdcp.hdcp_execute_type;
		meson_hdcp_disable();
	}
	/* TODO: sync meson_hdmi.c meson_hdmitx_encoder_atomic_enable() */
	set_vout_mode_pre_process(vmode);
	set_vout_vmode(vmode);
	set_vout_mode_post_process(vmode);
	/* msleep(1000); */
	if (hdmitx_dev->tx_comm.hdcp_ctl_lvl > 0) {
		hdmitx_common_avmute_locked(&hdmitx_dev->tx_comm,
			CLR_AVMUTE, AVMUTE_PATH_HDMITX);
		meson_hdcp_enable(last_hdcp_mode);
	}
}

static void am_hdmitx_hdcp_disable(void)
{
	struct hdmitx_dev *hdmitx_dev = get_hdmitx_device();

	if (hdmitx_dev->tx_comm.hdcp_ctl_lvl >= 1)
		meson_hdcp_disable();
	HDMITX_INFO("hdcp disable manually\n");
}

static void am_hdmitx_hdcp_disconnect(void)
{
	struct hdmitx_dev *hdmitx_dev = get_hdmitx_device();

	if (hdmitx_dev->tx_comm.hdcp_ctl_lvl >= 1)
		meson_hdcp_disconnect();
	HDMITX_INFO("hdcp disconnect manually\n");
}

/***** debug interface end *****/

static void meson_hdcp_key_notify(struct work_struct *work)
{
	struct delayed_work *dwork = to_delayed_work(work);
	struct meson_hdmitx_hdcp *meson_hdcp =
		container_of(dwork, struct meson_hdmitx_hdcp,
			     notify_work);

	meson_hdcp->drm_hdcp_cb.hdcp_notify(meson_hdcp->drm_hdcp_cb.data,
		HDCP_KEY_UPDATE, HDCP_AUTH_UNKNOWN);
}

void hdcp_key_check(struct timer_list *timer)
{
	struct meson_hdmitx_hdcp *meson_hdcp =
		container_of(timer, struct meson_hdmitx_hdcp,
			     daemon_load_timer);

	/*send notify when key load finished or timeout.*/
	if (meson_hdcp->hdcp22_daemon_state == HDCP22_DAEMON_DONE) {
		HDMITX_INFO("hdcp_tx22 load ready, stop timer\n");
		/*meson_hdcp->key_chk_cnt = 0;*/
		schedule_delayed_work(&meson_hdcp->notify_work, HZ / 100);
	} else if (meson_hdcp->key_chk_cnt++ < TIMER_CHK_CNT) {
		meson_hdcp->daemon_load_timer.expires = jiffies + TIMER_CHECK;
		add_timer(&meson_hdcp->daemon_load_timer);
	} else {
		HDMITX_INFO("hdcp_tx22 load timeout\n");
		meson_hdcp->hdcp22_daemon_state = HDCP22_DAEMON_TIMEOUT;
		/*meson_hdcp->key_chk_cnt = 0;*/
		schedule_delayed_work(&meson_hdcp->notify_work, 0);
	}
}

bool hdcp_tx22_daemon_ready(void)
{
	bool ret = false;

	if (meson_hdcp.hdcp22_daemon_state != HDCP22_DAEMON_DONE)
		ret = false;
	else
		ret = true;
	return ret;
}

static int drm_hdmitx_register_hdcp_cb(struct drm_hdmitx_hdcp_cb *hdcp_cb)
{
	struct hdmitx_dev *hdev = get_hdmitx_device();

	hdev->drm_hdcp_cb.callback = hdcp_cb->callback;
	hdev->drm_hdcp_cb.data = hdcp_cb->data;
	return 0;
}

void meson_hdcp_init(void)
{
	int ret;
	struct drm_hdmitx_hdcp_cb hdcp_cb;
	struct hdmitx_dev *hdmitx_dev;

	meson_hdcp.hdcp_debug_type = 0x3;
	meson_hdcp.hdcp_report = HDCP_TX22_DISCONNECT;
	meson_hdcp.hdcp_en = 0;
	meson_hdcp.hdcp22_daemon_state = HDCP22_DAEMON_LOADING;
	init_waitqueue_head(&meson_hdcp.hdcp_comm_queue);
	meson_hdcp.hdcp_comm_device.minor = MISC_DYNAMIC_MINOR;
	meson_hdcp.hdcp_comm_device.name = "tee_comm_hdcp";
	meson_hdcp.hdcp_comm_device.fops = &hdcp_comm_file_operations;
	meson_hdcp.drm_hdcp_cb.hdcp_notify = NULL;
	meson_hdcp.drm_hdcp_cb.data = NULL;

	ret = misc_register(&meson_hdcp.hdcp_comm_device);
	if (ret < 0)
		HDMITX_ERROR("%s misc_register fail\n", __func__);

	hdcp_cb.callback = meson_hdmitx_hdcp_cb;
	hdcp_cb.data = &meson_hdcp;
	drm_hdmitx_register_hdcp_cb(&hdcp_cb);

	hdmitx_dev = get_hdmitx_device();
	hdmitx_dev->tx_comm.drm_hdcp.test_set_hdmi_mode = am_hdmitx_set_hdmi_mode;
	hdmitx_dev->tx_comm.drm_hdcp.test_set_out_mode = am_hdmitx_set_out_mode;
	hdmitx_dev->tx_comm.drm_hdcp.test_hdcp_disable = am_hdmitx_hdcp_disable;
	hdmitx_dev->tx_comm.drm_hdcp.test_hdcp_enable = meson_hdcp_enable;
	hdmitx_dev->tx_comm.drm_hdcp.test_hdcp_disconnect = am_hdmitx_hdcp_disconnect;

	INIT_DELAYED_WORK(&meson_hdcp.notify_work, meson_hdcp_key_notify);
	meson_hdcp.key_chk_cnt = 0;
	timer_setup(&meson_hdcp.daemon_load_timer, hdcp_key_check, 0);
	meson_hdcp.daemon_load_timer.expires = jiffies + TIMER_CHECK;
	add_timer(&meson_hdcp.daemon_load_timer);
}

void meson_hdcp_exit(void)
{
	misc_deregister(&meson_hdcp.hdcp_comm_device);
	del_timer_sync(&meson_hdcp.daemon_load_timer);
	cancel_delayed_work(&meson_hdcp.notify_work);
}

/* bit[1]: hdcp22, bit[0]: hdcp14 */
/* apart from hdcp key, it will also check hdcp2.2 daemon status */
unsigned int meson_hdcp_get_tx_cap(void)
{
	struct hdmitx_dev *hdev = get_hdmitx_device();
	unsigned int hdcp_tx_type = meson_hdcp_get_tx_key_version();

	/* check hdcp daemon: android or daemon is running */
	if (hdev->tx_comm.hdcp_ctl_lvl > 0 && !hdcp_tx22_daemon_ready())
		hdcp_tx_type &= 0x1;

	return hdcp_tx_type & 0x3;
}

/* bit[1]: hdcp22, bit[0]: hdcp14 */
unsigned int meson_hdcp_get_rx_cap(void)
{
	unsigned int ver = 0x0;
	struct hdmitx_dev *hdev = get_hdmitx_device();

	/*
	 * note that during hdcp1.4 authentication, read hdcp version
	 * of connected TV set(capable of hdcp2.2) may cause TV
	 * switch its hdcp mode, and flash screen. should not
	 * read hdcp version of sink during hdcp1.4 authentication.
	 * if hdcp1.4 authentication currently, force return hdcp1.4
	 */
	if (hdev->tx_comm.hdcp_mode == 1)
		return 0x1;
	/* if TX don't have HDCP22 key, skip RX hdcp22 ver */
	if (hdmitx_hw_cntl_ddc(&hdev->hw_comm,
		DDC_HDCP_22_LSTORE, 0) == 0)
		return 0x1;

	/* Detect RX support HDCP22 */
	ver = hdcp_rd_hdcp22_ver();
	/* Here, must assume RX support HDCP14, otherwise affect 1A-03 */
	if (ver)
		return 0x3;
	else
		return 0x1;
}

/* after TEE hdcp key valid, do hdcp22 init before tx22 start */
void drm_hdmitx_hdcp22_init(void)
{
	struct hdmitx_dev *hdev = get_hdmitx_device();

	hdmitx_hdcp_do_work(&hdev->tx_comm);
	hdmitx_hw_cntl_ddc(&hdev->hw_comm,
		DDC_HDCP_MUX_INIT, 2);
}

/* echo 1/2 > hdcp_mode */
void drm_hdmitx_enable_hdcp_mode(unsigned int content_type)
{
	struct hdmitx_dev *hdev = get_hdmitx_device();
	enum hdmi_vic vic = HDMI_0_UNKNOWN;

	vic = hdmitx_hw_get_state(&hdev->hw_comm, STAT_VIDEO_VIC, 0);
	hdmitx_hw_cntl_ddc(&hdev->hw_comm, DDC_HDCP_GET_AUTH, 0);

	if (content_type == 1) {
		hdmitx_hw_cntl_ddc(&hdev->hw_comm, DDC_HDCP_MUX_INIT, 1);
		if (vic == HDMI_17_720x576p50_4x3 || vic == HDMI_18_720x576p50_16x9)
			usleep_range(500000, 500010);
		hdev->tx_comm.hdcp_mode = 1;
		hdmitx_hdcp_do_work(&hdev->tx_comm);
		hdmitx_hw_cntl_ddc(&hdev->hw_comm,
			DDC_HDCP_OP, HDCP14_ON);
	} else if (content_type == 2) {
		hdev->tx_comm.hdcp_mode = 2;
		hdmitx_hdcp_do_work(&hdev->tx_comm);
		/*
		 * for drm hdcp_tx22, esm init only once
		 * don't do HDCP22 IP reset after init done!
		 */
		hdmitx_hw_cntl_ddc(&hdev->hw_comm,
			DDC_HDCP_MUX_INIT, 3);
	}
}

/* echo -1 > hdcp_mode;echo stop14/22 > hdcp_ctrl */
void drm_hdmitx_disable_hdcp_mode(unsigned int content_type)
{
	struct hdmitx_dev *hdev = get_hdmitx_device();

	hdmitx_hw_cntl_ddc(&hdev->hw_comm, DDC_HDCP_MUX_INIT, 1);
	hdmitx_hw_cntl_ddc(&hdev->hw_comm, DDC_HDCP_GET_AUTH, 0);

	if (content_type == 1) {
		hdmitx_hw_cntl_ddc(&hdev->hw_comm,
			DDC_HDCP_OP, HDCP14_OFF);
	} else if (content_type == 2) {
		hdmitx_hw_cntl_ddc(&hdev->hw_comm,
			DDC_HDCP_OP, HDCP22_OFF);
	}

	hdev->tx_comm.hdcp_mode = 0;
	hdmitx_hdcp_do_work(&hdev->tx_comm);
	hdmitx_current_status(HDMITX_HDCP_NOT_ENABLED);
}

unsigned char drm_hdmitx_get_hdcp_topo_info(void)
{
	struct hdmitx_dev *hdev = get_hdmitx_device();

	HDMITX_DEBUG("%s %d\n", __func__, hdev->tx_comm.hdcp22_type);
	return hdev->tx_comm.hdcp22_type;
}

/* DRM HDCP API */
static struct drm_hdcp_ctrl_ops tx20_drm_hdcp_ctrl_ops = {
	.hdcp_init = meson_hdcp_init,
	.hdcp_exit = meson_hdcp_exit,
	.hdcp_enable = meson_hdcp_enable,
	.hdcp_disable = meson_hdcp_disable,
	.hdcp_disconnect = meson_hdcp_disconnect,
	.get_tx_hdcp_cap = meson_hdcp_get_tx_cap,
	.get_rx_hdcp_cap = meson_hdcp_get_rx_cap,
	.register_hdcp_notify = meson_hdcp_reg_result_notify,
	.get_dw_hdcp_topo_info = drm_hdmitx_get_hdcp_topo_info,
};

/* HDCP API */
static struct hdcp_ctrl_ops tx20_hdcp_ctrl_ops = {
	.set_hdcp_mode = hdmitx20_set_hdcp_mode,
	.get_hdcp_ver = hdmitx20_get_hdcp_ver,
};

const struct file_operations hdcplog_ops = {
	.read = hdcplog_read,
	.open = hdcplog_open,
};

static struct dentry *hdmitx_dbgfs;

int hdmitx20_hdcp_init(struct hdmitx_dev *hdev)
{
	struct dentry *entry;
	struct hdmitx_common *tx_comm = &hdev->tx_comm;

	tx_comm->drm_hdcp_ctrl_ops = &tx20_drm_hdcp_ctrl_ops;
	tx_comm->hdcp_ctrl_ops = &tx20_hdcp_ctrl_ops;
	if (!hdev->tx_comm.hdtx_dev) {
		HDMITX_DEBUG_HDCP("exit for null device of hdmitx!\n");
		return -ENODEV;
	}

	esm_init();
	memset(&hdcplog_buf, 0, sizeof(hdcplog_buf));
	init_waitqueue_head(&hdcplog_buf.wait);

	tx_comm->task_hdcp = kthread_run(hdmitx_hdcp_task,	(void *)hdev,
				      "kthread_hdcp");

	hdmitx_dbgfs = hdmitx_get_dbgfsdentry();
	if (!hdmitx_dbgfs)
		hdmitx_dbgfs = debugfs_create_dir(DEVICE_NAME, NULL);
	if (!hdmitx_dbgfs) {
		HDMITX_ERROR("can't create %s debugfs dir\n", DEVICE_NAME);
		return 0;
	}
	entry = debugfs_create_file("hdcp_log", S_IFREG | 0444,
			hdmitx_dbgfs, NULL,
			&hdcplog_ops);
	if (!entry)
		HDMITX_ERROR("debugfs create file %s failed\n", "hdcp_log");
	return 0;
}

void hdmitx20_hdcp_exit(struct hdmitx_dev *hdev)
{
	struct hdmitx_common *tx_comm = &hdev->tx_comm;

	if (hdev && tx_comm) {
		kthread_stop(tx_comm->task_hdcp);
		cancel_delayed_work_sync(&hdev->work_do_hdcp);
		kfree(tx_comm->topo_info);
		tx_comm->topo_info = NULL;
	}
	esm_exit();
}

MODULE_PARM_DESC(hdmi_authenticated, "\n hdmi_authenticated\n");
module_param(hdmi_authenticated, int, 0444);
