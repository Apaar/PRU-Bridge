
/*
 * PRU Bridge driver
 *
 * Copyright (C) 2015 Apaar Gupta
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

#define NUM_CHANNELS 10
#define TOTAL_BUFFER_SIZE 11500

/*
Defining control channel where-

init-check - is set to '1' once the channels have been initialised
channel_size - holds size of each channel must be in multiples of 4
index_data - holds number of characters to be read also can check if buffer is full
head,tail - they are used to maintain the circular buffer
*/
struct control_channel
{
	volatile uint16_t init_check;
	volatile uint16_t channel_size[NUM_CHANNELS];
	volatile uint16_t index_data[NUM_CHANNELS];
	volatile uint16_t buffer_start[NUM_CHANNELS];
	volatile uint16_t head[NUM_CHANNELS];
	volatile uint16_t tail[NUM_CHANNELS];
}size_control;

volatile struct control_channel* control_channel;

#define CONTROL_SIZE sizeof(size_control)+(sizeof(size_control)%4)    //to make sure we are allocating in 4 byte chunks only

/*
Definition of a the virtual buffer
*/
struct circular_buffers
{
	uint8_t data[TOTAL_BUFFER_SIZE];
}size_ring;

volatile struct circular_buffers* ring;

#define CIRCULAR_BUFFER_SIZE sizeof(size_ring)


struct pru_bridge_dev {
	/* Misc device descriptor */
	struct miscdevice miscdev;

	/* Imported members */
	int (*downcall_idx)(int, u32, u32, u32, u32, u32, u32);

	/* Data */
	struct device *p_dev; /* parent platform device */
};

void write_buffer(int ring_no,uint8_t data)			//writing to pru shared memory
{
    ring->data[control_channel->buffer_start[ring_no] + control_channel->tail[ring_no]] = (uint8_t)data;

    if((control_channel->index_data[ring_no])<(control_channel->channel_size[ring_no]))     //allows pru to check if there is data to read or not
        (control_channel->index_data[ring_no])++;

    printk("WRITE-> Data :%d Location :%d Index :%d\n",ring->data[control_channel->buffer_start[ring_no] + control_channel->tail[ring_no]]
                                              ,control_channel->buffer_start[ring_no] + control_channel->tail[ring_no]
                                              ,control_channel->index_data[ring_no]);

    control_channel->tail[ring_no] = (control_channel->tail[ring_no]+1)%(control_channel->channel_size[ring_no]);
}

void write_buffer_wrapper(int ring_no,const char* buf)
{
    write_buffer(ring_no,(uint8_t)buf[0]);
}

uint8_t read_buffer(int ring_no)
{
    if(control_channel->index_data[ring_no] != 0)
    {
        uint8_t value = ring->data[control_channel->buffer_start[ring_no] + control_channel->head[ring_no]];

        (control_channel->index_data[ring_no])--;

        printk("READ -> Data :%d Location :%d Index :%d\n",ring->data[control_channel->buffer_start[ring_no] + control_channel->head[ring_no]]
                                              ,control_channel->buffer_start[ring_no] + control_channel->head[ring_no]
                                              ,control_channel->index_data[ring_no]);

        control_channel->head[ring_no] = (control_channel->head[ring_no]+1)%(control_channel->channel_size[ring_no]);
        return (char)value;
    }
    else
    {
	printk("READ -> NO DATA\n");
        return 0;
    }
}

void flush_buffer(int ring_no,uint8_t* data)
{

    int i = 0,index = control_channel->index_data[ring_no];
    while(i < index)
    {
        data[i] = read_buffer(ring_no);
        i++;
    }
}

static const struct file_operations pru_bridge_fops;

static ssize_t pru_bridge_init_channels(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int i=0,temp_index=0;
    char * p =(char*) buf;
	printk("%s\n",buf);
    while (*p)								//extracting the sizes as integers from character buffer recieved from the handler
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
    printk("Sizes set now initialising the start buffer\n");

    for(i=0;i<NUM_CHANNELS;i++)
    {
        control_channel->buffer_start[i] = temp_index;
        temp_index = temp_index + control_channel->channel_size[i];
        printk("Start number:%d \n",control_channel->buffer_start[i]);
        control_channel->index_data[i] = 0;
        control_channel->head[i] = 0;
        control_channel->tail[i] = 0;
    }

    control_channel->init_check = 1;                                            //setting flags
    printk("Initialised Init : %d\n",control_channel->init_check);
    return strlen(buf)+1;
}

static ssize_t pru_bridge_ch1_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	write_buffer_wrapper(0,buf);
	return strlen(buf)+1;
}

static ssize_t pru_bridge_ch1_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE,"%c\n",read_buffer(0));
}

static ssize_t pru_bridge_ch2_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	write_buffer_wrapper(1,buf);
	return strlen(buf)+1;
}

static ssize_t pru_bridge_ch2_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE,"%c\n",read_buffer(1));
}

static ssize_t pru_bridge_ch3_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	write_buffer_wrapper(2,buf);
	return strlen(buf)+1;
}

static ssize_t pru_bridge_ch3_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE,"%c\n",read_buffer(2));
}

static ssize_t pru_bridge_ch4_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	write_buffer_wrapper(3,buf);
	return strlen(buf)+1;
}

static ssize_t pru_bridge_ch4_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE,"%c\n",read_buffer(3));
}

static ssize_t pru_bridge_ch5_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	write_buffer_wrapper(4,buf);
	return strlen(buf)+1;
}

static ssize_t pru_bridge_ch5_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE,"%c\n",read_buffer(4));
}

static ssize_t pru_bridge_ch6_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	write_buffer_wrapper(5,buf);
	return strlen(buf)+1;
}

static ssize_t pru_bridge_ch6_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE,"%c\n",read_buffer(5));
}

static ssize_t pru_bridge_ch7_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	write_buffer_wrapper(6,buf);
	return strlen(buf)+1;
}

static ssize_t pru_bridge_ch7_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE,"%c\n",read_buffer(6));
}

static ssize_t pru_bridge_ch8_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	write_buffer_wrapper(7,buf);
	return strlen(buf)+1;
}

static ssize_t pru_bridge_ch8_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    return scnprintf(buf, PAGE_SIZE,"%c\n",read_buffer(7));
}

static ssize_t pru_bridge_ch9_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	write_buffer_wrapper(8,buf);
	return strlen(buf)+1;
}

static ssize_t pru_bridge_ch9_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    uint8_t data[control_channel->channel_size[8]];
    flush_buffer(8,data);
    return scnprintf(buf, PAGE_SIZE,"%s\n",data);
}

static ssize_t pru_bridge_ch10_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	write_buffer_wrapper(9,buf);
	return strlen(buf)+1;
}

static ssize_t pru_bridge_ch10_read(struct device *dev, struct device_attribute *attr, char *buf)
{
    uint8_t data[control_channel->channel_size[9]];
    flush_buffer(9,data);
    return scnprintf(buf, PAGE_SIZE,"%s\n",data);
}

static ssize_t pru_bridge_downcall(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    struct platform_device *pdev = to_platform_device(dev);
    struct pru_bridge_dev *pp = platform_get_drvdata(pdev);

    int i=0,downcall_value[7],ret;
    char * p =(char*) buf;
	printk("%s\n",buf);
    while (*p)
    {
        if (isdigit(*p))
        {
            downcall_value[i] = (uint16_t) simple_strtoul(p,&p,10);
            printk("Downcall values:%d\n ",downcall_value[i]);
            i++;
        }
        else
        {
            p++;
        }
    }
    ret = pp->downcall_idx(downcall_value[0],downcall_value[1],downcall_value[2],downcall_value[3],downcall_value[4],downcall_value[5],downcall_value[6]);
	return strlen(buf)+1;
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
static DEVICE_ATTR(ch6_write,S_IWUSR|S_IRUGO,NULL,pru_bridge_ch6_write);
static DEVICE_ATTR(ch6_read, S_IWUSR|S_IRUGO, pru_bridge_ch6_read, NULL);
static DEVICE_ATTR(ch7_write,S_IWUSR|S_IRUGO,NULL,pru_bridge_ch7_write);
static DEVICE_ATTR(ch7_read, S_IWUSR|S_IRUGO, pru_bridge_ch7_read, NULL);
static DEVICE_ATTR(ch8_write,S_IWUSR|S_IRUGO,NULL,pru_bridge_ch8_write);
static DEVICE_ATTR(ch8_read, S_IWUSR|S_IRUGO, pru_bridge_ch8_read, NULL);
static DEVICE_ATTR(ch9_write,S_IWUSR|S_IRUGO,NULL,pru_bridge_ch9_write);
static DEVICE_ATTR(ch9_read, S_IWUSR|S_IRUGO, pru_bridge_ch9_read, NULL);
static DEVICE_ATTR(ch10_write,S_IWUSR|S_IRUGO,NULL,pru_bridge_ch10_write);
static DEVICE_ATTR(ch10_read, S_IWUSR|S_IRUGO, pru_bridge_ch10_read, NULL);
static DEVICE_ATTR(downcall, S_IWUSR|S_IRUGO,NULL,pru_bridge_downcall);

static int pru_bridge_probe(struct platform_device *pdev)
{
	struct pru_bridge_dev *pp;
	struct pru_rproc_external_glue g;
	struct device *dev;
	int err,i=0;

	 /*mapping shared memory for control channel*/
	control_channel = (volatile struct control_channel*)ioremap(SHMDRAM_BASE,CONTROL_SIZE);
	ring = (volatile struct circular_buffers*)ioremap(SHMDRAM_BASE + CONTROL_SIZE , TOTAL_BUFFER_SIZE);
	printk("Control size: %d Buffers :%d\n",CONTROL_SIZE,TOTAL_BUFFER_SIZE);
    control_channel->init_check = 0;
	for(i=0;i<NUM_CHANNELS;i++)
    {
         control_channel->index_data[i] = 0;
         control_channel->channel_size[i] = 0;
         control_channel->buffer_start[i] = 0;
         control_channel->head[i] = 0;
         control_channel->tail[i] = 0;		//initialising pru_data
    }

    for(i=0;i<TOTAL_BUFFER_SIZE;i++)
    {
        ring->data[i] = 0;
    }

	printk("Memory allocated for control channel :%p Buffers :%p\n",control_channel,ring);


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

    err = device_create_file(dev, &dev_attr_ch6_write);
	if (err != 0){
		dev_err(dev, "device_create_file failed\n");
		goto err_fail;
	}

	err = device_create_file(dev, &dev_attr_ch6_read);
    if (err != 0){
            dev_err(dev, "device_create_file failed\n");
            goto err_fail;
    }

    err = device_create_file(dev, &dev_attr_ch7_write);
	if (err != 0){
		dev_err(dev, "device_create_file failed\n");
		goto err_fail;
	}

	err = device_create_file(dev, &dev_attr_ch7_read);
    if (err != 0){
            dev_err(dev, "device_create_file failed\n");
            goto err_fail;
    }

        err = device_create_file(dev, &dev_attr_ch8_write);
	if (err != 0){
		dev_err(dev, "device_create_file failed\n");
		goto err_fail;
	}

	err = device_create_file(dev, &dev_attr_ch8_read);
    if (err != 0){
            dev_err(dev, "device_create_file failed\n");
            goto err_fail;
    }

        err = device_create_file(dev, &dev_attr_ch9_write);
	if (err != 0){
		dev_err(dev, "device_create_file failed\n");
		goto err_fail;
	}

	err = device_create_file(dev, &dev_attr_ch9_read);
    if (err != 0){
            dev_err(dev, "device_create_file failed\n");
            goto err_fail;
    }

        err = device_create_file(dev, &dev_attr_ch10_write);
	if (err != 0){
		dev_err(dev, "device_create_file failed\n");
		goto err_fail;
	}

	err = device_create_file(dev, &dev_attr_ch10_read);
    if (err != 0){
            dev_err(dev, "device_create_file failed\n");
            goto err_fail;
    }

    err = device_create_file(dev, &dev_attr_downcall);
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
    device_remove_file(dev, &dev_attr_ch6_write);
    device_remove_file(dev, &dev_attr_ch6_read);
    device_remove_file(dev, &dev_attr_ch7_write);
    device_remove_file(dev, &dev_attr_ch7_read);
    device_remove_file(dev, &dev_attr_ch8_write);
    device_remove_file(dev, &dev_attr_ch8_read);
    device_remove_file(dev, &dev_attr_ch9_write);
    device_remove_file(dev, &dev_attr_ch9_read);
    device_remove_file(dev, &dev_attr_ch10_write);
    device_remove_file(dev, &dev_attr_ch10_read);
    device_remove_file(dev, &dev_attr_downcall);
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
