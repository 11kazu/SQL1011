/******************************************************************************
* File Name	: fpga_profile.c
******************************************************************************/
#include <machine.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mathf.h>
#include <limits.h>
#include "iodefine.h"
#include "typedefine.h"
#include "user.h"
#include "user_define.h"

void send_to_fpga_profile(void);				// FPGAへのﾃﾞｰﾀ送信関数(ﾌﾟﾛﾌｧｲﾙ処理)



//************************************************************/
//				FPGAへのﾃﾞｰﾀ送信関数(ﾌﾟﾛﾌｧｲﾙ処理)
//	22 → 11 → 14 → 15 → 16 → 11
//	                           → 21 → 11
//	                                 → 2
//	                                 → 22
//************************************************************/
// ﾌﾟﾛﾌｧｲﾙ処理
void send_to_fpga_profile(void)
{
	float temp;
	long result;
	long work[2];

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
			send_to_cbus_zero();						// ｺﾏﾝﾄﾞﾅﾝﾊﾞｰ出力関数0
			send_to_dbus_zero();						// ﾃﾞｰﾀ出力関数0
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
			}
			break;
			
		case 14:	// 割り込み後
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
			if(SEQ.CBUS_NUMBER == SEQ.LAST_CBUS_NUMBER){	// CBUSﾅﾝﾊﾞｰ(1ｻｲｸﾙ最終)のとき
				SEQ.FPGA_SEND_STATUS = 21;
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
			
			if(COM0.NO300.BIT.EXE == 0){			// ﾌﾟﾛﾌｧｲﾙで実行していないとき
				SEQ.FLAG.BIT.MEASUREMENT = 0;		// 計測を停止する
			}
			
			if(SEQ.FLAG.BIT.AFTER_STOPPING == 0){
				if(F_PRIO_IN == 0){					// F_PRIO_INが「L」のとき
					if(SEQ.FLAG.BIT.MEASUREMENT == 0){
						// 待機を送信(390)
						C_PRIO_OUT	= 1;			// C_PRIO
						SEQ.CBUS_NUMBER = 390;
						SEQ.FPGA_SEND_STATUS = 2;
					}else{
						if(SEQ.FLAG6.BIT.PROFILE_PROCESSING == 1){	// ﾌﾟﾛﾌｧｲﾙ処理ﾌﾗｸﾞ
							SEQ.FLAG6.BIT.PROFILE_PROCESSING = 0;	// ﾌﾟﾛﾌｧｲﾙ処理ﾌﾗｸﾞ
							
							//
							SEQ.FLAG3.BIT.PEAKHOLD_ENABLE = 0;		// ﾋﾟｰｸﾎｰﾙﾄﾞを中断(measureまで待機)
							SEQ.BUFFER_COUNT = 0;					// ﾊﾞｯﾌｧｶｳﾝﾄをﾘｾｯﾄ
							SEQ.TUNING_COUNT = 0;					// ﾁｭｰﾆﾝｸﾞｶｳﾝﾄ
							SEQ.FPGA_RESTART_COUNT = 0;				// FPGA再ｽﾀｰﾄｶｳﾝﾄ
							
							SEQ.INPUT_DBUS_BEFORE[1] = 0;			// DBUSの入力ﾃﾞｰﾀ(1つ前のﾃﾞｰﾀ)
							SEQ.INPUT_DBUS_BEFORE[2] = 0;			// DBUSの入力ﾃﾞｰﾀ(1つ前のﾃﾞｰﾀ)
							
							SEQ.END_TIMEOUT_PERIOD	= 0;			// ﾀｲﾑｱｳﾄ時間
							SEQ.OK_COUNT		= 0;				// OKｶｳﾝﾄ数
							SEQ.NG_COUNT		= 0;				// 連続NGｶｳﾝﾄ数
							SEQ.TOTAL_COUNT		= 0;				// 計測総数
							SEQ.FLAG3.BIT.AUTO_ERROR = 0;			// 自動ｴﾗｰﾌﾗｸﾞ
							SEQ.FLAG3.BIT.TUNING_ERROR = 0;
							
							LED.FOCUS.BIT.R = 0;					// 焦点R
							LED.FOCUS.BIT.L = 0;					// 焦点L
							LED.FOCUS.BIT.Z = 0;					// 焦点Z
							
							SEQ.FLAG6.BIT.PROFILE_PROCESSING = 0;	// ﾌﾟﾛﾌｧｲﾙ処理ﾌﾗｸﾞ
							
							SEQ.FLAG3.BIT.SWING_RESET = 0;
							
							SEQ.FLAG3.BIT.HDI_OUTPUT = 0;			// HDI出力ﾌﾗｸﾞ
							SEQ.FOCUSING_HDI = 0;					// 11ﾋﾞｯﾄ右にｼﾌﾄ HDI出力を初期化
							LED.FOCUSING = 0;						// X軸LED
							LED.Z_FOCUSING = 0;						// Z軸LED

							SEQ.INPUT_DBUS_LONG = 0;
							set_result_skip();
							
							COM0.NO313 = 0;	// 313 計測結果 現在値(REAL)	上位
							COM0.NO314 = 0;	// 314 計測結果 現在値(REAL)	下位
							COM0.NO315 = 0;	// 315 計測結果 最終値(d)		上位
							COM0.NO316 = 0;	// 316 計測結果 最終値(d)		下位
							COM0.NO317 = 0;	// 317 計測結果 最終値(D)		上位
							COM0.NO318 = 0;	// 318 計測結果 最終値(D)		下位
							COM0.NO319 = 0;	// 319 計測結果 最終値(ΔX)		上位
							COM0.NO320 = 0;	// 320 計測結果 最終値(ΔX)		下位
							COM0.NO321 = 0;	// 321 計測結果 最終値(振れ)	上位
							COM0.NO322 = 0;	// 322 計測結果 最終値(振れ)	下位
							max_min_reset();						// 最大値・最小値ﾘｾｯﾄ
							
							SEQ.TP_CONTROL_STATUS = 3;
							OUT.SUB_STATUS = 11;
							
							if(SEQ.TUNING_ENABLE == 1){					// ﾁｭｰﾆﾝｸﾞ「有」のとき
								if(SEQ.TUNING_SECONDS > 0){				// ﾁｭｰﾆﾝｸﾞ秒数が「0」より上のとき
									SEQ.BUFFER_COUNT = 2;
									SEQ.CHANGE_FPGA = 5;				// ﾁｭｰﾆﾝｸﾞ処理へ
								}
							}else{
								SEQ.CHANGE_FPGA = 6;					// 本計測へ
							}
							SEQ.FPGA_SEND_STATUS = 22;
							//
						}
					}
				}
			}
			break;
			
		case 22:
			if(SEQ.FLAG.BIT.BUFFER_RESET == 1){									// ﾊﾞｯﾌｧｶｳﾝﾄが「0」のとき
				SEQ.FLAG.BIT.BUFFER_RESET = 0;									// ﾊﾞｯﾌｧﾘｾｯﾄﾌﾗｸﾞ
				
				//
				if(COM0.NO300.BIT.EXE){											// 実行
					COM0.NO310.BIT.STR = 1;										// STROBE
					SEQ.FLAG2.BIT.PROFILE_AUTO = 1;								// ﾌﾟﾛﾌｧｲﾙ自動判別済ﾌﾗｸﾞ
					
					if(SEQ.FLAG2.BIT.AUTO_MODE == MODE_D4_AUTO_LOW){			// 「1:工具径(d≦4)」
						if((SEQ.FLAG4.BIT.EDGE_L_SCAN == 1) && (SEQ.FLAG4.BIT.EDGE_R_SCAN == 1)){
							temp = RESULT.LARGE_D_DIFF[0];
						}
					}else if(SEQ.FLAG2.BIT.AUTO_MODE == MODE_D4_AUTO_LEFT){		// 「2:工具径(d＞4 左側)」
						if(SEQ.FLAG4.BIT.EDGE_R_SCAN == 1){
							temp = RESULT.GREATER_D_DIFF_MAX[0];
						}
					}else if(SEQ.FLAG2.BIT.AUTO_MODE == MODE_D4_AUTO_RIGHT){	// 「3:工具径(d＞4 右側)」
						if(SEQ.FLAG4.BIT.EDGE_L_SCAN == 1){
							temp = RESULT.GREATER_D_DIFF_RIGHT_MIN[0];
						}
					}

// add 2016.09.08 K.Uemura start	G90801
					if(SEQ.FLAG2.BIT.AUTO_MODE != MODE_D4_AUTO_LOW){
						if((RESULT_SIGN == 1) && (SEQ.MEASUREMENT_DIRECTION == X_DIRECTION)){
							temp *= -1;
						}
					}
// add 2016.09.08 K.Uemura end

					result = temp * 10.0;
					COM0.NO3000[SEQ.PROFILE_BUFFER_COUNT] = (short)result;
// add 2015.08.19 K.Uemura start	プロファイル最大／最小出力
					SEQ.PROFILE_OUTPUT_COUNT++;									// プロファイル計測出力ｶｳﾝﾄ
					COM0.NO321 = (SEQ.PROFILE_OUTPUT_COUNT) >> 16;
					COM0.NO322 = (SEQ.PROFILE_OUTPUT_COUNT);

					if(SEQ.PROFILE_OUTPUT_COUNT == 1){
//						RESULT.PROFILE_MIN = SHRT_MAX;
//						RESULT.PROFILE_MAX = SHRT_MIN;
						RESULT.PROFILE_MIN = RESULT.PROFILE_MAX = (short)COM0.NO3000[SEQ.PROFILE_BUFFER_COUNT];
					}else{
						RESULT.PROFILE_MIN = min((short)COM0.NO3000[SEQ.PROFILE_BUFFER_COUNT], RESULT.PROFILE_MIN);
						RESULT.PROFILE_MAX = max(RESULT.PROFILE_MAX, (short)COM0.NO3000[SEQ.PROFILE_BUFFER_COUNT]);
					}

					//最大、最小出力
					COM0.NO315 = (RESULT.PROFILE_MIN) >> 16;
					COM0.NO316 = (RESULT.PROFILE_MIN);
					COM0.NO317 = (RESULT.PROFILE_MAX) >> 16;
					COM0.NO318 = (RESULT.PROFILE_MAX);
					//最上位ビットが1の時は負の数
					if(result < 0){
						COM0.NO319 = 0xFFFF;			// -1を設定
					}else{
						COM0.NO319 = 0;
					}
					COM0.NO320 = COM0.NO3000[SEQ.PROFILE_BUFFER_COUNT];

					//TPD表示グラフのスケール調整
					if(RESULT.PROFILE_MAX < 0){
						//マイナス方向
						temp = abs(RESULT.PROFILE_MIN + abs(RESULT.PROFILE_MAX)) / 10.0;
					}else{
						temp = (RESULT.PROFILE_MAX - RESULT.PROFILE_MIN) / 10.0;
					}
					//調整単位が1以下の場合は、強制的に1を設定
					if(temp < 1){
						temp = 1;
					}

					work[0] = RESULT.PROFILE_MIN;
					work[1] = RESULT.PROFILE_MAX;
					COM0.NO325 = work[0] - temp;
					COM0.NO326 = work[1] + temp;

//					if((short)COM0.NO3000[SEQ.PROFILE_BUFFER_COUNT] < (short)COM0.NO325){
//						COM0.NO325 = COM0.NO3000[SEQ.PROFILE_BUFFER_COUNT];		// 最小
//					}
//					if((short)COM0.NO326 < (short)COM0.NO3000[SEQ.PROFILE_BUFFER_COUNT]){
//						COM0.NO326 = COM0.NO3000[SEQ.PROFILE_BUFFER_COUNT];		// 最大
//					}
// add 2015.08.19 K.Uemura end
					
					SEQ.BUFFER_NO_NEW = SEQ.PROFILE_BUFFER_COUNT;				// ﾊﾞｯﾌｧﾅﾝﾊﾞｰ新
					SEQ.PROFILE_BUFFER_COUNT++;									// ﾌﾟﾛﾌｧｲﾙﾊﾞｯﾌｧ格納用ｶｳﾝﾄ
					if(SEQ.PROFILE_BUFFER_COUNT >= BUFFER_NUMBER)	SEQ.PROFILE_BUFFER_COUNT = 0;
					
				}else{
					COM0.NO310.BIT.RDY = 1;										// READY
				}
				//
				
				SEQ.FPGA_SEND_STATUS = 11;
			}
			break;
	}
}
