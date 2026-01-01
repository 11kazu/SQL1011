/******************************************************************************
* File Name	: fpga_measure_stop.c
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

void send_to_fpga_measure_stop(void);				// FPGAへのﾃﾞｰﾀ送信関数(計測停止)



//************************************************************/
//				FPGAへのﾃﾞｰﾀ送信関数(計測停止)
//	11 → 14 → 15 → 16 → 11
//	                     → 30 → 21 → 11 → 
//	                                 → 2 → ・・・ → 8 → 11 → 
//	                                 → 1  [ 2]再計測(send_to_fpga_idleへ)
//	                                 → 1  [ 1]パラメータ転送(send_to_fpgaへ)
//	                                 → 71 [ 0]測定終了
//	                                 → 1  [10]デバッグ出力(send_to_fpga_debugへ)
//************************************************************/
// 計測停止
void send_to_fpga_measure_stop(void)
{
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
				
			// ADD 170518
			}else{	// F_PRIO_INが「1」にならないとき一旦停止する
				SEQ.FPGA_RESTART_COUNT++;				// FPGA再ｽﾀｰﾄｶｳﾝﾄ
				if(SEQ.FPGA_RESTART_COUNT >= 100){
					SEQ.FPGA_RESTART_COUNT = 0;
					SEQ.FPGA_SEND_STATUS = 21;
				}
				
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
// chg 2016.01.21 K.Uemura start	G12102
				SEQ.FPGA_SEND_STATUS = 30;
//				SEQ.FPGA_SEND_STATUS = 21;
// chg 2016.01.21 K.Uemura end
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
			
			if(SEQ.FLAG.BIT.AFTER_STOPPING == 0){
				if(F_PRIO_IN == 0){					// F_PRIO_INが「L」のとき
					if(SEQ.FLAG.BIT.MEASUREMENT == 0){
						// 待機を送信(390)
						C_PRIO_OUT	= 1;			// C_PRIO
						SEQ.CBUS_NUMBER = 390;
						SEQ.FPGA_SEND_STATUS = 2;
					}
				}
			}else{
				SEQ.FLAG.BIT.AFTER_STOPPING = 0;
				
				// HDIﾁｪｯｸの終了処理
				if(SEQ.FLAG6.BIT.HDI_CHECK_COMPLETION == 1){			// HDIﾁｪｯｸ完了ﾌﾗｸﾞ
					SEQ.FLAG6.BIT.HDI_CHECK_COMPLETION = 0;
					SEQ.FLAG.BIT.MEASUREMENT = 1;
					// 本計測へ移動
					if(SEQ.FLAG6.BIT.HDI_CHECK_OK == 1){			// HDIﾁｪｯｸOKﾌﾗｸﾞ
						SEQ.CBUS_NUMBER = 394;
						SEQ.CHANGE_FPGA = 2;						// 測定
						SEQ.FPGA_SEND_STATUS = 1;
						
						OUT.SUB_STATUS = 11;
						
					// ﾊﾟﾗﾒｰﾀの設定に戻る
					}else{
						SEQ.FLAG.BIT.PARAM_INITIAL = 1;				// ﾊﾟﾗﾒｰﾀ初期化ﾌﾗｸﾞ
						SEQ.CHANGE_FPGA = 1;						// ﾊﾟﾗﾒｰﾀ
						SEQ.FPGA_SEND_STATUS = 1;
						SEQ.POWER_COUNT = 0;						// 電源制御ｶｳﾝﾄをﾘｾｯﾄ
					}
					
				// 通常の終了処理
				}else{
// add 2016.02.18 K.Uemura start	G21804
					SEQ.FLAG6.BIT.CLEANING = 0;
					
// chg 2016.06.22 K.Uemura start	G62202
					if(((COM0.NO311 == 152)||((COM0.NO311 == 153) && (SEQ.MEASUREMENT_DIRECTION == X_DIRECTION)))&&(COM0.NO300.BIT.EXE)){
//					if((COM0.NO311 == 152)&&(COM0.NO300.BIT.EXE)){
// chg 2016.06.22 K.Uemura end
						SEQ.FLAG6.BIT.CLEANING = 1;
					// ADD 170413
					}else if(((COM0.NO311 == 152)||(COM0.NO311 == 153))&&(COM0.NO300.BIT.EXE == 0)){
						SEQ.FPGA_SEND_STATUS = 71;
						SEQ.CHANGE_FPGA = 0;
						DA.DADR0 = 0;								// DA0
					//
					}else{
						SEQ.FPGA_SEND_STATUS = 71;
						SEQ.CHANGE_FPGA = 0;
						DA.DADR0 = 0;								// DA0
// add 2016.10.20 K.Uemura start	GA2002
						if((COM0.NO311 == 710) || (COM0.NO311 == 711)){
							OUT.SUB_STATUS = 3;					// "-----"
						}
// add 2016.10.20 K.Uemura end
					}
// add 2016.02.18 K.Uemura end
					
#ifdef	DEBUG_OUTPUT232C
#ifndef	__LONGGING_SENSOR_LEVEL
					COM2.MASTER_STATUS = DRV_MODE;
#endif
					COM2.SUB_STATUS = 1;
#endif
				
					// ﾌﾟﾛﾌｧｲﾙのとき
					if(SEQ.SELECT.BIT.MEASURE == MODE_PROFILE){
						if(COM0.NO300.BIT.EXE){					// 実行
// add 2016.03.02 K.Uemura start	G30201
#if	0
//// add 2016.01.22 K.Uemura start	G12201
//							if(COM0.NO300.BIT.RST == 0){
//// add 2016.01.22 K.Uemura end
//								COM0.NO310.BIT.STR = 1;			// STROBE
//							}
//							SEQ.FLAG2.BIT.PROFILE_AUTO = 1;		// ﾌﾟﾛﾌｧｲﾙ自動判別済ﾌﾗｸﾞ
//							
//#if	1
//							if(SEQ.FLAG2.BIT.AUTO_MODE == MODE_D4_AUTO_LOW){			// 「1:工具径(d≦4)」
//								if((SEQ.FLAG4.BIT.EDGE_L_SCAN == 1) && (SEQ.FLAG4.BIT.EDGE_R_SCAN == 1)){
//									temp = RESULT.LARGE_D_DIFF[0];
//								}
//							}else if(SEQ.FLAG2.BIT.AUTO_MODE == MODE_D4_AUTO_LEFT){		// 「2:工具径(d＞4 左側)」
//								if(SEQ.FLAG4.BIT.EDGE_R_SCAN == 1){
//									temp = RESULT.GREATER_D_DIFF_MAX[0];
//								}
//							}else if(SEQ.FLAG2.BIT.AUTO_MODE == MODE_D4_AUTO_RIGHT){	// 「3:工具径(d＞4 右側)」
//								if(SEQ.FLAG4.BIT.EDGE_L_SCAN == 1){
//									temp = RESULT.GREATER_D_DIFF_RIGHT_MIN[0];
//								}
//							}
//							result = temp * 10.0;
//							COM0.NO3000[SEQ.PROFILE_BUFFER_COUNT] = result;
//// add 2015.08.19 K.Uemura start	プロファイル最大／最小出力
//							SEQ.PROFILE_OUTPUT_COUNT++;			// プロファイル計測出力ｶｳﾝﾄ
//							COM0.NO321 = (SEQ.PROFILE_OUTPUT_COUNT) >> 16;
//							COM0.NO322 = (SEQ.PROFILE_OUTPUT_COUNT);
//
//							if(SEQ.PROFILE_OUTPUT_COUNT == 1){
//								//初期化
//								COM0.NO325 = COM0.NO326 = COM0.NO3000[SEQ.PROFILE_BUFFER_COUNT];
//							}else{
//								if((short)COM0.NO3000[SEQ.PROFILE_BUFFER_COUNT] < (short)COM0.NO325){
//									COM0.NO325 = COM0.NO3000[SEQ.PROFILE_BUFFER_COUNT];		// 最小
//								}
//								if((short)COM0.NO326 < (short)COM0.NO3000[SEQ.PROFILE_BUFFER_COUNT]){
//									COM0.NO326 = COM0.NO3000[SEQ.PROFILE_BUFFER_COUNT];		// 最大
//								}
//							}
//// add 2015.08.19 K.Uemura end
//#else
//							if((SEQ.FLAG4.BIT.EDGE_L_SCAN == 1) && (SEQ.FLAG4.BIT.EDGE_R_SCAN == 1)){
//								SEQ.INPUT_DBUS_LONG = RESULT.LARGE_D_DIFF[0] * CALIBRATION * 1.0 / 1000;
//							}else{
//								SEQ.INPUT_DBUS_LONG = 99999;
//							}
//							COM0.NO3000[SEQ.PROFILE_BUFFER_COUNT] = SEQ.INPUT_DBUS_LONG;
//#endif
//							
//							SEQ.BUFFER_NO_NEW = SEQ.PROFILE_BUFFER_COUNT;				// ﾊﾞｯﾌｧﾅﾝﾊﾞｰ新
//							SEQ.PROFILE_BUFFER_COUNT++;									// ﾌﾟﾛﾌｧｲﾙﾊﾞｯﾌｧ格納用ｶｳﾝﾄ
//							if(SEQ.PROFILE_BUFFER_COUNT >= BUFFER_NUMBER)	SEQ.PROFILE_BUFFER_COUNT = 0;
#endif
// add 2016.03.02 K.Uemura end
						}else{
							COM0.NO310.BIT.RDY = 1;										// READY
						}
					}else{
						if(COM0.NO300.BIT.RST){											// 強制ﾘｾｯﾄがONのとき
							COM0.NO310.BIT.RDY = 1;										// 完了
							COM0.NO310.BIT.FIN = 1;										// 完了
							// ADD 170413
							SEQ.FLAG6.BIT.CLEANING = 0;
							SEQ.FPGA_SEND_STATUS = 71;
							SEQ.CHANGE_FPGA = 0;
							DA.DADR0 = 0;								// DA0
							//
							
						}else{
							// 清掃確認が継続中のときは計測方向を変更して再計測
// chg 2016.06.22 K.Uemura start	G62202
							if(((COM0.NO311 == 152)||(COM0.NO311 == 153))&&(COM0.NO300.BIT.EXE)){
//							if((COM0.NO311 == 152)&&(COM0.NO300.BIT.EXE)){
// chg 2016.06.22 K.Uemura end

// chg 2016.06.22 K.Uemura start	G62202
								if((COM0.NO311 == 153) && (SEQ.MEASUREMENT_DIRECTION == Z_DIRECTION)){
									//測定終了
									COM0.NO310.BIT.RDY = 1;								// READY
									COM0.NO310.BIT.FIN = 1;								// 完了
// add 2016.07.26 K.Uemura start	G72601
									SEQ.MSEC_BUFFER[5][5] = SEQ.MSEC_COUNTER;
// add 2016.07.26 K.Uemura end
								}else{
// chg 2016.06.22 K.Uemura end
									SEQ.MEASUREMENT_DIRECTION = !SEQ.MEASUREMENT_DIRECTION;		// 計測方向
									
									SEQ.FLAG.BIT.PARAM_INITIAL = 1;								// ﾊﾟﾗﾒｰﾀ初期化ﾌﾗｸﾞ
									SEQ.CHANGE_FPGA = 1;										// ﾊﾟﾗﾒｰﾀ
									SEQ.FPGA_SEND_STATUS = 1;
									SEQ.POWER_COUNT = 0;										// 電源制御ｶｳﾝﾄをﾘｾｯﾄ
									
									SEQ.FLAG.BIT.MEASUREMENT = 1;
								}
							}else{
								if(SEQ.FLAG3.BIT.AUTO_ERROR == 1){
									SEQ.FLAG3.BIT.AUTO_ERROR = 0;							// 自動ｴﾗｰﾌﾗｸﾞ

									switch(SEQ.SELECT.BIT.MEASURE){
										case MODE_D4_AUTO:				// 10:工具径(自動)
										case MODE_D4_LOW:				// 0:工具径(d≦4)
										case MODE_RUNOUT:				// 2:振れ
										case MODE_PROFILE:				// 5:ﾌﾟﾛﾌｧｲﾙ
											COM0.NO310.BIT.RDY = 1;								// READY
											if(SEQ.FLAG.BIT.PORTABLE == OPERATION_AUTO){
												SEQ.TP_CONTROL_STATUS++;
											}
											break;
									}
								}else if(SEQ.FLAG3.BIT.TUNING_ERROR == 1){
									SEQ.FLAG3.BIT.TUNING_ERROR = 0;
									// ﾁｭｰﾆﾝｸﾞｴﾗｰﾌﾗｸﾞ
									COM0.NO310.BIT.RDY = 1;									// READY
									if(SEQ.SELECT.BIT.MEASURE != MODE_PROFILE){				// 5:ﾌﾟﾛﾌｧｲﾙ以外の時
										COM0.NO310.BIT.FIN = 1;								// 完了
									}
									if(SEQ.FLAG.BIT.PORTABLE == OPERATION_AUTO)	SEQ.TP_CONTROL_STATUS++;
								}else{
// add 2016.03.08 K.Uemura start	G30802
									switch(SEQ.SELECT.BIT.MEASURE){
										case MODE_D4_LOW:			// d≦4
											if(((COM0.NO318 << 16) | COM0.NO317) == 999999){
												COM0.NO312 = ERR_NO_RESULT;
											}
											break;

										case MODE_D4_AUTO:			// d＞4 自動
										case MODE_D4_LEFT:			// d＞4 左
										case MODE_D4_RIGHT:			// d＞4 右
											if(((COM0.NO315 << 16) | COM0.NO316) == 999999){
												COM0.NO312 = ERR_NO_RESULT;
											}
											break;
									}
// add 2016.03.08 K.Uemura end
									COM0.NO310.BIT.RDY = 1;									// READY
								}
							}
						}
					}
					
					led_measure_set();										// LED 測定・設定ｾｯﾄ  
					
					// 特定画素終了ﾌﾗｸﾞが「1」のときRS232Cから自動でﾃﾞﾊﾞｯｸﾞ出力を行う
					if(SEQ.FLAG6.BIT.PIXEL_END == 1){						// 特定画素終了ﾌﾗｸﾞ
						SEQ.FLAG6.BIT.PIXEL_END = 0;						// 特定画素終了ﾌﾗｸﾞ
						SEQ.CBUS_NUMBER = 405;								// 演算前ﾃﾞｰﾀ読み出し
						DEBUG_STR.DEBUG_OUTPUT = 0;							// ﾃﾞﾊﾞｯｸﾞ出力用出力ﾌｫｰﾏｯﾄ		// 0 整数3桁
						SEQ.CHANGE_FPGA = 10;
						SEQ.FPGA_SEND_STATUS = 1;
						DEBUG_STR.DEBUG_COUNT = 0;							// ﾃﾞﾊﾞｯｸﾞ出力用ｶｳﾝﾀ
						SCI2.SCR.BIT.RIE = 0;								// RXIおよびERI割り込み要求を禁止
						SCI2.SCR.BIT.RE = 0;								// ｼﾘｱﾙ受信動作を禁止
						COM2.SUB_STATUS = 4;								// 次へ
					}
					
					// ｾﾝｻﾚﾍﾞﾙﾌﾗｸﾞが「1」のときTPにﾃﾞﾊﾞｯｸﾞ出力を行う
					if(SEQ.FLAG6.BIT.DEBUG_LEVEL == 1){
						SEQ.FLAG6.BIT.DEBUG_LEVEL = 0;					// ｾﾝｻﾚﾍﾞﾙﾌﾗｸﾞ
						SEQ.CBUS_NUMBER = 405;								// 演算前ﾃﾞｰﾀ読み出し
						DEBUG_STR.DEBUG_OUTPUT = 0;							// ﾃﾞﾊﾞｯｸﾞ出力用出力ﾌｫｰﾏｯﾄ		// 0 整数3桁
						SEQ.CHANGE_FPGA = 17;
						SEQ.FPGA_SEND_STATUS = 1;
						DEBUG_STR.DEBUG_COUNT = 0;							// ﾃﾞﾊﾞｯｸﾞ出力用ｶｳﾝﾀ
					}
					
					// ﾎﾟｰﾀﾌﾞﾙ版で計測中にﾓｰﾄﾞが切り替えられたとき再計測を行う
					if(SEQ.FLAG.BIT.CHANGE_MODE == 1){
						SEQ.FLAG.BIT.CHANGE_MODE = 0;						// 計測ﾓｰﾄﾞ切り替えﾌﾗｸﾞ
						SEQ.FLAG.BIT.MEASUREMENT = 1;
						
						SEQ.CBUS_NUMBER = 394;
						SEQ.CHANGE_FPGA = 2;								// 測定
						SEQ.FPGA_SEND_STATUS = 1;
						SEQ.FLAG.BIT.PORTABLE = OPERATION_MANUAL;			// ﾎﾟｰﾀﾌﾞﾙ操作ﾌﾗｸﾞ
					}
				}
			}
			break;
			
		case 22:
			if(SEQ.FLAG.BIT.BUFFER_RESET == 1){							// ﾊﾞｯﾌｧｶｳﾝﾄが「0」のとき
				SEQ.FLAG.BIT.BUFFER_RESET = 0;							// ﾊﾞｯﾌｧﾘｾｯﾄﾌﾗｸﾞ
				SEQ.FPGA_SEND_STATUS = 11;
			}
			break;

// add 2016.01.21 K.Uemura start	G12102
		case 30:
			if(SEQ.FLAG4.BIT.OKCOUNT_FLAG == 1){
				if(COM0.NO300.BIT.ROF == 1){								// READY OFF検知が「1」のとき
					SEQ.FLAG4.BIT.OKCOUNT_FLAG = 0;
					SEQ.FPGA_SEND_STATUS = 21;
				}
			}else{
				SEQ.FPGA_SEND_STATUS = 21;
			}
			break;
// add 2016.01.21 K.Uemura end
	}
}
