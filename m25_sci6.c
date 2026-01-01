/******************************************************************************
* File Name	: m25_sci6.c
******************************************************************************/
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "iodefine.h"
#include "typedefine.h"
#include "user.h"
#include "user_define.h"

void sci6_init(void);					// M25初期設定

void m25_rxi(void);						// M25受信
void m25_txi(void);						// M25送信
void m25_tei(void);						// M25送信終了

void m25_init(void);					// M25初期化関数

void m25_write_enable(void);			// M25ﾗｲﾄｲﾈｰﾌﾞﾙ
void m25_bulk_erase(void);				// M25ﾊﾞﾙｸｲﾚｰｽ

void m25_status_register_read(void);	// M25ｽﾃｰﾀｽﾚｼﾞｽﾀﾘｰﾄﾞ


//************************************************************/
//				RS232C初期設定(SCI6)
//************************************************************/
void sci6_init(void)
{
	MSTP(SCI6) = 0;			// RIIC0ﾓｼﾞｭｰﾙｽﾄｯﾌﾟ解除
	
	// 割り込み要求禁止
	IEN(SCI6, TEI6) = 0;
	IEN(SCI6, ERI6) = 0;
	IEN(SCI6, TXI6) = 0;
	IEN(SCI6, RXI6) = 0;
	
	SCI6.SCR.BYTE = 0x00;

	while (0x00 != (SCI6.SCR.BYTE & 0xF0)){}

	// Select an On-chip baud rate generator to the clock source
	//SCI6.SCR.BIT.CKE = 0;
	SCI6.SCR.BIT.CKE = 1;	// SCKから同じ周波数のｸﾛｯｸを出力

	// SMR - Serial Mode Register
	//SCI6.SMR.BYTE = 0x00;
	SCI6.SMR.BYTE = 0x80;

	// SCMR - Smart Card Mode Register
	//SCI6.SCMR.BYTE = 0xF2;
	SCI6.SCMR.BYTE = 0xFA;

	// SEMR - Serial Extended Mode Register
	SCI6.SEMR.BYTE = 0x00;
	
	// BRR - Bit Rate Register
	// 921600bps
	// ｸﾛｯｸ同期式ﾓｰﾄﾞ
	//Bit Rate: (48MHz/(8*2^(-1)*921600bps))-1 = 12.0208 ≒ 12
	//SCI6.BRR = 12;//			// 921600bps
	//SCI6.BRR = 8;// 140821 PASS
	
	//SCI6.BRR = 18;//			// 921600bps
	
	//SCI6.BRR = 25;//			// 461538bps
	
	//市川改変箇所 2014/09/18
	SCI6.BRR = 6;
	
	//SCI6.BRR = 12;	// 140919
	//SCI6.BRR = 16;	// 140919
	
	delay_ms(1);			// 1ﾋﾞｯﾄ期間待機
	
	// 割り込み優先ﾚﾍﾞﾙ設定
/*	IPR(SCI6, TXI6) = 1;
	IPR(SCI6, RXI6) = 1;
	IPR(SCI6, TEI6) = 1;
	IPR(SCI6, ERI6) = 1;*/
	
	//市川改変箇所 2014/09/18	
	IPR(SCI6, TXI6) = 14;
	IPR(SCI6, RXI6) = 14;
	IPR(SCI6, TEI6) = 14;
	IPR(SCI6, ERI6) = 14;
	
	// 割り込み要求ｸﾘｱ
	IR(SCI6, TXI6) = 0;
	IR(SCI6, RXI6) = 0;
	IR(SCI6, TEI6) = 0;
	IR(SCI6, ERI6) = 0;
	
	// 割り込み要求許可
	IEN(SCI6, TXI6) = 1;
	IEN(SCI6, RXI6) = 1;
	IEN(SCI6, ERI6) = 1;
	IEN(SCI6, TEI6) = 1;
}

/********************************************************/
/*		M25(SCI6)RXI(受信ﾃﾞｰﾀﾌﾙ割り込み)
/********************************************************/
void m25_rxi(void)
{
	COM6.RE_BUF[COM6.RE_CONT] = SCI6.RDR;
	COM6.RE_CONT++;
	
	if(COM6.RE_CONT == 256){
		SCI6.SCR.BIT.RIE = 0;					// RXIおよびERI割り込み要求を禁止
		SCI6.SCR.BIT.RE = 0;					// ｼﾘｱﾙ受信動作を禁止
		M25_CS_OUT = 1;								// M25_CSを「H」にする
		COM6.SUB_STATUS++;
	}
}

/********************************************************/
/*		M25(SCI6)TXI(送信ﾃﾞｰﾀｴﾝﾌﾟﾃｨ割り込み)
/********************************************************/
void m25_txi(void)
{
	SCI6.TDR = COM6.WR_BUF[COM6.WR_CONT];
	
	if(COM6.WR_CONT == COM6.SEND_COUNT){		// 最後まで送信したら
		SCI6.SCR.BIT.TIE = 0;					// TXI割り込み要求を禁止
		SCI6.SCR.BIT.TEIE = 1;					// TEI割り込み要求を許可
		
		while(SCI6.SSR.BIT.TEND != 1){}				// TENDﾌﾗｸﾞが「1」になるまで待機
		
		SCI6.SCR.BIT.TEIE = 0;						// TEI割り込み要求を禁止
		SCI6.SCR.BIT.TIE = 0;						// TXI割り込み要求を禁止
		SCI6.SCR.BIT.TE = 0;						// ｼﾘｱﾙ送信動作を禁止
		
		if(COM6.SUB_STATUS != 15){
			M25_CS_OUT = 1;								// M25_CSを「H」にする
		}
		
		if(COM6.SUB_STATUS == 15){
			COM6.RE_CONT = 0;
			SCI6.SCR.BIT.RIE = 1;						// RXIおよびERI割り込み要求を許可
			SCI6.SCR.BIT.RE = 1;						// ｼﾘｱﾙ受信動作を許可
		}	
		COM6.SUB_STATUS++;
	}
	
	COM6.WR_CONT++;
}

/********************************************************/
/*		M25(SCI6)TEI(送信終了割り込み)
/********************************************************/
void m25_tei(void)
{
	while(SCI6.SSR.BIT.TEND != 1){}				// TENDﾌﾗｸﾞが「1」になるまで待機
	
	SCI6.SCR.BIT.TEIE = 0;						// TEI割り込み要求を禁止
	SCI6.SCR.BIT.TIE = 0;						// TXI割り込み要求を禁止
	SCI6.SCR.BIT.TE = 0;						// ｼﾘｱﾙ送信動作を禁止
	
	M25_CS_OUT = 1;								// M25_CSを「H」にする
	
	SCI6.SCR.BIT.RIE = 1;						// RXIおよびERI割り込み要求を許可
	SCI6.SCR.BIT.RE = 1;						// ｼﾘｱﾙ受信動作を許可
	
	COM6.SUB_STATUS++;
}

/********************************************************/
/*		M25ﾗｲﾄｲﾈｰﾌﾞﾙ
/********************************************************/
void m25_write_enable(void)
{
	M25_CS_OUT = 0;						// M25_CSを「L」にする
	COM6.SEND_COUNT = 0;
	COM6.WR_BUF[COM6.SEND_COUNT] = 0x06;
}

/********************************************************/
/*		M25ﾊﾞﾙｸｲﾚｰｽ
/********************************************************/
void m25_bulk_erase(void)
{
	M25_CS_OUT = 0;						// M25_CSを「L」にする
	COM6.START_ADDRESS = 0;
	COM6.SEND_COUNT = 0;
	COM6.WR_BUF[COM6.SEND_COUNT] = 0xC7;
}

/********************************************************/
/*		M25ｽﾃｰﾀｽﾚｼﾞｽﾀﾘｰﾄﾞ
/********************************************************/
void m25_status_register_read(void)
{
	M25_CS_OUT = 0;						// M25_CSを「L」にする
	COM6.START_ADDRESS = 0;
	COM6.SEND_COUNT = 0;
	COM6.WR_BUF[COM6.SEND_COUNT] = 0x05;
	//COM6.WR_BUF[COM6.SEND_COUNT] = 0x9F;	// READ IDENTIFICATION
}

//************************************************************/
//				M25初期化関数
//************************************************************/
void m25_init(void)
{
	_UWORD i;
	
	if(TIM1.MSEC_10.BIT.UNI_UP3	== 1){
		TIM1.MSEC_10.BIT.UNI_UP3 = 0;
		if(COM6.DATA_SELECT == 1){			// 0 - 255
			COM6.DATA_SELECT = 0;
		}else if(COM6.DATA_SELECT == 0){	// 256 - 511
			COM6.DATA_SELECT = 1;
		}
		COM6.SUB_STATUS = 1;
	}
	
	switch(COM6.SUB_STATUS){
		// ﾗｲﾄｲﾈｰﾌﾞﾙ書き込み
		case 1:
			SCI6.SCR.BIT.RIE = 0;							// RXIおよびERI割り込み要求を禁止
			SCI6.SCR.BIT.RE = 0;							// ｼﾘｱﾙ受信動作を禁止
			
			m25_write_enable();								// ﾗｲﾄｲﾈｰﾌﾞﾙ
			
			COM6.WR_CONT = 0;
			COM6.RE_CONT = 0;
			
			COM6.SUB_STATUS++;
			
			SCI6.SCR.BIT.TIE = 1;							// TIEを「1」
			SCI6.SCR.BIT.TE = 1;							// TEを「1」
			break;
			
		// ﾃﾞｰﾀ書き込み
		case 3:
			SCI6.SCR.BIT.RIE = 0;							// RXIおよびERI割り込み要求を禁止
			SCI6.SCR.BIT.RE = 0;							// ｼﾘｱﾙ受信動作を禁止
			
			M25_CS_OUT = 0;									// M25_CSを「L」にする
			
			// ﾍﾟｰｼﾞﾌﾟﾛｸﾞﾗﾑ
			COM6.SEND_COUNT = 0;
			COM6.WR_BUF[COM6.SEND_COUNT] = 0x02;
			
			// ｱﾄﾞﾚｽ
			COM6.SEND_COUNT++;
			COM6.WR_BUF[COM6.SEND_COUNT] = (COM6.START_ADDRESS >> 8);
			COM6.SEND_COUNT++;
			COM6.WR_BUF[COM6.SEND_COUNT] = COM6.START_ADDRESS;
			COM6.SEND_COUNT++;
			COM6.WR_BUF[COM6.SEND_COUNT] = 0;
			
			// ﾃﾞｰﾀ
			if(COM6.DATA_SELECT == 1){			// 0 - 255
				for(i = 0; i<=255; i++){
					COM6.SEND_COUNT++;
					COM6.WR_BUF[COM6.SEND_COUNT] = COM2.RE_BUF[i];
					COM2.RE_BUF[i] = 0xFF;
				}
			}else if(COM6.DATA_SELECT == 0){	// 256 - 511
				for(i = 256; i<=511; i++){
					COM6.SEND_COUNT++;
					COM6.WR_BUF[COM6.SEND_COUNT] = COM2.RE_BUF[i];
					COM2.RE_BUF[i] = 0xFF;
				}
			}
			
			// ｱﾄﾞﾚｽをｲﾝｸﾘﾒﾝﾄ
			COM6.START_ADDRESS++;
			
			COM6.WR_CONT = 0;
			COM6.RE_CONT = 0;
			
			COM6.SUB_STATUS++;
			
			SCI6.SCR.BIT.TIE = 1;							// TIEを「1」
			SCI6.SCR.BIT.TE = 1;							// TEを「1」
			
			break;
			
		case 5:
			//COM6.SUB_STATUS = 5;
			break;
			
		// ﾃﾞｰﾀ読み込み
		case 11:
			CFG_PROG_OUT = 0;					// CFG_PROGを「L」にする
			COM6.SUB_STATUS++;
			break;
			
		case 12:
			if(CFG_INT_IN == 0){
				CFG_PROG_OUT = 1;					// CFG_PROGを「H」にする
				COM6.SUB_STATUS++;
			}
			break;
			
		case 13:
			if(CFG_INT_IN == 1){
				COM6.SUB_STATUS++;
			}
			break;
			
		// ﾃﾞｰﾀ読み込み
		case 14:
			SCI6.SCR.BIT.RIE = 0;							// RXIおよびERI割り込み要求を禁止
			SCI6.SCR.BIT.RE = 0;							// ｼﾘｱﾙ受信動作を禁止
			
			// 動作確認用
			if(CFG_INT_IN == 0){
				M25_CS_OUT = 0;						// M25_CSを「L」にする
			}
			
			// 
			M25_CS_OUT = 0;						// M25_CSを「L」にする
			
			COM6.SEND_COUNT = 0;
			COM6.WR_BUF[COM6.SEND_COUNT] = 0x03;	// ﾘｰﾄﾞ
			// ｱﾄﾞﾚｽ
			COM6.SEND_COUNT++;
			COM6.WR_BUF[COM6.SEND_COUNT] = (COM6.START_ADDRESS >> 8);
			COM6.SEND_COUNT++;
			COM6.WR_BUF[COM6.SEND_COUNT] = COM6.START_ADDRESS;
			COM6.SEND_COUNT++;
			COM6.WR_BUF[COM6.SEND_COUNT] = 0;
			
			
			COM6.WR_CONT = 0;
			COM6.RE_CONT = 0;
			
			COM6.SUB_STATUS++;
			
			SCI6.SCR.BIT.TIE = 1;							// TIEを「1」
			SCI6.SCR.BIT.TE = 1;							// TEを「1」
			
			break;
			
		case 17:
			for(i = 0; i<=255; i++){
				COM3.WR_BUF[i] = COM6.RE_BUF[i];
			}
			COM3.SEND_COUNT = 255;
			COM3.WR_CONT = 0;
				
			COM6.SUB_STATUS++;
			
			SCI3.SCR.BIT.TIE = 1;							// TIEを「1」
			SCI3.SCR.BIT.TE = 1;							// TEを「1」
			
			break;
			
		case 18:
			if(CFG_DONE_IN == 0){
				// 続けて送信
				COM6.START_ADDRESS++;
				COM6.SUB_STATUS = 14;
				
				if(TIM1.MSEC_10.BIT.UNI_UP4	== 1){
					TIM1.MSEC_10.BIT.UNI_UP4 = 0;
					COM6.SUB_STATUS = 20;
				}
			
			}else{
				// 終了
				COM6.SUB_STATUS++;
				
				//
				SEQ.FLAG.BIT.POWER = !SEQ.FLAG.BIT.POWER;	// 電源(状態反転)
				SEQ.FPGA_SEND_STATUS = 1;
				//OUT.MASTER_STATUS = OUT_DRV_MODE;		// 通常ﾓｰﾄﾞ
				//OUT.SUB_STATUS = 1;
				SEQ.FLAG.BIT.POWER_ON = 1;				// 電源ONﾌﾗｸﾞをｾｯﾄ
				
				SEQ.FLAG3.BIT.BLINK_LED = 1;				// LED点滅ﾌﾗｸﾞ
				
				//
				
				//LED.FLAG.BIT.LOGO = 0;				// 緑
				//led_logo();					// ﾛｺﾞ点灯色設定
			}
			break;
			
		// ﾗｲﾄｲﾈｰﾌﾞﾙ書き込み
		case 21:
			SCI6.SCR.BIT.RIE = 0;							// RXIおよびERI割り込み要求を禁止
			SCI6.SCR.BIT.RE = 0;							// ｼﾘｱﾙ受信動作を禁止
			
			m25_write_enable();								// ﾗｲﾄｲﾈｰﾌﾞﾙ
			
			COM6.WR_CONT = 0;
			COM6.RE_CONT = 0;
			
			COM6.SUB_STATUS++;
			
			SCI6.SCR.BIT.TIE = 1;							// TIEを「1」
			SCI6.SCR.BIT.TE = 1;							// TEを「1」
			
			break;
			
		// ﾊﾞﾙｸｲﾚｰｽ書き込み
		case 23:
			SCI6.SCR.BIT.RIE = 0;							// RXIおよびERI割り込み要求を禁止
			SCI6.SCR.BIT.RE = 0;							// ｼﾘｱﾙ受信動作を禁止
			
			m25_bulk_erase();								// ﾊﾞﾙｸｲﾚｰｽ
			
			COM6.WR_CONT = 0;
			COM6.RE_CONT = 0;
			
			COM6.SUB_STATUS++;
			
			SCI6.SCR.BIT.TIE = 1;							// TIEを「1」
			SCI6.SCR.BIT.TE = 1;							// TEを「1」
			
			break;
	}
}