#include ".\comm\head.h"

#define MAIN_Fosc        12000000L   //定义主时钟（精确计算115200波特率）
#define Baudrate      115200L
#define TM            (65536 -(MAIN_Fosc/Baudrate/4))


/*************	本地常量声明	**************/

#define	ADC_SPEED	15		/* 0~15, ADC转换时间(CPU时钟数) = (n+1)*32  ADCCFG */
#define	RES_FMT		(1<<5)	/* ADC结果格式 0: 左对齐, ADC_RES: D11 D10 D9 D8 D7 D6 D5 D4, ADC_RESL: D3 D2 D1 D0 0 0 0 0 */
							/* ADCCFG      1: 右对齐, ADC_RES: 0 0 0 0 D11 D10 D9 D8, ADC_RESL: D7 D6 D5 D4 D3 D2 D1 D0 */

/*************	本地变量声明	**************/

unsigned char adc_result[256];
unsigned char temp_result[10];
unsigned char mcuid[20];
unsigned char temp_mcuid[10];

/*************	本地函数声明	**************/

void	delay_ms(u8 ms);
void	ADC_convert(u8 chn);	//chn=0~7对应P1.0~P1.7, chn=8~14对应P0.0~P0.6, chn=15对应BandGap电压
u16	Get_ADC12bitResult(u8 channel);

/******************** 串口打印函数 ********************/
//void UartInit(void)
//{
//	S2_S = 1;       //UART2 switch to: 0: P1.0 P1.1,  1: P4.6 P4.7
//    S2CFG |= 0x01;  //使用串口2时，W1位必需设置为1，否则可能会产生不可预期的错误
//	S2CON = (S2CON & 0x3f) | 0x40; 
//	T2L  = TM;
//	T2H  = TM>>8;
//	AUXR |= 0x14;	      //定时器2时钟1T模式,开始计时
//}

//void UartPutc(unsigned char dat)
//{
//	S2BUF  = dat; 
//	while(S2TI == 0);
//	S2TI = 0;    //Clear Tx flag
//}

//char putchar(char c)
//{
//	UartPutc(c);
//	return c;
//}

/**********************************************/
void main(void)
{
	u8	i;


    WTST = 0;  //设置程序指令延时参数，赋值为0可将CPU执行指令的速度设置为最快
    EAXFR = 1; //扩展寄存器(XFR)访问使能
    CKCON = 0; //提高访问XRAM速度

    P0M1 = 0x7f;   P0M0 = 0x00;   //设置为高阻输入
    P1M1 = 0xfb;   P1M0 = 0x00;   //设置为高阻输入
    P2M1 = 0x3c;   P2M0 = 0x3c;   //设置P2.2~P2.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P3M1 = 0x50;   P3M0 = 0x50;   //设置P3.4、P3.6为漏极开路(实验箱加了上拉电阻到3.3V)    P3M1 = 0x50;   P3M0 = 0x54;   
    P4M1 = 0x3c;   P4M0 = 0x3c;   //设置P4.2~P4.5为漏极开路(实验箱加了上拉电阻到3.3V)
    P5M1 = 0x1c;   P5M0 = 0x0c;   //设置P5.2、P5.3为漏极开路(实验箱加了上拉电阻到3.3V)，设置P5.4为高阻输入
    P6M1 = 0xff;   P6M0 = 0xff;   //设置为漏极开路(实验箱加了上拉电阻到3.3V)
    P7M1 = 0x00;   P7M0 = 0x00;   //设置为准双向口

	ADCTIM = 0x3f;  //设置通道选择时间、保持时间、采样时间
	ADCCFG = RES_FMT + ADC_SPEED;
	//ADC模块电源打开后，需等待1ms，MCU内部ADC电源稳定后再进行AD转换
	ADC_CONTR = 0x80 + 0;	//ADC on + channel

//	UartInit();
	UART1_config(2);
	EA = 1;
	
//	TxOut_EN = 0;
	P32 = 0;
	printf("NanDu Program Start\r\n");
	printf("南都汽车电子阻值检测，版本号：Ver%02d.%02d!\r\n",Major_Ver,Minor_Ver);
//	TxOut_EN = 0;
	
	for(i=0; i<7; i++)
		{
//		printf("%02X",ID_ADDR[i]);	
		sprintf (temp_mcuid,"%02X",ID_ADDR[i]);
		strcat(mcuid,temp_mcuid);
		}
		mcuid[15]='\0';
//		printf("MCUID:%s \r\n",mcuid);
	while (1)
	{
		adc_result[0] = '\0';	
		for(i=0; i<15; i++)
		{
			delay_ms(200);
			ADC_convert(i);		//发送轮询通道AD值
			if(i==14)	
			{
				printf("MCUID;%s;",mcuid);
				printf("adc_result;%s",adc_result);	
				printf("\r\n");
//				TxOut_EN = 0;
			}
		}
	}
}


//========================================================================
// 函数: u16 Get_ADC12bitResult(u8 channel))	//channel = 0~15
// 描述: 查询法读一次ADC结果.
// 参数: channel: 选择要转换的ADC, 0~15.
// 返回: 12位ADC结果.
// 版本: V1.0, 2016-4-28
//========================================================================
u16	Get_ADC12bitResult(u8 channel)	//channel = 0~15
{
	ADC_RES = 0;
	ADC_RESL = 0;

    ADC_CONTR = (ADC_CONTR & 0xf0) | channel; //设置ADC转换通道
    ADC_START = 1;//启动ADC转换
    _nop_();
	_nop_();
	_nop_();
	_nop_();
    while(ADC_FLAG == 0);   //wait for ADC finish
    ADC_FLAG = 0;     //清除ADC结束标志
	return (((u16)ADC_RES << 8) | ADC_RESL);
}

#define		SUM_LENGTH	16	/* 平均值采样次数 最大值16 */

/***********************************
查询方式做一次ADC, chn为通道号, chn=0~7对应P1.0~P1.7, chn=8~14对应P0.0~P0.6, chn=15对应BandGap电压.
***********************************/
void ADC_convert(u8 chn)
{
	u16	j;
	u8	k;		//平均值滤波时使用
	double f;

	Get_ADC12bitResult(chn);		//参数i=0~15,查询方式做一次ADC, 切换通道后第一次转换结果丢弃. 避免采样电容的残存电压影响.
	Get_ADC12bitResult(chn);		//参数i=0~15,查询方式做一次ADC, 切换通道后第二次转换结果丢弃. 避免采样电容的残存电压影响.
	for(k=0, j=0; k<SUM_LENGTH; k++)	j += Get_ADC12bitResult(chn);	// 采样累加和, 参数0~15,查询方式做一次ADC, 返回值就是结果
	j = j / SUM_LENGTH;		// 求平均

	f=get_r(300,j);

	if(chn != 15)
	{			
//		printf("ADC%02d=%0.2f  ",chn,f);
	sprintf (temp_result,"%0.2f;",f);
	strcat(adc_result,temp_result);
	}
}

//========================================================================
// 函数: void delay_ms(u8 ms)
// 描述: 延时函数。
// 参数: ms,要延时的ms数, 这里只支持1~255ms. 自动适应主时钟.
// 返回: none.
// 版本: VER1.0
// 日期: 2013-4-1
// 备注: 
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

float get_r(float R0,unsigned int ad)
{
	float r;
//	if(ad>=4000) return(9999);
	r=R0*(float)ad/(float)(4095.00-ad);	
	if(r<=1) r=1;
	return(r);
}