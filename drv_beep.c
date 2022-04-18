#include "linux/init.h"
#include "linux/module.h"
#include "linux/fs.h"
#include "linux/device.h"
#include "linux/err.h"
#include "linux/slab.h"
#include "asm/io.h"
#include <asm/uaccess.h>

#define REG_GPD0CON 0xE02000A0


struct S5pv210_beep //D0_1
{
	int  major;
	struct class * cls;
	struct  device *  devi;
	unsigned int * gpio_ctl;	//GPIOC03
	unsigned int * gpio_dat;
};

struct S5pv210_beep * drv_beep;



int drv_beep_open (struct inode * inode, struct file * file)
{	
    printk("--------------%s---------------\n",__FUNCTION__);
    return 0;
}

int drv_beep_close (struct inode * inode, struct file * file)
{		
	
    printk("--------------%s---------------\n",__FUNCTION__);
    return 0;
}


ssize_t drv_beep_write(struct file * file, const char * buf, size_t size, loff_t * flag)
{
	int on;
	int ret;
	printk("--------------%s---------------\n",__FUNCTION__);


	ret= copy_from_user(&on,buf,size);
	if(ret!=0){
		printk("copy_from_user is error\n");
		ret=-1;
	}
	return ret;
}

#define BEEP_ON 		_IO('B',1) 
#define BEEP_OFF 		_IO('B',2)  



long drv_beep_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	unsigned int reg_val=0;

	printk("--------------%s---------------\n",__FUNCTION__);
	switch(cmd)
	{
		case BEEP_ON:
			printk("--------------BEEP_ON---------------\n");
			reg_val=readl(drv_beep->gpio_dat);
			reg_val |= (1<<1);
			writel(reg_val,drv_beep->gpio_dat);
			break;
		case BEEP_OFF:
			printk("--------------BEEP_OFF---------------\n");
			reg_val=readl(drv_beep->gpio_dat);
			reg_val &=~ (1<<1);
			writel(reg_val,drv_beep->gpio_dat);
			break;
		default:
			printk("--------------default---------------\n");
			break;

	}
	return 0;
}


//文件接口
static const struct file_operations fops={
	.open			=	drv_beep_open,
	.release 		= 	drv_beep_close,
	.write			=	drv_beep_write,
	.unlocked_ioctl = 	drv_beep_ioctl,
};





// 挂载模块
static int __init drv_beep_init(void)
{
	unsigned int reg_val;
	int ret=0;
    printk("--------------%s---------------\n",__FUNCTION__);

	//实例化对象
	drv_beep=kzalloc(sizeof(struct S5pv210_beep), GFP_KERNEL);
	if(IS_ERR(drv_beep)){
		printk("drv_beep kzalloc is error\n");
		ret=PTR_ERR(drv_beep);
		return ret;
	}	

	//注册主设备号
	drv_beep->major = register_chrdev(0,"drv_beep", &fops);
	if(drv_beep->major<0)
	{
		goto register_err;
		ret = drv_beep->major ;
	}
	//创建类
	drv_beep->cls=class_create(THIS_MODULE,"drv_beep");
	if(IS_ERR(drv_beep->cls)){
		ret = PTR_ERR(drv_beep->cls);
		goto class_err;
	}
	// 创建设备 
	drv_beep->devi=device_create(drv_beep->cls,NULL,MKDEV(drv_beep->major,5),NULL,"drv_beep");
	if(IS_ERR(drv_beep->devi)){
		ret = PTR_ERR(drv_beep->devi);
		goto devi_err;
	}

	//映射寄存器
	drv_beep->gpio_ctl = ioremap (REG_GPD0CON,8);
	if(IS_ERR(drv_beep->gpio_ctl)){
		ret = PTR_ERR(drv_beep->gpio_ctl);
		goto ioremap_err;
	}
	drv_beep->gpio_dat = drv_beep->gpio_ctl+1;
	reg_val=0;




	// 硬件初始化
	reg_val=readl(drv_beep->gpio_ctl);
	reg_val &=~(0xf<<4);
	reg_val |= (1<<4);
	writel(reg_val,drv_beep->gpio_ctl);

	reg_val=0;
	reg_val=readl(drv_beep->gpio_dat);
	reg_val &=~ (1<<1);
	writel(reg_val,drv_beep->gpio_dat);

	return ret;


	ioremap_err:
		device_destroy(drv_beep->cls,MKDEV(drv_beep->major,5));
	devi_err:
		class_destroy(drv_beep->cls);
	class_err:
		unregister_chrdev(drv_beep->major,"drv_beep");
	register_err:

	kfree(drv_beep);//释放申请的空间
    return ret;
}


// 卸载模块
static void __exit drv_beep_exit(void)
{
    printk("--------------%s---------------\n",__FUNCTION__);


	iounmap(drv_beep->gpio_ctl);

	//删除设备
	device_destroy(drv_beep->cls,MKDEV(drv_beep->major,5));
	//删除类
	class_destroy(drv_beep->cls);  
	//删除主设备号
	unregister_chrdev(drv_beep->major,"drv_beep");

	
}



module_init(drv_beep_init);
module_exit(drv_beep_exit);
// 同一GPL
MODULE_LICENSE("GPL");
