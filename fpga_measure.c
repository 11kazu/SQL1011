/******************************************************************************
* File Name	: fpga_measure.c
******************************************************************************/
#include <machine.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include <mathf.h>
#include "iodefine.h"
#include "user_define.h"
#include "typedefine.h"
#include "usercopy.h"

// deb 2015.07.29 K.Uemura start	
#define	__GET_SCANMODE	0			// 走査方向(0:右エッジ 1:左エッジ)
									// ※Z測定は「0」であること
// deb 2015.07.29 K.Uemura end

void send_to_fpga2(void);						// FPGAへのﾃﾞｰﾀ送信関数

void set_result_edge_left(void);				// 左ｴｯｼﾞ検出結果の設定、座標変換
void set_result_edge_right(void);				// 右ｴｯｼﾞ検出結果の設定、座標変換
void set_result_focus_left(void);				// 左ｴｯｼﾞ焦点結果の設定
void set_result_focus_right(void);				// 右ｴｯｼﾞ焦点結果の設定

void set_result_skip(void);						// SKIP結果の設定
void result_output(void);						// 結果出力
void max_min_reset(void);						// 最大値・最小値ﾘｾｯﾄ
void swing_no_tooth_reset(void);				// 最大値・最小値ﾘｾｯﾄ(振れ刃数0)
void output_error(unsigned long);
_UWORD get_execute_timer(void);					// 実行時間の取得
_UBYTE get_execute_mode(_UBYTE, _UBYTE);		// 動作ﾓｰﾄﾞの取得(ORIGIN／d≦4／・・・)
_UBYTE get_execute_mode_edge(_UBYTE, _UBYTE);	// 動作ﾓｰﾄﾞの取得(右ｴｯｼﾞ／左ｴｯｼﾞ／両ｴｯｼﾞ)
_UBYTE get_scan_mode(_UBYTE, _UBYTE);			// 動作ﾓｰﾄﾞの取得(右ｴｯｼﾞ／左ｴｯｼﾞ)
_UBYTE get_tuning_mode(_UBYTE);					// ﾁｭｰﾆﾝｸﾞ実施の取得(ﾁｭｰﾆﾝｸﾞ実施／しない)
float get_threshold(_UBYTE, _UBYTE );			// しきい値取得(計測各種／焦点合わせ／ORIGIN[ｴｯｼﾞ考慮])
short set_scan_flag(_UWORD);
short set_ok_flag(_UWORD);
short set_result(_UWORD);
short set_7seg_led_upper(long, _UBYTE);
short set_7seg_led_lower(long, _UBYTE);
void set_7seg_upper_no_data(void);				// 7ｾｸﾞ表示(-----)(上段)
void set_7seg_lower_no_data(void);				// 7ｾｸﾞ表示(-----)(下段)

//************************************************************/
//				FPGAへのﾃﾞｰﾀ送信関数(計測)
//	22 → 11 → 14 → 15 → 16 → 11
//	                           → 21 → 11
//	                                 → 2 → 3 → 4 → 5 → 6 → 7 → 8 → 11
//	                                 → 22 [16]プロファイル開始(send_to_fpga_profileへ)
//	                                 → 71 [ 0]測定終了
//	                           → 103 [ 2]再計測(fpga_idleへ)
//************************************************************/
// 計測
void send_to_fpga2(void)
{
	_UWORD i;
	_UWORD ave_data,min_data,max_data;				// 平均値・最小値・最大値の一時ﾃﾞｰﾀ
	float temp[RESULT_AREA_MAX];
	long result[RESULT_AREA_MAX];
	_UBYTE exec_mode;
	_UBYTE scan_mode;
	_UBYTE result_flag;

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
				SEQ.FPGA_RESTART_COUNT = 0;
				C_ACK_OUT	= 1;					// C_ACK
				C_ACK_OUT	= 0;					// C_ACK
			// ADD 160907
			}else{	// F_PRIO_INが「1」にならないとき一旦停止する
				SEQ.FPGA_RESTART_COUNT++;				// FPGA再ｽﾀｰﾄｶｳﾝﾄ
				if(SEQ.FPGA_RESTART_COUNT >= 100){
					SEQ.FPGA_RESTART_COUNT = 0;
					C_PRIO_OUT	= 1;					// C_PRIO
					SEQ.CBUS_NUMBER = 394;
					SEQ.FPGA_SEND_STATUS = 2;			// 次へ
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
					//SEQ.INPUT_DBUS_LONG = SEQ.INPUT_DBUS * 10.0;
					SEQ.INPUT_DBUS_LONG = (long)(SEQ.INPUT_DBUS * 10.0);
					COM0.NO105 = SEQ.INPUT_DBUS_LONG;
					
// add 2016.02.18 K.Uemura start	G21804
					// 追加ﾃﾞｰﾀﾌﾗｸﾞが「0」のとき ADD 160226
					if(SEQ.FLAG6.BIT.ADD_DATA == 0)		result_output();	// 結果出力
// add 2016.02.18 K.Uemura end
					
// deb 2015.07.29 K.Uemura start	
#ifdef	DEBUG_EACHSCAN
if(SEQ.FLAG3.BIT.SWING_RESET){
	if(DEBUG_STR.DEBUG_COUNT<8000){
#if	__GET_SCANMODE
		SEQ.TABLE_TEMP_DATA[0] = RESULT.FOCUS_LEFT[SEQ.BUFFER_COUNT];
		SEQ.TABLE_TEMP_DATA[1] = RESULT.GREATER_D_DIFF_RIGHT[SEQ.BUFFER_COUNT];
		SEQ.TABLE_TEMP_DATA[2] = RESULT.EDGE_LEFT_PIXEL[SEQ.BUFFER_COUNT];
#else
		SEQ.TABLE_TEMP_DATA[0] = RESULT.FOCUS_RIGHT[SEQ.BUFFER_COUNT];
		SEQ.TABLE_TEMP_DATA[1] = RESULT.GREATER_D_DIFF[SEQ.BUFFER_COUNT];
		SEQ.TABLE_TEMP_DATA[2] = RESULT.EDGE_RIGHT_PIXEL[SEQ.BUFFER_COUNT];
#endif

		// FOCUS値が25.5を超える場合は、全て25.5とする
		if(25.5 < SEQ.TABLE_TEMP_DATA[1]){
			SEQ.TABLE_TEMP_DATA[1] = 25.5;
		}

		// 設定
		DEBUG_STR.DEBUG_FOCUS[DEBUG_STR.DEBUG_COUNT] = (unsigned char)(SEQ.TABLE_TEMP_DATA[0] * 10.0);
		DEBUG_STR.DEBUG_EDGE[DEBUG_STR.DEBUG_COUNT] = (unsigned short)(SEQ.TABLE_TEMP_DATA[1] * 10.0);
//		DEBUG_STR.DEBUG_EDGE2[DEBUG_STR.DEBUG_COUNT] = (unsigned short)(SEQ.TABLE_TEMP_DATA[2] * 10.0);
	}else if(DEBUG_STR.DEBUG_COUNT == 8000){
		DEBUG_STR.DEBUG_COUNT = 8000;
		DEBUG_STR.DEBUG_COUNT++;
		DEBUG_STR.DEBUG_COUNT--;
	}else{
		DEBUG_STR.DEBUG_COUNT++;
		DEBUG_STR.DEBUG_COUNT--;
	}
	DEBUG_STR.DEBUG_COUNT++;
}
#endif
// deb 2015.07.29 K.Uemura end
					break;
					
// add 2016.02.18 K.Uemura start	G21804
				// ADD 160304
				case 216:		// 平均値・最小値・最大値
// chg 2016.07.13 K.Uemura start	G71302
					ave_data = (short)((SEQ.INPUT_DBUS_LONG >> 16) & 0x000000FF);		// 平均値
					min_data = (short)((SEQ.INPUT_DBUS_LONG >> 8) & 0x000000FF);		// 最小値
					max_data = (short)(SEQ.INPUT_DBUS_LONG & 0x000000FF);				// 最大値
//					ave_data = (short)(SEQ.INPUT_DBUS_LONG & 0x000000FF);				// 平均値
//					min_data = (short)((SEQ.INPUT_DBUS_LONG >> 8) & 0x000000FF);		// 最小値
//					max_data = (short)((SEQ.INPUT_DBUS_LONG >> 16) & 0x000000FF);		// 最大値
// chg 2016.07.13 K.Uemura end
					
					// 平均値・最小値・最大値が全て「0」でないとき
					if((ave_data != 0)&&(min_data != 0)&&(max_data != 0)){
						COM0.NO133 = ave_data;											// 平均値
						COM0.NO134 = min_data;											// 最小値
						COM0.NO135 = max_data;											// 最大値
						
						SEQ.AVE_TOTAL += COM0.NO133;									// 合計
						if(SEQ.AVE_MIN > COM0.NO133)	SEQ.AVE_MIN = COM0.NO133;		// 最小
						if(SEQ.AVE_MAX < COM0.NO133)	SEQ.AVE_MAX = COM0.NO133;		// 最大
						
						SEQ.MIN_TOTAL += COM0.NO134;									// 合計
						if(SEQ.MIN_MIN > COM0.NO134)	SEQ.MIN_MIN = COM0.NO134;		// 最小
						if(SEQ.MIN_MAX < COM0.NO134)	SEQ.MIN_MAX = COM0.NO134;		// 最大
						
						SEQ.MAX_TOTAL += COM0.NO135;									// 合計
						if(SEQ.MAX_MIN > COM0.NO135)	SEQ.MAX_MIN = COM0.NO135;		// 最小
						if(SEQ.MAX_MAX < COM0.NO135)	SEQ.MAX_MAX = COM0.NO135;		// 最大
						
						SEQ.CLEANING_COUNT_PASS++;										// 清掃ﾊﾟﾙｽｶｳﾝﾄ(正常)
					}
					SEQ.CLEANING_COUNT_TOTAL++;											// 清掃ﾊﾟﾙｽｶｳﾝﾄ(合計)
					
					result_output();	// 結果出力
					break;
				//
// add 2016.02.18 K.Uemura end
				
				case 223:		// ｽｷｯﾌﾟ出力
					set_result_skip();
					hdi_output();					// HDI出力
// add 2016.07.26 K.Uemura start	G72601
					if(SEQ.MSEC_FLAG == 26){
						SEQ.MSEC_BUFFER[25][1] = SEQ.MSEC_COUNTER;
						SEQ.MSEC_FLAG = 27;
					}
// add 2016.07.26 K.Uemura end
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
			
			if(SEQ.CBUS_NUMBER == SEQ.LAST_CBUS_NUMBER){	// CBUSﾅﾝﾊﾞｰ(1ｻｲｸﾙ最終)のとき
			
				SEQ.FPGA_SEND_STATUS = 21;
				
				SEQ.BUFFER_COUNT2++;						// ﾊﾞｯﾌｧｶｳﾝﾄ2(動作確認用)
				
				// ADD 150728
				if(SEQ.FLAG6.BIT.DATA_TRANSMISSION == 0){
					if(SEQ.FLAG.BIT.MEASUREMENT == MEASURE_STOP){
						SEQ.FLAG.BIT.BUFFER_RESET = 1;			// ﾊﾞｯﾌｧﾘｾｯﾄﾌﾗｸﾞ
					}
				}
				
				// 100ms間隔でﾃﾞｰﾀ送信
				if(SEQ.FLAG.BIT.BUFFER_RESET == 1){			// ﾊﾞｯﾌｧｶｳﾝﾄが「0」のとき
					SEQ.FLAG.BIT.BUFFER_RESET = 0;			// ﾊﾞｯﾌｧﾘｾｯﾄﾌﾗｸﾞ
					
					SEQ.FLAG6.BIT.DATA_TRANSMISSION = 1;	// ﾃﾞｰﾀ送信ﾌﾗｸﾞ
					
					// ADD 150608
					COM0.NO132 = SEQ.BUFFER_COUNT2;			// ﾊﾞｯﾌｧｶｳﾝﾄ2(動作確認用)
					SEQ.BUFFER_COUNT2 = 0;					// ﾊﾞｯﾌｧｶｳﾝﾄ2(動作確認用)
					//
					
					COM0.NO310.BIT.RDY = 0;					// READYのﾋﾞｯﾄをOFFにし状態番号をｾｯﾄする
// add 2016.07.26 K.Uemura start	G72601
					if(SEQ.MSEC_FLAG == 27){
						SEQ.MSEC_BUFFER[25][2] = SEQ.MSEC_COUNTER;
						SEQ.MSEC_FLAG = 0;
					}
// add 2016.07.26 K.Uemura end
					
					// 検出ﾌﾗｸﾞの設定
					if(SEQ.FLAG4.BIT.EDGE_L_POLLING == 1){
						SEQ.FLAG4.BIT.EDGE_L_SCAN = 1;
					}
					if(SEQ.FLAG4.BIT.EDGE_R_POLLING == 1){
						SEQ.FLAG4.BIT.EDGE_R_SCAN = 1;
					}
					if(SEQ.FLAG4.BIT.EDGE_LR_POLLING == 1){
						SEQ.FLAG4.BIT.EDGE_LR_SCAN = 1;
					}

					// 動作ﾓｰﾄﾞ取得
					exec_mode = get_execute_mode( SEQ.SELECT.BIT.MEASURE, SEQ.FLAG2.BIT.AUTO_MODE );

					// 走査方向取得
					scan_mode = get_scan_mode( SEQ.SELECT.BIT.MEASURE, SEQ.FLAG2.BIT.AUTO_MODE );

					// 初期化
					for(i=0; i<RESULT_AREA_MAX; i++){
						temp[i] = 999999.9;
						result[i] = 999999;
					}
					result_flag = 0;		// 0:結果なし 1:結果あり

					switch(exec_mode){
						case MODE_D4_LOW:			// 0:d≦4
							if(SEQ.FLAG4.BIT.EDGE_LR_POLLING == 1){
								temp[0] = RESULT.WORK_EDGE_DIFF[SEQ.BUFFER_COUNT];	// d
								result[0] = temp[0] * 10.0;
							}

							if(SEQ.FLAG4.BIT.EDGE_LR_SCAN == 1){
								temp[1] = RESULT.SMALL_D_MAX[0];					// d-max
								result[1] = temp[1] * 10.0;
							}

							if((SEQ.FLAG4.BIT.EDGE_L_SCAN == 1) && (SEQ.FLAG4.BIT.EDGE_R_SCAN == 1)){
								temp[2] = RESULT.LARGE_D_DIFF[0];					// D(左右ｴｯｼﾞ最大最小の差)
								result[2] = temp[2] * 10.0;

								temp[3] = RESULT.DELTA_X_DIFF[0];					// ΔX 差
								result[3] = temp[3] * 10.0;

								if(SEQ.FLAG4.BIT.EDGE_LR_SCAN == 1){
									result[4] = result[2] - result[1];					// 振れ(Dとd-maxの差)
								}
							}

							if(SEQ.FLAG.BIT.PORTABLE == OPERATION_MANUAL){				// ﾎﾟｰﾀﾌﾞﾙ操作ﾌﾗｸﾞ
								if(SEQ.SELECT.BIT.PEAKHOLD == PEAK_HOLD_OFF){
									if(SEQ.FLAG4.BIT.EDGE_LR_POLLING == 1){
										SEQ.PEAKHOLD_DATA1[1] = result[0];		// d
									}
								}else{
									if((SEQ.FLAG4.BIT.EDGE_L_SCAN == 1) && (SEQ.FLAG4.BIT.EDGE_R_SCAN == 1)){
										SEQ.PEAKHOLD_DATA1[1] = result[2];		// D
										SEQ.PEAKHOLD_DATA2[1] = result[4];		// 振れ
									}
								}
							}
#if	EXHIBITION
							// for exhibitions 141021
							else{
								SEQ.SELECT.BIT.PEAKHOLD = PEAK_HOLD_5S;
								if((SEQ.FLAG4.BIT.EDGE_L_SCAN == 1) && (SEQ.FLAG4.BIT.EDGE_R_SCAN == 1)){
									SEQ.PEAKHOLD_DATA1[1] = result[2];			// D
									SEQ.PEAKHOLD_DATA2[1] = result[4];			// 振れ
								}
							}
#endif
// add 2017.02.10 K.Uemura start	H21001
							// 「d」の更新時に限り結果更新を行う
							if(SEQ.FLAG4.BIT.EDGE_LR_POLLING == 1){
// add 2017.02.10 K.Uemura end
								COM0.NO313 = result[0] >> 16;
								COM0.NO314 = result[0];
								COM0.NO315 = result[1] >> 16;
								COM0.NO316 = result[1];
								COM0.NO317 = result[2] >> 16;
								COM0.NO318 = result[2];
								COM0.NO319 = result[3] >> 16;
								COM0.NO320 = result[3];
								COM0.NO321 = result[4] >> 16;
								COM0.NO322 = result[4];

								// 7seg表示(上段)
								if(SEQ.FLAG.BIT.PORTABLE == OPERATION_MANUAL){				// ﾎﾟｰﾀﾌﾞﾙ操作ﾌﾗｸﾞ
									set_7seg_led_upper( SEQ.PEAKHOLD_DATA1[1], SEQ.SELECT.BIT.MEASURE );
								}else{
#if	EXHIBITION
									// for exhibitions 141021
									set_7seg_led_upper( SEQ.PEAKHOLD_DATA1[1], SEQ.SELECT.BIT.MEASURE );
#else
									set_7seg_led_upper( result[2], SEQ.SELECT.BIT.MEASURE );
#endif
								}

								// 7seg表示(下段)
								if((SEQ.FLUTES & 1) == 0){
									// 刃数が偶数のとき
									if(SEQ.FLAG.BIT.PORTABLE == OPERATION_AUTO){			// ﾎﾟｰﾀﾌﾞﾙ操作ﾌﾗｸﾞ
#if	EXHIBITION
										// for exhibitions 141021
										set_7seg_led_lower( SEQ.PEAKHOLD_DATA2[1], SEQ.SELECT.BIT.MEASURE );
#else
										set_7seg_led_lower( result[4], SEQ.SELECT.BIT.MEASURE );
#endif
									}else{
										if(SEQ.SELECT.BIT.PEAKHOLD == PEAK_HOLD_OFF){
											set_7seg_lower_no_data();				// 7ｾｸﾞ表示(-----)(下段)
										}else{
											set_7seg_led_lower( SEQ.PEAKHOLD_DATA2[1], SEQ.SELECT.BIT.MEASURE );
										}
										
									}
								
								}else{
									set_7seg_lower_no_data();				// 7ｾｸﾞ表示(-----)(下段)
								}
							}
							break;
							
						case MODE_D4_LEFT:			//  1:d＞4 左
													// 30:工具長(Z)
							if(SEQ.FLAG4.BIT.EDGE_R_POLLING == 1){
								temp[0] = RESULT.GREATER_D_DIFF[SEQ.BUFFER_COUNT];	// REAL
								result[0] = temp[0] * 10.0;

// chg 2016.09.08 K.Uemura start	G90801
								if((RESULT_SIGN == 1) && (SEQ.MEASUREMENT_DIRECTION == X_DIRECTION)){
//								if(RESULT_SIGN == 1){
// chg 2016.09.08 K.Uemura end
									result[0] *= -1;
								}
							}

							if(SEQ.FLAG4.BIT.EDGE_R_SCAN == 1){
								temp[1] = RESULT.GREATER_D_DIFF_MAX[0];				// D>4 最大
								result[1] = temp[1] * 10.0;

// chg 2016.09.08 K.Uemura start	G90801
								if((RESULT_SIGN == 1) && (SEQ.MEASUREMENT_DIRECTION == X_DIRECTION)){
//								if(RESULT_SIGN == 1){
// chg 2016.09.08 K.Uemura end
									result[1] *= -1;
								}
								result_flag = 1;
							}

							if(SEQ.FLAG.BIT.PORTABLE == OPERATION_MANUAL){				// ﾎﾟｰﾀﾌﾞﾙ操作ﾌﾗｸﾞ
								if(SEQ.FLAG4.BIT.EDGE_R_SCAN == 1){
									if(SEQ.SELECT.BIT.PEAKHOLD == PEAK_HOLD_OFF){
										SEQ.PEAKHOLD_DATA1[1] = result[0];		// REAL
									}else{
										SEQ.PEAKHOLD_DATA1[1] = result[1];		// D>4 最大
									}
								}
							}

							if(result_flag == 1){
								//出力範囲の確認
								result[2] = DL_MAX;
								result[3] = DL_Z_MAX;
							}

							COM0.NO313 = result[0] >> 16;
							COM0.NO314 = result[0];
							COM0.NO315 = result[1] >> 16;
							COM0.NO316 = result[1];
							COM0.NO317 = 0;
							COM0.NO318 = 0;
							COM0.NO319 = 0;
							COM0.NO320 = 0;
							COM0.NO321 = 0;
							COM0.NO322 = 0;

							// 7seg表示
							if(SEQ.FLAG.BIT.PORTABLE == OPERATION_MANUAL){				// ﾎﾟｰﾀﾌﾞﾙ操作ﾌﾗｸﾞ
								set_7seg_led_upper( SEQ.PEAKHOLD_DATA1[1], SEQ.SELECT.BIT.MEASURE );
								set_7seg_lower_no_data();				// 7ｾｸﾞ表示(-----)(下段)
							}else{
								set_7seg_led_upper( result[1], SEQ.SELECT.BIT.MEASURE );
								set_7seg_led_lower( result[0], SEQ.SELECT.BIT.MEASURE );
							}
							break;
							
							//
						case MODE_GROWTH:			// 6:伸び測定
							if(SEQ.FLAG4.BIT.EDGE_R_POLLING == 1){
								temp[0] = RESULT.GREATER_D_DIFF[SEQ.BUFFER_COUNT];	// REAL
								result[0] = temp[0] * 10.0;

// chg 2016.09.08 K.Uemura start	G90801
//								if(RESULT_SIGN == 1){
//									result[0] *= -1;
//								}
// chg 2016.09.08 K.Uemura end
							}

// chg 2015.08.21 K.Uemura start	
#if	0
//							if(SEQ.FLAG4.BIT.EDGE_R_SCAN == 1){
//								temp[1] = RESULT.GREATER_D_DIFF_MAX[0];				// D>4 最大
//								result[1] = temp[1] * 10.0;
//
//// chg 2015.06.10 K.Uemura start	
//								temp[2] = RESULT.GREATER_D_DIFF_MIN[0];				// D>4 最小
//								result[2] = temp[2] * 10.0;
//
//								if(RESULT_SIGN == 1){
//									result[1] *= -1;
//									result[2] *= -1;
//								}
//
////								if(RESULT_SIGN == 1){
////									result[1] *= -1;
////								}
//// chg 2015.06.10 K.Uemura end
//							}
//
//							if(SEQ.FLAG.BIT.PORTABLE == OPERATION_MANUAL){				// ﾎﾟｰﾀﾌﾞﾙ操作ﾌﾗｸﾞ
//								if(SEQ.FLAG4.BIT.EDGE_R_SCAN == 1){
//									if(SEQ.SELECT.BIT.PEAKHOLD == PEAK_HOLD_OFF){
//										SEQ.PEAKHOLD_DATA1[1] = result[0];		// REAL
//									}else{
//										SEQ.PEAKHOLD_DATA1[1] = result[1];		// D>4 最大
//									}
//								}
//							}
//
//							COM0.NO315 = result[1] >> 16;
//							COM0.NO316 = result[1];
//							COM0.NO317 = result[2] >> 16;
//							COM0.NO318 = result[2];
#endif
// chg 2015.08.21 K.Uemura end

							COM0.NO313 = result[0] >> 16;
							COM0.NO314 = result[0];

// chg 2015.06.10 K.Uemura start	
							//COM0.NO319 320は、main.c内で実施
							if(SEQ.GROWTH_OUTPUT_COUNT == 0 && SEQ.SWING_BUFFER_COUNT == 0){
								COM0.NO315 = 0;
								COM0.NO316 = 0;
								COM0.NO317 = 0;
								COM0.NO318 = 0;
//								COM0.NO319 = 0;
//								COM0.NO320 = 0;
							}else{
								COM0.NO315 = SEQ.GROWTH_MIN >> 16;
								COM0.NO316 = SEQ.GROWTH_MIN;
								COM0.NO317 = SEQ.GROWTH_MAX >> 16;
								COM0.NO318 = SEQ.GROWTH_MAX;

// chg 2016.04.21 K.Uemura start	G42101
								result[1] = COM0.NO315 << 16 | COM0.NO316;
// chg 2016.04.21 K.Uemura end
							}
//							COM0.NO317 = 0;
//							COM0.NO318 = 0;
//							COM0.NO319 = 0;
//							COM0.NO320 = 0;
// chg 2015.06.10 K.Uemura end
							//COM0.NO321 = 0;
							//COM0.NO322 = 0;

							// 7seg表示
							if(SEQ.FLAG.BIT.PORTABLE == OPERATION_MANUAL){				// ﾎﾟｰﾀﾌﾞﾙ操作ﾌﾗｸﾞ
								set_7seg_led_upper( SEQ.PEAKHOLD_DATA1[1], SEQ.SELECT.BIT.MEASURE );
								set_7seg_lower_no_data();				// 7ｾｸﾞ表示(-----)(下段)
							}else{
								set_7seg_led_upper( result[1], SEQ.SELECT.BIT.MEASURE );
								set_7seg_led_lower( result[0], SEQ.SELECT.BIT.MEASURE );
							}
							
							// 伸び測定結果出力	150512
							// RESULT.GROWTH[0] : 最大値(ﾊﾞｯﾌｧ出力)
							// RESULT.GROWTH[1] : 現在値
							// RESULT.GROWTH[2] : 基準値(相対値のとき)
							
							RESULT.GROWTH[1] = result[0];	// 測定結果を格納する
							
							// 結果が無い場合は最大値にする
							if(result[0] > 9999)	RESULT.GROWTH[1] = 9999;
							
							// 値が小さいとき更新する(ﾏｲﾅｽ方向を最大とする)
							if(RESULT.GROWTH[0] > RESULT.GROWTH[1])		RESULT.GROWTH[0] = RESULT.GROWTH[1];
							
							// 伸び計測間隔が「0」でないとき
							if(GROWTH_INTERVAL > 0){
								SEQ.GROWTH_INTERVAL_COUNT++;				// 伸び計測間隔ｶｳﾝﾄ
								// 伸び計測間隔ｶｳﾝﾄと伸び計測間隔が一致したとき
// add 2016.03.10 K.Uemura start	G31002
								if(SEQ.EXTEND_CYCLE){
									if(SEQ.GROWTH_INTERVAL_COUNT >= (SEQ.EXTEND_CYCLE * 10)){
										SEQ.GROWTH_INTERVAL_COUNT = 0;			// 伸び計測間隔ｶｳﾝﾄ
										SEQ.FLAG.BIT.GROWTH_OUTPUT = 1;			// 伸び計測出力ﾌﾗｸﾞ
									}
								}else
// add 2016.03.10 K.Uemura end
								if(SEQ.GROWTH_INTERVAL_COUNT >= GROWTH_INTERVAL){
									SEQ.GROWTH_INTERVAL_COUNT = 0;			// 伸び計測間隔ｶｳﾝﾄ
									SEQ.FLAG.BIT.GROWTH_OUTPUT = 1;			// 伸び計測出力ﾌﾗｸﾞ
								}
							}
							//
							
							break;
							//
							
						case MODE_D4_RIGHT:			// 11:d＞4 右
							if(SEQ.FLAG4.BIT.EDGE_L_POLLING == 1){
								temp[0] = RESULT.GREATER_D_DIFF_RIGHT[SEQ.BUFFER_COUNT];			// REAL
								result[0] = temp[0] * 10.0;

								if(RESULT_SIGN == 1){
									result[0] *= -1;
								}
							}

							if(SEQ.FLAG4.BIT.EDGE_L_SCAN == 1){
								temp[1] = RESULT.GREATER_D_DIFF_RIGHT_MIN[0];						// D>4 最大
								result[1] = temp[1] * 10.0;

								if(RESULT_SIGN == 1){
									result[1] *= -1;
								}
							}

							if(SEQ.FLAG.BIT.PORTABLE == OPERATION_MANUAL){				// ﾎﾟｰﾀﾌﾞﾙ操作ﾌﾗｸﾞ
								if(SEQ.FLAG4.BIT.EDGE_L_SCAN == 1){
									if(SEQ.SELECT.BIT.PEAKHOLD == PEAK_HOLD_OFF){
										SEQ.PEAKHOLD_DATA1[1] = result[0];		// REAL
									}else{
										SEQ.PEAKHOLD_DATA1[1] = result[1];		// D>4 最大
									}
								}
							}

							COM0.NO313 = result[0] >> 16;
							COM0.NO314 = result[0];
							COM0.NO315 = result[1] >> 16;
							COM0.NO316 = result[1];
							COM0.NO317 = 0;
							COM0.NO318 = 0;
							COM0.NO319 = 0;
							COM0.NO320 = 0;
							COM0.NO321 = 0;
							COM0.NO322 = 0;

							// 7seg表示
							if(SEQ.FLAG.BIT.PORTABLE == OPERATION_MANUAL){				// ﾎﾟｰﾀﾌﾞﾙ操作ﾌﾗｸﾞ
								set_7seg_led_upper( SEQ.PEAKHOLD_DATA1[1], SEQ.SELECT.BIT.MEASURE );
								set_7seg_lower_no_data();				// 7ｾｸﾞ表示(-----)(下段)
							}else{
								set_7seg_led_upper( result[1], SEQ.SELECT.BIT.MEASURE );
								set_7seg_led_lower( result[0], SEQ.SELECT.BIT.MEASURE );
							}
							break;
							
						case MODE_RUNOUT:			// 2:振れ
							if(scan_mode == TOOL_LEFT){
								// 右ｴｯｼﾞ
								if(SEQ.FLAG4.BIT.EDGE_R_SCAN == 1){
									temp[0] = RESULT.TIR[0];
									temp[1] = RESULT.TIR[3];
									temp[2] = RESULT.TIR[4];
									result_flag = 1;
								}
							}else{
								// 左ｴｯｼﾞ
								if(SEQ.FLAG4.BIT.EDGE_L_SCAN == 1){
									temp[0] = RESULT.TIR[0];
									temp[1] = RESULT.TIR[3];
									temp[2] = RESULT.TIR[4];
									result_flag = 1;
								}
							}

							if(result_flag == 1){
								result[0] = temp[0] * 10.0;						// 最大最小の差 (REAL)
								result[1] = temp[1] * 10.0;						// 最大
								result[2] = temp[2] * 10.0;						// 最小
								result[3] = abs(result[1] - result[2]);			// 最大最小の差

								if(RESULT_SIGN == 1){
									result[0] *= -1;
									result[1] *= -1;
									result[2] *= -1;
								}

								if(INITIAL_MIN < temp[0] && temp[0] < INITIAL_MAX){
									COM0.NO313 = result[0] >> 16;
									COM0.NO314 = result[0];
								}

								if(SEQ.FLAG3.BIT.SWING_RISE == 2){
									COM0.NO315 = result[1] >> 16;
									COM0.NO316 = result[1];
									COM0.NO317 = result[2] >> 16;
									COM0.NO318 = result[2];
									COM0.NO319 = result[3] >> 16;
									COM0.NO320 = result[3];
//									COM0.NO321 = 0;
//									COM0.NO322 = 0;
									
									// 7seg表示
									if((SEQ.FLAG.BIT.PORTABLE == OPERATION_MANUAL) && (SEQ.SELECT.BIT.PEAKHOLD == PEAK_HOLD_OFF)){
										// ﾎﾟｰﾀﾌﾞﾙ操作ﾌﾗｸﾞ／ピークホールドOFFの場合
										set_7seg_led_upper( result[0], SEQ.SELECT.BIT.MEASURE);
										set_7seg_lower_no_data();				// 7ｾｸﾞ表示(-----)(下段)
									}else{
										set_7seg_led_upper( result[1], SEQ.SELECT.BIT.MEASURE );
										set_7seg_led_lower( result[3], SEQ.SELECT.BIT.MEASURE );
									}
								}
							}
							break;
							
						case MODE_FOCUS:			// 3:焦点
							if(SEQ.FLAG4.BIT.EDGE_L_POLLING == 1){
								result[0] = RESULT.FOCUS_LEFT_MIN[SEQ.BUFFER_COUNT] * 1000.0;		// 焦点 左最小 (REAL)
							}

							if(SEQ.FLAG4.BIT.EDGE_R_POLLING == 1){
								result[1] = RESULT.FOCUS_RIGHT_MIN[SEQ.BUFFER_COUNT] * 1000.0;		// 焦点 右最小 (REAL)
							}

							if(SEQ.FLAG4.BIT.EDGE_L_SCAN == 1){
								result[2] = RESULT.FOCUS_LEFT_MIN[0] * 1000.0;						// 焦点 左最小
							}

							if(SEQ.FLAG4.BIT.EDGE_R_SCAN == 1){
								result[3] = RESULT.FOCUS_RIGHT_MIN[0] * 1000.0;						// 焦点 右最小
							}

							COM0.NO313 = result[0] >> 16;
							COM0.NO314 = result[0];
							COM0.NO315 = result[1] >> 16;
							COM0.NO316 = result[1];
							COM0.NO317 = result[2] >> 16;
							COM0.NO318 = result[2];
							COM0.NO319 = result[3] >> 16;
							COM0.NO320 = result[3];
							COM0.NO321 = 0;
							COM0.NO322 = 0;

							// 7seg表示
							set_7seg_led_upper( result[0], SEQ.SELECT.BIT.MEASURE );
							set_7seg_led_lower( result[1], SEQ.SELECT.BIT.MEASURE );

							break;
							
						case MODE_CENTER:			// 4:中心位置設定
							if((SEQ.FLAG4.BIT.EDGE_L_SCAN == 1) && (SEQ.FLAG4.BIT.EDGE_R_SCAN == 1)){
								temp[0] = RESULT.DELTA_X_DIFF[0];						// ΔX 差
								result[0] = temp[0] * 10.0;

								if(RESULT_SIGN == 1){
									result[0] *= -1;
								}
							}

							COM0.NO313 = result[0] >> 16;
							COM0.NO314 = result[0];
							COM0.NO315 = 0;
							COM0.NO316 = 0;
							COM0.NO317 = 0;
							COM0.NO318 = 0;
							COM0.NO319 = 0;
							COM0.NO320 = 0;
							COM0.NO321 = 0;
							COM0.NO322 = 0;

							// 7seg表示
							set_7seg_led_upper( result[0], SEQ.SELECT.BIT.MEASURE );
							set_7seg_lower_no_data();				// 7ｾｸﾞ表示(-----)(下段)
							
							break;
							
						case MODE_PROFILE:			// 5:ﾌﾟﾛﾌｧｲﾙ
							if(SEQ.FLAG2.BIT.AUTO_MODE == MODE_D4_AUTO_LOW){			// 「1:工具径(d≦4)」
								// d (REAL)
								if(SEQ.FLAG4.BIT.EDGE_LR_POLLING == 1){
									temp[0] = RESULT.WORK_EDGE_DIFF[SEQ.BUFFER_COUNT];
								}
								if((SEQ.FLAG4.BIT.EDGE_L_SCAN == 1) && (SEQ.FLAG4.BIT.EDGE_R_SCAN == 1)){
									temp[1] = RESULT.LARGE_D_DIFF[0];
								}
							}else if(SEQ.FLAG2.BIT.AUTO_MODE == MODE_D4_AUTO_LEFT){		// 「2:工具径(d＞4 左側)」
								if(SEQ.FLAG4.BIT.EDGE_R_POLLING == 1){
									temp[0] = RESULT.GREATER_D_DIFF[SEQ.BUFFER_COUNT];
// chg 2016.09.08 K.Uemura start	G90801
									if((RESULT_SIGN == 1) && (SEQ.MEASUREMENT_DIRECTION == X_DIRECTION)){
//									if(RESULT_SIGN == 1){
// chg 2016.09.08 K.Uemura end
										temp[0] *= -1;
									}
								}
								if(SEQ.FLAG4.BIT.EDGE_R_SCAN == 1){
									temp[1] = RESULT.GREATER_D_DIFF_MAX[0];
// chg 2016.09.08 K.Uemura start	G90801
									if((RESULT_SIGN == 1) && (SEQ.MEASUREMENT_DIRECTION == X_DIRECTION)){
//									if(RESULT_SIGN == 1){
// chg 2016.09.08 K.Uemura end
										temp[1] *= -1;
									}
								}
							}else if(SEQ.FLAG2.BIT.AUTO_MODE == MODE_D4_AUTO_RIGHT){	// 「3:工具径(d＞4 右側)」
								if(SEQ.FLAG4.BIT.EDGE_L_POLLING == 1){
									temp[0] = RESULT.GREATER_D_DIFF_RIGHT[SEQ.BUFFER_COUNT];
									if(RESULT_SIGN == 1){
										temp[0] *= -1;
									}
								}
								if(SEQ.FLAG4.BIT.EDGE_L_SCAN == 1){
									temp[1] = RESULT.GREATER_D_DIFF_RIGHT_MIN[0];
									if(RESULT_SIGN == 1){
										temp[1] *= -1;
									}
								}
							}
							result[0] = temp[0] * 10.0;
							result[1] = temp[1] * 10.0;

							COM0.NO313 = result[0] >> 16;
							COM0.NO314 = result[0];
							COM0.NO315 = result[1] >> 16;
							COM0.NO316 = result[1];
							COM0.NO317 = 0;
							COM0.NO318 = 0;
							COM0.NO319 = 0;
							COM0.NO320 = 0;
							COM0.NO321 = 0;
							COM0.NO322 = 0;
							
							// 7seg表示(上段)
							set_7seg_led_upper( result[0], SEQ.SELECT.BIT.MEASURE );
							// 7seg表示(下段)
							set_7seg_led_lower( result[1], SEQ.SELECT.BIT.MEASURE );
							break;
							
						case MODE_ORIGIN:			// 7:ORIGIN
						case MODE_ORIGIN_EDGE:		// 8:ORIGIN(ｴｯｼﾞ考慮)
							if(SEQ.FLAG6.BIT.CLEANING == 0){
								// 313 左エッジ(REAL)
								if(SEQ.FLAG4.BIT.EDGE_L_POLLING == 1){
									temp[0] = RESULT.GREATER_D_DIFF_RIGHT[SEQ.BUFFER_COUNT];			// REAL
									result[0] = temp[0] * 10.0;

// chg 2016.09.08 K.Uemura start	G90801
									if((RESULT_SIGN == 1) && (SEQ.MEASUREMENT_DIRECTION == X_DIRECTION)){
//									if(RESULT_SIGN == 1){
// chg 2016.09.08 K.Uemura end
										result[0] *= -1;
									}
								}

								// 315 左エッジ(MAX)
								if(SEQ.FLAG4.BIT.EDGE_L_SCAN == 1){
									temp[1] = RESULT.GREATER_D_DIFF_RIGHT_MIN[0];						// D>4 最大
									result[1] = temp[1] * 10.0;

// chg 2016.09.08 K.Uemura start	G90801
									if((RESULT_SIGN == 1) && (SEQ.MEASUREMENT_DIRECTION == X_DIRECTION)){
//									if(RESULT_SIGN == 1){
// chg 2016.09.08 K.Uemura end
										result[1] *= -1;
									}
								}

								// 317 右エッジ(REAL)
								if(SEQ.FLAG4.BIT.EDGE_R_POLLING == 1){
									temp[2] = RESULT.GREATER_D_DIFF[SEQ.BUFFER_COUNT];	// REAL
									result[2] = temp[2] * 10.0;

// chg 2016.09.08 K.Uemura start	G90801
									if((RESULT_SIGN == 1) && (SEQ.MEASUREMENT_DIRECTION == X_DIRECTION)){
//									if(RESULT_SIGN == 1){
// chg 2016.09.08 K.Uemura end
										result[2] *= -1;
									}
								}

								// 319 右エッジ(MAX)
								if(SEQ.FLAG4.BIT.EDGE_R_SCAN == 1){
									temp[3] = RESULT.GREATER_D_DIFF_MAX[0];				// D>4 最大
									result[3] = temp[3] * 10.0;

// chg 2016.09.08 K.Uemura start	G90801
									if((RESULT_SIGN == 1) && (SEQ.MEASUREMENT_DIRECTION == X_DIRECTION)){
//									if(RESULT_SIGN == 1){
// chg 2016.09.08 K.Uemura end
										result[3] *= -1;
									}
								}

								COM0.NO313 = result[0] >> 16;
								COM0.NO314 = result[0];
								COM0.NO315 = result[1] >> 16;
								COM0.NO316 = result[1];
								COM0.NO317 = result[2] >> 16;
								COM0.NO318 = result[2];
								COM0.NO319 = result[3] >> 16;
								COM0.NO320 = result[3];
							}else{
								// 清掃確認
								temp[0] = min(SEQ.AVE_RATE, SEQ.MIN_RATE);
								temp[0] = min(SEQ.MAX_RATE, temp[0]);
								if(SEQ.MEASUREMENT_DIRECTION == X_DIRECTION){
									result[0] = temp[0];

									COM0.NO313 = result[0] >> 16;
									COM0.NO314 = result[0];

									set_7seg_led_upper( result[0], SEQ.SELECT.BIT.MEASURE );

// add 2016.06.22 K.Uemura start	G62202
									RESULT.SENSOR_LEVEL_X = (short)(result[0]);
// add 2016.06.22 K.Uemura end
								}else{
									result[1] = temp[0];

									COM0.NO315 = result[1] >> 16;
									COM0.NO316 = result[1];

									set_7seg_led_lower( result[1], SEQ.SELECT.BIT.MEASURE );

// add 2016.06.22 K.Uemura start	G62202
									//しきい値未満の場合、エラー出力
									RESULT.SENSOR_LEVEL_Z = (short)(result[1]);

									if(COM0.NO311 == 153){
										if((RESULT.SENSOR_LEVEL_X < SENSOR_LEVEL) || 
										   (RESULT.SENSOR_LEVEL_Z < SENSOR_LEVEL)){
											COM0.NO312 = ERR_SENSOR_LEVEL;
										}
									}
// add 2016.07.26 K.Uemura start	G72601
									SEQ.MSEC_BUFFER[5][4] = SEQ.MSEC_COUNTER;
// add 2016.07.26 K.Uemura end
// add 2016.06.22 K.Uemura end
								}
							}
							break;
					}
					
					//COM0.NO9030 = ADCOV.V_BATT;			// ctl_ad_func()に移動	150512
					//COM0.NO9031 = EXT_POWER_IN;			// main()に移動			150512
					
					SEQ.FLAG3.BIT.EDGE_L_ERROR = 0;			// ｴｯｼﾞLｴﾗｰﾌﾗｸﾞ
					SEQ.FLAG3.BIT.EDGE_R_ERROR = 0;			// ｴｯｼﾞRｴﾗｰﾌﾗｸﾞ
					SEQ.FLAG4.BIT.EDGE_L_POLLING = 0;		// ｴｯｼﾞL検出ﾌﾗｸﾞ(ﾎﾟｰﾘﾝｸﾞ間)
					SEQ.FLAG4.BIT.EDGE_R_POLLING = 0;		// ｴｯｼﾞR検出ﾌﾗｸﾞ(ﾎﾟｰﾘﾝｸﾞ間)
					SEQ.FLAG4.BIT.EDGE_LR_POLLING = 0;		// ｴｯｼﾞLR検出ﾌﾗｸﾞ(ﾎﾟｰﾘﾝｸﾞ間)
					SEQ.BUFFER_COUNT = 0;

					if(SEQ.FLAG2.BIT.MEMORY_RESET == 1){			// 記憶ﾘｾｯﾄﾌﾗｸﾞ
						SEQ.FLAG2.BIT.MEMORY_RESET = 0;				// 記憶ﾘｾｯﾄﾌﾗｸﾞ
						max_min_reset();
					}

					if(SEQ.FLAG.BIT.PORTABLE == OPERATION_MANUAL){		// ﾎﾟｰﾀﾌﾞﾙ操作ﾌﾗｸﾞ
						if(SEQ.FLAG3.BIT.PEAKHOLD_RESET == 1){
							SEQ.FLAG3.BIT.PEAKHOLD_RESET = 0;			// ﾋﾟｰｸﾎｰﾙﾄﾞﾘｾｯﾄﾌﾗｸﾞ
							max_min_reset();

							if(SEQ.SELECT.BIT.MEASURE == MODE_RUNOUT){
								// 再計測(fpga_idleから再スタート)
								SEQ.CHANGE_FPGA = 2;			// FPGA idle
								SEQ.FPGA_SEND_STATUS = 103;
								OUT.SUB_STATUS = 11;
							}
							clear_result();
						}
					}
#if	EXHIBITION
					// for exhibitions 141021
					else{
						if(SEQ.FLAG3.BIT.PEAKHOLD_RESET == 1){
							SEQ.FLAG3.BIT.PEAKHOLD_RESET = 0;		// ﾋﾟｰｸﾎｰﾙﾄﾞﾘｾｯﾄﾌﾗｸﾞ
							max_min_reset();
						}
					}
#endif
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
			
			if(SEQ.FLAG.BIT.AFTER_STOPPING == 0){
				if(F_PRIO_IN == 0){					// F_PRIO_INが「L」のとき
					if(SEQ.FLAG.BIT.MEASUREMENT == MEASURE_STOP){
						if(SEQ.FLAG6.BIT.DATA_TRANSMISSION == 1){		// ﾃﾞｰﾀ送信ﾌﾗｸﾞ
							if((SEQ.SELECT.BIT.MEASURE == MODE_PROFILE)&&(COM0.NO300.BIT.EXE == 1)){	// ﾌﾟﾛﾌｧｲﾙで実行中のとき
								SEQ.FLAG.BIT.MEASUREMENT = 1;		// 計測を開始する
								SEQ.CHANGE_FPGA = 16;				// ﾌﾟﾛﾌｧｲﾙ処理
								SEQ.FPGA_SEND_STATUS = 22;
							}else{
								// 待機を送信(390)
								C_PRIO_OUT	= 1;					// C_PRIO
								SEQ.CBUS_NUMBER = 390;
								SEQ.FPGA_SEND_STATUS = 2;
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
					if(COM0.NO300.BIT.EXE){								// 実行
						COM0.NO310.BIT.STR = 1;							// STROBE
						if((SEQ.FLAG4.BIT.EDGE_L_SCAN == 1) && (SEQ.FLAG4.BIT.EDGE_R_SCAN == 1)){
							SEQ.INPUT_DBUS_LONG = RESULT.LARGE_D_DIFF[0] * 10.0;
						}else{
							SEQ.INPUT_DBUS_LONG = 99999;
						}
						COM0.NO3000[SEQ.PROFILE_BUFFER_COUNT] = SEQ.INPUT_DBUS_LONG;
						SEQ.BUFFER_NO_NEW = SEQ.PROFILE_BUFFER_COUNT;	// ﾊﾞｯﾌｧﾅﾝﾊﾞｰ新
						SEQ.PROFILE_BUFFER_COUNT++;						// ﾌﾟﾛﾌｧｲﾙﾊﾞｯﾌｧ格納用ｶｳﾝﾄ
						if(SEQ.PROFILE_BUFFER_COUNT >= BUFFER_NUMBER)	SEQ.PROFILE_BUFFER_COUNT = 0;
						
					}else{
						COM0.NO310.BIT.RDY = 1;							// READY
					}
				}else{
					if(COM0.NO300.BIT.RST){								// 強制ﾘｾｯﾄがONのとき
						COM0.NO310.BIT.FIN = 1;							// 完了
					}else{
						COM0.NO310.BIT.RDY = 1;							// READY
					}
				}
			}
			break;
			
		case 22:
			if(SEQ.FLAG.BIT.BUFFER_RESET == 1){				// ﾊﾞｯﾌｧｶｳﾝﾄが「0」のとき
				SEQ.FLAG.BIT.BUFFER_RESET = 0;				// ﾊﾞｯﾌｧﾘｾｯﾄﾌﾗｸﾞ
				SEQ.BUFFER_COUNT2 = 0;						// ﾊﾞｯﾌｧｶｳﾝﾄ2(動作確認用)
				
				OUT.SUB_STATUS = 99;						// 表示停止
// chg 2016.06.22 K.Uemura start	G62202
				if(((COM0.NO311 != 152) && (COM0.NO311 != 153)) || (SEQ.CLEANING_CYCLE == 0)){
//				if((COM0.NO311 != 152) || (SEQ.CLEANING_CYCLE == 0)){
// chg 2016.06.22 K.Uemura end
					set_7seg_upper_no_data();					// 7ｾｸﾞ表示(-----)(上段)
					set_7seg_lower_no_data();					// 7ｾｸﾞ表示(-----)(下段)
				}
				
				SEQ.FLAG3.BIT.HDI_OUTPUT = 1;				// HDI出力ﾌﾗｸﾞ
				
				SEQ.FPGA_SEND_STATUS = 11;
				max_min_reset();
				swing_no_tooth_reset();						// 最大値・最小値ﾘｾｯﾄ(振れ刃数0)
				//
				SEQ.PEAKHOLD_COUNT = 0;						// ﾋﾟｰｸﾎｰﾙﾄﾞｶｳﾝﾀ
				// ﾋﾟｰｸﾎｰﾙﾄﾞ実施ﾌﾗｸﾞ(振れの場合は、ﾋﾟｰｸ1点目取得後にON)
				if(SEQ.SELECT.BIT.MEASURE != MODE_RUNOUT){
					SEQ.FLAG3.BIT.PEAKHOLD_ENABLE = 1;		// ﾋﾟｰｸﾎｰﾙﾄﾞ有効ﾌﾗｸﾞ
				}
				SEQ.PEAKHOLD_DATA1[0] = SEQ.PEAKHOLD_DATA2[0] = 0;
				//
				RESULT.GROWTH[0] = 9999;				// 伸び測定最大値初期化(値が無い場合は「9999」とする)	150512
				RESULT.GROWTH[2] = 10000;				// 伸び測定設定値初期化	150512
				
				SEQ.FLAG6.BIT.DATA_TRANSMISSION = 0;		// ﾃﾞｰﾀ送信ﾌﾗｸﾞ
				
				SEQ.END_TIMEOUT_PERIOD	= 0;				// ﾀｲﾑｱｳﾄ時間

// add 2015.08.19 K.Uemura start	振れ座標出力
				if(SEQ.SELECT.BIT.MEASURE == MODE_RUNOUT){
					// 左オフセット(右エッジ検出＠プラスが最大)
					COM0.NO325 = 0x7FFF;		// 最小(最大で初期化)
					COM0.NO326 = 0x8000;		// 最大(最小で初期化)
				}
#if	0
				if(SEQ.SELECT.BIT.MEASURE == MODE_RUNOUT){
					if(SEQ.FLAG2.BIT.AUTO_MODE == 3){
						// max ～ minの設定を反転する（グラフ描画できないため）
						COM0.NO325 = 0x7FFF;		// 最小(最大で初期化)
						COM0.NO326 = 0x8000;		// 最大(最小で初期化)
//						// 右オフセット(左エッジ検出＠マイナスが最大)
//						COM0.NO325 = 0x8000;		// 最大(最小で初期化)
//						COM0.NO326 = 0x7FFF;		// 最小(最大で初期化)
					}else{
						// 左オフセット(右エッジ検出＠プラスが最大)
						COM0.NO325 = 0x7FFF;		// 最小(最大で初期化)
						COM0.NO326 = 0x8000;		// 最大(最小で初期化)
					}
				}
#endif
// add 2015.08.19 K.Uemura end

// deb 2015.07.29 K.Uemura start	
#ifdef	DEBUG_EACHSCAN
memset(&DEBUG_STR.DEBUG_EDGE[0], 0, sizeof(DEBUG_STR.DEBUG_EDGE));
//memset(&DEBUG_STR.DEBUG_EDGE2[0], 0, sizeof(DEBUG_STR.DEBUG_EDGE2));
memset(&DEBUG_STR.DEBUG_FOCUS[0], 0, sizeof(DEBUG_STR.DEBUG_FOCUS));
memset(&DEBUG_STR.DEBUG_NO[0], 0, sizeof(DEBUG_STR.DEBUG_NO));
DEBUG_STR.DEBUG_COUNT = 0;
DEBUG_STR.DEBUG_NO_COUNT = 0;
DEBUG_STR.DEBUG_FLAG = 0;
DEBUG_STR.DEBUG_TIR_TIME = 0;
#endif
// deb 2015.07.29 K.Uemura end

// deb 2015.08.27 K.Uemura start	
COM0.NO140 = 0;
// deb 2015.08.27 K.Uemura end

			}
			break;
	}
}

//************************************************************/
//		エッジ検出結果の設定
//			FPGAより取得した結果（画素）を、
//			あらかじめ設定されたテーブルデータと照らし合わせ
//			画素→μm変換を行う
//				RESULT_EDGE_LEFT      :実寸
//				RESULT_EDGE_LEFT_PIXEL:画素
//************************************************************/
// 左ｴｯｼﾞ検出結果の設定、座標変換
void set_result_edge_left(void)
{
	_UWORD i;
	union UFloatLong Variable;

	// 左エッジ（0画素目が最大）

	// 浮動小数点のﾃﾞｰﾀに変換し値を表示する
	Variable.lLong = SEQ.INPUT_DBUS_LONG;
	SEQ.INPUT_DBUS = Variable.fFloat;
	RESULT.EDGE_LEFT_PIXEL[SEQ.BUFFER_COUNT] = SEQ.INPUT_DBUS;
	// エッジ(最小)
	if(RESULT.EDGE_LEFT_PIXEL[SEQ.BUFFER_COUNT] < RESULT.EDGE_LEFT_PIXEL[0]){
		RESULT.EDGE_LEFT_PIXEL[0] = RESULT.EDGE_LEFT_PIXEL[SEQ.BUFFER_COUNT];
	}
	COM0.NO101 = SEQ.INPUT_DBUS * 10;
	
	SEQ.FLAG3.BIT.EDGE_L_ERROR = 1;							// ｴｯｼﾞLｴﾗｰﾌﾗｸﾞ
	RESULT.EDGE_LEFT_SCALE[SEQ.BUFFER_COUNT] = 0;				// 値を「0」にする

	// 画素が最小画素の末端のとき
	if(SEQ.INPUT_DBUS == LINESENSOR_X_MEASURE_MIN){
		SEQ.INPUT_DBUS = SEQ.INPUT_DBUS_BEFORE[1];			// DBUSの入力ﾃﾞｰﾀ(1つ前のﾃﾞｰﾀ)
	}
	
	// 分割数が「1」より大きいとき
	if(1 < X_DIVISION_NUMBER){
		if(SEQ.TABLE_EDGE_LEFT[0] < (SEQ.INPUT_DBUS*10)){
			for(i=1; i<=X_DIVISION_NUMBER-1; i++){
				// ﾃｰﾌﾞﾙの値が計測値を上回ったとき
				if((SEQ.INPUT_DBUS*10) <= SEQ.TABLE_EDGE_LEFT[i]){
					RESULT.EDGE_LEFT_SCALE[SEQ.BUFFER_COUNT]	= (float)(SEQ.TABLE_EDGE_DISTANCE[i] - SEQ.TABLE_EDGE_DISTANCE[i-1])
															* ((SEQ.INPUT_DBUS*10) - SEQ.TABLE_EDGE_LEFT[i-1])
															/ (SEQ.TABLE_EDGE_LEFT[i] - SEQ.TABLE_EDGE_LEFT[i-1])
															+ SEQ.TABLE_EDGE_DISTANCE[i-1];
					SEQ.FLAG3.BIT.EDGE_L_ERROR = 0;						// ｴｯｼﾞL 正常取得
					break;
				}
			}
		}
	}
	
	SEQ.INPUT_DBUS_BEFORE[1] = SEQ.INPUT_DBUS;				// DBUSの入力ﾃﾞｰﾀ(1つ前のﾃﾞｰﾀ)
	
	// 計測値がﾃｰﾌﾞﾙ全ての値を上回ったとき
	if((SEQ.INPUT_DBUS*10) >= SEQ.TABLE_EDGE_LEFT[X_DIVISION_NUMBER-1]){
		RESULT.EDGE_LEFT_SCALE[SEQ.BUFFER_COUNT] = SEQ.TABLE_EDGE_DISTANCE[X_DIVISION_NUMBER-1];	// 値をﾃｰﾌﾞﾙの最大値にする
		SEQ.FLAG3.BIT.EDGE_L_ERROR = 1;						// ｴｯｼﾞLｴﾗｰﾌﾗｸﾞ
	}
	
	COM0.NO121 = RESULT.EDGE_LEFT_SCALE[SEQ.BUFFER_COUNT] * 10;
}

// 右ｴｯｼﾞ検出結果の設定、座標変換
void set_result_edge_right(void)
{
	_UWORD i;
	union UFloatLong Variable;
	
	// 右エッジ（4095画素目が最大）

	// 浮動小数点のﾃﾞｰﾀに変換し値を表示する
	Variable.lLong = SEQ.INPUT_DBUS_LONG;
	SEQ.INPUT_DBUS = Variable.fFloat;
	RESULT.EDGE_RIGHT_PIXEL[SEQ.BUFFER_COUNT] = SEQ.INPUT_DBUS;
	// エッジ(最大)
	if(RESULT.EDGE_RIGHT_PIXEL[0] < RESULT.EDGE_RIGHT_PIXEL[SEQ.BUFFER_COUNT]){
		RESULT.EDGE_RIGHT_PIXEL[0] = RESULT.EDGE_RIGHT_PIXEL[SEQ.BUFFER_COUNT];
	}
	COM0.NO102 = SEQ.INPUT_DBUS * 10;
	
	SEQ.FLAG3.BIT.EDGE_R_ERROR = 1;							// ｴｯｼﾞRｴﾗｰﾌﾗｸﾞ
	RESULT.EDGE_RIGHT_SCALE[SEQ.BUFFER_COUNT] = 0;
	
	if(SEQ.MEASUREMENT_DIRECTION == X_DIRECTION){				// 計測方向がX方向のとき
		// 画素が最大画素の末端のとき(最小画素の末端)
		if(SEQ.INPUT_DBUS == LINESENSOR_X_MEASURE_MAX){
			SEQ.INPUT_DBUS = SEQ.INPUT_DBUS_BEFORE[2];			// DBUSの入力ﾃﾞｰﾀ(1つ前のﾃﾞｰﾀ)
		}
		
		// 分割数が「1」より大きいとき
		if(1 < X_DIVISION_NUMBER){
			if(SEQ.TABLE_EDGE_RIGHT[0] < (SEQ.INPUT_DBUS*10)){
				for(i=1; i<=X_DIVISION_NUMBER-1; i++){
					// ﾃｰﾌﾞﾙの値が計測値を上回ったとき
					if((SEQ.INPUT_DBUS*10) <= SEQ.TABLE_EDGE_RIGHT[i]){
						RESULT.EDGE_RIGHT_SCALE[SEQ.BUFFER_COUNT]	= (float)(SEQ.TABLE_EDGE_DISTANCE[i] - SEQ.TABLE_EDGE_DISTANCE[i-1])
																* ((SEQ.INPUT_DBUS*10) - SEQ.TABLE_EDGE_RIGHT[i-1])
																/ (SEQ.TABLE_EDGE_RIGHT[i] - SEQ.TABLE_EDGE_RIGHT[i-1])
																+ SEQ.TABLE_EDGE_DISTANCE[i-1];
						SEQ.FLAG3.BIT.EDGE_R_ERROR = 0;						// ｴｯｼﾞR 正常取得
						break;
					}
				}
			}
		}
		
		SEQ.INPUT_DBUS_BEFORE[2] = SEQ.INPUT_DBUS;				// DBUSの入力ﾃﾞｰﾀ(1つ前のﾃﾞｰﾀ)
		
		// 計測値がﾃｰﾌﾞﾙ全ての値を上回ったとき
		if((SEQ.INPUT_DBUS*10) >= SEQ.TABLE_EDGE_RIGHT[X_DIVISION_NUMBER-1]){
			RESULT.EDGE_RIGHT_SCALE[SEQ.BUFFER_COUNT] = SEQ.TABLE_EDGE_DISTANCE[X_DIVISION_NUMBER-1];	// 値をﾃｰﾌﾞﾙの最大値にする
			SEQ.FLAG3.BIT.EDGE_R_ERROR = 1;						// ｴｯｼﾞRｴﾗｰﾌﾗｸﾞ
		}
	
	}else{														// 計測方向がZ方向のとき
		// 画素が最大画素の末端のとき(最小画素の末端)
		if(SEQ.INPUT_DBUS == LINESENSOR_Z_MEASURE_MAX){
			SEQ.INPUT_DBUS = SEQ.INPUT_DBUS_BEFORE[2];			// DBUSの入力ﾃﾞｰﾀ(1つ前のﾃﾞｰﾀ)
		}
		
		// 分割数が「1」より大きいとき
		if(1 < Z_DIVISION_NUMBER){
			if(SEQ.Z_TABLE_EDGE_RIGHT[0] < (SEQ.INPUT_DBUS*10)){
				for(i=1; i<=Z_DIVISION_NUMBER-1; i++){
					// ﾃｰﾌﾞﾙの値が計測値を上回ったとき
					if((SEQ.INPUT_DBUS*10) <= SEQ.Z_TABLE_EDGE_RIGHT[i]){
						RESULT.EDGE_RIGHT_SCALE[SEQ.BUFFER_COUNT]	= (float)(SEQ.Z_TABLE_EDGE_DISTANCE[i] - SEQ.Z_TABLE_EDGE_DISTANCE[i-1])
																* ((SEQ.INPUT_DBUS*10) - SEQ.Z_TABLE_EDGE_RIGHT[i-1])
																/ (SEQ.Z_TABLE_EDGE_RIGHT[i] - SEQ.Z_TABLE_EDGE_RIGHT[i-1])
																+ SEQ.Z_TABLE_EDGE_DISTANCE[i-1];
						SEQ.FLAG3.BIT.EDGE_R_ERROR = 0;						// ｴｯｼﾞR 正常取得
						break;
					}
				}
			}
		}
		
		SEQ.INPUT_DBUS_BEFORE[2] = SEQ.INPUT_DBUS;				// DBUSの入力ﾃﾞｰﾀ(1つ前のﾃﾞｰﾀ)
		
		// 計測値がﾃｰﾌﾞﾙ全ての値を上回ったとき
		if((SEQ.INPUT_DBUS*10) >= SEQ.Z_TABLE_EDGE_RIGHT[Z_DIVISION_NUMBER-1]){
			RESULT.EDGE_RIGHT_SCALE[SEQ.BUFFER_COUNT] = SEQ.Z_TABLE_EDGE_DISTANCE[Z_DIVISION_NUMBER-1];	// 値をﾃｰﾌﾞﾙの最大値にする
			SEQ.FLAG3.BIT.EDGE_R_ERROR = 1;						// ｴｯｼﾞRｴﾗｰﾌﾗｸﾞ
		}
	}
	
	COM0.NO122 = RESULT.EDGE_RIGHT_SCALE[SEQ.BUFFER_COUNT] * 10;
	COM0.NO123 = COM0.NO102 - COM0.NO101;
	COM0.NO124 = COM0.NO122 - COM0.NO121;
}

//************************************************************/
//		焦点結果の設定
//			FPGAより取得した傾きを設定する
//************************************************************/
// 左ｴｯｼﾞ焦点結果の設定
void set_result_focus_left(void)
{
	union UFloatLong Variable;
	float threshold;
	
	// 浮動小数点のﾃﾞｰﾀに変換し値を表示する
	Variable.lLong = SEQ.INPUT_DBUS_LONG;
	SEQ.INPUT_DBUS = Variable.fFloat;
	
	// 焦点左の結果取得
	RESULT.FOCUS_LEFT[SEQ.BUFFER_COUNT] = 99.99999;
	if(SEQ.FLAG3.BIT.EDGE_L_ERROR == 0){								// ｴｯｼﾞLｴﾗｰﾌﾗｸﾞ
		if((0 < SEQ.INPUT_DBUS) && (SEQ.INPUT_DBUS < 99.99)){
			RESULT.FOCUS_LEFT[SEQ.BUFFER_COUNT] = SEQ.INPUT_DBUS;	// 焦点 左
		}
	}
	
	if(SEQ.CHANGE_FPGA == 6){
		// FOCUS LEDの点灯制御
		// 焦点左最小≦LR点灯しきい値のときLEDを点灯
		LED.FOCUS.BIT.L = 0;											// 焦点L
		threshold = get_threshold(SEQ.SELECT.BIT.MEASURE, EDGE_LEFT);
		if(RESULT.FOCUS_LEFT[SEQ.BUFFER_COUNT]*10.0 <= threshold){
			LED.FOCUS.BIT.L = 1;										// 焦点L
		}
	}
	
	SEQ.INPUT_DBUS_LONG = SEQ.INPUT_DBUS * 1000;
	COM0.NO103 = SEQ.INPUT_DBUS_LONG;
}

// 右ｴｯｼﾞ焦点結果の設定
void set_result_focus_right(void)
{
	union UFloatLong Variable;
	float threshold;
	
	// 浮動小数点のﾃﾞｰﾀに変換し値を表示する
	Variable.lLong = SEQ.INPUT_DBUS_LONG;
	SEQ.INPUT_DBUS = Variable.fFloat;
	
	// 焦点右の結果取得
	RESULT.FOCUS_RIGHT[SEQ.BUFFER_COUNT] = 99.99999;
	if(SEQ.FLAG3.BIT.EDGE_R_ERROR == 0){								// ｴｯｼﾞRｴﾗｰﾌﾗｸﾞ
		if((0 < SEQ.INPUT_DBUS) && (SEQ.INPUT_DBUS < 99.99)){
			RESULT.FOCUS_RIGHT[SEQ.BUFFER_COUNT] = SEQ.INPUT_DBUS;	// 焦点 右
		}
	}
	
	if(SEQ.CHANGE_FPGA == 6){
		// FOCUS LEDの点灯制御
		// 焦点右最小≦LR点灯しきい値のときLEDを出力
		LED.FOCUS.BIT.R = 0;											// 焦点R
		LED.FOCUS.BIT.Z = 0;											// 焦点Z
		threshold = get_threshold(SEQ.SELECT.BIT.MEASURE, EDGE_RIGHT);
		if(RESULT.FOCUS_RIGHT[SEQ.BUFFER_COUNT]*10.0 <= threshold){
			// 計測方向がX方向のとき
			if(SEQ.MEASUREMENT_DIRECTION == X_DIRECTION){
				LED.FOCUS.BIT.R = 1;									// 焦点R
			}else{
				LED.FOCUS.BIT.Z = 1;									// 焦点Z
			}
		}
	}
	
	SEQ.INPUT_DBUS_LONG = SEQ.INPUT_DBUS * 1000;
	COM0.NO104 = SEQ.INPUT_DBUS_LONG;
}

//************************************************************/
//		SKIP結果の設定
//			FPGAより取得したSKIP結果を設定する
//************************************************************/
void set_result_skip(void)
{
	SEQ.FOCUSING_HDI = SEQ.INPUT_DBUS_LONG;			// 焦点合わせ
	
	if(IN.FLAG.BIT.HARDWARE_TYPE == CNC_EDITION){
		if(SEQ.MEASUREMENT_DIRECTION == Z_DIRECTION){
			LED.Z_FOCUSING	= SEQ.FOCUSING_HDI & 0x0000000F;		//  4ﾋﾞｯﾄ分のﾏｽｸ
		}else{
			LED.FOCUSING	= SEQ.FOCUSING_HDI & 0x000001FF;		//  9ﾋﾞｯﾄ分のﾏｽｸ
		}
	}else{
			LED.FOCUSING	= SEQ.FOCUSING_HDI & 0x000007FF;		// 11ﾋﾞｯﾄ分のﾏｽｸ
	}
	
	// SKIP検出が存在する場合、ビットをON
	if(LED.FOCUSING){
		COM0.NO310.BIT.LED = 1;
	}else if(LED.Z_FOCUSING){
		COM0.NO310.BIT.LED = 1;
	}else{
		COM0.NO310.BIT.LED = 0;
	}
	
	// SKIP結果とFOCUS結果
	if((IN.FLAG.BIT.HARDWARE_TYPE == CNC_EDITION)&&(SEQ.MEASUREMENT_DIRECTION == Z_DIRECTION)){
		COM0.NO332 = LED.Z_FOCUSING | (LED.FOCUS.BIT.Z << 15);
	}else{
		COM0.NO332 = LED.FOCUSING | (LED.FOCUS.BIT.R << 13) | (LED.FOCUS.BIT.L << 14);
	}
}

//************************************************************/
//				結果出力
//************************************************************/
void result_output(void)
{
	_UWORD i;
	_UBYTE scan_mode;
	_UBYTE result_flag = 0;
	_UBYTE cleaning_count;
	float temp[3];
	
	i = SEQ.BUFFER_COUNT;
	
	if(i >= 1){

		//----------
		// 以下の処理順序は変更しないこと！！
		//   １．検出ﾌﾗｸﾞ設定
		//   ２．OKﾌﾗｸﾞ設定
		//   ３．結果の算出
		//----------

		// ﾌﾗｸﾞ設定
		set_scan_flag( i );

		// 検出結果設定
		set_result( i );

		// 振れﾊﾞｯﾌｧ出力
		if(SEQ.SELECT.BIT.MEASURE == MODE_RUNOUT){

			// 走査方向取得
			scan_mode = get_scan_mode( SEQ.SELECT.BIT.MEASURE, SEQ.FLAG2.BIT.AUTO_MODE );

			if(scan_mode == TOOL_LEFT){
				if(SEQ.FLAG4.BIT.EDGE_R_SCAN == 1){
					// 計測方向がX方向のとき
					if(SEQ.MEASUREMENT_DIRECTION == X_DIRECTION){
						// 右エッジの最大取得(プラスが最大)
						if(RESULT.TIR[0] < RESULT.GREATER_D_DIFF_MAX[0]){
							RESULT.TIR[0] = RESULT.GREATER_D_DIFF_MAX[0];
						}

						// 右エッジの最小取得(マイナスが最大)
						if(RESULT.TIR[6] > RESULT.GREATER_D_DIFF_MIN[0]){
							RESULT.TIR[6] = RESULT.GREATER_D_DIFF_MIN[0];
						}
					}else{
						// 右エッジの最大取得(マイナスが最大)
						// ※Z走査時は符号が反転する
						if(RESULT.TIR[0] > RESULT.GREATER_D_DIFF_MAX[0]){
							RESULT.TIR[0] = RESULT.GREATER_D_DIFF_MAX[0];
						}

						// 右エッジの最小取得(プラスが最大)
						if(RESULT.TIR[6] < RESULT.GREATER_D_DIFF_MIN[0]){
							RESULT.TIR[6] = RESULT.GREATER_D_DIFF_MIN[0];
						}
					}

					if(RESULT.TIR[5] < RESULT.EDGE_RIGHT_PIXEL[0]){
						RESULT.TIR[5] = RESULT.EDGE_RIGHT_PIXEL[0];
					}

					// 振れ計測開始の設定
					if(RESULT.TIR[2] < RESULT.EDGE_RIGHT_PIXEL[0]){
						// 検出結果あり
						result_flag = 1;
					}
				}
			}else{
				if(SEQ.FLAG4.BIT.EDGE_L_SCAN == 1){
					// 左エッジの最大取得(マイナス方向が最大)
					if(RESULT.TIR[0] > RESULT.GREATER_D_DIFF_RIGHT_MIN[0]){
						RESULT.TIR[0] = RESULT.GREATER_D_DIFF_RIGHT_MIN[0];
					}
					
					if(RESULT.TIR[5] > RESULT.EDGE_LEFT_PIXEL[0]){
						RESULT.TIR[5] = RESULT.EDGE_LEFT_PIXEL[0];
					}
					
					// 振れ計測開始の設定
					if(RESULT.TIR[2] > RESULT.EDGE_LEFT_PIXEL[i]){
						result_flag = 1;
					}

					// 右エッジの最小取得(プラス方向が最大)
					if(RESULT.TIR[6] < RESULT.GREATER_D_DIFF_RIGHT_MAX[0]){
						RESULT.TIR[6] = RESULT.GREATER_D_DIFF_RIGHT_MAX[0];
					}
				}
			}

			if(result_flag == 1){
				// SWING_RISEカウンタ
				switch(SEQ.FLAG3.BIT.SWING_RISE){
					case 0:	SEQ.FLAG3.BIT.SWING_RISE++;
						SEQ.MEMORY_TIME = 0;					// 記憶時間(ms)
						break;
					case 1:	SEQ.FLAG3.BIT.SWING_RISE++;	break;
					default:
						break;
				}
			}

			if(SEQ.FLAG3.BIT.SWING_RISE){
				if(SEQ.FLAG3.BIT.SWING_RESET == 0){
					if(SEQ.MEMORY_TIME >= (SEQ.TRIGGER_TIME_PERIOD/2)){
						SEQ.MEMORY_TIME = 0;					// 記憶時間(ms)
						SEQ.FLAG3.BIT.SWING_RESET = 1;			// 振れｶｳﾝﾄﾘｾｯﾄﾌﾗｸﾞ
						max_min_reset();
// deb 2015.07.29 K.Uemura start	
#ifdef	DEBUG_EACHSCAN
DEBUG_STR.DEBUG_FLAG = 1;
#endif
// deb 2015.07.29 K.Uemura end

// deb 2015.08.27 K.Uemura start	
if(COM0.NO310.BIT.DIR == 1){	//	左オフセット
	COM0.NO140 = COM0.NO122;
}else{
	COM0.NO140 = COM0.NO121;
}
// deb 2015.08.27 K.Uemura end

					}
				}
			}else{
				SEQ.MEMORY_TIME = 0;		// 記憶時間(ms)
			}
		}
		
		// ﾌﾗｸﾞ設定
		set_ok_flag( i );

		//----------
		// OK／NGｶｳﾝﾄ数
		//----------
		SEQ.TOTAL_COUNT++;					// 計測総数
		
		if(SEQ.FLAG2.BIT.OK == 1){			// OKﾌﾗｸﾞをｾｯﾄ
			SEQ.OK_COUNT++;					// OKｶｳﾝﾄ数
			SEQ.NG_COUNT = 0;				// 連続NGｶｳﾝﾄ数
		}else{
			SEQ.NG_COUNT++;					// 連続NGｶｳﾝﾄ数
		}
		
		// ADD 150720
		// 終了条件判定
		if(COM0.NO300.BIT.EXE){										// 実行のﾋﾞｯﾄがONのとき
			// 工具径(自動)・工具径(4以下)・工具径(4より上　左側)・工具径(4より上　右側)・振れ測定・ﾌﾟﾛﾌｧｲﾙ・焦点合わせ・中心位置設定のとき
			if((COM0.NO311 >= 10)&&(COM0.NO311 <= 111)&&(COM0.NO311 != 70)){
				if(SEQ.SELECT.BIT.MEASURE != MODE_RUNOUT){
					// ﾀｲﾑｱｳﾄ時間(終了条件)(ms)がﾀｲﾑｱｳﾄ時間ｾｯﾄ(終了条件)(s)以上になったとき
					if(SEQ.END_TIMEOUT_PERIOD_SET > 0){
						if(SEQ.END_TIMEOUT_PERIOD >= SEQ.END_TIMEOUT_PERIOD_SET*1000U){
							SEQ.FLAG.BIT.MEASUREMENT = 0;							// 計測を停止する
							COM0.NO312 = ERR_END_TIMEOUT_PERIOD;					// 5010	ﾀｲﾑｱｳﾄ設定時間に到達
						}
					}
					
					if(SEQ.OK_COUNT_SET > 0){
// chg 2017.01.06 K.Uemura start	H10601	
						// OKｶｳﾝﾄ数(終了条件)がOKｶｳﾝﾄ数ｾｯﾄ(終了条件)以上になったとき かつ WAITカウント数到達時
						if((SEQ.OK_COUNT >= SEQ.OK_COUNT_SET)&&(SEQ.WAIT_COUNT_SET < SEQ.TOTAL_COUNT)){
//						// OKｶｳﾝﾄ数(終了条件)がOKｶｳﾝﾄ数ｾｯﾄ(終了条件)以上になったとき
//						if(SEQ.OK_COUNT >= SEQ.OK_COUNT_SET){
// chg 2017.01.06 K.Uemura end
// add 2016.01.21 K.Uemura start	G12102
							//FINが出力される処理は、READY OFF検知のハンドシェーク待ち
							if(((10 <= COM0.NO311)&&(COM0.NO311 < 30)) || 
							   ((COM0.NO311 == 50)&&(COM0.NO311 == 51))){
								SEQ.FLAG4.BIT.OKCOUNT_FLAG = 1;
							}
// add 2016.01.21 K.Uemura end
							SEQ.FLAG.BIT.MEASUREMENT = 0;							// 計測を停止する
							COM0.NO312 = 0;											// 0 ｴﾗｰなし
						}
					}
				}
				
				// 連続NGｶｳﾝﾄ数(終了条件)が連続NGｶｳﾝﾄ数ｾｯﾄ(終了条件)の100倍以上になったとき
				if(SEQ.NG_COUNT_SET > 0){
					if(SEQ.NG_COUNT >= SEQ.NG_COUNT_SET*100U){
						SEQ.FLAG.BIT.MEASUREMENT = 0;							// 計測を停止する
						COM0.NO312 = ERR_NG_COUNT;								// 5011	連続NGｶｳﾝﾄ数に到達
					}
				}
			}
			
			//
// add 2016.02.18 K.Uemura start	G21804
			// 清掃確認のとき
// chg 2016.06.22 K.Uemura start	G62202
			else if((COM0.NO311 == 152) || (COM0.NO311 == 153)){
//			else if(COM0.NO311 == 152){
// chg 2016.06.22 K.Uemura end
				// 清掃ﾊﾟﾙｽｶｳﾝﾄ(合計)が既定回数になったとき
				if(SEQ.CLEANING_COUNT_TOTAL >= 200){
					// 合計から平均を算出(清掃ﾊﾟﾙｽｶｳﾝﾄ(正常)で割る)
// chg 2016.07.22 K.Uemura start	G72201
//					SEQ.AVE_AVE = (short)(SEQ.AVE_TOTAL / SEQ.CLEANING_COUNT_PASS);	// 平均値
//					SEQ.MIN_AVE = (short)(SEQ.MIN_TOTAL / SEQ.CLEANING_COUNT_PASS);	// 最小値
//					SEQ.MAX_AVE = (short)(SEQ.MAX_TOTAL / SEQ.CLEANING_COUNT_PASS);	// 最大値

					// 切り上げ
					temp[0] = ((float)SEQ.AVE_TOTAL / SEQ.CLEANING_COUNT_PASS);	// 平均値
					temp[1] = ((float)SEQ.MIN_TOTAL / SEQ.CLEANING_COUNT_PASS);	// 最小値
					temp[2] = ((float)SEQ.MAX_TOTAL / SEQ.CLEANING_COUNT_PASS);	// 最大値

					SEQ.AVE_AVE = (short)(ceil(temp[0]));	// 平均値
					SEQ.MIN_AVE = (short)(ceil(temp[1]));	// 最小値
					SEQ.MAX_AVE = (short)(ceil(temp[2]));	// 最大値
// chg 2016.07.22 K.Uemura end
					
					if(SEQ.MEASUREMENT_DIRECTION == X_DIRECTION){
						SEQ.AVE_RATE = (float)SEQ.AVE_AVE / X_DIRTY_BASE_AVE * 100.;
						SEQ.MIN_RATE = (float)SEQ.MIN_AVE / X_DIRTY_BASE_MIN * 100.;
						SEQ.MAX_RATE = (float)SEQ.MAX_AVE / X_DIRTY_BASE_MAX * 100.;
					}else{
						SEQ.AVE_RATE = (float)SEQ.AVE_AVE / Z_DIRTY_BASE_AVE * 100.;
						SEQ.MIN_RATE = (float)SEQ.MIN_AVE / Z_DIRTY_BASE_MIN * 100.;
						SEQ.MAX_RATE = (float)SEQ.MAX_AVE / Z_DIRTY_BASE_MAX * 100.;
					}
					
					if(SEQ.MEASUREMENT_DIRECTION == X_DIRECTION){
						cleaning_count = 0;
						SEQ.BUFFER_NO_OLD = BUFFER_NUMBER+1;					// ﾊﾞｯﾌｧﾅﾝﾊﾞｰ旧
						SEQ.BUFFER_NO_NEW = cleaning_count+11;					// ﾊﾞｯﾌｧﾅﾝﾊﾞｰ新
					}else{
						cleaning_count = 12;
						SEQ.BUFFER_NO_OLD = cleaning_count-1;					// ﾊﾞｯﾌｧﾅﾝﾊﾞｰ旧
						SEQ.BUFFER_NO_NEW = cleaning_count+11;					// ﾊﾞｯﾌｧﾅﾝﾊﾞｰ新
					}
					
					COM0.NO3000[cleaning_count]   = SEQ.AVE_AVE;				// 平均値(平均)
					COM0.NO3000[cleaning_count+1] = SEQ.AVE_MIN;				// 平均値(最小)
					COM0.NO3000[cleaning_count+2] = SEQ.AVE_MAX;				// 平均値(最大)
					COM0.NO3000[cleaning_count+3] = SEQ.AVE_RATE;				// 平均値(率)
					
					COM0.NO3000[cleaning_count+4] = SEQ.MIN_AVE;				// 最小値(平均)
					COM0.NO3000[cleaning_count+5] = SEQ.MIN_MIN;				// 最小値(最小)
					COM0.NO3000[cleaning_count+6] = SEQ.MIN_MAX;				// 最小値(最大)
					COM0.NO3000[cleaning_count+7] = SEQ.MIN_RATE;				// 最小値(率)
					
					COM0.NO3000[cleaning_count+8] = SEQ.MAX_AVE;				// 最大値(平均)
					COM0.NO3000[cleaning_count+9] = SEQ.MAX_MIN;				// 最大値(最小)
					COM0.NO3000[cleaning_count+10] = SEQ.MAX_MAX;				// 最大値(最大)
					COM0.NO3000[cleaning_count+11] = SEQ.MAX_RATE;				// 最大値(率)
					
					SEQ.CLEANING_COUNT_TOTAL = SEQ.CLEANING_COUNT_PASS = 0;		// 清掃ﾊﾟﾙｽｶｳﾝﾄ(合計・正常)
					SEQ.CLEANING_CYCLE++;										// 清掃ｶｳﾝﾄUP
					
					SEQ.FLAG.BIT.MEASUREMENT = 0;								// 計測を停止する
					COM0.NO312 = 0;												// 0 ｴﾗｰなし
// add 2016.07.26 K.Uemura start	G72601
					if(COM0.NO301 == 153){
						if(SEQ.MEASUREMENT_DIRECTION == X_DIRECTION){
							SEQ.MSEC_BUFFER[5][2] = SEQ.MSEC_COUNTER;
						}else{
							SEQ.MSEC_BUFFER[5][3] = SEQ.MSEC_COUNTER;
						}
					}
// add 2016.07.26 K.Uemura end
				}
			}
// add 2016.02.18 K.Uemura end
			//
		}
		//
		
	}else{
		i = SEQ.BUFFER_COUNT;
	}
}


//************************************************************/
//				最大値・最小値ﾘｾｯﾄ
//************************************************************/
void max_min_reset(void)
{
	float max_data,min_data;
	_UBYTE scan_mode;
	
	max_data = INITIAL_MAX;	//
	min_data = INITIAL_MIN;	// 
	
	RESULT.WORK_EDGE_DIFF[0]			= min_data;				// d
	RESULT.WORK_CENTER[0]				= 0;					// d中心
	
	RESULT.SMALL_D_MAX[0]				= min_data;				// d 最大

	RESULT.LARGE_D_MIN[0]				= max_data;				// D 最小
	RESULT.LARGE_D_MAX[0]				= min_data;				// D 最大
	RESULT.LARGE_D_DIFF[0]				= min_data;				// D 最大最小の差
	
	RESULT.SWING_MIN[0]					= max_data;				// 振れ 最小
	RESULT.SWING_MAX[0]					= min_data;				// 振れ 最大
	RESULT.SWING_DIFF[0]				= min_data;				// 振れ 最大最小の差
	
	RESULT.DELTA_X_DIFF[0]				= 0;					// ΔX 差
	RESULT.DELTA_X_DIFF_MIN[0]			= max_data;				// ΔX 差 最小
	RESULT.DELTA_X_DIFF_MAX[0]			= min_data;				// ΔX 差 最大
	
	RESULT.GREATER_D_DIFF_MIN[0]		= max_data;				// D>4 最小
	RESULT.GREATER_D_DIFF_MAX[0]		= min_data;				// D>4 最大
	
	// 計測方向がZ方向のとき
	if(SEQ.MEASUREMENT_DIRECTION == Z_DIRECTION){
		RESULT.GREATER_D_DIFF_MIN[0]	= min_data;				// D>4 最小
		RESULT.GREATER_D_DIFF_MAX[0]	= max_data;				// D>4 最大
	}
	
	RESULT.GREATER_D_DIFF_RIGHT_MIN[0]	= max_data;				// D>4 最小(右側)
	RESULT.GREATER_D_DIFF_RIGHT_MAX[0]	= min_data;				// D>4 最大(右側)
	
	RESULT.EDGE_LEFT_PIXEL[0]			= max_data;				// エッジ 左最小
	RESULT.EDGE_RIGHT_PIXEL[0]			= min_data;				// エッジ 右最小

	RESULT.FOCUS_LEFT_MIN[0]			= max_data;				// 焦点 左最小
	RESULT.FOCUS_RIGHT_MIN[0]			= max_data;				// 焦点 右最小

	SEQ.FLAG4.BIT.EDGE_L_POLLING = 0;					// ﾎﾟｰﾘﾝｸﾞ間検出ﾌﾗｸﾞ
	SEQ.FLAG4.BIT.EDGE_R_POLLING = 0;
	SEQ.FLAG4.BIT.EDGE_LR_POLLING = 0;

	SEQ.FLAG4.BIT.EDGE_L_SCAN = 0;						// 計測開始～検出ﾌﾗｸﾞ
	SEQ.FLAG4.BIT.EDGE_R_SCAN = 0;
	SEQ.FLAG4.BIT.EDGE_LR_SCAN = 0;

	SEQ.PEAKHOLD_DATA1[0] = 99999;
	SEQ.PEAKHOLD_DATA2[0] = 99999;
	SEQ.PEAKHOLD_DATA1[1] = 99999;
	SEQ.PEAKHOLD_DATA2[1] = 99999;

															//         エッジ位置
															// ┏━━━━━━━━━━━┓
															// ┗━━━━━━━━━━━┛
															//     ■■■      ■■■
															//    ①    ②    ③    ④
	RESULT.LEFT_L[0]					= max_data;		// ①左側 左
	RESULT.LEFT_R[0]					= min_data;		// ②左側 右
	RESULT.RIGHT_L[0]					= max_data;		// ③右側 左
	RESULT.RIGHT_R[0]					= min_data;		// ④右側 右

	// 走査方向取得
	scan_mode = get_scan_mode( SEQ.SELECT.BIT.MEASURE, SEQ.FLAG2.BIT.AUTO_MODE );

	if(scan_mode == TOOL_LEFT){
		RESULT.TIR[0] = INITIAL_MIN;
		RESULT.TIR[6] = INITIAL_MAX;

		// 計測方向がZ方向のとき
		if(SEQ.MEASUREMENT_DIRECTION == Z_DIRECTION){
			RESULT.TIR[0] = INITIAL_MAX;
			RESULT.TIR[6] = INITIAL_MIN;
		}
	}else{
		RESULT.TIR[0] = INITIAL_MAX;
		RESULT.TIR[6] = INITIAL_MIN;
	}
}

//************************************************************/
//				最大値・最小値ﾘｾｯﾄ(振れ刃数0)
//************************************************************/
void swing_no_tooth_reset(void)
{
	float max_data,min_data;
	
	max_data = INITIAL_MAX;	//
	min_data = INITIAL_MIN;	// 
	
	RESULT.SWING_MIN[1]					= max_data;				// 振れ 最小
	RESULT.SWING_MAX[1]					= min_data;				// 振れ 最大
}

void output_error(unsigned long line)
{
	COM0.NO312 = line;
}

//************************************************************/
//		実行時間タイマの取得
//************************************************************/
_UWORD get_execute_timer(void)
{
	_UWORD execute_time;

	// ﾁｭｰﾆﾝｸﾞ時間の設定
	// 回転数が「0」のとき    ：DLPM115
	// 回転数が「0」以外のとき：回転数に依存
	//                           1回転に要する時間が1秒以内の時は1秒(例:100ms→1秒)
	//                           1秒以上のときは実際の時間+DLPM800を設定
	execute_time = SEQ.TUNING_SECONDS;

	if(SEQ.SPINDLE_SPEED != 0){
		execute_time = 60000 / SEQ.SPINDLE_SPEED;
		if(execute_time < 1000){
			execute_time = 10;				// 1秒[10倍値]
		}else{
			execute_time /= POLLING_CYCLE;	// 
			execute_time += 1;				// 繰り上げ
		}
	}

	return(execute_time);
}

//************************************************************/
//				動作ﾓｰﾄﾞの取得
//************************************************************/
_UBYTE get_execute_mode( _UBYTE measure, _UBYTE mode )
{
	short retMode = ARGUMENT_ERROR;

	switch(measure){
		case 7:		retMode = MODE_ORIGIN;			break;	// ORIGIN
		case 8:		retMode = MODE_ORIGIN_EDGE;		break;	// ORIGIN(ｴｯｼﾞ考慮)
		case 0:		retMode = MODE_D4_LOW;			break;	// d≦4
		case 1:		retMode = MODE_D4_LEFT;			break;	// d＞4 左(右ｴｯｼﾞ)
		case 11:	retMode = MODE_D4_RIGHT;		break;	// d＞4 右(左ｴｯｼﾞ)
		case 2:		retMode = MODE_RUNOUT;			break;	// 振れ
		case 3:		retMode = MODE_FOCUS;			break;	// 焦点合わせ
		case 4:		retMode = MODE_CENTER;			break;	// 中心位置設定
		case 5:		retMode = MODE_PROFILE;			break;	// ﾌﾟﾛﾌｧｲﾙ
		
		case 6:		retMode = MODE_GROWTH;			break;	// 伸び測定
		
		case MODE_D4_AUTO:		//	自動
			switch(mode){
				case MODE_D4_AUTO_LEFT:
					retMode = MODE_D4_LEFT;			// d＞4 左(右ｴｯｼﾞ)
					break;
				case MODE_D4_AUTO_RIGHT:
					retMode = MODE_D4_RIGHT;		// d＞4 右(左ｴｯｼﾞ)
					break;
			}
			break;
	}

	if(retMode == ARGUMENT_ERROR){
		__FILE__;
		__LINE__;
		output_error(__LINE__);
		COM0.NO312 = ERR_NOMODE;
	}

	return( retMode );
}

//************************************************************/
//				動作ﾓｰﾄﾞの取得(右ｴｯｼﾞ／左ｴｯｼﾞ／両ｴｯｼﾞ／問わない)
//					右ｴｯｼﾞ  ：1 EDGE_LEFT
//					左ｴｯｼﾞ  ：2 EDGE_RIGHT
//					両ｴｯｼﾞ  ：3 EDGE_AND
//					問わない：0 EDGE_OR
//************************************************************/
_UBYTE get_execute_mode_edge( _UBYTE measure, _UBYTE mode )
{
	short retMode = ARGUMENT_ERROR;
	short execute_mode = ARGUMENT_ERROR;

	// 動作ﾓｰﾄﾞ取得

	execute_mode = get_execute_mode( measure, mode );

	switch(execute_mode){
		case MODE_ORIGIN:
		case MODE_ORIGIN_EDGE:
		case MODE_FOCUS:
			retMode = EDGE_OR;
			break;

		case MODE_D4_LOW:		//	d≦4
		case MODE_CENTER:		//	中心位置設定
			retMode = EDGE_AND;
			break;

		case MODE_D4_LEFT:		//	左ｵﾌｾｯﾄ
		case MODE_GROWTH:		// 伸び測定
			retMode = EDGE_RIGHT;
			break;

		case MODE_D4_RIGHT:		//	右ｵﾌｾｯﾄ
			retMode = EDGE_LEFT;
			break;

		case MODE_D4_AUTO:		//	自動
			switch(mode){
				case MODE_D4_AUTO_LEFT:		retMode = EDGE_RIGHT;	break;		// d＞4 左(右ｴｯｼﾞ)
				case MODE_D4_AUTO_RIGHT:	retMode = EDGE_LEFT;	break;		// d＞4 右(左ｴｯｼﾞ)
			}
			break;

		case MODE_RUNOUT:		//	振れ
		case MODE_PROFILE:		//	プロファイル
			switch(mode){
				case MODE_D4_AUTO_LOW:		retMode = EDGE_AND;		break;		// d≦4
				case MODE_D4_AUTO_LEFT:		retMode = EDGE_RIGHT;	break;		// d＞4 左(右ｴｯｼﾞ)
				case MODE_D4_AUTO_RIGHT:	retMode = EDGE_LEFT;	break;		// d＞4 右(左ｴｯｼﾞ)
			}
			break;

		default:
			retMode = ARGUMENT_ERROR;
			break;
	}

	return( retMode );
}

//************************************************************/
//				動作ﾓｰﾄﾞの取得
//					両エッジ検出の場合は、左オフセットを優先
//************************************************************/
_UBYTE get_scan_mode( _UBYTE measure, _UBYTE mode )
{
	short retMode = ARGUMENT_ERROR;

	switch(measure){
		case MODE_RUNOUT:
		case MODE_PROFILE:
			switch(mode){
				case MODE_D4_AUTO_LOW:		// 1:工具径(d≦4)
				case MODE_D4_AUTO_LEFT:		// 2:工具径(d＞4 左側)
					retMode = TOOL_LEFT;
					break;
				default:
					retMode = TOOL_RIGHT;
			}
			break;

		case MODE_D4_LEFT:
			retMode = TOOL_LEFT;
			break;

		case MODE_D4_RIGHT:
			retMode = TOOL_RIGHT;
			break;
	}

	return(retMode);
}

//************************************************************/
//				ﾁｭｰﾆﾝｸﾞﾓｰﾄﾞの取得
//************************************************************/
_UBYTE get_tuning_mode( _UBYTE measure )
{
	_UBYTE retMode = TUNING_NO;

	// 下記ﾓｰﾄﾞは、ﾁｭｰﾆﾝｸﾞ処理が実施される
	switch(measure){
		case MODE_D4_AUTO:
		case MODE_D4_LOW:
		case MODE_D4_LEFT:
		case MODE_D4_RIGHT:
		case MODE_RUNOUT:
		case MODE_PROFILE:
		case MODE_GROWTH:		// 伸び測定
			retMode = TUNING_YES;
			break;
	}

	// 計測方向がZ方向のとき工具長測定ではﾁｭｰﾆﾝｸﾞを行わない
	if(SEQ.MEASUREMENT_DIRECTION == Z_DIRECTION){
		switch(measure){
			case MODE_D4_LEFT:
				retMode = TUNING_NO;
				break;
		}
	}
	
	return(retMode);
}

//************************************************************/
//				傾きしきい値の取得
//				※取得値は10倍値(99.9 → 999)
//					計測各種
//					  チューニングなし：MANUAL_GRADIENT
//					  チューニングあり
//					    左側          ：SEQ.TUNING_FOCUS_LEFT_MIN * 10.0
//					    右側          ：SEQ.TUNING_FOCUS_RIGHT_MIN * 10.0
//					焦点合わせ        ：FOCUSING_GRADIENT
//					ORIGIN(ｴｯｼﾞ考慮)  ：ORIGIN_GRADIENT
//************************************************************/
float get_threshold( _UBYTE measure, _UBYTE mode )
{
	float threshold;
	_UBYTE tuning;

	if(SEQ.CHANGE_FPGA == 5){
		// チューニング中(傾斜画素最大)
		threshold = 999;							// 99.9を指定
	}else{
		// 手動傾斜画素
		threshold = SEQ.MANUAL_GRADIENT;			// SEQ.ALL_DATA[70] [90]

// chg 2016.12.06 K.Uemura start	GC0602
		if(SEQ.FLAG6.BIT.ROUGH_SCAN){
// chg 2017.01.10 K.Uemura start	H11001
			if(ROUGH_SCAN_RATIO != 0){
// chg 2017.01.10 K.Uemura end
				threshold *= (ROUGH_SCAN_RATIO / 10.);
			}
		}
// chg 2016.12.06 K.Uemura end

		if(measure == MODE_FOCUS){					// 焦点合わせ
			threshold = SEQ.FOCUSING_GRADIENT;		// SEQ.ALL_DATA[71] [91]
		}else
		if(measure == MODE_ORIGIN_EDGE){			// ORIGIN(ｴｯｼﾞ考慮)
			threshold = SEQ.ORIGIN_THRESHOLD;		// SEQ.ALL_DATA[72] [92]
		}else{
			// ﾁｭｰﾆﾝｸﾞの存在しないﾓｰﾄﾞの場合はﾃﾞﾌｫﾙﾄ値
			tuning = get_tuning_mode(measure);

			if((SEQ.TUNING_ENABLE == 1) && (tuning == TUNING_YES)){
				switch(mode){
					case EDGE_LEFT:		threshold = SEQ.TUNING_FOCUS_LEFT_MIN*10.0;		break;	// ﾁｭｰﾆﾝｸﾞ 左
					case EDGE_RIGHT:	threshold = SEQ.TUNING_FOCUS_RIGHT_MIN*10.0;	break;	// ﾁｭｰﾆﾝｸﾞ 右
				}
			}
		}
	}

	return( threshold );
}

//************************************************************/
//				動作ﾌﾗｸﾞの設定
//************************************************************/
short set_scan_flag( _UWORD i )
{
	short	ret = -1;
	_UBYTE	execute_mode;
	_UBYTE	execute_mode_edge;
	float	threshold[2];
	float	work;
	_UBYTE	edge_flag_L, edge_flag_R;
	_UBYTE	tuning;
	
	edge_flag_L = edge_flag_R = 0;

	threshold[0] = get_threshold(SEQ.SELECT.BIT.MEASURE, EDGE_LEFT);				// 左
	threshold[1] = get_threshold(SEQ.SELECT.BIT.MEASURE, EDGE_RIGHT);				// 右

	execute_mode = get_execute_mode(SEQ.SELECT.BIT.MEASURE, SEQ.FLAG2.BIT.AUTO_MODE);

	//----------
	// 焦点しきい値との比較
	if(execute_mode == MODE_FOCUS){
		// 焦点数値が得られるとき(99.9以下)は結果ありとする
		if(RESULT.FOCUS_LEFT[i] <= 99.99){
			edge_flag_L = 1;
		}
		if(RESULT.FOCUS_RIGHT[i] <= 99.99){
			edge_flag_R = 1;
		}
	}else{
		if((RESULT.FOCUS_LEFT[i]*10.0) <= threshold[0]){	// 焦点 左
			edge_flag_L = 1;
		}

		if((RESULT.FOCUS_RIGHT[i]*10.0) <= threshold[1]){	// 焦点 右
			edge_flag_R = 1;
		}
	}

	//----------
	// ｴｯｼﾞ未検出時はﾌﾗｸﾞをOFF
	if(RESULT.EDGE_LEFT_PIXEL[i] <= LINESENSOR_X_MEASURE_MIN){
		edge_flag_L = 0;
	}

	if(SEQ.MEASUREMENT_DIRECTION == X_DIRECTION){				// 計測方向がX方向のとき
		if(RESULT.EDGE_RIGHT_PIXEL[i] >= LINESENSOR_X_MEASURE_MAX){
			edge_flag_R = 0;
		}
	}else{
		if(RESULT.EDGE_RIGHT_PIXEL[i] >= LINESENSOR_Z_MEASURE_MAX){
			edge_flag_R = 0;
		}
	}
	
	//----------
	// 換算TBLで変換出来ない時
	execute_mode_edge = get_execute_mode_edge(SEQ.SELECT.BIT.MEASURE, SEQ.FLAG2.BIT.AUTO_MODE);

	// ｴｯｼﾞ左
	if(RESULT.EDGE_LEFT_SCALE[i] == SEQ.TABLE_EDGE_DISTANCE[0]){
		// 左側ｴｯｼﾞが必要な動作ﾓｰﾄﾞ
		if((execute_mode_edge == EDGE_LEFT) || (execute_mode_edge == EDGE_AND)){
			edge_flag_L = 0;
		}
	}

	// ｴｯｼﾞ右
	if(SEQ.MEASUREMENT_DIRECTION == X_DIRECTION){				// 計測方向がX方向のとき
		if(RESULT.EDGE_RIGHT_SCALE[i] == SEQ.TABLE_EDGE_DISTANCE[X_DIVISION_NUMBER-1]){
			// 右側ｴｯｼﾞが必要な動作ﾓｰﾄﾞ
			if((execute_mode_edge == EDGE_RIGHT) || (execute_mode_edge == EDGE_AND)){
				edge_flag_R = 0;
			}
		}
	}else{
		if(RESULT.EDGE_RIGHT_SCALE[i] == SEQ.Z_TABLE_EDGE_DISTANCE[Z_DIVISION_NUMBER-1]){
			// 右側ｴｯｼﾞが必要な動作ﾓｰﾄﾞ
			if((execute_mode_edge == EDGE_RIGHT) || (execute_mode_edge == EDGE_AND)){
				edge_flag_R = 0;
			}
		}
	}
	
	//----------
	// ﾁｭｰﾆﾝｸﾞ条件との比較
	if(SEQ.CHANGE_FPGA != 5){		//	ﾁｭｰﾆﾝｸﾞ実行中は判定不可能

// chg 2015.06.30 K.Uemura start	
		// ﾁｭｰﾆﾝｸﾞの存在しないﾓｰﾄﾞの場合はﾃﾞﾌｫﾙﾄ値
		tuning = get_tuning_mode(SEQ.SELECT.BIT.MEASURE);

		if((SEQ.TUNING_ENABLE == 1) && (tuning == TUNING_YES)){
//		if(SEQ.TUNING_ENABLE == 1){
// chg 2015.06.30 K.Uemura end
			//計測ﾓｰﾄﾞ
			if(execute_mode == MODE_D4_RIGHT){
				// 左ｴｯｼﾞの比較
				if(SEQ.TUNING_EDGE_LEFT_MIN_LOWER < RESULT.EDGE_LEFT_SCALE[i])			// 4095画素に近づく側
				{
					edge_flag_L = 0;
				}
			}

			if(execute_mode == MODE_D4_LEFT){
				// 右ｴｯｼﾞの比較
				if(RESULT.EDGE_RIGHT_SCALE[i] < SEQ.TUNING_EDGE_RIGHT_MAX_LOWER)			// 0画素に近づく側
				{
					edge_flag_R = 0;
				}
			}

			if(execute_mode == MODE_D4_LOW){
				// 工具径との比較
				// 両ｴｯｼﾞが検出できる場合で、細くなる場合に取得しない
				if((edge_flag_L == 1) && (edge_flag_R == 1)){
					work = RESULT.EDGE_RIGHT_SCALE[i] - RESULT.EDGE_LEFT_SCALE[i];
					if(work < SEQ.TUNING_EDGE_SMALL_D_LOWER){
						edge_flag_L = 0;
						edge_flag_R = 0;
					}
				}
			}
		}
	}

	//----------
	// 測定結果の算出(ﾎﾟｰﾘﾝｸﾞ間／周期ﾘｾｯﾄ間)
	//   ﾎﾟｰﾘﾝｸﾞ間に結果が得られるとﾌﾗｸﾞON(1:結果あり)
	//----------
	SEQ.FLAG4.BIT.EDGE_L = edge_flag_L;
	SEQ.FLAG4.BIT.EDGE_R = edge_flag_R;

	if(edge_flag_L == 1){
		SEQ.FLAG4.BIT.EDGE_L_POLLING = 1;
		SEQ.FLAG4.BIT.EDGE_L_SCAN = 1;
	}

	if(edge_flag_R == 1){
		SEQ.FLAG4.BIT.EDGE_R_POLLING = 1;
		SEQ.FLAG4.BIT.EDGE_R_SCAN = 1;
	}

	if((edge_flag_R == 1) && (edge_flag_L == 1)){
		SEQ.FLAG4.BIT.EDGE_LR = 1;
		SEQ.FLAG4.BIT.EDGE_LR_POLLING = 1;
		SEQ.FLAG4.BIT.EDGE_LR_SCAN = 1;
	}else{
		SEQ.FLAG4.BIT.EDGE_LR = 0;
	}

	return( ret );
}

//************************************************************/
//				OKﾌﾗｸﾞの設定
//				※あらかじめ検出ﾌﾗｸﾞの設定が完了していること
//************************************************************/
short set_ok_flag( _UWORD i )
{
	short	ret = -1;
	_UBYTE	execute_mode;
	float	threshold[2];

	// OK・NG判定
	SEQ.FLAG2.BIT.OK = 0;					// OKﾌﾗｸﾞをﾘｾｯﾄ

	threshold[0] = get_threshold(SEQ.SELECT.BIT.MEASURE, EDGE_LEFT);				// 左
	threshold[1] = get_threshold(SEQ.SELECT.BIT.MEASURE, EDGE_RIGHT);				// 右

	if((RESULT.FOCUS_LEFT[i]*10.0) <= threshold[0]){	// 焦点 左
		SEQ.FLAG2.BIT.OK = 1;
	}

	if((RESULT.FOCUS_RIGHT[i]*10.0) <= threshold[1]){	// 焦点 右
		SEQ.FLAG2.BIT.OK = 1;
	}

	execute_mode = get_execute_mode(SEQ.SELECT.BIT.MEASURE, SEQ.FLAG2.BIT.AUTO_MODE);

	if(execute_mode == MODE_CENTER){
		// 両ｴｯｼﾞが検出されないとNG
		if((SEQ.FLAG4.BIT.EDGE_L == 0) || (SEQ.FLAG4.BIT.EDGE_R == 0)){
			SEQ.FLAG2.BIT.OK = 0;
		}
	}else if(execute_mode == MODE_D4_LEFT){
		if(SEQ.FLAG4.BIT.EDGE_R == 0){
			// 右ｴｯｼﾞが無いときNG
			SEQ.FLAG2.BIT.OK = 0;
		}
	}else if(execute_mode == MODE_D4_RIGHT){
		if(SEQ.FLAG4.BIT.EDGE_L == 0){
			// 左ｴｯｼﾞが無いときNG
			SEQ.FLAG2.BIT.OK = 0;
		}
	}else if(execute_mode == MODE_D4_LOW){
		// 左右ｴｯｼﾞが1回も検出されない時はNG
		if(SEQ.FLAG4.BIT.EDGE_LR_SCAN == 0){
			SEQ.FLAG2.BIT.OK = 0;
		}
	}else if(execute_mode == MODE_ORIGIN){
		// SKIP出力が1点でも存在する場合ｶｳﾝﾄｱｯﾌﾟ
		if(SEQ.FOCUSING_HDI >> 11){
			SEQ.FLAG2.BIT.OK = 1;
		}
	}else if(execute_mode == MODE_RUNOUT){
		if(SEQ.FLAG3.BIT.SWING_RISE == 0){
			SEQ.FLAG2.BIT.OK = 0;
		}
	}

	return( ret );
}

//************************************************************/
//				検出結果の設定
//************************************************************/
short set_result( _UWORD i )
{
	short	ret = ARGUMENT_ERROR;

	if(i < 1){
		return( ret );
	}

	//----------
	// 測定結果の算出
	//----------
	if(SEQ.FLAG4.BIT.EDGE_L == 1){
		// 左結果あり
		//-----
#ifdef	OUTPUT232C
#ifdef	__DEBUG
		// エッジ(左)最大を取得
		DEBUG_STR.DEBUG[21] = max(RESULT.EDGE_LEFT_PIXEL[i]*10., DEBUG_STR.DEBUG[21]);
		DEBUG_STR.DEBUG[23] = min(RESULT.EDGE_LEFT_PIXEL[i]*10., DEBUG_STR.DEBUG[23]);
		// 焦点(左)最大を取得
		DEBUG_STR.DEBUG[31] = max(RESULT.FOCUS_LEFT[i]*100., DEBUG_STR.DEBUG[31]);
		DEBUG_STR.DEBUG[33] = min(RESULT.FOCUS_LEFT[i]*100., DEBUG_STR.DEBUG[33]);
#endif
#endif
		// 焦点 左(最小)
		RESULT.FOCUS_LEFT_MIN[i] = RESULT.FOCUS_LEFT[i];
		if(RESULT.FOCUS_LEFT[i] < RESULT.FOCUS_LEFT_MIN[0]){
			RESULT.FOCUS_LEFT_MIN[0] = RESULT.FOCUS_LEFT[i];
		}
		
		// D(最小)
		if(RESULT.EDGE_LEFT_SCALE[i] < RESULT.LARGE_D_MIN[0]){
			RESULT.LARGE_D_MIN[0] = RESULT.EDGE_LEFT_SCALE[i];
			COM0.NO115 = RESULT.LARGE_D_MIN[0] * 10.;	// 左エッジ最大
		}
		
		// D>4 左-中央
		RESULT.GREATER_D_DIFF_RIGHT[i] = RESULT.EDGE_LEFT_SCALE[i] - DL_X_CENTER;

		// D>4 最小(右側)
		if(RESULT.GREATER_D_DIFF_RIGHT[i] < RESULT.GREATER_D_DIFF_RIGHT_MIN[0]){
			RESULT.GREATER_D_DIFF_RIGHT_MIN[0] = RESULT.GREATER_D_DIFF_RIGHT[i];
		}

		// D>4 最大(右側)
		if(RESULT.GREATER_D_DIFF_RIGHT[i] > RESULT.GREATER_D_DIFF_RIGHT_MAX[0]){
			RESULT.GREATER_D_DIFF_RIGHT_MAX[0] = RESULT.GREATER_D_DIFF_RIGHT[i];
		}
	}

	if(SEQ.FLAG4.BIT.EDGE_R == 1){
		// 右結果あり
		//-----
#ifdef	OUTPUT232C
#ifdef	__DEBUG
		// エッジ(右)最大を取得
		DEBUG_STR.DEBUG[22] = max(RESULT.EDGE_RIGHT_PIXEL[i]*10., DEBUG_STR.DEBUG[22]);
		DEBUG_STR.DEBUG[24] = min(RESULT.EDGE_RIGHT_PIXEL[i]*10., DEBUG_STR.DEBUG[24]);
		// 焦点(右)最大を取得
		DEBUG_STR.DEBUG[32] = max(RESULT.FOCUS_RIGHT[i]*100., DEBUG_STR.DEBUG[32]);
		DEBUG_STR.DEBUG[34] = min(RESULT.FOCUS_RIGHT[i]*100., DEBUG_STR.DEBUG[34]);
#endif
#endif

		// 計測方向がX方向のとき
		if(SEQ.MEASUREMENT_DIRECTION == X_DIRECTION){
			// 焦点 右(最小)
			RESULT.FOCUS_RIGHT_MIN[i] = RESULT.FOCUS_RIGHT[i];
			if(RESULT.FOCUS_RIGHT[i] < RESULT.FOCUS_RIGHT_MIN[0]){
				if(RESULT.FOCUS_RIGHT[i]<0){
					ret = RESULT.FOCUS_RIGHT[i];
					ret = ARGUMENT_ERROR;
				}
				RESULT.FOCUS_RIGHT_MIN[0] = RESULT.FOCUS_RIGHT[i];
			}
			
			// D(最大)
			if(LINESENSOR_X_MEASURE_MAX <= RESULT.EDGE_RIGHT_SCALE[i]){
				ret = RESULT.EDGE_RIGHT_SCALE[i];
				ret = ARGUMENT_ERROR;
			}
			if(RESULT.EDGE_RIGHT_SCALE[i] > RESULT.LARGE_D_MAX[0]){
				RESULT.LARGE_D_MAX[0] = RESULT.EDGE_RIGHT_SCALE[i];
				COM0.NO116 = RESULT.LARGE_D_MAX[0] * 10.;	// 右エッジ最大
			}

			// D>4 右-中央
			RESULT.GREATER_D_DIFF[i] = RESULT.EDGE_RIGHT_SCALE[i] - DL_X_CENTER;

			// D>4 最小
			if(RESULT.GREATER_D_DIFF[i] < RESULT.GREATER_D_DIFF_MIN[0]){
				RESULT.GREATER_D_DIFF_MIN[0] = RESULT.GREATER_D_DIFF[i];
			}

			// D>4 最大
			if(RESULT.GREATER_D_DIFF[i] > RESULT.GREATER_D_DIFF_MAX[0]){
				RESULT.GREATER_D_DIFF_MAX[0] = RESULT.GREATER_D_DIFF[i];
			}
		
		// 計測方向がZ方向のとき
		}else{
			// 焦点 右(最小)
			RESULT.FOCUS_RIGHT_MIN[i] = RESULT.FOCUS_RIGHT[i];
			if(RESULT.FOCUS_RIGHT[i] < RESULT.FOCUS_RIGHT_MIN[0]){
				if(RESULT.FOCUS_RIGHT[i]<0){
					ret = RESULT.FOCUS_RIGHT[i];
					ret = ARGUMENT_ERROR;
				}
				RESULT.FOCUS_RIGHT_MIN[0] = RESULT.FOCUS_RIGHT[i];
			}
			
			// D(最大)
			if(LINESENSOR_Z_MEASURE_MAX <= RESULT.EDGE_RIGHT_SCALE[i]){
				ret = RESULT.EDGE_RIGHT_SCALE[i];
				ret = ARGUMENT_ERROR;
			}
			if(RESULT.EDGE_RIGHT_SCALE[i] > RESULT.LARGE_D_MAX[0]){
				RESULT.LARGE_D_MAX[0] = RESULT.EDGE_RIGHT_SCALE[i];
				COM0.NO116 = RESULT.LARGE_D_MAX[0] * 10.;	// 右エッジ最大
			}

			// D>4 右-中央
			RESULT.GREATER_D_DIFF[i] = DL_Z_CENTER - RESULT.EDGE_RIGHT_SCALE[i];
			
			// D>4 最小
			if(RESULT.GREATER_D_DIFF[i] > RESULT.GREATER_D_DIFF_MIN[0]){
				RESULT.GREATER_D_DIFF_MIN[0] = RESULT.GREATER_D_DIFF[i];
			}

			// D>4 最大
			if(RESULT.GREATER_D_DIFF[i] < RESULT.GREATER_D_DIFF_MAX[0]){
				RESULT.GREATER_D_DIFF_MAX[0] = RESULT.GREATER_D_DIFF[i];
			}
		}
	}

	if((SEQ.FLAG4.BIT.EDGE_L_SCAN == 1) && (SEQ.FLAG4.BIT.EDGE_R_SCAN == 1)){
		// 左右 結果あり(両方検出)
		// ※片端ずつでもOK

		// D
		RESULT.LARGE_D_DIFF[0] = RESULT.LARGE_D_MAX[0] - RESULT.LARGE_D_MIN[0];

		// ΔX
		RESULT.DELTA_X_DIFF[0] = (RESULT.LARGE_D_MAX[0] + RESULT.LARGE_D_MIN[0]) / 2.0 - DL_X_CENTER;

		// ΔX 最小
		if(RESULT.DELTA_X_DIFF[i] < RESULT.DELTA_X_DIFF_MIN[0]){
			RESULT.DELTA_X_DIFF_MIN[0] = RESULT.DELTA_X_DIFF[i];
		}

		// ΔX 最大
		if(RESULT.DELTA_X_DIFF[i] > RESULT.DELTA_X_DIFF_MAX[0]){
			RESULT.DELTA_X_DIFF_MAX[0] = RESULT.DELTA_X_DIFF[i];
		}
	}

	if((SEQ.FLAG4.BIT.EDGE_L == 1) && (SEQ.FLAG4.BIT.EDGE_R == 1)){
		// 左右 結果あり(同時検出)
		RESULT.WORK_EDGE_DIFF[i] = RESULT.EDGE_RIGHT_SCALE[i] - RESULT.EDGE_LEFT_SCALE[i];		// WORK ｴｯｼﾞ左右の差
		RESULT.WORK_CENTER[i] = (RESULT.EDGE_RIGHT_SCALE[i] + RESULT.EDGE_LEFT_SCALE[i]) / 2.0;	// WORK 中央

		RESULT.SMALL_D_MAX[i] = RESULT.WORK_EDGE_DIFF[i];
#ifdef	OUTPUT232C
#ifdef	__DEBUG
		// 工具径最大を取得
		DEBUG_STR.DEBUG[10] = max(RESULT.WORK_EDGE_DIFF[i]*10., DEBUG_STR.DEBUG[10]);
//		if(DEBUG_STR.DEBUG[10] < 10000){
//			DEBUG_STR.DEBUG[11] = DEBUG_STR.DEBUG[10];
//		}
#endif
#endif
		// d(最大)
		if(RESULT.WORK_EDGE_DIFF[i] > RESULT.SMALL_D_MAX[0]){
			RESULT.SMALL_D_MAX[0] = RESULT.WORK_EDGE_DIFF[i];
		}

		// d中心(最小)
		if(RESULT.WORK_CENTER[i] < RESULT.SWING_MIN[0]){
			RESULT.SWING_MIN[0] = RESULT.WORK_CENTER[i];
			COM0.NO117 = RESULT.WORK_EDGE_DIFF[i] * 10.;		// 左最大工具径(d)
			COM0.NO119 = RESULT.SWING_MIN[0] * 10.;				// 工具中心左最大
		}

		// d中心(最大)
		if(RESULT.WORK_CENTER[i] > RESULT.SWING_MAX[0]){
			RESULT.SWING_MAX[0] = RESULT.WORK_CENTER[i];
			COM0.NO118 = RESULT.WORK_EDGE_DIFF[i] * 10.;		// 右最大工具径(d)
			COM0.NO120 = RESULT.SWING_MAX[0] * 10.;				// 工具中心右最大
		}
		
		// ADD 150727
		if(SEQ.FLUTES == 0){										// 刃数が「0」のとき
			// 振れ刃数0(最小)
			if(RESULT.SWING_MIN[1] > RESULT.SWING_MIN[0])	RESULT.SWING_MIN[1] = RESULT.SWING_MIN[0];
			// 振れ刃数0(最大)
			if(RESULT.SWING_MAX[1] < RESULT.SWING_MAX[0])	RESULT.SWING_MAX[1] = RESULT.SWING_MAX[0];
		}
		//
		
		// 振れ
		RESULT.SWING_DIFF[0] = RESULT.SWING_MAX[0] - RESULT.SWING_MIN[0];
	}

	return(0);
}

//************************************************************/
//				7セグLEDの設定
//************************************************************/
short set_7seg_led_upper( long temp, _UBYTE measure )
{
	short	ret = ARGUMENT_ERROR;

	if((temp <= INITIAL_MIN) || (INITIAL_MAX <= temp)){
		return(ret);
	}

	if(measure != MODE_FOCUS){
// add 2016.02.18 K.Uemura start	G21804
// chg 2016.06.22 K.Uemura start	G62202
		if((COM0.NO311 == 152) || (COM0.NO311 == 153)){
//		if(COM0.NO311 == 152){
// chg 2016.06.22 K.Uemura end
			LED.SEG_BUF[1] = ' ';
			LED.SEG_BUF[2] = ' ';
			LED.SEG_BUF[3] = ((COM0.NO314 / 100) % 10 + 0x30);		// 百の位
			if(LED.SEG_BUF[3] == '0'){
				LED.SEG_BUF[3] = ' ';	// 空白
			}
			LED.SEG_BUF[4] = ((COM0.NO314 / 10) % 10 + 0x30);		// 十の位
			LED.SEG_BUF[5] = ((COM0.NO314 / 1) % 10 + 0x30);		// 一の位
		}else{
// add 2016.02.18 K.Uemura end
			LED.SEG_BUF[1] = ' ';	// 符号
			// -1の表示が、『-0.000』になるため、-10以下のときに符号を表示
			if(temp <= -10){
				LED.SEG_BUF[1] = '-';
				temp *= -1;
			}
			
			LED.SEG_BUF[2] = ((temp / 10000) % 10 + 0x30);		// 一の位
			LED.SEG_BUF[2] |= 0x80;								// 小数点
			LED.SEG_BUF[3] = ((temp / 1000) % 10 + 0x30);		// 小数第一位
			LED.SEG_BUF[4] = ((temp / 100) % 10 + 0x30);		// 小数第二位
			LED.SEG_BUF[5] = ((temp / 10) % 10 + 0x30);			// 小数第三位
		}
	}else{
		// 3桁以上のときは「99.99」の表示を行う
		if(temp >= 999999){
			temp = 999999;
		}

		LED.SEG_BUF[1] = ' ';	// 符号
		
		LED.SEG_BUF[2] = ((temp / 10000) % 10 + 0x30);		// 十の位
		if(LED.SEG_BUF[2] == '0'){
			LED.SEG_BUF[2] = ' ';	// 空白
		}
		LED.SEG_BUF[3] = ((temp / 1000) % 10 + 0x30);		// 一の位
		LED.SEG_BUF[3] |= 0x80;								// 小数点
		LED.SEG_BUF[4] = ((temp / 100) % 10 + 0x30);		// 小数第一位
		LED.SEG_BUF[5] = ((temp / 10) % 10 + 0x30);			// 小数第二位
	}

	return( ret );
}

//************************************************************/
//				7セグLEDの設定
//************************************************************/
short set_7seg_led_lower( long temp, _UBYTE measure )
{
	short	ret = ARGUMENT_ERROR;

	if((temp <= INITIAL_MIN) || (INITIAL_MAX <= temp)){
		return(ret);
	}

	if(measure != MODE_FOCUS){
// add 2016.02.18 K.Uemura start	G21804
// chg 2016.06.22 K.Uemura start	G62202
		if((COM0.NO311 == 152) || (COM0.NO311 == 153)){
//		if(COM0.NO311 == 152){
// chg 2016.06.22 K.Uemura end
			LED.SEG_BUF[6] = ' ';
			LED.SEG_BUF[7] = ' ';
			LED.SEG_BUF[8] = ((COM0.NO316 / 100) % 10 + 0x30);		// 百の位
			if(LED.SEG_BUF[8] == '0'){
				LED.SEG_BUF[8] = ' ';	// 空白
			}
			LED.SEG_BUF[9] = ((COM0.NO316 / 10) % 10 + 0x30);		// 十の位
			LED.SEG_BUF[10] = ((COM0.NO316 / 1) % 10 + 0x30);		// 一の位
		}else{
// add 2016.02.18 K.Uemura end
			LED.SEG_BUF[6] = ' ';
			// -1の表示が、『-0.000』になるため、-10以下のときに符号を表示
			if(temp <= -10){
				LED.SEG_BUF[6] = '-';
				temp *= -1;
			}
			
			LED.SEG_BUF[7] = ((temp / 10000) % 10 + 0x30);		// 一の位
			LED.SEG_BUF[7] |= 0x80;								// 小数点
			LED.SEG_BUF[8] = ((temp / 1000) % 10 + 0x30);		// 小数第一位
			LED.SEG_BUF[9] = ((temp / 100) % 10 + 0x30);		// 小数第二位
			LED.SEG_BUF[10] = ((temp / 10) % 10 + 0x30);		// 小数第三位
		}
	}else{
		// 3桁以上のときは「99.99」の表示を行う
		if(temp >= 999999){
			temp = 999999;
		}
		
		LED.SEG_BUF[6] = ' ';	// 符号
		
		LED.SEG_BUF[7] = ((temp / 10000) % 10 + 0x30);		// 十の位
		if(LED.SEG_BUF[7] == '0'){
			LED.SEG_BUF[7] = ' ';	// 空白
		}
		LED.SEG_BUF[8] = ((temp / 1000) % 10 + 0x30);		// 一の位
		LED.SEG_BUF[8] |= 0x80;								// 小数点
		LED.SEG_BUF[9] = ((temp / 100) % 10 + 0x30);		// 小数第一位
		LED.SEG_BUF[10] = ((temp / 10) % 10 + 0x30);		// 小数第二位
	}

	return( ret );
}

//************************************************************/
//				7ｾｸﾞ表示(-----)(上段)
//************************************************************/
void set_7seg_upper_no_data(void)
{
	LED.SEG_BUF[1]	= '-';
	LED.SEG_BUF[2]	= '-';
	LED.SEG_BUF[3]	= '-';
	LED.SEG_BUF[4]	= '-';
	LED.SEG_BUF[5]	= '-';
}

//************************************************************/
//				7ｾｸﾞ表示(-----)(下段)
//************************************************************/
void set_7seg_lower_no_data(void)
{
	LED.SEG_BUF[6]	= '-';
	LED.SEG_BUF[7]	= '-';
	LED.SEG_BUF[8]	= '-';
	LED.SEG_BUF[9]	= '-';
	LED.SEG_BUF[10]	= '-';
}
