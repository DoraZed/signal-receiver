#include<ioCC2530.h>
#include"SD_Drive.h"
#include"74LS164_8LED.h"
char cycle=12,amplitude=2,mode=0,most=80; 
//   3:周期   4:振幅   5:定间隔模式/变间隔模式选择   
//   6:最大脉冲间隔 (变脉冲间隔模式有效)
uint xiqi_time=2000,huqi_time=5000,maijian=80;
uint count;             //用于定时器计数
char maikuan;
extern char SendPacket[];
void Init32M()
{
	SLEEPCMD &=0xFB;//1111 1011 开启2个高频时钟源
	while(0==(SLEEPSTA & 0x40));// 0100 0000 等待32M稳定
	Delay();
	CLKCONCMD &=0xF8;//1111 1000 不分频输出
	CLKCONCMD &=0XBF;//1011 1111 设置32M作为系统主时钟
	while(CLKCONSTA & 0x40); //0100 0000 等待32M成功成为当前系统主时钟
}
void Init_Watchdog(void) 
{ 
    WDCTL = 0x00;       //打开IDLE才能设置看门狗
    WDCTL |= 0x08;      //定时器间隔选择,间隔一秒
}

void FeedDog(void) 
{ 
    WDCTL = 0xa0;       //清除定时器。当0xA跟随0x5写到这些位，定时器被清除
    WDCTL = 0x50; 
}

void Delaymaikuan()
{
    int x;
    for(x=255;x>0;x--);
}
void main()
{
    Init32M(); //主时钟晶振工作在32M
    Init_Watchdog(); //看门狗初始化
    halRfInit();
    DriveCfg();
   
    SHORT_ADDR0=0x18;
    SHORT_ADDR1=0x54;//设置本模块地址  0x5418

    LS164_BYTE(1); 
    
    maikuan=0;
    
        //////////根据强度选择IO口 amplitude分为4个等级/////////
        switch(amplitude)
        {
            case 1 :
                P0DIR=0x80;P0INP=0xff;//P07输出,其余为输入
            break;
            case 2 :
                P0DIR=0x40;P0INP=0xff;//P06输出,其余为输入 
            break;
            case 3 :
                P0DIR=0x20;P0INP=0xff;//P05输出,其余为输入
            break;
            case 4 :
                P0DIR=0x10;P0INP=0xff;//P04输出,其余为输入 
            break;
        }
        ////////////////选择触发模式为定频或者变频//////////////
        switch(mode)
        {
            case 0 :
                maijian=most;//40<most<130
            break;
            case 1 :
                maijian=most;//P06输出,其余为输入 
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

//定时器T3中断处理函数
#pragma vector = T3_VECTOR 
__interrupt void T3_ISR(void) 
{ 
    IRCON = 0x00;            //清中断标志, 也可由硬件自动完成 
    
    if(mode==0)         //定频
    {
        if(count++ < xiqi_time)        //定义脉冲间隔 
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
#pragma vector=RF_VECTOR                      //射频中断函数
__interrupt void RF_IRQ(void)
{
    EA=0;
    if( RFIRQF0 & 0x40 )
    {
        RevRFProc();
        //////////////////吸气 呼气时间确定/////////////////////
        xiqi_time=24000/cycle;//  60000ms/周期*(2/5)   1:1.5=2:3
        huqi_time=60000/cycle;//  60000ms/周期   1:1.5=2:3
        //////////根据强度选择IO口 amplitude分为4个等级/////////
        switch(amplitude)
        {
        case 1 :
          P0DIR=0x80;P0INP=0xff;//P07输出,其余为输入
        break;
        case 2 :
          P0DIR=0x40;P0INP=0xff;//P06输出,其余为输入 
        break;
        case 3 :
          P0DIR=0x20;P0INP=0xff;//P05输出,其余为输入
        break;
        case 4 :
          P0DIR=0x10;P0INP=0xff;//P04输出,其余为输入 
        break;
        }
        ////////////////选择触发模式为定频或者变频//////////////
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
    RFST = 0xEC;//清接收缓冲器
    RFST = 0xE3;//开启接收使能 
    EA=1;
}