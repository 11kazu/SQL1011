/******************************************************************************
* File Name	: init.c
******************************************************************************/
#include <machine.h>
#include "iodefine.h"
#include "typedefine.h"
#include "usercopy.h"
#include "user_define.h"
#include "vect.h"

void delay_us(int);					// ﾃﾞｨﾚｲ関数(1us)
void delay_ms(int);					// ﾃﾞｨﾚｲ関数(1ms)
void system_init(void);				// ｼｽﾃﾑ初期設定
void port_init(void);				// ﾎﾟｰﾄ初期設定
void tmr_init(void);				// ﾀｲﾏｰ初期設定

//************************************************************/
//				ﾃﾞｨﾚｲ関数(1us)
//************************************************************/
void delay_us(int c)
{	int i, j;
	for(i = 0; i < c; i++)
	{	for(j = 0; j < 11; j++) {};
	}
}

//************************************************************/
//				ﾃﾞｨﾚｲ関数(1ms)
//************************************************************/
void delay_ms(int c)
{	int i;
	for(i = 0; i < c; i++)
	{
		delay_us(1000);
	}
}

//************************************************************/
//				ｼｽﾃﾑ初期化関数
//************************************************************/
void system_init(void)
{
	// ｼｽﾃﾑｸﾛｯｸｺﾝﾄﾛｰﾙﾚｼﾞｽﾀ(SCKCR)
	// ｼｽﾃﾑｸﾛｯｸ8倍					ICK	= 0	(12MHz * 8 = 96MHz)
	// 外部ﾊﾞｽｸﾛｯｸ・SDRAMｸﾛｯｸ2倍	BCK	= 2	(12MHz * 2 = 24MHz)
	// 周辺ﾓｼﾞｭｰﾙｸﾛｯｸ4倍			PCK	= 1	(12MHz * 4 = 48MHz)
	SYSTEM.SCKCR.LONG	= 0x00020100;
}

//************************************************************/
//				ﾎﾟｰﾄ初期化関数
//************************************************************/
void port_init(void)
{
	IEN(ICU, IRQ3) = 0;
	IEN(ICU, IRQ15) = 0;
	
	// ﾃﾞｰﾀﾃﾞｨﾚｸｼｮﾝﾚｼﾞｽﾀ(DDR)	0:入力	1:出力	X:未使用(1)	-:予約ﾋﾞｯﾄ(0)	+:ﾃﾞﾊﾞｯｶﾞ(0)
	//PORT0.DDR.BYTE		= 0xA0;			// X-X-0000	// P03ｱﾅﾛｸﾞ出力、P00-02通信(ｼﾘｱﾙ)、その他無し
	PORT0.DDR.BYTE		= 0x20;			// 0-X-0000	// P07入力、P03ｱﾅﾛｸﾞ出力、P00-02通信(ｼﾘｱﾙ)、その他無し
	// 初期設定はP12・13を出力とする（通信を行うときには入力にする）
	PORT1.DDR.BYTE		= 0x6C;			// 011011--	// P14・17入力、P15・16出力、P12・13通信(I2C)、その他無し
	PORT2.DDR.BYTE		= 0x18;			// ++011000	// P22・25入力、P23・24出力、P20・21通信(ｼﾘｱﾙ)、その他無し
	PORT3.DDR.BYTE		= 0x04;			// ---+01++	// P33入力(割込)、P32出力、その他無し
	PORT4.DDR.BYTE		= 0x12;			// 00010010	// P42・43入力、P45-47入力(割込)、P40ｱﾅﾛｸﾞ入力、P41・44出力
	PORT5.DDR.BYTE		= 0x32;			// -0110010	// P56入力、P51・54・55出力、P50・52通信(ｼﾘｱﾙ)、その他無し
	PORT6.DDR.BYTE		= 0x80;			// 10000000	// P60-66入力、その他無し
	PORT7.DDR.BYTE		= 0xFF;			// 11111111	// P70-77出力
	PORT8.DDR.BYTE		= 0x0F;			// ----1111	// P80-83出力
	PORT9.DDR.BYTE		= 0x06;			// ----0110	// P90・93入力、P91・92出力
	PORTA.DDR.BYTE		= 0x00;			// 00000000	// PA0-A7入力
	PORTB.DDR.BYTE		= 0x00;			// 00000000	// PB0-B7入力
	PORTC.DDR.BYTE		= 0x00;			// 00000000	// PC0-C7入力
	PORTD.DDR.BYTE		= 0x00;			// 00000000	// PD0-D7入力
	PORTE.DDR.BYTE		= 0x00;			// 00000000	// PE0-E7入力
	
	DA.DACR.BYTE = 0x1F;				// DA0・DA1 アナログ出力禁止
	MSTP(DA) = 0;						// ﾓｼﾞｭｰﾙｽﾄｯﾌﾟ解除
	
	DA.DADR0 = 0;
	DA.DACR.BIT.DAOE0 = 1;				// DA0 アナログ出力許可
	
	// ﾎﾟｰﾄﾌｧﾝｸｼｮﾝﾚｼﾞｽﾀ
	IOPORT.PF8IRQ.BIT.ITS15 = 1;		// P47をIRQ15-B入力端子として設定
	
	// 入力ﾊﾞｯﾌｧｺﾝﾄﾛｰﾙﾚｼﾞｽﾀ(ICR)(PORTﾚｼﾞｽﾀを読むときにはICRを「1」にする必要がある)
	PORT0.ICR.BYTE			= 0x02;		// 00000010
	PORT1.ICR.BYTE			= 0x90;		// 10010000
	PORT2.ICR.BYTE			= 0x26;		// 00100110
	PORT3.ICR.BYTE			= 0x08;		// 00001000
	PORT4.ICR.BYTE			= 0xEC;		// 11101100
	PORT5.ICR.BYTE			= 0x44;		// 01000100
	PORT6.ICR.BYTE			= 0x7F;		// 01111111
	
	PORT9.ICR.BYTE			= 0x09;		// 00001001
	PORTA.ICR.BYTE			= 0xFF;		// 11111111
	PORTB.ICR.BYTE			= 0xFF;		// 11111111
	PORTC.ICR.BYTE			= 0xFF;		// 11111111
	PORTD.ICR.BYTE			= 0xFF;		// 11111111
	PORTE.ICR.BYTE			= 0xFF;		// 11111111
	
	// C_INT割り込み
	// 割り込み設定
	IPR(ICU, IRQ3) = 1;
	ICU.IRQCR[3].BIT.IRQMD = 2;			// 立ち上がりｴｯｼﾞ
	IR(ICU, IRQ3) = 0;
	IEN(ICU, IRQ3) = 1;
	
	// 高速割り込み設定
	ICU.FIR.WORD = 0x8000 + 67;
	set_fintv((void *)Excep_ICU_IRQ3);
	//
	
	// POWERSW割り込み
	// 割り込み設定
	IPR(ICU, IRQ15) = 1;
	ICU.IRQCR[15].BIT.IRQMD = 2;		// 立ち上がりｴｯｼﾞ
	IR(ICU, IRQ15) = 0;
	IEN(ICU, IRQ15) = 1;
}

//************************************************************/
//				TMR初期設定
//************************************************************/
void tmr_init(void)
{
	//IPR(TMR0, CMIA0) = 1;
	IPR(TMR0, CMIA0) = 12;
	
	MSTP(TMR0) = 0;
	
	//ﾁｬﾝﾈﾙ0設定
	TMR0.TCR.BIT.CMIEA	= 1;			//ｺﾝﾍﾟｱﾏｯﾁA許可
	TMR0.TCR.BIT.OVIE	= 0;			//ｵｰﾊﾞｰﾌﾛｰ禁止
	TMR0.TCR.BIT.CCLR	= 1;			//ｸﾘｱ条件ｺﾝﾍﾟｱﾏｯﾁA
	TMR0.TCCR.BYTE		= 0x08;
	//TMR0.TCORA			= 47;			// 0.001msで割り込みを行う
	TMR0.TCORA			= 239;			// 0.005msで割り込みを行う
	
	// f = PCLK/(N+1) (f:ｶｳﾝﾀ周波数 PCLK:動作周波数 N:TCORA、TCORBﾚｼﾞｽﾀの設定値)
	// N = (PCLK/f) - 1 = ((48000000 / 1000000) - 1 = 47	// 0.001msで割り込みを行う
	// N = (PCLK/f) - 1 = ((48000000 / 200000) - 1 = 239	// 0.005msで割り込みを行う
	
	IEN(TMR0, CMIA0) = 1;
}

