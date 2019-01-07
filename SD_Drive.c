#include<ioCC2530.h>
#include"SD_Drive.h"
#include"74LS164_8LED.h"
extern char cycle,amplitude,mode,most;
char SendPacket[]={0x0f,0x61,0x88,0x00,0x90,0xEB,0x18,0x55,0x18,0x54,0x0c,0x02,0x00,0x55};
char zhi;
//第一个字节0x0C含义，这个自己后面还有12个字节要发送
//第5 6个字节表示的是PANID
//第7 8个字节是无线模块目标设备的网络地址 0xEB90
//第9 10就是本地模块的网络地址
//11 个字节是我们有用的数据
// CRC码 12 13个字节 是硬件自动追加
//第一位=0x0b+我的数据长度
char *pSendPacket=SendPacket;

void halRfInit(void);
void RevRFProc();
void Delay()
{
    int y,x;
    for(y=1000;y>0;y--)
      for(x=40;x>0;x--);
}
void halRfInit(void)
{
   EA=0;
    FRMCTRL0 |= 0x60;   //  推荐射频接收设置
    TXFILTCFG = 0x09;
    AGCCTRL1 = 0x15;
    FSCAL1 = 0x00;
    // enable RXPKTDONE interrupt  
    RFIRQM0 |= 0x40;
    // enable general RF interrupts
    IEN2 |= 0x01; 
//设置工作信道
      FREQCTRL =(11+(25-11)*5);//(MIN_CHANNEL + (channel - MIN_CHANNEL) * CHANNEL_SPACING);    
//设置PANID,个域网ID，由于发送模块和接受模块都会执行这个函数，所以很显然他们的个域网ID是一样的，信道也是一样的
      PAN_ID0=0x90;
      PAN_ID1=0xEB;       
//halRfRxInterruptConfig(basicRfRxFrmDoneIsr);    
    RFST = 0xEC;//清接收缓冲器
    RFST = 0xE3;//开启接收使能 
    EA=1;    
}
void RevRFProc()
{
    static char len;
    static char ch;
    len=ch=0;
    RFIRQM0 &= ~0x40;
    IEN2 &= ~0x01;
    EA=1;
 
    len=RFD;//读第一个字节判断这一串数据后面有几个字节；
    if(len==15)
      //3:周期   4:振幅   5:定间隔模式/变间隔模式选择   6:最大脉冲间隔 (变脉冲间隔模式有效)
    {
        while (len>0) 
        {//只要后面还有数据那么就把他都从接受缓冲区取出来
            ch=RFD;
               switch(len)
              {
              case 6 :
                if((ch>=6)&&(ch<=24))cycle=ch;
              break;
              case 5 :
                if((ch>=1)&&(ch<=4))amplitude=ch; 
              break;
              case 4 :
               if((ch==0)||(ch==1)) mode=ch;
              break;
              case 3 :
                if((ch>=40)&&(ch<=130))
                {
                    most=ch; 
                    //SendPacket[10]=1;
                    //SendPacket[11]=1;
                    //SendPacket[12]=1;
                    //SendPacket[13]=1;
                    //RFSend(SendPacket,14);
                }
              break;
              }
            len--;
         } 
        
    }
else 
{
RFST = 0xEC;//清接收缓冲器   
RFST = 0xE3;//开启接收使能 
}
    EA=0;
    // enable RXPKTDONE interrupt
    RFIRQM0 |= 0x40;
    // enable general RF interrupts
    IEN2 |= 0x01;        
}

void RFSend(char *pstr,char len)
{
  char i;
    RFST = 0xEC; //确保接收是空的
    RFST = 0xE3; //清接收标志位
    while (FSMSTAT1 & 0x22);//等待射频发送准备好
    RFST = 0xEE;//确保发送队列是空
    RFIRQF1 &= ~0x02;//清发送标志位
//为数据发送做好准备工作

    for(i=0;i<len;i++)
    {
       RFD=pstr[i];
    }  //循环的作用是把我们要发送的数据全部压到发送缓冲区里面
    
    RFST = 0xE9; //这个寄存器一旦被设置为0xE9,发送缓冲区的数据就被发送出去
    while(!(RFIRQF1 & 0x02) );//等待发送完成
    RFIRQF1 = ~0x02;//清发送完成标志
}
void InitT3()
{    
    EA=0;
    T3CC0=62;  
    T3CTL  |= 0x08 ;          //开溢出中断     
    T3IE = 1;                //开总中断和T3中断
    T3CTL |= 0xE0;           //128分频,128/16000000*N=0.001S,N=125  1110 0000  1/16M*128=8US
    T3CTL |= 0x03;          //自动重装 00－>T3CC0  1111 1100
    T3CTL |= 0x10;           //启动
    EA = 1;                  //开总中断
}
void DriveCfg()
{
//     LEDs_Cfg();
      P0SEL=0x00;//P0普通IO口模式
      P1SEL&=~0x01;//P1_0（LED0）  0000 0001   //指示灯
      P1DIR|=0x01;//输出
//     Bee_Cfg();
//     LxChangR();
//     KeysIntCfg();
//     Uart0_Cfg(); 
     halRfInit();
     InitT3();
}