#include <linux/init.h>
#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/fs.h>
#define NAME "m74hc595"
int major = 0;
struct class *cls;
struct device *dev;
struct spi_device *gspi;
u8 code[] = {
	0x3f, //0
	0x06, //1
	0x5b, //2
	0x4f, //3
	0x66, //4
	0x6d, //5
	0x7d, //6
	0x07, //7
	0x7f, //8
	0x6f, //9
	0x77, //A
	0x7c, //b
	0x39, //c
	0x5e, //d
	0x79, //e
	0x71, //f
};

u8 which[] = {
	0x8, //sg0
	0x4, //sg1
	0x2, //sg2
	0x1, //sg3
};

int m74hc595_open(struct inode *inode, struct file *file)
{
	printk("%s:%d\n",__func__,__LINE__);
	return 0;
}
long m74hc595_ioctl(struct file *file, 
	unsigned int cmd, unsigned long args)
{	
	int a;
	unsigned long num;
	unsigned short b;
	num = args;
	switch(cmd){
		case 1:
		case 2:
			b = 0x80;
			b = b << 8;
			b |= which[2];
			spi_write(gspi,&b,2);
			for(a = 0;a < 4;a++)
			{
				b = code[num%10];
				b = b << 8;
				b |= which[a];
				spi_write(gspi,&b,2);
				num /= 10;
			}
			break;
		case 0://操作码
			for(a = 0;a < 4;a++)
			{
				b = code[num%10];
				b = b << 8;
				b |= which[a];
				spi_write(gspi,&b,2);
				num /= 10;
			}
			break;
		default: printk("ioctl error\n");break;
	}
	
	return 0;
}

int m74hc595_close(struct inode *inode, struct file *file)
{
	printk("%s:%d\n",__func__,__LINE__);
	return 0;
}
struct file_operations fops = {
	.open = m74hc595_open,
	.unlocked_ioctl = m74hc595_ioctl,
	.release = m74hc595_close,
};

int	m74hc595_probe(struct spi_device *spi)
{
	u8 buf[2] = {0xf,0x0};
	printk("%s:%d\n",__func__,__LINE__);
	gspi = spi;
	spi_write(gspi,buf,ARRAY_SIZE(buf));
	major = register_chrdev(0,NAME,&fops);
	if(major < 0){
		printk("register chrdev error\n");
		return major;
	}

	cls = class_create(THIS_MODULE,NAME);
	if(IS_ERR(cls)){
		printk("class create  error\n");
		return PTR_ERR(cls);
	}
	dev = device_create(cls,NULL,MKDEV(major,0),NULL,NAME);
	if(IS_ERR(dev)){
		printk("device create  error\n");
		return PTR_ERR(dev);
	}
	return 0;
}

int	m74hc595_remove(struct spi_device *spi)
{
	printk("%s:%d\n",__func__,__LINE__);
	device_destroy(cls,MKDEV(major,0));
	class_destroy(cls);
	unregister_chrdev(major,NAME);
	return 0;
}

const struct of_device_id of_match[] = {
	{.compatible = "hqyj,m74hc595",},
	{},
};
MODULE_DEVICE_TABLE(of,of_match);

const struct spi_device_id id_match[] = {
	{"id_m74hc595",0},
	{},
};
MODULE_DEVICE_TABLE(spi,id_match);

struct spi_driver m74hc595 = {
	.probe = m74hc595_probe,
	.remove = m74hc595_remove,
	.driver = {
		.name = "hello_m74hc595",
		.of_match_table = of_match,
	},	
	.id_table = id_match,
};
module_spi_driver(m74hc595);
MODULE_LICENSE("GPL");
