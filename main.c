/******************************************************************************
* File Name	: main.c
******************************************************************************/
#include <machine.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include <mathf.h>
#include "iodefine.h"
#include "user_define.h"
#include "typedefine.h"
#define DEFINE_USER_GLOBALS
#include "user.h"

void main(void);

void peakhold_process(void);		// ﾋﾟｰｸﾎｰﾙﾄﾞ処理

void polling_process(void);			// ﾎﾟｰﾘﾝｸﾞ処理
void timeout_setting(void);			// ﾀｲﾑｱｳﾄ設定
void memory_time_setting(void);		// 記憶時間設定
void strobe_output_stop(void);		// STROBE出力停止
void initial_sequence(void);		// 起動処理

void start_process_timeout(void);	// 起動処理のﾀｲﾑｱｳﾄ設定
void fpga_control(void);			// FPGA制御
void hdi_count(void);				// HDI出力保持用ｶｳﾝﾄ
void start_delay_time_count(void);	// 計測開始遅延時間ｶｳﾝﾄ
// add 2015.12.22 K.Uemura start	FB2201
void cover_process(void);
// add 2015.12.22 K.Uemura end

#ifdef __cplusplus
//#include <ios>                        // Remove the comment when you use ios
//_SINT ios_base::Init::init_cnt;       // Remove the comment when you use ios
#endif

void main(void);
#ifdef __cplusplus
extern "C" {
void abort(void);
}
#endif

void main(void)
{
ldiv_t	tsDiv;
	system_init();				// ｼｽﾃﾑ初期設定
	port_init();				// ﾎﾟｰﾄ初期設定
	tmr_init();					// ﾀｲﾏｰ初期設定
	sci0_init();				// SCI初期設定
	
	sci2_init();				// SCI初期設定
	sci3_init();				// SCI初期設定
	sci6_init();				// SCI初期設定
	
	adc_init();					// AD初期設定
	
#ifdef	OUTPUT232C
	COM2.MASTER_STATUS = DRV_MODE;
	COM2.SUB_STATUS = 1;
#ifdef	__DEBUG
	COM2.MASTER_STATUS = DEBUG_MODE;
	COM2.SUB_STATUS = 1;
#endif
#endif

	SEQ.FLAG5.BIT.BOOTUP_PROCESS = 0;

	I2C.SUB_STATUS = 1;
	COM0.SUB_STATUS = 1;
	
	SEQ.LAST_CBUS_NUMBER = 215;						// CBUSﾅﾝﾊﾞｰ(1ｻｲｸﾙ最終)
	
	COM0.NO310.BIT.POW = 1;							// POWER ON
	COM0.NO333.BIT.POW = 1;							// POWER ON
	
	// 起動時電源判別
	IN.FLAG.BIT.EXT_POWER = EXT_POWER_IN;			// 0:外部電源	1:ﾊﾞｯﾃﾘｰ
	COM0.NO131 = IN.FLAG.BIT.EXT_POWER;
	
	// 起動時CNC/ﾎﾟｰﾀﾌﾞﾙ判別
	IN.FLAG.BIT.HARDWARE_TYPE = CNC_CONTROL_IN;		// 0:ﾎﾟｰﾀﾌﾞﾙ	1:CNC
	
	ADCOV.STATUS = 1;
	SEQ.TP_CONTROL_STATUS = 1;						// ﾀｯﾁﾊﾟﾈﾙ制御ｽﾃｰﾀｽ
	SEQ.POWER_STATUS = 1;							// 電源ｽｲｯﾁｽﾃｰﾀｽ
	//
	
	SEQ.SELECT.BIT.PEAKHOLD = PEAK_HOLD_OFF;
	
	// 7ｾｸﾞ・LED初期化
	led_off_state();
	OUT.MASTER_STATUS = DRV_MODE;					// 通常ﾓｰﾄﾞ
	
	OUT.LED_STATUS = 1;	//
	
	SEQ.MASTER_COUNT = 0;
	
	SEQ.PROFILE_BUFFER_COUNT = 0;					// ﾌﾟﾛﾌｧｲﾙﾊﾞｯﾌｧ格納用ｶｳﾝﾄ
	
	CFG_PROG_OUT = 1;								// FPGAを運転するため
	M25_CS_OUT = 1;									// M25_CS
	
	COM6.START_ADDRESS = 0;
	COM6.SUB_STATUS = 11;
	
	SEQ.STARTUP_COUNT = 0;							// 起動待ち時間
// add 2016.07.26 K.Uemura start	G72601
	SEQ.MSEC_COUNTER = 0;							// msecｶｳﾝﾀ
	memset(&SEQ.MSEC_BUFFER, 0, sizeof(SEQ.MSEC_BUFFER));
// add 2016.07.26 K.Uemura end
	
	// 電圧安定化待ち
	// 0.5s待機
	while (SEQ.STARTUP_COUNT < 50)
	{
		// 10msごとの処理
		// 入力処理
		if(SEQ.FLAG1.BIT.I_SHOT){
			SEQ.FLAG1.BIT.I_SHOT = 0;				// 入力関数処理
			SEQ.STARTUP_COUNT++;					// 起動待ち時間
		}
	}
	
	SEQ.STARTUP_COUNT = 0;							// 起動待ち時間
	//
	
	// 電圧測定
	while (ADCOV.INTERVAL == 0)						// 電源ONﾌﾗｸﾞをｾｯﾄ
	{
		// 10msごとの処理
		if(SEQ.FLAG1.BIT.A_SHOT){
			SEQ.FLAG1.BIT.A_SHOT = 0;
			ctl_ad_func();							// ｱﾅﾛｸﾞ入力処理
		}
	}
	//
	
	// 電圧判定
	if(SEQ.FLAG5.BIT.VOLTAGE_ERROR == 1){			// 電圧ｴﾗｰﾌﾗｸﾞがONのとき
		SEQ.FLAG3.BIT.BLINK_LED = 1;				// LED点滅ﾌﾗｸﾞ
		while (1)
		{
			// 100usごとの処理
			// 出力処理
			if(SEQ.FLAG1.BIT.O_SHOT){
				SEQ.FLAG1.BIT.O_SHOT = 0;			// 出力関数処理
				out_master();
			}
		}
	}
	
	ctl_uni_timer4(3000);							// FPGAﾀｲﾑｱｳﾄ 30s
	
	// FPGAにｺﾝﾌｨｸﾞﾚｰｼｮﾝ処理(約10秒＠2017.01.19)
	while (SEQ.FLAG.BIT.POWER_ON != 1)				// 電源ONﾌﾗｸﾞをｾｯﾄ
	{
		// 100usごとの処理
		// 出力処理
		if(SEQ.FLAG1.BIT.O_SHOT){
			SEQ.FLAG1.BIT.O_SHOT = 0;				// 出力関数処理
			out_master();
		}
		
		// 100usごとの処理
		if(SEQ.FLAG1.BIT.M25_SHOT){
			SEQ.FLAG1.BIT.M25_SHOT = 0;				// M25関数処理
			m25_init();
		}
		
		// 10msごとの処理
		if(SEQ.FLAG1.BIT.A_SHOT){
			SEQ.FLAG1.BIT.A_SHOT = 0;
			ctl_ad_func();							// ｱﾅﾛｸﾞ入力処理
		}
		
		if(TIM1.MSEC_10.BIT.UNI_UP4	== 1){
			TIM1.MSEC_10.BIT.UNI_UP4 = 0;
			SEQ.FLAG3.BIT.BLINK_LED = 1;			// LED点滅ﾌﾗｸﾞ
			break;
		}
	}
	
	// FPGAｿﾌﾄｳｪｱﾊﾞｰｼﾞｮﾝ取得
	SEQ.CBUS_NUMBER = 386;							// ﾌﾞﾗｯｸ固定値設定
	SEQ.CHANGE_FPGA = 11;
	SEQ.FPGA_SEND_STATUS = 1;
	//
	
	// 1s待機
	while (SEQ.STARTUP_COUNT < 100)
	{
		// 10usごとの処理
		// FPGA処理
		if(SEQ.FLAG1.BIT.F_SHOT){
			SEQ.FLAG1.BIT.F_SHOT = 0;
			fpga_control();					// FPGA制御
		}
		
		if(SEQ.FLAG1.BIT.R_SHOT){
			SEQ.FLAG1.BIT.R_SHOT = 0;
			i2c_master();					// ROM処理
		}
		
		// 100usごとの処理
		// 出力処理
		if(SEQ.FLAG1.BIT.O_SHOT){
			SEQ.FLAG1.BIT.O_SHOT = 0;		// 出力関数処理
			LED.FOCUS.BYTE = 0x00;
			out_master();
		}
			
		// 10msごとの処理
		// 入力処理
		if(SEQ.FLAG1.BIT.I_SHOT){
			SEQ.FLAG1.BIT.I_SHOT = 0;		// 入力関数処理
			SEQ.STARTUP_COUNT++;			// 起動待ち時間
		}
	}
	
	SEQ.STARTUP_COUNT = 0;					// 起動待ち時間
	
	// ﾊﾟﾗﾒｰﾀ転送 ADD150917
	COM0.SUB_STATUS = 1;
	COM0.MASTER_STATUS = INIT_START_MODE;
	
	ctl_uni_timer5(300);					// ﾊﾟﾗﾒｰﾀ転送ﾀｲﾑｱｳﾄ 3s
	
	while (COM0.MASTER_STATUS != DRV_MODE)
	{
		// 1msごとの処理
		if(SEQ.FLAG1.BIT.MSEC_SHOT){
			SEQ.FLAG1.BIT.MSEC_SHOT = 0;	// 1msﾌﾗｸﾞ(1msごとに処理を行う)
			polling_process();				// ﾎﾟｰﾘﾝｸﾞ処理
		}
		
		// 100usごとの処理
		// ﾀｯﾁﾊﾟﾈﾙ処理
		if(SEQ.FLAG1.BIT.TP_SHOT){
			SEQ.FLAG1.BIT.TP_SHOT = 0;
			rs422_master();					// ﾒｲﾝｼｰｹﾝｽ関数
		
			if(TIM1.MSEC_10.BIT.UNI_UP5	== 1){
				TIM1.MSEC_10.BIT.UNI_UP5 = 0;
				break;
			}
		}
	}
	//
	
	// ﾎﾟｰﾀﾌﾞﾙ版で起動したとき、ﾊﾞｰｼﾞｮﾝ表示しない
	if(IN.FLAG.BIT.HARDWARE_TYPE != PORTABLE_EDITION){

// add 2015.05.13 K.Uemura start	
		// 表示フラグON
		SEQ.FLAG5.BIT.DISP_SOFTVERSION = 1;
// add 2015.05.13 K.Uemura end

		disp_software_version();			// ｿﾌﾄｳｪｱﾊﾞｰｼﾞｮﾝ表示
		
		// ﾊﾞｰｼﾞｮﾝ表示(3s)
		while (SEQ.STARTUP_COUNT < 300)
		{
			// 100usごとの処理
			// 出力処理
			if(SEQ.FLAG1.BIT.O_SHOT){
				SEQ.FLAG1.BIT.O_SHOT = 0;	// 出力関数処理
				out_master();
			}
				
			// 10msごとの処理
			// 入力処理
			if(SEQ.FLAG1.BIT.I_SHOT){
				SEQ.FLAG1.BIT.I_SHOT = 0;	// 入力関数処理
				SEQ.STARTUP_COUNT++;		// 起動待ち時間
			}
		}

// add 2015.05.13 K.Uemura start	
		// 表示フラグOFF
		SEQ.FLAG5.BIT.DISP_SOFTVERSION = 0;
// add 2015.05.13 K.Uemura end
	}
	
	// ﾎﾟｰﾀﾌﾞﾙ版で起動したとき、自動で電源ONの状態にしない
	if(IN.FLAG.BIT.HARDWARE_TYPE == PORTABLE_EDITION){
		SEQ.FLAG.BIT.POWER_ON = 0;
		SEQ.FLAG.BIT.POWER = 0;
		COM0.SUB_STATUS = 1;
		COM0.MASTER_STATUS = INIT_START_MODE;
		OUT.MASTER_STATUS = IDLE_MODE;		// 待機ﾓｰﾄﾞ

		// 電池動作中は、ECOモードで起動させる
		if(IN.FLAG.BIT.EXT_POWER == POWER_SUPPLY_BATTERY){
			SEQ.FLAG.BIT.ECO = 1;
		}
	}
	
	SEQ.FLAG5.BIT.BOOTUP_PROCESS = 1;

	// デバッグ用初期化
	SEQ.DEB_RESET_TIME = 0;
	SEQ.FLAG5.BIT.DEB_RESET_FLAG = 0;
	
	while (1)
	{
		// debug
		COM0.NO141 = SEQ.CHANGE_FPGA;
		COM0.NO142 = SEQ.FPGA_SEND_STATUS;
		COM0.NO143 = SEQ.TP_CONTROL_STATUS;		// tp_control.c
		COM0.NO144 = COM0.SUB_STATUS;			// rs422_sci0.c
		COM0.NO145 = COM2.SUB_STATUS;			// rs232_sci2.c

		// 10usごとの処理
		// FPGA処理
		if(SEQ.FLAG1.BIT.F_SHOT){
			SEQ.FLAG1.BIT.F_SHOT = 0;
			fpga_control();					// FPGA制御
		}
		
		// 100usごとの処理
		// 出力処理
		if(SEQ.FLAG1.BIT.HDI_SHOT){
			SEQ.FLAG1.BIT.HDI_SHOT = 0;		// HDI関数処理(100usごとに処理を行う)
			hdi_output();					// HDI出力
		}
		
		// 100usごとの処理
		else if(SEQ.FLAG1.BIT.O_SHOT){
			SEQ.FLAG1.BIT.O_SHOT = 0;		// 出力関数処理
			out_master();
		}
		
		// 100usごとの処理
		else if(SEQ.FLAG1.BIT.M25_SHOT){
			SEQ.FLAG1.BIT.M25_SHOT = 0;		// M25関数処理(100usごとに処理を行う)
			//m25_init();
		}
			
		// 100usごとの処理
		// ﾀｯﾁﾊﾟﾈﾙ処理
		else if(SEQ.FLAG1.BIT.TP_SHOT){
			SEQ.FLAG1.BIT.TP_SHOT = 0;
			rs422_master();					// ﾒｲﾝｼｰｹﾝｽ関数
		}
		
		// 1msごとの処理
		if(SEQ.FLAG1.BIT.MSEC_SHOT){
			SEQ.FLAG1.BIT.MSEC_SHOT = 0;	// 1msﾌﾗｸﾞ(1msごとに処理を行う)
// add 2016.07.26 K.Uemura start	G72601
			SEQ.MSEC_COUNTER++;				// 1msｶｳﾝﾀ
			tsDiv = ldiv(SEQ.MSEC_COUNTER, 10000);
			if(tsDiv.rem == 0){
#ifdef	DEBUG_EACHSCAN
				DEBUG_STR.DEBUG_EDGE[0]++;
#endif
			}
// add 2016.07.26 K.Uemura end
			polling_process();				// ﾎﾟｰﾘﾝｸﾞ処理
			timeout_setting();				// ﾀｲﾑｱｳﾄ設定
			memory_time_setting();			// 記憶時間設定
			strobe_output_stop();			// STROBE出力停止
// chg 2015.09.02 K.Uemura start	
			initial_sequence();				// 起動処理
// chg 2015.09.02 K.Uemura end
// add 2015.12.22 K.Uemura start	FB2201
			cover_process();				// ｶﾊﾞｰ動作
// add 2015.12.22 K.Uemura end
			hdi_count();					// HDI出力保持用ｶｳﾝﾄ
			tp_control();
			start_process_timeout();		// 起動処理のﾀｲﾑｱｳﾄ設定
			peakhold_process();				// ﾋﾟｰｸﾎｰﾙﾄﾞ処理
		}
		
		// 10msごとの処理
		// 入力処理
		if(SEQ.FLAG1.BIT.I_SHOT){
			SEQ.FLAG1.BIT.I_SHOT = 0;		// 入力関数処理
//			IN.FLAG.BIT.HARDWARE_TYPE = CNC_CONTROL_IN;		// 0:ﾎﾟｰﾀﾌﾞﾙ	1:CNC
			sw_input();
			power_sequence();				// POWERｼｰｹﾝｽ
		}
		
		// 10msごとの処理
		if(SEQ.FLAG1.BIT.R_SHOT){
			SEQ.FLAG1.BIT.R_SHOT = 0;
			i2c_master();					// ROM処理
		}
		
		// 10msごとの処理
		if(SEQ.FLAG1.BIT.A_SHOT){
			SEQ.FLAG1.BIT.A_SHOT = 0;
			ctl_ad_func();					// ｱﾅﾛｸﾞ入力処理
			start_delay_time_count();		// 計測開始遅延時間ｶｳﾝﾄ
		}
		
		// 10msごとの処理
		if(SEQ.FLAG1.BIT.DEB_SHOT){
			SEQ.FLAG1.BIT.DEB_SHOT = 0;
			rs232c_master();				// ﾒｲﾝｼｰｹﾝｽ関数
		}
	}
}

//************************************************************/
//				ﾋﾟｰｸﾎｰﾙﾄﾞ処理
//************************************************************/
// 1msごとの処理
void peakhold_process(void)
{
	if(SEQ.FLAG3.BIT.PEAKHOLD_ENABLE){					// ﾋﾟｰｸﾎｰﾙﾄﾞ有効ﾌﾗｸﾞ
		// 0:d≦4・10:d＞4・2:振れのとき
		if((SEQ.SELECT.BIT.MEASURE == MODE_D4_LOW)||(SEQ.SELECT.BIT.MEASURE == MODE_D4_AUTO)||(SEQ.SELECT.BIT.MEASURE == MODE_RUNOUT)){
			if(SEQ.SELECT.BIT.PEAKHOLD == PEAK_HOLD_5S){
				SEQ.PEAKHOLD_COUNT++;					// ﾋﾟｰｸﾎｰﾙﾄﾞｶｳﾝﾀ
				if(SEQ.PEAKHOLD_COUNT >= 5000){			// 5s
					SEQ.PEAKHOLD_COUNT = 0;
					SEQ.FLAG3.BIT.PEAKHOLD_RESET = 1;	// ﾋﾟｰｸﾎｰﾙﾄﾞﾘｾｯﾄﾌﾗｸﾞ
				}
			}else if(SEQ.SELECT.BIT.PEAKHOLD == PEAK_HOLD_10S){
				SEQ.PEAKHOLD_COUNT++;					// ﾋﾟｰｸﾎｰﾙﾄﾞｶｳﾝﾀ
				if(SEQ.PEAKHOLD_COUNT >= 10000){		// 10s
					SEQ.PEAKHOLD_COUNT = 0;
					SEQ.FLAG3.BIT.PEAKHOLD_RESET = 1;	// ﾋﾟｰｸﾎｰﾙﾄﾞﾘｾｯﾄﾌﾗｸﾞ
				}
			}else{
				SEQ.PEAKHOLD_COUNT = 0;					// ﾋﾟｰｸﾎｰﾙﾄﾞｶｳﾝﾀ
			}
		}else{
			SEQ.PEAKHOLD_COUNT = 0;						// ﾋﾟｰｸﾎｰﾙﾄﾞｶｳﾝﾀ
		}
	}else{
		SEQ.PEAKHOLD_COUNT = 0;							// ﾋﾟｰｸﾎｰﾙﾄﾞｶｳﾝﾀ
	}
}

//************************************************************/
//				ﾎﾟｰﾘﾝｸﾞ処理
//************************************************************/
// 1msごとの処理
void polling_process(void)
{
	SEQ.POLLING_COUNT++;						// ﾎﾟｰﾘﾝｸﾞ周期ｶｳﾝﾄ
	
	if(SEQ.POLLING_COUNT >= POLLING_CYCLE){		// ﾎﾟｰﾘﾝｸﾞ周期ｶｳﾝﾄがﾎﾟｰﾘﾝｸﾞ周期に一致したとき
		// ﾀｯﾁﾊﾟﾈﾙとの通信がﾀｲﾑｱｳﾄしたときｼｰｹﾝｽをﾘｾｯﾄする
		if(TIM1.MSEC_10.BIT.UNI_UP2){
			TIM1.MSEC_10.BIT.UNI_UP2 = 0;
			COM0.SUB_STATUS = 1;
		}
		
		if(COM0.SUB_STATUS == 201){				// 100msに1回ﾀｯﾁﾊﾟﾈﾙに対してのﾎﾟｰﾘﾝｸﾞ
			COM0.SUB_STATUS = 1;
		}
		SEQ.POLLING_COUNT = 0;
		SEQ.FLAG.BIT.BUFFER_RESET = 1;			// ﾊﾞｯﾌｧﾘｾｯﾄﾌﾗｸﾞ
		
		SEQ.READY_PULSE_TIME = 0;				// READYのﾊﾟﾙｽ時間
	}
}
	
//************************************************************/
//				ﾀｲﾑｱｳﾄ設定
//************************************************************/
// 1msごとの処理
void timeout_setting(void)
{
	if((SEQ.FLAG.BIT.MEASUREMENT == 1)&&(COM0.NO310.BIT.RDY == 0)){		// 測定しているときでかつREADYのﾋﾞｯﾄがOFFのとき
		// 工具径(自動)・工具径(4以下)・工具径(4より上　左側)・工具径(4より上　右側)・振れ測定・ﾌﾟﾛﾌｧｲﾙ・焦点合わせ・中心位置設定のとき
		if((COM0.NO311 >= 10)&&(COM0.NO311 <= 111)&&(COM0.NO311 != 70)){
			if(SEQ.END_TIMEOUT_PERIOD_SET > 0){
				SEQ.END_TIMEOUT_PERIOD++;		// ﾀｲﾑｱｳﾄ時間(終了条件)
			}
		}
	}else{
		SEQ.END_TIMEOUT_PERIOD	= 0;			// ﾀｲﾑｱｳﾄ時間(終了条件)
	}
	
	if(SEQ.FLAG.BIT.MEASUREMENT == 1){			// 測定しているとき
		SEQ.COM_START_PERIOD++;					// 計測開始時間(ms)
		if(COM0.NO310.BIT.RDY == 0){
			SEQ.COM_TIMEOUT_PERIOD++;			// ﾀｲﾑｱｳﾄ時間(通信設定)(ms)
		}
	}else{
		SEQ.COM_START_PERIOD = 0;				// 計測開始時間(ms)
		SEQ.COM_TIMEOUT_PERIOD	= 0;			// ﾀｲﾑｱｳﾄ時間(通信設定)(ms)
	}
}

//************************************************************/
//				記憶時間設定
//************************************************************/
// 1msごとの処理
void memory_time_setting(void)
{
	_UBYTE scan_mode;
	long result[RESULT_AREA_MAX];
	float work;

	if(SEQ.CHANGE_FPGA == 6){										// 測定しているとき
		switch(SEQ.SELECT.BIT.MEASURE){
			case MODE_FOCUS:						// 焦点合わせ
				if(FOCUS_MEMORY_TIME_SET > 0){
					SEQ.MEMORY_TIME++;									// 記憶時間(ms)
					if(SEQ.MEMORY_TIME >= FOCUS_MEMORY_TIME_SET){
						SEQ.MEMORY_TIME = 0;							// 記憶時間(ms)
						SEQ.FLAG2.BIT.MEMORY_RESET = 1;					// 記憶ﾘｾｯﾄﾌﾗｸﾞ
					}
				}
				break;
			
			case MODE_CENTER:						// 中心位置設定
				if(CENTER_MEMORY_TIME_SET > 0){
					SEQ.MEMORY_TIME++;									// 記憶時間(ms)
					if(SEQ.MEMORY_TIME >= CENTER_MEMORY_TIME_SET){
						SEQ.MEMORY_TIME = 0;							// 記憶時間(ms)
						SEQ.FLAG2.BIT.MEMORY_RESET = 1;					// 記憶ﾘｾｯﾄﾌﾗｸﾞ
					}
				}
				break;
			
			case MODE_RUNOUT:						// 振れ
	// chg 2015.06.29 K.Uemura start	
//				if(SEQ.TRIGGER_TIME_PERIOD >= 0){						// 周期ﾄﾘｶﾞ時間
	// chg 2015.06.29 K.Uemura end
					SEQ.MEMORY_TIME++;									// 記憶時間(ms)
					if(SEQ.MEMORY_TIME >= SEQ.TRIGGER_TIME_PERIOD){
						SEQ.MEMORY_TIME = 0;							// 記憶時間(ms)

// deb 2015.07.29 K.Uemura start	
#ifdef	DEBUG_EACHSCAN
if(DEBUG_STR.DEBUG_NO_COUNT < 20){
	DEBUG_STR.DEBUG_NO[DEBUG_STR.DEBUG_NO_COUNT++] = DEBUG_STR.DEBUG_COUNT;
}
#endif
// deb 2015.07.29 K.Uemura end

						if(SEQ.FLAG3.BIT.SWING_RESET == 1){				// 振れｶｳﾝﾄﾘｾｯﾄﾌﾗｸﾞ

							// ﾎﾟｰﾀﾌﾞﾙ動作時、ﾋﾟｰｸﾎｰﾙﾄﾞを起動
							if(SEQ.FLAG.BIT.PORTABLE == OPERATION_MANUAL){
								SEQ.FLAG3.BIT.PEAKHOLD_ENABLE = 1;		// ﾋﾟｰｸﾎｰﾙﾄﾞ有効ﾌﾗｸﾞ
							}

							// 初期化(未検出＠0x8000 ≒ 32768)
							COM0.NO3000[SEQ.SWING_BUFFER_COUNT] = 0x8000;

							// 走査方向取得
							scan_mode = get_scan_mode( SEQ.SELECT.BIT.MEASURE, SEQ.FLAG2.BIT.AUTO_MODE );
							
							if(scan_mode == TOOL_LEFT){					// 工具位置左側（右ｴｯｼﾞ）のとき
								if(SEQ.FLAG4.BIT.EDGE_R_SCAN == 1){
									// 計測方向がX方向のとき
									if(SEQ.MEASUREMENT_DIRECTION == X_DIRECTION){
										// 右エッジの最大取得(プラスが最大)
										if(RESULT.TIR[3] < RESULT.TIR[0]){
											RESULT.TIR[3] = RESULT.TIR[0];
										}

										// 右エッジの最小取得(プラスが最大)
										if(RESULT.TIR[0] != INITIAL_MIN){
											if(SEQ.FLUTES != 0){							// 刃数が「0」以外のとき
												if(RESULT.TIR[4] > RESULT.TIR[0]){
													RESULT.TIR[4] = RESULT.TIR[0];
												}
											}else{
												if(RESULT.TIR[4] > RESULT.TIR[6]){
													RESULT.TIR[4] = RESULT.TIR[6];
												}
											}
										}
									}else{
										// 右エッジの最大取得(マイナスが最大)
										if(RESULT.TIR[3] > RESULT.TIR[0]){
											RESULT.TIR[3] = RESULT.TIR[0];
										}

										// 右エッジの最小取得(プラスが最大)
										if(RESULT.TIR[0] != INITIAL_MAX){
											if(SEQ.FLUTES != 0){							// 刃数が「0」以外のとき
												if(RESULT.TIR[4] < RESULT.TIR[0]){
													RESULT.TIR[4] = RESULT.TIR[0];
												}
											}else{
												if(RESULT.TIR[4] < RESULT.TIR[6]){
													RESULT.TIR[4] = RESULT.TIR[6];
												}
											}

										}
									}
								}
								
							}else{										// 工具位置右側（左ｴｯｼﾞ）のとき
								if(SEQ.FLAG4.BIT.EDGE_L_SCAN == 1){
									// 左エッジの最大取得(0画素が最大)
									if(RESULT.TIR[3] > RESULT.TIR[0]){
										RESULT.TIR[3] = RESULT.TIR[0];
									}

									if(RESULT.TIR[0] != INITIAL_MAX){
										if(SEQ.FLUTES != 0){							// 刃数が「0」以外のとき
											if(RESULT.TIR[4] < RESULT.TIR[0]){
												RESULT.TIR[4] = RESULT.TIR[0];
											}
										}else{
											if(RESULT.TIR[4] < RESULT.TIR[6]){
												RESULT.TIR[4] = RESULT.TIR[6];
											}
										}
									}
								}
							}

							if(SEQ.FLAG3.BIT.SWING_RISE == 2){
								if(SEQ.FLUTES != 0){							// 刃数が「0」以外のとき
									if(((scan_mode == TOOL_LEFT) && (SEQ.FLAG4.BIT.EDGE_R_SCAN == 1)) ||	// 工具位置左側（右ｴｯｼﾞ）のとき
									   ((scan_mode == TOOL_RIGHT) && (SEQ.FLAG4.BIT.EDGE_L_SCAN == 1))){	// 工具位置右側（左ｴｯｼﾞ）のとき

										work = RESULT.TIR[0] * 10.0;
										if(RESULT_SIGN == 1){
											work *= -1;
										}
										COM0.NO3000[SEQ.SWING_BUFFER_COUNT] = work;
// add 2015.08.19 K.Uemura start	振れ座標出力
										if(SEQ.SWING_BUFFER_COUNT2 == 0){
											RESULT.RUNOUT_MIN = RESULT.RUNOUT_MAX = (short)COM0.NO3000[SEQ.SWING_BUFFER_COUNT];
										}else{
											RESULT.RUNOUT_MIN = min((short)COM0.NO3000[SEQ.SWING_BUFFER_COUNT], RESULT.RUNOUT_MIN);
											RESULT.RUNOUT_MAX = max(RESULT.RUNOUT_MAX, (short)COM0.NO3000[SEQ.SWING_BUFFER_COUNT]);
										}
										SEQ.SWING_BUFFER_COUNT2++;				// 振れﾊﾞｯﾌｧ格納用ｶｳﾝﾄ(検出あり)
//										COM0.NO325 = min((short)COM0.NO3000[SEQ.SWING_BUFFER_COUNT], (short)COM0.NO325);
//										COM0.NO326 = max((short)COM0.NO326, (short)COM0.NO3000[SEQ.SWING_BUFFER_COUNT]);
// add 2015.08.19 K.Uemura end
									}

									SEQ.BUFFER_NO_NEW = SEQ.SWING_BUFFER_COUNT;			// ﾊﾞｯﾌｧﾅﾝﾊﾞｰ新
									SEQ.SWING_BUFFER_COUNT++;
									if(SEQ.SWING_BUFFER_COUNT >= BUFFER_NUMBER)	SEQ.SWING_BUFFER_COUNT = 0;
								}else{											// 刃数が「0」のとき
									if(((scan_mode == TOOL_LEFT) && (SEQ.FLAG4.BIT.EDGE_R_SCAN == 1)) ||	// 工具位置左側（右ｴｯｼﾞ）のとき
									   ((scan_mode == TOOL_RIGHT) && (SEQ.FLAG4.BIT.EDGE_L_SCAN == 1))){	// 工具位置右側（左ｴｯｼﾞ）のとき

										work = RESULT.TIR[6] * 10.0;
										if(RESULT_SIGN == 1){
											work *= -1;
										}
										COM0.NO3000[SEQ.SWING_BUFFER_COUNT] = work;
										SEQ.BUFFER_NO_NEW = SEQ.SWING_BUFFER_COUNT;			// ﾊﾞｯﾌｧﾅﾝﾊﾞｰ新
										SEQ.SWING_BUFFER_COUNT++;
										
										work = RESULT.TIR[0] * 10.0;
										if(RESULT_SIGN == 1){
											work *= -1;
										}
										COM0.NO3000[SEQ.SWING_BUFFER_COUNT] = work;
											
// add 2015.08.19 K.Uemura start	振れ座標出力
										work = min((short)COM0.NO3000[SEQ.SWING_BUFFER_COUNT], (short)COM0.NO3000[SEQ.SWING_BUFFER_COUNT-1]);
										if((short)work < RESULT.RUNOUT_MIN){
											RESULT.RUNOUT_MIN = (short)work;
										}
										work = max((short)COM0.NO3000[SEQ.SWING_BUFFER_COUNT], (short)COM0.NO3000[SEQ.SWING_BUFFER_COUNT-1]);
										if(RESULT.RUNOUT_MAX < (short)work){
											RESULT.RUNOUT_MAX = (short)work;
										}
// add 2015.08.19 K.Uemura end
										SEQ.BUFFER_NO_NEW = SEQ.SWING_BUFFER_COUNT;			// ﾊﾞｯﾌｧﾅﾝﾊﾞｰ新
										SEQ.SWING_BUFFER_COUNT++;
										if(SEQ.SWING_BUFFER_COUNT >= BUFFER_NUMBER)	SEQ.SWING_BUFFER_COUNT = 0;
									}
								}

								// 振れ計測出力回数
								COM0.NO321 = (SEQ.SWING_BUFFER_COUNT) >> 16;
								COM0.NO322 = (SEQ.SWING_BUFFER_COUNT);

// add 2015.09.30 K.Uemura start	
								//TPD表示グラフのスケール調整
								work = abs(abs(RESULT.RUNOUT_MAX) - abs(RESULT.RUNOUT_MIN)) / 10.0;
								//調整単位が1以下の場合は、強制的に1を設定
								if(work < 1){
									work = 1;
								}
								COM0.NO325 = RESULT.RUNOUT_MIN - work;
								COM0.NO326 = RESULT.RUNOUT_MAX + work;
// add 2015.09.30 K.Uemura end
							}

							if(SEQ.TRIGGER_TIME_PERIOD != 0){
								COM0.NO310.BIT.STR = 1;								// STROBE
								SEQ.STROBE_COUNT = 0;								// STROBEﾊﾟﾙｽｶｳﾝﾄをﾘｾｯﾄ

								if(SEQ.FLAG.BIT.PORTABLE == OPERATION_AUTO && COM0.NO300.BIT.ECD == 0){

									// 12回の計測で終了
									if(SEQ.SWING_BUFFER_COUNT == SEQ.OK_COUNT_SET){
										result[0] = RESULT.TIR[0] * 10.0;				// 最大最小の差 (REAL)
										result[1] = RESULT.TIR[3] * 10.0;				// 最大
										result[2] = RESULT.TIR[4] * 10.0;				// 最小
										result[3] = abs(result[1] - result[2]);				// 最大最小の差

										if(RESULT_SIGN == 1){
											result[0] *= -1;
											result[1] *= -1;
											result[2] *= -1;
										}

										if(INITIAL_MIN < RESULT.TIR[0] && RESULT.TIR[0] < INITIAL_MAX){
											COM0.NO313 = result[0] >> 16;
											COM0.NO314 = result[0];
										}

										COM0.NO315 = result[1] >> 16;
										COM0.NO316 = result[1];
										COM0.NO317 = result[2] >> 16;
										COM0.NO318 = result[2];
										COM0.NO319 = result[3] >> 16;
										COM0.NO320 = result[3];
										
										// 7seg表示
										set_7seg_led_upper( result[1], SEQ.SELECT.BIT.MEASURE );
										set_7seg_led_lower( result[3], SEQ.SELECT.BIT.MEASURE );

										SEQ.CHANGE_FPGA = 9;
										
										SEQ.FLAG.BIT.MEASUREMENT = 0;
										SEQ.FLAG3.BIT.SWING_RESET = 0;
										SEQ.TP_CONTROL_STATUS++;
									}
								}

								max_min_reset();
								swing_no_tooth_reset();								// 最大値・最小値ﾘｾｯﾄ(振れ刃数0)
								
								SEQ.OK_COUNT = 0;									// OKｶｳﾝﾄ数
								SEQ.NG_COUNT = 0;									// 連続NGｶｳﾝﾄ数
							}

							if(scan_mode == TOOL_LEFT){
								RESULT.TIR[0] = INITIAL_MIN;

								// 計測方向がZ方向のとき
								if(SEQ.MEASUREMENT_DIRECTION == Z_DIRECTION){
									RESULT.TIR[0] = INITIAL_MAX;
								}
							}else{
								RESULT.TIR[0] = INITIAL_MAX;
							}
						}
					}
//				}
				break;
				
			// 伸び測定のとき	150512
			// RESULT.GROWTH[0] : 最大値(ﾊﾞｯﾌｧ出力)
			// RESULT.GROWTH[1] : 現在値
			// RESULT.GROWTH[2] : 基準値(相対値のとき)
			case MODE_GROWTH:						// 伸び測定
				if(SEQ.FLAG.BIT.GROWTH_OUTPUT > 0){						// 伸び計測出力ﾌﾗｸﾞ
					SEQ.FLAG.BIT.GROWTH_OUTPUT = 0;						// 伸び計測出力ﾌﾗｸﾞ
					
					if(RESULT.GROWTH[0] == 9999){
						// 検出結果なし
						COM0.NO3000[SEQ.SWING_BUFFER_COUNT] = 9999;
					}else{
						// 
						if(GROWTH_STARTING_POINT == 0){						// 相対値のとき
							if(RESULT.GROWTH[2] == 10000)	RESULT.GROWTH[2] = RESULT.GROWTH[0];	// 伸び測定基準値
							work = RESULT.GROWTH[0] - RESULT.GROWTH[2];									// 基準値との差
						}else{												// 測定値のとき
							work = RESULT.GROWTH[0];	
						}
						
						COM0.NO3000[SEQ.SWING_BUFFER_COUNT] = (short)work;
						
// add 2015.08.21 K.Uemura start	出力値のmax／min
						if((long)work < SEQ.GROWTH_MIN) SEQ.GROWTH_MIN = (long)work;	// 伸び計測最小
						if(SEQ.GROWTH_MAX < (long)work) SEQ.GROWTH_MAX = (long)work;	// 伸び計測最大

// chg 2015.09.30 K.Uemura start	
						//TPD表示グラフのスケール調整
						if(SEQ.GROWTH_MAX < 0){
							work = abs(SEQ.GROWTH_MIN + abs(SEQ.GROWTH_MAX)) / 10.0;
						}else{
							work = abs(SEQ.GROWTH_MAX - SEQ.GROWTH_MIN) / 10.0;
						}
						//調整単位が1以下の場合は、強制的に1を設定
						if(work < 1){
							work = 1;
						}
						COM0.NO325 = SEQ.GROWTH_MIN - work;
						COM0.NO326 = SEQ.GROWTH_MAX + work;
// chg 2015.09.30 K.Uemura end
//						COM0.NO325 = (short)SEQ.GROWTH_MIN;		// 最小
//						COM0.NO326 = (short)SEQ.GROWTH_MAX;		// 最大
// add 2015.08.21 K.Uemura end
					}

// chg 2016.04.22 K.Uemura start	G42201
					//出力データを設定(COM0.NO319-320)
					//符号設定
					if(0x7FFF < COM0.NO3000[SEQ.SWING_BUFFER_COUNT]){
						COM0.NO319 = 0xFFFF;			// -1を設定
					}else{
						COM0.NO319 = 0;
					}
					COM0.NO320 = COM0.NO3000[SEQ.SWING_BUFFER_COUNT];
// chg 2016.04.22 K.Uemura end

					RESULT.GROWTH[0] = 9999;						// 伸び測定最大値初期化(値が無い場合は「9999」とする)	150512
					SEQ.BUFFER_NO_NEW = SEQ.SWING_BUFFER_COUNT;			// ﾊﾞｯﾌｧﾅﾝﾊﾞｰ新
					SEQ.SWING_BUFFER_COUNT++;
					if(SEQ.SWING_BUFFER_COUNT >= BUFFER_NUMBER)	SEQ.SWING_BUFFER_COUNT = 0;
					
					// 伸び計測出力回数
					SEQ.GROWTH_OUTPUT_COUNT++;							// 伸び計測出力ｶｳﾝﾄ
					COM0.NO321 = (SEQ.GROWTH_OUTPUT_COUNT) >> 16;
					COM0.NO322 = (SEQ.GROWTH_OUTPUT_COUNT);
				}
				break;
		}
	}else{
		SEQ.MEMORY_TIME = 0;										// 記憶時間(ms)
		SEQ.FLAG2.BIT.MEMORY_RESET = 0;								// 記憶ﾘｾｯﾄﾌﾗｸﾞ
	}
}

//************************************************************/
//				STROBE出力停止
//************************************************************/
// 1msごとの処理
void strobe_output_stop(void)
{
	// 振れ測定にてﾊﾞｯﾌｧにﾃﾞｰﾀが格納されるたびにSTROBEを出力する
	// 振れ測定のとき
	if(SEQ.SELECT.BIT.MEASURE == MODE_RUNOUT){						// 2:振れ
		if(STROBE_ON_TIME > 0){										// STROBE信号ON時間の設定値が0より上のとき
			if(SEQ.STROBE_COUNT < STROBE_ON_TIME){
				SEQ.STROBE_COUNT++;
				if(SEQ.STROBE_COUNT >= STROBE_ON_TIME){
					SEQ.STROBE_COUNT = 0;							// STROBEﾊﾟﾙｽｶｳﾝﾄをﾘｾｯﾄ
					COM0.NO310.BIT.STR = 0;							// STROBE信号をOFF
				}
			}
		}else{
			COM0.NO310.BIT.STR = 0;									// STROBE信号をOFF
		}
	}
}

// add 2015.09.02 K.Uemura start	
// 150コマンド(初期化)の実行
void initial_sequence(void)
{
	if(SEQ.INITIAL_COUNT != 0){
		SEQ.INITIAL_COUNT++;

		// 2回目と500回目のみ動作
		if((SEQ.INITIAL_COUNT == 2) || (SEQ.INITIAL_COUNT == 500)){
			SEQ.FLAG.BIT.POWER = !SEQ.FLAG.BIT.POWER;	// 電源(状態反転)

			if(SEQ.FLAG.BIT.POWER){
				//OUT.MASTER_STATUS = OUT_DRV_MODE;		// 通常ﾓｰﾄﾞ
				//OUT.SUB_STATUS = 1;
				
				SEQ.FLAG.BIT.POWER_ON = 1;				// 電源ONﾌﾗｸﾞをｾｯﾄ

				SEQ.INITIAL_COUNT = 0;
			}else{
				OUT.MASTER_STATUS = IDLE_MODE;		// 待機ﾓｰﾄﾞ
				
				SEQ.FLAG.BIT.POWER_OFF = 1;				// 電源OFFﾌﾗｸﾞをｾｯﾄ
				SEQ.FLAG.BIT.MEASUREMENT = 0;
			}
		}
	}
}
// add 2015.09.02 K.Uemura end

// add 2015.12.22 K.Uemura start	FB2201
// ｶﾊﾞｰｾﾝｻ応答待ち
void cover_process(void)
{
	short retMode = ARGUMENT_ERROR;

	if(SEQ.COVER_COUNT != 0){
		SEQ.COVER_COUNT++;

		if(COVER_CONTROL == 1){
			// コントロールボックスでカバー開閉制御
			retMode = get_cover_status();

			if((COVER_DRIVE_TIME * 1000) < SEQ.COVER_COUNT){
				//タイムオーバー
				COM0.NO312 = ERR_COVER_TIMEOUT;
				SEQ.COVER_COUNT = 0;
				SEQ.TP_CONTROL_STATUS = 13;
			}

			if(retMode == COVER_STATUS_BREAK){
				//センサ故障
				COM0.NO312 = ERR_COVER_BROKEN;
				SEQ.COVER_COUNT = 0;
				SEQ.TP_CONTROL_STATUS = 13;
			}

			if(COM0.NO301 == 200){
				// ｶﾊﾞｰOpen動作
				if(retMode == COVER_STATUS_OPEN){
					SEQ.COVER_COUNT = 0;
					SEQ.TP_CONTROL_STATUS = 13;
				}
			}else{
				// ｶﾊﾞｰClose動作
				if(retMode == COVER_STATUS_CLOSE){
					SEQ.COVER_COUNT = 0;
					SEQ.TP_CONTROL_STATUS = 13;
				}
			}
		}else{
			SEQ.COVER_COUNT = 0;
			SEQ.TP_CONTROL_STATUS = 13;
		}
	}
}
// add 2015.12.22 K.Uemura end

//************************************************************/
//				起動処理のﾀｲﾑｱｳﾄ設定
//************************************************************/
// 1msごとの処理
void start_process_timeout(void)
{
	// 起動処理のﾀｲﾑｱｳﾄ設定
	if(COM0.MASTER_STATUS != INIT_START_MODE){
		if(COM0.STARTUP_COUNT != 0)		COM0.STARTUP_COUNT = 0;		// 起動処理ｶｳﾝﾄ
	}else{
		COM0.STARTUP_COUNT++;										// 起動処理ｶｳﾝﾄ
		if(COM0.STARTUP_COUNT > 30000){								// 起動処理ｶｳﾝﾄが30秒(30000)以上になったとき
			COM0.MASTER_STATUS = RESET_MODE;						// 通信を行わない
		}
	}
}
	
//************************************************************/
//				FPGA制御
//************************************************************/
// 10usごとの処理
void fpga_control(void)
{
	switch(SEQ.CHANGE_FPGA){
		case 1:		send_to_fpga();					break;		// ﾊﾟﾗﾒｰﾀ
		case 2:		send_to_fpga_idle();			break;		// FPGAへのﾃﾞｰﾀ送信関数(空回し)
		case 3:		send_to_fpga3();				break;		// ﾌﾞﾗｯｸﾁｪｯｸ・ﾎﾜｲﾄﾁｪｯｸ・ﾗｲﾝｾﾝｻﾁｪｯｸ・調光
		case 4:		send_to_fpga_auto();			break;		// FPGAへのﾃﾞｰﾀ送信関数(自動判別)
		case 5:		send_to_fpga_tuning();			break;		// FPGAへのﾃﾞｰﾀ送信関数(ﾁｭｰﾆﾝｸﾞ)
		case 6:		send_to_fpga2();				break;		// FPGAへのﾃﾞｰﾀ送信関数(本計測)
		
		case 8:		send_to_fpga_conversion();		break;		// FPGAへのﾃﾞｰﾀ送信関数(換算ﾃﾞｰﾀ取得)
		case 9:		send_to_fpga_measure_stop();	break;		// FPGAへのﾃﾞｰﾀ送信関数(計測停止)
		case 10:	send_to_fpga_debug();			break;		// FPGAへのﾃﾞｰﾀ送信関数(ﾃﾞﾊﾞｯｸﾞ出力)
		case 11:	send_to_fpga_revision();		break;		// FPGAへのﾃﾞｰﾀ送信関数(ﾊﾞｰｼﾞｮﾝ取得)
		case 12:	send_to_fpga_verify();			break;		// FPGAへのﾃﾞｰﾀ送信関数(ﾊﾟﾗﾒｰﾀ比較)
		case 13:	send_to_fpga_pixel();			break;		// FPGAへのﾃﾞｰﾀ送信関数(特定画素)
		case 14:	send_to_fpga_add_data();		break;		// FPGAへのﾃﾞｰﾀ送信関数(平均値・最小値・最大値のﾃﾞｰﾀ出力設定)
		case 15:	send_to_fpga_hdi_check();		break;		// FPGAへのﾃﾞｰﾀ送信関数(HDI出力検査)
		
		case 16:	send_to_fpga_profile();			break;		// FPGAへのﾃﾞｰﾀ送信関数(ﾌﾟﾛﾌｧｲﾙ処理)
// add 2016.10.20 K.Uemura start	GA2002
		case 17:	send_to_fpga_debug_level();		break;		// FPGAへのﾃﾞｰﾀ送信関数(ﾃﾞﾊﾞｯｸﾞ出力)
// add 2016.10.20 K.Uemura end
	}
}
	
//************************************************************/
//				HDI出力保持用ｶｳﾝﾄ
//************************************************************/
void hdi_count(void)
{
// add 2016.03.11 K.Uemura start	G31101
	if(SEQ.HDI0_DELAY_COUNT < 1000)		SEQ.HDI0_DELAY_COUNT++;				// HDI0ｽｷｯﾌﾟ遅延ｶｳﾝﾄ
	if(SEQ.HDI1_DELAY_COUNT < 1000)		SEQ.HDI1_DELAY_COUNT++;				// HDI1ｽｷｯﾌﾟ遅延ｶｳﾝﾄ
	if(SEQ.HDI2_DELAY_COUNT < 1000)		SEQ.HDI2_DELAY_COUNT++;				// HDI2ｽｷｯﾌﾟ遅延ｶｳﾝﾄ
	if(SEQ.HDI3_DELAY_COUNT < 1000)		SEQ.HDI3_DELAY_COUNT++;				// HDI3ｽｷｯﾌﾟ遅延ｶｳﾝﾄ
	if(SEQ.HDI4_DELAY_COUNT < 1000)		SEQ.HDI4_DELAY_COUNT++;				// HDI4ｽｷｯﾌﾟ遅延ｶｳﾝﾄ
	if(SEQ.HDI5_DELAY_COUNT < 1000)		SEQ.HDI5_DELAY_COUNT++;				// HDI5ｽｷｯﾌﾟ遅延ｶｳﾝﾄ
// add 2016.03.11 K.Uemura end
	if(SEQ.HDI0_HOLD_COUNT < 1000)		SEQ.HDI0_HOLD_COUNT++;				// HDI0ｽｷｯﾌﾟ保持ｶｳﾝﾄ
	if(SEQ.HDI1_HOLD_COUNT < 1000)		SEQ.HDI1_HOLD_COUNT++;				// HDI1ｽｷｯﾌﾟ保持ｶｳﾝﾄ
	if(SEQ.HDI2_HOLD_COUNT < 1000)		SEQ.HDI2_HOLD_COUNT++;				// HDI2ｽｷｯﾌﾟ保持ｶｳﾝﾄ
	if(SEQ.HDI3_HOLD_COUNT < 1000)		SEQ.HDI3_HOLD_COUNT++;				// HDI3ｽｷｯﾌﾟ保持ｶｳﾝﾄ
	if(SEQ.HDI4_HOLD_COUNT < 1000)		SEQ.HDI4_HOLD_COUNT++;				// HDI4ｽｷｯﾌﾟ保持ｶｳﾝﾄ
	if(SEQ.HDI5_HOLD_COUNT < 1000)		SEQ.HDI5_HOLD_COUNT++;				// HDI5ｽｷｯﾌﾟ保持ｶｳﾝﾄ
}

//************************************************************/
//				計測開始遅延時間ｶｳﾝﾄ
//************************************************************/
void start_delay_time_count(void)
{
	// 計測開始遅延時間ﾌﾗｸﾞが「1」のとき
	if(SEQ.FLAG6.BIT.START_DELAY_TIME == 1){
		SEQ.START_DELAY_TIME_COUNT++;							// 計測開始遅延時間ｶｳﾝﾀ
	}
}

#ifdef __cplusplus
void abort(void)
{

}
#endif
