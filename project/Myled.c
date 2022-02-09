#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include "operation.h"


int gpiono;
unsigned int major = 0;
struct class * cls;
struct device * dev;


int myled_open(struct inode *inode, struct file *file)
{
    printk("%s:%d\n",__func__,__LINE__);
    return 0;
}
ssize_t  myled_read(struct file *file,
    char __user *ubuf, size_t size, loff_t *offs)
{
    
	printk("%s:%d\n",__func__,__LINE__);

    return size;
}
long myled_ioctl(struct file *filp, 
        unsigned int cmd, unsigned long args)
{
    int data,ret;
    switch(cmd){
        case LED1_OP:
            ret = copy_from_user((void *)&data,
                (void *)args,GET_CMD_SIZE(LED1_OP));                            
            if(ret){
                printk("copy data from user error\n");
                return -EINVAL;
            }
            data==1?gpio_set_value(gpiono,1):gpio_set_value(gpiono,0);
            break;    
    }   
    return 0;
    
}


int myled_close(struct inode *inode, struct file *file)
{
    printk("%s:%d\n",__func__,__LINE__);
    return 0;
}


const struct file_operations fops =
{
    .open = myled_open,
    .read = myled_read,
    .release = myled_close,
    .unlocked_ioctl = myled_ioctl,  
};
int pdrv_probe(struct platform_device *pdev)
{
    int ret;
    printk("%s:%d\n",__func__,__LINE__);
    gpiono = of_get_named_gpio(pdev->dev.of_node,"led1",0);
    //×¢²á×Ö·ûÉè±¸Çý¶¯
    major = register_chrdev(0,"myled",&fops);
    if(major < 0){
        printk("register chrdev error\n");
        ret =  major;
        goto ERR1;
    }
    cls = class_create(THIS_MODULE,"myled");
    if(IS_ERR(cls)){
        printk("class create error\n");
        ret = PTR_ERR(cls);
        goto ERR2;
    }
    dev = device_create(cls,NULL,MKDEV(major,0),NULL,"myled");
    if(IS_ERR(dev)){
        printk("device create error\n");
        ret = PTR_ERR(dev);
        goto ERR3;
    }
    ret = gpio_request(gpiono,NULL);
    if(ret < 0){
        printk("gpio request error\n");
        goto ERR4;
    }
    gpio_direction_output(gpiono,0); 
	 return 0;
	 ERR4:
		 device_destroy(cls,MKDEV(major,0));
	 ERR3:
		 class_destroy(cls);
	 ERR2:
		 unregister_chrdev(major,"myled");
	 ERR1:
	return ret;
 
	 
 }
 int pdrv_remove(struct platform_device *pdev)
 {
 
	 gpio_free(gpiono);
	 device_destroy(cls,MKDEV(major,0));
	 class_destroy(cls);
	 unregister_chrdev(major,"myled");
	 printk("%s:%d\n",__func__,__LINE__);
	 return 0;																	 
 }
 
 
 struct of_device_id oftable[] = {
	 {.compatible = "hqyj,myled",},
	 {/*end*/}
 };
 struct platform_driver pdrv = {
	 .probe = pdrv_probe,
	 .remove = pdrv_remove,
	 .driver = {
		 .name = "woyaogaoxin",
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
 
