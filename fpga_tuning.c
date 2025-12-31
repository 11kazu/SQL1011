/******************************************************************************
* File Name	: fpga_tuning.c
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

void send_to_fpga_tuning(void);				// FPGAへのﾃﾞｰﾀ送信関数(ﾁｭｰﾆﾝｸﾞ)

//************************************************************/
//				FPGAへのﾃﾞｰﾀ送信関数(ﾁｭｰﾆﾝｸﾞ)
//	2 → ・・・ → 8 → 11 → 14 → 15 → 16 → 11
//	                                         → 21 → 11
//	                                               → 2
//	                                               → 22 → 11
//	                                               → 71
//************************************************************/
// ﾁｭｰﾆﾝｸﾞ処理を行う
// 動作モード：d≦4／d＞4／振れ／プロファイル
void send_to_fpga_tuning(void)
{
	_UBYTE execute_mode_edge;
	_UBYTE edge_flag_L, edge_flag_R;
	_UBYTE scan_mode;
	_UWORD tuning_timer;
	float work;
	
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
			if(SEQ.CBUS_NUMBER == 211){							// 左ｴｯｼﾞ最小位置
				//SEQ.BUFFER_COUNT値の推移
				//2→0→1→1・・・
				if(SEQ.BUFFER_COUNT == 2){
					SEQ.BUFFER_COUNT = 0;
				}
				set_result_edge_left();							// 左ｴｯｼﾞ検出結果の設定、座標変換
				
			}else if(SEQ.CBUS_NUMBER == 212){					// 右ｴｯｼﾞ最大位置
				set_result_edge_right();						// 右ｴｯｼﾞ検出結果の設定、座標変換
				
			}else if(SEQ.CBUS_NUMBER == 213){					// 左ｴｯｼﾞ傾斜
				set_result_focus_left();						// 左ｴｯｼﾞ焦点結果の設定
				
			}else if(SEQ.CBUS_NUMBER == 214){					// 右ｴｯｼﾞ傾斜
				set_result_focus_right();						// 右ｴｯｼﾞ焦点結果の設定
				
				// ﾌﾗｸﾞ設定
				set_scan_flag( SEQ.BUFFER_COUNT );
				
				// 検出結果設定
				set_result( SEQ.BUFFER_COUNT );

				// 振れﾊﾞｯﾌｧ出力
				if(SEQ.SELECT.BIT.MEASURE == MODE_RUNOUT){		// 2:振れ

					// 走査方向取得
					scan_mode = get_scan_mode( SEQ.SELECT.BIT.MEASURE, SEQ.FLAG2.BIT.AUTO_MODE );

					if(scan_mode == TOOL_LEFT){
						if(SEQ.FLAG4.BIT.EDGE_R_SCAN == 1){
							// 右エッジの最大取得
							if(RESULT.TIR[1] < RESULT.EDGE_RIGHT_PIXEL[0]){
								RESULT.TIR[1] = RESULT.EDGE_RIGHT_PIXEL[0];
							}
						}
					}else{
						if(SEQ.FLAG4.BIT.EDGE_L_SCAN == 1){
							// 左エッジの最大取得
							if(RESULT.TIR[1] > RESULT.EDGE_LEFT_PIXEL[0]){
								RESULT.TIR[1] = RESULT.EDGE_LEFT_PIXEL[0];
							}
						}
					}
					SEQ.MEMORY_TIME = 0;						// 記憶時間(ms)
				}

				if(SEQ.BUFFER_COUNT == 0){
					SEQ.BUFFER_COUNT = 1;
				}
				
			}else if(SEQ.CBUS_NUMBER == 223){					// ｽｷｯﾌﾟ出力
				set_result_skip();								// SKIP結果の設定
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
				
				// 100ms間隔でﾃﾞｰﾀ送信	add 140424
				if(SEQ.FLAG.BIT.BUFFER_RESET == 1){			// ﾊﾞｯﾌｧｶｳﾝﾄが「0」のとき
					SEQ.FLAG.BIT.BUFFER_RESET = 0;			// ﾊﾞｯﾌｧﾘｾｯﾄﾌﾗｸﾞ
					//SEQ.BUFFER_COUNT = 0;
					SEQ.TUNING_COUNT++;						// ﾁｭｰﾆﾝｸﾞｶｳﾝﾄ
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
			SEQ.FPGA_SEND_STATUS = 11;
			
			if(SEQ.FLAG.BIT.AFTER_STOPPING == 0){
				if(F_PRIO_IN == 0){							// F_PRIO_INが「L」のとき
					if(SEQ.FLAG.BIT.MEASUREMENT == 0){
						// 待機を送信(390)
						C_PRIO_OUT	= 1;					// C_PRIO
						SEQ.CBUS_NUMBER = 390;
						OUT.SUB_STATUS = 3;//
						SEQ.FPGA_SEND_STATUS = 2;
					}else{

						// ﾁｭｰﾆﾝｸﾞ時間の設定
						// 回転数が「0」のとき    ：DLPM115
						// 回転数が「0」以外のとき：回転数に依存
						//                           1回転に要する時間が1秒以内の時は1秒(例:100ms→1秒)
						//                           1秒以上のときは実際の時間を設定
						tuning_timer = SEQ.TUNING_SECONDS;

						if(SEQ.SPINDLE_SPEED != 0){
							tuning_timer = 60000 / SEQ.SPINDLE_SPEED;
							if(tuning_timer < 1000){
								tuning_timer = 10;				// 1秒[10倍値]
							}else{
								tuning_timer /= POLLING_CYCLE;	// 
								tuning_timer += 1;				// 繰り上げ
							}
						}

						if(SEQ.TUNING_COUNT >= tuning_timer){			// ﾁｭｰﾆﾝｸﾞｶｳﾝﾀがﾁｭｰﾆﾝｸﾞ秒数と同じ値になったとき

							// ﾁｭｰﾆﾝｸﾞｴﾗｰ判定
							edge_flag_L = edge_flag_R = 1;

							// 走査方向取得
							execute_mode_edge = get_execute_mode_edge(SEQ.SELECT.BIT.MEASURE, SEQ.FLAG2.BIT.AUTO_MODE);

							// ｴｯｼﾞ左
							if((execute_mode_edge == EDGE_LEFT) || (execute_mode_edge == EDGE_AND)){
								if(SEQ.FLAG4.BIT.EDGE_L_SCAN == 0){
									edge_flag_L = 0;
								}
							}

							// ｴｯｼﾞ右
							if((execute_mode_edge == EDGE_RIGHT) || (execute_mode_edge == EDGE_AND)){
								if(SEQ.FLAG4.BIT.EDGE_R_SCAN == 0){
									edge_flag_R = 0;
								}
							}
							
							// 計測方向がX方向のとき
							if((SEQ.MEASUREMENT_DIRECTION == X_DIRECTION)&&((edge_flag_L == 0)||(edge_flag_R == 0))){
								C_PRIO_OUT	= 1;						// C_PRIO
								SEQ.CBUS_NUMBER = 390;
								SEQ.FPGA_SEND_STATUS = 2;
								SEQ.FLAG3.BIT.TUNING_ERROR = 1;			// ﾁｭｰﾆﾝｸﾞｴﾗｰﾌﾗｸﾞ
								COM0.NO312 = ERR_TUNING_NO_RESULT;		// 5007	ﾁｭｰﾆﾝｸﾞ結果なし
								set_7seg_led_error_no(COM0.NO312);		// 7セグLEDのｴﾗｰﾅﾝﾊﾞｰ設定
								OUT.SUB_STATUS = 3;//
								SEQ.FLAG.BIT.MEASUREMENT = 0;
								
							// 計測方向がZ方向のとき
							}else if((SEQ.MEASUREMENT_DIRECTION == Z_DIRECTION)&&(edge_flag_R == 0)){
								C_PRIO_OUT	= 1;						// C_PRIO
								SEQ.CBUS_NUMBER = 390;
								SEQ.FPGA_SEND_STATUS = 2;
								SEQ.FLAG3.BIT.TUNING_ERROR = 1;			// ﾁｭｰﾆﾝｸﾞｴﾗｰﾌﾗｸﾞ
								COM0.NO312 = ERR_TUNING_NO_RESULT;		// 5007	ﾁｭｰﾆﾝｸﾞ結果なし
								set_7seg_led_error_no(COM0.NO312);		// 7セグLEDのｴﾗｰﾅﾝﾊﾞｰ設定
								OUT.SUB_STATUS = 3;//
								SEQ.FLAG.BIT.MEASUREMENT = 0;
								
							}else{

								COM0.NO106 = RESULT.SMALL_D_MAX[0] * 10;			// d最大
								COM0.NO107 = RESULT.LARGE_D_MIN[0] * 10;			// 左エッジ最小
								COM0.NO108 = RESULT.LARGE_D_MAX[0] * 10;			// 右エッジ最大
								COM0.NO109 = RESULT.FOCUS_LEFT_MIN[0] * 1000;		// 左焦点最小
								COM0.NO110 = RESULT.FOCUS_RIGHT_MIN[0] * 1000;		// 右焦点最小

								// 比率乗算
								SEQ.TUNING_FOCUS_LEFT_MIN = 0.0;						// 焦点数値を最小で初期化
								SEQ.TUNING_FOCUS_RIGHT_MIN = 0.0;

								if(execute_mode_edge == EDGE_AND){
									if(SEQ.FLAG.BIT.PORTABLE == OPERATION_MANUAL){
										SEQ.TUNING_EDGE_SMALL_D_LOWER = DL_MIN;			// d最大下限(ﾁｭｰﾆﾝｸﾞ)
										SEQ.TUNING_EDGE_SMALL_D_UPPER = DL_MAX;			// d最大上限(ﾁｭｰﾆﾝｸﾞ)
									}else{
										// dに比率乗算
										SEQ.TUNING_EDGE_SMALL_D_LOWER = RESULT.SMALL_D_MAX[0] * ((float)SEQ.TUNING_RATIO_LOWER_LIMIT / 1000.0);		// d最大下限(ﾁｭｰﾆﾝｸﾞ)
										SEQ.TUNING_EDGE_SMALL_D_UPPER = RESULT.SMALL_D_MAX[0] * ((float)SEQ.TUNING_RATIO_UPPER_LIMIT / 1000.0);		// d最大上限(ﾁｭｰﾆﾝｸﾞ)
									}
									COM0.NO111 = SEQ.TUNING_EDGE_SMALL_D_LOWER * 10.;		// エッジ最小
									COM0.NO112 = SEQ.TUNING_EDGE_SMALL_D_UPPER * 10.;		// エッジ最大

									SEQ.TUNING_FOCUS_LEFT_MIN = RESULT.FOCUS_LEFT_MIN[0] + ((float)SEQ.TUNING_OFFSET / 10.0);	// 焦点 左最小(ﾁｭｰﾆﾝｸﾞ)
									SEQ.TUNING_FOCUS_RIGHT_MIN = RESULT.FOCUS_RIGHT_MIN[0] + ((float)SEQ.TUNING_OFFSET / 10.0);	// 焦点 右最小(ﾁｭｰﾆﾝｸﾞ)
								}else if(execute_mode_edge == EDGE_LEFT){
									// 左ｴｯｼﾞの結果(0方向(降順)を基準)
									if(SEQ.FLAG.BIT.PORTABLE == OPERATION_MANUAL){
										SEQ.TUNING_EDGE_LEFT_MIN_LOWER = DL_MAX;
										SEQ.TUNING_EDGE_LEFT_MIN_UPPER = DL_MIN;
									}else{
// chg 2015.05.26 K.Uemura start	
										work = DL_MAX - RESULT.LARGE_D_MIN[0];
//										work = LINESENSOR_END - RESULT.LARGE_D_MIN[0];
// chg 2015.05.26 K.Uemura end
										SEQ.TUNING_EDGE_LEFT_MIN_LOWER = DL_MAX - (work * ((float)SEQ.TUNING_RATIO_LOWER_LIMIT / 1000.0));
										SEQ.TUNING_EDGE_LEFT_MIN_UPPER = DL_MAX - (work * ((float)SEQ.TUNING_RATIO_UPPER_LIMIT / 1000.0));
									}
									COM0.NO111 = SEQ.TUNING_EDGE_LEFT_MIN_LOWER * 10.;		// エッジ最小
									COM0.NO112 = SEQ.TUNING_EDGE_LEFT_MIN_UPPER * 10.;		// エッジ最大

									SEQ.TUNING_FOCUS_LEFT_MIN = RESULT.FOCUS_LEFT_MIN[0] + ((float)SEQ.TUNING_OFFSET / 10.0);	// 焦点 左最小(ﾁｭｰﾆﾝｸﾞ)
								}else if(execute_mode_edge == EDGE_RIGHT){
									// 右ｴｯｼﾞの結果
									if(SEQ.FLAG.BIT.PORTABLE == OPERATION_MANUAL){
										SEQ.TUNING_EDGE_RIGHT_MAX_LOWER = DL_MIN;
										//SEQ.TUNING_EDGE_RIGHT_MAX_UPPER = DL_MAX;
										if(SEQ.MEASUREMENT_DIRECTION == X_DIRECTION)	SEQ.TUNING_EDGE_RIGHT_MAX_UPPER = DL_MAX;
										else											SEQ.TUNING_EDGE_RIGHT_MAX_UPPER = DL_Z_MAX;
									}else{
										SEQ.TUNING_EDGE_RIGHT_MAX_LOWER = RESULT.LARGE_D_MAX[0] * ((float)SEQ.TUNING_RATIO_LOWER_LIMIT / 1000.0);	// ｴｯｼﾞ 右最大下限(ﾁｭｰﾆﾝｸﾞ)
										SEQ.TUNING_EDGE_RIGHT_MAX_UPPER = RESULT.LARGE_D_MAX[0] * ((float)SEQ.TUNING_RATIO_UPPER_LIMIT / 1000.0);	// ｴｯｼﾞ 右最大上限(ﾁｭｰﾆﾝｸﾞ)
									}
									COM0.NO111 = SEQ.TUNING_EDGE_RIGHT_MAX_LOWER * 10.;	// エッジ最小
									COM0.NO112 = SEQ.TUNING_EDGE_RIGHT_MAX_UPPER * 10.;	// エッジ最大

									SEQ.TUNING_FOCUS_RIGHT_MIN = RESULT.FOCUS_RIGHT_MIN[0] + ((float)SEQ.TUNING_OFFSET / 10.0);	// 焦点 右最小(ﾁｭｰﾆﾝｸﾞ)
								}
								
								COM0.NO113 = SEQ.TUNING_FOCUS_LEFT_MIN * 1000;		// 左焦点最小
								COM0.NO114 = SEQ.TUNING_FOCUS_RIGHT_MIN * 1000;	// 右焦点最小
								
								SEQ.BUFFER_COUNT = 0;
								
								OUT.SUB_STATUS = 6;//
								
								SEQ.CHANGE_FPGA = 6;								// 本計測へ
								
								// 振れﾊﾞｯﾌｧ出力
								if(SEQ.SELECT.BIT.MEASURE == MODE_RUNOUT){			// 2:振れ

									scan_mode = get_scan_mode( SEQ.SELECT.BIT.MEASURE, SEQ.FLAG2.BIT.AUTO_MODE );

// add 2015.08.31 K.Uemura start	振れ比率は、画素分解能から減算
									if(SEQ.MEASUREMENT_DIRECTION == X_DIRECTION){			// 計測方向がX方向のとき
										// Xセンサ
										work = abs(((SEQ.TABLE_EDGE_RIGHT[X_DIVISION_NUMBER-1] + SEQ.TABLE_EDGE_LEFT[X_DIVISION_NUMBER-1]) / 2.0)
													 - ((SEQ.TABLE_EDGE_RIGHT[0] + SEQ.TABLE_EDGE_LEFT[0]) / 2.0));
										work = (X_DIVISION_NUMBER * X_CORRECTION_INTERVAL*10) / work;		// μm/画素

// add 2016.06.22 K.Uemura start	G62201
										// 換算[μm → 画素]
										work = ((float)X_RATIO_SWING / 100.0) / work;
// add 2016.06.22 K.Uemura end
									}else{
										// Zセンサ
										work = abs(SEQ.Z_TABLE_EDGE_RIGHT[0] - SEQ.Z_TABLE_EDGE_RIGHT[Z_DIVISION_NUMBER-1]);
										work = (Z_DIVISION_NUMBER * Z_CORRECTION_INTERVAL*10) / work;

// add 2016.06.22 K.Uemura start	G62201
										// 換算[μm → 画素]
										work = ((float)Z_RATIO_SWING / 100.0) / work;
// add 2016.06.22 K.Uemura end
									}

// chg 2016.06.22 K.Uemura start	G62201
//									// 換算[μm → 画素]
//									work = ((float)X_RATIO_SWING / 100.0) / work;
// chg 2016.06.22 K.Uemura end
// add 2015.08.31 K.Uemura end

									if(scan_mode == TOOL_LEFT){
										if(SEQ.FLUTES == 0){
											// 比率使用しない
											RESULT.TIR[2] = LINESENSOR_START;
										}else{
											RESULT.TIR[2] = RESULT.TIR[1] - work;			// 振れ比率

											if(RESULT.TIR[2] < 0){
												RESULT.TIR[2] =0;
											}
										}
									}else if(scan_mode == TOOL_RIGHT){
										if(SEQ.FLUTES == 0){
											// 比率使用しない
											RESULT.TIR[2] = LINESENSOR_END;
										}else{
											RESULT.TIR[2] = RESULT.TIR[1] + work;			// 振れ比率
										}
									}
									SEQ.MEMORY_TIME = 0;						// 記憶時間(ms)
//									SEQ.FLAG3.BIT.SWING_FALL = 0;				// 振れ下降ﾌﾗｸﾞ
									SEQ.FLAG3.BIT.SWING_RISE = 0;				// 振れ上昇ﾌﾗｸﾞ
									SEQ.FLAG3.BIT.SWING_RESET = 0;				// 振れｶｳﾝﾄﾘｾｯﾄﾌﾗｸﾞ

// add 2015.07.28 K.Uemura start	
									COM0.NO136 = RESULT.TIR[1]*10;		// 最大ｴｯｼﾞ[pix]
									COM0.NO137 = RESULT.TIR[2]*10;		// 比率乗算結果
//									COM0.NO106 = RESULT.TIR[1]*10;		// 最大ｴｯｼﾞ[pix]
//									COM0.NO107 = RESULT.TIR[2]*10;		// 比率乗算結果
// add 2015.07.28 K.Uemura end	
								}
								
								if(COM0.NO300.BIT.EXE){							// ﾀｯﾁﾊﾟﾈﾙから実行しているとき
									end_condition_set();						// 終了条件ｾｯﾄ
								}
								
								SEQ.FPGA_SEND_STATUS = 22;
							}
							set_7seg_upper_no_data();				// 7ｾｸﾞ表示(-----)(上段)
							set_7seg_lower_no_data();				// 7ｾｸﾞ表示(-----)(下段)
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
				SEQ.FPGA_SEND_STATUS = 11;
				SEQ.TUNING_COUNT = 0;
			}
			break;
	}
}
