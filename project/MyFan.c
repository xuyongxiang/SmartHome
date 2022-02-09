#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include "stm32mp1xx_gpio.h"
#include "stm32mp1xx_rcc.h"
#include "stm32mp1xx_tim.h"
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/slab.h>
#include "operation.h"
#define CNAME "myfan"
struct cdev *cdev;
struct class *cls;
struct device *dev;
dev_t devno;
rcc_t * rcc = NULL;
gpio_t * gpioe = NULL;
tim1_t * tim1 = NULL;
int myfan_open(struct inode *inode, struct file *filp)
{
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);


	return 0;
}
int myfan_close(struct inode *inode, struct file *filp)
{
	//tim1->CCER &=~(0x1);
	printk("%s:%s:%d\n",__FILE__,__func__,__LINE__);
	return 0;
}
long myfan_ioctl(struct file *filp, 
		unsigned int cmd, unsigned long args)
{

 int data=0,ret;
    switch(cmd){
    case FAN_OP:
        ret = copy_from_user((void *)&data,
                (void *)args,GET_CMD_SIZE(FAN_OP));                                
        if(ret){
              printk("copy data from user error\n");
              return -EINVAL;
         }                                           
         data==1?(tim1->BDTR |=(0x1<<15)):(tim1->BDTR &=~(0x1<<15));
        break;    
    }            		
	return 0;

}

const struct file_operations fops = {
	.open = myfan_open,
	.unlocked_ioctl = myfan_ioctl,  	
	.release = myfan_close,
};


int pdrv_remove(struct platform_device *pdev)
{
	iounmap(gpioe);
	iounmap(tim1);
	iounmap(rcc);
	device_destroy(cls,devno);
	class_destroy(cls);
	cdev_del(cdev);
	unregister_chrdev_region(devno,1);
	kfree(cdev);
	return 0;

}


int pdrv_probe(struct platform_device *pdev)
{
	int ret=0;
	printk("2222222222222222222\n");
	cdev = cdev_alloc();
	if(cdev==NULL)
	{
		printk("cdev alloc error\n");
		ret = -ENOMEM;
		goto ERR1;
	}
	//2.对象初始化
	cdev_init(cdev,&fops);

	ret = alloc_chrdev_region(&devno,0,1,CNAME);
	if(ret){
		printk("dynamic:alloc device number error\n");
		goto ERR2;
	}
	ret = cdev_add(cdev,devno,1);
	if(ret){
		printk("cdev register error\n");
		goto ERR3;
	}
	cls = class_create(THIS_MODULE,"my123fan");
	if(IS_ERR(cls)){
		printk("class create error\n");
		ret = PTR_ERR(cls);
		goto ERR4;
	}
	dev = device_create(cls,NULL,devno,
			NULL,CNAME);
	if(IS_ERR(dev)){
		printk("device create error\n");
		ret = PTR_ERR(dev);
		goto ERR5;
	}
	rcc = ioremap(RCC,sizeof(rcc_t));
	rcc->MP_AHB4ENSETR |= (0x1 << 4);	
	rcc->MP_APB2ENSETR |= (0x1);
	if(rcc == NULL){
		printk("ioremap rcc error\n");
		return -ENOMEM;
	}
	gpioe = ioremap(GPIOE,sizeof(gpio_t));
	if(gpioe == NULL){
		printk("ioremap gpioe base error\n");
		return -ENOMEM;
	}
	gpioe->MODER&=~(0x3<<18);
	gpioe->MODER|=(0x2<<18);
	gpioe->AFRH &=~(0xf<<4);
	gpioe->AFRH |=(0x1<<4);
	tim1 = ioremap(TIM1,sizeof(tim1_t));
	if(tim1 == NULL){
		printk("ioremap tim1 base error\n");
		return -ENOMEM;
	}
	tim1->PSC = 208;
	tim1->ARR = 1000;
	tim1->CCR1 = 500;
	tim1->CCMR1 &=(~(0x1<<16));
	tim1->CCMR1 &=(~(0x7 <<4));	
	tim1->CCMR1 |=(0x6 <<4);
	tim1->CCMR1 |=(0x1<<3);
	tim1->CCER &=~(0x1 << 3);	
	tim1->CCER |=(0x1 <<1);
	tim1->CCER |=(0x1 <<2);
	tim1->CCER |=0x1;
	tim1->CR1 |=(0x1 <<7);
	tim1->CR1 &=~(0x3 <<5);
	tim1->CR1 |=(0x1 <<4);
	tim1->CR1 |=(0x1);
	return 0;
ERR5:
	device_destroy(cls,devno);
	class_destroy(cls);
ERR4:
	cdev_del(cdev);
ERR3:
	unregister_chrdev_region(devno,1);
ERR2:
	kfree(cdev);
ERR1:
	return ret;
}

struct of_device_id oftable[] = {
	{.compatible = "hqyj,fan",},
	{/*end*/}
};
struct platform_driver pdrv = {
	.probe = pdrv_probe,
	.remove = pdrv_remove,
	.driver = {
		.name = "woyaogaoxin3",
		.of_match_table = oftable,
	},
}; 																			 
static int __init pdrv_init(void)
{
	return platform_driver_register(&pdrv);
}
static void __exit pdrv_exit(void)
{
	platform_driver_unregister(&pdrv);
}
module_init(pdrv_init);
module_exit(pdrv_exit);
MODULE_LICENSE("GPL");

