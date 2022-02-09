#include <linux/init.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include "stm32mp1xx_gpio.h"
#include "stm32mp1xx_rcc.h"
#include "stm32mp1xx_tim.h"

#define MOTORNAME "myMotor"

struct cdev *cdev;
struct class *cls;
struct device *dev;
int major = 0;
int minor = 0;
rcc_t * rcc = NULL;
gpio_t * gpiof = NULL;
tim16_17_t * tim16 = NULL;

int myMotor_open(struct inode *inode,struct file *file)
{
	printk("%s:%d\n",__func__,__LINE__);
	return 0;
}

int myMotor_close(struct inode *inode,struct file *file)
{
	printk("%s:%d\n",__func__,__LINE__);
	return 0;
}

long myMotor_ioctl(struct file *file, unsigned int cmd,unsigned long args)
{

	switch(cmd)
	{
	case 0:
			tim16->BDTR |= (0x1 << 15);
		break;
	case 1:
			tim16->BDTR &= (~(0x1 << 15));
		break;
	}
	return 0;
}

const struct file_operations fops = {
	.open = myMotor_open,
	.unlocked_ioctl = myMotor_ioctl,
	.release = myMotor_close,
};

static int __init myMotor_init(void)
{
	int ret;
	dev_t devno;
	printk("%s:%d\n",__func__,__LINE__);

	cdev = cdev_alloc();
	if(cdev == NULL)
	{
		printk("cdev alloc error\n");
		ret = -ENOMEM;
		return ret;
	}

	cdev_init(cdev,&fops);

	ret = alloc_chrdev_region(&devno,minor,1,"myMotor");
	if(ret)
	{
		printk("alloc device number error\n");
		goto ERR1;
	}
	major = MAJOR(devno);
	minor = MINOR(devno);

	ret = cdev_add(cdev,devno,1);
	if(ret)
	{
		printk("cdev register error\n");
		goto ERR2;
	}

	cls = class_create(THIS_MODULE,"2");
	if(IS_ERR(cls))
	{
		printk("class create error\n");
		ret = PTR_ERR(cls);
		goto ERR3;
	}
	dev = device_create(cls,NULL,devno,NULL,MOTORNAME);
	if(IS_ERR(dev))
	{
		printk("device create error\n");
		ret = PTR_ERR(dev);
		goto ERR4;
	}
	rcc = ioremap(RCC,sizeof(rcc_t));
	rcc->MP_AHB4ENSETR |= (0x1 << 5);	
	rcc->MP_APB2ENSETR |= (0x1 << 3);
	if(rcc == NULL){
		printk("ioremap rcc error\n");
		return -ENOMEM;
	}
	gpiof = ioremap(GPIOF,sizeof(gpio_t));
	if(gpiof == NULL){
		printk("ioremap gpiof base error\n");
		return -ENOMEM;
	}
	gpiof->MODER&=~(0x3<<12);
	gpiof->MODER|=(0x2<<12);
	gpiof->AFRL &=~(0xf<<24);
	gpiof->AFRL |=(0x1<<24);
	tim16 = ioremap(TIM16,sizeof(tim16_17_t));
	if(tim16 == NULL){
		printk("ioremap tim16 base error\n");
		return -ENOMEM;
	}

	tim16->PSC = 208;
	tim16->ARR = 1000;
	tim16->CCR1 = 500;
	tim16->CCMR1 &=(~(0x1<<16));
	tim16->CCMR1 &=(~(0x7 <<4));	
	tim16->CCMR1 |=(0x6 <<4);
	tim16->CCMR1 |=(0x1<<3);
	tim16->CCER &=~(0x1 << 3);	
	tim16->CCER |=(0x1 <<1);
	tim16->CCER |=(0x1 <<2);
	tim16->CCER |=0x1;
	tim16->CR1 |=(0x1 <<7);
	tim16->CR1 &=~(0x3 <<5);
	tim16->CR1 |=(0x1 <<4);
	tim16->CR1 |=(0x1);

	
	return 0;

ERR4:
	class_destroy(cls);

ERR3:
	cdev_del(cdev);

ERR2:
	unregister_chrdev_region(devno,1);

ERR1:
	kfree(cdev);
	return ret;
}

void __exit myMotor_exit(void)
{
	int devno;
	devno = ((major << 20) + minor);
	iounmap(gpiof);
	iounmap(rcc);
	iounmap(tim16);
	
	
	device_destroy(cls,devno);

	class_destroy(cls);

	cdev_del(cdev);

	unregister_chrdev_region(devno,1);

	kfree(cdev);
}

module_init(myMotor_init);
module_exit(myMotor_exit);
MODULE_LICENSE("GPL");

