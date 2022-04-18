#include "linux/init.h"
#include "linux/module.h"
#include "linux/fs.h"
#include "linux/device.h"
#include "linux/err.h"
#include "linux/slab.h"
#include "linux/platform_device.h"

#include "asm/io.h"
#include <asm/uaccess.h>
#include "led_plat.h"




// do_sys_open  do_dentry_open chrdev_open do_sys_open chrdev_open
// shmem_get_inode kobject 
// init_special_inode(struct inode * inode, umode_t mode, dev_t rdev)

struct S5pv210_led 
{
	int  major;
	struct class * cls;
	struct  device *  devi;
	
	struct resource *res;
	struct led_platdata_t *pd;

	unsigned int * gpioc_ctl;		//虚拟空间下的GPC0控制寄存器的地址
	unsigned int * gpioc_dat;

	
};

struct S5pv210_led * led_plat;



int led_plat_open (struct inode * inode, struct file * file)
{	
	int reg_val;
	int clear =led_plat->pd->reg_confg_clear;
	int data =led_plat->pd->reg_confg_data;
	int shift =led_plat->pd->shift*4;
	
    printk("--------------%s---------------\n",__FUNCTION__);

	// 硬件初始化
	reg_val=readl(led_plat->gpioc_ctl);
	reg_val &=~(clear<<shift);
	reg_val |= (data<<shift);
	writel(reg_val,led_plat->gpioc_ctl);
    return 0;
}

int led_plat_close (struct inode * inode, struct file * file)
{		
	
    printk("--------------%s---------------\n",__FUNCTION__);
    return 0;
}


ssize_t led_plat_write(struct file * file, const char * buf, size_t size, loff_t * flag)
{
	unsigned int reg_val=0;
	int on;
	int ret;
	int clear =led_plat->pd->reg_confg_clear;
	int data =led_plat->pd->reg_data;
	int shift =led_plat->pd->shift;

	printk("--------------%s---------------\n",__FUNCTION__);

	ret= copy_from_user(&on,buf,size);
	if(ret!=0){
		printk("copy_from_user is error\n");
		ret=-1;
	}
		
	if(on==0){
		reg_val=readl(led_plat->gpioc_dat);
		reg_val &=~ (data<<shift);
		writel(reg_val,led_plat->gpioc_dat);
	}else{
		reg_val=readl(led_plat->gpioc_dat);
		reg_val |= (data<<shift);
		writel(reg_val,led_plat->gpioc_dat);
	}
	return ret;
}

#define LED_ALL_ON 		_IO('L',1) 
#define LED_ALL_OFF 	_IO('L',2) 
#define LED_NUM_ON 		_IOW('L',3,int) 
#define LED_NUM_OFF 	_IOW('L',4,int) 


long led_plat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	unsigned int reg_val=0;

	printk("--------------%s---------------\n",__FUNCTION__);
	switch(cmd)
	{
		case LED_ALL_ON:
			reg_val=readl(led_plat->gpioc_dat);
			reg_val |= (3<<3);
			writel(reg_val,led_plat->gpioc_dat);
			break;
		case LED_ALL_OFF:
			reg_val=readl(led_plat->gpioc_dat);
			reg_val &=~ (3<<3);
			writel(reg_val,led_plat->gpioc_dat);
			break;
		case LED_NUM_ON:
			reg_val=readl(led_plat->gpioc_dat);
			reg_val |= (1<<arg);
			writel(reg_val,led_plat->gpioc_dat);
			break;
		case LED_NUM_OFF:
			reg_val=readl(led_plat->gpioc_dat);
			reg_val &=~ (1<<arg);
			writel(reg_val,led_plat->gpioc_dat);
			break;
	}
	return 0;
}


//文件接口
static const struct file_operations fops={
	.open			=	led_plat_open,
	.release 		= 	led_plat_close,
	.write			=	led_plat_write,
	.unlocked_ioctl = 	led_plat_ioctl,
	.compat_ioctl 	= 	led_plat_ioctl,
};




int led_pdrv_probe(struct platform_device * pdev)
{
	int ret=0;

	
	printk("--------------%s---------------\n",__FUNCTION__);


	


	//实例化对象
	led_plat=kzalloc(sizeof(struct S5pv210_led), GFP_KERNEL);
	if(IS_ERR(led_plat)){
		printk("led_plat kzalloc is error\n");
		ret=PTR_ERR(led_plat);
		return ret;
	}	

	
	// 获取平台总线资源
	led_plat->res=platform_get_resource(pdev,IORESOURCE_MEM,0);
	printk("kernel:res->start=%d\n",led_plat->res->start);
	printk("kernel:res->name=%s\n",led_plat->res->name);	

	// 获取平台总线自定义数据
	led_plat->pd =(pdev->dev.platform_data);

	//注册主设备号到设备号的离散表
	led_plat->major = register_chrdev(0,"led_plat", &fops);
	if(led_plat->major<0)
	{
		goto register_err;
		ret = led_plat->major ;
	}


	
	//创建类
	led_plat->cls=class_create(THIS_MODULE,"led_plat");
	if(IS_ERR(led_plat->cls)){
		ret = PTR_ERR(led_plat->cls);
		goto class_err;
	}
	// 创建设备 
	led_plat->devi=device_create(led_plat->cls,NULL,MKDEV(led_plat->major,5),NULL,"led_plat");
	if(IS_ERR(led_plat->devi)){
		ret = PTR_ERR(led_plat->devi);
		goto devi_err;
	}

	//映射寄存器
	led_plat->gpioc_ctl = ioremap (led_plat->res->start,8);
	if(IS_ERR(led_plat->gpioc_ctl)){
		ret = PTR_ERR(led_plat->gpioc_ctl);
		goto ioremap_err;
	}
	led_plat->gpioc_dat = led_plat->gpioc_ctl+1;




	return ret;


	ioremap_err:
		device_destroy(led_plat->cls,MKDEV(led_plat->major,5));
	devi_err:
		class_destroy(led_plat->cls);
	class_err:
		unregister_chrdev(led_plat->major,"led_plat");
	register_err:
		kfree(led_plat);//释放申请的空间
	

}

int led_pdrv_remove(struct platform_device * pdev)
{
	int ret;
	printk("--------------%s---------------\n",__FUNCTION__);


	iounmap(led_plat->gpioc_ctl);

	//删除设备
	device_destroy(led_plat->cls,MKDEV(led_plat->major,5));
	//删除类
	class_destroy(led_plat->cls);  
	//删除主设备号
	unregister_chrdev(led_plat->major,"led_pdrv");

	return ret;
}




struct platform_driver led_pdrv ={
	.probe		=led_pdrv_probe,
	.remove		=led_pdrv_remove,
	.driver		={
		.name = "S5pv210_led",
	}
};


// 挂载模块
static int __init led_pdrv_init(void)
{
	printk("--------------%s---------------\n",__FUNCTION__);
    return platform_driver_register(&led_pdrv);
}


// 卸载模块
static void __exit led_pdrv_exit(void)
{
    printk("--------------%s---------------\n",__FUNCTION__);
	platform_driver_unregister(&led_pdrv);
}



module_init(led_pdrv_init);
module_exit(led_pdrv_exit);
// 同一GPL
MODULE_LICENSE("GPL");
