/******************************************************************************
* File Name	: fpga_init.c
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

void send_to_fpga1(void);				// FPGAへのﾃﾞｰﾀ送信関数

//************************************************************/
//				FPGAへのﾃﾞｰﾀ送信関数(ﾌﾞﾗｯｸ・ﾎﾜｲﾄ)
//************************************************************/
// DEBUG用
void send_to_fpga1(void)
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
			SEQ.FPGA_SEND_STATUS++;						// 次へ
			break;
			
		// ｺﾏﾝﾄﾞﾊﾞｽ・ﾃﾞｰﾀﾊﾞｽを設定する
		case 3:
			SEQ.FLAG2.BIT.DIMMER = 0;					// 調光調整完了ﾌﾗｸﾞをﾘｾｯﾄ
			
			if(SEQ.CBUS_NUMBER == 200){					// 初期ﾊﾟﾗﾒｰﾀｾｯﾄ完了のとき
				SEQ.CBUS_NUMBER = 380;					// 初期ﾊﾟﾗﾒｰﾀｾｯﾄ
				
			}else if(SEQ.CBUS_NUMBER == 201){			// ﾌﾞﾗｯｸ固定値設定完了のとき
				SEQ.CBUS_NUMBER = 381;					// ﾌﾞﾗｯｸ固定値設定
				DA.DADR0 = 0;							// DA出力値をｾｯﾄ(最小)
				
			}else if(SEQ.CBUS_NUMBER == 203){			// ﾎﾜｲﾄ固定値設定完了のとき
				SEQ.CBUS_NUMBER = 383;					// ﾎﾜｲﾄ固定値設定
				DA.DADR0 = SEQ.LED_BRIGHTNESS;			// DA出力値をｾｯﾄ(DA0設定値)
			}
			
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
			
			if(SEQ.CBUS_NUMBER == 390){
				SEQ.FPGA_SEND_STATUS = 110;
			}else if(SEQ.CBUS_NUMBER == 394){
				SEQ.FPGA_SEND_STATUS = 110;
			}else{
				SEQ.FPGA_SEND_STATUS = 11;
			}
			break;
			
		// FPGA → RXにﾃﾞｰﾀ送信
		// F_PRIO_INが「H」になったらC_ACKを「H」にする
		case 11:
			if(SEQ.FLAG2.BIT.DIMMER == 0){				// 調光の調整が完了していないとき
				if(C_PRIO_OUT == 1){
					C_PRIO_OUT = 0;
				}else if(F_PRIO_IN == 1){
					SEQ.FPGA_SEND_STATUS++;
					C_ACK_OUT	= 1;						// C_ACK
				}
			}else{
				SEQ.FPGA_SEND_STATUS = 21;
			}
			break;
			
		case 12:
			SEQ.FPGA_SEND_STATUS++;						// 次へ
			C_ACK_OUT	= 0;							// C_ACK
			break;
			
		case 13:	// 割り込み
			SEQ.FPGA_SEND_STATUS++;						// 次へ
			//C_ACK_OUT	= 0;							// C_ACK
			// C_ACKを「L」にする
			break;
			
		// F_PRIO_INが「L」になったら、LED輝度信号返信のとき値の判定を行う
		case 14:
			//if(F_PRIO_IN == 0){
				if(SEQ.CBUS_NUMBER == 202){				// LED輝度信号返信のとき値の判定を行う
				
				}else{	// 202以外のとき
					SEQ.FPGA_SEND_STATUS++;
				}
			//}
			break;
			
		case 15:
			SEQ.FPGA_SEND_STATUS++;						// 次へ(割り込みがすぐに入る可能性があるため、あらかじめｽﾃｰﾀｽを進めておく)
			C_ACK_OUT	= 1;							// C_ACK
			break;
			
		case 16:
			if(SEQ.CBUS_NUMBER == 382){					// LED輝度信号返信のとき値の判定を行う
				if(SEQ.FLAG2.BIT.DIMMER == 0){			// 調光の調整が完了していないとき
					SEQ.FPGA_SEND_STATUS = 1;
				}else{
					SEQ.FPGA_SEND_STATUS = 18;
				}
			}else{
				SEQ.FPGA_SEND_STATUS = 18;
			}
			C_ACK_OUT	= 0;							// C_ACK
			break;
	}
}
