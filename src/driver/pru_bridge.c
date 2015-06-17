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

#include <linux/ctype.h>
#include "external_glue.h"

#define SHMDRAM_BASE 0x4a312000

#define NUM_CHANNELS 5


/*
Defining control channle where
pru_data - holds number of values o be read by the pru
driver_data - number of values to be read by the driver
*/
struct control_channel
{
	volatile uint16_t init_check;
	volatile uint16_t channel_size[NUM_CHANNELS];
	volatile uint16_t pru_data[NUM_CHANNELS];
	volatile uint16_t driver_data[NUM_CHANNELS];
}size_control;

volatile struct control_channel* control_channel;

#define CONTROL_SIZE sizeof(size_control)

struct circular_buffer
{
	volatile uint16_t head;
	volatile uint16_t tail;
	volatile uint8_t* buffer;
}size_ring;

volatile struct circular_buffer* ring[NUM_CHANNELS];

#define CIRCULAR_BUFFER_SIZE sizeof(size_ring)


struct pru_bridge_dev {
	/* Misc device descriptor */
	struct miscdevice miscdev;

	/* Imported members */
	int (*downcall_idx)(int, u32, u32, u32, u32, u32, u32);

	/* Data */
	struct device *p_dev; /* parent platform device */
};

void write_buffer(int ring_no,char data)
{
	printk("Ring:%p  Buffer value : %c\n Tail : %d \n",ring[ring_no],data,ring[ring_no]->tail);
    *(ring[ring_no]->buffer + ring[ring_no]->tail) = (uint8_t)data;
    printk("Stored :%c\n",*(ring[ring_no]->buffer + ring[ring_no]->tail));
    ring[ring_no]->tail = (ring[ring_no]->tail+1)%(control_channel->channel_size[ring_no]);
}

void write_buffer_wrapper(int ring_no,const char* buf)
{
    int i=0;
	printk("%s/n",buf);
	while(buf[i] != '\n')
	{
		printk("Buffer value : %c \n",buf[i]);
		write_buffer(ring_no,buf[i]);
		i++;
	}
        printk("Write complete\n");
}

char read_buffer(int ring_no)
{
    uint8_t value = *(ring[ring_no]->buffer + ring[ring_no]->head);
    ring[ring_no]->head = (ring[ring_no]->head+1)%(control_channel->channel_size[ring_no]);
    return (char)value;
}

/*function to initialise all circular buffers and assign ring values*/
void init_circular_buffer(void)
{
   int last_address,i=0;
    last_address = SHMDRAM_BASE+CONTROL_SIZE;
    printk("Base addr : %x Circular Buffer Size : %d Control size : %d\n",SHMDRAM_BASE,CIRCULAR_BUFFER_SIZE,CONTROL_SIZE);
    while(i<NUM_CHANNELS)
    {
        printk("Last Addr : %x\n",last_address);
        ring[i] = (volatile struct circular_buffer*)ioremap(last_address,CIRCULAR_BUFFER_SIZE);
        ring[i]->head = 0;
        ring[i]->tail = 0;
	printk("Ring : %p\n",ring[i]);
        ring[i]->buffer = (volatile uint8_t*)ioremap(last_address+CIRCULAR_BUFFER_SIZE,(sizeof(uint8_t)*(control_channel->channel_size[i])));
        printk("Ring buffer : %p Size : %d\n",ring[i]->buffer,(sizeof(uint8_t)*(control_channel->channel_size[i])));
        last_address = last_address + CIRCULAR_BUFFER_SIZE +(int)(sizeof(uint8_t)*control_channel->channel_size[i]);
        i++;
    }
    printk("Channels successfully initialised\n");
}

static const struct file_operations pru_bridge_fops;

static ssize_t pru_bridge_init_channels(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int i=0;
	const char * p = buf;
	printk("%s\n",buf);
    while (*p)
    {
        if (isdigit(*p))
        {
            control_channel->channel_size[i] = (uint16_t) simple_strtoul(p,&p,10);
            printk("Channel number:%d Size:%d\n",i+1,control_channel->channel_size[i]);
            i++;
        }
        else
        {
            p++;
        }
    }
    printk("Sizes set now initialising the channels\n");
    init_circular_buffer();
    control_channel->init_check = 1;
    return strlen(buf);
}


static ssize_t pru_bridge_ch1_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	write_buffer_wrapper(0,buf);
	return strlen(buf);
}


static ssize_t pru_bridge_ch1_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE,"%c\n",read_buffer(0));
}

static ssize_t pru_bridge_ch2_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	write_buffer_wrapper(1,buf);
	return strlen(buf);
}


static ssize_t pru_bridge_ch2_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE,"%c\n",read_buffer(1));
}

static ssize_t pru_bridge_ch3_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	write_buffer_wrapper(2,buf);
	return strlen(buf);
}


static ssize_t pru_bridge_ch3_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE,"%c\n",read_buffer(2));
}

static ssize_t pru_bridge_ch4_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	write_buffer_wrapper(3,buf);
	return strlen(buf);
}


static ssize_t pru_bridge_ch4_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE,"%c\n",read_buffer(3));
}

static ssize_t pru_bridge_ch5_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	write_buffer_wrapper(4,buf);
	return strlen(buf);
}


static ssize_t pru_bridge_ch5_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE,"%c\n",read_buffer(4));
}



static DEVICE_ATTR(init, S_IWUSR|S_IRUGO,NULL,pru_bridge_init_channels);
static DEVICE_ATTR(ch1_write,S_IWUSR|S_IRUGO,NULL,pru_bridge_ch1_write);
static DEVICE_ATTR(ch1_read, S_IWUSR|S_IRUGO, pru_bridge_ch1_read, NULL);
static DEVICE_ATTR(ch2_write,S_IWUSR|S_IRUGO,NULL,pru_bridge_ch2_write);
static DEVICE_ATTR(ch2_read, S_IWUSR|S_IRUGO, pru_bridge_ch2_read, NULL);
static DEVICE_ATTR(ch3_write,S_IWUSR|S_IRUGO,NULL,pru_bridge_ch3_write);
static DEVICE_ATTR(ch3_read, S_IWUSR|S_IRUGO, pru_bridge_ch3_read, NULL);
static DEVICE_ATTR(ch4_write,S_IWUSR|S_IRUGO,NULL,pru_bridge_ch4_write);
static DEVICE_ATTR(ch4_read, S_IWUSR|S_IRUGO, pru_bridge_ch4_read, NULL);
static DEVICE_ATTR(ch5_write,S_IWUSR|S_IRUGO,NULL,pru_bridge_ch5_write);
static DEVICE_ATTR(ch5_read, S_IWUSR|S_IRUGO, pru_bridge_ch5_read, NULL);

static int pru_bridge_probe(struct platform_device *pdev)
{
	struct pru_bridge_dev *pp;
	struct pru_rproc_external_glue g;
	struct device *dev;
	int err;

	 /*mapping shared memory for control channel*/
	control_channel = (volatile struct control_channel*)ioremap(SHMDRAM_BASE,CIRCULAR_BUFFER_SIZE);
        control_channel->init_check = 0;
	printk("Memory allocated for control channel\n");


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


	printk("Creating sysfs entries\n");

	err = device_create_file(dev, &dev_attr_init);
	if (err != 0){
		dev_err(dev, "device_create_file failed\n");
		goto err_fail;
	}

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

    err = device_create_file(dev, &dev_attr_ch2_write);
	if (err != 0){
		dev_err(dev, "device_create_file failed\n");
		goto err_fail;
	}

	err = device_create_file(dev, &dev_attr_ch2_read);
    if (err != 0){
            dev_err(dev, "device_create_file failed\n");
            goto err_fail;
    }

    err = device_create_file(dev, &dev_attr_ch3_write);
	if (err != 0){
		dev_err(dev, "device_create_file failed\n");
		goto err_fail;
	}

	err = device_create_file(dev, &dev_attr_ch3_read);
    if (err != 0){
            dev_err(dev, "device_create_file failed\n");
            goto err_fail;
    }

    err = device_create_file(dev, &dev_attr_ch4_write);
	if (err != 0){
		dev_err(dev, "device_create_file failed\n");
		goto err_fail;
	}

	err = device_create_file(dev, &dev_attr_ch4_read);
    if (err != 0){
            dev_err(dev, "device_create_file failed\n");
            goto err_fail;
    }

    err = device_create_file(dev, &dev_attr_ch5_write);
	if (err != 0){
		dev_err(dev, "device_create_file failed\n");
		goto err_fail;
	}

	err = device_create_file(dev, &dev_attr_ch5_read);
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
	 iounmap(control_channel);

	 printk("removing sysfs files\n");

    device_remove_file(dev, &dev_attr_init);
	device_remove_file(dev, &dev_attr_ch1_write);
	device_remove_file(dev, &dev_attr_ch1_read);
	device_remove_file(dev, &dev_attr_ch2_write);
	device_remove_file(dev, &dev_attr_ch2_read);
	device_remove_file(dev, &dev_attr_ch3_write);
	device_remove_file(dev, &dev_attr_ch3_read);
	device_remove_file(dev, &dev_attr_ch4_write);
	device_remove_file(dev, &dev_attr_ch4_read);
	device_remove_file(dev, &dev_attr_ch5_write);
	device_remove_file(dev, &dev_attr_ch5_read);


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
