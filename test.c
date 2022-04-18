#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <sys/mman.h>
#include<string.h>


#define PAGE_SHIFT	(12)
#define PAGE_SIZE	(1UL << PAGE_SHIFT)


#define BEEP_ON 		_IO('B',1) 
#define BEEP_OFF 		_IO('B',2) 

#define LED_ALL_ON 		_IO('L',1) 
#define LED_ALL_OFF 	_IO('L',2) 
#define LED_NUM_ON 		_IOW('L',3,int) 
#define LED_NUM_OFF 	_IOW('L',4,int) 

#define KEY_MMAP_BUF 	_IOR('K',1,int) 



void main(void)
{
	int type;
	int cmd;
	int on;
	int i;
	int fd_key,fd_beep,fd_led;
	unsigned int key;
	struct pollfd fds[2];

	char buf[100];
	char buf_ioctl[100];
	char buf_read[100];
	void* mmap_addr;

	
//	fd_key=open("/dev/drv_key",O_RDWR);
//	fd_beep=open("/dev/drv_beep",O_RDWR);
	fd_led=open("/dev/led_plat",O_RDWR);





//	mmap_addr=mmap(NULL,PAGE_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,fd_key,0);
//	if(mmap_addr==NULL){
//		printf("app:mmap err\r\n");
//	}

	
	while(1)
	{
		
		on =1;
		write(fd_led,&on,sizeof(on));
		sleep(1);
		on =0;
		write(fd_led,&on,sizeof(on));
		sleep(1);


	/*
		fgets(buf,sizeof(buf),stdin);
		memcpy(mmap_addr,buf,sizeof(buf));
		
		ioctl(fd_key,KEY_MMAP_BUF,buf_ioctl);
		printf("app:buf_ioctl=%s\n",buf_ioctl);

		read(fd_key,buf_read,sizeof(buf_read));
		printf("app:buf_read=%s\n",buf_read);
*/
		
	/*
		printf("app:输入按键\r\n");
		read(fd_key,&key,sizeof(key));
		switch(key)
		{
			case 1:
				ioctl(fd_led,LED_ALL_ON);
				break;
			case 2:
				ioctl(fd_led,LED_ALL_OFF);		
				break;
			case 3:
				ioctl(fd_beep,BEEP_ON);		
				break;
			case 4:
				ioctl(fd_beep,BEEP_OFF);		
				break;
			default:
				break;

		}
*/

	
/*		printf("输入0控制蜂鸣器 输入1控制LED\r\n");
		scanf("%d",&type);
		if(type==0){
			fd=open("/dev/drv_beep",O_RDWR);
			while(1)
			{
				printf("输入0停止蜂鸣响 输入1控制蜂鸣响 其他退出\r\n");
				scanf("%d",&cmd);
				if(cmd==0)
					ioctl(fd,BEEP_OFF);
				if(cmd==1)
					ioctl(fd,BEEP_ON);
				else
					break;
			}
			close(fd);
		}
		else if(1==type){
			fd=open("/dev/drv_led",O_RDWR);
			for( i=0;i<10;i++)
			{
				ioctl(fd,LED_ALL_OFF);
				sleep(1);
				ioctl(fd,LED_NUM_ON,3);
				sleep(1);
				ioctl(fd,LED_NUM_ON,4);
				sleep(1);
				ioctl(fd,LED_NUM_OFF,3);
				sleep(1);
				ioctl(fd,LED_NUM_OFF,4);
				sleep(1);
				ioctl(fd,LED_ALL_ON);
				sleep(1);
			}
		}else
			break;
	*/

	}

	close(fd_key);
	close(fd_beep);
	close(fd_led);

	
}

