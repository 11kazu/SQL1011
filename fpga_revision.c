/******************************************************************************
* File Name	: fpga_revision.c
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

void send_to_fpga_revision(void);		// FPGAへのﾃﾞｰﾀ送信関数(ﾊﾞｰｼﾞｮﾝ取得)

//************************************************************/
//				FPGAへのﾃﾞｰﾀ送信関数(ﾊﾞｰｼﾞｮﾝ取得)
//************************************************************/
// ﾊﾞｰｼﾞｮﾝ取得
void send_to_fpga_revision(void)
{
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
			send_to_cbus_zero();						// ｺﾏﾝﾄﾞﾅﾝﾊﾞｰ出力関数0
			SEQ.FPGA_SEND_STATUS++;						// 次へ
			break;
			
		// ｺﾏﾝﾄﾞﾊﾞｽ・ﾃﾞｰﾀﾊﾞｽを設定する
		case 3:
			SEQ.CBUS_NUMBER = 386;						// ﾊﾞｰｼﾞｮﾝ取得
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
				C_ACK_OUT	= 1;					// C_ACK
				C_ACK_OUT	= 0;					// C_ACK
			}
			break;
			
		// F_PRIO_INが「L」になったら、LED輝度信号返信のとき値の判定を行う
		case 14:
			if(SEQ.CBUS_NUMBER == 386){						// ﾊﾞｰｼﾞｮﾝ取得
				SEQ.FPGA_RIVISION = SEQ.INPUT_DBUS_LONG;	// ﾊﾞｰｼﾞｮﾝ取得
				
				// ADD 150728
				COM0.NO138 = SEQ.FPGA_RIVISION;				// FPGAﾊﾞｰｼﾞｮﾝ
				COM0.NO139 = RX_RIVISION;					// RXﾊﾞｰｼﾞｮﾝ
			}
			
			SEQ.FPGA_SEND_STATUS++;
			break;
			
		case 15:
			SEQ.FPGA_SEND_STATUS++;						// 次へ(割り込みがすぐに入る可能性があるため、あらかじめｽﾃｰﾀｽを進めておく)
			C_ACK_OUT	= 1;							// C_ACK
			break;
			
		case 16:
			C_ACK_OUT	= 0;							// C_ACK
			
			if(SEQ.CBUS_NUMBER == 386){					// ﾊﾞｰｼﾞｮﾝ取得
				SEQ.FPGA_SEND_STATUS++;
				SEQ.CHANGE_FPGA = 0;
			}
			break;
	}
}
