/******************************************************************************
* File Name	: fpga_verify.c
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

void send_to_fpga_verify(void);			// FPGAへのﾃﾞｰﾀ送信関数(ﾊﾟﾗﾒｰﾀ比較)

//************************************************************/
//				FPGAへのﾃﾞｰﾀ送信関数(ﾊﾟﾗﾒｰﾀ比較)
//	1 → ・・・ → 8 → 11 → 14 → 15 → 16 → 11
//	                                         → 1  [ 1]send_to_fpga
//	                                         → 11 [ 0]
//	                                         → 1  [15]send_to_fpga_hdi_check
//	                                         → 1  [ 2]send_to_fpga_idle
//************************************************************/
void send_to_fpga_verify(void)
{
	union UFloatLong Variable;
	
	switch(SEQ.FPGA_SEND_STATUS){
		// RX → FPGAにﾃﾞｰﾀ送信
		// C_PRIOを「H」にする
		case 1:
			//if(F_PRIO_IN == 0){
				SEQ.FPGA_PARAM_COUNT = 0;				// FPGAﾊﾟﾗﾒｰﾀ再送信ｶｳﾝﾄ add 150508
				SEQ.FLAG6.BIT.VERIFY_ERROR = 0;			// ﾊﾟﾗﾒｰﾀ確認ｴﾗｰﾌﾗｸﾞ
				
				C_PRIO_OUT	= 1;						// C_PRIO
				SEQ.CBUS_NUMBER		= 388;				// CBUSﾅﾝﾊﾞｰをｾｯﾄ(ﾊﾟﾗﾒｰﾀ比較)
				SEQ.FPGA_SEND_STATUS++;
			//}
			break;
			
		// ﾎﾟｰﾄを出力に設定する
		case 2:
			bus_to_out();								// ﾊﾞｽ出力変更関数
			
			send_to_cbus_zero();						// ｺﾏﾝﾄﾞﾅﾝﾊﾞｰ出力関数0
			send_to_dbus_zero();						// ﾃﾞｰﾀ出力関数0
			
			SEQ.FPGA_SEND_STATUS++;
			break;
			
		// ｺﾏﾝﾄﾞﾊﾞｽ・ﾃﾞｰﾀﾊﾞｽを設定する
		case 3:
			send_to_cbus(SEQ.CBUS_NUMBER);
			send_to_dbus_zero();						// ﾃﾞｰﾀ出力関数0
			SEQ.FPGA_SEND_STATUS++;
			break;
			
		// C_ACKを「H」にする
		case 4:
			C_ACK_OUT	= 1;							// C_ACK
			SEQ.FPGA_SEND_STATUS++;
			break;
			
		// C_ACKを「L」にする
		case 5:
			C_ACK_OUT	= 0;							// C_ACK
			SEQ.FPGA_SEND_STATUS++;
			break;
			
		// ｺﾏﾝﾄﾞﾊﾞｽ・ﾃﾞｰﾀﾊﾞｽを「L」にする
		case 6:
			send_to_cbus_zero();						// ｺﾏﾝﾄﾞﾅﾝﾊﾞｰ出力関数0
			send_to_dbus_zero();						// ﾃﾞｰﾀ出力関数0
			SEQ.FPGA_SEND_STATUS++;
			break;
			
		// ﾎﾟｰﾄを入力に設定する
		case 7:
			bus_to_in();								// ﾊﾞｽ入力変更関数
			SEQ.FPGA_SEND_STATUS++;
			break;
			
		// C_PRIOを「L」にする
		case 8:
			C_PRIO_OUT	= 0;							// C_PRIO
			SEQ.FPGA_SEND_STATUS = 11;
			break;
			
		// FPGA → RXにﾃﾞｰﾀ送信
		// F_PRIO_INが「H」になったらC_ACKを「H」にする
		case 11:
			if(C_ACK_OUT == 1){
				C_ACK_OUT	= 0;
			}else{
				if(F_PRIO_IN == 1){
					C_ACK_OUT	= 1;				// C_ACK
					C_ACK_OUT	= 0;				// C_ACK
					SEQ.FPGA_SEND_STATUS = 14;
				}
			}
			break;
			
		// F_PRIO_INが「L」になったらﾎﾟｰﾄを入力に設定する
		case 14:
			// 4～19は値を変換する
			switch(SEQ.CBUS_NUMBER){
				case 4:		// 黒比率傾斜大ﾚﾍﾞﾙ
				case 5:		// 黒比率傾斜小ﾚﾍﾞﾙ
				case 6:		// ｽｷｯﾌﾟ係数
				case 7:		// 荒ｴｯｼﾞｻｰﾁ係数
					// FLOAT型をLONG型に変更し100倍する
					Variable.lLong = SEQ.INPUT_DBUS_LONG;
					SEQ.INPUT_DBUS = Variable.fFloat;
					SEQ.INPUT_DBUS_LONG = SEQ.INPUT_DBUS * 100;
					break;
					
				case 8:		// HDI0　開始
				case 9:		// HDI0　終了
				case 10:	// HDI1　開始
				case 11:	// HDI1　終了
				case 12:	// HDI2　開始
				case 13:	// HDI2　終了
				case 14:	// HDI3　開始
				case 15:	// HDI3　終了
				case 16:	// HDI4　開始
				case 17:	// HDI4　終了
				case 18:	// HDI5　開始
				case 19:	// HDI5　終了
					// HDI用のﾃﾞｰﾀをﾊﾟﾗﾒｰﾀのﾃﾞｰﾀにｺﾋﾟｰする
					// 計測方向がZ方向のとき
					if(SEQ.MEASUREMENT_DIRECTION == Z_DIRECTION)	SEQ.PARA_DATA[SEQ.CBUS_NUMBER] = SEQ.Z_HDI_DATA[SEQ.CBUS_NUMBER-7];
					// 計測方向がX方向のとき
					else											SEQ.PARA_DATA[SEQ.CBUS_NUMBER] = SEQ.X_HDI_DATA[SEQ.CBUS_NUMBER-7];
					
					break;
			}
			
			// 受信ﾃﾞｰﾀとﾊﾟﾗﾒｰﾀﾃﾞｰﾀが異なるときｴﾗｰとする
			if(SEQ.INPUT_DBUS_LONG != SEQ.PARA_DATA[SEQ.CBUS_NUMBER]){
				SEQ.FLAG6.BIT.VERIFY_ERROR = 1;				// ﾊﾟﾗﾒｰﾀ確認ｴﾗｰﾌﾗｸﾞ
			}
			SEQ.FPGA_SEND_STATUS++;
			
			break;
			
		// C_PRIOを「H」にする
		case 15:
			SEQ.FPGA_SEND_STATUS++;
			C_ACK_OUT	= 1;								// C_ACK
			C_ACK_OUT	= 0;								// C_ACK
			break;
			
		// ﾎﾟｰﾄを出力に設定する
		case 16:
			//C_ACK_OUT	= 0;								// C_ACK
			SEQ.FPGA_SEND_STATUS = 11;
			
			if(SEQ.CBUS_NUMBER == 42){
				if(SEQ.FLAG6.BIT.VERIFY_ERROR == 1){		// ﾊﾟﾗﾒｰﾀ確認ｴﾗｰﾌﾗｸﾞが「1」のとき
					// 再送
					SEQ.CHANGE_FPGA = 1;
					SEQ.FPGA_SEND_STATUS = 1;
					SEQ.POWER_COUNT = 0;					// 電源制御ｶｳﾝﾄをﾘｾｯﾄ

// add 2016.10.18 K.Uemura start	GA2001
//					SEQ.FLAG6.BIT.VERIFY_COUNT++;
//
//					// verifyﾘﾄﾗｲｴﾗｰ
//					if(SEQ.FLAG6.BIT.VERIFY_COUNT == 100){
//						SEQ.CHANGE_FPGA = 0;
//						COM0.NO312 = ERR_PARAMETER_VERIFY;
//						SEQ.FLAG6.BIT.VERIFY_COUNT = 0;
//					}
// add 2016.10.18 K.Uemura end
				}else{
					// 測定
					SEQ.FLAG.BIT.PARAM_INITIAL = 0;
					SEQ.CHANGE_FPGA = 0;
					SEQ.POWER_COUNT = 250;					// 電源制御ｶｳﾝﾄをﾘｾｯﾄ
					
					SEQ.MEASUREMENT_DIRECTION_BEFORE = 2;	// 計測方向(直前)をX・Z以外にする
					
					if(SEQ.FLAG.BIT.MEASUREMENT == 1){
						SEQ.CBUS_NUMBER = 394;
						SEQ.CHANGE_FPGA = 15;				// HDI出力検査
						SEQ.FPGA_SEND_STATUS = 1;
// add 2016.02.18 K.Uemura start	G21804
						// 清掃確認の場合、hdi_checkを実行しない
// chg 2016.06.22 K.Uemura start	G62202
//						if(COM0.NO301 == 152){
						if((COM0.NO301 == 152)||(COM0.NO301 == 153)){
// chg 2016.06.22 K.Uemura end
							SEQ.CBUS_NUMBER = 395;
							SEQ.CHANGE_FPGA = 2;						// 測定
							SEQ.FPGA_SEND_STATUS = 1;
						}
// add 2016.02.18 K.Uemura end
					}
					// ADD 170413
					else{
						if(COM0.NO300.BIT.RST){					// 強制ﾘｾｯﾄがONのとき
							COM0.NO310.BIT.RDY = 1;				// READY
							COM0.NO310.BIT.FIN = 1;				// 完了
						}else{
							COM0.NO310.BIT.RDY = 1;				// READY
						}
					}
					//
				}
			}
			break;
	}
}
