/******************************************************************************
* File Name	: fpga_pixel.c
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
#include <stddef.h>

void send_to_fpga_pixel(void);					// FPGAへのﾃﾞｰﾀ送信関数(特定画素)

//************************************************************/
//				FPGAへのﾃﾞｰﾀ送信関数(特定画素)
//************************************************************/
// 計測
void send_to_fpga_pixel(void)
{
	union UFloatLong Variable;
	
	switch(SEQ.FPGA_SEND_STATUS){
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
			if(SEQ.CBUS_NUMBER == 390){
				SEQ.FLAG.BIT.AFTER_STOPPING = 1;
				SEQ.CHANGE_FPGA = 9;
			}
			SEQ.FPGA_SEND_STATUS = 11;
			break;
	
		// FPGA → RXにﾃﾞｰﾀ送信
		// F_PRIO_INが「H」になったらC_ACKを「H」にする
		case 11:
			if(C_PRIO_OUT == 1){
				C_PRIO_OUT = 0;
			}else if(F_PRIO_IN == 1){
				SEQ.FPGA_SEND_STATUS = 14;
				C_ACK_OUT	= 1;					// C_ACK
				C_ACK_OUT	= 0;					// C_ACK
			}else{
				// 4096個のﾃﾞｰﾀを取得し終わるとFPGAは計測を停止するのでしばらくすると停止処理を行う
				SEQ.PIXEL_END_COUNT++;				// 特定画素終了ｶｳﾝﾄ
				if(SEQ.PIXEL_END_COUNT >= 1000){
					SEQ.FLAG.BIT.AFTER_STOPPING = 1;
					SEQ.CHANGE_FPGA = 9;
					SEQ.FPGA_SEND_STATUS = 21;
					//SEQ.PIXEL_END_FLAG = 1;			// 特定画素終了ﾌﾗｸﾞ
					SEQ.FLAG6.BIT.PIXEL_END = 1;		// 特定画素終了ﾌﾗｸﾞ
				}
			}
			break;
			
		case 14:	// 割り込み後
			switch(SEQ.CBUS_NUMBER){
				case 211:		// 左ｴｯｼﾞ最小位置
					SEQ.BUFFER_COUNT = 2;						// ﾊﾞｯﾌｧ
					set_result_edge_left();			// 左ｴｯｼﾞ検出結果の設定、座標変換
					break;
					
				case 212:		// 右ｴｯｼﾞ最大位置
					set_result_edge_right();		// 右ｴｯｼﾞ検出結果の設定、座標変換
					break;
					
				case 213:		// 左ｴｯｼﾞ傾斜
					set_result_focus_left();		// 左ｴｯｼﾞ焦点結果の設定
					break;
					
				case 214:		// 右ｴｯｼﾞ傾斜
					set_result_focus_right();		// 右ｴｯｼﾞ焦点結果の設定
					break;
					
				case 215:		// ｴｯｼﾞ間の差
					// 浮動小数点のﾃﾞｰﾀに変換し値を表示する
					Variable.lLong = SEQ.INPUT_DBUS_LONG;
					SEQ.INPUT_DBUS = Variable.fFloat;
					SEQ.INPUT_DBUS_LONG = (long)(SEQ.INPUT_DBUS * 10.0);
					COM0.NO105 = SEQ.INPUT_DBUS_LONG;
					
					result_output();				// 結果出力
					
					SEQ.PIXEL_END_COUNT = 0;				// 特定画素終了ｶｳﾝﾄ
					break;

				case 223:		// ｽｷｯﾌﾟ出力
					set_result_skip();
					hdi_output();					// HDI出力
					break;
			}
			SEQ.FPGA_SEND_STATUS++;
			break;
			
		// C_ACKを「H」にする
		case 15:
			SEQ.FPGA_SEND_STATUS++;
			C_ACK_OUT	= 1;						// C_ACK
			break;
			
		// C_ACKを「L」にする
		case 16:
			C_ACK_OUT	= 0;						// C_ACK
			SEQ.FPGA_SEND_STATUS = 11;
			
			//if(SEQ.CBUS_NUMBER == 215){					// 右ｴｯｼﾞﾃﾞｰﾀのとき
			if(SEQ.CBUS_NUMBER == SEQ.LAST_CBUS_NUMBER){	// CBUSﾅﾝﾊﾞｰ(1ｻｲｸﾙ最終)のとき
			
				SEQ.FPGA_SEND_STATUS = 21;
				
				// 100ms間隔でﾃﾞｰﾀ送信
				if(SEQ.FLAG.BIT.BUFFER_RESET == 1){			// ﾊﾞｯﾌｧｶｳﾝﾄが「0」のとき
					SEQ.FLAG.BIT.BUFFER_RESET = 0;			// ﾊﾞｯﾌｧﾘｾｯﾄﾌﾗｸﾞ
					COM0.NO310.BIT.RDY = 0;					// READYのﾋﾞｯﾄをOFFにし状態番号をｾｯﾄする
				}
				//
			}
			break;
			
		// C_ACKを「H」にする
		case 17:
			SEQ.FPGA_SEND_STATUS = 13;
			C_ACK_OUT	= 1;						// C_ACK
			break;
			
		// F_PRIO_INが「L」になっていることを確認する
		case 21:
			//if(F_PRIO_IN == 0){					// F_PRIO_INが「L」のとき
				SEQ.FPGA_SEND_STATUS = 11;
			//}
			break;
			
		case 22:
			if(SEQ.FLAG.BIT.BUFFER_RESET == 1){				// ﾊﾞｯﾌｧｶｳﾝﾄが「0」のとき
				SEQ.FLAG.BIT.BUFFER_RESET = 0;				// ﾊﾞｯﾌｧﾘｾｯﾄﾌﾗｸﾞ
				SEQ.FPGA_SEND_STATUS = 11;
				
				OUT.SUB_STATUS = 3;		// "-----"
			}
			break;
	}
}
