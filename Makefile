KERNEL_DIR=/home/qcz/S5pv210/kernel/linux-3.0.8
CUE_DIR   =$(shell pwd)
APPS	=test

MOUDLE1 =led_pdev
MOUDLE2 =led_pdrv


# -C指定执行的子Makefile文件的位置 M是指当前Makefile文件的文件参与到编译中
all:
	make -C $(KERNEL_DIR)  M=$(CUE_DIR) modules
	arm-none-linux-gnueabi-gcc -o $(APPS) $(APPS).c

clean:
	make -C $(KERNEL_DIR)  M=$(CUE_DIR) clean
	rm  $(APPS)
install:
	cp *.ko /opt/rootfs/drv_module
	cp $(APPS)  /opt/rootfs/drv_module

# 编译的模块
obj-m +=$(MOUDLE1).o
obj-m +=$(MOUDLE2).o

