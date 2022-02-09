#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include "operation.h"
#include <pthread.h>
int fanFlag;
int beepFlag;
int ledFlag;
int motorFlag;

struct ArgsFd{
	int fanFd;
	int beepFd;
	int spiFd;
	int i2cFd;
	int mtrFd;
	int ledFd;
	int *SMDflag;
	unsigned int *SMDnum;
};


void display()
{
	fanFlag = 0;
	beepFlag = 0;
	ledFlag = 0;
	motorFlag = 0;
	printf("******************************************\n");
	printf("***************智能家居系统***************\n");
	printf("***************1.led开关******************\n");
	printf("***************2.读取温湿度***************\n");
	printf("***************3.蜂鸣器开关***************\n");
	printf("***************4.风扇开关*****************\n");
	printf("***************5.马达开关*****************\n");
	printf("***************6.数码管显示*****************\n");
	printf("***************7.退出系统*****************\n");
	printf("******************************************\n");

}

void showLed(int ledFd)
{
	char choose;
	int data = 0;
	while (1)
	{
		printf("******************************************\n");
		printf("***************LED开关********************\n");
		printf("***************1.开***********************\n");
		printf("***************2.关***********************\n");
		printf("***************3.返回上一级***************\n");
		printf("******************************************\n");
		choose = getchar();
		while(getchar()!=10);
		switch (choose)
		{
			case  '1':
				data = 1;
				ledFlag = 1;
				ioctl(ledFd,LED1_OP,&data);
				break;
			case '2':
				data = 0;
				ioctl(ledFd,LED1_OP,&data);
				break;
			case '3':
				return;
			default:
				printf("输入有误,请重新输入\n");
				continue;
		}
	}
}

void showBeep(int beepFd)
{
	char choose;
	int data = 0;
	while (1)
	{
		printf("******************************************\n");
		printf("***************蜂鸣器开关********************\n");
		printf("***************1.开***********************\n");
		printf("***************2.关***********************\n");
		printf("***************3.返回上一级***************\n");
		printf("******************************************\n");
		choose = getchar();
		while(getchar()!=10);
		switch (choose)
		{
			case  '1':
				data = 1;
				beepFlag = 1;
				ioctl(beepFd,BEEP_OP,&data);
				break;
			case '2':
				data = 0;
				ioctl(beepFd,BEEP_OP,&data);
				break;
			case '3':
				return;
			default:
				printf("输入有误,请重新输入\n");
				continue;
		}
	}
}

void showFan(int fanFd)
{
	char choose;
	int data = 0;
	while (1)
	{
		printf("******************************************\n");
		printf("***************风扇开关********************\n");
		printf("***************1.开***********************\n");
		printf("***************2.关***********************\n");
		printf("***************3.返回上一级***************\n");
		printf("******************************************\n");
		choose = getchar();
		while(getchar()!=10);
		switch (choose)
		{
			case  '1':
				data = 1;
				fanFlag = 1;
				ioctl(fanFd,FAN_OP,&data);
				break;
			case '2':
				data = 0;
				ioctl(fanFd,FAN_OP,&data);
				break;
			case '3':
				return;
			default:
				printf("输入有误,请重新输入\n");
				continue;
		}
	}
}

void showMotor(int mtrFd)
{
	char choose;
	int data = 0;
	while (1)
	{
		printf("******************************************\n");
		printf("***************马达开关********************\n");
		printf("***************1.开***********************\n");
		printf("***************2.关***********************\n");
		printf("***************3.返回上一级***************\n");
		printf("******************************************\n");
		choose = getchar();
		while(getchar()!=10);
		switch (choose)
		{
			case  '1':
				motorFlag = 1;
				ioctl(mtrFd,0,0);
				break;
			case '2':
				data = 0;
				ioctl(mtrFd,1,0);
				break;
			case '3':
				return;
			default:
				printf("输入有误,请重新输入\n");
				continue;
		}
	}
}

void showSMD(int *SMDflag,unsigned int *SMDnum)
{
	char choose;
	while (1)
	{
		printf("******************************************\n");
		printf("***************数码管显示*****************\n");
		printf("***************1.显示温度*****************\n");
		printf("***************2.显示湿度*****************\n");
		printf("***************3.显示温湿度***************\n");
		printf("***************4.显自定义数字*************\n");
		printf("***************5.返回上一级***************\n");
		printf("******************************************\n");
		choose = getchar();
		while(getchar()!=10);
		switch (choose)
		{
			case  '1':
				*SMDflag = 1;
				break;
			case '2':
				*SMDflag = 2;
				break;
			case '3':
				*SMDflag = 3;
				break;
			case '4':
				*SMDflag = 4;
				printf("输入一个数字\n");
				scanf("%u",SMDnum);
				getchar();
				break;
			case '5':
				return;
			default:
				printf("输入有误,请重新输入\n");
				continue;
		}
	}
}

//显示数码管
void * showDigital(void * arg)
{
	struct ArgsFd args = *((struct ArgsFd*)arg);

	unsigned long count = 0;
	char data[2] = {0};
	unsigned short val;

	int on = 1;
	int off = 0;
	float tem,hum;
	unsigned short th;
	while(1)
	{
		if(count == 0)
		{
			count = 2500;
			ioctl(args.i2cFd,Temperature,data);
	
			val = data[1];
			val = val << 8 | data[0];
			tem = (175.72 * val) / 65536 - 46.85;
			hum = (125 * val) / 65536 - 6;

			th = (unsigned short)tem;
			th = th*100;
			th += (unsigned short)hum;
			if(tem > 30||hum > 50)
			{
				ioctl(args.fanFd,0,&on);
				ioctl(args.mtrFd,0,0);
				ioctl(args.beepFd,BEEP_OP,&on);
				ioctl(args.ledFd,LED1_OP,&on);
			}
			else if(tem < 30&&hum < 50)
			{
				if(!fanFlag)
				{
					ioctl(args.fanFd,1,&off);
				}
				if(!motorFlag)
				{
					ioctl(args.mtrFd,1,0);
				}
				if(!beepFlag)
				{
					ioctl(args.beepFd,BEEP_OP,&off);
				}
				if(!ledFlag)
				{
					ioctl(args.ledFd,LED1_OP,&off);
				}
			}
		}
		count--;
		if(*args.SMDflag == 1)
		{
			ioctl(args.spiFd,1,(unsigned int)(tem*100));
		}
		else if(*args.SMDflag == 2)
		{
			ioctl(args.spiFd,1,(unsigned int)(hum*100));
		}
		else if(*args.SMDflag == 3)
		{
			ioctl(args.spiFd,0,th);
		}
		else if(*args.SMDflag == 4)
		{
			ioctl(args.spiFd,0,*args.SMDnum);
		}
	}

	pthread_exit(NULL);

}
void showTemAndHum(int i2cFd)
{
	char data[2] = {0};
	unsigned short val;
	float tem,hum;
	ioctl(i2cFd,Temperature,data);
	val = data[1];
	val = val << 8 | data[0];
	tem = (175.72 * val) / 65536 - 46.85;
	hum = (125 * val) / 65536 - 6;
	printf("TEM = %f,HUM = %f\n",tem,hum);


}


int main(int argc, const char *argv[])
{

	system("insmod MyFan.ko");
	system("insmod MyBeep.ko");
	system("insmod Myled.ko");
	system("insmod si7006.ko");
	system("insmod m74hc595.ko");
	system("insmod Motor.ko");
	int ledFd = open("/dev/myled",O_RDWR);
	if(ledFd<0)
	{
		perror("open myled");
	}
	int beepFd = open("/dev/mybeep",O_RDWR);
	if(beepFd<0)
	{
		perror("open mybeep");
	}
	int fanFd = open("/dev/myfan",O_RDWR);
	if(fanFd<0)
	{
		perror("open myfan");
	}
	int i2cFd;
	if((i2cFd = open("/dev/mysi7006",O_RDWR)) < 0){
		perror("open error");
	}
	int spiFd;
	if((spiFd = open("/dev/m74hc595",O_RDWR)) < 0){
		perror("open error");
	}
	int mtrFd;
	if((mtrFd = open("/dev/myMotor",O_RDWR)) < 0){
		perror("open error");
	}
	
	int SMDflag = 3;
	unsigned int SMDnum = 0;

	struct ArgsFd args;
	args.fanFd = fanFd;
	args.beepFd = beepFd;
	args.i2cFd = i2cFd;
	args.ledFd = ledFd;
	args.spiFd = spiFd;
	args.mtrFd = mtrFd;
	args.SMDflag = &SMDflag;
	args.SMDnum = &SMDnum;


	pthread_t pid;
	pthread_create(&pid,NULL,showDigital,&args);

	char choose;
     while(1)
	 {
		
	 	display();
		choose = getchar();
		while(getchar()!=10);
		switch (choose)
		{
			case  '1':
				showLed(ledFd);
				break;
			case  '2':
				showTemAndHum(i2cFd);
				break;
			case  '3':
				showBeep(beepFd);
				break;
			case '4':
				showFan(fanFd);
				break;
			case '5':
				showMotor(mtrFd);
				break;
			case '6':
				showSMD(&SMDflag,&SMDnum);
				break;
			case '7':
				goto end;
			default:
				printf("输入有误,请重新输入\n");
				continue;
		}
	 }
end:
	close(ledFd);
	close(i2cFd);
	close(spiFd);
	close(mtrFd);
	close(beepFd);
	close(fanFd);
	pthread_detach(pid);
	system("rmmod MyFan.ko");
	system("rmmod MyBeep.ko");
	system("rmmod Myled.ko");
	system("rmmod si7006.ko");
	system("rmmod m74hc595.ko");
	system("rmmod Motor.ko");
	return 0;
}
