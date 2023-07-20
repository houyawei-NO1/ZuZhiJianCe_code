#include ".\comm\head.h"
void SetTimer2Baudraye(u32 dat);
/******************** ���ڴ�ӡ���� ********************/
u8  TX1_Cnt;    //���ͼ���
u8  RX1_Cnt;    //���ռ���

bit B_TX1_Busy; //����æ��־


u8  RX1_Buffer[UART1_BUF_LENGTH]; //���ջ���


void UartPutc(unsigned char dat)
{
	   SBUF = dat;
        B_TX1_Busy = 1;
        while(B_TX1_Busy);
}

char putchar(char c)
{
	UartPutc(c);
	return c;
}


//========================================================================
// ����: SetTimer2Baudraye(u32 dat)
// ����: ����Timer2�������ʷ�������
// ����: dat: Timer2����װֵ.
// ����: none.
// �汾: VER1.0
// ����: 2014-11-28
// ��ע: 
//========================================================================
void SetTimer2Baudraye(u32 dat)
{
    T2R = 0;		//Timer stop
    T2_CT = 0;	//Timer2 set As Timer
    T2x12 = 1;	//Timer2 set as 1T mode
    T2H = (u8)(dat / 256);
    T2L = (u8)(dat % 256);
    ET2 = 0;    //��ֹ��ʱ���ж�
    T2R = 1;		//Timer run enable
}

//========================================================================
// ����: void UART1_config(u8 brt)
// ����: UART1��ʼ��������
// ����: brt: ѡ������, 2: ʹ��Timer2��������, ����ֵ: ʹ��Timer1��������.
// ����: none.
// �汾: VER1.0
// ����: 2014-11-28
// ��ע: 
//========================================================================
void UART1_config(u8 brt)
{
    brt = 0;

    /*********** ������ʹ�ö�ʱ��2 *****************/
    if(brt == 2)
    {
        S1BRT = 1;	//S1 BRT Use Timer2;
        SetTimer2Baudraye(Baudrate1);
    }

    /*********** ������ʹ�ö�ʱ��1 *****************/
    else
    {
        TR1 = 0;
        S1BRT = 0;		//S1 BRT Use Timer1;
        T1_CT = 0;		//Timer1 set As Timer
        T1x12 = 1;		//Timer1 set as 1T mode
        TMOD &= ~0x30;//Timer1_16bitAutoReload;
        TH1 = (u8)(Baudrate1 / 256);
        TL1 = (u8)(Baudrate1 % 256);
        ET1 = 0;    //��ֹ��ʱ���ж�
        TR1 = 1;
    }
    /*************************************************/

    SCON = (SCON & 0x3f) | 0x40;    //UART1ģʽ, 0x00: ͬ����λ���, 0x40: 8λ����,�ɱ䲨����, 0x80: 9λ����,�̶�������, 0xc0: 9λ����,�ɱ䲨����
//  PS  = 1;    //�����ȼ��ж�
    ES  = 1;    //���������ж�
    REN = 1;    //��������
    P_SW1 &= 0x3f;
//  P_SW1 |= 0x00;      //UART1 switch to, 0x00: P3.0 P3.1, 0x40: P3.6 P3.7, 0x80: P1.6 P1.7, 0xC0: P4.3 P4.4

    B_TX1_Busy = 0;
    TX1_Cnt = 0;
    RX1_Cnt = 0;
}




//========================================================================
// ����: void UART1_int (void) interrupt UART1_VECTOR
// ����: UART1�жϺ�����
// ����: nine.
// ����: none.
// �汾: VER1.0
// ����: 2014-11-28
// ��ע: 
//========================================================================
void UART1_int (void) interrupt 4
{
    if(RI)
    {
        RI = 0;
        RX1_Buffer[RX1_Cnt] = SBUF;
        if(++RX1_Cnt >= UART1_BUF_LENGTH)   RX1_Cnt = 0;
    }

    if(TI)
    {
        TI = 0;
        B_TX1_Busy = 0;
    }
}

//========================================================================

