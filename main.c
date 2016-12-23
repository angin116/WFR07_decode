/*
  Ӳ����Э����ؽ����뿴�����̵�README��
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

unsigned char ch_num[7];	//�������ͨ����⵽�ߵ�ƽ����
unsigned char ch_mk[7];	// ����ͨ����������Ϣ����ΪҪ���͵�����
bool flag_ready = false;	//����׼���ñ�־
unsigned char step = 0;	// 0-��ǰ���ڲɼ�����ͨ���źš�
			//1-����ͨ���ź��Ѿ��ɼ���ɣ����ڵȴ��µ����ڵ�����

void sendB(unsigned char dat)
{
    SBUF=dat;
    while(!TI);
    TI=0;
}

void t0() interrupt 1	//20usһ���ж�
{
    static unsigned char over_num = 0;	//����������⵽����ͨ�����ǵ͵�ƽ�Ĵ���
#ifdef DEBUG
    test0 = 1;
#endif
    switch(step)
    {
    case 0:	//�ɼ��źţ�������һ��˵���Ѿ�����������һ·Ϊ�ߵ�ƽ
	//����ͨ������������ͨ���ĸߵ�ƽ�����γ��ֵģ�����ͬʱ����
	if(ch1)		{ch_num[0]++; over_num = 0;}
	else if(ch2)	{ch_num[1]++; over_num = 0;}
	else if(ch3)	{ch_num[2]++; over_num = 0;}
	else if(ch4)	{ch_num[3]++; over_num = 0;}
	else if(ch5)	{ch_num[4]++; over_num = 0;}
	else if(ch6)	{ch_num[5]++; over_num = 0;}
	else if(ch7)	{ch_num[6]++; over_num = 0;}
	else	//����ͨ������0���ܿ���������ͨ���źŽ���
	{
	    over_num ++;
	    if(over_num == 100)	//�����������ͨ�����ǵ͵�ƽ��ȷ�ϱ������źŽ���
				/*�����������Ϊ��ʶ�������źŽ�����һ�������ڽ���ǰ���нϳ�ʱ��ȫ��ͨ����Ϊ�͵�ƽ����WFR07����
				  �еĽ��ջ��ڸ���ͨ���ź������϶Ҳ����ֽϳ�ʱ�������ͨ���͵�ƽ���,��ʱ��Ҫ�Ӵ�����֣�
				  �Ѽ�϶֮��ĵ͵�ƽ�����ڽ���ǰ�ĵ͵�ƽ���ֿ�������WFR07���ջ���Ҫ��2ms���֡�*/
	    {
		unsigned char i;
		for(i=0; i<7; i++)	//������ͨ���Ƿ���С��500us�ĸߵ�ƽ����
		    if(ch_num[i] < 25)
			break;
		if(i == 7)	//����ͨ���źŶ���Ч
		    flag_ready = true;	//֪ͨ�����̷������ݣ��˱���������������
		step = 1;	//����ȴ��׶�
	    }
	}
	break;
    case 1:	//�ɼ���ɣ��ȴ��¸����ڵ���
	if(ch1 | ch2 | ch3 | ch4 | ch5 | ch6 | ch7)	//�Ѿ���⵽��ͨ��Ϊ�ߵ�ƽ���µ��źŵ���
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
    bool first_frame = true;	//��һ֡��־
    // ��ʼ���ж�
    ET0 = 1;
    ES=0;	//�ش����жϣ�����ѭ�����ò�ѯ��ʽ��������
    EA = 1;
    
    // ��ʼ�����ڣ�������115200
    PCON |= 0x80;		//ʹ�ܲ����ʱ���λSMOD
    SCON = 0x50;		//8λ����,�ɱ䲨����
    AUXR |= 0x40;		//��ʱ��1ʱ��ΪFosc,��1T
    AUXR &= 0xFE;		//����1ѡ��ʱ��1Ϊ�����ʷ�����
    TMOD &= 0x0F;		//�����ʱ��1ģʽλ
    TMOD |= 0x20;		//�趨��ʱ��1Ϊ8λ�Զ���װ��ʽ
    TL1 = 0xF3;		//�趨��ʱ��ֵ
    TH1 = 0xF3;		//�趨��ʱ����װֵ
    ET1 = 0;		//��ֹ��ʱ��1�ж�
    TR1 = 1;		//������ʱ��1

    // ��ʼ��T0���ж�����20us
    AUXR &= 0x7F;		//��ʱ��0������12Tģʽ
    TMOD &= 0xF0; TMOD |= 0x02;	//��ʱ��0������8Ϊ�Զ���װģʽ
    TL0 = 216;
    TH0 = 216;
    TF0 = 0;		//���TF0��־
    TR0 = 1;		//��ʱ��0��ʼ��ʱ

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
		//�����������Ϣת��������ֵ��������
		sendB(0x01);	//һ֡������ʼ��־
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
