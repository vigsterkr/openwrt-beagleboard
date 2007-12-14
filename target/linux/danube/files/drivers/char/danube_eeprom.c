/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *   This driver was originally based on the INCA-IP driver, but due to
 *   fundamental conceptual drawbacks there has been changed a lot.
 *
 *   Based on INCA-IP driver Copyright (c) 2003 Gary Jennejohn <gj@denx.de>
 *   Based on the VxWorks drivers Copyright (c) 2002, Infineon Technologies.
 *
 *   Copyright (C) 2006 infineon
 *   Copyright (C) 2007 John Crispin <blogic@openwrt.org> 
 *
 */

#define IFAP_EEPROM_DRV_VERSION "0.0.1"

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/major.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/ptrace.h>
#include <linux/mm.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <asm/system.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/uaccess.h>
#include <asm/bitops.h>

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/version.h>

#include <asm/danube/danube.h>
#include <asm/danube/danube_irq.h>
#include <asm/danube/ifx_ssc_defines.h>
#include <asm/danube/ifx_ssc.h>

/* allow the user to set the major device number */
static int danube_eeprom_maj = 0;

static ssize_t danube_eeprom_fops_read (struct file *, char *, size_t, loff_t *);
static ssize_t danube_eeprom_fops_write (struct file *, const char *, size_t,
				    loff_t *);
static int danube_eeprom_ioctl (struct inode *, struct file *, unsigned int,
				unsigned long);
static int danube_eeprom_open (struct inode *, struct file *);
static int danube_eeprom_close (struct inode *, struct file *);

//ifx_ssc.c
extern int ifx_ssc_init (void);
extern int ifx_ssc_open (struct inode *inode, struct file *filp);
extern int ifx_ssc_close (struct inode *inode, struct file *filp);
extern void ifx_ssc_cleanup_module (void);
extern int ifx_ssc_ioctl (struct inode *inode, struct file *filp,
			  unsigned int cmd, unsigned long data);
extern ssize_t ifx_ssc_kwrite (int port, const char *kbuf, size_t len);
extern ssize_t ifx_ssc_kread (int port, char *kbuf, size_t len);

extern int ifx_ssc_cs_low (unsigned int pin);
extern int ifx_ssc_cs_high (unsigned int pin);
extern int ifx_ssc_txrx (char *tx_buf, unsigned int tx_len, char *rx_buf, unsigned int rx_len);
extern int ifx_ssc_tx (char *tx_buf, unsigned int tx_len);
extern int ifx_ssc_rx (char *rx_buf, unsigned int rx_len);

#define EEPROM_CS IFX_SSC_WHBGPOSTAT_OUT0_POS

/* commands for EEPROM, x25160, x25140 */
#define EEPROM_WREN			((unsigned char)0x06)
#define EEPROM_WRDI			((unsigned char)0x04)
#define EEPROM_RDSR			((unsigned char)0x05)
#define EEPROM_WRSR			((unsigned char)0x01)
#define EEPROM_READ			((unsigned char)0x03)
#define EEPROM_WRITE			((unsigned char)0x02)
#define EEPROM_PAGE_SIZE		4
#define EEPROM_SIZE			512

static int
eeprom_rdsr (char *status)
{
	int ret = 0;
	unsigned char cmd = EEPROM_RDSR;
	unsigned long flag;

	local_irq_save(flag);

	if ((ret = ifx_ssc_cs_low (EEPROM_CS)))
	{
		local_irq_restore(flag);
		goto out;
	}

	if ((ret = ifx_ssc_txrx (&cmd, 1, status, 1)) < 0)
	{
		ifx_ssc_cs_high(EEPROM_CS);
		local_irq_restore(flag);
		goto out;
	}

	if ((ret = ifx_ssc_cs_high(EEPROM_CS)))
	{
		local_irq_restore(flag);
		goto out;
	}

	local_irq_restore(flag);

out:
	return ret;
}

static inline int
eeprom_wip_over (void)
{
	int ret = 0;
	unsigned char status;

	while (1)
	{
		ret = eeprom_rdsr(&status);
		printk("status %x \n", status);

		if (ret)
		{
			printk("read back status fails %d\n", ret);
			break;
		}

		if (((status) & 1) != 0)
			printk("read back status not zero %x\n", status);
		else
			break;
	}

	return ret;
}

static int
eeprom_wren (void)
{
	unsigned char cmd = EEPROM_WREN;
	int ret = 0;
	unsigned long flag;

	local_irq_save(flag);
	if ((ret = ifx_ssc_cs_low(EEPROM_CS)))
	{
		local_irq_restore(flag);
		goto out;
	}

	if ((ret = ifx_ssc_tx(&cmd, 1)) < 0)
	{
		ifx_ssc_cs_high(EEPROM_CS);
		local_irq_restore(flag);
		goto out;
	}

	if ((ret = ifx_ssc_cs_high(EEPROM_CS)))
	{
		local_irq_restore(flag);
		goto out;
	}

	local_irq_restore(flag);
	eeprom_wip_over();

out:
	return ret;
}

static int
eeprom_wrsr (void)
{
	int ret = 0;
	unsigned char cmd[2];
	unsigned long flag;

	cmd[0] = EEPROM_WRSR;
	cmd[1] = 0;

	if ((ret = eeprom_wren()))
	{
		printk ("eeprom_wren fails\n");
		goto out1;
	}

	local_irq_save(flag);

	if ((ret = ifx_ssc_cs_low(EEPROM_CS)))
		goto out;

	if ((ret = ifx_ssc_tx(cmd, 2)) < 0) {
		ifx_ssc_cs_high(EEPROM_CS);
		goto out;
	}

	if ((ret = ifx_ssc_cs_low(EEPROM_CS)))
		goto out;

	local_irq_restore(flag);
	eeprom_wip_over();

	return ret;

out:
	local_irq_restore (flag);
	eeprom_wip_over ();

out1:
	return ret;
}

static int
eeprom_read (unsigned int addr, unsigned char *buf, unsigned int len)
{
	int ret = 0;
	unsigned char write_buf[2];
	unsigned int eff = 0;
	unsigned long flag;

	while (1)
	{
		eeprom_wip_over();
		eff = EEPROM_PAGE_SIZE - (addr % EEPROM_PAGE_SIZE);
		eff = (eff < len) ? eff : len;
		local_irq_save(flag);

		if ((ret = ifx_ssc_cs_low(EEPROM_CS)) < 0)
			goto out;

		write_buf[0] = EEPROM_READ | ((unsigned char) ((addr & 0x100) >> 5));
		write_buf[1] = (addr & 0xff);

		if ((ret = ifx_ssc_txrx (write_buf, 2, buf, eff)) != eff)
		{
			printk("ssc_txrx fails %d\n", ret);
			ifx_ssc_cs_high (EEPROM_CS);
			goto out;
		}

		buf += ret;
		len -= ret;
		addr += ret;

		if ((ret = ifx_ssc_cs_high(EEPROM_CS)))
			goto out;

		local_irq_restore(flag);

		if (len <= 0)
			goto out2;
	}

out:
	local_irq_restore (flag);
out2:
	return ret;
}

static int
eeprom_write (unsigned int addr, unsigned char *buf, unsigned int len)
{
	int ret = 0;
	unsigned int eff = 0;
	unsigned char write_buf[2];
	int i;
	unsigned char rx_buf[EEPROM_PAGE_SIZE];

	while (1)
	{
		eeprom_wip_over();

		if ((ret = eeprom_wren()))
		{
			printk("eeprom_wren fails\n");
			goto out;
		}

		write_buf[0] = EEPROM_WRITE | ((unsigned char) ((addr & 0x100) >> 5));
		write_buf[1] = (addr & 0xff);

		eff = EEPROM_PAGE_SIZE - (addr % EEPROM_PAGE_SIZE);
		eff = (eff < len) ? eff : len;

		printk("EEPROM Write:\n");
		for (i = 0; i < eff; i++) {
			printk("%2x ", buf[i]);
			if ((i % 16) == 15)
				printk("\n");
		}
		printk("\n");

		if ((ret = ifx_ssc_cs_low(EEPROM_CS)))
			goto out;

		if ((ret = ifx_ssc_tx (write_buf, 2)) < 0)
		{
			printk("ssc_tx fails %d\n", ret);
			ifx_ssc_cs_high(EEPROM_CS);
			goto out;
		}

		if ((ret = ifx_ssc_tx (buf, eff)) != eff)
		{
			printk("ssc_tx fails %d\n", ret);
			ifx_ssc_cs_high(EEPROM_CS);
			goto out;
		}

		buf += ret;
		len -= ret;
		addr += ret;

		if ((ret = ifx_ssc_cs_high (EEPROM_CS)))
			goto out;

		printk ("<==");
		eeprom_read((addr - eff), rx_buf, eff);
		for (i = 0; i < eff; i++)
		{
			printk ("[%x]", rx_buf[i]);
		}
		printk ("\n");

		if (len <= 0)
			break;
	}

out:
	return ret;
}

int
danube_eeprom_open (struct inode *inode, struct file *filp)
{
	filp->f_pos = 0;
	return 0;
}

int
danube_eeprom_close (struct inode *inode, struct file *filp)
{
	return 0;
}

int
danube_eeprom_ioctl (struct inode *inode, struct file *filp, unsigned int cmd, unsigned long data)
{
	return 0;
}

ssize_t
danube_eeprom_read (char *buf, size_t len, unsigned int addr)
{
	int ret = 0;
	unsigned int data;

	printk("addr:=%d\n", addr);
	printk("len:=%d\n", len);

	if ((addr + len) > EEPROM_SIZE)
	{
		printk("invalid len\n");
		addr = 0;
		len = EEPROM_SIZE / 2;
	}

	if ((ret = ifx_ssc_open ((struct inode *) 0, NULL)))
	{
		printk("danube_eeprom_open fails\n");
		goto out;
	}

	data = (unsigned int)IFX_SSC_MODE_RXTX;

	if ((ret = ifx_ssc_ioctl ((struct inode *) 0, NULL, IFX_SSC_RXTX_MODE_SET, (unsigned long) &data)))
	{
		printk("set RXTX mode fails\n");
		goto out;
	}

	if ((ret = eeprom_wrsr ()))
	{
		printk("EEPROM reset fails\n");
		goto out;
	}

	if ((ret = eeprom_read (addr, buf, len)))
	{
		printk("eeprom read fails\n");
		goto out;
	}

out:
	if (ifx_ssc_close ((struct inode *) 0, NULL))
		printk("danube_eeprom_close fails\n");

	return len;
}
EXPORT_SYMBOL(danube_eeprom_read);

static ssize_t
danube_eeprom_fops_read (struct file *filp, char *ubuf, size_t len, loff_t * off)
{
	int ret = 0;
	unsigned char ssc_rx_buf[EEPROM_SIZE];
	long flag;

	if (*off >= EEPROM_SIZE)
		return 0;

	if (*off + len > EEPROM_SIZE)
		len = EEPROM_SIZE - *off;

	if (len == 0)
		return 0;

	local_irq_save(flag);

	if ((ret = danube_eeprom_read(ssc_rx_buf, len, *off)) < 0)
	{
		printk("read fails, err=%x\n", ret);
		local_irq_restore(flag);
		return ret;
	}

	if (copy_to_user((void*)ubuf, ssc_rx_buf, ret) != 0)
	{
		local_irq_restore(flag);
		ret = -EFAULT;
	}

	local_irq_restore(flag);
	*off += len;

	return len;
}

ssize_t
danube_eeprom_write (char *buf, size_t len, unsigned int addr)
{
	int ret = 0;
	unsigned int data;

	if ((ret = ifx_ssc_open ((struct inode *) 0, NULL)))
	{
		printk ("danube_eeprom_open fails\n");
		goto out;
	}

	data = (unsigned int) IFX_SSC_MODE_RXTX;

	if ((ret = ifx_ssc_ioctl ((struct inode *) 0, NULL, IFX_SSC_RXTX_MODE_SET, (unsigned long) &data)))
	{
		printk ("set RXTX mode fails\n");
		goto out;
	}

	if ((ret = eeprom_wrsr ())) {
		printk ("EEPROM reset fails\n");
		goto out;
	}

	if ((ret = eeprom_write (addr, buf, len))) {
		printk ("eeprom write fails\n");
		goto out;
	}

out:
	if (ifx_ssc_close ((struct inode *) 0, NULL))
		printk ("danube_eeprom_close fails\n");

	return ret;
}
EXPORT_SYMBOL(danube_eeprom_write);

static ssize_t
danube_eeprom_fops_write (struct file *filp, const char *ubuf, size_t len, loff_t * off)
{
	int ret = 0;
	unsigned char ssc_tx_buf[EEPROM_SIZE];

	if (*off >= EEPROM_SIZE)
		return 0;

	if (len + *off > EEPROM_SIZE)
		len = EEPROM_SIZE - *off;

	if ((ret = copy_from_user (ssc_tx_buf, ubuf, len)))
		return EFAULT;

	ret = danube_eeprom_write (ssc_tx_buf, len, *off);

	if (ret > 0)
		*off = ret;

	return ret;
}

loff_t
danube_eeprom_llseek (struct file * filp, loff_t off, int whence)
{
	loff_t newpos;
	switch (whence) {
	case SEEK_SET:
		newpos = off;
		break;

	case SEEK_CUR:
		newpos = filp->f_pos + off;
		break;

	default:
		return -EINVAL;
	}

	if (newpos < 0)
		return -EINVAL;

	filp->f_pos = newpos;

	return newpos;
}

static struct file_operations danube_eeprom_fops = {
      owner:THIS_MODULE,
      llseek:danube_eeprom_llseek,
      read:danube_eeprom_fops_read,
      write:danube_eeprom_fops_write,
      ioctl:danube_eeprom_ioctl,
      open:danube_eeprom_open,
      release:danube_eeprom_close,
};

int __init
danube_eeprom_init (void)
{
	int ret = 0;

	danube_eeprom_maj = register_chrdev(0, "eeprom", &danube_eeprom_fops);

	if (danube_eeprom_maj < 0)
	{
		printk("failed to register eeprom device\n");
		ret = -EINVAL;
		
		goto out;
	}

	printk("danube_eeprom : /dev/eeprom mayor %d\n", danube_eeprom_maj);

out:
	return ret;
}

void __exit
danube_eeprom_cleanup_module (void)
{
	/*if (unregister_chrdev (danube_eeprom_maj, "eeprom")) {
		printk ("Unable to unregister major %d for the EEPROM\n",
			maj);
	}*/
}

module_exit (danube_eeprom_cleanup_module);
module_init (danube_eeprom_init);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Peng Liu");
MODULE_DESCRIPTION ("IFAP EEPROM driver");
MODULE_SUPPORTED_DEVICE ("danube_eeprom");

