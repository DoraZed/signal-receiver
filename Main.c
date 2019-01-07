#include<ioCC2530.h>
#include"SD_Drive.h"
#include"74LS164_8LED.h"
char cycle=12,amplitude=2,mode=0,most=80; 
//   3:����   4:���   5:�����ģʽ/����ģʽѡ��   
//   6:��������� (��������ģʽ��Ч)
uint xiqi_time=2000,huqi_time=5000,maijian=80;
uint count;             //���ڶ�ʱ������
char maikuan;
extern char SendPacket[];
void Init32M()
{
	SLEEPCMD &=0xFB;//1111 1011 ����2����Ƶʱ��Դ
	while(0==(SLEEPSTA & 0x40));// 0100 0000 �ȴ�32M�ȶ�
	Delay();
	CLKCONCMD &=0xF8;//1111 1000 ����Ƶ���
	CLKCONCMD &=0XBF;//1011 1111 ����32M��Ϊϵͳ��ʱ��
	while(CLKCONSTA & 0x40); //0100 0000 �ȴ�32M�ɹ���Ϊ��ǰϵͳ��ʱ��
}
void Init_Watchdog(void) 
{ 
    WDCTL = 0x00;       //��IDLE�������ÿ��Ź�
    WDCTL |= 0x08;      //��ʱ�����ѡ��,���һ��
}

void FeedDog(void) 
{ 
    WDCTL = 0xa0;       //�����ʱ������0xA����0x5д����Щλ����ʱ�������
    WDCTL = 0x50; 
}

void Delaymaikuan()
{
    int x;
    for(x=255;x>0;x--);
}
void main()
{
    Init32M(); //��ʱ�Ӿ�������32M
    Init_Watchdog(); //���Ź���ʼ��
    halRfInit();
    DriveCfg();
   
    SHORT_ADDR0=0x18;
    SHORT_ADDR1=0x54;//���ñ�ģ���ַ  0x5418

    LS164_BYTE(1); 
    
    maikuan=0;
    
        //////////����ǿ��ѡ��IO�� amplitude��Ϊ4���ȼ�/////////
        switch(amplitude)
        {
            case 1 :
                P0DIR=0x80;P0INP=0xff;//P07���,����Ϊ����
            break;
            case 2 :
                P0DIR=0x40;P0INP=0xff;//P06���,����Ϊ���� 
            break;
            case 3 :
                P0DIR=0x20;P0INP=0xff;//P05���,����Ϊ����
            break;
            case 4 :
                P0DIR=0x10;P0INP=0xff;//P04���,����Ϊ���� 
            break;
        }
        ////////////////ѡ�񴥷�ģʽΪ��Ƶ���߱�Ƶ//////////////
        switch(mode)
        {
            case 0 :
                maijian=most;//40<most<130
            break;
            case 1 :
                maijian=most;//P06���,����Ϊ���� 
            break;
        }
        
    while(1)
    {
      if(maikuan==1)
      {
        P0=0xff; 
        Delaymaikuan();
        P0=0x00;
        maikuan=0;
      }
      FeedDog();
    }

}

//��ʱ��T3�жϴ�����
#pragma vector = T3_VECTOR 
__interrupt void T3_ISR(void) 
{ 
    IRCON = 0x00;            //���жϱ�־, Ҳ����Ӳ���Զ���� 
    
    if(mode==0)         //��Ƶ
    {
        if(count++ < xiqi_time)        //���������� 
        {                        
            if((count%maijian)==0)
            {
              maikuan=1;
            }
        }
        else if(count < huqi_time)
        {
        }
        else 
        {
            count = 0; 
        }
    }
}
#pragma vector=RF_VECTOR                      //��Ƶ�жϺ���
__interrupt void RF_IRQ(void)
{
    EA=0;
    if( RFIRQF0 & 0x40 )
    {
        RevRFProc();
        //////////////////���� ����ʱ��ȷ��/////////////////////
        xiqi_time=24000/cycle;//  60000ms/����*(2/5)   1:1.5=2:3
        huqi_time=60000/cycle;//  60000ms/����   1:1.5=2:3
        //////////����ǿ��ѡ��IO�� amplitude��Ϊ4���ȼ�/////////
        switch(amplitude)
        {
        case 1 :
          P0DIR=0x80;P0INP=0xff;//P07���,����Ϊ����
        break;
        case 2 :
          P0DIR=0x40;P0INP=0xff;//P06���,����Ϊ���� 
        break;
        case 3 :
          P0DIR=0x20;P0INP=0xff;//P05���,����Ϊ����
        break;
        case 4 :
          P0DIR=0x10;P0INP=0xff;//P04���,����Ϊ���� 
        break;
        }
        ////////////////ѡ�񴥷�ģʽΪ��Ƶ���߱�Ƶ//////////////
        switch(mode)
        {
        case 0 :
          maijian=most;//40<most<130
        break;
        case 1 :
          maijian=most;//
        break;
        }
        RFIRQF0 &= ~0x40;   // Clear RXPKTDONE interrupt
    }
    S1CON= 0;                   // Clear general RF interrupt flag
    RFST = 0xEC;//����ջ�����
    RFST = 0xE3;//��������ʹ�� 
    EA=1;
}