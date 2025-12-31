/******************************************************************************
* File Name	: ctl_interface_func.c
******************************************************************************/
#include <machine.h>
#include "iodefine.h"
#include "typedefine.h"
#include "usercopy.h"
#include "user_define.h"

//************************************************************/
//				内部関数プロトタイプ宣言					
//************************************************************/
void disp_battery_power(void);
void disp_software_version(void);			// ｿﾌﾄｳｪｱﾊﾞｰｼﾞｮﾝ表示

void disp_data_reset(void);					// LED表示ﾃﾞｰﾀﾘｾｯﾄ

void disp0_data_set(void);					// DISP0ﾃﾞｰﾀｾｯﾄ
void disp1_data_set(void);					// DISP1ﾃﾞｰﾀｾｯﾄ
void disp2_data_set(void);					// DISP2ﾃﾞｰﾀｾｯﾄ
void disp3_data_set(void);					// DISP3ﾃﾞｰﾀｾｯﾄ
void disp4_data_set(void);					// DISP4ﾃﾞｰﾀｾｯﾄ
void disp5_data_set(void);					// DISP5ﾃﾞｰﾀｾｯﾄ
void disp6_data_set(void);					// DISP6ﾃﾞｰﾀｾｯﾄ
void disp7_data_set(void);					// DISP7ﾃﾞｰﾀｾｯﾄ

void clk1_pulse(void);						// CLK1ﾊﾟﾙｽ
void clk2_pulse(void);						// CLK2ﾊﾟﾙｽ
void clk3_pulse(void);						// CLK3ﾊﾟﾙｽ
void clk4_pulse(void);						// CLK4ﾊﾟﾙｽ

void led_output(void);						// LED出力

void drv_disp_data_out(_UBYTE w_data);		// 表示ﾃﾞｰﾀ出力関数

_UBYTE drv_seg_change(_UBYTE seg);			// ｾｸﾞﾒﾝﾄﾃﾞｰﾀ変換関数

//void analog_value_verification(void);		// ｱﾅﾛｸﾞ値検証(ﾃﾞﾊﾞｯｸﾞ用)

void out_master(void);						// 出力ｼｰｹﾝｽ関数(O.1msごとに処理)

void led_on_state(void);					// LED ON状態
void led_off_state(void);					// LED OFF状態

//************************************************************/
//				ｱﾅﾛｸﾞ値検証(ﾃﾞﾊﾞｯｸﾞ用)
//************************************************************/
/*
void analog_value_verification(void)
{
	// ｱﾅﾛｸﾞ値検証
	if(ADCOV.V_BATT >= 506){
		SEQ.BATTERY_POWER = 100;								// 電池残量(%)
	}else if(ADCOV.V_BATT <= 352){
		SEQ.BATTERY_POWER = 0;									// 電池残量(%)
	}else{
		SEQ.BATTERY_POWER = (ADCOV.V_BATT-352)*100 / 154;		// 電池残量(%)
	}
					
	// 百の位
	if(ADCOV.V_BATT < 100)		LED.SEG_BUF[11] = ' ';
	else						LED.SEG_BUF[11] = (((ADCOV.V_BATT / 100) % 10) + 0x30);
	// 十の位
	if(ADCOV.V_BATT < 10)		LED.SEG_BUF[12] = ' ';
	else						LED.SEG_BUF[12] = (((ADCOV.V_BATT / 10) % 10) + 0x30);
	// 一の位
	LED.SEG_BUF[13]	= ((ADCOV.V_BATT % 10) + 0x30);
	//
	
	// 百の位
	if(SEQ.BATTERY_POWER < 100)		LED.SEG_BUF[11] = ' ';
	else							LED.SEG_BUF[11] = (((SEQ.BATTERY_POWER / 100) % 10) + 0x30);
	// 十の位
	if(SEQ.BATTERY_POWER < 10)		LED.SEG_BUF[12] = ' ';
	else							LED.SEG_BUF[12] = (((SEQ.BATTERY_POWER / 10) % 10) + 0x30);
	// 一の位
	LED.SEG_BUF[13]	= ((SEQ.BATTERY_POWER % 10) + 0x30);
	//
}
*/

//************************************************************/
//				出力ｼｰｹﾝｽ関数(O.1msごとに処理)
//************************************************************/
void out_master(void)
{
	//analog_value_verification();						// ｱﾅﾛｸﾞ値検証(ﾃﾞﾊﾞｯｸﾞ用)
	
	switch(OUT.MASTER_STATUS){
		case INIT_START_MODE:							// 初期ﾓｰﾄﾞ
			break;
			
		case DRV_MODE:									// 通常ﾓｰﾄﾞ
			led_on_state();								// LED ON状態
			if(SEQ.FLAG5.BIT.LOW_BATTERY == 1){			// POWER LEDの点滅
				// 電圧エラーが発生すると点滅させない
				if(SEQ.FLAG5.BIT.VOLTAGE_ERROR == 0){
					if(SEQ.FLAG5.BIT.LOW_BATTERY_FLAG == 1)		LED.MSP.BIT.POWER = 0;
					else										LED.MSP.BIT.POWER = 1;
				}
			}
			led_output();
			break;
			
		case IDLE_MODE:									// 待機ﾓｰﾄﾞ
			led_off_state();							// LED OFF状態
			if(SEQ.FLAG5.BIT.LOW_BATTERY == 1){			// POWER LEDの点滅
				// 電圧エラーが発生すると点滅させない
				if(SEQ.FLAG5.BIT.VOLTAGE_ERROR == 0){
					if(SEQ.FLAG5.BIT.LOW_BATTERY_FLAG == 1)		LED.MSP.BIT.POWER = 0;
					else										LED.MSP.BIT.POWER = 1;
				}
			}else{
				LED.MSP.BIT.POWER = 0;
			}
			led_output();
			break;
			
		default:
			OUT.MASTER_STATUS = IDLE_MODE;	// 待機ﾓｰﾄﾞ
	}
}

//************************************************************/
//				LED ON状態
//	1 → 2 → 3 → 4
//
//	「1」スクロール
//	11 → 99
//	   → 12 → 13 → 99
//	               → 14 → 15 → 99
//	                           → 16 → 17 → 99
//	                                       → 18 → 19 → 99
//	                                                   → 20 → 11
//
//	31 → 99
//	   → 32 → 33 → 99
//	               → 34 → 35 → 99
//	                           → 36 → 37 → 99
//	                                       → 38 → 39 → 99
//	                                                   → 40 → 41 → 99
//	                                                               → 42 → 31
//************************************************************/
void led_on_state(void)
{
	_UBYTE i;

	switch(OUT.SUB_STATUS){
		case 1:
			// 電源ON起動時
			// 設定値の読み込み
			LED.FOCUSING = 0x07FF;		// X軸LED
			LED.Z_FOCUSING = 0x0F;		// Z軸LED
			LED.FOCUS.BYTE = 0x07;		// 焦点L・R・Z
			LED.MSP.WORD = 0x0FFF;		// Measure・Setting・Peak Hold・POWER・ECO・予備
			
			for(i=1;i<=14;i++){
				LED.SEG_BUF[i] = '8';
			}
			
			ctl_uni_timer1(100);		// 1s
			OUT.SUB_STATUS++;
			break;
			
		case 2:
			// 1s待機後次へ
			if(TIM1.MSEC_10.BIT.UNI_UP1){
				TIM1.MSEC_10.BIT.UNI_UP1 = 0;
				LED.MSP.WORD = 0x0000;		// Measure・Setting・Peak Hold・POWER・ECO・予備
				LED.MSP.BIT.POWER = 1;
				LED.MSP.BIT.ECO = SEQ.FLAG.BIT.ECO;
				
// chg 2015.09.02 K.Uemura start	
				if(IN.FLAG.BIT.HARDWARE_TYPE == PORTABLE_EDITION){			// ﾎﾟｰﾀﾌﾞﾙ版のとき
					COM0.NO311 = 100;		// 焦点合わせ(X)
				}else{
					COM0.NO311 = 0;			// なし
				}
//				COM0.NO311 = 100;			// 焦点合わせ(X)
// chg 2015.09.02 K.Uemura end
				SEQ.SELECT.BIT.MEASURE = MODE_FOCUS;
				SEQ.FLUTES = PARAM_FLUTES;			// 刃数

				for(i=11;i<=14;i++){
					LED.SEG_BUF[i] = ' ';
				}

				led_measure_set();						// LED 測定・設定ｾｯﾄ
				
				SEQ.SELECT.BIT.PEAKHOLD = PEAK_HOLD_5S;
				led_peakhold_set();						// LED ﾋﾟｰｸﾎｰﾙﾄﾞｾｯﾄ
				
				switch(SEQ.SELECT.BIT.MEASURE){
					case MODE_D4_LOW:
					case MODE_RUNOUT:
						disp_flutes();						// 刃数表示
						break;

					default:
						LED.SEG_BUF[14]	= ' ';
						break;
				}
				
				OUT.SUB_STATUS++;

#ifdef	DEBUG_AUTOSTART
				// sw.c S2：START_SW_IN
				// 計測開始
				SEQ.FLAG.BIT.MEASUREMENT = 1;
				
				SEQ.CBUS_NUMBER = 394;
				SEQ.CHANGE_FPGA = 2;			// 測定
				SEQ.FPGA_SEND_STATUS = 1;
				SEQ.FLAG.BIT.PORTABLE = 1;				// ﾎﾟｰﾀﾌﾞﾙ操作ﾌﾗｸﾞ
#endif
			}
			break;
			
		case 3:
			// 電源ON起動後待機中
			// 設定値の読み込み
			LED.FOCUSING = 0x0000;		// X軸LED
			LED.Z_FOCUSING = 0x00;		// Z軸LED
			LED.FOCUS.BYTE = 0x00;		// 焦点L・R・Z
			
			if(COM0.NO312 == 0)		set_7seg_upper_no_data();				// 7ｾｸﾞ表示(-----)(上段)
			else					set_7seg_led_error_no(COM0.NO312);		// 7セグLEDのｴﾗｰﾅﾝﾊﾞｰ設定
			
			set_7seg_lower_no_data();				// 7ｾｸﾞ表示(-----)(下段)
			
			OUT.SUB_STATUS++;
			
#if	EXHIBITION
			// POWERｼｰｹﾝｽ完了後工具径(4以下)の計測を行う 141021
			// for exhibitions 141021
			COM0.NO301 = 10;							// 工具径(4以下)
			COM0.NO300.BIT.EXE = 1;						// 司令(実行)がONのとき
			COM0.NO300.BIT.ROF = 1;						// READY OFF検知
			//
#endif
			
			break;
			
		case 4:
			break;
			
		// START実行中
		// ﾁｭｰﾆﾝｸﾞ
		case 11:
			if(SEQ.FLAG.BIT.MEASUREMENT == MEASURE_STOP){
				OUT.SUB_STATUS = 99;
			}
			//LED.FOCUSING = 0x0000;		// FOCUS全消灯
			// ﾁｭｰﾆﾝｸﾞ1
			LED.SEG_BUF[1]	= ' ';
			LED.SEG_BUF[2]	= ' ';
			LED.SEG_BUF[3]	= ' ';
			LED.SEG_BUF[4]	= ' ';
			LED.SEG_BUF[6]	= ' ';
			LED.SEG_BUF[7]	= ' ';
			LED.SEG_BUF[8]	= ' ';
			LED.SEG_BUF[9]	= ' ';
			
			LED.SEG_BUF[5]	= '1';
			LED.SEG_BUF[10]	= '1';
			
			ctl_uni_timer1(20);	// 0.2s
			OUT.SUB_STATUS++;
			break;
			
		case 12:
			// 0.2s待機後次へ
			if(TIM1.MSEC_10.BIT.UNI_UP1){
				TIM1.MSEC_10.BIT.UNI_UP1 = 0;
				OUT.SUB_STATUS++;
			}
			break;
			
		case 13:
			if(SEQ.FLAG.BIT.MEASUREMENT == MEASURE_STOP){
				OUT.SUB_STATUS = 99;
			}
			// ﾁｭｰﾆﾝｸﾞ2
			LED.SEG_BUF[4]	= '1';
			LED.SEG_BUF[9]	= '1';
			
			ctl_uni_timer1(20);	// 0.2s
			OUT.SUB_STATUS++;
			break;
			
		case 14:
			// 0.2s待機後次へ
			if(TIM1.MSEC_10.BIT.UNI_UP1){
				TIM1.MSEC_10.BIT.UNI_UP1 = 0;
				OUT.SUB_STATUS++;
			}
			break;
			
		case 15:
			if(SEQ.FLAG.BIT.MEASUREMENT == MEASURE_STOP){
				OUT.SUB_STATUS = 99;
			}
			// ﾁｭｰﾆﾝｸﾞ3
			LED.SEG_BUF[3]	= '1';
			LED.SEG_BUF[8]	= '1';
			
			ctl_uni_timer1(20);	// 0.2s
			OUT.SUB_STATUS++;
			break;
			
		case 16:
			// 0.2s待機後次へ
			if(TIM1.MSEC_10.BIT.UNI_UP1){
				TIM1.MSEC_10.BIT.UNI_UP1 = 0;
				OUT.SUB_STATUS++;
			}
			break;
			
		case 17:
			if(SEQ.FLAG.BIT.MEASUREMENT == MEASURE_STOP){
				OUT.SUB_STATUS = 99;
			}
			// ﾁｭｰﾆﾝｸﾞ4
			LED.SEG_BUF[2]	= '1';
			LED.SEG_BUF[7]	= '1';
			
			ctl_uni_timer1(20);	// 0.2s
			OUT.SUB_STATUS++;
			break;
			
		case 18:
			// 0.2s待機後次へ
			if(TIM1.MSEC_10.BIT.UNI_UP1){
				TIM1.MSEC_10.BIT.UNI_UP1 = 0;
				OUT.SUB_STATUS++;
			}
			break;
			
		case 19:
			if(SEQ.FLAG.BIT.MEASUREMENT == MEASURE_STOP){
				OUT.SUB_STATUS = 99;
			}
			// ﾁｭｰﾆﾝｸﾞ5
			LED.SEG_BUF[1]	= '1';
			LED.SEG_BUF[6]	= '1';
			
			ctl_uni_timer1(20);	// 0.2s
			OUT.SUB_STATUS++;
			break;
			
		case 20:
			// 0.2s待機後次へ
			if(TIM1.MSEC_10.BIT.UNI_UP1){
				TIM1.MSEC_10.BIT.UNI_UP1 = 0;
				OUT.SUB_STATUS = 11;
			}
			break;

#if	1
		// START実行中
		// ﾁｭｰﾆﾝｸﾞ
		case 31:
			if(SEQ.FLAG.BIT.MEASUREMENT == MEASURE_STOP){
				OUT.SUB_STATUS = 99;
			}
			//LED.FOCUSING = 0x0000;		// FOCUS全消灯
			// ﾁｭｰﾆﾝｸﾞ1
			LED.SEG_BUF[1]	= ' ';
			LED.SEG_BUF[2]	= ' ';
			LED.SEG_BUF[3]	= ' ';
			LED.SEG_BUF[4]	= ' ';
			LED.SEG_BUF[6]	= ' ';
			LED.SEG_BUF[7]	= ' ';
			LED.SEG_BUF[8]	= ' ';
			LED.SEG_BUF[9]	= ' ';
			LED.SEG_BUF[10]	= ' ';
			
			LED.SEG_BUF[5]	= OUT_SEG_NO1;
			
			ctl_uni_timer1(20);	// 0.2s
			OUT.SUB_STATUS++;
			break;
			
		case 32:
			// 0.2s待機後次へ
			if(TIM1.MSEC_10.BIT.UNI_UP1){
				TIM1.MSEC_10.BIT.UNI_UP1 = 0;
				OUT.SUB_STATUS++;
			}
			break;
			
		case 33:
			if(SEQ.FLAG.BIT.MEASUREMENT == MEASURE_STOP){
				OUT.SUB_STATUS = 99;
			}
			// ﾁｭｰﾆﾝｸﾞ2
			LED.SEG_BUF[5]	= OUT_SEG_NO2;
			
			ctl_uni_timer1(20);	// 0.2s
			OUT.SUB_STATUS++;
			break;
			
		case 34:
			// 0.2s待機後次へ
			if(TIM1.MSEC_10.BIT.UNI_UP1){
				TIM1.MSEC_10.BIT.UNI_UP1 = 0;
				OUT.SUB_STATUS++;
			}
			break;
			
		case 35:
			if(SEQ.FLAG.BIT.MEASUREMENT == MEASURE_STOP){
				OUT.SUB_STATUS = 99;
			}
			// ﾁｭｰﾆﾝｸﾞ3
			LED.SEG_BUF[5]	= OUT_SEG_NO3;
			
			ctl_uni_timer1(20);	// 0.2s
			OUT.SUB_STATUS++;
			break;
			
		case 36:
			// 0.2s待機後次へ
			if(TIM1.MSEC_10.BIT.UNI_UP1){
				TIM1.MSEC_10.BIT.UNI_UP1 = 0;
				OUT.SUB_STATUS++;
			}
			break;
			
		case 37:
			if(SEQ.FLAG.BIT.MEASUREMENT == MEASURE_STOP){
				OUT.SUB_STATUS = 99;
			}
			// ﾁｭｰﾆﾝｸﾞ4
			LED.SEG_BUF[10]	= OUT_SEG_NO1;
			
			ctl_uni_timer1(20);	// 0.2s
			OUT.SUB_STATUS++;
			break;
			
		case 38:
			// 0.2s待機後次へ
			if(TIM1.MSEC_10.BIT.UNI_UP1){
				TIM1.MSEC_10.BIT.UNI_UP1 = 0;
				OUT.SUB_STATUS++;
			}
			break;
			
		case 39:
			if(SEQ.FLAG.BIT.MEASUREMENT == MEASURE_STOP){
				OUT.SUB_STATUS = 99;
			}
			// ﾁｭｰﾆﾝｸﾞ5
			LED.SEG_BUF[10]	= OUT_SEG_NO2;
			
			ctl_uni_timer1(20);	// 0.2s
			OUT.SUB_STATUS++;
			break;
			
		case 40:
			// 0.2s待機後次へ
			if(TIM1.MSEC_10.BIT.UNI_UP1){
				TIM1.MSEC_10.BIT.UNI_UP1 = 0;
				OUT.SUB_STATUS++;
			}
			break;
			
		case 41:
			if(SEQ.FLAG.BIT.MEASUREMENT == MEASURE_STOP){
				OUT.SUB_STATUS = 99;
			}
			// ﾁｭｰﾆﾝｸﾞ5
			LED.SEG_BUF[10]	= OUT_SEG_NO3;
			
			ctl_uni_timer1(20);	// 0.2s
			OUT.SUB_STATUS++;
			break;
			
		case 42:
			// 0.2s待機後次へ
			if(TIM1.MSEC_10.BIT.UNI_UP1){
				TIM1.MSEC_10.BIT.UNI_UP1 = 0;
				OUT.SUB_STATUS = 31;
			}
			break;
#endif

		case 99:
			break;
	}
}
			
//************************************************************/
//				LED OFF状態
//************************************************************/
// 全てのLED表示をOFFにする
void led_off_state(void)
{
	// 設定値の読み込み
	LED.FOCUSING = 0x0000;		// X軸LED
	LED.Z_FOCUSING = 0x00;		// Z軸LED
	LED.FOCUS.BYTE = 0x00;		// 焦点L・R・Z
	LED.MSP.WORD = 0x0000;		// Measure・Setting・Peak Hold・POWER・ECO・予備
	
	LED.SEG_BUF[1]	= ' ';
	LED.SEG_BUF[2]	= ' ';
	LED.SEG_BUF[3]	= ' ';
	LED.SEG_BUF[4]	= ' ';
	LED.SEG_BUF[5]	= ' ';
	LED.SEG_BUF[6]	= ' ';
	LED.SEG_BUF[7]	= ' ';
	LED.SEG_BUF[8]	= ' ';
	LED.SEG_BUF[9]	= ' ';
	LED.SEG_BUF[10]	= ' ';
	LED.SEG_BUF[11]	= ' ';
	LED.SEG_BUF[12]	= ' ';
	LED.SEG_BUF[13]	= ' ';
	LED.SEG_BUF[14]	= ' ';
	
	// 電池残量表示
	disp_battery_power();

	// 電圧エラー表示
	disp_voltage_error();
	
// chg 2015.03.18 K.Uemura start	
	// ｿﾌﾄｳｪｱﾊﾞｰｼﾞｮﾝ表示
	disp_software_version();
//	// ﾓｰﾄﾞｽｲｯﾁ(S6)を押しているときｿﾌﾄｳｪｱﾊﾞｰｼﾞｮﾝ表示
//	if(IN.FLAG.BIT.MODE_SW == 1)	disp_software_version();
// chg 2015.03.18 K.Uemura end
}

//************************************************************/
//				バッテリー残量表示
//************************************************************/
void disp_battery_power(void)
{
// add 2015.03.23 K.Uemura start	F32301
	// バッテリ動作時
	if(IN.FLAG.BIT.EXT_POWER == POWER_SUPPLY_BATTERY){
// add 2015.03.23 K.Uemura end
		// 上下ｽｲｯﾁを同時押ししているとき
		if((IN.UP_SW.BIT.ON >= 10)&&(IN.DOWN_SW.BIT.ON >= 10)){
			// 百の位
			if(SEQ.BATTERY_POWER < 100)		LED.SEG_BUF[11] = ' ';
			else							LED.SEG_BUF[11] = (((SEQ.BATTERY_POWER / 100) % 10) + 0x30);
			// 十の位
			if(SEQ.BATTERY_POWER < 10)		LED.SEG_BUF[12] = ' ';
			else							LED.SEG_BUF[12] = (((SEQ.BATTERY_POWER / 10) % 10) + 0x30);
			// 一の位
			LED.SEG_BUF[13]	= ((SEQ.BATTERY_POWER % 10) + 0x30);
		}
	}
}

//************************************************************/
//				ｿﾌﾄｳｪｱﾊﾞｰｼﾞｮﾝ表示
//************************************************************/
void disp_software_version(void)
{
	_UDWORD temp;
	
// chg 2015.05.13 K.Uemura start	
	_UBYTE disp_flag = 0;

	if(IN.FLAG.BIT.HARDWARE_TYPE == PORTABLE_EDITION){
// add 2015.03.18 K.Uemura start	
		// 上下ｽｲｯﾁを同時押ししているとき
		if((IN.UP_SW.BIT.ON >= 10)&&(IN.DOWN_SW.BIT.ON >= 10)){
			disp_flag = 1;
		}
// add 2015.03.18 K.Uemura end
	}else{
		if(SEQ.FLAG5.BIT.DISP_SOFTVERSION == 1){
			disp_flag = 1;
		}
	}

	if(disp_flag == 1){
// chg 2015.05.13 K.Uemura end
		// FPGAｿﾌﾄｳｪｱﾊﾞｰｼﾞｮﾝ
		temp = SEQ.FPGA_RIVISION;
		//LED.SEG_BUF[1] = 'F';
		if(temp < 10000)	LED.SEG_BUF[1] = ' ';
		else				LED.SEG_BUF[1] = ((temp / 10000) % 10 + 0x30);	// 百の位
		if(temp < 1000)		LED.SEG_BUF[2] = ' ';
		else				LED.SEG_BUF[2] = ((temp / 1000) % 10 + 0x30);	// 十の位
							LED.SEG_BUF[3] = ((temp / 100) % 10 + 0x30);	// 一の位
							LED.SEG_BUF[3] |= 0x80;							// 小数点
							LED.SEG_BUF[4] = ((temp / 10) % 10 + 0x30);		// 小数第一位
							LED.SEG_BUF[5] = (temp % 10 + 0x30);			// 小数第二位
										
		// RXｿﾌﾄｳｪｱﾊﾞｰｼﾞｮﾝ
		temp = RX_RIVISION;
		//LED.SEG_BUF[6] = 'r';
		if(temp < 10000)	LED.SEG_BUF[6] = ' ';
		else				LED.SEG_BUF[6] = ((temp / 10000) % 10 + 0x30);	// 百の位
		if(temp < 1000)		LED.SEG_BUF[7] = ' ';
		else				LED.SEG_BUF[7] = ((temp / 1000) % 10 + 0x30);	// 十の位
							LED.SEG_BUF[8] = ((temp / 100) % 10 + 0x30);	// 一の位
							LED.SEG_BUF[8] |= 0x80;							// 小数点
							LED.SEG_BUF[9] = ((temp / 10) % 10 + 0x30);		// 小数第一位
							LED.SEG_BUF[10] = (temp % 10 + 0x30);			// 小数第二位
	}
}

//************************************************************/
//				LED表示ﾃﾞｰﾀﾘｾｯﾄ
//************************************************************/
// LED表示を全てOFFにするためのﾃﾞｰﾀをｾｯﾄする
void disp_data_reset(void)
{
	/*
	DISP0_OUT = 0;
	DISP1_OUT = 0;
	DISP2_OUT = 0;
	DISP3_OUT = 0;
	DISP4_OUT = 0;
	DISP5_OUT = 0;
	DISP6_OUT = 0;
	DISP7_OUT = 0;
	*/
	
	PORT7.DR.BYTE = 0;
}

//************************************************************/
//				DISP0ﾃﾞｰﾀｾｯﾄ
//************************************************************/
void disp0_data_set(void)
{
	disp_data_reset();
	DISP0_OUT = 1;
}

//************************************************************/
//				DISP1ﾃﾞｰﾀｾｯﾄ
//************************************************************/
void disp1_data_set(void)
{
	disp_data_reset();
	DISP1_OUT = 1;
}

//************************************************************/
//				DISP2ﾃﾞｰﾀｾｯﾄ
//************************************************************/
void disp2_data_set(void)
{
	disp_data_reset();
	DISP2_OUT = 1;
}

//************************************************************/
//				DISP3ﾃﾞｰﾀｾｯﾄ
//************************************************************/
void disp3_data_set(void)
{
	disp_data_reset();
	DISP3_OUT = 1;
}

//************************************************************/
//				DISP4ﾃﾞｰﾀｾｯﾄ
//************************************************************/
void disp4_data_set(void)
{
	disp_data_reset();
	DISP4_OUT = 1;
}

//************************************************************/
//				DISP5ﾃﾞｰﾀｾｯﾄ
//************************************************************/
void disp5_data_set(void)
{
	disp_data_reset();
	DISP5_OUT = 1;
}

//************************************************************/
//				DISP6ﾃﾞｰﾀｾｯﾄ
//************************************************************/
void disp6_data_set(void)
{
	disp_data_reset();
	DISP6_OUT = 1;
}

//************************************************************/
//				DISP7ﾃﾞｰﾀｾｯﾄ
//************************************************************/
void disp7_data_set(void)
{
	disp_data_reset();
	DISP7_OUT = 1;
}

//************************************************************/
//				CLK1ﾊﾟﾙｽ
//************************************************************/
void clk1_pulse(void)
{
	CLK1_OUT = 1;
	CLK1_OUT = 0;
}

//************************************************************/
//				CLK2ﾊﾟﾙｽ
//************************************************************/
void clk2_pulse(void)
{
	CLK2_OUT = 1;
	CLK2_OUT = 0;
}

//************************************************************/
//				CLK3ﾊﾟﾙｽ
//************************************************************/
void clk3_pulse(void)
{
	CLK3_OUT = 1;
	CLK3_OUT = 0;
}

//************************************************************/
//				CLK4ﾊﾟﾙｽ
//************************************************************/
void clk4_pulse(void)
{
	CLK4_OUT = 1;
	CLK4_OUT = 0;
}

//************************************************************/
//				LED出力
//************************************************************/
// LEDを表示するためのｼｰｹﾝｽ
void led_output(void)
{
	_UBYTE buf;
	const char on_count_7seg = 4;				// 7ｾｸﾞ用	0.4msON・0.1msOFF
	const char on_count_led = 29;				// LED用	2.9msON・0.1msOFF
	
	// 0.1msごとに制御
	// ﾒﾓﾘ操作時に7ｾｸﾞ・LEDがちらつくため表示を消す
	switch(OUT.LED_STATUS){
		// 全てのﾃﾞｰﾀをﾘｾｯﾄ
		// 7SEG
		case 2:
		case 4:
		case 6:
		case 8:
		case 10:
		case 12:
		case 14:
			LED.ON_COUNT++;						// ON時間を加算
			if(LED.ON_COUNT >= on_count_7seg){	// ON時間が設定値に達したら
				LED.ON_COUNT = 0;				// ON時間をﾘｾｯﾄ
				disp_data_reset();				// LED表示ﾃﾞｰﾀﾘｾｯﾄ
				clk3_pulse();					// CLK3ﾊﾟﾙｽ
				OUT.LED_STATUS++;
			}
			break;
			
		// 7SEG1
		case 1:
			// ﾒﾓﾘ操作時に7ｾｸﾞ・LEDがちらつくため表示を消す
			if(SEQ.FLAG.BIT.MEMORY_CONTROL == 1)	break;
		
			// TLC59210にﾃﾞｰﾀをｾｯﾄした後、TLC59213にﾃﾞｰﾀをｾｯﾄする
			// 「D」「R.O.」1
			DISP8_OUT = 1;						// CLRを解除
			
			// 「DBUS00-07」
			drv_disp_data_out(drv_seg_change(LED.SEG_BUF[1]));
			clk1_pulse();						// CLK1ﾊﾟﾙｽ
			
			// 「DBUS08-15」
			drv_disp_data_out(drv_seg_change(LED.SEG_BUF[6]));
			clk2_pulse();						// CLK2ﾊﾟﾙｽ
			
			// 「DEG10」
			disp0_data_set();					// DISP0ﾃﾞｰﾀｾｯﾄ
			clk3_pulse();						// CLK3ﾊﾟﾙｽ
			
			OUT.LED_STATUS++;
			break;
		
		// 7SEG2
		case 3:
			// 「D」「R.O.」2
			// 「DBUS00-07」
			drv_disp_data_out(drv_seg_change(LED.SEG_BUF[2]));
			clk1_pulse();						// CLK1ﾊﾟﾙｽ
			
			// 「DBUS08-15」
			drv_disp_data_out(drv_seg_change(LED.SEG_BUF[7]));
			clk2_pulse();						// CLK2ﾊﾟﾙｽ
			
			// 「DEG11」
			disp1_data_set();					// DISP1ﾃﾞｰﾀｾｯﾄ
			clk3_pulse();						// CLK3ﾊﾟﾙｽ
			
			OUT.LED_STATUS++;
			break;

		// 7SEG3
		case 5:
			// 「D」「R.O.」3
			// 「DBUS00-07」
			drv_disp_data_out(drv_seg_change(LED.SEG_BUF[3]));
			clk1_pulse();						// CLK1ﾊﾟﾙｽ
			
			// 「DBUS08-15」
			drv_disp_data_out(drv_seg_change(LED.SEG_BUF[8]));
			clk2_pulse();						// CLK2ﾊﾟﾙｽ
			
			// 「DEG12」
			disp2_data_set();					// DISP2ﾃﾞｰﾀｾｯﾄ
			clk3_pulse();						// CLK3ﾊﾟﾙｽ
			
			OUT.LED_STATUS++;
			break;

		// 7SEG4
		case 7:
			// 「D」「R.O.」4
			// 「DBUS00-07」
			drv_disp_data_out(drv_seg_change(LED.SEG_BUF[4]));
			clk1_pulse();						// CLK1ﾊﾟﾙｽ
			
			// 「DBUS08-15」
			drv_disp_data_out(drv_seg_change(LED.SEG_BUF[9]));
			clk2_pulse();						// CLK2ﾊﾟﾙｽ
			
			// 「DEG13」
			disp3_data_set();					// DISP3ﾃﾞｰﾀｾｯﾄ
			clk3_pulse();						// CLK3ﾊﾟﾙｽ
			
			OUT.LED_STATUS++;
			break;

		// 7SEG5
		case 9:
			// 「D」「R.O.」5
			// 「DBUS00-07」
			drv_disp_data_out(drv_seg_change(LED.SEG_BUF[5]));
			clk1_pulse();						// CLK1ﾊﾟﾙｽ
			
			// 「DBUS08-15」
			drv_disp_data_out(drv_seg_change(LED.SEG_BUF[10]));
			clk2_pulse();						// CLK2ﾊﾟﾙｽ
			
			// 「DEG14」
			disp4_data_set();					// DISP4ﾃﾞｰﾀｾｯﾄ
			clk3_pulse();						// CLK3ﾊﾟﾙｽ
			
			OUT.LED_STATUS++;
			break;

		// 7SEG6
		case 11:
			// 「Spindle Speed」1「Number of Flutes」
			// 「DBUS00-07」
			drv_disp_data_out(drv_seg_change(LED.SEG_BUF[11]));
			clk1_pulse();						// CLK1ﾊﾟﾙｽ
			
			// 「DBUS08-15」
			drv_disp_data_out(drv_seg_change(LED.SEG_BUF[14]));
			clk2_pulse();						// CLK2ﾊﾟﾙｽ
			
			// 「DEG15」
			disp7_data_set();					// DISP7ﾃﾞｰﾀｾｯﾄ
			clk3_pulse();						// CLK3ﾊﾟﾙｽ
			
			OUT.LED_STATUS++;
			break;

		// 7SEG7
		case 13:
			// 「Spindle Speed」2・3
			// 「DBUS00-07」
			drv_disp_data_out(drv_seg_change(LED.SEG_BUF[12]));
			clk1_pulse();						// CLK1ﾊﾟﾙｽ
			
			// 「DBUS08-15」
			drv_disp_data_out(drv_seg_change(LED.SEG_BUF[13]));
			clk2_pulse();						// CLK2ﾊﾟﾙｽ
			
			// 「DEG16」
			disp6_data_set();					// DISP6ﾃﾞｰﾀｾｯﾄ
			clk3_pulse();						// CLK3ﾊﾟﾙｽ
			
			OUT.LED_STATUS++;
			break;

		// 焦点L・R・X軸11灯LED
		case 15:
			// 「DBUS00-07」
			buf = LED.FOCUSING;					// X軸11灯LED(下位8ﾋﾞｯﾄ)
			drv_disp_data_out(buf);
			clk1_pulse();						// CLK1ﾊﾟﾙｽ
			
			// 「DBUS08-15」
			buf =	(LED.FOCUSING >> 8) & 0x07;	// X軸11灯LED(上位3ﾋﾞｯﾄ)
			buf |=	(LED.FOCUS.BIT.R << 4) | (LED.FOCUS.BIT.L << 3);	// 焦点L・R
			drv_disp_data_out(buf);
			clk2_pulse();						// CLK2ﾊﾟﾙｽ
			
			// 「DEG17」
			disp5_data_set();					// DISP5ﾃﾞｰﾀｾｯﾄ
			clk3_pulse();						// CLK3ﾊﾟﾙｽ
			
			OUT.LED_STATUS++;
			break;
			
		// 全てのﾃﾞｰﾀをﾘｾｯﾄ
		case 16:
			LED.ON_COUNT++;						// ON時間を加算
			if(LED.ON_COUNT >= on_count_led){	// ON時間が設定値に達したら
				LED.ON_COUNT = 0;				// ON時間をﾘｾｯﾄ
				disp_data_reset();				// LED表示ﾃﾞｰﾀﾘｾｯﾄ
				clk3_pulse();					// CLK3ﾊﾟﾙｽ
				OUT.LED_STATUS++;
			}
			break;
			
		// 測定・設定・ﾋﾟｰｸﾎｰﾙﾄﾞ
		case 17:
			// 「DBUS00-07」
			buf = LED.MSP.WORD;					// Measure・Setting・Peak Hold点灯設定(下位8ﾋﾞｯﾄ)
			drv_disp_data_out(buf);
			clk1_pulse();						// CLK1ﾊﾟﾙｽ
			
			// 「DBUS08-15」
			buf = (LED.MSP.WORD >> 8);			// Measure・Setting・Peak Hold点灯設定(上位1ﾋﾞｯﾄ)
			drv_disp_data_out(buf);
			clk2_pulse();						// CLK2ﾊﾟﾙｽ
			
			// 「DEG18」
			disp0_data_set();					// DISP0ﾃﾞｰﾀｾｯﾄ
			clk4_pulse();						// CLK4ﾊﾟﾙｽ
			
			OUT.LED_STATUS++;
			break;
			
		// 全てのﾃﾞｰﾀをﾘｾｯﾄ
		case 18:
			LED.ON_COUNT++;						// ON時間を加算
			if(LED.ON_COUNT >= on_count_led){	// ON時間が設定値に達したら
				LED.ON_COUNT = 0;				// ON時間をﾘｾｯﾄ
				disp_data_reset();				// LED表示ﾃﾞｰﾀﾘｾｯﾄ
				clk4_pulse();					// CLK4ﾊﾟﾙｽ
				//OUT.LED_STATUS = 1;
				OUT.LED_STATUS++;
			}
			break;
			
		//
		// 焦点Z・Z軸4灯LED
		case 19:
			// 「DBUS00-07」
			buf = LED.Z_FOCUSING;				// Z軸4灯LED
			buf |=	LED.FOCUS.BIT.Z << 4;		// 焦点Z
			drv_disp_data_out(buf);
			clk1_pulse();						// CLK1ﾊﾟﾙｽ
			
			// 「DBUS08-15」
			
			// 「DEG16」(CNC)
			disp1_data_set();					// DISP0ﾃﾞｰﾀｾｯﾄ
			clk4_pulse();						// CLK4ﾊﾟﾙｽ
			
			OUT.LED_STATUS++;
			break;
			
		// 全てのﾃﾞｰﾀをﾘｾｯﾄ
		case 20:
			LED.ON_COUNT++;						// ON時間を加算
			if(LED.ON_COUNT >= on_count_led){	// ON時間が設定値に達したら
				LED.ON_COUNT = 0;				// ON時間をﾘｾｯﾄ
				disp_data_reset();				// LED表示ﾃﾞｰﾀﾘｾｯﾄ
				clk4_pulse();					// CLK4ﾊﾟﾙｽ
				OUT.LED_STATUS = 1;
			}
			break;
		//
			
		default:
			OUT.LED_STATUS = 1;
	}
}
//************************************************************/
//				表示ﾃﾞｰﾀ出力関数
//************************************************************/
void drv_disp_data_out(_UBYTE w_data)
{
	/*
	// 7ｾｸﾞ・LED出力
	DISP0_OUT = w_data & 0x01;
	DISP1_OUT = (w_data >> 1) & 0x01;
	DISP2_OUT = (w_data >> 2) & 0x01;
	DISP3_OUT = (w_data >> 3) & 0x01;
	DISP4_OUT = (w_data >> 4) & 0x01;
	DISP5_OUT = (w_data >> 5) & 0x01;
	DISP6_OUT = (w_data >> 6) & 0x01;
	DISP7_OUT = (w_data >> 7) & 0x01;
	*/
	PORT7.DR.BYTE = w_data;
	
	//#define DISP0_OUT			PORT7.DR.BIT.B0			// DISP0出力
}

//************************************************************/
//				ｾｸﾞﾒﾝﾄﾃﾞｰﾀ変換関数
//************************************************************/
//ASCﾃﾞｰﾀをｾｸﾞﾒﾝﾄﾃﾞｰﾀに変換する
_UBYTE drv_seg_change(_UBYTE seg)
{
	_UBYTE ret;

	switch(seg){
		case '0':		ret	= OUT_SEG_0;	break;
		case '1':		ret	= OUT_SEG_1;	break;
		case '2':		ret	= OUT_SEG_2;	break;
		case '3':		ret	= OUT_SEG_3;	break;
		case '4':		ret	= OUT_SEG_4;	break;
		case '5':		ret	= OUT_SEG_5;	break;
		case '6':		ret	= OUT_SEG_6;	break;
		case '7':		ret	= OUT_SEG_7;	break;
		case '8':		ret	= OUT_SEG_8;	break;
		case '9':		ret	= OUT_SEG_9;	break;
		
		case '-':		ret	= OUT_SEG_MI;	break;
		case '.':		ret	= OUT_SEG_DOT;	break;
		case ' ':		ret	= OUT_SEG_BL;	break;
		
		case 'E':		ret	= OUT_SEG_E;	break;
		case 'F':		ret	= OUT_SEG_F;	break;
		
		case 'r':		ret	= OUT_SEG_r;	break;
		
		// 小数点付き
		case 0xB0:		ret	= OUT_SEG_0_DOT;	break;	// 0.
		case 0xB1:		ret	= OUT_SEG_1_DOT;	break;	// 1.
		case 0xB2:		ret	= OUT_SEG_2_DOT;	break;	// 2.
		case 0xB3:		ret	= OUT_SEG_3_DOT;	break;	// 3.
		case 0xB4:		ret	= OUT_SEG_4_DOT;	break;	// 4.
		case 0xB5:		ret	= OUT_SEG_5_DOT;	break;	// 5.
		case 0xB6:		ret	= OUT_SEG_6_DOT;	break;	// 6.
		case 0xB7:		ret	= OUT_SEG_7_DOT;	break;	// 7.
		case 0xB8:		ret	= OUT_SEG_8_DOT;	break;	// 8.
		case 0xB9:		ret	= OUT_SEG_9_DOT;	break;	// 9.

		case OUT_SEG_NO1:	ret	= OUT_SEG_NO1;	break;	// high
		case OUT_SEG_NO2:	ret	= OUT_SEG_NO2;	break;	// middle
		case OUT_SEG_NO3:	ret	= OUT_SEG_NO3;	break;	// low
	}
	return(ret);
}
