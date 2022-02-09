#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/mod_devicetable.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include "operation.h"
unsigned int major = 0,minor = 0;
struct cdev *cdev;
struct class *cls;
struct device *dev;
unsigned short kbuf = 0;
int condition = 0;
struct i2c_client *gclient;
int i2c_read_serial_firware(struct i2c_client *client,unsigned short reg)
{
	int ret;
	char val[2] = {0};
	char regs[2];
	unsigned short rval = 0;
	
	regs[0] = reg >> 8 & 0xff;
	regs[1] = reg & 0xff;
//	char r_buf[] = {((reg>>8)&0xff),(reg&0xff)};
	struct i2c_msg r_msg[] = {
		[0] = {
			.addr = client->addr,
			.flags= 0,
			.len  = 2,
			.buf  = regs,
		},
		[1] = {
			.addr = client->addr,
			.flags= 1,
			.len  = 2,
			.buf  = val,
		},
	};

	ret = i2c_transfer(client->adapter,r_msg,ARRAY_SIZE(r_msg));
	if(ret != ARRAY_SIZE(r_msg)){
		printk("i2c transfer error\n");
		return -EAGAIN;
	}

	rval = val[0];
	rval = rval << 8 | val[1];
	return rval;
}

int si7006_remove(struct i2c_client *client)
{
	dev_t devno;
	devno = (major<<20) + minor;
	device_destroy(cls,devno);

	class_destroy(cls);

	cdev_del(cdev);

	unregister_chrdev_region(devno,1);

	kfree(cdev);

	printk("%s:%d\n",__func__,__LINE__);
	return 0;
}

int si7006_open(struct inode *inode,struct file *file)
{
	printk("%s:%d\n",__func__,__LINE__);
	return 0;
}

long si7006_ioctl(struct file *file,unsigned int cmd,unsigned long args)
{
	int ret;
	unsigned short data;
	switch(cmd)
	{
	case Temperature:
		ret = copy_from_user((void *)&kbuf,(void *)args,GET_CMD_SIZE(Temperature));
		if(ret){
			printk("copy data from user error\n");
			return -EINVAL;
		}
		data = i2c_read_serial_firware(gclient,0xE3E5);
			kbuf = data;
		ret = copy_to_user((void *)args,(void *)&kbuf,GET_CMD_SIZE(Temperature));
		if(ret){
			printk("copy buf from user error\n");
			return -EINVAL;
		}
		break;
	case Humidity:
		ret = copy_from_user((void *)&kbuf,(void *)args,GET_CMD_SIZE(Temperature));
		if(ret){
			printk("copy data from user error\n");
			return -EINVAL;
		}
		data = i2c_read_serial_firware(gclient,0xE0);
		kbuf = data;
		ret = copy_to_user((void *)args,(void *)&kbuf,GET_CMD_SIZE(Temperature));
		if(ret){
			printk("copy buf from user error\n");
			return -EINVAL;
		}
		break;

	}
	return 0;
}

int si7006_close(struct inode *inode,struct file *file)
{
	printk("%s:%d\n",__func__,__LINE__);
	return 0;
}

const struct file_operations fops = {
	.open = si7006_open,
	.unlocked_ioctl = si7006_ioctl,
	.release = si7006_close,
};

int si7006_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int ret;
	dev_t devno;
	gclient = client;
	printk("%s:%d\n",__func__,__LINE__);
	cdev = cdev_alloc();
	if(cdev == NULL)
	{
		printk("cdev alloc error\n");
		ret = -ENOMEM;
		return ret;
	}
	
	cdev_init(cdev,&fops);
	
	ret = alloc_chrdev_region(&devno,minor,1,"si7006");
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

	cls = class_create(THIS_MODULE,"123");
	if(IS_ERR(cls))
	{
		printk("class create error\n");
		ret = PTR_ERR(cls);
		goto ERR3;
	}
	dev = device_create(cls,NULL,devno,NULL,"mysi7006");
	if(IS_ERR(dev))
	{
		printk("device create error\n");
		ret = PTR_ERR(dev);
		goto ERR4;
	}

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



const struct of_device_id of_table[] = {
	{.compatible = "hqyj,si7006",},
	{/*end*/},
};
MODULE_DEVICE_TABLE(of,of_table);

struct i2c_driver si7006 = {
	.probe = si7006_probe,
	.remove = si7006_remove,
	.driver = {
		.name = "haha",
		.of_match_table = of_table,	
	}
};


module_i2c_driver(si7006);
MODULE_LICENSE("GPL");

