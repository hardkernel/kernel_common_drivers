// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 */

#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/export.h>
#include <linux/io.h>
#include <linux/interrupt.h>
#include <linux/mailbox_controller.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/mailbox_client.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/list.h>
#include <linux/amlogic/aml_mbox.h>
#include <dt-bindings/mailbox/amlogic,mbox.h>
#include "meson_mbox_devfs.h"
#include <asm/siginfo.h>

#define DRIVER_NAME		"mbox-devfs"

struct aml_mbox_dev {
	struct list_head list;
	dev_t dev_t;
	struct cdev cdev;
	struct device *dev;
	struct device *p_dev;
	struct mbox_chan *mbox_chan;
	struct fasync_struct *async_queue;
	struct class *class;
	const char *name;
	u32 dest;
	u32 mbox_nums;
	void *priv_data;
};

struct aml_mbox_priv_data {
	u32 cmd;
	char data[MBOX_USER_SIZE];
} __packed;

struct aml_mbox_priv {
	struct aml_mbox_dev *aml_dev;
	struct aml_mbox_data *aml_ree2remote_data;
	struct aml_mbox_data *aml_ao2ree_data;
	struct aml_mbox_data *aml_dspa2ree_data;
	struct aml_mbox_data *aml_dspb2ree_data;
};

static ssize_t mbox_message_write(struct file *filp,
				  const char __user *userbuf,
				  size_t count, loff_t *ppos)
{
	struct aml_mbox_priv *aml_priv = filp->private_data;
	struct aml_mbox_dev *aml_dev = aml_priv->aml_dev;
	struct mbox_chan *mbox_chan = aml_dev->mbox_chan;
	struct aml_mbox_chan *aml_chan = mbox_chan->con_priv;
	struct aml_mbox_data *ree2remote_data = aml_priv->aml_ree2remote_data;
	struct aml_mbox_priv_data aml_priv_data;
	struct device *dev = aml_dev->p_dev;
	int ret = 0;

	if (count > MBOX_USER_SIZE) {
		dev_err(dev, "Msg len %zd over range dest %d\n", count, aml_dev->dest);
		return -EINVAL;
	}
	ret = copy_from_user(&aml_priv_data, userbuf, count);
	if (ret)
		return ret;

	ree2remote_data->txbuf = aml_priv_data.data;
	ree2remote_data->cmd = aml_priv_data.cmd;
	ree2remote_data->txsize = count - sizeof(u32);

	switch (aml_dev->dest) {
	case MAILBOX_AOCPU:
		ree2remote_data->rxsize = count - sizeof(u32);
		ree2remote_data->sync = MBOX_SYNC;
		break;
	case MAILBOX_DSP:
		ree2remote_data->rxsize = MBOX_DATA_SIZE;
		ree2remote_data->sync = MBOX_TSYNC;
		break;
	case MAILBOX_SECPU:
		ree2remote_data->txbuf = &aml_priv_data;
		ree2remote_data->rxsize = 0;
		ree2remote_data->txsize = count;
		ree2remote_data->sync = MBOX_SYNC;
		break;
	default:
		break;
	};

	mutex_lock(&aml_chan->mutex);
	ret = mbox_send_message(mbox_chan, ree2remote_data);
	if (ret < 0) {
		dev_err(dev, "Msg send fail %d dest %d\n", ret, aml_dev->dest);
		complete(&ree2remote_data->complete);
		mutex_unlock(&aml_chan->mutex);
		return ret;
	}

	if (ree2remote_data->sync == MBOX_SYNC)
		complete(&ree2remote_data->complete);
	mutex_unlock(&aml_chan->mutex);
	return count;
}

static ssize_t mbox_message_read(struct file *filp, char __user *userbuf,
				 size_t count, loff_t *ppos)
{
	struct aml_mbox_priv *aml_priv = filp->private_data;
	struct aml_mbox_dev *aml_dev = aml_priv->aml_dev;
	struct aml_mbox_data *ree2remote_data = aml_priv->aml_ree2remote_data;
	struct aml_mbox_data *ao2ree_data = aml_priv->aml_ao2ree_data;
	struct aml_mbox_data *dspa2ree_data = aml_priv->aml_dspa2ree_data;
	struct aml_mbox_data *dspb2ree_data = aml_priv->aml_dspb2ree_data;
	struct device *dev = aml_dev->p_dev;
	int ret = 0;
	int rxsize;
	void *rxbuf;

	if (aml_dev->dest != MAILBOX_ARM) {
		ret = wait_for_completion_killable(&ree2remote_data->complete);
		if (ret < 0) {
			dev_err(dev, "Read msg wait killed %d\n", ret);
			return -ENXIO;
		}
	}

	barrier();
	*ppos = 0;
	if (count > MBOX_USER_SIZE)
		count = MBOX_USER_SIZE;

	switch (aml_dev->dest) {
	case MAILBOX_ARM:
		if (!strcmp(aml_dev->name, "aocpu2ree")) {
			rxbuf = ao2ree_data->rxbuf;
			rxsize = ao2ree_data->rxsize;
		} else if (!strcmp(aml_dev->name, "dspa2ree")) {
			rxbuf = dspa2ree_data->rxbuf;
			rxsize = dspa2ree_data->rxsize;
		} else if (!strcmp(aml_dev->name, "dspb2ree")) {
			rxbuf = dspb2ree_data->rxbuf;
			rxsize = dspb2ree_data->rxsize;
		} else {
			dev_err(dev, "Error: Unrecognized device source\n");
			return -ENXIO;
		}
		break;
	default:
		rxbuf = ree2remote_data->rxbuf;
		rxsize = ree2remote_data->rxsize;
		break;
	}

	if (!rxbuf) {
		dev_err(dev, "Error: rxbuf is NULL!\n");
		return -ENXIO;
	}

	rxsize = count > rxsize ? rxsize : count;
	dev_dbg(dev, "%s: rxsize = %d\n", __func__, rxsize);
	ret = simple_read_from_buffer(userbuf, rxsize, ppos,
			rxbuf, MBOX_USER_SIZE);

	return ret;
}

static long mbox_message_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	return 0;
}

static int mbox_message_open(struct inode *inode, struct file *filp)
{
	struct cdev *cdev = inode->i_cdev;
	struct aml_mbox_dev *aml_dev = container_of(cdev, struct aml_mbox_dev, cdev);
	struct device *dev = aml_dev->p_dev;
	struct aml_mbox_priv *aml_priv;
	struct aml_mbox_data *ree2remote_data;
	struct aml_mbox_data *ao2ree_data;
	struct aml_mbox_data *dspa2ree_data;
	struct aml_mbox_data *dspb2ree_data;

	aml_priv = devm_kzalloc(aml_dev->p_dev, sizeof(*aml_priv), GFP_KERNEL);
	if (IS_ERR(aml_priv))
		return -ENOMEM;

	dev_dbg(dev, "Open dev name: %s\n", aml_dev->name);

	switch (aml_dev->dest) {
	case MAILBOX_AOCPU:
	case MAILBOX_DSP:
	case MAILBOX_SECPU:
		ree2remote_data = devm_kzalloc(aml_dev->p_dev, sizeof(*ree2remote_data),
					       GFP_KERNEL);
		if (IS_ERR(ree2remote_data)) {
			devm_kfree(aml_dev->p_dev, aml_priv);
			return -ENOMEM;
		}

		ree2remote_data->rxbuf = devm_kzalloc(aml_dev->p_dev, MBOX_USER_SIZE,
						      GFP_KERNEL);
		if (IS_ERR(ree2remote_data->rxbuf)) {
			devm_kfree(aml_dev->p_dev, aml_priv);
			devm_kfree(aml_dev->p_dev, ree2remote_data);
			return -ENOMEM;
		}

		init_completion(&ree2remote_data->complete);
		aml_priv->aml_ree2remote_data = ree2remote_data;
		break;
	case MAILBOX_ARM:
		if (!strcmp(aml_dev->name, "aocpu2ree")) {
			ao2ree_data = devm_kzalloc(aml_dev->p_dev,
					sizeof(*ao2ree_data), GFP_KERNEL);
			if (IS_ERR(ao2ree_data)) {
				devm_kfree(aml_dev->p_dev, aml_priv);
				return -ENOMEM;
			}

			ao2ree_data->rxbuf = devm_kzalloc(aml_dev->p_dev,
					MBOX_DATA_SIZE, GFP_KERNEL);
			if (IS_ERR(ao2ree_data->rxbuf)) {
				devm_kfree(aml_dev->p_dev, aml_priv);
				devm_kfree(aml_dev->p_dev, ao2ree_data);
				return -ENOMEM;
			}
			aml_priv->aml_ao2ree_data = ao2ree_data;
		}

		if (!strcmp(aml_dev->name, "dspa2ree")) {
			dspa2ree_data = devm_kzalloc(aml_dev->p_dev,
					sizeof(*dspa2ree_data), GFP_KERNEL);
			if (IS_ERR(dspa2ree_data)) {
				devm_kfree(aml_dev->p_dev, aml_priv);
				return -ENOMEM;
			}

			dspa2ree_data->rxbuf = devm_kzalloc(aml_dev->p_dev,
					MBOX_DATA_SIZE, GFP_KERNEL);
			if (IS_ERR(dspa2ree_data->rxbuf)) {
				devm_kfree(aml_dev->p_dev, aml_priv);
				devm_kfree(aml_dev->p_dev, dspa2ree_data);
				return -ENOMEM;
			}
			aml_priv->aml_dspa2ree_data = dspa2ree_data;
		}

		if (!strcmp(aml_dev->name, "dspb2ree")) {
			dspb2ree_data = devm_kzalloc(aml_dev->p_dev,
					sizeof(*dspb2ree_data), GFP_KERNEL);
			if (IS_ERR(dspb2ree_data)) {
				devm_kfree(aml_dev->p_dev, aml_priv);
				return -ENOMEM;
			}

			dspb2ree_data->rxbuf = devm_kzalloc(aml_dev->p_dev,
					MBOX_DATA_SIZE, GFP_KERNEL);
			if (IS_ERR(dspb2ree_data->rxbuf)) {
				devm_kfree(aml_dev->p_dev, aml_priv);
				devm_kfree(aml_dev->p_dev, dspb2ree_data);
				return -ENOMEM;
			}
			aml_priv->aml_dspb2ree_data = dspb2ree_data;
		}
		break;
	default:
		break;
	};

	aml_priv->aml_dev = aml_dev;
	aml_dev->priv_data = aml_priv;
	filp->private_data = aml_priv;
	return 0;
}

static int mbox_message_release(struct inode *inode, struct file *filp)
{
	struct aml_mbox_priv *aml_priv = filp->private_data;
	struct aml_mbox_dev *aml_dev = aml_priv->aml_dev;

	switch (aml_dev->dest) {
	case MAILBOX_AOCPU:
	case MAILBOX_DSP:
	case MAILBOX_SECPU:
		devm_kfree(aml_dev->p_dev, aml_priv->aml_ree2remote_data->rxbuf);
		devm_kfree(aml_dev->p_dev, aml_priv->aml_ree2remote_data);
		break;
	case MAILBOX_ARM:
		if (!strcmp(aml_dev->name, "aocpu2ree")) {
			devm_kfree(aml_dev->p_dev, aml_priv->aml_ao2ree_data->rxbuf);
			devm_kfree(aml_dev->p_dev, aml_priv->aml_ao2ree_data);
		}

		if (!strcmp(aml_dev->name, "dspa2ree")) {
			devm_kfree(aml_dev->p_dev, aml_priv->aml_dspa2ree_data->rxbuf);
			devm_kfree(aml_dev->p_dev, aml_priv->aml_dspa2ree_data);
		}

		if (!strcmp(aml_dev->name, "dspb2ree")) {
			devm_kfree(aml_dev->p_dev, aml_priv->aml_dspb2ree_data->rxbuf);
			devm_kfree(aml_dev->p_dev, aml_priv->aml_dspb2ree_data);
		}
		break;
	default:
		break;
	};

	devm_kfree(aml_dev->p_dev, aml_priv);
	return 0;
}

static int mbox_message_fasync(int fd, struct file *filp, int on)
{
	int ret = 0;
	struct aml_mbox_priv *aml_priv = filp->private_data;
	struct aml_mbox_dev *aml_dev = aml_priv->aml_dev;
	struct device *dev = aml_dev->p_dev;

	if (aml_dev->dest == MAILBOX_ARM) {
		dev_dbg(dev, "register mbox message fasync signal\n");
		ret = fasync_helper(fd, filp, on, &aml_dev->async_queue);
	}

	return ret;
}

static const struct file_operations mbox_message_ops = {
	.write		= mbox_message_write,
	.read		= mbox_message_read,
	.open		= mbox_message_open,
	.release	= mbox_message_release,
	.unlocked_ioctl = mbox_message_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl	= mbox_message_ioctl,
#endif
	.fasync		= mbox_message_fasync,
};

static void mbox_ao2ree_rx_callback(struct mbox_client *cl, void *mssg)
{
	struct device *dev = cl->dev;
	struct aml_mbox_priv *aml_priv;
	struct aml_mbox_rx_data *aml_rx_data;
	struct aml_mbox_data *ao2ree_data;
	struct aml_mbox_dev *aml_devs;
	struct aml_mbox_dev *aml_dev;
	u32 cmd;
	u32 mbox_nums;
	int idx;

	aml_rx_data = mssg;
	cmd = aml_rx_data->cmd;
	dev_dbg(dev, "%s: cmd = 0x%x\n", __func__, (unsigned int)cmd);

	aml_devs = dev_get_drvdata(dev);
	mbox_nums = aml_devs->mbox_nums;
	for (idx = 0; idx < mbox_nums; idx++) {
		aml_dev = &aml_devs[idx];
		if (!strcmp(aml_dev->name, "aocpu2ree"))
			break;
	}

	if (idx == mbox_nums) {
		dev_err(dev, "Can't find aocpu2ree dev\n");
		return;
	}

	aml_priv = aml_dev->priv_data;
	if (IS_ERR_OR_NULL(aml_priv)) {
		dev_err(dev, "Err: Device %s has not been opened yet\n",
			aml_dev->name);
		return;
	}

	ao2ree_data = aml_priv->aml_ao2ree_data;
	if (!ao2ree_data || !ao2ree_data->rxbuf) {
		dev_err(dev, "Err: ao2ree RX pointer is NULL\n");
	} else {
		memcpy(ao2ree_data->rxbuf, aml_rx_data->buf, aml_rx_data->size);
		ao2ree_data->rxsize = aml_rx_data->size;
		dev_dbg(dev, "ao2ree: notify userspace to read data\n");
		kill_fasync(&aml_dev->async_queue, SIGIO, POLL_IN);
	}
}

static int mbox_dsp2ree_notify(struct aml_mbox_dev *aml_dev, void *mssg)
{
	struct device *dev = aml_dev->p_dev;
	struct aml_mbox_priv *aml_priv;
	struct aml_mbox_rx_data *aml_rx_data;
	struct aml_mbox_data *dsp2ree_data = NULL;
	u32 cmd;
	int ret = 0;

	aml_rx_data = mssg;
	cmd = aml_rx_data->cmd;
	dev_dbg(dev, "%s: cmd = 0x%x\n", __func__, (unsigned int)cmd);

	aml_priv = aml_dev->priv_data;
	if (IS_ERR_OR_NULL(aml_priv)) {
		dev_err(dev, "Err: Device %s has not been opened yet\n",
			aml_dev->name);
		return PTR_ERR(aml_priv);
	}

	if (!strcmp(aml_dev->name, "dspa2ree"))
		dsp2ree_data = aml_priv->aml_dspa2ree_data;
	else if (!strcmp(aml_dev->name, "dspb2ree"))
		dsp2ree_data = aml_priv->aml_dspb2ree_data;

	if (!dsp2ree_data || !dsp2ree_data->rxbuf) {
		dev_err(dev, "Err: dsp2ree RX pointer is NULL\n");
		ret = -EINVAL;
	} else {
		memcpy(dsp2ree_data->rxbuf, aml_rx_data->buf, aml_rx_data->size);
		dsp2ree_data->rxsize = aml_rx_data->size;
		dev_dbg(dev, "dsp2ree: notify userspace to read data\n");
		kill_fasync(&aml_dev->async_queue, SIGIO, POLL_IN);
	}

	return ret;
}

static void mbox_dspa2ree_rx_callback(struct mbox_client *cl, void *mssg)
{
	struct device *dev = cl->dev;
	struct aml_mbox_dev *aml_devs;
	struct aml_mbox_dev *aml_dev;
	u32 mbox_nums;
	int idx;

	aml_devs = dev_get_drvdata(dev);
	mbox_nums = aml_devs->mbox_nums;
	for (idx = 0; idx < mbox_nums; idx++) {
		aml_dev = &aml_devs[idx];
		if (!strcmp(aml_dev->name, "dspa2ree"))
			break;
	}

	if (idx == mbox_nums) {
		dev_err(dev, "Can't find dspa2ree dev\n");
		return;
	}

	mbox_dsp2ree_notify(aml_dev, mssg);
}

static void mbox_dspb2ree_rx_callback(struct mbox_client *cl, void *mssg)
{
	struct device *dev = cl->dev;
	struct aml_mbox_dev *aml_devs;
	struct aml_mbox_dev *aml_dev;
	u32 mbox_nums;
	int idx;

	aml_devs = dev_get_drvdata(dev);
	mbox_nums = aml_devs->mbox_nums;
	for (idx = 0; idx < mbox_nums; idx++) {
		aml_dev = &aml_devs[idx];
		if (!strcmp(aml_dev->name, "dspb2ree"))
			break;
	}

	if (idx == mbox_nums) {
		dev_err(dev, "Can't find dspb2ree dev\n");
		return;
	}

	mbox_dsp2ree_notify(aml_dev, mssg);
}

static int mbox_cdev_init(struct device *dev)
{
	struct class *mbox_class;
	struct aml_mbox_dev *mbox_devs;
	struct aml_mbox_dev *mbox_dev;
	dev_t dev_t;
	u32 idx;
	int ret = 0;
	int mbox_nums = 0;
	int i;

	dev_dbg(dev, "mbox devfs init start\n");
	ret = of_property_read_u32(dev->of_node,
			     "mbox-nums", &mbox_nums);
	if (ret < 0) {
		dev_err(dev, "No mbox-nums\n");
		return -EINVAL;
	}

	mbox_class = class_create("mbox_devfs");
	if (IS_ERR(mbox_class)) {
		dev_err(dev, "Failed to create class: %ld\n", PTR_ERR(mbox_class));
		return PTR_ERR(mbox_class);
	}

	ret = alloc_chrdev_region(&dev_t, 0, mbox_nums, DRIVER_NAME);
	if (ret < 0) {
		dev_err(dev, "alloc dev_t failed\n");
		goto class_err;
	}

	mbox_devs = devm_kzalloc(dev, sizeof(*mbox_dev) * mbox_nums, GFP_KERNEL);
	if (IS_ERR(mbox_devs)) {
		ret = -ENOMEM;
		dev_err(dev, "Failed to alloc mbox_devs\n");
		goto chrdev_err;
	}

	for (idx = 0; idx < mbox_nums; idx++) {
		mbox_dev = &mbox_devs[idx];
		mbox_dev->p_dev = dev;
		mbox_dev->class = mbox_class;
		mbox_dev->dev_t = MKDEV(MAJOR(dev_t), idx);
		cdev_init(&mbox_dev->cdev, &mbox_message_ops);
		ret = cdev_add(&mbox_dev->cdev, mbox_dev->dev_t, 1);
		if (ret) {
			dev_err(dev, "mbox fail to add cdev\n");
			goto cdev_err;
		}

		if (of_property_read_string_index(dev->of_node,
						  "mbox-names", idx, &mbox_dev->name)) {
			ret = -ENOMEM;
			dev_err(dev, "%s get mbox[%d] name fail\n",
				__func__, idx);
			goto cdev_err;
		}

		if (of_property_read_u32_index(dev->of_node, "mbox-dests",
					       idx, &mbox_dev->dest)) {
			ret = -ENOMEM;
			dev_err(dev, "%s get mbox[%d] dest fail\n",
				__func__, idx);
			goto cdev_err;
		}

		mbox_dev->dev = device_create(mbox_class, NULL, mbox_dev->dev_t,
					mbox_dev, "%s", mbox_dev->name);
		if (IS_ERR(mbox_dev->dev)) {
			ret = PTR_ERR(mbox_dev->dev);
			dev_err(dev, "Failed to create device: %d\n", ret);
			goto device_err;
		}
		mbox_dev->mbox_chan = aml_mbox_request_channel_byidx(dev, idx);
		if (IS_ERR(mbox_dev->mbox_chan)) {
			ret = PTR_ERR(mbox_dev->dev);
			dev_err(dev, "Failed to request mbox chan: %d\n", ret);
			goto chan_err;
		}
		mbox_dev->mbox_nums = mbox_nums;

		if (!strcmp(mbox_dev->name, "aocpu2ree"))
			mbox_dev->mbox_chan->cl->rx_callback = mbox_ao2ree_rx_callback;
		else if (!strcmp(mbox_dev->name, "dspa2ree"))
			mbox_dev->mbox_chan->cl->rx_callback = mbox_dspa2ree_rx_callback;
		else if (!strcmp(mbox_dev->name, "dspb2ree"))
			mbox_dev->mbox_chan->cl->rx_callback = mbox_dspb2ree_rx_callback;
	}
	dev_set_drvdata(dev, mbox_devs);
	dev_dbg(dev, "mbox devfs init done\n");
	return 0;
chan_err:
	for (i = 0; i < idx; i++)
		mbox_free_channel(mbox_devs[i].mbox_chan);
device_err:
	for (i = 0; i < idx; i++)
		device_destroy(mbox_class, mbox_devs[i].dev_t);
cdev_err:
	for (i = 0; i < idx; i++)
		cdev_del(&mbox_devs[i].cdev);
chrdev_err:
	unregister_chrdev_region(dev_t, mbox_nums);
class_err:
	class_destroy(mbox_class);

	return ret;
}

static int mbox_devfs_probe(struct platform_device *pdev)
{
	return mbox_cdev_init(&pdev->dev);
}

static void mbox_devfs_remove(struct platform_device *pdev)
{
	struct aml_mbox_dev *mbox_devs = dev_get_drvdata(&pdev->dev);
	u32 mbox_nums = mbox_devs->mbox_nums;
	dev_t base_dev = mbox_devs[0].dev_t;
	struct class *mbox_class = mbox_devs[0].class;
	int i;

	for (i = 0; i < mbox_nums; i++) {
		device_destroy(mbox_class, mbox_devs[i].dev_t);
		cdev_del(&mbox_devs[i].cdev);
	}

	class_destroy(mbox_class);
	unregister_chrdev_region(base_dev, mbox_nums);
	platform_set_drvdata(pdev, NULL);
}

static const struct of_device_id mbox_of_match[] = {
	{ .compatible = "amlogic, mbox-devfs" },
	{},
};

static struct platform_driver mbox_devfs_drvier = {
	.probe = mbox_devfs_probe,
	.remove = mbox_devfs_remove,
	.driver = {
		.owner		= THIS_MODULE,
		.name		= DRIVER_NAME,
		.of_match_table = mbox_of_match,
	},
};

int __init mbox_devfs_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&mbox_devfs_drvier);
	return ret;
}

void __exit mbox_devfs_exit(void)
{
	platform_driver_unregister(&mbox_devfs_drvier);
}
