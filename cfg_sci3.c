/******************************************************************************
* File Name	: cfg_sci3.c
******************************************************************************/
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "iodefine.h"
#include "typedefine.h"
#include "usercopy.h"
#include "user_define.h"

void sci3_init(void);					// M25初期設定

void cfg_txi(void);						// M25送信
void cfg_tei(void);						// M25送信終了

//************************************************************/
//				CFG初期設定(SCI3)
//************************************************************/
void sci3_init(void)
{
	MSTP(SCI3) = 0;			// RIIC0ﾓｼﾞｭｰﾙｽﾄｯﾌﾟ解除
	
	// 割り込み要求禁止
	IEN(SCI3, TEI3) = 0;
	IEN(SCI3, ERI3) = 0;
	IEN(SCI3, TXI3) = 0;
	IEN(SCI3, RXI3) = 0;
	
	IOPORT.PFFSCI.BIT.SCI3S = 1;		// P23-25をｼﾘｱﾙ端子として設定
	
	SCI3.SCR.BYTE = 0x00;

	while (0x00 != (SCI3.SCR.BYTE & 0xF0)){}

	// Select an On-chip baud rate generator to the clock source
	//SCI3.SCR.BIT.CKE = 0;
	SCI3.SCR.BIT.CKE = 1;

	// SMR - Serial Mode Register
	//SCI3.SMR.BYTE = 0x00;
	SCI3.SMR.BYTE = 0x80;

	// SCMR - Smart Card Mode Register
	//SCI3.SCMR.BYTE = 0xF2;
	SCI3.SCMR.BYTE = 0xFA;

	// SEMR - Serial Extended Mode Register
	SCI3.SEMR.BYTE = 0x00;
	
	// BRR - Bit Rate Register
	// 921600bps
	// ｸﾛｯｸ同期式ﾓｰﾄﾞ
	//Bit Rate: (48MHz/(8*2^(-1)*921600bps))-1 = 12.0208 ≒ 12
	//SCI3.BRR = 12;			// 921600bps
	//SCI3.BRR = 6;// 140821 PASS
//	SCI3.BRR = 8;//
	//市川改変箇所 2014/09/18	
	SCI3.BRR = 1;//
	
	//SCI3.BRR = 4;//	// 140919
	
	//SCI3.BRR = 22;ng
	
	delay_ms(1);			// 1ﾋﾞｯﾄ期間待機
	
	// 割り込み優先ﾚﾍﾞﾙ設定
/*	IPR(SCI3, TXI3) = 1;
	IPR(SCI3, RXI3) = 1;
	IPR(SCI3, TEI3) = 1;
	IPR(SCI3, ERI3) = 1;*/
	
	//市川改変箇所 2014/09/18	
	IPR(SCI3, TXI3) = 13;
	IPR(SCI3, RXI3) = 13;
	IPR(SCI3, TEI3) = 13;
	IPR(SCI3, ERI3) = 13;
	
	// 割り込み要求ｸﾘｱ
	IR(SCI3, TXI3) = 0;
	IR(SCI3, RXI3) = 0;
	IR(SCI3, TEI3) = 0;
	IR(SCI3, ERI3) = 0;
	
	// 割り込み要求許可
	IEN(SCI3, TXI3) = 1;
	//IEN(SCI3, RXI3) = 1;
	//IEN(SCI3, ERI3) = 1;
	IEN(SCI3, TEI3) = 1;
}

/********************************************************/
/*		CFG(SCI3)TXI(送信ﾃﾞｰﾀｴﾝﾌﾟﾃｨ割り込み)
/********************************************************/
void cfg_txi(void)
{
	SCI3.TDR = COM3.WR_BUF[COM3.WR_CONT];
	
	if(COM3.WR_CONT == COM3.SEND_COUNT){		// 最後まで送信したら
		SCI3.SCR.BIT.TIE = 0;					// TXI割り込み要求を禁止
		SCI3.SCR.BIT.TEIE = 1;					// TEI割り込み要求を許可
		
		while(SCI3.SSR.BIT.TEND != 1){}				// TENDﾌﾗｸﾞが「1」になるまで待機
		
		SCI3.SCR.BIT.TEIE = 0;						// TEI割り込み要求を禁止
		SCI3.SCR.BIT.TIE = 0;						// TXI割り込み要求を禁止
		SCI3.SCR.BIT.TE = 0;						// ｼﾘｱﾙ送信動作を禁止
	}
	
	COM3.WR_CONT++;
}

/********************************************************/
/*		CFG(SCI3)TEI(送信終了割り込み)
/********************************************************/
void cfg_tei(void)
{
	while(SCI3.SSR.BIT.TEND != 1){}				// TENDﾌﾗｸﾞが「1」になるまで待機
	
	SCI3.SCR.BIT.TEIE = 0;						// TEI割り込み要求を禁止
	SCI3.SCR.BIT.TIE = 0;						// TXI割り込み要求を禁止
	SCI3.SCR.BIT.TE = 0;						// ｼﾘｱﾙ送信動作を禁止
	
	SCI3.SCR.BIT.RIE = 1;						// RXIおよびERI割り込み要求を許可
	SCI3.SCR.BIT.RE = 1;						// ｼﾘｱﾙ受信動作を許可
	
	COM3.SUB_STATUS++;
}
