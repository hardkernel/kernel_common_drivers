// SPDX-License-Identifier: (GPL-2.0+ OR MIT)
/*
 * Copyright (c) 2025 Amlogic, Inc. All rights reserved.
 */

#include <linux/clk.h>
#include <linux/completion.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/types.h>
#include <linux/reset-controller.h>
#include <linux/reset.h>
#include <linux/delay.h>
#ifdef CONFIG_AMLOGIC_MODIFY
#include <linux/pm_runtime.h>
#include <linux/pm_domain.h>
#include <linux/pinctrl/consumer.h>
#include <linux/arm-smccc.h>
#include <linux/amlogic/secure_pwm_i2c.h>
#endif

// #define I2C_DEBUG
// #define I2C_PXP
#define FUTURE_USE		0
/* Meson I2C register map */
#define REG_CFG_RDY		0x00
#define REG_CFG_I2C		0x04
#define REG_CFG_START		0x08
#define REG_CFG_BUS		0x0c
#define REG_TX_RD_ADDR		0x10
#define REG_TX_WR_ADDR		0x14
#define REG_RX_RD_ADDR		0x18
#define REG_RX_WR_ADDR		0x1c
#define REG_CGF_TX		0x20
#define REG_CGF_RX		0x24
#define REG_CGF_IRQ_STATE	0x30
#define REG_CGF_IRQ_ENABLE	0x34
#define REG_CGF_SHAKE_BLK	0x38
#define REG_FIFO_BASE	0x80
/* CFG RDY fields */
#define RDY_TEE_ONLY		BIT(1)
#define RDY_IF			BIT(0)

/* CFG I2C fields */
#define I2C_RX_THR		GENMASK(23, 16)
#define I2C_TX_THR		GENMASK(15, 8)
#define I2C_DMA			BIT(4)
#define I2C_HW_NEG		BIT(3)
#define I2C_HW_POS		BIT(2)
#define I2C_JOIN_RX		BIT(1)
#define I2C_JOIN_TX		BIT(0)

/* CFG BUS fields */
#define BUS_SLAVE_MODE		BIT(16)
#define BUS_FILTER_MASK		GENMASK(15, 12)
/*SCL = clk/b_ratio	if b_ratio<=8, SCL = clk/8 */
#define BUS_RATIO_MASK		GENMASK(11, 0)

/* CFG START fields */
#define START_STRAT		BIT(31)
#define START_LEN		GENMASK(23, 12)
#define START_10_BITS		BIT(11) /*only 7bit now*/
#define START_SLAVE_ADDR	GENMASK(10, 1)
#define START_READ		BIT(0)

/* CFG TX_RX fields */
#define TX_RX_EMPTY		BIT(9)
#define TX_RX_FULL		BIT(8)
#define TX_RX_DATA		GENMASK(7, 0)

/*CGF IRQ fields */
#define RCH_ERROR		BIT(0)
#define WCH_ERROR		BIT(1)
#define NCK_ERROR		BIT(2)
#define PHY_DONE		BIT(3)
// #define TX_FULL			BIT(4)
// #define RX_FIFO_READ		BIT(5)
// #define TX_FIFO_WRITE		BIT(6)

#define TRANS_ERROR		(RCH_ERROR | WCH_ERROR | NCK_ERROR)
#define TRANS_DONE		(PHY_DONE | TRANS_ERROR)

/* keep the same with I2C_TIMEOUT_MS */
#define MESON_I2C_PM_TIMEOUT	1000
#define I2C_TIMEOUT_MS		1000
#define SCHEDULE_TIMEOUT_US		300
// #define M_DEBUG
enum {
	STATE_IDLE,
	STATE_READ,
	STATE_WRITE,
	STATE_WAIT,
};

struct meson_i2c_data {
	bool tee;
	u32 tee_id;
};

/**
 * struct meson_i2c - Meson I2C device private data
 *
 * @adap:	I2C adapter instance
 * @dev:	Pointer to device structure
 * @regs:	Base address of the device memory mapped registers
 * @clk:	Pointer to clock structure
 * @msg:	Pointer to the current I2C message
 * @state:	Current state in the driver state machine
 * @count:	Number of bytes to be sent/received in current transfer
 * @pos:	Current position in the send/receive buffer
 * @error:	Flag set when an error is received
 * @lock:	To avoid race conditions between irq handler and xfer code
 * @done:	Completion used to wait for transfer termination
 * @frequency:	Operating frequency of I2C bus clock
 * @data:	Pointer to the controller's platform data
 */
struct meson_i2c {
	struct i2c_adapter	adap;
	struct device		*dev;
	void __iomem		*regs;
	struct clk		*clk;

	struct i2c_msg		*msg;
	int			state;
	int			count;
	int			pos;
	int			error;
	int			fifo_depth;

	spinlock_t		lock;		//protection for i2c controller critical section
	struct completion	done;
#ifdef CONFIG_AMLOGIC_MODIFY
	unsigned long		clk_rate;
	unsigned int		frequency;
	int retain_fastmode;
	int irq;
	bool is_runtime_sleep;
#endif
	struct meson_i2c_data		*data;
	struct reset_control *rst;
};

static void meson_i2c_writel(struct meson_i2c *i2c, u32 data, int reg)
{
#if FUTURE_USE
	struct arm_smccc_res res;

	if (i2c->data->tee)
		arm_smccc_smc(SECURE_PWM_I2C, SECID_I2C, i2c->data->tee_id,
			reg, data, 0, 0, 0, &res);
	else
#endif
		writel(data, i2c->regs + reg);
}

#ifdef I2C_DEBUG
static inline void meson_i2c_dump_reg(struct meson_i2c *i2c, const char *func, int line)
{
	u32 data;
	int i;

	dev_err(i2c->dev, " %s, line:%d\n", func, line);
	for (i = 0 ; i < 8; i++) {
		data = readl(i2c->regs + i * 4);
		dev_err(i2c->dev, "i2c reg%x : 0x%x\n", i, data);
	}
	for (i = 0xb ; i < 0xf; i++) {
		data = readl(i2c->regs + i * 4);
		dev_err(i2c->dev, "i2c reg%x : 0x%x\n", i, data);
	}
}
#endif

static void meson_i2c_set_mask(struct meson_i2c *i2c, int reg, u32 mask,
			       u32 val)
{
	u32 data;

	data = readl(i2c->regs + reg);
	data &= ~mask;
	data |= val & mask;
	meson_i2c_writel(i2c, data, reg);
}

/*
 * SCL = clk/b_ratio. if b_ratio<=8, SCL = clk/8
 */
static void meson_i2c_set_clk_div(struct meson_i2c *i2c)
{
	unsigned long clk_rate = i2c->clk_rate;
	unsigned int div;

	div = DIV_ROUND_UP(clk_rate, i2c->frequency);

	/* clock divider has 12 bits */
	if (div >= (1 << 12))
		dev_err(i2c->dev, "requested bus frequency too low\n");
	else if (div <= 8)
		dev_err(i2c->dev, "requested bus frequency too high\n");
#ifdef I2C_PXP
	div = 0x40;
#endif
	/*CFG_BUS reg:11 - 0 bits*/
	meson_i2c_set_mask(i2c, REG_CFG_BUS, BUS_RATIO_MASK, div);
	dev_dbg(i2c->dev, "%s: clk %lu, freq %u, div %u\n",
		__func__, clk_rate, i2c->frequency, div);
}

static void meson_i2c_reset_fifo(struct meson_i2c *i2c)
{
	meson_i2c_writel(i2c, 0x00, REG_TX_RD_ADDR);
	meson_i2c_writel(i2c, 0x20, REG_TX_WR_ADDR);//fixed param for 32byte fifo
	/* Must reset rx_wr_addr before rx_rd_addr for continues read.
	 * Because reset rd will pull data to fifo, so we need reset wr early.
	 * And then, data will be push rightly to 1st fifo position.
	 */
	meson_i2c_writel(i2c, 0x00, REG_RX_WR_ADDR);
	meson_i2c_writel(i2c, 0x00, REG_RX_RD_ADDR);
}

static int meson_i2c_init(struct meson_i2c *i2c)
{
	int ret;

	ret = reset_control_reset(i2c->rst);
	if (ret) {
		dev_err(i2c->dev, "reset i2c error\n");
		return ret;
	}
	meson_i2c_set_mask(i2c, REG_CFG_BUS, BUS_FILTER_MASK, 1 << 12);
	meson_i2c_writel(i2c, 0, REG_CFG_START);
	meson_i2c_writel(i2c, 0xffff, REG_CGF_IRQ_STATE);//clear irq
	/* enable irq, critical trans done bit must be set.
	 * tx/rx thresh bit used for continuously transfer,
	 * set it manul when transfer.
	 * TX_FULL RX_EMPTY not used.
	 */
	meson_i2c_writel(i2c, TRANS_DONE, REG_CGF_IRQ_ENABLE);
	meson_i2c_set_mask(i2c, REG_CFG_BUS, BUS_FILTER_MASK, 1 << 12);
	meson_i2c_reset_fifo(i2c);
	// set rx thresh, when fifo has data will generate irq for cpu read
	// set tx thresh, when fifo empty will generate irq for continue transfer
	// meson_i2c_set_mask(i2c, REG_CFG_I2C, I2C_RX_THR | I2C_TX_THR, 1 << 16 | 0 << 8);
	i2c->fifo_depth = 32;
	meson_i2c_set_clk_div(i2c);

	return 0;
}

static void meson_i2c_push_data_to_user(struct meson_i2c *i2c, struct i2c_msg *msg, bool last)
{
	u8 *buf = i2c->msg->buf + i2c->pos;
	u32 *reg;
	u32 i, j, left, times, data = 0, trans_cnt;

	if (last)
		trans_cnt = msg->len - i2c->pos;
	else
		trans_cnt = i2c->fifo_depth;
	i2c->count = trans_cnt;
	times = (trans_cnt + 4 - 1) / 4;
	left = trans_cnt % 4;
#ifdef M_DEBUG
	pr_err("TX times :%d left:%d\n", times, left);
#endif
	for (i = 0; i < times; i++) {
		reg = (u32 *)((unsigned char *)i2c->regs + 0x80 + 32 + 4 * i);
		data = readl(reg);
		// pr_err("meson i2c: read data %08x from reg:0x%p\n", data, reg);
		if (left != 0 && i == (times - 1)) {
			for (j = 0; j < left; j++, buf++) {
				*buf = data >> (j * 8);
#ifdef M_DEBUG
				pr_err("RX buff: 0x%x\n", *buf);
				pr_err("RX reg: 0x%08x\n", data);
#endif
			}
		} else {
			for (j = 0; j < 4; j++, buf++) {
				*buf = data >> (j * 8);
#ifdef M_DEBUG
				pr_err("RX buff: 0x%x\n", *buf);
				pr_err("RX reg: 0x%08x\n", data);
#endif
			}
		}
	}
#ifdef M_DEBUG
	pr_err("meson i2c: %u has been transferred\n", i2c->count);
#endif

	meson_i2c_reset_fifo(i2c);
}

static int meson_i2c_put_data(struct meson_i2c *i2c, char *buf, int len)
{
	u8 i, j, times, left;
	u32 reg_val = 0;

	if (!buf) {//this i2c module when trans 0 byte, must put at least 1.
		pr_err("meson i2c t6w: can not start empty transfer!!!\n");
		// writel(0x00,  &i2c->regs->cfg_tx);
		return -1;
	}
	times = (len + 4 - 1) / 4;
	left = len % 4;
#ifdef M_DEBUG
	pr_err("times :%d left:%d\n", times, left);
#endif
	for (i = 0; i < times; i++) {
		reg_val = 0;
		if (left != 0 && i == (times - 1)) {
			for (j = 0; j < left; j++, buf++) {
				reg_val += *buf << (8 * j);
#ifdef M_DEBUG
				pr_err("buff: 0x%x\n", *buf);
				pr_err("reg: 0x%08x\n", reg_val);
#endif
			}
		} else {
			for (j = 0; j < 4; j++, buf++)
				reg_val += *buf << (8 * j);
		}
		meson_i2c_writel(i2c, reg_val, REG_FIFO_BASE + i * 4);
#ifdef M_DEBUG
		pr_err("meson i2c t6w: write data %08x reg 0x%p\n",
			reg_val, REG_FIFO_BASE + i * 4);
#endif
	}
	meson_i2c_reset_fifo(i2c);//should reset fifo for every transfer

	return 0;
}

static int meson_i2c_prepare_xfer(struct meson_i2c *i2c)
{
	bool write = !(i2c->msg->flags & I2C_M_RD);
	int ret = 0;

	if (write) {
		// must reset TX_RD_ADDR after put data for continuous TX
		// meson_i2c_writel(i2c, 0x00, REG_TX_RD_ADDR);
		i2c->count = min(i2c->msg->len - i2c->pos, i2c->fifo_depth);
		ret = meson_i2c_put_data(i2c, i2c->msg->buf + i2c->pos, i2c->count);
	}

	return ret;
}

#if FUTURE_USE
static irqreturn_t meson_i2c_irq(int irqno, void *dev_id)
{
	struct meson_i2c *i2c = dev_id;
	unsigned int irq_state, rd_val;

	i2c->count = 0;
	spin_lock(&i2c->lock);
	irq_state = readl(i2c->regs + REG_CGF_IRQ_STATE);
	if (irq_state & TRANS_ERROR) {
		dev_dbg(i2c->dev, "bus phy erro set\n");
		i2c->error = -ENXIO;
		goto trans_done;
	}
	switch (i2c->state) {
	case STATE_READ:
		if (irq_state & RX_FIFO_READ) {
			rd_val =  readl(i2c->regs + REG_CGF_RX);
			if (!(rd_val & TX_RX_EMPTY))
				meson_i2c_push_data_to_user(i2c, rd_val & TX_RX_DATA);
		}
		dev_dbg(i2c->dev, "for rx\n");
		break;
	case STATE_WRITE:
		dev_dbg(i2c->dev, "for tx\n");
		if (irq_state & TX_FIFO_WRITE)
			meson_i2c_prepare_xfer(i2c);
		break;
	case STATE_WAIT:
		dev_dbg(i2c->dev, "wait\n");
		if (irq_state & TRANS_DONE)
			goto trans_done;
		goto trans_out;
	case STATE_IDLE:
		dev_dbg(i2c->dev, "idle\n");
		goto trans_done;
	default:
			break;
	}
	i2c->pos += i2c->count;

	if (i2c->pos >= i2c->msg->len && i2c->state == STATE_READ) {
		goto trans_done;
	} else if (i2c->pos >= i2c->msg->len && i2c->state == STATE_WRITE) {
		if (irq_state & TRANS_DONE)
			goto trans_done;
		i2c->state = STATE_WAIT;//wait trans done
	}
	goto trans_out;

trans_done:
	dev_dbg(i2c->dev, "trans_done\n");
	meson_i2c_writel(i2c, TRANS_DONE, REG_CGF_IRQ_STATE);//clear done irq
	i2c->state = STATE_IDLE;
	complete(&i2c->done);
	reset_fifo_pos(i2c);
	meson_i2c_set_mask(i2c, REG_CGF_IRQ_ENABLE,
		RX_FIFO_READ | TX_FIFO_WRITE, 0);//disable thresh bit
trans_out:
	dev_dbg(i2c->dev, "IRQ out\n");
	meson_i2c_writel(i2c, RX_FIFO_READ | TX_FIFO_WRITE,
		REG_CGF_IRQ_STATE);//clear trans irq bit
	spin_unlock(&i2c->lock);

	return IRQ_HANDLED;
}
#endif

static int meson_i2c_do_start(struct meson_i2c *i2c, struct i2c_msg *msg)
{
	unsigned int reg_val;

	reg_val = (msg->addr << 1) |
				((msg->flags & I2C_M_RD) ? START_READ : 0) |
				((msg->len) << 12);
	meson_i2c_set_mask(i2c, REG_CFG_START,
		START_SLAVE_ADDR | START_READ | START_LEN, reg_val);

	return 0;
}

static int meson_i2c_xfer_msg(struct meson_i2c *i2c, struct i2c_msg *msg)
{
	int ret = 0;
	int time_count = 0;

	i2c->msg = msg;
	i2c->pos = 0;
	i2c->count = 0;
	i2c->error = 0;
	//fill trans len, slave addr and R/W cmd for this msg
	ret = meson_i2c_do_start(i2c, msg);
	if (ret)
		return ret;

	meson_i2c_reset_fifo(i2c);
	/* start the transfer */
	if (msg->flags & I2C_M_RD)//for rx
		meson_i2c_set_mask(i2c, REG_CFG_START, START_STRAT, START_STRAT);
	do {
		if (msg->flags & I2C_M_RD) {//for rx
			time_count = 0;
			if ((msg->len - i2c->pos) >= i2c->fifo_depth) {/*wait fifo full*/
				while (!(readl(i2c->regs + REG_CGF_RX) & TX_RX_FULL)) {
					usleep_range(SCHEDULE_TIMEOUT_US, SCHEDULE_TIMEOUT_US + 10);
					time_count++;
					if (time_count > I2C_TIMEOUT_MS) {/*time out*/
						meson_i2c_set_mask(i2c, REG_CFG_START,
								START_STRAT, 0);
						meson_i2c_reset_fifo(i2c);
						meson_i2c_writel(i2c, 0xffff, REG_CGF_IRQ_ENABLE);
						pr_err("meson i2c: rx timeout\n");
						return -ETIMEDOUT;
					}
				}
				meson_i2c_push_data_to_user(i2c, msg, 0);
			} else {
				break;
			}
		} else { //for tx
			time_count = 0;
			ret = meson_i2c_prepare_xfer(i2c);
			if (ret)
				return ret;
			if (!(readl(i2c->regs + REG_CFG_START) & START_STRAT))
				meson_i2c_set_mask(i2c, REG_CFG_START, START_STRAT, START_STRAT);
			if (i2c->count == i2c->fifo_depth) {/*wait fifo empty*/
				while (!(readl(i2c->regs + REG_CGF_TX) & TX_RX_EMPTY)) {
					usleep_range(SCHEDULE_TIMEOUT_US, SCHEDULE_TIMEOUT_US + 10);
					time_count++;
					if (time_count > I2C_TIMEOUT_MS) {/*time out*/
						meson_i2c_set_mask(i2c, REG_CFG_START,
							START_STRAT, 0);
						meson_i2c_reset_fifo(i2c);
						meson_i2c_writel(i2c, 0xffff, REG_CGF_IRQ_ENABLE);
						pr_err("meson i2c: tx timeout\n");
						return -ETIMEDOUT;
					}
				}
			}
		}
		if (readl(i2c->regs + REG_CGF_IRQ_STATE) & TRANS_ERROR) {
			meson_i2c_set_mask(i2c, REG_CFG_START, START_STRAT, 0);
			// pr_err("meson i2c: error 0x%x\n", readl(i2c->regs + REG_CGF_IRQ_STATE));
			meson_i2c_writel(i2c, 0xffff, REG_CGF_IRQ_ENABLE);
			meson_i2c_reset_fifo(i2c);
			return -EREMOTEIO;
		}
		i2c->pos += i2c->count;
	} while (i2c->pos < msg->len);

	time_count = 0;
	while (!(readl(i2c->regs + REG_CGF_IRQ_STATE) & TRANS_DONE)) {
		// pr_err("irq state reg: 0x%x\n", readl(i2c->regs + REG_CGF_IRQ_STATE));
		usleep_range(SCHEDULE_TIMEOUT_US, SCHEDULE_TIMEOUT_US + 10);
		time_count++;
		if (time_count > I2C_TIMEOUT_MS) {
			meson_i2c_set_mask(i2c, REG_CFG_START, START_STRAT, 0);
			meson_i2c_reset_fifo(i2c);
			meson_i2c_writel(i2c, 0xffff, REG_CGF_IRQ_ENABLE);//clear irq bit
			pr_err("meson i2c: trans timeout\n");

			return -ETIMEDOUT;
		}
	}
	if (msg->flags & I2C_M_RD)//for rx
		meson_i2c_push_data_to_user(i2c, msg, 1);
	meson_i2c_set_mask(i2c, REG_CFG_START, START_STRAT, 0);
	if (readl(i2c->regs + REG_CGF_IRQ_STATE) & TRANS_ERROR) {
		pr_debug("meson i2c: error 0x%x\n", readl(i2c->regs + REG_CGF_IRQ_STATE));
		meson_i2c_writel(i2c, 0xffff, REG_CGF_IRQ_ENABLE);;//clear irq bit
		meson_i2c_reset_fifo(i2c);
		return -EREMOTEIO;
	}
	meson_i2c_writel(i2c, 0xffff, REG_CGF_IRQ_ENABLE);;//clear irq bit
	meson_i2c_reset_fifo(i2c);

	return ret;
}

static int meson_i2c_xfer(struct i2c_adapter *adap, struct i2c_msg *msgs,
			  int num)
{
	struct meson_i2c *i2c = adap->algo_data;
	int i, ret = 0;
#if FUTURE_USE
	ret = pm_runtime_get_sync(i2c->dev);
	if (ret < 0)
		goto out;
#endif
	ret = meson_i2c_init(i2c);
	if (ret)
		return ret;
	for (i = 0; i < num; i++) {
		ret = meson_i2c_xfer_msg(i2c, msgs + i);
		if (ret)
			break;
	}
#if FUTURE_USE
out:
	pm_runtime_mark_last_busy(i2c->dev);
	pm_runtime_put_autosuspend(i2c->dev);
#endif

	return ret ?: i;
}

static u32 meson_i2c_func(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL;
}

static const struct i2c_algorithm meson_i2c_algorithm = {
	.master_xfer	= meson_i2c_xfer,
	.functionality	= meson_i2c_func,
};

static ssize_t speed_show(struct device *child,
			  struct device_attribute *attr, char *buf)
{
	struct meson_i2c *i2c =
		(struct meson_i2c *)dev_get_drvdata(child);
	return sprintf(buf, "%d\n", i2c->frequency);
}

static ssize_t speed_store(struct device *child,
			   struct device_attribute *attr,
			   const char *buf, size_t size)
{
	struct meson_i2c *i2c =
		(struct meson_i2c *)dev_get_drvdata(child);
	int ret, val;

	ret = kstrtouint(buf, 0, &val);
	if (ret)
		return ret;

	i2c->frequency = val;
	meson_i2c_set_clk_div(i2c);

	return size;
}

static DEVICE_ATTR_RW(speed);

static int meson_i2c_speed_debug(struct device *dev)
{
	return sysfs_create_file(&dev->kobj, &dev_attr_speed.attr);
}

static int meson_i2c_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct meson_i2c *i2c;
	struct i2c_timings timings;
	int ret = 0;

	i2c = devm_kzalloc(&pdev->dev, sizeof(struct meson_i2c), GFP_KERNEL);
	if (!i2c)
		return -ENOMEM;

	i2c_parse_fw_timings(&pdev->dev, &timings, true);
	i2c->frequency = timings.bus_freq_hz;
	i2c->data = (struct meson_i2c_data *)of_device_get_match_data(&pdev->dev);
	i2c->dev = &pdev->dev;
	i2c->rst = devm_reset_control_get(&pdev->dev, "i2c_reset");
	if (IS_ERR(i2c->rst)) {
		dev_err(&pdev->dev, "can't get device reset control\n");
		return PTR_ERR(i2c->rst);
	}
	platform_set_drvdata(pdev, i2c);

	spin_lock_init(&i2c->lock);
	init_completion(&i2c->done);

	i2c->clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(i2c->clk)) {
		dev_err(&pdev->dev, "can't get device clock\n");
		return PTR_ERR(i2c->clk);
	}
	i2c->regs  = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(i2c->regs))
		return PTR_ERR(i2c->regs);
	if (i2c->data->tee) {
		/* get i2c tee_id property */
		ret = of_property_read_u32(pdev->dev.of_node, "tee_id",
				   &i2c->data->tee_id);
		if (ret) {
			dev_err(&pdev->dev, "not config tee_id\n");
			return ret;
		}
	}
#if FUTURE_USE
	ret = devm_request_irq(&pdev->dev, irq, meson_i2c_irq, 0, NULL, i2c);
	if (ret < 0) {
		dev_err(&pdev->dev, "can't request IRQ\n");
		return ret;
	}
#endif
	strscpy(i2c->adap.name, "Meson I2C V2 adapter",
		sizeof(i2c->adap.name));
	i2c->adap.owner = THIS_MODULE;
	i2c->adap.algo = &meson_i2c_algorithm;
	i2c->adap.dev.parent = &pdev->dev;
	i2c->adap.dev.of_node = np;
	i2c->adap.algo_data = i2c;

	//i2c->irq = irq;
	/* Disable the interrupt so that the system can enter low-power mode */
#if FUTURE_USE
	disable_irq(i2c->irq);
	pm_runtime_enable(i2c->dev);
	pm_runtime_set_autosuspend_delay(i2c->dev, MESON_I2C_PM_TIMEOUT);
	pm_runtime_use_autosuspend(i2c->dev);

	ret = pm_runtime_get_sync(i2c->dev);
	if (ret < 0) {
		pm_runtime_put_noidle(i2c->dev);
		pm_runtime_disable(i2c->dev);
		return ret;
	}
#endif
	/* speed sysfs */
	ret = meson_i2c_speed_debug(&pdev->dev);
	if (ret)
		dev_err(&pdev->dev, "Creat i2c speed debug sysfs failed\n");
	i2c->clk_rate = clk_get_rate(i2c->clk);
	if (i2c->clk_rate == 0) {
		dev_err(&pdev->dev, "failed to get clk rate\n");
		return -EINVAL;
	}

#if FUTURE_USE
	pm_runtime_mark_last_busy(i2c->dev);
	pm_runtime_put_autosuspend(i2c->dev);
#endif
	ret = i2c_add_adapter(&i2c->adap);
	if (ret < 0)
		return ret;

	return 0;
}

static void meson_i2c_remove(struct platform_device *pdev)
{
	struct meson_i2c *i2c = platform_get_drvdata(pdev);
#if FUTURE_USE
	int ret;

	ret = pm_runtime_get_sync(&pdev->dev);
	if (ret < 0)
		return ret;
#endif

	i2c_del_adapter(&i2c->adap);
#if FUTURE_USE
	pm_runtime_dont_use_autosuspend(&pdev->dev);
	pm_runtime_put_sync(&pdev->dev);
	pm_runtime_disable(&pdev->dev);
#endif
}

#if FUTURE_USE
static int meson_i2c_put(struct meson_i2c *i2c)
{
	disable_irq(i2c->irq);
	pinctrl_pm_select_sleep_state(i2c->dev);
	clk_disable_unprepare(i2c->clk);

	return 0;
}

static int meson_i2c_regain(struct meson_i2c *i2c)
{
	int ret;

	ret = clk_prepare_enable(i2c->clk);
	if (ret < 0) {
		dev_err(i2c->dev, "can not enable clock\n");
		return ret;
	}
	pinctrl_pm_select_default_state(i2c->dev);
	enable_irq(i2c->irq);

	return 0;
}

#ifdef CONFIG_HIBERNATION
static int meson_i2c_freeze(struct device *dev)
{
	int ret;
	struct meson_i2c *i2c = (struct meson_i2c *)dev_get_drvdata(dev);

	if (i2c->is_runtime_sleep)
		return 0;
	ret = meson_i2c_put(i2c);

	return ret;
}

static int meson_i2c_thaw(struct device *dev)
{
	return 0;
}

static int meson_i2c_restore(struct device *dev)
{
	int ret;
	struct meson_i2c *i2c = (struct meson_i2c *)dev_get_drvdata(dev);

	if (i2c->is_runtime_sleep)
		return 0;
	ret = meson_i2c_regain(i2c);
	meson_i2c_set_mask(i2c, REG_CTRL, REG_CTRL_START, 0);
	meson_i2c_set_clk_div(i2c);

	return ret;
}
#endif

#ifdef CONFIG_AMLOGIC_MODIFY
static int __maybe_unused meson_i2c_runtime_suspend(struct device *dev)
{
	int ret;
	struct meson_i2c *i2c = (struct meson_i2c *)dev_get_drvdata(dev);

	ret = meson_i2c_put(i2c);
	i2c->is_runtime_sleep = true;

	return ret;
}

static int __maybe_unused meson_i2c_runtime_resume(struct device *dev)
{
	int ret;
	struct meson_i2c *i2c = (struct meson_i2c *)dev_get_drvdata(dev);

	ret = meson_i2c_regain(i2c);
	i2c->is_runtime_sleep = false;

	return ret;
}

static const struct dev_pm_ops meson_i2c_pm_ops = {
	SET_RUNTIME_PM_OPS(meson_i2c_runtime_suspend,
			   meson_i2c_runtime_resume, NULL)
#ifdef CONFIG_HIBERNATION
	.freeze		= meson_i2c_freeze,
	.thaw		= meson_i2c_thaw,
	.restore	= meson_i2c_restore,
#endif
};
#endif
#endif //FUTURE_USE

static struct meson_i2c_data i2c_normal_data = {
	.tee = false,
};

static const struct of_device_id meson_i2c_t6w_match[] = {
	{	.compatible = "amlogic,meson-i2c-t6w",
		.data = &i2c_normal_data
	},
	{},
};

MODULE_DEVICE_TABLE(of, meson_i2c_t6w_match);

static struct platform_driver meson_i2c_driver = {
	.probe   = meson_i2c_probe,
	.remove  = meson_i2c_remove,
	.driver  = {
		.name  = "meson-i2c-t6w",
#if FUTURE_USE
		.pm	= &meson_i2c_pm_ops,
#endif
		.of_match_table = meson_i2c_t6w_match,
	},
};

int __init i2c_meson_t6w_init(void)
{
	return platform_driver_register(&meson_i2c_driver);
}

void __exit i2c_meson_t6w_exit(void)
{
	platform_driver_unregister(&meson_i2c_driver);
}

MODULE_DESCRIPTION("Amlogic Meson I2C T6W TCON Bus driver");
MODULE_AUTHOR("Junyi Zhao <junyi.zhao@amlogic.com>");
MODULE_LICENSE("GPL v2");
