#include "linux/init.h"
#include "linux/module.h"
#include "linux/fs.h"
#include "linux/device.h"
#include "linux/err.h"
#include "linux/slab.h"
#include "linux/interrupt.h"
#include "linux/wait.h"
#include "linux/sched.h"
#include "linux/poll.h"

#include "asm/io.h"
#include "asm/uaccess.h"
#include "asm/pgtable.h"

#include <asm/pgalloc.h>
#include <asm/tlb.h>
#include <asm/tlbflush.h>







#define KEY_MMAP_BUF 	_IOR('K',1,int)

struct key_mseg_t
{
	char* key_name;
	unsigned int key_irq;
	unsigned int key_value;
};

struct key_mseg_t key_mseg[]={
	[0]={
		.key_name = "up",
		.key_irq = IRQ_EINT(0),
		.key_value = 1,
	},
	[1]={
		.key_name = "down",
		.key_irq = IRQ_EINT(1),
		.key_value = 2,
	},
	[2]={
		.key_name = "left",
		.key_irq = IRQ_EINT(2),
		.key_value = 3,
	},
	[3]={
		.key_name = "right",
		.key_irq = IRQ_EINT(3),
		.key_value = 4,
	},
	[4]={
		.key_name = "enter",
		.key_irq = IRQ_EINT(4),
		.key_value = 5,
	},
	[5]={
		.key_name = "esc",
		.key_irq = IRQ_EINT(5),
		.key_value = 6,
	},
	[6]={
		.key_name = "home",
		.key_irq = IRQ_EINT(22),
		.key_value = 7,
	},
	[7]={
		.key_name = "power",
		.key_irq = IRQ_EINT(23),
		.key_value = 8,
	},
};
		

struct S5pv210_key 
{
	int  major;
	struct class * cls;
	struct  device *  devi;
	unsigned int * gpio_ctl;	//GPIOC03
	unsigned int * gpio_dat;
	wait_queue_head_t wq;
	int have_data;
	struct key_mseg_t * key_mseg_p; 
	void * vir_addr ;
};

struct S5pv210_key * drv_key;



int drv_key_open (struct inode * inode, struct file * file)
{	
    printk("--------------%s---------------\n",__FUNCTION__);
    return 0;
}

int drv_key_close (struct inode * inode, struct file * file)
{		
    printk("--------------%s---------------\n",__FUNCTION__);
    return 0;
}

ssize_t drv_key_write(struct file * file, const char * buf, size_t size, loff_t * flag)
{
	int ret,on;
	printk("--------------%s---------------\n",__FUNCTION__);
	ret= copy_from_user(&on,buf,size);
	return ret;
}

long drv_key_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret;
	char buf[100];

	printk("--------------%s---------------\n",__FUNCTION__);
	switch(cmd)
	{
		case KEY_MMAP_BUF:
			memcpy(buf,drv_key->vir_addr,sizeof(buf));
			ret=copy_to_user((void __user *)arg, buf, sizeof(buf));
			if(ret!=0){
				printk("copy_to_user is error\n");
				return ret;
			}
			break;
		default:
			break;
		
	}

	return 0;
}

ssize_t drv_key_read (struct file *file, char *buf, size_t size, loff_t *flag)
{
	int ret=0;
    printk("--------------%s---------------\n",__FUNCTION__);


	
	if((file->f_flags&O_NONBLOCK)!=0 &&  drv_key->have_data == 0){
		return 0;
	}

	ret=copy_to_user(buf,drv_key->vir_addr,size);
//	wait_event_interruptible(drv_key->wq,drv_key->have_data);
//	ret=copy_to_user(buf,&drv_key->key_mseg_p->key_value,size);
	if(ret!=0){
		printk("copy_to_user is error\n");
		return ret;
	}
	drv_key->have_data=0;
	return 0;

}


unsigned int drv_key_poll (struct file * filp, struct poll_table_struct *pts)
{
	unsigned int mask=0;
	
    printk("--------------%s---------------\n",__FUNCTION__);


	poll_wait(filp,&drv_key->wq,pts);
	
	if(drv_key->have_data){
		return POLLIN;
	}

	return mask;
}


int drv_key_mmap(struct file * file, struct vm_area_struct * vma)
{
	unsigned long phy_addr;

    printk("--------------%s---------------\n",__FUNCTION__);


	//让这一块虚拟内存，对应一块物理内存
	phy_addr = virt_to_phys(drv_key->vir_addr);
	//将这块物理内存映射给用户
	if(remap_pfn_range(vma,vma->vm_start,phy_addr>>PAGE_SHIFT,PAGE_SIZE,vma->vm_page_prot))
	{
		printk("remap_pfn_range is error\n");
		return -EAGAIN;
	}
	return 0;
}

//文件接口
static const struct file_operations fops={
	.open			=	drv_key_open,
	.release 		= 	drv_key_close,
	.write			=	drv_key_write,
	.unlocked_ioctl = 	drv_key_ioctl,
	.read			=	drv_key_read,
	.poll			=	drv_key_poll,
	.mmap			=	drv_key_mmap,

};


irqreturn_t key_irq_handle(int irq,void* dev)
{
    printk("--------------%s---------------\n",__FUNCTION__);


	
	drv_key->key_mseg_p=(struct key_mseg_t*)dev;
	drv_key->have_data=1;
	wake_up_interruptible(&drv_key->wq);

	printk("ke:buf=%s\n",drv_key->vir_addr);

	return IRQ_HANDLED;
}




// 挂载模块
static int __init drv_key_init(void)
{
	int i;
	int ret=0;
    printk("--------------%s---------------\n",__FUNCTION__);

	//实例化对象
	drv_key=kzalloc(sizeof(struct S5pv210_key), GFP_KERNEL);
	if(IS_ERR(drv_key)){
		printk("drv_key kzalloc is error\n");
		ret=PTR_ERR(drv_key);
		return ret;
	}	

	// 申请虚拟空间给mmap用
	drv_key->vir_addr=kzalloc(PAGE_SIZE, GFP_KERNEL);
	if(IS_ERR(drv_key->vir_addr)){
		printk("drv_key->vir_addr kzalloc is error\n");
		ret=PTR_ERR(drv_key->vir_addr);
		goto vir_addr_err;
	}	



	//注册主设备号
	drv_key->major = register_chrdev(0,"drv_key", &fops);
	if(drv_key->major<0)
	{
		goto register_err;
		ret = drv_key->major ;
	}
	//创建类
	drv_key->cls=class_create(THIS_MODULE,"drv_key");
	if(IS_ERR(drv_key->cls)){
		ret = PTR_ERR(drv_key->cls);
		goto class_err;
	}
	// 创建设备 
	drv_key->devi=device_create(drv_key->cls,NULL,MKDEV(drv_key->major,5),NULL,"drv_key");
	if(IS_ERR(drv_key->devi)){
		ret = PTR_ERR(drv_key->devi);
		goto devi_err;
	}

	// 硬件初始化
	drv_key->have_data =0;
	init_waitqueue_head(&drv_key->wq);
	
	for(i=0;i<ARRAY_SIZE(key_mseg);i++)
	{
		ret=request_irq(key_mseg[i].key_irq,key_irq_handle,IRQF_TRIGGER_RISING,key_mseg[i].key_name, &key_mseg[i]);
		if(ret!=0)
		{
			printk("key request_irq is error\n");
			goto request_irq_err;
		}

	}

	return ret;

	
	request_irq_err:
		for(;i>0;i--){
			free_irq(key_mseg[i].key_irq,&key_mseg[i].key_value);
		}
		device_destroy(drv_key->cls,MKDEV(drv_key->major,5));
	devi_err:
		class_destroy(drv_key->cls);
	class_err:
		unregister_chrdev(drv_key->major,"drv_key");
	register_err:
		kfree(drv_key->vir_addr);
	vir_addr_err:
		kfree(drv_key);//释放申请的空间
    return ret;
}


// 卸载模块
static void __exit drv_key_exit(void)
{
	int i;
    printk("--------------%s---------------\n",__FUNCTION__);


	for(i=0;i<ARRAY_SIZE(key_mseg);i++){
		free_irq(key_mseg[i].key_irq,&key_mseg[i]);
	}

	//删除设备
	device_destroy(drv_key->cls,MKDEV(drv_key->major,5));
	//删除类
	class_destroy(drv_key->cls);  
	//删除主设备号
	unregister_chrdev(drv_key->major,"drv_key");

	kfree(drv_key->vir_addr);
	kfree(drv_key);
}



module_init(drv_key_init);
module_exit(drv_key_exit);
// 同一GPL
MODULE_LICENSE("GPL");
