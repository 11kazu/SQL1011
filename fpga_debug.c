/******************************************************************************
* File Name	: fpga_debug.c
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

void send_to_fpga_debug(void);				// FPGAへのﾃﾞｰﾀ送信関数(ﾃﾞﾊﾞｯｸﾞ出力)

//************************************************************/
//				FPGAへのﾃﾞｰﾀ送信関数(ﾃﾞﾊﾞｯｸﾞ出力)
//	1 → ・・・ → 8 → 11 → 14 → 15 → 11
//	                                   →  1
//************************************************************/
// DEBUG用
void send_to_fpga_debug(void)
{
	union UFloatLong Variable;
	
	switch(SEQ.FPGA_SEND_STATUS){
		// RX → FPGAにﾃﾞｰﾀ送信
		// C_PRIOを「H」にする
		case 1:
			//if(F_PRIO_IN == 0){						// F_PRIO_INが「L」のとき
				C_PRIO_OUT	= 1;						// C_PRIO
				SEQ.FPGA_SEND_STATUS++;					// 次へ
			//}
			break;
			
		// ﾎﾟｰﾄを出力に設定する
		case 2:
			bus_to_out();								// ﾊﾞｽを出力に設定
			SEQ.FPGA_SEND_STATUS++;						// 次へ
			break;
			
		// ｺﾏﾝﾄﾞﾊﾞｽ・ﾃﾞｰﾀﾊﾞｽを設定する
		case 3:
			send_to_cbus(SEQ.CBUS_NUMBER);				// ｺﾏﾝﾄﾞﾅﾝﾊﾞｰ出力
			send_to_dbus_zero();						// ﾃﾞｰﾀ出力関数0
			SEQ.FPGA_SEND_STATUS++;						// 次へ
			break;
			
		// C_ACKを「H」にする
		case 4:
			C_ACK_OUT	= 1;							// C_ACK
			SEQ.FPGA_SEND_STATUS++;						// 次へ
			break;
			
		// C_ACKを「L」にする
		case 5:
			C_ACK_OUT	= 0;							// C_ACK
			SEQ.FPGA_SEND_STATUS++;						// 次へ
			break;
			
		// ｺﾏﾝﾄﾞﾊﾞｽ・ﾃﾞｰﾀﾊﾞｽを「L」にする
		case 6:
			send_to_cbus_zero();						// ｺﾏﾝﾄﾞﾅﾝﾊﾞｰ出力関数0
			send_to_dbus_zero();						// ﾃﾞｰﾀ出力関数0
			SEQ.FPGA_SEND_STATUS++;						// 次へ
			break;
			
		// ﾎﾟｰﾄを入力に設定する
		case 7:
			bus_to_in();								// ﾊﾞｽを入力に設定
			SEQ.FPGA_SEND_STATUS++;						// 次へ
			break;
			
		// C_PRIOを「L」にする
		case 8:
			C_PRIO_OUT	= 0;							// C_PRIO
			SEQ.FPGA_SEND_STATUS = 11;
			break;
			
		// FPGA → RXにﾃﾞｰﾀ送信
		// F_PRIO_INが「H」になったらC_ACKを「H」にする
		case 11:
			if(C_PRIO_OUT == 1){
				C_PRIO_OUT = 0;
			}else if(F_PRIO_IN == 1){
				SEQ.FPGA_SEND_STATUS = 14;
				C_ACK_OUT	= 1;						// C_ACK
				C_ACK_OUT	= 0;						// C_ACK
			}
			break;
			
		// F_PRIO_INが「L」になったら、LED輝度信号返信のとき値の判定を行う
		case 14:
			if(DEBUG_STR.DEBUG_COUNT < 4096){
				if(DEBUG_STR.DEBUG_OUTPUT == 0){			// ﾃﾞｼﾞﾀﾙ値・ﾌﾞﾗｯｸ値	// 0 整数3桁
					DEBUG_STR.DEBUG[DEBUG_STR.DEBUG_COUNT] = SEQ.INPUT_DBUS_LONG;		// ﾃﾞﾊﾞｯｸﾞ出力用ﾃﾞｰﾀ
				}else if(DEBUG_STR.DEBUG_OUTPUT == 1){	// 処理済みﾃﾞｰﾀ				// 1 符号・整数1桁・小数点3桁
					// 浮動小数点のﾃﾞｰﾀに変換し値を表示する
					Variable.lLong = SEQ.INPUT_DBUS_LONG;
					SEQ.INPUT_DBUS = Variable.fFloat;
					DEBUG_STR.DEBUG[DEBUG_STR.DEBUG_COUNT] = SEQ.INPUT_DBUS * 1000;	// ﾃﾞﾊﾞｯｸﾞ出力用ﾃﾞｰﾀ(桁合わせのため1000倍している)
				}else if(DEBUG_STR.DEBUG_OUTPUT == 2){	// ﾎﾜｲﾄ値					// 2 整数3桁・小数点3桁
					// 浮動小数点のﾃﾞｰﾀに変換し値を表示する
					Variable.lLong = SEQ.INPUT_DBUS_LONG;
					SEQ.INPUT_DBUS = Variable.fFloat;
					DEBUG_STR.DEBUG[DEBUG_STR.DEBUG_COUNT] = SEQ.INPUT_DBUS * 1000;	// ﾃﾞﾊﾞｯｸﾞ出力用ﾃﾞｰﾀ(桁合わせのため1000倍している)
				}
			}
			DEBUG_STR.DEBUG_COUNT++;							// ﾃﾞﾊﾞｯｸﾞ出力用ｶｳﾝﾀ
			
			C_ACK_OUT	= 1;							// C_ACK
			C_ACK_OUT	= 0;							// C_ACK
			
			SEQ.FPGA_SEND_STATUS++;
			break;
			
		case 15:
			if(DEBUG_STR.DEBUG_COUNT <= 4096){
				SEQ.FPGA_SEND_STATUS = 11;
			}else{
				SEQ.CHANGE_FPGA = 0;
				SEQ.FPGA_SEND_STATUS = 1;
				COM2.SUB_STATUS++;							// 次へ

// add 2016.10.20 K.Uemura start	GA2002
				// 4096画素のﾃﾞｰﾀを、256点のﾃﾞｰﾀに縮小する（1/16）
				convert_data_4096_to_256( &DEBUG_STR.DEBUG[0], 0, &COM0.NO3000[0] );

				// TPD転送
				SEQ.BUFFER_NO_NEW = 512;
// add 2016.10.20 K.Uemura end
			}
			break;
	}
}
