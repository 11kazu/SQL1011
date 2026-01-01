/******************************************************************************
* File Name	: ad_conversion.c
******************************************************************************/
#include <machine.h>
#include "iodefine.h"
#include "user_define.h"
#include "typedefine.h"
#include "user.h"

void adc_init(void);				// AD初期設定
void ctl_ad_func(void);
void drv_ad_start(void);
void disp_voltage_error(void);

/************************************************************/
//				AD初期設定
/************************************************************/
void adc_init( void )
{
	IPR(AD0, ADI0) = 1;

	MSTP(AD0) = 0;
	
	AD0.ADCR.BYTE = 0x00;
	AD0.ADCSR.BYTE = 0x00;
	
	IEN(AD0, ADI0) = 1;
}

//************************************************************/
//				ｱﾅﾛｸﾞﾃﾞｰﾀ取得制御関数
//************************************************************/
void ctl_ad_func(void)
{
	_UBYTE i;
	unsigned short diff;

	switch(ADCOV.STATUS){
		case 1:
			//ﾊﾟﾗﾒｰﾀを初期化してAD変換をｽﾀｰﾄ
			ADCOV.COUNT			= 1;
			ADCOV.STATUS		= 2;
			ADCOV.INPUT			= 1;
			drv_ad_start();
			break;
			
		case 2:
			//AD変換が終了していたら以下の処理
			if(ADCOV.FLAG.BIT.FIN){
				ADCOV.FLAG.BIT.FIN		= 0;
				// 18回ﾃﾞｰﾀを取得して最大値・最小値を削除して平均化
				if(ADCOV.COUNT >= 19){
					//規定回数ﾃﾞｰﾀを取得していればﾃﾞｰﾀ平均化
					ADCOV.ADIN0[0] = 0;		// 電源電圧計測
					
					for(i=1;i<19;i++){
						// 取得値を合算する
						ADCOV.ADIN0[0]	+= ADCOV.ADIN0[i];
						
						if(i == 1){
							ADCOV.ADIN0MAX = ADCOV.ADIN0MIN = ADCOV.ADIN0[i];
						}
						// 最大値・最小値を格納する
						if(ADCOV.ADIN0MAX < ADCOV.ADIN0[i]){
							ADCOV.ADIN0MAX = ADCOV.ADIN0[i];
						}
						else if(ADCOV.ADIN0MIN > ADCOV.ADIN0[i]){
							ADCOV.ADIN0MIN = ADCOV.ADIN0[i];
						}
						
					}
					// 取得値の合計から最大値・最小値を取り除く
					ADCOV.ADIN0[0] = ADCOV.ADIN0[0] - ADCOV.ADIN0MAX - ADCOV.ADIN0MIN;
					//　平均値(ﾃﾞｰﾀ16個分の平均)
					ADCOV.ADIN0[0]	>>= 4;
					
					// 平均値を電源電圧値とする
					ADCOV.V_BATT = ADCOV.ADIN0[0];				// 電源電圧
					COM0.NO130 = ADCOV.V_BATT;
					
					ADCOV.STATUS		= 3;
					ADCOV.INTERVAL		= 0;
					
					// 起動時電源判別
					//IN.FLAG.BIT.EXT_POWER = EXT_POWER_IN;	// 0:外部電源	1:ﾊﾞｯﾃﾘｰ

					// 電池の残量算出
					// ●表示
					//   9.0V-7.4Vを100%-0%
					// ●動作
					//   7.6V以下を残量警告、7.0V以下を計測不可(バッテリーアラーム)
					// 9.0Vのとき	543(実測値)
					// 7.6Vのとき	456(実測値)
					// 7.0Vのとき	418(実測値)
					diff = BATTERY_MAX - BATTERY_MIN_DISP;
					if(ADCOV.V_BATT >= BATTERY_MAX){
						SEQ.BATTERY_POWER = 100;
					}else if(ADCOV.V_BATT <= BATTERY_MIN_DISP){
						SEQ.BATTERY_POWER = 0;
					}else{
						SEQ.BATTERY_POWER = (ADCOV.V_BATT-BATTERY_MIN_DISP)*100 / diff;		// 電池残量(%)
					}

// add 2015.03.23 K.Uemura start	F32302
					// 待機中の場合、BATTERY_MIN_DISP(443@7.4V)を下回るとエラー出力
					if(SEQ.FLAG.BIT.POWER == 0){
						if(SEQ.BATTERY_POWER == 0){
							// エラーフラグをON
							SEQ.FLAG5.BIT.VOLTAGE_ERROR = 1;
						}
					}else{
// add 2015.03.23 K.Uemura end
						// 計測中の場合、BATTERY_ERROR(418@7.0V)を下回るとエラー出力
						// 7.0V未満の場合、計測させない
						if(ADCOV.V_BATT < BATTERY_ERROR){
							// エラーフラグをON
							SEQ.FLAG5.BIT.VOLTAGE_ERROR = 1;

							// 計測中の場合は停止させる
							SEQ.FLAG.BIT.MEASUREMENT = MEASURE_STOP;
						}
					}

					// バッテリーエラー表示
					disp_voltage_error();

					// 7.6V未満の場合、電池残量少(警告)
					if(ADCOV.V_BATT < BATTERY_WARNING){
						SEQ.FLAG5.BIT.LOW_BATTERY = 1;
					}else{
						SEQ.FLAG5.BIT.LOW_BATTERY = 0;
					}
				}else{
					//規定回数に到達していなければ次の計測
					drv_ad_start();
				}
			}
			break;
			
		case 3://ﾃﾞｰﾀ平均化が終了したら次の計測までｲﾝﾀｰﾊﾞﾙを置く(1s置き)
			ADCOV.INTERVAL++;
			if(ADCOV.INTERVAL >= 81){
				ADCOV.STATUS		= 1;
			}
			break;
	}
}

//************************************************************/
//				AD変換開始関数					
//************************************************************/
void drv_ad_start(void)
{
	AD0.ADCSR.BIT.ADST	= 1;	//A/D変換開始
	AD0.ADCSR.BIT.ADIE	= 1;	//完了割込許可
}

//************************************************************/
//				電圧エラー表示
//************************************************************/
void disp_voltage_error(void)
{
	if(SEQ.FLAG5.BIT.VOLTAGE_ERROR == 1){
		COM0.NO312 = ERR_UNDERVOLTAGE;			// 100	電圧不足
		set_7seg_led_error_no(COM0.NO312);		// 7ｾｸﾞLEDのｴﾗｰﾅﾝﾊﾞｰ設定
		set_7seg_lower_no_data();				// 7ｾｸﾞ表示(-----)(下段)
		
		// 強制的に電源OFFの状態とする
		SEQ.FLAG.BIT.POWER = 0;
	}
}

