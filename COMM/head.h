//����HEAH.H
#ifndef   _HEAD_H_
#define   _HEAD_H_

#include ".\STC32G.h"
#include "intrins.h"
#include "stdio.h"
#include "string.h"

//������������
typedef 	unsigned char	u8;
typedef 	unsigned int	u16;
typedef 	unsigned long	u32;

//��Ƶ
#define MAIN_Fosc        12000000L 
#define Major_Ver      0
#define Minor_Ver      1

//mcu id
#define ID_ADDR (&CHIPID0)

//max485TXʹ��
sbit  TxOut_EN  = P3^2;

//��ʱ��װ����ֵ

#define Timer0_Reload    (MAIN_Fosc /1000)      //       (65536UL -(MAIN_Fosc / 1000))       //Timer 0 �ж�Ƶ��, 1000��/�� 1ms
#define Timer3_Reload    (MAIN_Fosc /1000)      //Timer 3 �ж�Ƶ��, 4000��/��
#define Timer1_Reload    (MAIN_Fosc /2460)      //Timer 1 �ж�Ƶ��, 2000��/��
void Timer0_init(void);
void Timer1_init(void);
void Timer3_init(void);
void display_init(void);


//��ʱ����
void	delay_ms(u8 ms);


//**********************************AD����**********************************//
void move_ad_data(unsigned int *p);
unsigned int average_move_ad_max_min(unsigned int *p);
u16 Get_ADC12bitResult(u8 channel) ;
void Dell_Voltage(void);
void Manage_Fule(void)  ;
extern u16 Voltage_Data;
float get_r(float R0,unsigned int ad);
unsigned int get_ad_result(unsigned char channl);

//----------------���ڴ�ӡ--------------------------------------------//

#define Baudrate1   (65536 - MAIN_Fosc / 115200 / 4)

#define UART1_BUF_LENGTH    128

void UartPutc(unsigned char dat);
char putchar(char c);
void UART1_config(u8 brt);

#endif
