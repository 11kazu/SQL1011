/******************************************************************************
* File Name	: fpga_hdi_check.c
******************************************************************************/
#include <machine.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mathf.h>
#include "iodefine.h"
#include "typedefine.h"
#include "usercopy.h"
#include "user_define.h"

void send_to_fpga_hdi_check(void);				// FPGAへのﾃﾞｰﾀ送信関数(HDI出力検査)

//************************************************************/
//				FPGAへのﾃﾞｰﾀ送信関数(空回し)
//	1 → ・・・ → 8 → 正常終了[ 9]send_to_fpga_measure_stop()
//	                    11 → 14 → 15 → 16 → 11
//	                                            21 → 11
//	                                                  2
//	                          31 → ・・・ → 39 → 1
//	
//************************************************************/
// HDI出力検査
// LED消灯状態で計測を行い、HDI出力を確認する
void send_to_fpga_hdi_check(void)
{
	_UDWORD hdi;
	
	switch(SEQ.FPGA_SEND_STATUS){
		// RX → FPGAにﾃﾞｰﾀ送信
		// C_PRIOを「H」にする
		case 1:
			//if(F_PRIO_IN == 0){							// F_PRIO_INが「L」のとき
				SEQ.HDI_CHECK_COUNT = 0;					// HDIﾁｪｯｸｶｳﾝﾄをﾘｾｯﾄ
				SEQ.FLAG6.BIT.HDI_CHECK_OK = 0;				// HDIﾁｪｯｸOKﾌﾗｸﾞ
				SEQ.FLAG6.BIT.HDI_CHECK_COMPLETION = 0;		// HDIﾁｪｯｸ完了ﾌﾗｸﾞ
				
				C_PRIO_OUT	= 1;							// C_PRIO
				SEQ.FPGA_SEND_STATUS++;						// 次へ
			//}
			break;
			
		// ﾎﾟｰﾄを出力に設定する
		case 2:
			bus_to_out();							// ﾊﾞｽを出力に設定
			SEQ.FPGA_SEND_STATUS++;					// 次へ
			break;
			
		// ｺﾏﾝﾄﾞﾊﾞｽ・ﾃﾞｰﾀﾊﾞｽを設定する
		case 3:
			send_to_cbus(SEQ.CBUS_NUMBER);			// ｺﾏﾝﾄﾞﾅﾝﾊﾞｰ出力
			send_to_dbus_zero();					// ﾃﾞｰﾀ出力関数0
			SEQ.FPGA_SEND_STATUS++;					// 次へ
			break;
			
		// C_ACKを「H」にする
		case 4:
			C_ACK_OUT	= 1;						// C_ACK
			SEQ.FPGA_SEND_STATUS++;					// 次へ
			break;
			
		// C_ACKを「L」にする
		case 5:
			C_ACK_OUT	= 0;						// C_ACK
			SEQ.FPGA_SEND_STATUS++;					// 次へ
			break;
			
		// C_PRIO・ｺﾏﾝﾄﾞﾊﾞｽ・ﾃﾞｰﾀﾊﾞｽを「L」にする
		case 6:
			send_to_cbus_zero();					// ｺﾏﾝﾄﾞﾅﾝﾊﾞｰ出力関数0
			send_to_dbus_zero();					// ﾃﾞｰﾀ出力関数0
			SEQ.FPGA_SEND_STATUS++;					// 次へ
			break;
			
		// ﾎﾟｰﾄを入力に設定する
		case 7:
			bus_to_in();							// ﾊﾞｽを入力に設定
			SEQ.FPGA_SEND_STATUS++;					// 次へ
			break;
			
		// C_PRIOを「L」にする
		case 8:
			C_PRIO_OUT	= 0;						// C_PRIO
			if(SEQ.CBUS_NUMBER == 390){				// 指令ｺﾏﾝﾄﾞが待機のとき
				SEQ.FLAG.BIT.AFTER_STOPPING = 1;	// 計測停止後1ｻｲｸﾙ取得ﾌﾗｸﾞ
				SEQ.CHANGE_FPGA = 9;		// 計測停止
			}
			SEQ.FPGA_SEND_STATUS = 11;
			break;
	
		// FPGA → RXにﾃﾞｰﾀ送信
		// F_PRIO_INが「H」になったらC_ACKを「H」にする
		case 11:
			if(C_PRIO_OUT == 1){
				C_PRIO_OUT = 0;
			}else if(F_PRIO_IN == 1){
				SEQ.FPGA_RESTART_COUNT = 0;				// FPGA再ｽﾀｰﾄｶｳﾝﾄ
				SEQ.FPGA_SEND_STATUS = 14;
				C_ACK_OUT	= 1;					// C_ACK
				C_ACK_OUT	= 0;					// C_ACK
			}else{	// 1ms間F_PRIO_INが「1」にならないとき一旦停止する
				SEQ.FPGA_RESTART_COUNT++;				// FPGA再ｽﾀｰﾄｶｳﾝﾄ
				if(SEQ.FPGA_RESTART_COUNT >= 1000){
					SEQ.FPGA_SEND_STATUS = 31;
				}
				
			}
			break;
			
		case 14:	// 割り込み後
			switch(SEQ.CBUS_NUMBER){
				case 223:		// ｽｷｯﾌﾟ出力
					SEQ.FOCUSING_HDI = SEQ.INPUT_DBUS_LONG;			// 焦点合わせ
					hdi = (SEQ.FOCUSING_HDI >> 11) & 0x0000003F;	// 6ﾋﾞｯﾄ分のﾏｽｸ
					
					// HDI出力の確認
					if(hdi == 0x3F){								// HDI出力が全てONのとき
						SEQ.FLAG6.BIT.HDI_CHECK_OK = 1;				// HDIﾁｪｯｸOKﾌﾗｸﾞ
						SEQ.FLAG6.BIT.HDI_CHECK_COMPLETION = 1;		// HDIﾁｪｯｸ完了ﾌﾗｸﾞ
						SEQ.FLAG.BIT.MEASUREMENT = 0;
					}else{
						SEQ.HDI_CHECK_COUNT++;						// HDIﾁｪｯｸｶｳﾝﾄ
						if(SEQ.HDI_CHECK_COUNT >= 1000){			// HDIﾁｪｯｸｶｳﾝﾄが「1000」以上になったら
							SEQ.HDI_CHECK_COUNT = 0;				// HDIﾁｪｯｸｶｳﾝﾄをﾘｾｯﾄ
							SEQ.FLAG6.BIT.HDI_CHECK_COMPLETION = 1;	// HDIﾁｪｯｸ完了ﾌﾗｸﾞ
							SEQ.FLAG.BIT.MEASUREMENT = 0;
						}
					}
					break;
			}
			SEQ.FPGA_SEND_STATUS++;
			break;
			
		// C_ACKを「H」にする
		case 15:
			SEQ.FPGA_SEND_STATUS++;
			C_ACK_OUT	= 1;							// C_ACK
			break;
			
		// C_ACKを「L」にする
		case 16:
			C_ACK_OUT	= 0;							// C_ACK
			SEQ.FPGA_SEND_STATUS = 11;
			
			if(SEQ.CBUS_NUMBER == SEQ.LAST_CBUS_NUMBER){	// CBUSﾅﾝﾊﾞｰ(1ｻｲｸﾙ最終)のとき
				SEQ.FPGA_SEND_STATUS = 21;
			}
			break;
			
		// F_PRIO_INが「L」になっていることを確認する
		case 21:
			SEQ.FPGA_SEND_STATUS = 11;
			
			if(F_PRIO_IN == 0){											// F_PRIO_INが「L」のとき
				if(SEQ.FLAG.BIT.MEASUREMENT == 0){
					// 待機を送信(390)
					C_PRIO_OUT	= 1;									// C_PRIO
					SEQ.CBUS_NUMBER = 390;
					SEQ.FPGA_SEND_STATUS = 2;
				}
			}
			break;
			
			//
		// C_PRIOを「H」にする
		case 31:
			//if(F_PRIO_IN == 0){					// F_PRIO_INが「L」のとき
				C_PRIO_OUT	= 1;					// C_PRIO
				SEQ.FPGA_SEND_STATUS++;				// 次へ
			//}
			break;
			
		// ﾎﾟｰﾄを出力に設定する
		case 32:
			bus_to_out();							// ﾊﾞｽを出力に設定
			SEQ.FPGA_SEND_STATUS++;					// 次へ
			break;
			
		// ｺﾏﾝﾄﾞﾊﾞｽ・ﾃﾞｰﾀﾊﾞｽを設定する
		case 33:
			send_to_cbus(SEQ.CBUS_NUMBER);			// ｺﾏﾝﾄﾞﾅﾝﾊﾞｰ出力
			send_to_dbus_zero();					// ﾃﾞｰﾀ出力関数0
			SEQ.FPGA_SEND_STATUS++;					// 次へ
			break;
			
		// C_ACKを「H」にする
		case 34:
			C_ACK_OUT	= 1;						// C_ACK
			SEQ.FPGA_SEND_STATUS++;					// 次へ
			break;
			
		// C_ACKを「L」にする
		case 35:
			C_ACK_OUT	= 0;						// C_ACK
			SEQ.FPGA_SEND_STATUS++;					// 次へ
			break;
			
		// C_PRIO・ｺﾏﾝﾄﾞﾊﾞｽ・ﾃﾞｰﾀﾊﾞｽを「L」にする
		case 36:
			send_to_cbus_zero();					// ｺﾏﾝﾄﾞﾅﾝﾊﾞｰ出力関数0
			send_to_dbus_zero();					// ﾃﾞｰﾀ出力関数0
			SEQ.FPGA_SEND_STATUS++;					// 次へ
			break;
			
		// ﾎﾟｰﾄを入力に設定する
		case 37:
			bus_to_in();							// ﾊﾞｽを入力に設定
			SEQ.FPGA_SEND_STATUS++;					// 次へ
			break;
			
		// C_PRIOを「L」にする
		case 38:
			C_PRIO_OUT	= 0;						// C_PRIO
			SEQ.CBUS_NUMBER = 394;
			SEQ.FPGA_SEND_STATUS++;					// 次へ
			SEQ.FPGA_RESTART_COUNT = 0;				// FPGA再ｽﾀｰﾄｶｳﾝﾄ
			break;
			//
			
		// 1ms待機
		case 39:
			SEQ.FPGA_RESTART_COUNT++;				// FPGA再ｽﾀｰﾄｶｳﾝﾄ
			if(SEQ.FPGA_RESTART_COUNT >= 1000){
				SEQ.FPGA_SEND_STATUS = 1;
			}
			break;
	}
}
