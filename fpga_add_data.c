/******************************************************************************
* File Name	: fpga_add_data.c
******************************************************************************/
#include <machine.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mathf.h>
#include "iodefine.h"
#include "typedefine.h"
#include "user.h"
#include "user_define.h"

/* Command numbers and end-of-cycle IDs: define for readability */
#define CBUS_CMD_ADD_DATA      387
#define CBUS_LAST_WITH_ADD     218
#define CBUS_LAST_NORMAL       215

void send_to_fpga_add_data(void);		// FPGAへのﾃﾞｰﾀ送信関数(平均値・最小値・最大値のﾃﾞｰﾀ出力設定)

//********************************************************************************/
//				FPGAへのﾃﾞｰﾀ送信関数(平均値・最小値・最大値のﾃﾞｰﾀ出力設定)
//********************************************************************************/
// 予備(302)の値「0」のときﾃﾞｰﾀ出力しない	(通常)
// 					「1」のときﾃﾞｰﾀ出力する		(通常の計測に平均値・最小値・最大値のﾃﾞｰﾀを付加)
void send_to_fpga_add_data(void)
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
			SEQ.CBUS_NUMBER = CBUS_CMD_ADD_DATA;        // ﾃﾞｰﾀ出力設定
			send_to_cbus(SEQ.CBUS_NUMBER);               // ｺﾏﾝﾄﾞﾅﾝﾊﾞｰ出力
			send_to_dbus(COM0.NO302);                    // ﾃﾞｰﾀ出力関数
			// 予備(302)の値    0:ﾃﾞｰﾀ出力しない    1:ﾃﾞｰﾀ出力する
			if (COM0.NO302 == 1)        SEQ.LAST_CBUS_NUMBER = CBUS_LAST_WITH_ADD;    // CBUSﾅﾝﾊﾞｰ(1ｻｲｸﾙ最終)    (平均値・最小値・最大値のﾃﾞｰﾀを付加)
			else                        SEQ.LAST_CBUS_NUMBER = CBUS_LAST_NORMAL;      // CBUSﾅﾝﾊﾞｰ(1ｻｲｸﾙ最終)    (通常)
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
			SEQ.CHANGE_FPGA = 0;
			SEQ.FPGA_SEND_STATUS = 11;
			
			SEQ.TP_CONTROL_STATUS++;
			COM0.NO310.BIT.RDY = 1;						// READYのﾋﾞｯﾄON
			break;

		default:
			/* 安全フェール: 想定外の状態が来たら初期状態に戻す */
			C_PRIO_OUT = 0;
			C_ACK_OUT = 0;
			SEQ.FPGA_SEND_STATUS = 1;
			break;
	}
}
