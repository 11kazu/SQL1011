/******************************************************************************
* File Name	: power_sequence.c
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

void set_7seg_led_error_no(_UWORD temp);		// 7セグLEDのｴﾗｰﾅﾝﾊﾞｰ設定
void clear_7seg_led_error_no();					// 7セグLEDのｴﾗｰﾅﾝﾊﾞｰｸﾘｱ
void power_sequence(void);						// POWERｼｰｹﾝｽ
void light_adjustment(void);					// 輝度調整(自動)
void light_adjustment_manual(void);				// 輝度調整(手動)

//************************************************************/
//				7セグLEDのｴﾗｰﾅﾝﾊﾞｰ設定
//************************************************************/
void set_7seg_led_error_no(_UWORD temp)
{
	LED.SEG_BUF[1] = 'E';							// ｴﾗｰ
	LED.SEG_BUF[2] = ((temp / 1000) % 10 + 0x30);	// 千の位
	LED.SEG_BUF[3] = ((temp / 100) % 10 + 0x30);	// 百の位
	LED.SEG_BUF[4] = ((temp / 10) % 10 + 0x30);		// 十の位
	LED.SEG_BUF[5] = (temp % 10 + 0x30);			// 一の位

// add 2016.06.22 K.Uemura start	G62401
	set_7seg_lower_no_data();						// 7ｾｸﾞ表示(-----)(下段)
// add 2016.06.22 K.Uemura end
}

//************************************************************/
//				7セグLEDのｴﾗｰﾅﾝﾊﾞｰ設定
//************************************************************/
void clear_7seg_led_error_no(void)
{
	set_7seg_upper_no_data();
	set_7seg_lower_no_data();
}

//************************************************************/
//				POWERｼｰｹﾝｽ
//	SEQ.POWER_STATUSの遷移（10msごと）
//		                                                               ┌輝度調整─┐
//		1 → 2 → 3 → 11 → 12 → 13 → 21 → 22 → 23 → 31 → 32 → 33 → 34 → 35 → 33
//																		  → 39 → 40 → 41 → 2 → Z方向センサ
//																							→ 52 → 完了
//		                     ↓
// 		                     11へ戻る(リトライ5回で終了しない場合はエラー)
//************************************************************/
void power_sequence(void)
{
	//float f_data;
	_UDWORD black_check, white_check, sensor_check_upper, sensor_check_lower;
	_UWORD bright;
// add 2016.01.21 K.Uemura start	G21701
	_UWORD bright_initial = 500;		// 調光初期値
// add 2016.01.21 K.Uemura end
	_UBYTE err_flg[3];
	unsigned long bright_check[3];
// add 2016.10.19 K.Uemura start	GA1901
	unsigned short static error_no = 0;
// add 2016.10.19 K.Uemura end

	
	switch(SEQ.POWER_STATUS){
		// 電源ON/OFFのとき
		// DA出力値をｾｯﾄ(最小)
		// 電源起動処理
		case 1:
			if(SEQ.FLAG.BIT.POWER_ON == 1){					// 電源ONﾌﾗｸﾞがONのとき
// add 2015.11.04 K.Uemura start	
				SEQ.FLAG5.BIT.INITIALIZE_FLAG = 0;			// 起動プロセス(初期化処理が完了すると1)
// add 2015.11.04 K.Uemura end

				SEQ.FLAG.BIT.POWER_ON = 0;					// 電源ONﾌﾗｸﾞをﾘｾｯﾄ
				DA.DADR0 = 0;								// DA出力値をｾｯﾄ(最小)
				SEQ.POWER_COUNT = 0;						// 電源制御ｶｳﾝﾄをﾘｾｯﾄ
				SEQ.CHECK_RETRY_COUNT = 0;					// BLACKﾁｪｯｸ・WHITEﾁｪｯｸ再送ｶｳﾝﾀ
				SEQ.POWER_SEQUENCE_COUNT = 0;				// POWERｼｰｹﾝｽｶｳﾝﾄ
				
				SEQ.FLAG.BIT.MEMORY_CONTROL = 1;			// 7ｾｸﾞを消灯
				if(COM0.MASTER_STATUS != RESET_MODE){
					COM0.MASTER_STATUS = IDLE_MODE;			// ﾀｯﾁﾊﾟﾈﾙ通信を停止
				}
				
				// 計測ﾀｲﾌﾟがCNC版のとき(X方向を設定してから続けてZ方向を設定する)
				if(IN.FLAG.BIT.HARDWARE_TYPE == CNC_EDITION){
					SEQ.FLAG.BIT.CNC_INITIAL	= 1;				// CNC電源起動時初期化ﾌﾗｸﾞ
					SEQ.MEASUREMENT_DIRECTION	= X_DIRECTION;		// 計測方向をX方向
				}
				
				// 値を初期化
				SEQ.LED_BRIGHTNESS = SEQ.X_LED_BRIGHTNESS = SEQ.Z_LED_BRIGHTNESS = 0;			// LED輝度値を初期化
				SEQ.X_BLACK_AVE = SEQ.X_WHITE_AVE = SEQ.X_WHITE_MAX = SEQ.X_WHITE_MIN = 0;		// CNC版:X軸／PBL版:Normal
				SEQ.Z_BLACK_AVE = SEQ.Z_WHITE_AVE = SEQ.Z_WHITE_MAX = SEQ.Z_WHITE_MIN = 0;		// CNC版:Z軸／PBL版:Eco
				//
				
				SEQ.POWER_STATUS++;
				
			}else if(SEQ.FLAG.BIT.POWER_OFF == 1){			// 電源OFFﾌﾗｸﾞがONのとき
				SEQ.FLAG.BIT.POWER_OFF = 0;					// 電源OFFﾌﾗｸﾞをﾘｾｯﾄ
				DA.DADR0 = 0;								// DA出力値をｾｯﾄ(最小)
				COM0.NO310.BIT.RDY = 0;						// READY OFF
				
				SEQ.FLAG.BIT.CNC_INITIAL = 0;				// CNC電源起動時初期化ﾌﾗｸﾞ
			}
			break;
			
		// ﾊﾟﾗﾒｰﾀ送信
		case 2:
			SEQ.POWER_COUNT = 0;							// 電源制御ｶｳﾝﾄをﾘｾｯﾄ
			// ﾊﾟﾗﾒｰﾀを送信
			SEQ.CHANGE_FPGA = 1;							// FPGA処理のﾌﾗｸﾞを切り替え(ﾊﾟﾗﾒｰﾀ転送 send_to_fpga)
			SEQ.FPGA_SEND_STATUS = 1;
			SEQ.POWER_STATUS++;
			break;
			
		// 1000ms後
		// ﾀｲﾑｱｳﾄ
		// ﾊﾟﾗﾒｰﾀ送信待機
		case 3:
			SEQ.POWER_COUNT++;								// 電源制御ｶｳﾝﾄ
			if(SEQ.POWER_COUNT >= 250){						// 電源制御ｶｳﾝﾄが「10」のとき
				SEQ.POWER_COUNT = 0;						// send_to_fpga_verify case16 で、SEQ.POWER_COUNT=250代入
				SEQ.POWER_STATUS = 11;
			}
			break;
			
		// 10ms後
		// ﾌﾞﾗｯｸ固定値設定
		// ﾌﾞﾗｯｸ固定値設定送信
		case 11:
			SEQ.POWER_COUNT++;								// 電源制御ｶｳﾝﾄ
			if(SEQ.POWER_COUNT >= 1){						// 電源制御ｶｳﾝﾄが「10」のとき
				SEQ.POWER_COUNT = 0;						// 電源制御ｶｳﾝﾄをﾘｾｯﾄ
				SEQ.BLACK_WHITE_CHECK[0] = SEQ.BLACK_WHITE_CHECK[1] = SEQ.BLACK_WHITE_CHECK[2] = SEQ.BLACK_WHITE_CHECK[3] = 0;
				
				SEQ.CBUS_NUMBER = 381;						// ﾌﾞﾗｯｸ固定値設定
				SEQ.CHANGE_FPGA = 3;
				SEQ.FPGA_SEND_STATUS = 1;
				SEQ.POWER_STATUS++;							// 次へ
			}
			break;
			
		// 100ms後
		// ﾀｲﾑｱｳﾄ
		// ﾌﾞﾗｯｸ固定値設定送信待機
		case 12:
			SEQ.POWER_COUNT++;								// 電源制御ｶｳﾝﾄ
			if(SEQ.POWER_COUNT >= 10){						// 電源制御ｶｳﾝﾄが「10」のとき
				// ﾀｲﾑｱｳﾄ
				SEQ.POWER_STATUS--;
				SEQ.CHECK_RETRY_COUNT++;					// BLACKﾁｪｯｸ・WHITEﾁｪｯｸ再送ｶｳﾝﾀ
				if(SEQ.CHECK_RETRY_COUNT >= 5){
					SEQ.POWER_STATUS = 98;
					COM0.NO312 = ERR_BLACK_TIMEOUT;			// 1001	BLACKﾀｲﾑｱｳﾄ(FPGAとのﾃﾞｰﾀ送受信)
				}
			}
			break;
			
		// ﾌﾞﾗｯｸ固定値設定判定
		case 13:
			SEQ.POWER_COUNT = 0;									// 電源制御ｶｳﾝﾄをﾘｾｯﾄ
			
			if(IN.FLAG.BIT.HARDWARE_TYPE == CNC_EDITION){			// 計測ﾀｲﾌﾟがCNC版のとき
				if(SEQ.MEASUREMENT_DIRECTION == X_DIRECTION){		// 計測方向がX方向のとき
					black_check = X_BLACK_CHECK;					// BLACKﾁｪｯｸ
					bright = SENSOR_INITIAL_BRIGHT;
				}else{												// 計測方向がZ方向のとき
					black_check = Z_BLACK_CHECK;					// BLACKﾁｪｯｸ(Z)
					bright = SENSOR_INITIAL_BRIGHT;
				}
			}else{													// 計測ﾀｲﾌﾟがﾎﾟｰﾀﾌﾞﾙ版のとき
				if(SEQ.FLAG.BIT.ECO == 0){							// 計測ﾓｰﾄﾞがNORMALのとき
					black_check = NORMAL_BLACK_CHECK;				// BLACKﾁｪｯｸ
					bright = 500;
				}else{												// 計測ﾓｰﾄﾞがECOのとき
					black_check = ECO_BLACK_CHECK;					// BLACKﾁｪｯｸ(ECO)
					bright = 250;
				}
			}
			
			if( ((IN.FLAG.BIT.HARDWARE_TYPE == CNC_EDITION) && (SEQ.MEASUREMENT_DIRECTION == X_DIRECTION)) || 
				((IN.FLAG.BIT.HARDWARE_TYPE == PORTABLE_EDITION) && (SEQ.FLAG.BIT.ECO == 0)) ){
				// CNC版：計測方向がXのとき
				// PBL版：露光時間Normalのとき
				SEQ.X_BLACK_AVE = SEQ.BLACK_WHITE_CHECK[0];		// ﾌﾞﾗｯｸ平均値(X)
			}else{
				SEQ.Z_BLACK_AVE = SEQ.BLACK_WHITE_CHECK[0];		// ﾌﾞﾗｯｸ平均値(Z)
			}
			
			// 計測値が「0」のときは異常値とみなし再送を行う 160304
			if(SEQ.BLACK_WHITE_CHECK[0] == 0){					// 計測値が「0」のとき
				SEQ.POWER_STATUS = 11;
				SEQ.CHECK_RETRY_COUNT++;						// BLACKﾁｪｯｸ・WHITEﾁｪｯｸ再送ｶｳﾝﾀ
				if(SEQ.CHECK_RETRY_COUNT >= 5){
					SEQ.POWER_STATUS = 98;
					COM0.NO312 = 1009;							// 1009	BLACK値NG
				}
			}else if(SEQ.BLACK_WHITE_CHECK[0] < black_check){	// 設定値よりも計測値が低いとき
				DA.DADR0 = bright;								// DA出力値をｾｯﾄ(DA0最大値)
				SEQ.CHECK_RETRY_COUNT = 0;						// BLACKﾁｪｯｸ・WHITEﾁｪｯｸ再送ｶｳﾝﾀ
				SEQ.POWER_STATUS = 21;							// 次へ
			}else{
				SEQ.POWER_STATUS = 11;
				SEQ.CHECK_RETRY_COUNT++;						// BLACKﾁｪｯｸ・WHITEﾁｪｯｸ再送ｶｳﾝﾀ
				if(SEQ.CHECK_RETRY_COUNT >= 5){
					SEQ.POWER_STATUS = 98;
					COM0.NO312 = ERR_BLACK_NG;					// 1010	BLACK値NG
				}
			}
			break;
			
		// 10ms後
		// ﾎﾜｲﾄ固定値設定
		// ﾎﾜｲﾄ固定値設定送信
		case 21:
			SEQ.POWER_COUNT++;								// 電源制御ｶｳﾝﾄ
			if(SEQ.POWER_COUNT == 1){						// 電源制御ｶｳﾝﾄが「10」のとき
				SEQ.POWER_COUNT = 0;						// 電源制御ｶｳﾝﾄをﾘｾｯﾄ
				SEQ.WHITE_CHECK_COUNT = 1;					// WHITEﾁｪｯｸｶｳﾝﾀ
				SEQ.CBUS_NUMBER = 384;						// ﾎﾜｲﾄﾁｪｯｸ
				SEQ.CHANGE_FPGA = 3;
				SEQ.FPGA_SEND_STATUS = 1;
				SEQ.POWER_STATUS++;							// 次へ
			}
			break;
			
		// 100ms後
		// ﾀｲﾑｱｳﾄ
		// ﾎﾜｲﾄ固定値設定待機
		case 22:
			SEQ.POWER_COUNT++;								// 電源制御ｶｳﾝﾄ
			if(SEQ.POWER_COUNT == 10){						// 電源制御ｶｳﾝﾄが「10」のとき
				// ﾀｲﾑｱｳﾄ
				SEQ.POWER_STATUS--;
				SEQ.CHECK_RETRY_COUNT++;					// BLACKﾁｪｯｸ・WHITEﾁｪｯｸ再送ｶｳﾝﾀ
				if(SEQ.CHECK_RETRY_COUNT >= 5){
					SEQ.POWER_STATUS = 98;
					COM0.NO312 = ERR_WHITE_TIMEOUT;			// 1002	WHITEﾀｲﾑｱｳﾄ(FPGAとのﾃﾞｰﾀ送受信)
				}
			}
			break;
			
		// ﾎﾜｲﾄ固定値設定判定
		case 23:
			SEQ.POWER_COUNT = 0;										// 電源制御ｶｳﾝﾄをﾘｾｯﾄ
			
			if(IN.FLAG.BIT.HARDWARE_TYPE == CNC_EDITION){				// 計測ﾀｲﾌﾟがCNC版のとき
				if(SEQ.MEASUREMENT_DIRECTION == X_DIRECTION){			// 計測方向がX方向のとき
					white_check			= X_WHITE_CHECK;				// WHITEﾁｪｯｸ
					sensor_check_upper	= X_SENSOR_CHECK_UPPER;			// ﾗｲﾝｾﾝｻﾁｪｯｸ下限
					sensor_check_lower	= X_SENSOR_CHECK_LOWER;			// ﾗｲﾝｾﾝｻﾁｪｯｸ上限
				}else{													// 計測方向がZ方向のとき
					white_check			= Z_WHITE_CHECK;				// WHITEﾁｪｯｸ(Z)
					sensor_check_upper	= Z_SENSOR_CHECK_UPPER;			// ﾗｲﾝｾﾝｻﾁｪｯｸ下限(Z)
					sensor_check_lower	= Z_SENSOR_CHECK_LOWER;			// ﾗｲﾝｾﾝｻﾁｪｯｸ上限(Z)
				}
			}else{														// 計測ﾀｲﾌﾟがﾎﾟｰﾀﾌﾞﾙ版のとき
				if(SEQ.FLAG.BIT.ECO == 0){								// 計測ﾓｰﾄﾞがNORMALのとき
					white_check			= NORMAL_WHITE_CHECK;			// WHITEﾁｪｯｸ
					sensor_check_upper	= NORMAL_SENSOR_CHECK_UPPER;	// ﾗｲﾝｾﾝｻﾁｪｯｸ下限
					sensor_check_lower	= NORMAL_SENSOR_CHECK_LOWER;	// ﾗｲﾝｾﾝｻﾁｪｯｸ上限
				}else{													// 計測ﾓｰﾄﾞがECOのとき
					white_check			= ECO_WHITE_CHECK;				// WHITEﾁｪｯｸ(ECO)
					sensor_check_upper	= ECO_SENSOR_CHECK_UPPER;		// ﾗｲﾝｾﾝｻﾁｪｯｸ下限(ECO)
					sensor_check_lower	= ECO_SENSOR_CHECK_LOWER;		// ﾗｲﾝｾﾝｻﾁｪｯｸ上限(ECO)
				}
			}
			
			if( ((IN.FLAG.BIT.HARDWARE_TYPE == CNC_EDITION) && (SEQ.MEASUREMENT_DIRECTION == X_DIRECTION)) || 
				((IN.FLAG.BIT.HARDWARE_TYPE == PORTABLE_EDITION) && (SEQ.FLAG.BIT.ECO == 0)) ){
				// CNC版：計測方向がXのとき
				// PBL版：露光時間Normalのとき
				SEQ.X_WHITE_AVE = SEQ.BLACK_WHITE_CHECK[1];		// 平均値
				SEQ.X_WHITE_MIN = SEQ.BLACK_WHITE_CHECK[3];		// 最小値
				SEQ.X_WHITE_MAX = SEQ.BLACK_WHITE_CHECK[2];		// 最大値
			}else{
				SEQ.Z_WHITE_AVE = SEQ.BLACK_WHITE_CHECK[1];		// 平均値
				SEQ.Z_WHITE_MIN = SEQ.BLACK_WHITE_CHECK[3];		// 最小値
				SEQ.Z_WHITE_MAX = SEQ.BLACK_WHITE_CHECK[2];		// 最大値
			}

			//エラーフラグの設定
			err_flg[0] = err_flg[1] = err_flg[2] = 1;

			//しきい値と比較（条件満たすとOK）
			if(SEQ.BLACK_WHITE_CHECK[1] > white_check)			err_flg[0] = 0;		// 平均値
			if(SEQ.BLACK_WHITE_CHECK[2] < sensor_check_upper)	err_flg[1] = 0;		// 最大値
			if(SEQ.BLACK_WHITE_CHECK[3] > sensor_check_lower)	err_flg[2] = 0;		// 最小値

			if(err_flg[0] | err_flg[1] | err_flg[2]){
				//エラーあり(一つでも)
				SEQ.POWER_STATUS = 21;
				SEQ.CHECK_RETRY_COUNT++;			// BLACKﾁｪｯｸ・WHITEﾁｪｯｸ再送ｶｳﾝﾀ
			}else{
				//エラー無し
				DA.DADR0 = 0;						// DA出力値をｾｯﾄ(最小)
				SEQ.POWER_STATUS = 31;
				COM0.NO312 = 0;						// 312 ｴﾗｰ番号
				SEQ.CHECK_RETRY_COUNT = 0;			// BLACKﾁｪｯｸ・WHITEﾁｪｯｸ再送ｶｳﾝﾀ
			}

			if(SEQ.CHECK_RETRY_COUNT >= 5){
				SEQ.POWER_STATUS = 98;

				// min max ave
				// 04  02  01
				// ○  ○  ×   0x01
				// ○  ×  ○   0x02
				// ×  ○  ○   0x04
				// ○  ×  ×   0x03
				// ×  ○  ×   0x05
				// ×  ×  ○   0x06
				// ×  ×  ×   0x07
				bright = (err_flg[2]<<2 | err_flg[1]<<1 | err_flg[0]);
				COM0.NO312 = ERR_WHITE_NG + (err_flg[2]<<2 | err_flg[1]<<1 | err_flg[0]);	// 1012	WHITE値NG

				//センサ出力に大差が無い場合は、LEDが点灯していないと判断
				bright_check[0] = abs(SEQ.BLACK_WHITE_CHECK[1] - SEQ.BLACK_WHITE_CHECK[0]);		//平均 - BLACK
				bright_check[1] = abs(SEQ.BLACK_WHITE_CHECK[2] - SEQ.BLACK_WHITE_CHECK[0]);		//最大 - BLACK
				bright_check[2] = abs(SEQ.BLACK_WHITE_CHECK[3] - SEQ.BLACK_WHITE_CHECK[0]);	//最小 - BLACK
				if((bright_check[0] < 5) && (bright_check[1] < 5) && (bright_check[2] < 5)){
					COM0.NO312 = ERR_NOLED;	// 1200	LED点灯しない
				}
			}
			break;
			
		// 調光値設定処理
		case 31:
#ifdef	__INITIAL_SEQUENCE
			SEQ.DIM_RETRY_COUNT = 0;					// 調光ﾘﾄﾗｲｶｳﾝﾄ
			for(SEQ.DIM_RETRY_COUNT = 0; SEQ.DIM_RETRY_COUNT < 50; SEQ.DIM_RETRY_COUNT++){
				DEBUG_STR.DEBUG[100+(SEQ.DIM_RETRY_COUNT*2)+0] = 0;	// 輝度値
				DEBUG_STR.DEBUG[100+(SEQ.DIM_RETRY_COUNT*2)+1] = 0;	// センサ出力(デジタル値)
			}
			DEBUG_STR.DEBUG[99] = SEQ.MEASUREMENT_DIRECTION;
#endif

			if(BRIGHTNESS_ADJUSTMENT == 1){					// LED輝度調整が「1(自動)」のとき
				SEQ.DIM_LEVEL = bright_initial;
			}else{
				SEQ.DIM_LEVEL = MANUAL_BRIGHTNESS;			// LED輝度値(手動)
			}
			SEQ.POWER_STATUS++;								// 次へ
			break;
			
		// 100ms後
		// 調光値設定送信
		case 32:
			SEQ.POWER_COUNT++;								// 電源制御ｶｳﾝﾄ
			if(SEQ.POWER_COUNT == 10){						// 電源制御ｶｳﾝﾄが「10」のとき
				SEQ.POWER_COUNT = 0;						// 電源制御ｶｳﾝﾄをﾘｾｯﾄ
				SEQ.CBUS_NUMBER = 384;						// ﾎﾜｲﾄﾁｪｯｸ
				SEQ.WHITE_CHECK_COUNT = 1;
				SEQ.DIM_RETRY_COUNT = 0;					// 調光ﾘﾄﾗｲｶｳﾝﾄ
				SEQ.BLACK_WHITE_CHECK[1] = 0;
				SEQ.CHANGE_FPGA = 3;
				SEQ.FPGA_SEND_STATUS = 1;
				SEQ.POWER_STATUS++;							// 次へ
			}
			break;
			
		// 100ms後
		// ﾀｲﾑｱｳﾄ
		// 調光値設定待機
		case 33:
			SEQ.POWER_COUNT++;								// 電源制御ｶｳﾝﾄ
			if(SEQ.POWER_COUNT == 10){						// 電源制御ｶｳﾝﾄが「10」のとき
				// ﾀｲﾑｱｳﾄ
				SEQ.POWER_STATUS--;
				SEQ.CHECK_RETRY_COUNT++;					// BLACKﾁｪｯｸ・WHITEﾁｪｯｸ再送ｶｳﾝﾀ
				if(SEQ.CHECK_RETRY_COUNT >= 5){
					SEQ.POWER_STATUS = 98;
					COM0.NO312 = ERR_BRIGHTNESS_TIMEOUT;	// 1003	調光ﾀｲﾑｱｳﾄ(FPGAとのﾃﾞｰﾀ送受信)
				}
			}
			break;
			
		// 調光値設定判定
		case 34:
			SEQ.POWER_COUNT = 0;							// 電源制御ｶｳﾝﾄをﾘｾｯﾄ
			
			// 輝度調整
			if(BRIGHTNESS_ADJUSTMENT == 1){					// LED輝度調整が「1(自動)」のとき
				light_adjustment();
			}else{
				light_adjustment_manual();
			}
			break;
			
		// 調光値設定再送
		case 35:
			SEQ.CBUS_NUMBER = 384;							// ﾎﾜｲﾄﾁｪｯｸ
			SEQ.WHITE_CHECK_COUNT = 1;
			SEQ.CHANGE_FPGA = 3;
			SEQ.FPGA_SEND_STATUS = 1;
			SEQ.POWER_STATUS = 33;							// 次へ
			break;
			
		// 10ms後
		// ﾎﾜｲﾄ固定値設定
		case 39:
			SEQ.POWER_COUNT++;								// 電源制御ｶｳﾝﾄ
			if(SEQ.POWER_COUNT >= 1){						// 電源制御ｶｳﾝﾄが「10」のとき
				SEQ.POWER_COUNT = 0;						// 電源制御ｶｳﾝﾄをﾘｾｯﾄ
				SEQ.CBUS_NUMBER = 383;						// ﾎﾜｲﾄ固定値設定
				SEQ.CHANGE_FPGA = 3;
				SEQ.FPGA_SEND_STATUS = 1;
				SEQ.POWER_STATUS++;							// 次へ
			}
			break;
			
		// 100ms後
		// ﾀｲﾑｱｳﾄ
		// ﾎﾜｲﾄ固定値設定待機
		case 40:
			SEQ.POWER_COUNT++;										// 電源制御ｶｳﾝﾄ
			if(SEQ.POWER_COUNT >= 10){								// 電源制御ｶｳﾝﾄが「10」のとき
				// ﾀｲﾑｱｳﾄ
				SEQ.POWER_STATUS--;
				SEQ.CHECK_RETRY_COUNT++;							// BLACKﾁｪｯｸ・WHITEﾁｪｯｸ再送ｶｳﾝﾀ
				if(SEQ.CHECK_RETRY_COUNT >= 5){
					SEQ.POWER_STATUS = 98;
					COM0.NO312 = ERR_WHITE_SETTING_TIMEOUT;			// 1004	WHITE設定ﾀｲﾑｱｳﾄ(FPGAとのﾃﾞｰﾀ送受信)
				}
			}
			break;
			
		// ﾊﾟﾗﾒｰﾀ送信
		case 41:
			SEQ.POWER_COUNT = 0;							// 電源制御ｶｳﾝﾄをﾘｾｯﾄ
			DA.DADR0 = 0;									// DA出力値をｾｯﾄ(最小)
			COM0.NO312 = 0;									// 312 ｴﾗｰ番号(なし)
			
			SEQ.FLAG.BIT.MEMORY_CONTROL = 1;				// 7ｾｸﾞを消灯
			
			SEQ.POWER_STATUS = 52;
			
			// CNC電源起動時初期化ﾌﾗｸﾞがONのとき
			if(SEQ.FLAG.BIT.CNC_INITIAL == 1){
				// 計測方向がX方向のとき(続けてZ方向を設定する)
				if(SEQ.MEASUREMENT_DIRECTION == X_DIRECTION){
					SEQ.MEASUREMENT_DIRECTION = Z_DIRECTION;		// 計測方向をZ方向
// add 2016.01.21 K.Uemura start	G21701
					DA.DADR0 = SENSOR_OUT_LEVEL_BRIGHT;				// DA出力値をｾｯﾄ(最小)
// add 2016.01.21 K.Uemura end
					SEQ.POWER_STATUS = 2;
				}
			}
			
			/*
			if(OUT.LED_STATUS == 1){
				// ﾊﾟﾗﾒｰﾀを送信
				SEQ.CHANGE_FPGA = 1;				// FPGA処理のﾌﾗｸﾞを切り替え
				SEQ.FPGA_SEND_STATUS = 1;
				SEQ.POWER_STATUS++;
			}
			*/
			break;
			
		// 1000ms後
		// ﾀｲﾑｱｳﾄ
		// ﾊﾟﾗﾒｰﾀ送信待機
		case 42:
			SEQ.POWER_COUNT++;								// 電源制御ｶｳﾝﾄ
			if(SEQ.POWER_COUNT >= 100){						// 電源制御ｶｳﾝﾄが「10」のとき
				SEQ.POWER_STATUS = 51;
			}
			break;
			
		// ﾊﾟﾗﾒｰﾀ送信
		case 51:
			SEQ.POWER_COUNT = 0;							// 電源制御ｶｳﾝﾄをﾘｾｯﾄ
			// ﾊﾟﾗﾒｰﾀを送信
			SEQ.CHANGE_FPGA = 1;					// FPGA処理のﾌﾗｸﾞを切り替え
			SEQ.FPGA_SEND_STATUS = 1;
			SEQ.POWER_STATUS++;
			break;
			
			
		// 1000ms後
		// ﾀｲﾑｱｳﾄ
		// ﾊﾟﾗﾒｰﾀ送信待機・電源起動処理完了
		case 52:
			SEQ.POWER_COUNT++;								// 電源制御ｶｳﾝﾄ
			if(SEQ.POWER_COUNT >= 100){						// 電源制御ｶｳﾝﾄが「10」のとき
				SEQ.POWER_COUNT = 0;						// 電源制御ｶｳﾝﾄをﾘｾｯﾄ
				
				SEQ.FLAG2.BIT.BLACK_WHITE_COMPLETE = 1;		// BLACKﾁｪｯｸ・WHITEﾁｪｯｸ完了ﾌﾗｸﾞ
				SEQ.POWER_STATUS = 1;
				
				SEQ.FLAG.BIT.CNC_INITIAL = 0;				// CNC電源起動時初期化ﾌﾗｸﾞ
// add 2015.11.04 K.Uemura start	
				SEQ.FLAG5.BIT.INITIALIZE_FLAG = 1;			// 起動プロセス(初期化処理が完了すると1)
// add 2015.11.04 K.Uemura end
				
				SEQ.CHANGE_FPGA = 0;
				
				SEQ.FLAG.BIT.MEMORY_CONTROL = 0;			// 7ｾｸﾞを点灯
				COM0.SUB_STATUS = 1;
				
				if(COM0.MASTER_STATUS == RESET_MODE){
					COM0.MASTER_STATUS = INIT_START_MODE;
				}else{
					COM0.MASTER_STATUS = DRV_MODE;			// ﾀｯﾁﾊﾟﾈﾙ通信を開始
				}
				
				OUT.MASTER_STATUS = DRV_MODE;				// 通常ﾓｰﾄﾞ
				OUT.SUB_STATUS = 1;
				
				COM0.NO310.BIT.RDY = 1;						// READY ON

			}
			break;
			
		// ｴﾗｰ発生 エラーコード出力
		case 98:
			// 計測方向がZ方向のとき 100加算
			if(SEQ.MEASUREMENT_DIRECTION == Z_DIRECTION){
				if(error_no != 0){
					COM0.NO312 += 100;
				}else{
					COM0.NO312 = error_no;
				}
				error_no = 0;
			}else{
				error_no = COM0.NO312;
			}
// add 2016.10.19 K.Uemura start	GA1901
//			// Xセンサのエラー時、Zセンサを継続実施させる
//			if(SEQ.MEASUREMENT_DIRECTION == X_DIRECTION){
//				SEQ.POWER_STATUS = 41;
//			}else{
// add 2016.10.19 K.Uemura end
				set_7seg_led_error_no(COM0.NO312);				// 7ｾｸﾞLEDのｴﾗｰﾅﾝﾊﾞｰ設定
				SEQ.POWER_STATUS++;
//			}
			break;

		// ｴﾗｰ発生・電源起動処理終了
		case 99:
			DA.DADR0 = 0;									// DA出力値をｾｯﾄ(最小)
			SEQ.FLAG2.BIT.BLACK_WHITE_COMPLETE = 1;			// BLACKﾁｪｯｸ・WHITEﾁｪｯｸ完了ﾌﾗｸﾞ
			
			SEQ.FLAG.BIT.CNC_INITIAL = 0;					// CNC電源起動時初期化ﾌﾗｸﾞ
			
			SEQ.CHANGE_FPGA = 0;
			SEQ.POWER_STATUS = 1;
			
			SEQ.FLAG.BIT.MEMORY_CONTROL = 0;				// 7ｾｸﾞを点灯
			COM0.SUB_STATUS = 1;
			
			if(COM0.MASTER_STATUS == RESET_MODE){
				COM0.MASTER_STATUS = INIT_START_MODE;
			}else{
				COM0.MASTER_STATUS = DRV_MODE;				// ﾀｯﾁﾊﾟﾈﾙ通信を開始
			}
			
			OUT.MASTER_STATUS = DRV_MODE;					// 通常ﾓｰﾄﾞ
			OUT.SUB_STATUS = 1;
			
			COM0.NO310.BIT.RDY = 1;							// READY ON
			
			break;

// add 2015.03.18 K.Uemura start	F31802	輝度変更後、LED点灯遅延を行う
		case 100:
			// 10msカウンタ
			SEQ.POWER_COUNT++;								// 電源制御ｶｳﾝﾄ
			if(SEQ.POWER_COUNT >= 10){						// 電源制御ｶｳﾝﾄが「10」のとき
				SEQ.POWER_STATUS = 35;
			}
			break;
// add 2015.03.18 K.Uemura end
	}
}

//************************************************************/
//				輝度調整(自動)
//************************************************************/
void light_adjustment(void)
{
	long ldv_target, ldv_range, retry_count, area_min, area_max, bright;

	//パラメータ取得
	if(IN.FLAG.BIT.HARDWARE_TYPE == CNC_EDITION){			// 計測ﾀｲﾌﾟがCNC版のとき
		if(SEQ.MEASUREMENT_DIRECTION == X_DIRECTION){		// 計測方向がX方向のとき
			ldv_target	= X_LDV_TARGET;						// LDVﾀｰｹﾞｯﾄ
			ldv_range	= X_LDV_RANGE;						// LDV有効範囲
			retry_count	= X_RETRY_COUNT;					// LDVﾘﾄﾗｲ回数
		}else{												// 計測方向がZ方向のとき
			ldv_target	= Z_LDV_TARGET;						// LDVﾀｰｹﾞｯﾄ(Z)
			ldv_range	= Z_LDV_RANGE;						// LDV有効範囲(Z)
			retry_count	= Z_RETRY_COUNT;					// LDVﾘﾄﾗｲ回数(Z)
		}
	}else{													// 計測ﾀｲﾌﾟがﾎﾟｰﾀﾌﾞﾙ版のとき
		if(SEQ.FLAG.BIT.ECO == 0){							// 計測ﾓｰﾄﾞがNORMALのとき
			ldv_target	= NORMAL_LDV_TARGET;				// LDVﾀｰｹﾞｯﾄ
			ldv_range	= NORMAL_LDV_RANGE;					// LDV有効範囲
			retry_count	= NORMAL_RETRY_COUNT;				// LDVﾘﾄﾗｲ回数
		}else{												// 計測ﾓｰﾄﾞがECOのとき
			ldv_target	= ECO_LDV_TARGET;					// LDVﾀｰｹﾞｯﾄ(ECO)
			ldv_range	= ECO_LDV_RANGE;					// LDV有効範囲(ECO)
			retry_count	= ECO_RETRY_COUNT;					// LDVﾘﾄﾗｲ回数(ECO)
		}
	}

	area_min = ldv_target - ldv_range;
	area_max = ldv_target + ldv_range;

#ifdef	__INITIAL_SEQUENCE
	DEBUG_STR.DEBUG[100+(SEQ.DIM_RETRY_COUNT*2)+0] = SEQ.DIM_LEVEL;				// 輝度値
	DEBUG_STR.DEBUG[100+(SEQ.DIM_RETRY_COUNT*2)+1] = SEQ.BLACK_WHITE_CHECK[1];	// センサ出力(デジタル値)
#endif

	//リトライ回数の確認
	if(SEQ.DIM_RETRY_COUNT >= retry_count){			// 調光ﾘﾄﾗｲｶｳﾝﾄが設定値に達したら
#ifdef	__INITIAL_SEQUENCE
		// デバッグ出力フラグ
		SEQ.FLAG5.BIT.DEB_SIOOUT_FLAG = 1;
#endif
		SEQ.POWER_STATUS = 98;							// ｴﾗｰ
		COM0.NO312 = ERR_BRIGHTNESS_NG;					// 1013	調光値NG
	}else{
		//範囲内にあるときは終了
		if((area_min <= SEQ.BLACK_WHITE_CHECK[1]) && (SEQ.BLACK_WHITE_CHECK[1] <= area_max)){		//範囲内にあるとき
			// 調光終了
			SEQ.LED_BRIGHTNESS = SEQ.DIM_LEVEL;
			if( ((IN.FLAG.BIT.HARDWARE_TYPE == CNC_EDITION) && (SEQ.MEASUREMENT_DIRECTION == X_DIRECTION)) || 
				((IN.FLAG.BIT.HARDWARE_TYPE == PORTABLE_EDITION))){	// 170203
				// CNC版：計測方向がXのとき
				// PBL版：露光時間Normalのとき & Ecoのとき170203
				SEQ.X_LED_BRIGHTNESS = SEQ.LED_BRIGHTNESS;				// LED輝度値を設定
// add 2016.03.08 K.Uemura start	G30702
				SEQ.X_LED_BRIGHTNESS_WHITE = SEQ.BLACK_WHITE_CHECK[1];	// LED輝度 ﾎﾜｲﾄ平均値(X)
// add 2016.03.08 K.Uemura end
			}else{
				SEQ.Z_LED_BRIGHTNESS = SEQ.LED_BRIGHTNESS;				// LED輝度値を設定
// add 2016.03.08 K.Uemura start	G30702
				SEQ.Z_LED_BRIGHTNESS_WHITE = SEQ.BLACK_WHITE_CHECK[1];	// LED輝度 ﾎﾜｲﾄ平均値(Z)
// add 2016.03.08 K.Uemura end
			}

			SEQ.CHECK_RETRY_COUNT = 0;									// BLACKﾁｪｯｸ・WHITEﾁｪｯｸ再送ｶｳﾝﾀ
			SEQ.POWER_STATUS = 39;
#ifdef	__INITIAL_SEQUENCE
			// デバッグ出力フラグ
			SEQ.FLAG5.BIT.DEB_SIOOUT_FLAG = 1;
#endif
		}else{
			//下限より小さい場合、輝度Up
			if(SEQ.BLACK_WHITE_CHECK[1] < area_min){
				if(SEQ.DIM_LEVEL == 1023){
					//輝度が「1023」の場合は上げられないのでエラー
					COM0.NO312 = ERR_BRIGHTNESS_UPPER;				// 1014	調光値NG 上限到達(輝度上げれない)
				}else{
					//輝度調整
					bright = SEQ.DIM_LEVEL + (ldv_target - SEQ.BLACK_WHITE_CHECK[1]);
					//1023より大きくなる場合は「1023」設定
					if(1023 <= bright){
						bright = 1023;
					}
				}
			}

			if(area_max < SEQ.BLACK_WHITE_CHECK[1]){
				if(SEQ.DIM_LEVEL == 0){
					//輝度が「0」の場合は下げられないのでエラー
					COM0.NO312 = ERR_BRIGHTNESS_LOWER;				// 1015	調光値NG 下限到達(輝度下けれない)
				}else{
					//輝度調整
					bright = SEQ.DIM_LEVEL - (SEQ.BLACK_WHITE_CHECK[1] - ldv_target);
					//0より小さくなる場合は「0」設定
					if(bright < 0){
						bright = 0;
					}
				}
			}

			if(COM0.NO312 == 0){
				SEQ.DIM_LEVEL = bright;
				DA.DADR0 = bright;
				SEQ.DIM_RETRY_COUNT++;						// 調光ﾘﾄﾗｲｶｳﾝﾄ
// chg 2016.03.07 K.Uemura start	G30701
				SEQ.POWER_STATUS = 35;						// waitなし
//				SEQ.POWER_STATUS = 100;						// wait 
// chg 2016.03.07 K.Uemura end
			}else{
				SEQ.POWER_STATUS = 98;						// ｴﾗｰ
			}
		}
	}
}

//************************************************************/
//				輝度調整(手動)
//************************************************************/
void light_adjustment_manual(void)
{
#ifdef	__INITIAL_SEQUENCE
	DEBUG_STR.DEBUG[100+(SEQ.DIM_RETRY_COUNT*2)+0] = SEQ.DIM_LEVEL;				// 輝度値
	DEBUG_STR.DEBUG[100+(SEQ.DIM_RETRY_COUNT*2)+1] = SEQ.BLACK_WHITE_CHECK[1];	// センサ出力(デジタル値)
#endif

	// 調光終了
	SEQ.LED_BRIGHTNESS = SEQ.DIM_LEVEL;
	if( ((IN.FLAG.BIT.HARDWARE_TYPE == CNC_EDITION) && (SEQ.MEASUREMENT_DIRECTION == X_DIRECTION)) || 
		((IN.FLAG.BIT.HARDWARE_TYPE == PORTABLE_EDITION))){		// 170203
		// CNC版：計測方向がXのとき
		// PBL版：露光時間Normalのとき & Ecoのとき170203
		SEQ.X_LED_BRIGHTNESS = SEQ.LED_BRIGHTNESS;				// LED輝度値を設定
		SEQ.X_LED_BRIGHTNESS_WHITE = SEQ.BLACK_WHITE_CHECK[1];	// LED輝度 ﾎﾜｲﾄ平均値(X)
	}else{
		SEQ.Z_LED_BRIGHTNESS = SEQ.LED_BRIGHTNESS;				// LED輝度値を設定
		SEQ.Z_LED_BRIGHTNESS_WHITE = SEQ.BLACK_WHITE_CHECK[1];	// LED輝度 ﾎﾜｲﾄ平均値(Z)
	}

	SEQ.CHECK_RETRY_COUNT = 0;									// BLACKﾁｪｯｸ・WHITEﾁｪｯｸ再送ｶｳﾝﾀ
	SEQ.POWER_STATUS = 39;
	
#ifdef	__INITIAL_SEQUENCE
	// デバッグ出力フラグ
	SEQ.FLAG5.BIT.DEB_SIOOUT_FLAG = 1;
#endif
}