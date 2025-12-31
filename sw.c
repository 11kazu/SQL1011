/******************************************************************************
* File Name	: sw.c
******************************************************************************/
#include "iodefine.h"
#include "typedefine.h"
#include "usercopy.h"
#include "user_define.h"
#include "parameter.h"

//************************************************************/
//				内部関数プロトタイプ宣言					
//************************************************************/
void sw_input(void);						// ｽｲｯﾁ入力
void led_peakhold_set(void);				// LED ﾋﾟｰｸﾎｰﾙﾄﾞｾｯﾄ
void led_measure_set(void);					// LED 測定・設定ｾｯﾄ

void disp_spindle_speed(void);				// 回転数表示
void disp_flutes(void);						// 刃数表示
void disp_stay_delay_time(void);			// 計測開始遅延時間表示

//************************************************************/
//				ｽｲｯﾁ入力
//				 S1：電源ｽｲｯﾁ			POWER_SW_IN
//				 S2：ｽﾀｰﾄｽｲｯﾁ			START_SW_IN
//				 S3：ｸﾘｱｽｲｯﾁ			CLEAR_SW_IN
//				 S4：ｾﾚｸﾄｽｲｯﾁ			SELECT_SW_IN
//				 S5：ﾋﾟｰｸﾎｰﾙﾄﾞｽｲｯﾁ		PEAKHOLD_SW_IN
//				 S6：ﾓｰﾄﾞｽｲｯﾁ			MODE_SW_IN
//				 S7：上ｽｲｯﾁ				UP_SW_IN
//				 S8：下ｽｲｯﾁ				DOWN_SW_IN
//				 S9：ECOｽｲｯﾁ			ECO_SW_IN
//				   ：ｶﾊﾞｰｽｲｯﾁ開			COVER_OPEN_IN
//				   ：ｶﾊﾞｰｽｲｯﾁ閉			COVER_CLOSE_IN
//************************************************************/
// 100ms以上連続でｽｲｯﾁを押しているとき、または離しているときに、
// ｽｲｯﾁの状態を確定(ﾁｬﾀﾘﾝｸﾞ防止対策)
void sw_input(void)
{
	//_UBYTE i;
	
	// ｽｲｯﾁ入力 //
	// 電源ｽｲｯﾁ(S1)		////////////////////////////////////////////////////////////
	if(POWER_SW_IN){
		IN.POWER_SW.BIT.OFF = 0;
		if(IN.POWER_SW.BIT.ON < 100){						// 1s連続
			IN.POWER_SW.BIT.ON++;
			if(IN.POWER_SW.BIT.ON == 100){					// 1s連続
				IN.FLAG.BIT.POWER_SW = 1;					// 入力あり
				SEQ.FLAG.BIT.POWER = !SEQ.FLAG.BIT.POWER;	// 電源(状態反転)
				
				if(SEQ.FLAG.BIT.POWER){
					if(SEQ.FLAG5.BIT.VOLTAGE_ERROR == 0){
						SEQ.FLAG.BIT.POWER_ON = 1;			// 電源ONﾌﾗｸﾞをｾｯﾄ
					}
				}else{
					OUT.MASTER_STATUS = IDLE_MODE;			// 待機ﾓｰﾄﾞ
					SEQ.FLAG.BIT.POWER_OFF = 1;				// 電源OFFﾌﾗｸﾞをｾｯﾄ
					SEQ.FLAG.BIT.MEASUREMENT = 0;
					SEQ.FLAG.BIT.PORTABLE = 0;				// ﾎﾟｰﾀﾌﾞﾙ操作ﾌﾗｸﾞ
					SEQ.FLAG3.BIT.PEAKHOLD_ENABLE = 0;		// ﾋﾟｰｸﾎｰﾙﾄﾞ有効ﾌﾗｸﾞ
					//SEQ.FLAG.BIT.ECO = 0;
				}
			}
		}
	}else{
		IN.POWER_SW.BIT.ON = 0;
		if(IN.POWER_SW.BIT.OFF < 100){						// 1s連続
			IN.POWER_SW.BIT.OFF++;
			if(IN.POWER_SW.BIT.OFF == 100){					// 1s連続
				IN.FLAG.BIT.POWER_SW = 0;					// 入力なし
			}
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	
	// ｽﾀｰﾄｽｲｯﾁ(S2)		////////////////////////////////////////////////////////////
	if(START_SW_IN){
		IN.START_SW.BIT.OFF = 0;
		if(IN.START_SW.BIT.ON < 10){											// 100ms連続
			IN.START_SW.BIT.ON++;
			if(IN.START_SW.BIT.ON == 10){										// 100ms連続
				IN.FLAG.BIT.START_SW = 1;										// 入力あり
				if(SEQ.FLAG.BIT.POWER){
					if(SEQ.FLAG.BIT.MEASUREMENT == 0){
						SEQ.FLAG.BIT.MEASUREMENT = 1;
						
						SEQ.CBUS_NUMBER = 394;
						SEQ.CHANGE_FPGA = 2;									// 測定
						SEQ.FPGA_SEND_STATUS = 1;
						SEQ.FLAG.BIT.PORTABLE = 1;								// ﾎﾟｰﾀﾌﾞﾙ操作ﾌﾗｸﾞ

						if((SEQ.SELECT.BIT.MEASURE == MODE_D4_LOW) || 			// 1:d≦4
						   (SEQ.SELECT.BIT.MEASURE == MODE_RUNOUT)){			// 2:振れ

							if(SEQ.FLUTES != PARAM_FLUTES){						// 一時ﾃﾞｰﾀ値とﾊﾟﾗﾒｰﾀ値が異なるとき
								PARAM_FLUTES = SEQ.FLUTES;						// 刃数(ﾊﾟﾗﾒｰﾀに書き込み)
								// ﾒﾓﾘに書き込み
								I2C.WR_CONT = 815;
								I2C.WR_BUF[I2C.WR_CONT]		= PARAM_FLUTES >> 8;
								I2C.WR_BUF[I2C.WR_CONT + 1]	= PARAM_FLUTES;
								
								I2C.RE_BUF[I2C.WR_CONT]		= I2C.WR_BUF[I2C.WR_CONT];
								I2C.RE_BUF[I2C.WR_CONT + 1]	= I2C.WR_BUF[I2C.WR_CONT + 1];
								
								I2C.SUB_STATUS = 48;							// 1ﾃﾞｰﾀ分書き込み
								//
							}
							
							// 振れ測定のとき
							if(SEQ.SELECT.BIT.MEASURE == MODE_RUNOUT){			// 2:振れのとき
								if(SEQ.SPINDLE_SPEED != PARAM_SPINDLE_SPEED){	// 一時ﾃﾞｰﾀ値とﾊﾟﾗﾒｰﾀ値が異なるとき
									PARAM_SPINDLE_SPEED = SEQ.SPINDLE_SPEED;	// 回転数(ﾊﾟﾗﾒｰﾀに書き込み)
									// ﾒﾓﾘに書き込み
									I2C.WR_CONT = 811;
									I2C.WR_BUF[I2C.WR_CONT]		= PARAM_SPINDLE_SPEED >> 8;
									I2C.WR_BUF[I2C.WR_CONT + 1]	= PARAM_SPINDLE_SPEED;
									
									I2C.RE_BUF[I2C.WR_CONT]		= I2C.WR_BUF[I2C.WR_CONT];
									I2C.RE_BUF[I2C.WR_CONT + 1]	= I2C.WR_BUF[I2C.WR_CONT + 1];
									
									I2C.SUB_STATUS = 48;						// 1ﾃﾞｰﾀ分書き込み
									//
								}

								// 周期ﾄﾘｶﾞ時間[ms] = 60[s] / 回転数[rpm] / 刃数 * 1000
								SEQ.TRIGGER_TIME_PERIOD = 60000 / SEQ.SPINDLE_SPEED / SEQ.FLUTES;
							}
						}
					}else{
						SEQ.FLAG.BIT.MEASUREMENT = 0;
						SEQ.FLAG.BIT.PORTABLE = 0;								// ﾎﾟｰﾀﾌﾞﾙ操作ﾌﾗｸﾞ
						SEQ.FLAG3.BIT.PEAKHOLD_ENABLE = 0;						// ﾋﾟｰｸﾎｰﾙﾄﾞ有効ﾌﾗｸﾞ
						switch(SEQ.CHANGE_FPGA){
							case 2:
							case 4:
							case 5:
								set_7seg_upper_no_data();						// 7ｾｸﾞ表示(-----)(上段)
								set_7seg_lower_no_data();						// 7ｾｸﾞ表示(-----)(下段)
								OUT.SUB_STATUS = 5;
								break;
						}
					}
				}
			}
		}
	}else{
		IN.START_SW.BIT.ON = 0;
		if(IN.START_SW.BIT.OFF < 10){											// 100ms連続
			IN.START_SW.BIT.OFF++;
			if(IN.START_SW.BIT.OFF == 10){										// 100ms連続
				IN.FLAG.BIT.START_SW = 0;										// 入力なし
			}
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	
	// ｸﾘｱｽｲｯﾁ(S3)		////////////////////////////////////////////////////////////
	if(CLEAR_SW_IN){
		IN.CLEAR_SW.BIT.OFF = 0;
		if(IN.CLEAR_SW.BIT.ON < 10){										// 100ms連続
			IN.CLEAR_SW.BIT.ON++;
			if(IN.CLEAR_SW.BIT.ON == 10){									// 100ms連続
				IN.FLAG.BIT.CLEAR_SW = 1;									// 入力あり
				if(SEQ.FLAG.BIT.POWER){										// 電源が入っているとき
					if(SEQ.FLAG.BIT.MEASUREMENT == 0){						// 測定していないとき
						// 設定値をｸﾘｱ
						if(SEQ.SELECT.BIT.MEASURE == MODE_D4_LOW){			// 0:d≦4のとき
							SEQ.FLUTES			 = INIT_FLUTES;				// 刃数
						}else if(SEQ.SELECT.BIT.MEASURE == MODE_RUNOUT){	// 2:振れのとき
							SEQ.SPINDLE_SPEED	 = INIT_SPINDLE_SPEED;		// 回転数
							SEQ.FLUTES			 = INIT_FLUTES;				// 刃数
						}else if(SEQ.SELECT.BIT.MEASURE == MODE_FOCUS){		// 3:焦点のとき
							SEQ.START_DELAY_TIME = INIT_START_DELAY_TIME;	// 計測開始遅延時間
						}
						led_measure_set();									// LED 測定・設定ｾｯﾄ
						
						// 表示を消去
						LED.FOCUSING = 0x0000;								// FOCUS全消灯
						LED.FOCUS.BIT.L = 0;
						LED.FOCUS.BIT.R = 0;
						LED.FOCUS.BIT.Z = 0;								// 焦点Z
						set_7seg_upper_no_data();							// 7ｾｸﾞ表示(-----)(上段)
						set_7seg_lower_no_data();							// 7ｾｸﾞ表示(-----)(下段)
						
					}else{													// 測定しているとき
						// 計測結果をｸﾘｱ
						if(SEQ.FLAG.BIT.PORTABLE == 1){						// ﾎﾟｰﾀﾌﾞﾙ操作ﾌﾗｸﾞ
							SEQ.FLAG3.BIT.PEAKHOLD_RESET = 1;				// ﾋﾟｰｸﾎｰﾙﾄﾞﾘｾｯﾄﾌﾗｸﾞ
							SEQ.PEAKHOLD_COUNT = 0;
						}
					}
				}
			}
		}
	}else{
		IN.CLEAR_SW.BIT.ON = 0;
		if(IN.CLEAR_SW.BIT.OFF < 10){										// 100ms連続
			IN.CLEAR_SW.BIT.OFF++;
			if(IN.CLEAR_SW.BIT.OFF == 10){									// 100ms連続
				IN.FLAG.BIT.CLEAR_SW = 0;									// 入力なし
			}
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	
	// ｾﾚｸﾄｽｲｯﾁ(S4)		////////////////////////////////////////////////////////////
	if(SELECT_SW_IN){
		IN.SELECT_SW.BIT.OFF = 0;
		IN.SELECT_SW.BIT.ON++;
		if(IN.SELECT_SW.BIT.ON == 10){					// 100ms連続
			IN.FLAG.BIT.SELECT_SW = 1;					// 入力あり
		}else if(IN.SELECT_SW.BIT.ON == 50){			// 500ms連続
			IN.FLAG.BIT.SELECT_SW = 1;					// 入力あり
		}else if(IN.SELECT_SW.BIT.ON >= 51){
			IN.SELECT_SW.BIT.ON = 11;
		}
		
		if(IN.FLAG.BIT.SELECT_SW == 1){					// 入力あり
			IN.FLAG.BIT.SELECT_SW = 0;
			if(SEQ.FLAG.BIT.POWER){
				if(SEQ.FLAG.BIT.MEASUREMENT == 0){
					// 0:d≦4または2:振れ
					if((SEQ.SELECT.BIT.MEASURE == MODE_D4_LOW)||(SEQ.SELECT.BIT.MEASURE == MODE_RUNOUT)){
						SEQ.FLUTES++;
						if(SEQ.FLUTES > 9){
							SEQ.FLUTES = 0;
						}
						led_measure_set();				// LED 測定・設定ｾｯﾄ
					}
				}
			}
		}
			
	}else{
		IN.SELECT_SW.BIT.ON = 0;
		if(IN.SELECT_SW.BIT.OFF < 10){					// 100ms連続
			IN.SELECT_SW.BIT.OFF++;
			if(IN.SELECT_SW.BIT.OFF == 10){				// 100ms連続
				IN.FLAG.BIT.SELECT_SW = 0;				// 入力なし
			}
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	
	// ﾋﾟｰｸﾎｰﾙﾄﾞｽｲｯﾁ(S5)	////////////////////////////////////////////////////////
	if(PEAKHOLD_SW_IN){
		IN.PEAKHOLD_SW.BIT.OFF = 0;
		if(IN.PEAKHOLD_SW.BIT.ON < 10){										// 100ms連続
			IN.PEAKHOLD_SW.BIT.ON++;
			if(IN.PEAKHOLD_SW.BIT.ON == 10){								// 100ms連続
				IN.FLAG.BIT.PEAKHOLD_SW = 1;								// 入力あり
				if(SEQ.FLAG.BIT.POWER){
//					if(SEQ.FLAG.BIT.MEASUREMENT == 0){
						// 0:d≦4・10:d＞4・2:振れのとき
						if((SEQ.SELECT.BIT.MEASURE == MODE_D4_LOW)||(SEQ.SELECT.BIT.MEASURE == MODE_D4_AUTO)||(SEQ.SELECT.BIT.MEASURE == MODE_RUNOUT)){
							if(SEQ.SELECT.BIT.PEAKHOLD < PEAK_HOLD_10S){
								SEQ.SELECT.BIT.PEAKHOLD++;
							}else{
								SEQ.SELECT.BIT.PEAKHOLD = PEAK_HOLD_OFF;	
							}
							led_peakhold_set();								// LED ﾋﾟｰｸﾎｰﾙﾄﾞｾｯﾄ
							max_min_reset();
							SEQ.FLAG3.BIT.PEAKHOLD_RESET = 1;				// ﾋﾟｰｸﾎｰﾙﾄﾞﾘｾｯﾄﾌﾗｸﾞ
							SEQ.PEAKHOLD_COUNT = 0;
							
							if(SEQ.SELECT.BIT.MEASURE == MODE_RUNOUT){		// 2:振れ
// add 2015.03.17 K.Uemura start	F31601	計測停止時に再計測させない
								if(SEQ.FLAG.BIT.MEASUREMENT == MEASURE_RUNNING){
// add 2015.03.17 K.Uemura end
									// 再計測(fpga_idleから再スタート)
									SEQ.CHANGE_FPGA = 2;					// FPGA idle
									SEQ.FPGA_SEND_STATUS = 103;
									OUT.SUB_STATUS = 11;
									SEQ.FLAG3.BIT.PEAKHOLD_ENABLE = 0;
									SEQ.FLAG3.BIT.PEAKHOLD_RESET = 0;		// ﾋﾟｰｸﾎｰﾙﾄﾞﾘｾｯﾄﾌﾗｸﾞ
								}
							}
						}
//					}
				}
			}
		}
	}else{
		IN.PEAKHOLD_SW.BIT.ON = 0;
		if(IN.PEAKHOLD_SW.BIT.OFF < 10){									// 100ms連続
			IN.PEAKHOLD_SW.BIT.OFF++;
			if(IN.PEAKHOLD_SW.BIT.OFF == 10){								// 100ms連続
				IN.FLAG.BIT.PEAKHOLD_SW = 0;								// 入力なし
			}
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	
	// ﾓｰﾄﾞｽｲｯﾁ(S6)		////////////////////////////////////////////////////////////
	if(MODE_SW_IN){
		IN.MODE_SW.BIT.OFF = 0;
		if(IN.MODE_SW.BIT.ON < 10){												// 100ms連続
			IN.MODE_SW.BIT.ON++;
			if(IN.MODE_SW.BIT.ON == 10){										// 100ms連続
				IN.FLAG.BIT.MODE_SW = 1;										// 入力あり
				if(SEQ.FLAG.BIT.POWER){
					// 計測中もﾓｰﾄﾞを変更可能
					//if(SEQ.FLAG.BIT.MEASUREMENT == 0){
						// 計測中のとき一旦計測を停止する
						if(SEQ.FLAG6.BIT.START_DELAY_TIME == 0){				// 計測開始遅延時間ﾌﾗｸﾞが「0」のとき
							if(SEQ.FLAG.BIT.MEASUREMENT == 1){
								SEQ.FLAG.BIT.MEASUREMENT = 0;					// 計測を停止する
								SEQ.FLAG.BIT.CHANGE_MODE = 1;					// 計測ﾓｰﾄﾞ切り替えﾌﾗｸﾞ
							}
						}
						
						if(SEQ.SELECT.BIT.MEASURE == MODE_CENTER){				// 工具径(4以下)
							COM0.NO311 = 10;
							SEQ.SELECT.BIT.MEASURE = MODE_D4_LOW;
							
						}else if(SEQ.SELECT.BIT.MEASURE == MODE_D4_LOW){		// 工具径(自動)
							COM0.NO311 = 20;
							SEQ.SELECT.BIT.MEASURE = MODE_D4_AUTO;
							
						}else if(SEQ.SELECT.BIT.MEASURE == MODE_D4_AUTO){		// 振れ測定
							COM0.NO311 = 50;
							SEQ.SELECT.BIT.MEASURE = MODE_RUNOUT;
							SEQ.SPINDLE_SPEED	= PARAM_SPINDLE_SPEED;			// 回転数
//							SEQ.FLUTES = PARAM_FLUTES;							// 刃数
							
						}else if(SEQ.SELECT.BIT.MEASURE == MODE_RUNOUT){		// 焦点合わせ
							COM0.NO311 = 100;
							SEQ.SELECT.BIT.MEASURE = MODE_FOCUS;
							SEQ.START_DELAY_TIME	= PARAM_START_DELAY_TIME;	// 計測開始遅延時間
							
						}else if(SEQ.SELECT.BIT.MEASURE == MODE_FOCUS){			// 中心位置設定
							COM0.NO311 = 110;
							SEQ.SELECT.BIT.MEASURE = MODE_CENTER;
							
							// 焦点合わせ測定にて計測開始遅延時間が変更されているときﾊﾟﾗﾒｰﾀに書き込む
							if(SEQ.START_DELAY_TIME != PARAM_START_DELAY_TIME){	// 一時ﾃﾞｰﾀ値とﾊﾟﾗﾒｰﾀ値が異なるとき
								PARAM_START_DELAY_TIME = SEQ.START_DELAY_TIME;	// 回転数(ﾊﾟﾗﾒｰﾀに書き込み)
								// ﾒﾓﾘに書き込み
								I2C.WR_CONT = 3799;
								I2C.WR_BUF[I2C.WR_CONT]		= PARAM_START_DELAY_TIME >> 8;
								I2C.WR_BUF[I2C.WR_CONT + 1]	= PARAM_START_DELAY_TIME;
								
								I2C.RE_BUF[I2C.WR_CONT]		= I2C.WR_BUF[I2C.WR_CONT];
								I2C.RE_BUF[I2C.WR_CONT + 1]	= I2C.WR_BUF[I2C.WR_CONT + 1];
								
								I2C.SUB_STATUS = 48;							// 1ﾃﾞｰﾀ分書き込み
								//
							}
							
						}else{													// 上記以外は焦点合わせ
							COM0.NO311 = 100;
							SEQ.SELECT.BIT.MEASURE = MODE_FOCUS;
							SEQ.START_DELAY_TIME	= PARAM_START_DELAY_TIME;	// 計測開始遅延時間
						}
						
						SEQ.FLAG3.BIT.PEAKHOLD_ENABLE = 0;						// ﾋﾟｰｸﾎｰﾙﾄﾞ有効ﾌﾗｸﾞ
						SEQ.SELECT.BIT.PEAKHOLD = PEAK_HOLD_5S;
						LED.FOCUSING = 0x0000;									// FOCUS全消灯
						LED.FOCUS.BIT.L = 0;
						LED.FOCUS.BIT.R = 0;
						LED.FOCUS.BIT.Z = 0;									// 焦点Z
						OUT.SUB_STATUS = 5;
						set_7seg_upper_no_data();								// 7ｾｸﾞ表示(-----)(上段)
						set_7seg_lower_no_data();								// 7ｾｸﾞ表示(-----)(下段)
						led_measure_set();										// LED 測定・設定ｾｯﾄ
						led_peakhold_set();										// LED ﾋﾟｰｸﾎｰﾙﾄﾞｾｯﾄ
					//}
				}
			}
		}
	}else{
		IN.MODE_SW.BIT.ON = 0;
		if(IN.MODE_SW.BIT.OFF < 10){											// 100ms連続
			IN.MODE_SW.BIT.OFF++;
			if(IN.MODE_SW.BIT.OFF == 10){										// 100ms連続
				IN.FLAG.BIT.MODE_SW = 0;										// 入力なし
			}
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	
	// 上ｽｲｯﾁ		////////////////////////////////////////////////////////////////
	if(UP_SW_IN){
		IN.UP_SW.BIT.OFF = 0;
		IN.UP_SW.BIT.ON++;
		
		if(SEQ.FLAG.BIT.POWER){
			if(IN.UP_SW.BIT.ON >= 500){					// 5000ms連続
				IN.UP_SW.BIT.ON = 500;		
				IN.FLAG.BIT.UP_SW = 1;					// 入力あり
			}else if(IN.UP_SW.BIT.ON >= 400){			// 4000ms連続
				if(IN.UP_SW.BIT.ON % 5 == 0){			// 「5」で割った余りが「0」のとき
					IN.FLAG.BIT.UP_SW = 1;				// 入力あり
				}else{
					IN.FLAG.BIT.UP_SW = 0;				// 入力なし
				}
			}else if(IN.UP_SW.BIT.ON >= 300){			// 3000ms連続
				if(IN.UP_SW.BIT.ON % 10 == 0){			// 「10」で割った余りが「0」のとき
					IN.FLAG.BIT.UP_SW = 1;				// 入力あり
				}else{
					IN.FLAG.BIT.UP_SW = 0;				// 入力なし
				}
			}else if(IN.UP_SW.BIT.ON >= 200){			// 2000ms連続
				if(IN.UP_SW.BIT.ON % 20 == 0){			// 「20」で割った余りが「0」のとき
					IN.FLAG.BIT.UP_SW = 1;				// 入力あり
				}else{
					IN.FLAG.BIT.UP_SW = 0;				// 入力なし
				}
			}else if(IN.UP_SW.BIT.ON >= 100){			// 1000ms連続
				if(IN.UP_SW.BIT.ON % 50 == 0){			// 「50」で割った余りが「0」のとき
					IN.FLAG.BIT.UP_SW = 1;				// 入力あり
				}else{
					IN.FLAG.BIT.UP_SW = 0;				// 入力なし
				}
			}else if(IN.UP_SW.BIT.ON == 50){			// 500ms連続
				IN.FLAG.BIT.UP_SW = 1;					// 入力あり
			}else if(IN.UP_SW.BIT.ON == 10){			// 100ms連続
				IN.FLAG.BIT.UP_SW = 1;					// 入力あり
			}else{
				IN.FLAG.BIT.UP_SW = 0;					// 入力なし
			}
			
			if(IN.FLAG.BIT.UP_SW == 1){									// 入力ありのとき
				if(SEQ.FLAG.BIT.MEASUREMENT == 0){
					if(SEQ.SELECT.BIT.MEASURE == MODE_RUNOUT){			// 2:振れのとき
						if(SEQ.FLUTES != 0){
							SEQ.SPINDLE_SPEED++;
							if(SEQ.SPINDLE_SPEED > SPINDLE_SPEED_MAX){
								SEQ.SPINDLE_SPEED = SPINDLE_SPEED_MIN;
							}
						}
						
					}else if(SEQ.SELECT.BIT.MEASURE == MODE_FOCUS){		// 3:焦点のとき
						SEQ.START_DELAY_TIME++;
						if(SEQ.START_DELAY_TIME > 999){
							SEQ.START_DELAY_TIME = 0;
						}
					}
					led_measure_set();									// LED 測定・設定ｾｯﾄ
				}
				IN.FLAG.BIT.UP_SW = 0;									// 入力なし
			}
		}
	}else{
		IN.UP_SW.BIT.ON = 0;
		if(IN.UP_SW.BIT.OFF < 10){										// 100ms連続
			IN.UP_SW.BIT.OFF++;
			if(IN.UP_SW.BIT.OFF == 10){									// 100ms連続
				IN.FLAG.BIT.UP_SW = 0;									// 入力なし
			}
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	
	// 下ｽｲｯﾁ		////////////////////////////////////////////////////////////////
	if(DOWN_SW_IN){
		IN.DOWN_SW.BIT.OFF = 0;
		IN.DOWN_SW.BIT.ON++;
		
		if(SEQ.FLAG.BIT.POWER){
			if(IN.DOWN_SW.BIT.ON >= 500){					// 5000ms連続
				IN.DOWN_SW.BIT.ON = 500;		
				IN.FLAG.BIT.DOWN_SW = 1;					// 入力あり
			}else if(IN.DOWN_SW.BIT.ON >= 400){				// 4000ms連続
				if(IN.DOWN_SW.BIT.ON % 5 == 0){				// 「5」で割った余りが「0」のとき
					IN.FLAG.BIT.DOWN_SW = 1;				// 入力あり
				}else{
					IN.FLAG.BIT.DOWN_SW = 0;				// 入力なし
				}
			}else if(IN.DOWN_SW.BIT.ON >= 300){				// 3000ms連続
				if(IN.DOWN_SW.BIT.ON % 10 == 0){			// 「10」で割った余りが「0」のとき
					IN.FLAG.BIT.DOWN_SW = 1;				// 入力あり
				}else{
					IN.FLAG.BIT.DOWN_SW = 0;				// 入力なし
				}
			}else if(IN.DOWN_SW.BIT.ON >= 200){				// 2000ms連続
				if(IN.DOWN_SW.BIT.ON % 20 == 0){			// 「20」で割った余りが「0」のとき
					IN.FLAG.BIT.DOWN_SW = 1;				// 入力あり
				}else{
					IN.FLAG.BIT.DOWN_SW = 0;				// 入力なし
				}
			}else if(IN.DOWN_SW.BIT.ON >= 100){				// 1000ms連続
				if(IN.DOWN_SW.BIT.ON % 50 == 0){			// 「50」で割った余りが「0」のとき
					IN.FLAG.BIT.DOWN_SW = 1;				// 入力あり
				}else{
					IN.FLAG.BIT.DOWN_SW = 0;				// 入力なし
				}
			}else if(IN.DOWN_SW.BIT.ON == 50){				// 500ms連続
				IN.FLAG.BIT.DOWN_SW = 1;					// 入力あり
			}else if(IN.DOWN_SW.BIT.ON == 10){				// 100ms連続
				IN.FLAG.BIT.DOWN_SW = 1;					// 入力あり
			}else{
				IN.FLAG.BIT.DOWN_SW = 0;					// 入力なし
			}
			
			if(IN.FLAG.BIT.DOWN_SW == 1){								// 入力ありのとき
				if(SEQ.FLAG.BIT.MEASUREMENT == 0){
					if(SEQ.SELECT.BIT.MEASURE == MODE_RUNOUT){			// 2:振れのとき
						if(SEQ.FLUTES != 0){
							if(SEQ.SPINDLE_SPEED > SPINDLE_SPEED_MIN){
								SEQ.SPINDLE_SPEED--;
							}else{
								SEQ.SPINDLE_SPEED = SPINDLE_SPEED_MAX;
							}
						}
						
					}else if(SEQ.SELECT.BIT.MEASURE == MODE_FOCUS){		// 3:焦点のとき
						if(SEQ.START_DELAY_TIME > 0){
							SEQ.START_DELAY_TIME--;
						}else{
							SEQ.START_DELAY_TIME = 999;
						}
					}
					led_measure_set();									// LED 測定・設定ｾｯﾄ
				}
				IN.FLAG.BIT.DOWN_SW = 0;								// 入力なし
			}
		}
	}else{
		IN.DOWN_SW.BIT.ON = 0;
		if(IN.DOWN_SW.BIT.OFF < 10){									// 100ms連続
			IN.DOWN_SW.BIT.OFF++;
			if(IN.DOWN_SW.BIT.OFF == 10){								// 100ms連続
				IN.FLAG.BIT.DOWN_SW = 0;								// 入力なし
			}
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	
	// ECOｽｲｯﾁ(S9)		////////////////////////////////////////////////////////////
	if(ECO_SW_IN){
		IN.ECO_SW.BIT.OFF = 0;
		if(SEQ.POWER_STATUS == 1){
			if(IN.ECO_SW.BIT.ON < 100){								// 1s連続
				IN.ECO_SW.BIT.ON++;
				if(IN.ECO_SW.BIT.ON == 100){						// 1s連続
					IN.FLAG.BIT.ECO_SW = 1;							// 入力あり
					if(SEQ.FLAG.BIT.POWER){
						if(SEQ.FLAG.BIT.MEASUREMENT == 0){
							SEQ.FLAG.BIT.ECO = !SEQ.FLAG.BIT.ECO;	// ECO(状態反転)
							LED.MSP.BIT.ECO = SEQ.FLAG.BIT.ECO;
							SEQ.FLAG.BIT.POWER_ON = 1;				// 電源ONﾌﾗｸﾞをｾｯﾄ
						}
					}
				}
			}
		}
	}else{
		IN.ECO_SW.BIT.ON = 0;
		if(IN.ECO_SW.BIT.OFF < 100){								// 1s連続
			IN.ECO_SW.BIT.OFF++;
			if(IN.ECO_SW.BIT.OFF == 100){							// 1s連続
				IN.FLAG.BIT.ECO_SW = 0;								// 入力なし
			}
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	
	// ｶﾊﾞｰｽｲｯﾁ開		////////////////////////////////////////////////////////////
	if(!(COVER_OPEN_IN)){
		IN.COVER_OPEN.BIT.OFF = 0;
		if(IN.COVER_OPEN.BIT.ON < 10){					// 100ms連続
			IN.COVER_OPEN.BIT.ON++;
			if(IN.COVER_OPEN.BIT.ON == 10){				// 100ms連続
				IN.FLAG.BIT.COVER_OPEN = 1;				// 入力あり
				COM0.NO310.BIT.CSO = 1;					// 近接ｾﾝｻ1(ｶﾊﾞｰOPEN)
// add 2016.07.26 K.Uemura start	G72601
				SEQ.MSEC_BUFFER[0][3] = SEQ.MSEC_COUNTER;
// add 2016.07.26 K.Uemura end
			}
		}
	}else{
		IN.COVER_OPEN.BIT.ON = 0;
		if(IN.COVER_OPEN.BIT.OFF < 10){					// 100ms連続
			IN.COVER_OPEN.BIT.OFF++;
			if(IN.COVER_OPEN.BIT.OFF == 10){			// 100ms連続
				IN.FLAG.BIT.COVER_OPEN = 0;				// 入力なし
				COM0.NO310.BIT.CSO = 0;					// 近接ｾﾝｻ1(ｶﾊﾞｰOPEN)
			}
		}
	}
	////////////////////////////////////////////////////////////////////////////////
	
	// ｶﾊﾞｰｽｲｯﾁ閉		////////////////////////////////////////////////////////////
	if(!(COVER_CLOSE_IN)){
		IN.COVER_CLOSE.BIT.OFF = 0;
		if(IN.COVER_CLOSE.BIT.ON < 10){					// 100ms連続
			IN.COVER_CLOSE.BIT.ON++;
			if(IN.COVER_CLOSE.BIT.ON == 10){			// 100ms連続
				IN.FLAG.BIT.COVER_CLOSE = 1;			// 入力あり
				COM0.NO310.BIT.CSC = 1;					// 近接ｾﾝｻ2(ｶﾊﾞｰCLOSE)
//// add 2016.07.26 K.Uemura start	G72601
//				SEQ.MSEC_BUFFER[0][3] = SEQ.MSEC_COUNTER;
//// add 2016.07.26 K.Uemura end
			}
		}
	}else{
		IN.COVER_CLOSE.BIT.ON = 0;
		if(IN.COVER_CLOSE.BIT.OFF < 10){				// 100ms連続
			IN.COVER_CLOSE.BIT.OFF++;
			if(IN.COVER_CLOSE.BIT.OFF == 10){			// 100ms連続
				IN.FLAG.BIT.COVER_CLOSE = 0;			// 入力なし
				COM0.NO310.BIT.CSC = 0;					// 近接ｾﾝｻ2(ｶﾊﾞｰCLOSE)
			}
		}
	}
	////////////////////////////////////////////////////////////////////////////////
}

//************************************************************/
//				LED ﾋﾟｰｸﾎｰﾙﾄﾞｾｯﾄ
//************************************************************/
void led_peakhold_set(void)
{
	LED.MSP.BIT.OFF = 0;
	LED.MSP.BIT.ON = 0;
	LED.MSP.BIT.S5 = 0;
	LED.MSP.BIT.S10 = 0;
	
	if((SEQ.SELECT.BIT.MEASURE == MODE_D4_LOW)||(SEQ.SELECT.BIT.MEASURE == MODE_D4_AUTO)||(SEQ.SELECT.BIT.MEASURE == MODE_RUNOUT)){
		switch(SEQ.SELECT.BIT.PEAKHOLD){
			case PEAK_HOLD_OFF:		LED.MSP.BIT.OFF = 1;	break;	// 0:Off
			case PEAK_HOLD_ON:		LED.MSP.BIT.ON = 1;		break;	// 1:On
			case PEAK_HOLD_5S:		LED.MSP.BIT.S5 = 1;		break;	// 2:5s
			case PEAK_HOLD_10S:		LED.MSP.BIT.S10 = 1;	break;	// 3:10s
		}
	}
}

//************************************************************/
//				LED 測定・設定ｾｯﾄ
//************************************************************/
void led_measure_set(void)
{
	// ﾎﾟｰﾀﾌﾞﾙ版に限り実行
	if(IN.FLAG.BIT.HARDWARE_TYPE != PORTABLE_EDITION){
		return;
	}

	LED.MSP.BIT.LESS	= 0;
	LED.MSP.BIT.GREATER	= 0;
	LED.MSP.BIT.RUN_OUT	= 0;
	LED.MSP.BIT.FOCUS	= 0;
	LED.MSP.BIT.CENTER	= 0;
	
	LED.SEG_BUF[11]	= ' ';
	LED.SEG_BUF[12]	= ' ';
	LED.SEG_BUF[13]	= ' ';
	LED.SEG_BUF[14]	= ' ';
	
	switch(SEQ.SELECT.BIT.MEASURE){
		case MODE_D4_LOW:						// d≦4
			LED.MSP.BIT.LESS = 1;
			// ﾊﾟﾗﾒｰﾀ値を一時ﾃﾞｰﾀにｺﾋﾟｰする
			disp_flutes();						// 刃数表示
			break;
			
		case MODE_D4_AUTO:						// d＞4 自動
			LED.MSP.BIT.GREATER = 1;
			break;
			
		case MODE_RUNOUT:						// 振れ
			LED.MSP.BIT.RUN_OUT = 1;
			// ﾊﾟﾗﾒｰﾀ値を一時ﾃﾞｰﾀにｺﾋﾟｰする
			disp_flutes();						// 刃数表示

			if(SEQ.FLUTES == 0){
				LED.SEG_BUF[11]	= '-';
				LED.SEG_BUF[12]	= '-';
				LED.SEG_BUF[13]	= '-';
			}else{
				disp_spindle_speed();			// 回転数表示
			}
			break;
			
		case MODE_FOCUS:						// 焦点合わせ
			LED.MSP.BIT.FOCUS = 1;
			disp_stay_delay_time();				// 計測開始遅延時間表示
			break;
			
		case MODE_CENTER:						// 中心位置設定
			LED.MSP.BIT.CENTER = 1;
			break;
			
	}
}

//************************************************************/
//				回転数表示
//************************************************************/
void disp_spindle_speed(void)
{
	LED.SEG_BUF[11]	= (((SEQ.SPINDLE_SPEED / 100) % 10) + 0x30);
	LED.SEG_BUF[12]	= (((SEQ.SPINDLE_SPEED / 10) % 10) + 0x30);
	LED.SEG_BUF[13]	= ((SEQ.SPINDLE_SPEED % 10) + 0x30);
}

//************************************************************/
//				刃数表示
//************************************************************/
void disp_flutes(void)
{
	LED.SEG_BUF[14]	= SEQ.FLUTES + 0x30;
}

//************************************************************/
//				計測開始遅延時間表示
//************************************************************/
void disp_stay_delay_time(void)
{
	// 計測していないときのみ表示する
	if(SEQ.FLAG.BIT.MEASUREMENT == 0){
		LED.SEG_BUF[11]	= (((SEQ.START_DELAY_TIME / 100) % 10) + 0x30);
		LED.SEG_BUF[12]	= (((SEQ.START_DELAY_TIME / 10) % 10) + 0x30);
		LED.SEG_BUF[13]	= ((SEQ.START_DELAY_TIME % 10) + 0x30);
	}else{
		LED.SEG_BUF[11]	= ' ';
		LED.SEG_BUF[12]	= ' ';
		LED.SEG_BUF[13]	= ' ';
	}
}
