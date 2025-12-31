/******************************************************************************
* File Name	: fpga_idle.c
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

void send_to_fpga_idle(void);				// FPGAへのﾃﾞｰﾀ送信関数(空回し)

//************************************************************/
//				FPGAへのﾃﾞｰﾀ送信関数(空回し)
//	1 → 102(ポータブル計測遅延) → 103
//	                             → 1
//	                             → 71 [ 0]測定終了
//	
//	  → 103 → 2 ・・・ 8 → 11 → 31 ・・・ → 39 → 1
//	                             → 14 → 15 → 16 → 11
//	                                               → 21 → 11 
//	                                                     → 2  
//	                                                     → 22 正常終了
//	                                                           [ 4]send_to_fpga_auto
//	                                                           [ 5]send_to_fpga_tuning
//	                                                           [ 6]send_to_fpga2
//	                                                           [ 8]send_to_fpga_conversion
//	                                                           [13]send_to_fpga_pixel
//	                                                     → 71 計測停止後1ｻｲｸﾙ取得ﾌﾗｸﾞ
//************************************************************/
// 計測当初は値が安定していないため、空回しを行いﾃﾞｰﾀを破棄する(最大200ms)
void send_to_fpga_idle(void)
{
	_UWORD buf;
	
	switch(SEQ.FPGA_SEND_STATUS){
		
		// ﾎﾟｰﾀﾌﾞﾙ版で計測ﾓｰﾄﾞ(d＞4 自動・d≦4・振れ)のときﾊﾟﾗﾒｰﾀで設定した時間だけ計測開始を遅延する
		case 1:
			SEQ.POWER_COUNT = 0;										// 電源制御ｶｳﾝﾄをﾘｾｯﾄ
			SEQ.FLAG6.BIT.START_DELAY_TIME = 0;							// 計測開始遅延時間ﾌﾗｸﾞ
			SEQ.SELECT.BIT.MEASURE_BEFORE = SEQ.SELECT.BIT.MEASURE;		// 現在の計測ﾓｰﾄﾞを一時記憶
			
			SEQ.MEASUREMENT_DIRECTION_BEFORE = SEQ.MEASUREMENT_DIRECTION;	// 計測方向(直前)を更新する
			
// chg 2016.10.20 K.Uemura start	GA2002
			if((COM0.NO301 == 700)||(COM0.NO301 == 701)||(COM0.NO301 == 710)||(COM0.NO301 == 711))
//			if((COM0.NO301 == 700)||(COM0.NO301 == 701))
				SEQ.MEASUREMENT_DIRECTION_BEFORE = 2;	// 計測方向(直前)をX・Z以外にする ADD 160311
// chg 2016.10.20 K.Uemura end
			
			led_measure_set();											// LED 測定・設定ｾｯﾄ
			SEQ.FPGA_SEND_STATUS = 103;
			if(IN.FLAG.BIT.HARDWARE_TYPE == PORTABLE_EDITION){			// ﾎﾟｰﾀﾌﾞﾙ版のとき
				if(SEQ.FLAG.BIT.PORTABLE){								// ﾎﾟｰﾀﾌﾞﾙの表示基板から計測開始されたとき
					if((SEQ.SELECT.BIT.MEASURE == MODE_D4_LOW)||(SEQ.SELECT.BIT.MEASURE == MODE_D4_AUTO)||(SEQ.SELECT.BIT.MEASURE == MODE_RUNOUT)){
						if(PARAM_START_DELAY_TIME > 0){					// 計測開始遅延時間
							SEQ.START_DELAY_TIME_COUNT = 0;				// 計測開始遅延時間ｶｳﾝﾀ
							SEQ.FLAG6.BIT.START_DELAY_TIME = 1;			// 計測開始遅延時間ﾌﾗｸﾞ
							
							buf = PARAM_START_DELAY_TIME;
							
							// 計測開始時間ｶｳﾝﾄﾀﾞｳﾝ表示
							LED.SEG_BUF[1]	= ' ';
							LED.SEG_BUF[2]	= ' ';
							// 百の位
							if(buf < 100)	LED.SEG_BUF[3] = ' ';
							else			LED.SEG_BUF[3] = (((buf / 100) % 10) + 0x30);
							// 十の位
							if(buf < 10)	LED.SEG_BUF[4] = ' ';
							else			LED.SEG_BUF[4] = (((buf / 10) % 10) + 0x30);
							// 一の位
							LED.SEG_BUF[5]	= ((buf % 10) + 0x30);
							
							set_7seg_lower_no_data();					// 7ｾｸﾞ表示(-----)(下段)
							
							SEQ.FPGA_SEND_STATUS = 102;
						}
					}
				}
			}
			break;
		
		case 102:
			if((SEQ.START_DELAY_TIME_COUNT % 100) == 0) {	// 計測開始遅延時間
				buf = PARAM_START_DELAY_TIME - (unsigned short)(SEQ.START_DELAY_TIME_COUNT / 100);
			
				if(buf == 0){
					set_7seg_upper_no_data();				// 7ｾｸﾞ表示(-----)(上段)
					
					SEQ.FLAG6.BIT.START_DELAY_TIME = 0;		// 計測開始遅延時間ﾌﾗｸﾞ
					SEQ.START_DELAY_TIME_COUNT = 0;			// 計測開始遅延時間ｶｳﾝﾀ
					
					SEQ.FPGA_SEND_STATUS++;					// 次へ
				}else{
					// 計測開始時間ｶｳﾝﾄﾀﾞｳﾝ表示
					LED.SEG_BUF[1]	= ' ';
					LED.SEG_BUF[2]	= ' ';
					// 百の位
					if(buf < 100)	LED.SEG_BUF[3] = ' ';
					else			LED.SEG_BUF[3] = (((buf / 100) % 10) + 0x30);
					// 十の位
					if(buf < 10)	LED.SEG_BUF[4] = ' ';
					else			LED.SEG_BUF[4] = (((buf / 10) % 10) + 0x30);
					// 一の位
					LED.SEG_BUF[5]	= ((buf % 10) + 0x30);
				}
			}
			
			// 途中で計測ﾓｰﾄﾞが変更されたとき
			if(SEQ.SELECT.BIT.MEASURE_BEFORE != SEQ.SELECT.BIT.MEASURE){
				SEQ.FPGA_SEND_STATUS = 1;					// 次へ
			}
			
			// 測定していないとき
			if(SEQ.FLAG.BIT.MEASUREMENT == 0){
				set_7seg_upper_no_data();					// 7ｾｸﾞ表示(-----)(上段)
				
				SEQ.FLAG6.BIT.START_DELAY_TIME = 0;				// 計測開始遅延時間ﾌﾗｸﾞ
				SEQ.START_DELAY_TIME_COUNT = 0;				// 計測開始遅延時間ｶｳﾝﾀ
				
				SEQ.FLAG.BIT.AFTER_STOPPING = 0;
				SEQ.FPGA_SEND_STATUS = 71;
				SEQ.CHANGE_FPGA = 0;
				DA.DADR0 = 0;								// DA0
				
				led_measure_set();							// LED 測定・設定ｾｯﾄ
				
#ifdef	OUTPUT232C
				COM2.MASTER_STATUS = DRV_MODE;
				COM2.SUB_STATUS = 1;
#endif
			}
			break;
		
		// RX → FPGAにﾃﾞｰﾀ送信
		// C_PRIOを「H」にする
		case 103:
			SEQ.FLAG3.BIT.PEAKHOLD_ENABLE = 0;		// ﾋﾟｰｸﾎｰﾙﾄﾞを中断(measureまで待機)
			SEQ.BUFFER_COUNT = 0;					// ﾊﾞｯﾌｧｶｳﾝﾄをﾘｾｯﾄ
			SEQ.FLAG.BIT.AFTER_STOPPING = 0;		// 計測停止後1ｻｲｸﾙ取得ﾌﾗｸﾞ
			SEQ.IDLE_COUNT = 0;						// 空回しｶｳﾝﾄ
			SEQ.TUNING_COUNT = 0;					// ﾁｭｰﾆﾝｸﾞｶｳﾝﾄ
			SEQ.FPGA_RESTART_COUNT = 0;				// FPGA再ｽﾀｰﾄｶｳﾝﾄ
			// ADD 140821
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
			
			if(COM0.NO310.BIT.RDY == 1){			// READYのﾋﾞｯﾄがONしているとき
				SEQ.FLAG2.BIT.PROFILE_AUTO = 0;		// ﾌﾟﾛﾌｧｲﾙ自動判別済ﾌﾗｸﾞ
			}
			
#ifndef	OUTPUT232C
			COM2.MASTER_STATUS = RESET_MODE;
#endif
			
			SEQ.FLAG3.BIT.SWING_RESET = 0;
			
			SEQ.FLAG3.BIT.HDI_OUTPUT = 0;			// HDI出力ﾌﾗｸﾞ
			SEQ.FOCUSING_HDI = 0;					// 11ﾋﾞｯﾄ右にｼﾌﾄ HDI出力を初期化
			LED.FOCUSING = 0;						// X軸LED
			LED.Z_FOCUSING = 0;						// Z軸LED
// add 2015.08.28 K.Uemura start	
			SEQ.INPUT_DBUS_LONG = 0;
			set_result_skip();
// add 2015.08.28 K.Uemura end
			
			//
			DA.DADR0 = SEQ.LED_BRIGHTNESS;			// DA0
// add 2016.10.20 K.Uemura start	GA2002
			// センサ波形確認時は、COM0.NO302を利用する
			// ※COM0.NO302 の最上位bitがONしているときは初期化後の値とする
			if((COM0.NO311 == 710) || (COM0.NO311 == 711)){
				if((COM0.NO302 & 0x8000) == 0){
					DA.DADR0 = COM0.NO302;			// DA0
				}
			}
// add 2016.10.20 K.Uemura end

// chg 2016.06.22 K.Uemura start	G62202
			if((COM0.NO311 == 152)||(COM0.NO311 == 153)){
//			if(COM0.NO311 == 152){
// chg 2016.06.22 K.Uemura end
				DA.DADR0 = SENSOR_INITIAL_BRIGHT2;
#ifdef	__LONGGING_SENSOR_LEVEL
				if(COM0.NO302 != 0){
					DA.DADR0 = COM0.NO302;
				}
#endif
			}

// chg 2016.07.01 K.Uemura start	G70101
#if	1
			// 以下条件の時は結果をｸﾘｱしない(ﾌﾟﾛﾌｧｲﾙ・ﾊﾞｯﾌｧ出力、清掃確認)
			switch(COM0.NO311){
				case 60:
				case 61:
				case 62:
				case 63:
				case 152:
				case 153:
				case 400:
				case 401:
				case 402:
				case 403:
				case 404:
				case 405:
					break;

				default:
					clear_result();						// 結果ｸﾘｱ
			}
#else
//			if((COM0.NO311 != 60) && (COM0.NO311 != 61) && (COM0.NO311 != 62) && (COM0.NO311 != 63) && 
//			   (COM0.NO311 != 401) && (COM0.NO311 != 402) && 
//// chg 2016.06.22 K.Uemura start	G62202
//			   (COM0.NO311 != 152) && (COM0.NO311 != 153)){
////			   (COM0.NO311 != 152)){
//// chg 2016.06.22 K.Uemura end
//				clear_result();						// 結果ｸﾘｱ
//			}
#endif
// chg 2016.07.01 K.Uemura end

			max_min_reset();						// 最大値・最小値ﾘｾｯﾄ
			
			SEQ.SWING_BUFFER_COUNT = 0;				// 振れﾊﾞｯﾌｧ格納用ｶｳﾝﾄ
			SEQ.SWING_BUFFER_COUNT2 = 0;			// 振れﾊﾞｯﾌｧ格納用ｶｳﾝﾄ
			SEQ.GROWTH_OUTPUT_COUNT = 0;			// 伸び計測出力ｶｳﾝﾄ
// add 2015.08.21 K.Uemura start	
			SEQ.GROWTH_MIN = INITIAL_MAX;			// 伸び計測出力ｶｳﾝﾄ
			SEQ.GROWTH_MAX = INITIAL_MIN;			// 伸び計測出力ｶｳﾝﾄ
// add 2015.08.21 K.Uemura end
			
// add 2016.02.18 K.Uemura start	G21804
			// ADD 160226
			// 清掃確認の使用値を初期化
			SEQ.CLEANING_COUNT_TOTAL = SEQ.CLEANING_COUNT_PASS = 0;		// 清掃ﾊﾟﾙｽｶｳﾝﾄ(合計・正常)
			// (平均)
			SEQ.AVE_AVE = SEQ.MIN_AVE = SEQ.MAX_AVE = 0;		// 平均値・最小値・最大値(平均)
			// (最小)
			SEQ.AVE_MIN = SEQ.MIN_MIN = SEQ.MAX_MIN = 255;		// 平均値・最小値・最大値(最小)
			// (最大)
			SEQ.AVE_MAX = SEQ.MIN_MAX = SEQ.MAX_MAX = 0;		// 平均値・最小値・最大値(最大)
			// (合計)
			SEQ.AVE_TOTAL = SEQ.MIN_TOTAL = SEQ.MAX_TOTAL = 0;	// 平均値・最小値・最大値(合計)
			//
// add 2016.02.18 K.Uemura end
			
			//if(F_PRIO_IN == 0){					// F_PRIO_INが「L」のとき
				C_PRIO_OUT	= 1;					// C_PRIO
				SEQ.FPGA_SEND_STATUS = 2;			// 次へ
			//}
			break;
			
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
				SEQ.CHANGE_FPGA = 9;		// 計測停止
			}
			SEQ.FPGA_SEND_STATUS = 11;
			break;
	
		// FPGA → RXにﾃﾞｰﾀ送信
		// F_PRIO_INが「H」になったらC_ACKを「H」にする
		case 11:
			if(C_PRIO_OUT == 1){
				C_PRIO_OUT = 0;
			}else if(F_PRIO_IN == 1){
				SEQ.FPGA_RESTART_COUNT = 0;				// FPGA再ｽﾀｰﾄｶｳﾝﾄ
				SEQ.FPGA_SEND_STATUS = 14;
				C_ACK_OUT	= 1;					// C_ACK
				C_ACK_OUT	= 0;					// C_ACK
				
			}else{	// 1ms間F_PRIO_INが「1」にならないとき一旦停止する
				SEQ.FPGA_RESTART_COUNT++;				// FPGA再ｽﾀｰﾄｶｳﾝﾄ
				if(SEQ.FPGA_RESTART_COUNT >= 1000){
					SEQ.FPGA_SEND_STATUS = 31;
				}
				
			}
			break;
			
		case 14:	// 割り込み後
			SEQ.FPGA_SEND_STATUS++;
			break;
			
		// C_ACKを「H」にする
		case 15:
			SEQ.FPGA_SEND_STATUS++;
			C_ACK_OUT	= 1;							// C_ACK
			break;
			
		// C_ACKを「L」にする
		case 16:
			C_ACK_OUT	= 0;							// C_ACK
			SEQ.FPGA_SEND_STATUS = 11;
			
			if(SEQ.CBUS_NUMBER == SEQ.LAST_CBUS_NUMBER){	// CBUSﾅﾝﾊﾞｰ(1ｻｲｸﾙ最終)のとき
				SEQ.FPGA_SEND_STATUS = 21;
				// 100ms間隔でﾃﾞｰﾀ送信
				if(SEQ.FLAG.BIT.BUFFER_RESET == 1){			// ﾊﾞｯﾌｧｶｳﾝﾄが「0」のとき
					SEQ.FLAG.BIT.BUFFER_RESET = 0;			// ﾊﾞｯﾌｧﾘｾｯﾄﾌﾗｸﾞ
					SEQ.IDLE_COUNT++;						// 空回しｶｳﾝﾄ
				}
				//
			}
			break;
			
		// F_PRIO_INが「L」になっていることを確認する
		case 21:
			SEQ.FPGA_SEND_STATUS = 11;
			
			if(SEQ.FLAG.BIT.AFTER_STOPPING == 0){
				if(F_PRIO_IN == 0){											// F_PRIO_INが「L」のとき
					if(SEQ.FLAG.BIT.MEASUREMENT == 0){
						// 待機を送信(390)
						C_PRIO_OUT	= 1;									// C_PRIO
						SEQ.CBUS_NUMBER = 390;
						SEQ.FPGA_SEND_STATUS = 2;
					}else{
						// ｾﾝｻﾚﾍﾞﾙ取得
						if((COM0.NO311 == 710) || (COM0.NO311 == 711)){
							// 強制的に1秒間のidleを実施する
							if(SEQ.IDLE_COUNT >= 10){			// 空回しｶｳﾝﾄが「SEQ.IDLE_COUNT_SET」以上のとき
								SEQ.IDLE_COUNT = 0;								// 空回しｶｳﾝﾄ
								
								// 「7:ORIGIN」「8:ORIGIN(ｴｯｼﾞ考慮)」「3:焦点合わせ」のとき
								if((SEQ.SELECT.BIT.MEASURE == MODE_ORIGIN     )||
								   (SEQ.SELECT.BIT.MEASURE == MODE_ORIGIN_EDGE)||
								   (SEQ.SELECT.BIT.MEASURE == MODE_FOCUS      )){
								}else{
									COM0.NO310.BIT.RDY = 0;						// READYのﾋﾞｯﾄをOFF
								}
								
								SEQ.AUTO_RIGHT_COUNT = 0;						// 自動判別(右)ｶｳﾝﾀ
								SEQ.AUTO_LEFT_COUNT = 0;						// 自動判別(左)ｶｳﾝﾀ
								SEQ.AUTO_BOTH_COUNT = 0;						// 自動判別(両端)ｶｳﾝﾀ
								SEQ.AUTO_UNDETECTED_COUNT = 0;					// 自動判別(未検出)ｶｳﾝﾀ
								
								SEQ.FPGA_SEND_STATUS = 22;
								
								// ADD 161025
								SEQ.FLAG6.BIT.DEBUG_LEVEL = 1;					// ｾﾝｻﾚﾍﾞﾙﾌﾗｸﾞ
								SEQ.FLAG.BIT.MEASUREMENT = 0;
								// 待機を送信(390)
								C_PRIO_OUT	= 1;								// C_PRIO
								SEQ.CBUS_NUMBER = 390;
								SEQ.FPGA_SEND_STATUS = 2;
								//
							}
						}else{
							if(SEQ.IDLE_COUNT >= SEQ.IDLE_COUNT_SET){			// 空回しｶｳﾝﾄが「SEQ.IDLE_COUNT_SET」以上のとき
								SEQ.IDLE_COUNT = 0;								// 空回しｶｳﾝﾄ
								
								// 「7:ORIGIN」「8:ORIGIN(ｴｯｼﾞ考慮)」「3:焦点合わせ」のとき
								if((SEQ.SELECT.BIT.MEASURE == MODE_ORIGIN     )||
								   (SEQ.SELECT.BIT.MEASURE == MODE_ORIGIN_EDGE)||
								   (SEQ.SELECT.BIT.MEASURE == MODE_FOCUS      )){
								}else{
									COM0.NO310.BIT.RDY = 0;						// READYのﾋﾞｯﾄをOFF
								}
								
								SEQ.AUTO_RIGHT_COUNT = 0;						// 自動判別(右)ｶｳﾝﾀ
								SEQ.AUTO_LEFT_COUNT = 0;						// 自動判別(左)ｶｳﾝﾀ
								SEQ.AUTO_BOTH_COUNT = 0;						// 自動判別(両端)ｶｳﾝﾀ
								SEQ.AUTO_UNDETECTED_COUNT = 0;					// 自動判別(未検出)ｶｳﾝﾀ
								
								SEQ.FPGA_SEND_STATUS = 22;

								// 工具径(自動)のとき
								if(SEQ.SELECT.BIT.MEASURE == MODE_D4_AUTO){
									RESULT.LARGE_D_MIN[0]	= INITIAL_MAX;	// D 最小
									RESULT.LARGE_D_MAX[0]	= INITIAL_MIN;	// D 最大
									SEQ.CHANGE_FPGA = 4;						// 自動判別へ
									
								// 工具径(d≦4)のとき
								}else if(SEQ.SELECT.BIT.MEASURE == MODE_D4_LOW){
									RESULT.LARGE_D_MIN[0]	= INITIAL_MAX;	// D 最小
									RESULT.LARGE_D_MAX[0]	= INITIAL_MIN;	// D 最大
									SEQ.CHANGE_FPGA = 4;						// 自動判別へ
									
								// 振れ測定のとき
								}else if(SEQ.SELECT.BIT.MEASURE == MODE_RUNOUT){
									SEQ.CHANGE_FPGA = 4;						// 自動判別へ
									
	// chg 2016.03.08 K.Uemura start	G32901
	//							// 伸び測定のとき
	//							}else if(SEQ.SELECT.BIT.MEASURE == MODE_GROWTH){
	//								SEQ.CHANGE_FPGA = 4;						// 自動判別へ
	// chg 2016.03.08 K.Uemura end
									
								// ﾌﾟﾛﾌｧｲﾙのとき
								}else if(SEQ.SELECT.BIT.MEASURE == MODE_PROFILE){
									if(SEQ.FLAG2.BIT.PROFILE_AUTO == 0){			// ﾌﾟﾛﾌｧｲﾙ自動判別済ﾌﾗｸﾞ
										RESULT.LARGE_D_MIN[0]	= INITIAL_MAX;	// D 最小
										RESULT.LARGE_D_MAX[0]	= INITIAL_MIN;	// D 最大
	// add 2015.08.19 K.Uemura start	プロファイル最大／最小出力
										SEQ.PROFILE_OUTPUT_COUNT = 0;			// プロファイル計測出力ｶｳﾝﾄ
	// add 2015.08.19 K.Uemura end
										SEQ.CHANGE_FPGA = 4;						// 自動判別へ

	#if	1
										if((COM0.NO311 == 61) || (COM0.NO311 == 62) || (COM0.NO311 == 63)){
											end_condition_set();					// 終了条件ｾｯﾄ

											if(COM0.NO311 == 62 || COM0.NO311 == 61){	// プロファイルＸ(左オフセット＠右エッジ)、プロファイルＺ
												SEQ.FLAG2.BIT.AUTO_MODE = 2;
											}else if(COM0.NO311 == 63){					// プロファイルＸ(右オフセット＠左エッジ)
												SEQ.FLAG2.BIT.AUTO_MODE = 3;
											}

											SEQ.CHANGE_FPGA = 6;						// 本計測へ
											if(SEQ.TUNING_ENABLE == 1){					// ﾁｭｰﾆﾝｸﾞ「有」のとき
												if(SEQ.TUNING_SECONDS > 0){				// ﾁｭｰﾆﾝｸﾞ秒数が「0」より上のとき
													SEQ.BUFFER_COUNT = 2;
													SEQ.CHANGE_FPGA = 5;				// ﾁｭｰﾆﾝｸﾞ処理へ
												}
											}
										}
	#else
	//// add 2016.03.08 K.Uemura start	G30801
	//									if((COM0.NO311 == 62) || (COM0.NO311 == 63)){
	//										if(COM0.NO300.BIT.EXE){						// ﾀｯﾁﾊﾟﾈﾙから実行しているとき
	//											end_condition_set();					// 終了条件ｾｯﾄ
	//										}
	//
	//										if(COM0.NO311 == 62){						// プロファイルＸ(左オフセット＠右エッジ)
	//											SEQ.FLAG2.BIT.AUTO_MODE = 2;
	//										}else if(COM0.NO311 == 63){					// プロファイルＸ(右オフセット＠左エッジ)
	//											SEQ.FLAG2.BIT.AUTO_MODE = 3;
	//										}
	//
	//										SEQ.CHANGE_FPGA = 6;						// 本計測へ
	//										if(SEQ.TUNING_ENABLE == 1){					// ﾁｭｰﾆﾝｸﾞ「有」のとき
	//											if(SEQ.TUNING_SECONDS > 0){				// ﾁｭｰﾆﾝｸﾞ秒数が「0」より上のとき
	//												SEQ.BUFFER_COUNT = 2;
	//												SEQ.CHANGE_FPGA = 5;				// ﾁｭｰﾆﾝｸﾞ処理へ
	//											}
	//										}
	//									}
	//// add 2016.03.08 K.Uemura end
	#endif
									}else{
										if(SEQ.TUNING_ENABLE == 1){					// ﾁｭｰﾆﾝｸﾞ「有」のとき
											if(SEQ.TUNING_SECONDS > 0){				// ﾁｭｰﾆﾝｸﾞ秒数が「0」より上のとき
												SEQ.BUFFER_COUNT = 2;
												SEQ.CHANGE_FPGA = 5;				// ﾁｭｰﾆﾝｸﾞ処理へ
											}
										}else{
											SEQ.CHANGE_FPGA = 6;					// 本計測へ
										}
									}
									
								// 換算ﾃﾞｰﾀ取得のとき
								}else if(COM0.NO311 == 600){
									SEQ.CHANGE_FPGA = 8;							// 換算ﾃﾞｰﾀ取得へ
									SEQ.TABLE_TEMP_COUNT = 0;						// ﾃｰﾌﾞﾙ一時ﾃﾞｰﾀｶｳﾝﾄ
									
								// 特定画素ﾃﾞｰﾀ取得
								}else if((COM0.NO311 == 700)||(COM0.NO311 == 701)){
									SEQ.CHANGE_FPGA = 13;							// 特定画素へ send_to_fpga_pixel()
// add 2016.10.20 K.Uemura start	GA2002
								// ｾﾝｻﾚﾍﾞﾙ取得
								}else if((COM0.NO311 == 710) || (COM0.NO311 == 711)){
									// ADD 161025
									SEQ.FLAG6.BIT.DEBUG_LEVEL = 1;					// ｾﾝｻﾚﾍﾞﾙﾌﾗｸﾞ
									SEQ.FLAG.BIT.MEASUREMENT = 0;
									// 待機を送信(390)
									C_PRIO_OUT	= 1;								// C_PRIO
									SEQ.CBUS_NUMBER = 390;
									SEQ.FPGA_SEND_STATUS = 2;
									//
// add 2016.10.20 K.Uemura end
								}else{
								// 工具径測定、工具長測定、伸び測定
									SEQ.CHANGE_FPGA = 6;							// 本計測へ
									
									// 計測方向がX方向のとき
									if(SEQ.MEASUREMENT_DIRECTION == X_DIRECTION){
										// 動作ﾓｰﾄﾞが工具径のとき
										if((SEQ.SELECT.BIT.MEASURE == MODE_D4_AUTO)||	// 10:工具径(自動)
										   (SEQ.SELECT.BIT.MEASURE == MODE_D4_LOW )||	// 0:工具径(d≦4)
										   (SEQ.SELECT.BIT.MEASURE == MODE_D4_LEFT)||	// 1:工具径(d＞4 左側)
										   (SEQ.SELECT.BIT.MEASURE == MODE_D4_RIGHT)){	// 11:工具径(d＞4 右側)
											if(SEQ.TUNING_ENABLE == 1){					// ﾁｭｰﾆﾝｸﾞ「有」のとき
												if(SEQ.TUNING_SECONDS > 0){				// ﾁｭｰﾆﾝｸﾞ秒数が「0」より上のとき
													SEQ.BUFFER_COUNT = 2;
													SEQ.CHANGE_FPGA = 5;				// ﾁｭｰﾆﾝｸﾞ処理へ
												}
											}
										}
									}
									
									if(SEQ.CHANGE_FPGA == 6){						// 本計測のとき
										if(COM0.NO300.BIT.EXE){						// ﾀｯﾁﾊﾟﾈﾙから実行しているとき
											end_condition_set();					// 終了条件ｾｯﾄ
										}
									}
									
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
			
			//
		// C_PRIOを「H」にする
		case 31:
			//if(F_PRIO_IN == 0){					// F_PRIO_INが「L」のとき
				C_PRIO_OUT	= 1;					// C_PRIO
				SEQ.FPGA_SEND_STATUS++;				// 次へ
			//}
			break;
			
		// ﾎﾟｰﾄを出力に設定する
		case 32:
			bus_to_out();							// ﾊﾞｽを出力に設定
			SEQ.FPGA_SEND_STATUS++;					// 次へ
			break;
			
		// ｺﾏﾝﾄﾞﾊﾞｽ・ﾃﾞｰﾀﾊﾞｽを設定する
		case 33:
			SEQ.CBUS_NUMBER = 390;	// 170131
			send_to_cbus(SEQ.CBUS_NUMBER);			// ｺﾏﾝﾄﾞﾅﾝﾊﾞｰ出力
			send_to_dbus_zero();					// ﾃﾞｰﾀ出力関数0
			SEQ.FPGA_SEND_STATUS++;					// 次へ
			break;
			
		// C_ACKを「H」にする
		case 34:
			C_ACK_OUT	= 1;						// C_ACK
			SEQ.FPGA_SEND_STATUS++;					// 次へ
			break;
			
		// C_ACKを「L」にする
		case 35:
			C_ACK_OUT	= 0;						// C_ACK
			SEQ.FPGA_SEND_STATUS++;					// 次へ
			break;
			
		// C_PRIO・ｺﾏﾝﾄﾞﾊﾞｽ・ﾃﾞｰﾀﾊﾞｽを「L」にする
		case 36:
			send_to_cbus_zero();					// ｺﾏﾝﾄﾞﾅﾝﾊﾞｰ出力関数0
			send_to_dbus_zero();					// ﾃﾞｰﾀ出力関数0
			//SEQ.FPGA_SEND_STATUS++;					// 次へ
			SEQ.FPGA_SEND_STATUS = 43;
			break;
			
		// 強制停止(396)	ADD 170204
		// ｺﾏﾝﾄﾞﾊﾞｽ・ﾃﾞｰﾀﾊﾞｽを設定する
		case 43:
			SEQ.CBUS_NUMBER = 396;
			send_to_cbus(SEQ.CBUS_NUMBER);			// ｺﾏﾝﾄﾞﾅﾝﾊﾞｰ出力
			send_to_dbus_zero();					// ﾃﾞｰﾀ出力関数0
			SEQ.FPGA_SEND_STATUS++;					// 次へ
			break;
			
		// C_ACKを「H」にする
		case 44:
			C_ACK_OUT	= 1;						// C_ACK
			SEQ.FPGA_SEND_STATUS++;					// 次へ
			break;
			
		// C_ACKを「L」にする
		case 45:
			C_ACK_OUT	= 0;						// C_ACK
			SEQ.FPGA_SEND_STATUS++;					// 次へ
			break;
			
		// C_PRIO・ｺﾏﾝﾄﾞﾊﾞｽ・ﾃﾞｰﾀﾊﾞｽを「L」にする
		case 46:
			send_to_cbus_zero();					// ｺﾏﾝﾄﾞﾅﾝﾊﾞｰ出力関数0
			send_to_dbus_zero();					// ﾃﾞｰﾀ出力関数0
			SEQ.FPGA_SEND_STATUS = 37;
			break;
		//
			
		// ﾎﾟｰﾄを入力に設定する
		case 37:
			bus_to_in();							// ﾊﾞｽを入力に設定
			SEQ.FPGA_SEND_STATUS++;					// 次へ
			break;
			
		// C_PRIOを「L」にする
		case 38:
			C_PRIO_OUT	= 0;						// C_PRIO
			
// add 2016.02.18 K.Uemura start	G21804
			if(SEQ.FLAG6.BIT.CLEANING == 0)		SEQ.CBUS_NUMBER = 394;
			else								SEQ.CBUS_NUMBER = 395;
// add 2016.02.18 K.Uemura end
			
			SEQ.FPGA_SEND_STATUS++;					// 次へ
			SEQ.FPGA_RESTART_COUNT = 0;				// FPGA再ｽﾀｰﾄｶｳﾝﾄ
			break;
			//
			
		// 1ms待機
		case 39:
			SEQ.FPGA_RESTART_COUNT++;				// FPGA再ｽﾀｰﾄｶｳﾝﾄ
			if(SEQ.FPGA_RESTART_COUNT >= 1000){
				SEQ.FPGA_SEND_STATUS = 1;
			}
			break;
	}
}
