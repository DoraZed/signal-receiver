#include<ioCC2530.h>
#include"SD_Drive.h"
#include"74LS164_8LED.h"
extern char cycle,amplitude,mode,most;
char SendPacket[]={0x0f,0x61,0x88,0x00,0x90,0xEB,0x18,0x55,0x18,0x54,0x0c,0x02,0x00,0x55};
char zhi;
//��һ���ֽ�0x0C���壬����Լ����滹��12���ֽ�Ҫ����
//��5 6���ֽڱ�ʾ����PANID
//��7 8���ֽ�������ģ��Ŀ���豸�������ַ 0xEB90
//��9 10���Ǳ���ģ��������ַ
//11 ���ֽ����������õ�����
// CRC�� 12 13���ֽ� ��Ӳ���Զ�׷��
//��һλ=0x0b+�ҵ����ݳ���
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
    FRMCTRL0 |= 0x60;   //  �Ƽ���Ƶ��������
    TXFILTCFG = 0x09;
    AGCCTRL1 = 0x15;
    FSCAL1 = 0x00;
    // enable RXPKTDONE interrupt  
    RFIRQM0 |= 0x40;
    // enable general RF interrupts
    IEN2 |= 0x01; 
//���ù����ŵ�
      FREQCTRL =(11+(25-11)*5);//(MIN_CHANNEL + (channel - MIN_CHANNEL) * CHANNEL_SPACING);    
//����PANID,������ID�����ڷ���ģ��ͽ���ģ�鶼��ִ��������������Ժ���Ȼ���ǵĸ�����ID��һ���ģ��ŵ�Ҳ��һ����
      PAN_ID0=0x90;
      PAN_ID1=0xEB;       
//halRfRxInterruptConfig(basicRfRxFrmDoneIsr);    
    RFST = 0xEC;//����ջ�����
    RFST = 0xE3;//��������ʹ�� 
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
 
    len=RFD;//����һ���ֽ��ж���һ�����ݺ����м����ֽڣ�
    if(len==15)
      //3:����   4:���   5:�����ģʽ/����ģʽѡ��   6:��������� (��������ģʽ��Ч)
    {
        while (len>0) 
        {//ֻҪ���滹��������ô�Ͱ������ӽ��ܻ�����ȡ����
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
RFST = 0xEC;//����ջ�����   
RFST = 0xE3;//��������ʹ�� 
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
    RFST = 0xEC; //ȷ�������ǿյ�
    RFST = 0xE3; //����ձ�־λ
    while (FSMSTAT1 & 0x22);//�ȴ���Ƶ����׼����
    RFST = 0xEE;//ȷ�����Ͷ����ǿ�
    RFIRQF1 &= ~0x02;//�巢�ͱ�־λ
//Ϊ���ݷ�������׼������

    for(i=0;i<len;i++)
    {
       RFD=pstr[i];
    }  //ѭ���������ǰ�����Ҫ���͵�����ȫ��ѹ�����ͻ���������
    
    RFST = 0xE9; //����Ĵ���һ��������Ϊ0xE9,���ͻ����������ݾͱ����ͳ�ȥ
    while(!(RFIRQF1 & 0x02) );//�ȴ��������
    RFIRQF1 = ~0x02;//�巢����ɱ�־
}
void InitT3()
{    
    EA=0;
    T3CC0=62;  
    T3CTL  |= 0x08 ;          //������ж�     
    T3IE = 1;                //�����жϺ�T3�ж�
    T3CTL |= 0xE0;           //128��Ƶ,128/16000000*N=0.001S,N=125  1110 0000  1/16M*128=8US
    T3CTL |= 0x03;          //�Զ���װ 00��>T3CC0  1111 1100
    T3CTL |= 0x10;           //����
    EA = 1;                  //�����ж�
}
void DriveCfg()
{
//     LEDs_Cfg();
      P0SEL=0x00;//P0��ͨIO��ģʽ
      P1SEL&=~0x01;//P1_0��LED0��  0000 0001   //ָʾ��
      P1DIR|=0x01;//���
//     Bee_Cfg();
//     LxChangR();
//     KeysIntCfg();
//     Uart0_Cfg(); 
     halRfInit();
     InitT3();
}