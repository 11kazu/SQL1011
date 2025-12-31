/******************************************************************************
* File Name	: fpga_auto.c
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

void send_to_fpga_auto(void);				// FPGAへのﾃﾞｰﾀ送信関数(自動判別)

//************************************************************/
//				FPGAへのﾃﾞｰﾀ送信関数(自動判別)
//	22 → 11 → 14 → 15 → 16 → 11
//	                           → 21 → 11
//	                                 → 2 → ・・・ → 8 → 11
//	                                 → 22 [6] send_to_fpga2
//	                                       [5] send_to_fpga_tuning
//	                                 → 71 [0]
//************************************************************/
// 自動判別
void send_to_fpga_auto(void)
{
	long temp;
	_UBYTE scan_mode;
	_UWORD judge_timer;
	_UWORD table_center;
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
			send_to_dbus_zero();						// ﾃﾞｰﾀ出力関数0
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
			if(SEQ.CBUS_NUMBER == 390){				// 指令ｺﾏﾝﾄﾞが待機のとき
				SEQ.FLAG.BIT.AFTER_STOPPING = 1;	// 計測停止後1ｻｲｸﾙ取得ﾌﾗｸﾞ
				SEQ.CHANGE_FPGA = 9;
				SEQ.FLAG.BIT.MEASUREMENT = 0;
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
				C_ACK_OUT	= 1;						// C_ACK
				C_ACK_OUT	= 0;						// C_ACK
			}
			break;
			
		case 14:	// 割り込み後
			//
			if(SEQ.CBUS_NUMBER == 211){						// 左ｴｯｼﾞ位置
				// 浮動小数点のﾃﾞｰﾀに変換し値を表示する
				Variable.lLong = SEQ.INPUT_DBUS_LONG;
				SEQ.INPUT_DBUS = Variable.fFloat;
// add 2015.05.15 K.Uemura start	
				RESULT.EDGE_LEFT_PIXEL[SEQ.BUFFER_COUNT] = SEQ.INPUT_DBUS;
				// エッジ(最小)
				if(RESULT.EDGE_LEFT_PIXEL[SEQ.BUFFER_COUNT] < RESULT.EDGE_LEFT_PIXEL[0]){
					RESULT.EDGE_LEFT_PIXEL[0] = RESULT.EDGE_LEFT_PIXEL[SEQ.BUFFER_COUNT];
				}
// add 2015.05.15 K.Uemura end
				COM0.NO101 = (unsigned short)(SEQ.INPUT_DBUS * 10);
				
			}else if(SEQ.CBUS_NUMBER == 212){				// 右ｴｯｼﾞ位置
				// 浮動小数点のﾃﾞｰﾀに変換し値を表示する
				Variable.lLong = SEQ.INPUT_DBUS_LONG;
				SEQ.INPUT_DBUS = Variable.fFloat;
// add 2015.05.15 K.Uemura start	
				RESULT.EDGE_RIGHT_PIXEL[SEQ.BUFFER_COUNT] = SEQ.INPUT_DBUS;
				// エッジ(最大)
				if(RESULT.EDGE_RIGHT_PIXEL[0] < RESULT.EDGE_RIGHT_PIXEL[SEQ.BUFFER_COUNT]){
					RESULT.EDGE_RIGHT_PIXEL[0] = RESULT.EDGE_RIGHT_PIXEL[SEQ.BUFFER_COUNT];
				}
// add 2015.05.15 K.Uemura end
				COM0.NO102 = (unsigned short)(SEQ.INPUT_DBUS * 10);
				
			}else if(SEQ.CBUS_NUMBER == 223){				// ｽｷｯﾌﾟ出力
				set_result_skip();
				
				SEQ.FLAG2.BIT.AUTO_LEFT = 0;				// 自動判別(左)ﾌﾗｸﾞ
				SEQ.FLAG2.BIT.AUTO_RIGHT = 0;				// 自動判別(右)ﾌﾗｸﾞ
				SEQ.FLAG2.BIT.AUTO_BOTH = 0;				// 自動判別(両端)ﾌﾗｸﾞ
				SEQ.FLAG2.BIT.AUTO_UNDETECTED = 0;			// 自動判別(未検出)ﾌﾗｸﾞ
				
				if(IN.FLAG.BIT.HARDWARE_TYPE == CNC_EDITION){
					if(SEQ.MEASUREMENT_DIRECTION == Z_DIRECTION){
						temp = LED.Z_FOCUSING;		//  4ﾋﾞｯﾄ
						if((temp & 0x0001) >= 1)				SEQ.FLAG2.BIT.AUTO_LEFT = 1;				// 自動判別(左)ﾌﾗｸﾞ
						if((temp & 0x0008) >= 1)				SEQ.FLAG2.BIT.AUTO_RIGHT = 1;				// 自動判別(右)ﾌﾗｸﾞ
						if((temp & 0x0006) >= 1)				SEQ.FLAG2.BIT.AUTO_BOTH = 1;				// 自動判別(両端)ﾌﾗｸﾞ
						if((temp == 0)||(temp == 0x000F))		SEQ.FLAG2.BIT.AUTO_UNDETECTED = 1;			// 自動判別(未検出)ﾌﾗｸﾞ
					}else{
						temp = LED.FOCUSING;		//  9ﾋﾞｯﾄ
						if((temp & 0x0001) >= 1)				SEQ.FLAG2.BIT.AUTO_LEFT = 1;				// 自動判別(左)ﾌﾗｸﾞ
						if((temp & 0x0100) >= 1)				SEQ.FLAG2.BIT.AUTO_RIGHT = 1;				// 自動判別(右)ﾌﾗｸﾞ
						if((temp & 0x00FE) >= 1)				SEQ.FLAG2.BIT.AUTO_BOTH = 1;				// 自動判別(両端)ﾌﾗｸﾞ
						if((temp == 0)||(temp == 0x01FF))		SEQ.FLAG2.BIT.AUTO_UNDETECTED = 1;			// 自動判別(未検出)ﾌﾗｸﾞ
					}
				}else{
						temp = LED.FOCUSING;		// 11ﾋﾞｯﾄ
						if((temp & 0x0001) >= 1)				SEQ.FLAG2.BIT.AUTO_LEFT = 1;				// 自動判別(左)ﾌﾗｸﾞ
						if((temp & 0x0400) >= 1)				SEQ.FLAG2.BIT.AUTO_RIGHT = 1;				// 自動判別(右)ﾌﾗｸﾞ
						if((temp & 0x03FE) >= 1)				SEQ.FLAG2.BIT.AUTO_BOTH = 1;				// 自動判別(両端)ﾌﾗｸﾞ
						if((temp == 0)||(temp == 0x07FF))		SEQ.FLAG2.BIT.AUTO_UNDETECTED = 1;			// 自動判別(未検出)ﾌﾗｸﾞ
				}
				
				if(SEQ.FLAG2.BIT.AUTO_UNDETECTED == 1){		// 自動判別(未検出)ﾌﾗｸﾞが立っているとき
					SEQ.AUTO_UNDETECTED_COUNT++;			// 自動判別(未検出)ｶｳﾝﾀ
				}else{										// 自動判別(未検出)ﾌﾗｸﾞが立っていないとき
					if(SEQ.FLAG2.BIT.AUTO_LEFT == 1){		// 自動判別(左)ﾌﾗｸﾞ
						SEQ.AUTO_LEFT_COUNT++;				// 自動判別(左)ｶｳﾝﾀ
					}
					if(SEQ.FLAG2.BIT.AUTO_RIGHT == 1){		// 自動判別(右)ﾌﾗｸﾞ
						SEQ.AUTO_RIGHT_COUNT++;				// 自動判別(右)ｶｳﾝﾀ
					}
					if(SEQ.FLAG2.BIT.AUTO_BOTH == 1){		// 自動判別(両端)ﾌﾗｸﾞ
						SEQ.AUTO_BOTH_COUNT++;				// 自動判別(両端)ｶｳﾝﾀ
					}
				}
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
			
			if(SEQ.CBUS_NUMBER == SEQ.LAST_CBUS_NUMBER){	// CBUSﾅﾝﾊﾞｰ(1ｻｲｸﾙ最終)のとき
				SEQ.FPGA_SEND_STATUS = 21;

				// 100ms間隔でﾃﾞｰﾀ送信
				if(SEQ.FLAG.BIT.BUFFER_RESET == 1){			// ﾊﾞｯﾌｧｶｳﾝﾄが「0」のとき
					SEQ.FLAG.BIT.BUFFER_RESET = 0;			// ﾊﾞｯﾌｧﾘｾｯﾄﾌﾗｸﾞ
					SEQ.AUTO_COUNT++;						// 方向判別ｶｳﾝﾄ
				}
			}
			break;
			
#if	1
		// C_ACKを「H」にする
		case 17:
			SEQ.FPGA_SEND_STATUS = 13;
			C_ACK_OUT	= 1;						// C_ACK
			break;
#endif
			
		// F_PRIO_INが「L」になっていることを確認する
		case 21:
			SEQ.FPGA_SEND_STATUS = 11;
			
			if(SEQ.FLAG.BIT.AFTER_STOPPING == 0){
				if(F_PRIO_IN == 0){								// F_PRIO_INが「L」のとき
					if(SEQ.FLAG.BIT.MEASUREMENT == 0){
						// 待機を送信(390)
						C_PRIO_OUT	= 1;						// C_PRIO
						SEQ.CBUS_NUMBER = 390;
						SEQ.FPGA_SEND_STATUS = 2;
						
					}else{

						// ﾁｭｰﾆﾝｸﾞ時間の設定
						// 回転数が「0」のとき    ：DLPM115
						// 回転数が「0」以外のとき：回転数に依存
						//                           1回転に要する時間が1秒以内の時は1秒(例:100ms→1秒)
						//                           1秒以上のときは実際の時間+DLPM800を設定
						judge_timer = (1000 / POLLING_CYCLE);

						if(SEQ.SPINDLE_SPEED != 0){
							judge_timer = 60000 / SEQ.SPINDLE_SPEED;
							if(judge_timer < 1000){
								judge_timer = 10;				// 1秒[10倍値]
							}else{
								judge_timer /= POLLING_CYCLE;	// 
								judge_timer += 1;				// 繰り上げ
							}
						}

						if(SEQ.AUTO_COUNT >= judge_timer){		// 方向判別ｶｳﾝﾀがﾁｭｰﾆﾝｸﾞ秒数と同じ値になったとき
							SEQ.FLAG2.BIT.AUTO_MODE = 0;		// 自動判別ﾓｰﾄﾞ	0:なし	1:工具径(d≦4)	2:工具径(d＞4 左側)	3:工具径(d＞4 右側)
							
							// ここで自動判別結果を判定
							if((SEQ.AUTO_LEFT_COUNT > 0)&&(SEQ.AUTO_RIGHT_COUNT > 0)){					// 自動判別(左)ｶｳﾝﾀが「0」以上	自動判別(右)ｶｳﾝﾀが「0」以上	のとき
								// 判別不可能
								SEQ.FLAG2.BIT.AUTO_MODE = 5;
							}else if((SEQ.AUTO_LEFT_COUNT > 0)&&(SEQ.AUTO_RIGHT_COUNT == 0)){			// 自動判別(左)ｶｳﾝﾀが「0」以上	自動判別(右)ｶｳﾝﾀが「0」		のとき
								// 左
								SEQ.FLAG2.BIT.AUTO_MODE = 2;											// 自動判別ﾓｰﾄﾞ	2:工具径(d＞4 左側)
							}else if((SEQ.AUTO_LEFT_COUNT == 0)&&(SEQ.AUTO_RIGHT_COUNT > 0)){			// 自動判別(左)ｶｳﾝﾀが「0」		自動判別(右)ｶｳﾝﾀが「0」以上	のとき
								// 右
								SEQ.FLAG2.BIT.AUTO_MODE = 3;											// 自動判別ﾓｰﾄﾞ	3:工具径(d＞4 右側)
							}else if((SEQ.AUTO_LEFT_COUNT == 0)&&(SEQ.AUTO_RIGHT_COUNT == 0)){			// 自動判別(左)ｶｳﾝﾀが「0」		自動判別(右)ｶｳﾝﾀが「0」		のとき
								if(SEQ.AUTO_BOTH_COUNT > 0){											// 自動判別(両端)ｶｳﾝﾀが「0」以上のとき
									if(SEQ.SELECT.BIT.MEASURE == MODE_D4_AUTO){							// 10:工具径(自動)	(工具径(d≦4)の判定は無し)
										// 両端
// chg 2015.05.20 K.Uemura start	
#if	1
										// 換算TBL中心
										table_center = (SEQ.TABLE_EDGE_LEFT[X_TABLE_CENTER] + SEQ.TABLE_EDGE_RIGHT[X_TABLE_CENTER])/2.0;

										// TBL中心と検出座標の差
										if(abs(table_center - RESULT.EDGE_RIGHT_PIXEL[0]*10) > 
											abs(table_center - RESULT.EDGE_LEFT_PIXEL[0]*10)){
#else
										// add 150511
										// 中心位置を算出する
										// (右ｴｯｼﾞ + 左ｴｯｼﾞ)/2 - 2048 が正のとき
// chg 2015.05.15 K.Uemura start	
										if(((RESULT.EDGE_RIGHT_PIXEL[0] + RESULT.EDGE_LEFT_PIXEL[0]) - 40960) > 0){
//										if(((COM0.NO9002 + COM0.NO9001) - 40960) > 0){
// chg 2015.05.15 K.Uemura end
#endif
// chg 2015.05.20 K.Uemura end
											SEQ.FLAG2.BIT.AUTO_MODE = 3;								// 自動判別ﾓｰﾄﾞ	3:工具径(d＞4 右側)	
										}else{
											SEQ.FLAG2.BIT.AUTO_MODE = 2;								// 自動判別ﾓｰﾄﾞ	2:工具径(d＞4 左側)	
										}
									}else{
										// 両端
										SEQ.FLAG2.BIT.AUTO_MODE = 1;									// 自動判別ﾓｰﾄﾞ	1:工具径(d≦4)
									}
								}
							}
							
// add 2015.08.19 K.Uemura start	プロファイル最大／最小出力
							switch(SEQ.SELECT.BIT.MEASURE){
								case MODE_RUNOUT:
								case MODE_PROFILE:
									// 左オフセット(右エッジ検出＠プラスが最大)
									COM0.NO325 = 0x7FFF;		// 最小(最大で初期化)
									COM0.NO326 = 0x8000;		// 最大(最小で初期化)
									break;
							}
// add 2015.08.19 K.Uemura end

							// 工具径(d≦4)のとき
							if(SEQ.SELECT.BIT.MEASURE == MODE_D4_LOW){
								if(SEQ.FLAG2.BIT.AUTO_MODE == 0){			// 自動判別ﾓｰﾄﾞ	0:なしのとき
									// 待機を送信(390)
									C_PRIO_OUT	= 1;						// C_PRIO
									SEQ.CBUS_NUMBER = 390;
									SEQ.FPGA_SEND_STATUS = 2;
									SEQ.FLAG3.BIT.AUTO_ERROR = 1;			// 自動ｴﾗｰﾌﾗｸﾞ
									COM0.NO312 = ERR_NO_EDGE;				// 5005	自動判別ﾓｰﾄﾞでｴｯｼﾞがなく判別不可能
									set_7seg_led_error_no(COM0.NO312);		// 7セグLEDのｴﾗｰﾅﾝﾊﾞｰ設定
									OUT.SUB_STATUS = 3;
									SEQ.FLAG.BIT.MEASUREMENT = 0;
									
								}else if(SEQ.FLAG2.BIT.AUTO_MODE == 5){		// 自動判別ﾓｰﾄﾞ	両方のとき
									// 待機を送信(390)
									C_PRIO_OUT	= 1;						// C_PRIO
									SEQ.CBUS_NUMBER = 390;
									SEQ.FPGA_SEND_STATUS = 2;
									SEQ.FLAG3.BIT.AUTO_ERROR = 1;			// 自動ｴﾗｰﾌﾗｸﾞ
									COM0.NO312 = ERR_BOTH_EDGE;				// 5006	自動判別ﾓｰﾄﾞで両方ｴｯｼﾞがあり判別不可能
									set_7seg_led_error_no(COM0.NO312);		// 7セグLEDのｴﾗｰﾅﾝﾊﾞｰ設定
									OUT.SUB_STATUS = 3;
									SEQ.FLAG.BIT.MEASUREMENT = 0;
									
								}else if(SEQ.FLAG2.BIT.AUTO_MODE == 2){		// 自動判別ﾓｰﾄﾞ	左側のとき
									// 待機を送信(390)
									C_PRIO_OUT	= 1;						// C_PRIO
									SEQ.CBUS_NUMBER = 390;
									SEQ.FPGA_SEND_STATUS = 2;
									SEQ.FLAG3.BIT.AUTO_ERROR = 1;			// 自動ｴﾗｰﾌﾗｸﾞ
									COM0.NO312 = ERR_ONE_EDGE;				// 5004	自動判別ﾓｰﾄﾞでどちらか一方ｴｯｼﾞがあり判別不可能
									set_7seg_led_error_no(COM0.NO312);		// 7セグLEDのｴﾗｰﾅﾝﾊﾞｰ設定
									OUT.SUB_STATUS = 3;
									SEQ.FLAG.BIT.MEASUREMENT = 0;
									
								}else if(SEQ.FLAG2.BIT.AUTO_MODE == 3){		// 自動判別ﾓｰﾄﾞ	右側のとき
									// 待機を送信(390)
									C_PRIO_OUT	= 1;						// C_PRIO
									SEQ.CBUS_NUMBER = 390;
									SEQ.FPGA_SEND_STATUS = 2;
									SEQ.FLAG3.BIT.AUTO_ERROR = 1;			// 自動ｴﾗｰﾌﾗｸﾞ
									COM0.NO312 = ERR_ONE_EDGE;				// 5004	自動判別ﾓｰﾄﾞでどちらか一方ｴｯｼﾞがあり判別不可能
									set_7seg_led_error_no(COM0.NO312);		// 7セグLEDのｴﾗｰﾅﾝﾊﾞｰ設定
									OUT.SUB_STATUS = 3;
									SEQ.FLAG.BIT.MEASUREMENT = 0;
									
								}else{
									SEQ.CHANGE_FPGA = 6;					// 本計測へ
									
									// ﾁｭｰﾆﾝｸﾞ処理実行
									if(SEQ.TUNING_ENABLE == 1){			// ﾁｭｰﾆﾝｸﾞ「有」のとき
										if(SEQ.TUNING_SECONDS > 0){		// ﾁｭｰﾆﾝｸﾞ秒数が「0」より上のとき
											SEQ.BUFFER_COUNT = 2;
											SEQ.CHANGE_FPGA = 5;		// ﾁｭｰﾆﾝｸﾞ処理へ
										}
									}
									
									// 走査方向(0:未検出 1:右ｴｯｼﾞ 2:左ｴｯｼﾞ 3:両ｴｯｼﾞ)
									switch(SEQ.FLAG2.BIT.AUTO_MODE){
										case 1:	COM0.NO310.BIT.DIR = 3;	break;
										case 2:	COM0.NO310.BIT.DIR = 1;	break;
										case 3:	COM0.NO310.BIT.DIR = 2;	break;
										default:
											COM0.NO310.BIT.DIR = 0;
											SEQ.FLAG3.BIT.AUTO_ERROR = 1;			// 自動ｴﾗｰﾌﾗｸﾞ
											break;
									}
									
									if(SEQ.CHANGE_FPGA == 6){						// 本計測のとき
										if(COM0.NO300.BIT.EXE){						// ﾀｯﾁﾊﾟﾈﾙから実行しているとき
											end_condition_set();					// 終了条件ｾｯﾄ
										}
									}
									
									SEQ.FPGA_SEND_STATUS = 22;
								}
								
							// 工具径(d≦4)以外のとき
							}else{
								if(SEQ.FLAG2.BIT.AUTO_MODE == 0){			// 自動判別ﾓｰﾄﾞ	0:なしのとき
									// 待機を送信(390)
									C_PRIO_OUT	= 1;						// C_PRIO
									SEQ.CBUS_NUMBER = 390;
									SEQ.FPGA_SEND_STATUS = 2;
									SEQ.FLAG3.BIT.AUTO_ERROR = 1;			// 自動ｴﾗｰﾌﾗｸﾞ
									COM0.NO312 = ERR_NO_EDGE;				// 5005	自動判別ﾓｰﾄﾞでｴｯｼﾞがなく判別不可能
									set_7seg_led_error_no(COM0.NO312);		// 7セグLEDのｴﾗｰﾅﾝﾊﾞｰ設定
									OUT.SUB_STATUS = 3;
									SEQ.FLAG.BIT.MEASUREMENT = 0;
									
								}else if(SEQ.FLAG2.BIT.AUTO_MODE == 5){		// 自動判別ﾓｰﾄﾞ	両方のとき
									// 待機を送信(390)
									C_PRIO_OUT	= 1;						// C_PRIO
									SEQ.CBUS_NUMBER = 390;
									SEQ.FPGA_SEND_STATUS = 2;
									SEQ.FLAG3.BIT.AUTO_ERROR = 1;			// 自動ｴﾗｰﾌﾗｸﾞ
									COM0.NO312 = ERR_BOTH_EDGE;				// 5006	自動判別ﾓｰﾄﾞで両方ｴｯｼﾞがあり判別不可能
									set_7seg_led_error_no(COM0.NO312);		// 7セグLEDのｴﾗｰﾅﾝﾊﾞｰ設定
									OUT.SUB_STATUS = 3;
									SEQ.FLAG.BIT.MEASUREMENT = 0;
									
								}else{
									SEQ.CHANGE_FPGA = 6;					// 本計測へ
									
									// ﾁｭｰﾆﾝｸﾞ処理実行
									switch(SEQ.SELECT.BIT.MEASURE){
										case MODE_D4_LOW:
										case MODE_D4_AUTO:
										case MODE_D4_LEFT:
										case MODE_D4_RIGHT:
										case MODE_PROFILE:
										case MODE_GROWTH:		// 伸び測定
											if(SEQ.TUNING_ENABLE == 1){			// ﾁｭｰﾆﾝｸﾞ「有」のとき
												if(SEQ.TUNING_SECONDS > 0){		// ﾁｭｰﾆﾝｸﾞ秒数が「0」より上のとき
													SEQ.BUFFER_COUNT = 2;
													SEQ.CHANGE_FPGA = 5;		// ﾁｭｰﾆﾝｸﾞ処理へ
												}
											}
											break;
									}
									
									// 走査方向(0:未検出 1:右ｴｯｼﾞ 2:左ｴｯｼﾞ 3:両ｴｯｼﾞ)
									switch(SEQ.FLAG2.BIT.AUTO_MODE){
										case 1:	COM0.NO310.BIT.DIR = 3;	break;
// chg 2016.09.08 K.Uemura start	G90801
										case 2:	
											if((RESULT_SIGN == 1) && (SEQ.MEASUREMENT_DIRECTION == X_DIRECTION)){
												COM0.NO310.BIT.DIR = 2;
											}else{
												COM0.NO310.BIT.DIR = 1;
											}
											break;
										case 3:	
											if((RESULT_SIGN == 1) && (SEQ.MEASUREMENT_DIRECTION == X_DIRECTION)){
												COM0.NO310.BIT.DIR = 1;
											}else{
												COM0.NO310.BIT.DIR = 2;
											}
											break;
//										case 2:	COM0.NO310.BIT.DIR = 1;	break;		// 左オフセット（右エッジ）
//										case 3:	COM0.NO310.BIT.DIR = 2;	break;		// 右オフセット（左エッジ）
// chg 2016.09.08 K.Uemura end
										default:
											COM0.NO310.BIT.DIR = 0;
											SEQ.FLAG3.BIT.AUTO_ERROR = 1;			// 自動ｴﾗｰﾌﾗｸﾞ
											break;
									}

									if(SEQ.SELECT.BIT.MEASURE == MODE_RUNOUT){		// 2:振れ

										COM0.NO310.BIT.DIR = 0;

										// 走査方向取得
										scan_mode = get_scan_mode( SEQ.SELECT.BIT.MEASURE, SEQ.FLAG2.BIT.AUTO_MODE );

										if(scan_mode == TOOL_LEFT){
											RESULT.TIR[0] = RESULT.TIR[1] = RESULT.TIR[3] = INITIAL_MIN;
											RESULT.TIR[4] = INITIAL_MAX;
											RESULT.TIR[5] = INITIAL_MIN;
											COM0.NO310.BIT.DIR = 1;
// chg 2015.07.28 K.Uemura start	15072801
											// 計測方向がZ方向のとき
											// μm換算の符号が反転するため初期値を上書き
											if(SEQ.MEASUREMENT_DIRECTION == Z_DIRECTION){
												RESULT.TIR[0] = RESULT.TIR[3] = INITIAL_MAX;
												RESULT.TIR[4] = INITIAL_MIN;
											}
// chg 2015.07.28 K.Uemura end
										}else if(scan_mode == TOOL_RIGHT){
											RESULT.TIR[0] = RESULT.TIR[1] = RESULT.TIR[3] = INITIAL_MAX;
											RESULT.TIR[4] = INITIAL_MIN;
											RESULT.TIR[5] = INITIAL_MAX;
											COM0.NO310.BIT.DIR = 2;
										}else{
											SEQ.FLAG3.BIT.AUTO_ERROR = 1;			// 自動ｴﾗｰﾌﾗｸﾞ
										}
										if(SEQ.TUNING_SECONDS > 0){					// ﾁｭｰﾆﾝｸﾞ秒数が「0」より上のとき
											SEQ.BUFFER_COUNT = 2;
											SEQ.CHANGE_FPGA = 5;					// ﾁｭｰﾆﾝｸﾞ処理へ
										}
										SEQ.MEMORY_TIME = 0;						// 記憶時間(ms)
									}
									
									if(SEQ.CHANGE_FPGA == 6){						// 本計測のとき
										if(COM0.NO300.BIT.EXE){						// ﾀｯﾁﾊﾟﾈﾙから実行しているとき
											end_condition_set();					// 終了条件ｾｯﾄ
										}
									}
									
									SEQ.FPGA_SEND_STATUS = 22;
								}
							}
						}
					}
				}
			}else{
				SEQ.FLAG.BIT.AFTER_STOPPING = 0;
				SEQ.FPGA_SEND_STATUS = 71;
				SEQ.CHANGE_FPGA = 0;
				DA.DADR0 = 0;							// DA0
				
				// ﾌﾟﾛﾌｧｲﾙのとき
				if(SEQ.SELECT.BIT.MEASURE == MODE_PROFILE){
					if(COM0.NO300.BIT.EXE){				// 実行
						COM0.NO310.BIT.STR = 1;			// STROBE
					}else{
						COM0.NO310.BIT.RDY = 1;			// READY
					}
				}else{
					if(COM0.NO300.BIT.RST){				// 強制ﾘｾｯﾄがONのとき
						COM0.NO310.BIT.FIN = 1;			// 完了
					}else{
						COM0.NO310.BIT.RDY = 1;			// READY
					}
				}
			}
			break;
			
		case 22:
			if(SEQ.FLAG.BIT.BUFFER_RESET == 1){			// ﾊﾞｯﾌｧｶｳﾝﾄが「0」のとき
				SEQ.FLAG.BIT.BUFFER_RESET = 0;			// ﾊﾞｯﾌｧﾘｾｯﾄﾌﾗｸﾞ
				SEQ.FLAG2.BIT.AUTO_MODE = 4;
				SEQ.FPGA_SEND_STATUS = 11;
				SEQ.AUTO_COUNT = 0;						// 方向判別ｶｳﾝﾄ
			}
			break;
	}
}
