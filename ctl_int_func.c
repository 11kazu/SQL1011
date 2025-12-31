/******************************************************************************
* File Name	: ctl_int_func.c
******************************************************************************/
#include <machine.h>
#include "iodefine.h"
#include "typedefine.h"
#include "usercopy.h"
#include "user_define.h"

void int_1us_func(void);		// 1usﾀｲﾏｰ割込関数
void int_10us_func(void);		// 10usﾀｲﾏｰ割込関数
void int_100us_func(void);		// 100usﾀｲﾏｰ割込関数

void int_1ms_func(void);		// 1msﾀｲﾏｰ割込関数
void int_10ms_timer(void);		// 10msﾀｲﾏｰ制御関数

//************************************************************/
//				1usﾀｲﾏｰ割込関数
//************************************************************/
void int_1us_func(void)
{
	TIM1.MASTER_COUNT_1US++;					// 1usﾏｽﾀｰｼｰｹﾝｽｽﾚｯﾄﾞｶｳﾝﾀ
	
	// 10usごとの処理
	switch(TIM1.MASTER_COUNT_1US){
		case 1:
			SEQ.FLAG1.BIT.F_SHOT = 1;			// FPGA関数ﾌﾗｸﾞ(10usごとに処理を行う)
			break;		
			
		case 2:
			TIM1.MASTER_COUNT_1US = 0;			// 10usﾀｲﾏｰ割込関数(10usごとに処理を行う)
			int_10us_func();
			break;
	}
}

//************************************************************/
//				10usﾀｲﾏｰ割込関数
//************************************************************/
void int_10us_func(void)
{
	TIM1.MASTER_COUNT_10US++;					// 10usﾏｽﾀｰｼｰｹﾝｽｽﾚｯﾄﾞｶｳﾝﾀ
	
	// 100usごとの処理
	switch(TIM1.MASTER_COUNT_10US){
		case 1:
		case 3:
		case 5:
		case 7:
		case 9:
			SEQ.FLAG1.BIT.F_SHOT = 1;			// FPGA関数ﾌﾗｸﾞ
			break;
		
		case 2:
			// 計測しているときで、かつHDI出力ﾌﾗｸﾞが「1」のとき
			if((SEQ.FLAG.BIT.MEASUREMENT == MEASURE_RUNNING)&&(SEQ.FLAG3.BIT.HDI_OUTPUT == 1)){
			}else{
				SEQ.FLAG1.BIT.HDI_SHOT = 1;		// HDI関数処理(100usごとに処理を行う)
			}
			break;
			
		case 4:
			SEQ.FLAG1.BIT.O_SHOT = 1;			// LED出力関数処理(100usごとに処理を行う)
			break;
			
		case 6:
			SEQ.FLAG1.BIT.M25_SHOT = 1;			// M25関数処理(100usごとに処理を行う)
			break;
			
		case 8:
			SEQ.FLAG1.BIT.TP_SHOT = 1;			// ﾀｯﾁﾊﾟﾈﾙ処理(100usごとに処理を行う)
			break;
			
		case 10:
			TIM1.MASTER_COUNT_10US = 0;
			int_100us_func();					// 100usﾀｲﾏｰ割込関数
			break;
	}
}

//************************************************************/
//				100usﾀｲﾏｰ割込関数
//************************************************************/
void int_100us_func(void)
{
	TIM1.MASTER_COUNT_100US++;					// 100usﾏｽﾀｰｼｰｹﾝｽｽﾚｯﾄﾞｶｳﾝﾀ
	
	// 1msごとの処理
	switch(TIM1.MASTER_COUNT_100US){
		case 1:
			SEQ.FLAG1.BIT.MSEC_SHOT = 1;		// 1msﾌﾗｸﾞ(1msごとに処理を行う)
			break;
			
		case 10:
			TIM1.MASTER_COUNT_100US = 0;
			int_1ms_func();						// 1msﾀｲﾏｰ割込関数
			break;
	}
}

/********************************************************/
/*		1msﾀｲﾏｰ割込関数								*/
/********************************************************/
void int_1ms_func(void)
{
	SEQ.MASTER_COUNT++;
	SEQ.TIM_COUNT++;
	
	// 10msごとの処理
	switch(SEQ.MASTER_COUNT){
		case 1:
			SEQ.FLAG1.BIT.I_SHOT = 1;			// 入力関数処理
			break;
			
		case 3:
			SEQ.FLAG1.BIT.R_SHOT = 1;			// ROM関数処理
			break;
			
		case 5:
			SEQ.FLAG1.BIT.A_SHOT = 1;			// ｱﾅﾛｸﾞ入力関数ﾌﾗｸﾞ
			break;
			
		case 7:
			SEQ.FLAG1.BIT.DEB_SHOT = 1;			// ﾃﾞﾊﾞｯｸﾞ出力関数ﾌﾗｸﾞ
			break;
			
		case 10:
			SEQ.MASTER_COUNT = 0;
			SEQ.FLAG1.BIT.T_SHOT = 1;			// ﾀｲﾏｰ関数処理
			int_10ms_timer();
			break;
	}
	
#ifdef	DEBUG_OUTPUT_100MSEC
	if((SEQ.TIM_COUNT % 100) == 1){
		// 1secごとのデバッグ出力フラグ
		SEQ.FLAG5.BIT.DEB_SIOOUT_FLAG = 1;
	}
#endif

	if(SEQ.TIM_COUNT == 1){
		MONITOR_OUT			= 1;
		if(SEQ.FLAG3.BIT.BLINK_LED == 0){		// LED点滅ﾌﾗｸﾞ
			LED.FOCUS.BYTE = 0x07;				// 焦点
		}
		if(SEQ.FLAG5.BIT.LOW_BATTERY == 1){		// POWER LEDの点滅
			SEQ.FLAG5.BIT.LOW_BATTERY_FLAG = 0;
		}
	}else if(SEQ.TIM_COUNT == 500){
		MONITOR_OUT		= 0;
		if(SEQ.FLAG3.BIT.BLINK_LED == 0){		// LED点滅ﾌﾗｸﾞ
			LED.FOCUS.BYTE = 0;					// 焦点
		}
		if(SEQ.FLAG5.BIT.LOW_BATTERY == 1){		// POWER LEDの点滅
			SEQ.FLAG5.BIT.LOW_BATTERY_FLAG = 1;
		}
	}else if(SEQ.TIM_COUNT == 1000){
		SEQ.TIM_COUNT = 0;
		SEQ.FLAG1.BIT.SEC_SHOT	= 1;

#ifdef	DEBUG_OUTPUT_1SEC
		// 1secごとのデバッグ出力フラグ
		SEQ.FLAG5.BIT.DEB_SIOOUT_FLAG = 1;
#endif

#ifdef	DEBUG_AUTOSTART
		SEQ.DEB_RESET_TIME++;

		// 電源ON → 電源OFF → 電源ON → 電源OFF → ・・ < 繰り返し >
		if(SEQ.FLAG5.BIT.BOOTUP_PROCESS == 1){
			if(SEQ.FLAG.BIT.POWER == 1){
				// 電源ON
				if(SEQ.DEB_RESET_TIME == DEBUG_ONTIME){
					SEQ.DEB_RESET_TIME = 0;

					// sw.c S1：POWER_SW_IN
					// 電源OFF処理開始
					SEQ.FLAG.BIT.POWER = !SEQ.FLAG.BIT.POWER;	// 電源(状態反転)

					OUT.MASTER_STATUS = IDLE_MODE;				// 待機ﾓｰﾄﾞ
					SEQ.FLAG.BIT.POWER_OFF = 1;					// 電源OFFﾌﾗｸﾞをｾｯﾄ
					SEQ.FLAG.BIT.MEASUREMENT = 0;
					SEQ.FLAG.BIT.PORTABLE = 0;					// ﾎﾟｰﾀﾌﾞﾙ操作ﾌﾗｸﾞ
					SEQ.FLAG3.BIT.PEAKHOLD_ENABLE = 0;			// ﾋﾟｰｸﾎｰﾙﾄﾞ有効ﾌﾗｸﾞ
				}
			}else{
				if(SEQ.DEB_RESET_TIME == DEBUG_OFFTIME){
					SEQ.DEB_RESET_TIME = 0;

					// sw.c S1：POWER_SW_IN
					// 電源ON処理開始
					SEQ.FLAG.BIT.POWER = !SEQ.FLAG.BIT.POWER;	// 電源(状態反転)

					SEQ.FLAG.BIT.POWER_ON = 1;					// 電源ONﾌﾗｸﾞをｾｯﾄ
				}
			}
		}
#endif

	}
}
/********************************************************/
/*		10msﾀｲﾏｰ制御関数								*/
/********************************************************/
void int_10ms_timer(void)
{
	ctl_drv_timer1();
	ctl_drv_timer2();
	ctl_drv_timer3();
	ctl_drv_timer4();
	ctl_drv_timer5();
	ctl_drv_timer6();
	ctl_drv_timer7();
	ctl_drv_timer8();
}
