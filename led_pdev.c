#include "linux/init.h"
#include "linux/module.h"
#include "linux/ioport.h"
#include "linux/platform_device.h"
#include "led_plat.h"



#define REG_GPC0_CTL 0xe0200060
#define REG_GPC0_ZIZE 8

struct led_platdata_t led_pdata={
	.reg_confg_clear	=0xff,
	.reg_confg_data		=0x11,
	.reg_data			=0x03,
	.shift				=3,
};

// LED资源
struct resource led_resource[] ={
	[0]= {
		.start 	= REG_GPC0_CTL,
		.end	= REG_GPC0_CTL+REG_GPC0_ZIZE-1,
		.name 	= "led_dev",
		.flags	= IORESOURCE_MEM, 					//内存资源，另外还有中断资源IORESOURCE_IRQ
	},
};



// 空函数，卸载的时候使用，没有会警告
void	led_release(struct device *dev)
{
	printk("--------------%s---------------\n",__FUNCTION__);
}

struct device	dev;

struct platform_device led_pdev =
{
	.name = "S5pv210_led",					//平台总线匹配需要
	.id = -1,						//默认-1
	.resource 		=led_resource,
	.num_resources	=ARRAY_SIZE(led_resource),
	.dev			={
		.release 		= led_release,
		.platform_data	= &led_pdata,
	},
};



// 模块加载
static int __init led_pdev_init(void)
{
	printk("--------------%s---------------\n",__FUNCTION__);

	// 注册平台总线设备
	return 	platform_device_register(&led_pdev);
}
module_init(led_pdev_init);


// 模块卸载
static void __exit led_pdev_exit(void)
{
	printk("--------------%s---------------\n",__FUNCTION__);

	// 卸载注册
	return	platform_device_unregister(&led_pdev);
}
module_exit(led_pdev_exit);

// 描述
MODULE_DESCRIPTION("led pdev moudle use paltform bus");
// 别名
MODULE_ALIAS("led pdev");
// 作者
MODULE_AUTHOR("权辰宙");
// 证书
MODULE_LICENSE("GPL");







