/*
 * PRU driver for TI's AM33xx series of SoCs
 *
 * Copyright (C) 2013 Pantelis Antoniou <panto@antoniou-consulting.com>
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2.  This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */

#include <asm/atomic.h>
#include <asm/uaccess.h>

#include <linux/module.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/poll.h>

#include <linux/platform_device.h>
#include <linux/miscdevice.h>

#include <linux/io.h>
#include <linux/irqreturn.h>
#include <linux/slab.h>
#include <linux/genalloc.h>
#include <linux/mm.h>
#include <linux/dma-mapping.h>

#include <linux/kobject.h>
#include <linux/string.h>

#include <linux/sysfs.h>
#include <linux/fs.h>

#include "external_glue.h"

#define SHMDRAM_BASE 0x4a312000

#define NUM_VALUES 10



struct circular_buffer
{
	volatile int head;
	volatile int tail;
	char buffer[NUM_VALUES];
};

volatile struct circular_buffer *ring;

#define size sizeof(*ring)


struct pru_bridge_dev {
	/* Misc device descriptor */
	struct miscdevice miscdev;

	/* Imported members */
	int (*downcall_idx)(int, u32, u32, u32, u32, u32, u32);

	/* Data */
	struct device *p_dev; /* parent platform device */
};

void write_buffer(char data)
{
	printk("Ring:%p  Buffer value1 : %c\n Tail : %d \n",ring,data,ring->tail);
    ring->buffer[ring->tail] = data;
    printk("Stored :%c\n",ring->buffer[ring->tail]);
    ring->tail = (ring->tail+1)%NUM_VALUES;
}

char read_buffer(void)
{
    char value = ring->buffer[ring->head];
    ring->head = (ring->head+1)%NUM_VALUES;
    return value;
}

static const struct file_operations pru_bridge_fops;


static ssize_t pru_bridge_ch1_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int i=0;
	while(buf[i] != '\0')                                   //right now one extra iteration containing blank spaces that i will deal with in a bit
	{
		printk("Buffer value : %c \n",buf[i]);
		write_buffer(buf[i]);
		i++;
	}
        printk("Write complete\n");

	return strlen(buf);
}


static ssize_t pru_bridge_ch1_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE,"%c\n",read_buffer());			//have to decide if i will return whole buffer right now only one character
}

static DEVICE_ATTR(ch1_write,S_IWUSR|S_IRUGO,NULL,pru_bridge_ch1_write);
static DEVICE_ATTR(ch1_read, S_IWUSR|S_IRUGO, pru_bridge_ch1_read, NULL);


static int pru_bridge_probe(struct platform_device *pdev)
{
	struct pru_bridge_dev *pp;
	struct pru_rproc_external_glue g;
	struct device *dev;
	int err;

	/* Allocate memory for our private structure */
	pp = kzalloc(sizeof(*pp), GFP_KERNEL);
	if (!pp)
		goto fail;

	pp->miscdev.fops = &pru_bridge_fops;
	pp->miscdev.minor = MISC_DYNAMIC_MINOR;
	pp->miscdev.mode = S_IRUGO;
	pp->miscdev.name = "pru_bridge";

	/* Link the platform device data to our private structure */
	pp->p_dev = &pdev->dev;
	dev_set_drvdata(pp->p_dev, pp);

	/* Bind to the pru_rproc module */
	err = pruproc_external_request_bind(&g);
	if (err)
		goto fail;
	pp->downcall_idx = g.downcall_idx;


	err = misc_register(&pp->miscdev);
	if (err)
		goto fail;
	dev = pp->miscdev.this_device;
	dev_set_drvdata(dev, pp);

    /*mapping shared memory and initialising circular buffer*/
	printk("Initialising shared memory of size :%d\n",size);
	ring = (volatile struct circular_buffer*)ioremap(SHMDRAM_BASE,size);
    ring->head = 0;
	ring->tail = 0;
	printk("Memory allocated at : %p head:%d tail:%d\n",ring,ring->head,ring->tail);


	printk("Creating sysfs entries\n");

	err = device_create_file(dev, &dev_attr_ch1_write);
	if (err != 0){
		dev_err(dev, "device_create_file failed\n");
		goto err_fail;
	}

	err = device_create_file(dev, &dev_attr_ch1_read);
        if (err != 0){
                dev_err(dev, "device_create_file failed\n");
                goto err_fail;
        }


	dev_info(dev, "Loaded OK\n");

	printk("Probe successful");

err_fail:
	return err;
fail:
	return -1;
}

static int pru_bridge_remove(struct platform_device *pdev)
{
	struct pru_bridge_dev *pp = platform_get_drvdata(pdev);
	struct device *dev = pp->miscdev.this_device;

	/*Deallocating memory*/
	printk("deallocating memory\n");
	 iounmap(ring);

	device_remove_file(dev, &dev_attr_ch1_write);
	device_remove_file(dev, &dev_attr_ch1_read);


	platform_set_drvdata(pdev, NULL);


	printk("PRU bridge Driver unloaded\n");
	return 0;
}

static const struct of_device_id pru_bridge_dt_ids[] = {
	{ .compatible = "ti,pru_bridge", .data = NULL, },
	{},
};


static struct platform_driver pru_bridge_driver = {
	.driver	= {
		.name	= "pru_bridge",
		.owner	= THIS_MODULE,
		.of_match_table = pru_bridge_dt_ids,
	},
	.probe	= pru_bridge_probe,
	.remove	= pru_bridge_remove,
};


static int __init pru_bridge_init(void)
{
	printk(KERN_INFO "pru_bridge loaded\n");
	platform_driver_register(&pru_bridge_driver);
	return 0;
}

static void __exit pru_bridge_exit(void)
{
	printk(KERN_INFO "pru_bridge unloaded\n");
	platform_driver_unregister(&pru_bridge_driver);
}

module_init(pru_bridge_init);
module_exit(pru_bridge_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("PRU-Bridge");
MODULE_AUTHOR("Apaar Gupta");
