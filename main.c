#include ".\comm\STC32G.h"
#include "intrins.h"
#include "stdio.h"

typedef 	unsigned char	u8;
typedef 	unsigned int	u16;
typedef 	unsigned long	u32;


#define MAIN_Fosc     22118400L  //������ʱ��
#define Baudrate      115200L
#define TM            (65536 -(MAIN_Fosc/Baudrate/4))
#define Major_Ver      0
#define Minor_Ver      1


/*************	���س�������	**************/

#define	ADC_SPEED	15		/* 0~15, ADCת��ʱ��(CPUʱ����) = (n+1)*32  ADCCFG */
#define	RES_FMT		(1<<5)	/* ADC�����ʽ 0: �����, ADC_RES: D11 D10 D9 D8 D7 D6 D5 D4, ADC_RESL: D3 D2 D1 D0 0 0 0 0 */
							/* ADCCFG      1: �Ҷ���, ADC_RES: 0 0 0 0 D11 D10 D9 D8, ADC_RESL: D7 D6 D5 D4 D3 D2 D1 D0 */

/*************	���ر�������	**************/


/*************	���غ�������	**************/

void	delay_ms(u8 ms);
void	ADC_convert(u8 chn);	//chn=0~7��ӦP1.0~P1.7, chn=8~14��ӦP0.0~P0.6, chn=15��ӦBandGap��ѹ
u16	Get_ADC12bitResult(u8 channel);

/******************** ���ڴ�ӡ���� ********************/
void UartInit(void)
{
	S2_S = 1;       //UART2 switch to: 0: P1.0 P1.1,  1: P4.6 P4.7
    S2CFG |= 0x01;  //ʹ�ô���2ʱ��W1λ��������Ϊ1��������ܻ��������Ԥ�ڵĴ���
	S2CON = (S2CON & 0x3f) | 0x40; 
	T2L  = TM;
	T2H  = TM>>8;
	AUXR |= 0x14;	      //��ʱ��2ʱ��1Tģʽ,��ʼ��ʱ
}

void UartPutc(unsigned char dat)
{
	S2BUF  = dat; 
	while(S2TI == 0);
	S2TI = 0;    //Clear Tx flag
}

char putchar(char c)
{
	UartPutc(c);
	return c;
}

/**********************************************/
void main(void)
{
	u8	i;

    WTST = 0;  //���ó���ָ����ʱ��������ֵΪ0�ɽ�CPUִ��ָ����ٶ�����Ϊ���
    EAXFR = 1; //��չ�Ĵ���(XFR)����ʹ��
    CKCON = 0; //��߷���XRAM�ٶ�

    P0M1 = 0x7f;   P0M0 = 0x00;   //����Ϊ��������
    P1M1 = 0xfb;   P1M0 = 0x00;   //����Ϊ��������
    P2M1 = 0x3c;   P2M0 = 0x3c;   //����P2.2~P2.5Ϊ©����·(ʵ��������������赽3.3V)
    P3M1 = 0x50;   P3M0 = 0x50;   //����P3.4��P3.6Ϊ©����·(ʵ��������������赽3.3V)
    P4M1 = 0x3c;   P4M0 = 0x3c;   //����P4.2~P4.5Ϊ©����·(ʵ��������������赽3.3V)
    P5M1 = 0x1c;   P5M0 = 0x0c;   //����P5.2��P5.3Ϊ©����·(ʵ��������������赽3.3V)������P5.4Ϊ��������
    P6M1 = 0xff;   P6M0 = 0xff;   //����Ϊ©����·(ʵ��������������赽3.3V)
    P7M1 = 0x00;   P7M0 = 0x00;   //����Ϊ׼˫���

	ADCTIM = 0x3f;  //����ͨ��ѡ��ʱ�䡢����ʱ�䡢����ʱ��
	ADCCFG = RES_FMT + ADC_SPEED;
	//ADCģ���Դ�򿪺���ȴ�1ms��MCU�ڲ�ADC��Դ�ȶ����ٽ���ADת��
	ADC_CONTR = 0x80 + 0;	//ADC on + channel

	UartInit();
	EA = 1;
	printf("�϶�����������ֵ��⣬�汾�ţ�Ver%02d.%02d!\r\n",Major_Ver,Minor_Ver);

	while (1)
	{
		for(i=0; i<16; i++)
		{
			delay_ms(200);
			//ADC_convert(3);		//���͹̶�ͨ��ADֵ

			ADC_convert(i);		//������ѯͨ��ADֵ
			if((i & 7) == 7)	//�����д�ӡ
			{
				printf("\r\n");
			}
		}
	}
}


//========================================================================
// ����: u16 Get_ADC12bitResult(u8 channel))	//channel = 0~15
// ����: ��ѯ����һ��ADC���.
// ����: channel: ѡ��Ҫת����ADC, 0~15.
// ����: 12λADC���.
// �汾: V1.0, 2016-4-28
//========================================================================
u16	Get_ADC12bitResult(u8 channel)	//channel = 0~15
{
	ADC_RES = 0;
	ADC_RESL = 0;

    ADC_CONTR = (ADC_CONTR & 0xf0) | channel; //����ADCת��ͨ��
    ADC_START = 1;//����ADCת��
    _nop_();
	_nop_();
	_nop_();
	_nop_();
    while(ADC_FLAG == 0);   //wait for ADC finish
    ADC_FLAG = 0;     //���ADC������־
	return (((u16)ADC_RES << 8) | ADC_RESL);
}

#define		SUM_LENGTH	16	/* ƽ��ֵ�������� ���ֵ16 */

/***********************************
��ѯ��ʽ��һ��ADC, chnΪͨ����, chn=0~7��ӦP1.0~P1.7, chn=8~14��ӦP0.0~P0.6, chn=15��ӦBandGap��ѹ.
***********************************/
void ADC_convert(u8 chn)
{
	u16	j;
	u8	k;		//ƽ��ֵ�˲�ʱʹ��

	Get_ADC12bitResult(chn);		//����i=0~15,��ѯ��ʽ��һ��ADC, �л�ͨ�����һ��ת���������. ����������ݵĲд��ѹӰ��.
	Get_ADC12bitResult(chn);		//����i=0~15,��ѯ��ʽ��һ��ADC, �л�ͨ����ڶ���ת���������. ����������ݵĲд��ѹӰ��.
	for(k=0, j=0; k<SUM_LENGTH; k++)	j += Get_ADC12bitResult(chn);	// �����ۼӺ�, ����0~15,��ѯ��ʽ��һ��ADC, ����ֵ���ǽ��
	j = j / SUM_LENGTH;		// ��ƽ��

	if(chn == 15)	printf("Bandgap=%04d  ",j);	//�ڻ�׼ 1.19V
	else		//ADC0~ADC14
	{
		printf("ADC%02d=%04d  ",chn,j);
	}
}

//========================================================================
// ����: void delay_ms(u8 ms)
// ����: ��ʱ������
// ����: ms,Ҫ��ʱ��ms��, ����ֻ֧��1~255ms. �Զ���Ӧ��ʱ��.
// ����: none.
// �汾: VER1.0
// ����: 2013-4-1
// ��ע: 
//========================================================================
void delay_ms(u8 ms)
{
	u16 i;
	do
	{
		i = MAIN_Fosc / 6000;
		while(--i);
	}while(--ms);
}