/*
  硬件和协议相关介绍请看本工程的README。
 */

#include "12C5A60S2.h"

//#define DEBUG
#ifdef DEBUG
sbit test0 = P2^0;
sbit test1 = P2^1;
#endif
sbit ch1 = P1^6;
sbit ch2 = P1^5;
sbit ch3 = P1^4;
sbit ch4 = P1^3;
sbit ch5 = P1^2;
sbit ch6 = P1^1;
sbit ch7 = P1^0;

typedef enum {
    false=0,
    true
} bool;

unsigned char ch_num[7];	//保存各个通道检测到高电平次数
unsigned char ch_mk[7];	// 三个通道的脉宽信息，此为要发送的数据
bool flag_ready = false;	//数据准备好标志
unsigned char step = 0;	// 0-当前正在采集各个通道信号。
			//1-所有通道信号已经采集完成，正在等待新的周期到来。

void sendB(unsigned char dat)
{
    SBUF=dat;
    while(!TI);
    TI=0;
}

void t0() interrupt 1	//20us一次中断
{
    static unsigned char over_num = 0;	//保存连续检测到所有通道都是低电平的次数
#ifdef DEBUG
    test0 = 1;
#endif
    switch(step)
    {
    case 0:	//采集信号，进入这一步说明已经出现至少有一路为高电平
	//各个通道计数，各个通道的高电平是依次出现的，不会同时出现
	if(ch1)		{ch_num[0]++; over_num = 0;}
	else if(ch2)	{ch_num[1]++; over_num = 0;}
	else if(ch3)	{ch_num[2]++; over_num = 0;}
	else if(ch4)	{ch_num[3]++; over_num = 0;}
	else if(ch5)	{ch_num[4]++; over_num = 0;}
	else if(ch6)	{ch_num[5]++; over_num = 0;}
	else if(ch7)	{ch_num[6]++; over_num = 0;}
	else	//三个通道都是0，很可能是所有通道信号结束
	{
	    over_num ++;
	    if(over_num == 100)	//连续多次所有通道都是低电平，确认本周期信号结束
				/*这里的数字是为了识别本周期信号结束，一般在周期结束前会有较长时间全部通道都为低电平（如WFR07）。
				  有的接收机在各个通道信号输出间隙也会出现较长时间的所有通道低电平情况,此时需要加大此数字，
				  把间隙之间的低电平和周期结束前的低电平区分开，对于WFR07接收机需要用2ms区分。*/
	    {
		unsigned char i;
		for(i=0; i<7; i++)	//检查各个通道是否有小于500us的高电平脉宽
		    if(ch_num[i] < 25)
			break;
		if(i == 7)	//所有通道信号都有效
		    flag_ready = true;	//通知主进程发送数据，此变量由主进程清零
		step = 1;	//进入等待阶段
	    }
	}
	break;
    case 1:	//采集完成，等待下个周期到来
	if(ch1 | ch2 | ch3 | ch4 | ch5 | ch6 | ch7)	//已经检测到有通道为高电平，新的信号到来
	{
	    unsigned char i;
	    step = 0;
	    over_num = 0;
	    for(i=0;i<7;i++)
		ch_num[i]=0;
	    if(ch1) ch_num[0]++;
	    else if(ch2) ch_num[1]++;
	    else if(ch3) ch_num[2]++;
	    else if(ch4) ch_num[3]++;
	    else if(ch5) ch_num[4]++;
	    else if(ch6) ch_num[5]++;
	    else if(ch7) ch_num[6]++;
	}
	break;
    }
#ifdef DEBUG
    test0 = 0;
#endif
}

void main()
{
    bool first_frame = true;	//第一帧标志
    // 初始化中断
    ET0 = 1;
    ES=0;	//关串口中断，在主循环中用查询方式发送数据
    EA = 1;
    
    // 初始化串口，波特率115200
    PCON |= 0x80;		//使能波特率倍速位SMOD
    SCON = 0x50;		//8位数据,可变波特率
    AUXR |= 0x40;		//定时器1时钟为Fosc,即1T
    AUXR &= 0xFE;		//串口1选择定时器1为波特率发生器
    TMOD &= 0x0F;		//清除定时器1模式位
    TMOD |= 0x20;		//设定定时器1为8位自动重装方式
    TL1 = 0xF3;		//设定定时初值
    TH1 = 0xF3;		//设定定时器重装值
    ET1 = 0;		//禁止定时器1中断
    TR1 = 1;		//启动定时器1

    // 初始化T0，中断周期20us
    AUXR &= 0x7F;		//定时器0工作在12T模式
    TMOD &= 0xF0; TMOD |= 0x02;	//定时器0工作在8为自动填装模式
    TL0 = 216;
    TH0 = 216;
    TF0 = 0;		//清除TF0标志
    TR0 = 1;		//定时器0开始计时

    while(1)
    {
	if(flag_ready)
	{
#ifdef DEBUG
	    test1 = ~test1;
#endif
	    if(first_frame)
	    {
		first_frame = false;
		flag_ready = false;
	    }
	    else
	    {
		unsigned char i;
		//把脉宽个数信息转换成脉宽值，并发送
		sendB(0x01);	//一帧数据起始标志
		for(i=0;i<7;i++)
		{
		    ch_mk[i] = ch_num[i] * 2;
		    sendB(ch_mk[i]);
		}
		flag_ready = false;
	    }
	}
    }
}
