/******************************************************************************
* File Name	: tp_control.c
******************************************************************************/
#include <stdint.h>
#include <stdlib.h>
#include "iodefine.h"
#include "typedefine.h"
#include "usercopy.h"
#include "user_define.h"

void state_number_set(void);					// 状態番号ｾｯﾄ(No.311)
void state_number_control(void);				// 状態番号制御(No.311)
void clear_result(void);						// 結果ｸﾘｱ
// add 2015.07.22 K.Uemura start	
short check_hardware_error(void);
// add 2015.07.22 K.Uemura end
// add 2016.03.24 K.Uemura start	G32401
short check_execute_error(void);
// add 2016.03.24 K.Uemura end
// add 2015.12.22 K.Uemura start	FB2201
short get_cover_status(void);
// add 2015.12.22 K.Uemura end
// add 2016.03.21 K.Uemura start	G32101
short get_cover_status_initilized(void);
void argument_number_set(void);
// add 2016.03.21 K.Uemura end

void tp_control(void);							// ﾀｯﾁﾊﾟﾈﾙからの制御

//************************************************************/
//				状態番号ｾｯﾄ(No.311)
//************************************************************/
void state_number_set(void)
{
	COM0.NO311 = 9999;					// 該当なし
	
	SEQ.SELECT.BIT.MEASURE = 19;		// 該当なし
	
// add 2016.02.18 K.Uemura start	G21804
	SEQ.FLAG6.BIT.ADD_DATA = 0;			// 追加ﾃﾞｰﾀﾌﾗｸﾞ(平均・最小・最大) ADD 160226
// add 2016.02.18 K.Uemura end

// chg 2016.03.21 K.Uemura start	G32101
#if	0
//// add 2015.10.07 K.Uemura start	
//	if(SEQ.FLAG.BIT.PORTABLE == OPERATION_AUTO){					// 通信操作の場合
//		SEQ.SPINDLE_SPEED = ((COM0.NO303 << 16) | COM0.NO304);		// 回転数
//		SEQ.FLUTES = COM0.NO305;									// 刃数
//		SEQ.RADIUS = COM0.NO306;									// 工具径
//// add 2016.03.10 K.Uemura start	G31002
//		SEQ.EXTEND_CYCLE = COM0.NO307;								// 引数
//// add 2016.03.10 K.Uemura end
//	}
//// add 2015.10.07 K.Uemura end
#endif
// chg 2016.03.21 K.Uemura end

	// 311 状態番号の設定
	switch(COM0.NO301){
		// 動作ﾓｰﾄﾞ
		case 0:		// TOP(起動時の画面)
			COM0.NO311 = 0;									// 何もしていない
			break;
			
		// ORIGIN
		case 1:		// ORIGIN(X)
		case 3:		// ORIGIN(Z)
			COM0.NO311 = COM0.NO301;
			SEQ.SELECT.BIT.MEASURE = MODE_ORIGIN;			// 7:ORIGIN
			break;
			
// add 2016.02.18 K.Uemura start	G21804
		// 清掃確認
		case 152:	// 清掃確認 清掃ﾚﾍﾞﾙ出力(連続)
// add 2016.06.22 K.Uemura start	G62202
		case 153:	// 清掃確認 清掃ﾚﾍﾞﾙ出力(1回)
// add 2016.06.22 K.Uemura end
			COM0.NO311 = COM0.NO301;
			SEQ.SELECT.BIT.MEASURE = MODE_ORIGIN;			// 7:ORIGIN
			SEQ.FLAG6.BIT.ADD_DATA = 1;						// 追加ﾃﾞｰﾀﾌﾗｸﾞ(平均・最小・最大) ADD 160226
			break;
// add 2016.02.18 K.Uemura end
			
		// ORIGIN(ｴｯｼﾞ考慮)
		case 2:		// ORIGIN(ｴｯｼﾞ考慮)(X)
		case 4:		// ORIGIN(ｴｯｼﾞ考慮)(Z)
			COM0.NO311 = COM0.NO301;
			SEQ.SELECT.BIT.MEASURE = MODE_ORIGIN_EDGE;		// 8:ORIGIN(ｴｯｼﾞ考慮)
			break;
			
		// 工具径(d≦4)(X)
		case 10:	// 工具径(d≦4)(X)
			COM0.NO311 = COM0.NO301;
			SEQ.SELECT.BIT.MEASURE = MODE_D4_LOW;			// 0:工具径(d≦4)
			break;
			
		// 工具径(d＞4 自動)(X)
		case 20:	// 工具径(d＞4 自動)(X)
			COM0.NO311 = COM0.NO301;
			SEQ.SELECT.BIT.MEASURE = MODE_D4_AUTO;			// 10:工具径(d＞4 自動)
			break;
			
		// 工具径(d＞4 左側)(X)
		case 21:	// 工具径(d＞4 左側)(X)
			COM0.NO311 = COM0.NO301;
			SEQ.SELECT.BIT.MEASURE = MODE_D4_LEFT;			// 1:工具径(d＞4 左側)
// add 2016.09.08 K.Uemura start	G90801
			if(RESULT_SIGN == 1){
				SEQ.SELECT.BIT.MEASURE = MODE_D4_RIGHT;		// 11:工具径(d＞4 右側)
			}
// add 2016.09.08 K.Uemura end
			break;
			
		// 工具径(d＞4 右側)(X)
		case 22:	// 工具径(d＞4 右側)(X)
			COM0.NO311 = COM0.NO301;
			SEQ.SELECT.BIT.MEASURE = MODE_D4_RIGHT;			// 11:工具径(d＞4 右側)
// add 2016.09.08 K.Uemura start	G90801
			if(RESULT_SIGN == 1){
				SEQ.SELECT.BIT.MEASURE = MODE_D4_LEFT;		// 11:工具径(d＞4 左側)
			}
// add 2016.09.08 K.Uemura end
			break;
			
		// 工具長(Z)
		case 30:	// 工具長(Z)
			COM0.NO311 = COM0.NO301;
			SEQ.SELECT.BIT.MEASURE = MODE_D4_LEFT;			// 1:工具径(d＞4 左側)
			break;
			
		// 振れ測定
		case 50:	// 振れ測定(X)
		case 51:	// 振れ測定(Z)
			// 振れパラメータの設定
			if(SEQ.FLUTES == 0){							// 刃数が「0」のとき
				SEQ.TRIGGER_TIME_PERIOD = 60000 / SEQ.SPINDLE_SPEED;						// ms
			}else{
				SEQ.TRIGGER_TIME_PERIOD = (60000 / SEQ.SPINDLE_SPEED) / SEQ.FLUTES;
			}
			
			COM0.NO125 = SEQ.TRIGGER_TIME_PERIOD;
			COM0.NO126 = SEQ.TRIGGER_TIME_PERIOD;

			COM0.NO127 = COM0.NO303;						// 回転数[上位]
			COM0.NO128 = COM0.NO304;						// 回転数[下位]
			COM0.NO129 = COM0.NO305;						// 刃数

			COM0.NO311 = COM0.NO301;
			SEQ.SELECT.BIT.MEASURE = MODE_RUNOUT;			// 2:振れ
			break;
			
		// ﾌﾟﾛﾌｧｲﾙ
		case 60:	// ﾌﾟﾛﾌｧｲﾙ(X)
		case 61:	// ﾌﾟﾛﾌｧｲﾙ(Z)
// add 2016.03.08 K.Uemura start	G30801
		case 62:	// ﾌﾟﾛﾌｧｲﾙ(X 左側)
		case 63:	// ﾌﾟﾛﾌｧｲﾙ(Z 右側)
// add 2016.03.08 K.Uemura end
			COM0.NO311 = COM0.NO301;
			SEQ.SELECT.BIT.MEASURE = MODE_PROFILE;			// 5:ﾌﾟﾛﾌｧｲﾙ
			break;
			
		// 伸び測定(Z)
		case 70:	// 伸び測定(Z)
			COM0.NO311 = COM0.NO301;
			SEQ.SELECT.BIT.MEASURE = MODE_GROWTH;			// 6:伸び測定(Z)
			break;
			
		// 焦点合わせ
		case 100:	// 焦点合わせ(X)
		case 101:	// 焦点合わせ(Z)
			COM0.NO311 = COM0.NO301;
			SEQ.SELECT.BIT.MEASURE = MODE_FOCUS;			// 3:焦点
			break;
			
		// 中心位置設定
		case 110:	// 中心位置設定(X)
		case 111:	// 中心位置設定(Z)
			COM0.NO311 = COM0.NO301;
			SEQ.SELECT.BIT.MEASURE = MODE_CENTER;			// 4:中心位置設定
			break;
			
		case 200:	// 電磁ﾊﾞﾙﾌﾞ1(ｶﾊﾞｰ状態) ON
		case 201:	// 電磁ﾊﾞﾙﾌﾞ1(ｶﾊﾞｰ状態) OFF
		case 250:	// HDI0 ON
		case 251:	// HDI0 OFF
		case 252:	// HDI1 ON
		case 253:	// HDI1 OFF
		case 254:	// HDI2 ON
		case 255:	// HDI2 OFF
		case 256:	// HDI3 ON
		case 257:	// HDI3 OFF
		case 258:	// HDI4 ON
		case 259:	// HDI4 OFF
		case 260:	// HDI5 ON
		case 261:	// HDI5 OFF
		case 262:	// HDI0-5 ON
		case 263:	// HDI0-5 OFF
		case 300:	// 電磁ﾊﾞﾙﾌﾞ2(清掃ｴｱ状態) ON
		case 301:	// 電磁ﾊﾞﾙﾌﾞ2(清掃ｴｱ状態) OFF
		case 505:	// 
		case 506:	// 
			COM0.NO311 = COM0.NO301;
			break;

		case 400:	// ﾊﾞｯﾌｧ取得(転送)
// add 2016.03.02 K.Uemura start	G30202
		case 401:	// ﾊﾞｯﾌｧ取得(転送) ﾌﾟﾛﾌｧｲﾙX
		case 402:	// ﾊﾞｯﾌｧ取得(転送) ﾌﾟﾛﾌｧｲﾙZ
// add 2016.03.02 K.Uemura end
// add 2016.07.01 K.Uemura start	G70101
		case 403:	// ﾊﾞｯﾌｧ取得(転送) 振れX
		case 404:	// ﾊﾞｯﾌｧ取得(転送) 振れZ
		case 405:	// ﾊﾞｯﾌｧ取得(転送) 伸び
// add 2016.07.01 K.Uemura end
		case 600:	// 換算ﾃｰﾌﾞﾙ自動設定
		case 601:	// 換算ﾃｰﾌﾞﾙ設定(TPD→RX)
		case 602:	// 換算ﾃｰﾌﾞﾙ取得(RX→TPD)

// add 2015.09.02 K.Uemura start	
		case 150:	// 初期設定
// add 2015.09.02 K.Uemura end
// add 2016.02.18 K.Uemura start	G21803
		case 151:	// 清掃確認 LED点灯
// add 2016.02.18 K.Uemura end
		case 504:	// 初期設定
		case 507:	// 調光ﾚﾍﾞﾙ
			COM0.NO311 = COM0.NO301;

// chg 2016.07.15 K.Uemura start	G71501
			if((600 <= COM0.NO301) && (COM0.NO301 <= 602)){
				COM0.NO312 = ERR_NOMODE;
				if(COM0.NO302 & 0x4000){
					COM0.NO312 = 0;
					if(COM0.NO301 == 600){
						// ｱﾄﾞﾚｽにより方向を判定
						if(COM0.NO302 & 0x8000)		SEQ.MEASUREMENT_DIRECTION = Z_DIRECTION;	// 計測方向(Z)
						else						SEQ.MEASUREMENT_DIRECTION = X_DIRECTION;	// 計測方向(X)
					}
				}
			}
// chg 2016.07.15 K.Uemura end
			break;
			
		case 700:	// 特定画素値の出力(X)
		case 701:	// 特定画素値の出力(Z)
// add 2016.10.20 K.Uemura start	GA2002
		case 710:	// ｾﾝｻﾚﾍﾞﾙ出力(X)
		case 711:	// ｾﾝｻﾚﾍﾞﾙ出力(Z)
			SEQ.BUFFER_NO_OLD = SEQ.BUFFER_NO_NEW = 0;
// add 2016.10.20 K.Uemura end
			COM0.NO311 = COM0.NO301;
// add 2016.03.10 K.Uemura start	G31001
			SEQ.SELECT.BIT.MEASURE = MODE_ORIGIN;			// 7:ORIGIN
// add 2016.03.10 K.Uemura end
			SEQ.MEASUREMENT_DIRECTION_BEFORE = 2;			// 計測方向(直前)をX・Z以外にする
			break;
			
			/*
		case 702:	// 平均値・最小値・最大値のﾃﾞｰﾀ出力設定
			COM0.NO311 = COM0.NO301;
			break;
			*/

// add 2016.07.01 K.Uemura start	G70102
		default:
			__FILE__;
			__LINE__;
			output_error(__LINE__);
			COM0.NO312 = ERR_NOMODE;
			break;
// add 2016.07.01 K.Uemura end
	}
	
	// 計測方向
	switch(COM0.NO301){
		// X方向
		case 1:		// ORIGIN(X)
		case 2:		// ORIGIN(X)(ｴｯｼﾞ考慮)
		case 10:	// 工具径(d≦4)
		case 20:	// 工具径(d＞4 自動)
		case 21:	// 工具径(d＞4 左側)
		case 22:	// 工具径(d＞4 右側)
		case 50:	// 振れ測定(X)
		case 60:	// ﾌﾟﾛﾌｧｲﾙ(X)
// add 2016.03.08 K.Uemura start	G30801
		case 62:	// ﾌﾟﾛﾌｧｲﾙ(X 左側)
		case 63:	// ﾌﾟﾛﾌｧｲﾙ(Z 右側)
// add 2016.03.08 K.Uemura end
		case 100:	// 焦点合わせ(X)
		case 110:	// 中心位置設定(X)
// add 2016.02.18 K.Uemura start	G21804
		case 152:	// 清掃確認 清掃ﾚﾍﾞﾙ出力(連続)
// add 2016.02.18 K.Uemura end
// add 2016.06.22 K.Uemura start	G62202
		case 153:	// 清掃確認 清掃ﾚﾍﾞﾙ出力(1回)
// add 2016.06.22 K.Uemura end
		case 700:	// 特定画素値の出力(X)
// add 2016.10.20 K.Uemura start	GA2002
		case 710:	// ｾﾝｻﾚﾍﾞﾙ出力
// add 2016.10.20 K.Uemura end
			SEQ.MEASUREMENT_DIRECTION = X_DIRECTION;	// 計測方向(X)
			break;
			
			
		// Z方向
		case 3:		// ORIGIN(Z)
		case 4:		// ORIGIN(Z)(ｴｯｼﾞ考慮)
		case 30:	// 工具長(Z)
		case 51:	// 振れ測定(Z)
		case 61:	// ﾌﾟﾛﾌｧｲﾙ(Z)
		case 70:	// 伸び測定(Z)
		case 101:	// 焦点合わせ(Z)
		case 111:	// 中心位置設定(Z)
		case 701:	// 特定画素値の出力(Z)
// add 2016.10.20 K.Uemura start	GA2002
		case 711:	// ｾﾝｻﾚﾍﾞﾙ出力(Z)
// add 2016.10.20 K.Uemura end
			SEQ.MEASUREMENT_DIRECTION = Z_DIRECTION;	// 計測方向(Z)
			break;
	}
}

//************************************************************/
//				状態番号制御(No.311)
//************************************************************/
void state_number_control(void)
{
	unsigned short i;
	
	SEQ.FLAG6.BIT.CLEANING = 0;

	// 311 状態番号の設定
	switch(COM0.NO301){
		// 動作ﾓｰﾄﾞ
		case 0:		// TOP(起動時の画面)
			break;
			
		case 1:		// ORIGIN(X)
		case 2:		// ORIGIN(X)(ｴｯｼﾞ考慮)
		case 3:		// ORIGIN(Z)
		case 4:		// ORIGIN(Z)(ｴｯｼﾞ考慮)
		case 10:	// 工具径(d≦4)
		case 20:	// 工具径(d＞4 自動)
		case 21:	// 工具径(d＞4
		case 22:	// 工具径(d＞4
		case 30:	// 工具長(Z)
		case 50:	// 振れ測定(X)
		case 51:	// 振れ測定(Z)
		case 60:	// ﾌﾟﾛﾌｧｲﾙ(X)
		case 61:	// ﾌﾟﾛﾌｧｲﾙ(Z)
// add 2016.03.08 K.Uemura start	G30801
		case 62:	// ﾌﾟﾛﾌｧｲﾙ(X 左側)
		case 63:	// ﾌﾟﾛﾌｧｲﾙ(Z 右側)
// add 2016.03.08 K.Uemura end
		case 70:	// 伸び測定(Z)
		case 100:	// 焦点合わせ(X)
		case 101:	// 焦点合わせ(Z)
		case 110:	// 中心位置設定(X)
		case 111:	// 中心位置設定(Z)
		case 600:	// 換算ﾃｰﾌﾞﾙ自動設定
// add 2016.07.15 K.Uemura start	G71501
			// コマンド600の場合、14bitがONでなければ不可
			if(COM0.NO301 == 600){
				if(COM0.NO302 & 0x4000){
				}else{
					break;
				}
			}
// add 2016.07.15 K.Uemura end

			if(SEQ.FLAG.BIT.MEASUREMENT == MEASURE_STOP){
				SEQ.FLAG.BIT.MEASUREMENT = MEASURE_RUNNING;
				
				SEQ.LAST_CBUS_NUMBER = 215;					// CBUSﾅﾝﾊﾞｰ(1ｻｲｸﾙ最終)	(通常)
				
				led_measure_set();							// LED 測定・設定ｾｯﾄ
				
				// 計測方向が直前と異なるときﾊﾟﾗﾒｰﾀ設定を送信
				if(SEQ.MEASUREMENT_DIRECTION_BEFORE != SEQ.MEASUREMENT_DIRECTION){
					SEQ.FLAG.BIT.PARAM_INITIAL = 1;			// ﾊﾟﾗﾒｰﾀ初期化ﾌﾗｸﾞ
					SEQ.CHANGE_FPGA = 1;					// ﾊﾟﾗﾒｰﾀ
					SEQ.FPGA_SEND_STATUS = 1;
					SEQ.POWER_COUNT = 0;					// 電源制御ｶｳﾝﾄをﾘｾｯﾄ
					
					set_7seg_upper_no_data();				// 7ｾｸﾞ表示(-----)(上段)
					set_7seg_lower_no_data();				// 7ｾｸﾞ表示(-----)(下段)
					
				// 計測方向が直前と同じとき計測(idle)を行う
				}else{
					SEQ.CBUS_NUMBER = 394;
					SEQ.CHANGE_FPGA = 2;					// 測定
					SEQ.FPGA_SEND_STATUS = 1;
					
//					OUT.SUB_STATUS = 11;
				}
// add 2016.03.02 K.Uemura start	G30203
				OUT.SUB_STATUS = 11;
// add 2016.03.02 K.Uemura end
			}
			break;
			
// add 2016.02.18 K.Uemura start	G21804
		case 152:	// 清掃確認(連続)
// add 2016.06.22 K.Uemura start	G62202
		case 153:	// 清掃確認(1回)
// add 2016.06.22 K.Uemura end
			if(SEQ.FLAG.BIT.MEASUREMENT == MEASURE_STOP){
				SEQ.FLAG.BIT.MEASUREMENT = MEASURE_RUNNING;
				
				clear_result();								// 結果ｸﾘｱ
				
				SEQ.FLAG6.BIT.CLEANING = 1;
				SEQ.CLEANING_CYCLE = 0;
				SEQ.LAST_CBUS_NUMBER = 216;					// CBUSﾅﾝﾊﾞｰ(1ｻｲｸﾙ最終)	(平均値・最小値・最大値のﾃﾞｰﾀを付加)
//				SEQ.LAST_CBUS_NUMBER = 218;					// CBUSﾅﾝﾊﾞｰ(1ｻｲｸﾙ最終)	(平均値・最小値・最大値のﾃﾞｰﾀを付加)
				
				led_measure_set();							// LED 測定・設定ｾｯﾄ
				
				// 計測方向が直前と異なるときﾊﾟﾗﾒｰﾀ設定を送信
				if(SEQ.MEASUREMENT_DIRECTION_BEFORE != SEQ.MEASUREMENT_DIRECTION){
					SEQ.FLAG.BIT.PARAM_INITIAL = 1;			// ﾊﾟﾗﾒｰﾀ初期化ﾌﾗｸﾞ
					SEQ.CHANGE_FPGA = 1;					// ﾊﾟﾗﾒｰﾀ
					SEQ.FPGA_SEND_STATUS = 1;
					SEQ.POWER_COUNT = 0;					// 電源制御ｶｳﾝﾄをﾘｾｯﾄ
					
					set_7seg_upper_no_data();				// 7ｾｸﾞ表示(-----)(上段)
					set_7seg_lower_no_data();				// 7ｾｸﾞ表示(-----)(下段)
					
				// 計測方向が直前と同じとき計測(idle)を行う
				}else{
					//SEQ.CBUS_NUMBER = 394;
					SEQ.CBUS_NUMBER = 395;
					SEQ.CHANGE_FPGA = 2;					// 測定
					SEQ.FPGA_SEND_STATUS = 1;
					
//					OUT.SUB_STATUS = 11;
				}
// add 2016.03.02 K.Uemura start	G30202
				OUT.SUB_STATUS = 11;
// add 2016.03.02 K.Uemura end
			}
			break;
// add 2016.02.18 K.Uemura end
			
		case 700:	// 特定画素値の出力(X)
		case 701:	// 特定画素値の出力(Z)
// add 2016.10.20 K.Uemura start	GA2002
		case 710:	// MODE01 ｺﾏﾝﾄﾞ(X)
		case 711:	// MODE01 ｺﾏﾝﾄﾞ(Z)
// add 2016.10.20 K.Uemura end
			if((COM0.NO301 == 700) || (COM0.NO301 == 701)){
				if(4095 < COM0.NO302){		// 予備(302)の値が4096以上のとき
					break;
				}
			}

			if(SEQ.FLAG.BIT.MEASUREMENT == MEASURE_STOP){
				SEQ.FLAG.BIT.MEASUREMENT = MEASURE_RUNNING;
				
				led_measure_set();							// LED 測定・設定ｾｯﾄ
				
				// 計測方向が直前と異なるときﾊﾟﾗﾒｰﾀ設定を送信
				if(SEQ.MEASUREMENT_DIRECTION_BEFORE != SEQ.MEASUREMENT_DIRECTION){
					SEQ.FLAG.BIT.PARAM_INITIAL = 1;			// ﾊﾟﾗﾒｰﾀ初期化ﾌﾗｸﾞ
					SEQ.CHANGE_FPGA = 1;					// ﾊﾟﾗﾒｰﾀ
					SEQ.FPGA_SEND_STATUS = 1;
					SEQ.POWER_COUNT = 0;					// 電源制御ｶｳﾝﾄをﾘｾｯﾄ
					
					set_7seg_upper_no_data();				// 7ｾｸﾞ表示(-----)(上段)
					set_7seg_lower_no_data();				// 7ｾｸﾞ表示(-----)(下段)
					
				// 計測方向が直前と同じとき計測(idle)を行う
				}else{
					SEQ.CBUS_NUMBER = 394;
					SEQ.CHANGE_FPGA = 2;					// 測定
					SEQ.FPGA_SEND_STATUS = 1;
					
					OUT.SUB_STATUS = 11;
				}
			}
			break;
			
			/*
		case 702:	// 平均値・最小値・最大値のﾃﾞｰﾀ出力設定
			// 予備(302)の値	0:ﾃﾞｰﾀ出力しない	1:ﾃﾞｰﾀ出力する
			if(COM0.NO302 <= 1){		// 予備(302)の値が1以下のとき
				SEQ.CBUS_NUMBER = 387;
				SEQ.CHANGE_FPGA = 14;					// 測定
				SEQ.FPGA_SEND_STATUS = 1;
			}
			break;
			*/
			
		// 外部機器
		case 200:	// 電磁ﾊﾞﾙﾌﾞ1(ｶﾊﾞｰ OPEN)
			break;
		case 201:	// 電磁ﾊﾞﾙﾌﾞ1(ｶﾊﾞｰ CLOSE)
			break;
#if	1
		case 250:	break;	// HDI0 ON
		case 251:	break;	// HDI0 OFF
		case 252:	break;	// HDI1 ON
		case 253:	break;	// HDI1 OFF
		case 254:	break;	// HDI2 ON
		case 255:	break;	// HDI2 OFF
		case 256:	break;	// HDI3 ON
		case 257:	break;	// HDI3 OFF
		case 258:	break;	// HDI4 ON
		case 259:	break;	// HDI4 OFF
		case 260:	break;	// HDI5 ON
		case 261:	break;	// HDI5 OFF
		case 262:	break;	// HDI0～5 ON
		case 263:	break;	// HDI0～5 OFF
#endif
		case 300:	// 電磁ﾊﾞﾙﾌﾞ2(清掃ｴｱ噴射)
			break;
		case 301:	// 電磁ﾊﾞﾙﾌﾞ2(清掃ｴｱ停止)
			break;
		
		// ｵﾌﾟｼｮﾝ
		case 400:	// ﾊﾞｯﾌｧ取得(転送)
			break;
// add 2016.03.02 K.Uemura start	G30202
		case 401:	// ﾊﾞｯﾌｧ取得(転送) ﾌﾟﾛﾌｧｲﾙX
		case 402:	// ﾊﾞｯﾌｧ取得(転送) ﾌﾟﾛﾌｧｲﾙZ
// add 2016.07.01 K.Uemura start	G70101
		case 403:	// ﾊﾞｯﾌｧ取得(転送) 振れX
		case 404:	// ﾊﾞｯﾌｧ取得(転送) 振れZ
		case 405:	// ﾊﾞｯﾌｧ取得(転送) 伸び
// add 2016.07.01 K.Uemura end
			break;
// add 2016.03.02 K.Uemura end
		
// 司令(READY OFF検知)がONのとき計測結果を出力する
// COM0.NO301		504		ﾊﾟﾗﾒｰﾀ → ﾌﾞﾗｯｸ → ﾎﾜｲﾄ → ﾊﾟﾗﾒｰﾀを送信

//					505		ﾊﾟﾗﾒｰﾀ読み出し	TP → RX
//					506		ﾊﾟﾗﾒｰﾀ書き込み	RX → TP
//					507		LED輝度設定
		
// add 2015.09.02 K.Uemura start	504コマンド修正
		case 150:	// 初期設定		// ﾊﾟﾗﾒｰﾀ・ﾌﾞﾗｯｸ・ﾎﾜｲﾄ・ﾊﾟﾗﾒｰﾀを送信
			SEQ.INITIAL_COUNT = 1;						// 起動処理開始
			break;
// add 2015.09.02 K.Uemura end
// add 2016.02.18 K.Uemura start	G21803
		case 151:	// 清掃確認 LED点灯
			SEQ.FLAG.BIT.MEMORY_CONTROL = 1;
			
			SEQ.LED_BRIGHTNESS = 500;
			DA.DADR0 = SEQ.LED_BRIGHTNESS;
			
			// 値を書き込み用ﾊﾞｯﾌｧにｺﾋﾟｰする
			for(i = 1; i<=4000; i++){
				I2C.WR_BUF[i] = I2C.RE_BUF[i];
			}
			
			I2C.WR_CONT = 0;
			
			if(RIIC0.ICCR2.BIT.BBSY == 0){
				RIIC0.ICCR2.BIT.ST = 1;			// ｽﾀｰﾄﾋﾞｯﾄ
				I2C.SUB_STATUS = 12;
				
				IR(RIIC0, ICEEI0) = 0;			// 割り込み要求ｸﾘｱ(通信ｴﾗｰ/ｲﾍﾞﾝﾄ発生)
				IEN(RIIC0, ICEEI0) = 1;			// 割り込み要求許可(通信ｴﾗｰ/ｲﾍﾞﾝﾄ発生)
			}
			break;
// add 2016.02.18 K.Uemura end

		case 504:	// 初期設定		// ﾊﾟﾗﾒｰﾀ・ﾌﾞﾗｯｸ・ﾎﾜｲﾄ・ﾊﾟﾗﾒｰﾀを送信
			SEQ.FLAG.BIT.POWER = !SEQ.FLAG.BIT.POWER;	// 電源(状態反転)
			
			if(SEQ.FLAG.BIT.POWER){
				//OUT.MASTER_STATUS = OUT_DRV_MODE;		// 通常ﾓｰﾄﾞ
				//OUT.SUB_STATUS = 1;
				
				SEQ.FLAG.BIT.POWER_ON = 1;				// 電源ONﾌﾗｸﾞをｾｯﾄ
			}else{
				OUT.MASTER_STATUS = IDLE_MODE;		// 待機ﾓｰﾄﾞ
				
				SEQ.FLAG.BIT.POWER_OFF = 1;				// 電源OFFﾌﾗｸﾞをｾｯﾄ
				SEQ.FLAG.BIT.MEASUREMENT = MEASURE_STOP;
			}
			break;
			
		case 505:	// DLPMﾊﾞｯｸｱｯﾌﾟ(FROM保存)	// ﾊﾟﾗﾒｰﾀ読み出し	TP → RX
			SEQ.MEASUREMENT_DIRECTION_BEFORE = 2;			// 計測方向(直前)をX・Z以外にする
			SEQ.FLAG3.BIT.PARA_READ = 1;					// ﾊﾟﾗﾒｰﾀ読み出しﾌﾗｸﾞ
			break;
			
		case 506:	// DLPMﾘｽﾄｱ(FROM呼び出し)	// ﾊﾟﾗﾒｰﾀ書き込み	RX → TP
			SEQ.FLAG3.BIT.PARA_WRITE = 1;				// ﾊﾟﾗﾒｰﾀ書き込みﾌﾗｸﾞ
			break;
			
		case 507:	// 調光ﾚﾍﾞﾙ		// LED輝度設定
			if(COM0.NO302 <= 1023){
				SEQ.FLAG.BIT.MEMORY_CONTROL = 1;
				
				SEQ.LED_BRIGHTNESS = COM0.NO302;
				DA.DADR0 = SEQ.LED_BRIGHTNESS;
				
				// 値を書き込み用ﾊﾞｯﾌｧにｺﾋﾟｰする
				for(i = 1; i<=4000; i++){
					I2C.WR_BUF[i] = I2C.RE_BUF[i];
				}
				
				LED.SEG_BUF[6]	= ' ';
				LED.SEG_BUF[7]	= ((SEQ.LED_BRIGHTNESS / 1000) % 10 + 0x30);
				LED.SEG_BUF[8]	= ((SEQ.LED_BRIGHTNESS / 100) % 10 + 0x30);
				LED.SEG_BUF[9]	= ((SEQ.LED_BRIGHTNESS / 10) % 10 + 0x30);
				LED.SEG_BUF[10]	= ((SEQ.LED_BRIGHTNESS / 1) % 10 + 0x30);
				
				I2C.WR_CONT = 0;
				
				if(RIIC0.ICCR2.BIT.BBSY == 0){
					RIIC0.ICCR2.BIT.ST = 1;			// ｽﾀｰﾄﾋﾞｯﾄ
					I2C.SUB_STATUS = 12;
					
					IR(RIIC0, ICEEI0) = 0;			// 割り込み要求ｸﾘｱ(通信ｴﾗｰ/ｲﾍﾞﾝﾄ発生)
					IEN(RIIC0, ICEEI0) = 1;			// 割り込み要求許可(通信ｴﾗｰ/ｲﾍﾞﾝﾄ発生)
				}
			}
			break;
			
		case 601:	// 換算ﾃｰﾌﾞﾙ設定(TPD→RX)
			SEQ.FLAG3.BIT.PARA_READ = 2;				// ﾊﾟﾗﾒｰﾀ読み出しﾌﾗｸﾞ
			break;
			
		case 602:	// 換算ﾃｰﾌﾞﾙ取得(RX→TPD)
			SEQ.FLAG3.BIT.PARA_WRITE = 2;				// ﾊﾟﾗﾒｰﾀ書き込みﾌﾗｸﾞ
			break;
	}
}

//************************************************************/
//				結果ｸﾘｱ
//************************************************************/
void clear_result(void)
{
	_UWORD i;
	
	//COM0.NO311 = 0;	// 311 状態番号
	COM0.NO312 = 0;	// 312 ｴﾗｰ番号
	// 結果ｸﾘｱ
	COM0.NO313 = 0;	// 313 計測結果 現在値(REAL)	上位
	COM0.NO314 = 0;	// 314 計測結果 現在値(REAL)	下位
	COM0.NO315 = 0;	// 315 計測結果 最終値(d)		上位
	COM0.NO316 = 0;	// 316 計測結果 最終値(d)		下位
	COM0.NO317 = 0;	// 317 計測結果 最終値(D)		上位
	COM0.NO318 = 0;	// 318 計測結果 最終値(D)		下位
	COM0.NO319 = 0;	// 319 計測結果 最終値(ΔX)		上位
	COM0.NO320 = 0;	// 320 計測結果 最終値(ΔX)		下位
	COM0.NO321 = 0;	// 321 計測結果 最終値(振れ)	上位
	COM0.NO322 = 0;	// 322 計測結果 最終値(振れ)	下位
	
	// 3000からﾊﾞｯﾌｧ数分をｸﾘｱする
	for(i=0; i<BUFFER_NUMBER; i++){
		COM0.NO3000[i] = 0;
	}
	
	// ﾊﾞｯﾌｧ初期化の設定
	// ﾊﾞｯﾌｧﾅﾝﾊﾞｰ旧をﾊﾞｯﾌｧ数最大値+1に設定し、ﾊﾞｯﾌｧﾅﾝﾊﾞｰ新をﾊﾞｯﾌｧ数最大値に設定する
	SEQ.BUFFER_NO_OLD = BUFFER_NUMBER+1;	// ﾊﾞｯﾌｧﾅﾝﾊﾞｰ旧
	SEQ.BUFFER_NO_NEW = BUFFER_NUMBER;		// ﾊﾞｯﾌｧﾅﾝﾊﾞｰ新
}

// add 2015.07.22 K.Uemura start	
short check_hardware_error(void)
{
	short retMode = ARGUMENT_ERROR;

	// 初期化確認
	if(SEQ.FLAG5.BIT.INITIALIZE_FLAG == 1){
		//初期化済み
		retMode = 0;
	}

	// 初期化完了していなくとも150番と、200番以上は有効
	// 動作に影響が出るコマンドは実行させない（200番未満はエラーとする）
	if((COM0.NO301 == 150) || (200 <= COM0.NO301)){
		retMode = 0;
	}

	return( retMode );
}
// add 2015.07.22 K.Uemura end

// add 2016.03.24 K.Uemura start	G32401
short check_execute_error(void)
{
	short retMode = 0;

	// エラー番号確認
	if(COM0.NO312 != 0){

		retMode = ARGUMENT_ERROR;

		if(100 <= COM0.NO301){
			//コマンド受付可
			retMode = 0;
		}
	}

	return( retMode );
}
// add 2016.03.24 K.Uemura end

//************************************************************/
//				ﾀｯﾁﾊﾟﾈﾙからの制御
//	main関数内の1msタイマで呼び出される
//
//	Case文の分岐
//	 1:EXEがONされた時
//	   32:ACKがOFF状態でRSTがONされたとき(case 1からの派生)
//	 2:状態番号(NO311)の設定
//      4:エラーの時
//	 3:各処理分岐
//	   14:プロファイル
//	    4:初期化、換算TBL
//	    8:バッファ転送完了
//	   10:電磁バルブ(カバー開閉)
//	   21:電磁バルブ(清掃エア)、スキップ出力確認
//	    5:タイムアウト
//	    7:RST ONのとき
//	21:500ms経過後、case 4へ
//	 6:EXEがOFF、ROFがOFFのときcase 1へ
//	 7:RST OFFのとき RDYをON、FINをOFF、case 6へ
//	 8:EXEがOFFのとき、RDYをON、FINをOFF、case 6へ
//
//	=============================================
//	 1 → 32 → 33 → 34 → 1
//	   →  1
//	   →  2 →  3 → 14 → 15 →  1
//	                           →  7 →  6
//	                     →  1
//	                     →  7
//	               →  4 →  5 →  6 →  1
//	               →  8 →  6
//	               → 10 → 11 → 12
//	               → 21 →  4
//	               →  5
//	               →  7
//	         →  4
//	13 →  4
//************************************************************/
void tp_control(void)
{
	short retMode = ARGUMENT_ERROR;

	switch(SEQ.TP_CONTROL_STATUS){
		// ①実行したいﾊﾟﾗﾒｰﾀ(動作番号)を入力	(ﾀｯﾁﾊﾟﾈﾙ)
		// ②指令(実行)をON						(ﾀｯﾁﾊﾟﾈﾙ)
		case 1:
			if(COM0.NO300.BIT.ACK == 0){								// ACKのﾋﾞｯﾄがOFFのとき
				if(COM0.NO300.BIT.RST){									// 強制ﾘｾｯﾄがONのとき
					SEQ.READY_PULSE_TIME = 1;							// READYのﾊﾟﾙｽ時間
					COM0.NO310.BIT.RDY = 0;								// READYのﾋﾞｯﾄをOFFにし状態番号をｾｯﾄする
					COM0.NO311 = 0;										// 0 状態番号なし
					COM0.NO312 = 0;										// 0 ｴﾗｰなし
					clear_7seg_led_error_no();							// 7セグLEDのｴﾗｰﾅﾝﾊﾞｰｸﾘｱ
					SEQ.TP_CONTROL_STATUS = 32;
				}else if(COM0.NO300.BIT.EXE){							// 司令(実行)がONのとき
// add 2016.07.26 K.Uemura start	G72601
					switch(COM0.NO301){
						case 200:	// COVER Open
							SEQ.MSEC_BUFFER[0][0] = SEQ.MSEC_COUNTER;
							break;
						case 153:	// 清掃確認
							SEQ.MSEC_BUFFER[5][0] = SEQ.MSEC_COUNTER;
							break;
						case 300:	//AIR ON
							SEQ.MSEC_BUFFER[20][0] = SEQ.MSEC_COUNTER;
							break;
						case 1:		//ORIGIN-X
						case 3:		//ORIGIN-Z
							SEQ.MSEC_BUFFER[25][0] = SEQ.MSEC_COUNTER;
							SEQ.MSEC_FLAG = 26;
							break;
						case 301:	//AIR OFF
							break;
						case 201:	// COVER Close
							break;
						default:
							break;
					}
// add 2016.07.26 K.Uemura end
					// 2000番未満のエラーコード時は実行を受け付けない
					if(check_hardware_error() == ARGUMENT_ERROR){
						SEQ.TP_CONTROL_STATUS = 1;
// add 2016.02.18 K.Uemura start	G21802
						COM0.NO312 = ERR_INITIALIZE;					// 1300	初期化未設定
						set_7seg_led_error_no(COM0.NO312);				// 7セグLEDのｴﾗｰﾅﾝﾊﾞｰ設定
// add 2016.02.18 K.Uemura end
						break;
					}

// add 2016.03.24 K.Uemura start	G32401
					if(check_execute_error() == ARGUMENT_ERROR){
						COM0.NO312 = ERR_MEASURE_START;					// 5000	測定開始不可
						set_7seg_led_error_no(COM0.NO312);				// 7セグLEDのｴﾗｰﾅﾝﾊﾞｰ設定
						break;
					}
// add 2016.03.24 K.Uemura end

// add 2016.03.21 K.Uemura start	G32101
					//実行bitの入力ﾀｲﾐﾝｸﾞで引数を設定
					argument_number_set();
// add 2016.03.21 K.Uemura end
					SEQ.TP_CONTROL_STATUS++;
				}
			}
//			COM0.NO310.BIT.DIR = 0;		// 走査方向(0:未検出 1:右ｴｯｼﾞ 2:左ｴｯｼﾞ 3:両ｴｯｼﾞ)
			break;
			
		// READYのﾎﾟｰﾘﾝｸﾞ時間待ち 
		case 32:
			if(SEQ.READY_PULSE_TIME == 0){								// ACKのﾋﾞｯﾄがOFFのとき
				SEQ.READY_PULSE_TIME = 1;								// READYのﾊﾟﾙｽ時間
				SEQ.TP_CONTROL_STATUS++;
			}
			break;
			
		// READY・完了を立ち上げる
		case 33:
			if(SEQ.READY_PULSE_TIME == 0){								// ACKのﾋﾞｯﾄがOFFのとき
				COM0.NO310.BIT.RDY = 1;
				COM0.NO310.BIT.FIN = 1;									// 完了
				SEQ.TP_CONTROL_STATUS++;
			}
			break;
			
		// ﾘｾｯﾄがOFFになると完了を下げる
		case 34:
			if(COM0.NO300.BIT.RST == 0){								// 強制ﾘｾｯﾄがONのとき
				COM0.NO310.BIT.FIN = 0;									// 完了
				SEQ.TP_CONTROL_STATUS = 1;
			}
			break;
			
		// ③結果ｸﾘｱ
		// ④指令を受けとりREADYをOFF後動作開始		(計測開始)
		// ⑤受け取った指令の状態を出力
		case 2:
			COM0.NO310.BIT.LED = 0;		// LEDﾌﾗｸﾞ
			COM0.NO310.BIT.DIR = 0;		// 走査方向(0:未検出 1:右ｴｯｼﾞ 2:左ｴｯｼﾞ 3:両ｴｯｼﾞ)
// add 2017.01.19 K.Uemura start	H11901	
			SEQ.ERROR_BEFORE = COM0.NO312;
// add 2017.01.19 K.Uemura end
			COM0.NO312 = 0;				// 312 ｴﾗｰ番号
			
			SEQ.END_TIMEOUT_PERIOD	= 0;			// ﾀｲﾑｱｳﾄ時間
			SEQ.OK_COUNT		= 0;				// OKｶｳﾝﾄ数
			SEQ.NG_COUNT		= 0;				// 連続NGｶｳﾝﾄ数
			
			state_number_set();											// 状態番号ｾｯﾄ(No.311)
			
			// 振れのとき
			if(SEQ.SELECT.BIT.MEASURE == MODE_RUNOUT){
// chg 2015.09.07 K.Uemura start	
				if((SEQ.FLUTES != 0) && (SEQ.SPINDLE_SPEED == 0)){
//				if(SEQ.SPINDLE_SPEED == 0){
// chg 2015.09.07 K.Uemura end
					COM0.NO312 = ERR_ZERO_SPINDLE_SPEED;					// 6000	「振れ」のとき回転数「0」
				}else if(999 < SEQ.SPINDLE_SPEED){
					COM0.NO312 = ERR_OUT_OF_SPINDLE_SPEED;					// 6001	「振れ」のとき回転数範囲外
// chg 2016.11.28 K.Uemura start	GB2801
				}else if(12 < SEQ.FLUTES){
//				}else if(9 < SEQ.FLUTES){
// chg 2016.11.28 K.Uemura end
					COM0.NO312 = ERR_OUT_OF_FLUTES;							// 6002	「振れ」のとき刃数範囲外
				}

				if(COM0.NO312 != 0){
					SEQ.FLAG.BIT.MEASUREMENT = MEASURE_STOP;							// 計測を停止する
					SEQ.TP_CONTROL_STATUS = 4;
					break;
				}
 			}
			state_number_control();										// 状態番号制御(No.311)

			// 計測以外のときはこの時点でREADYのﾋﾞｯﾄをOFFにする
			// 特定画素で計測中のとき
// chg 2016.10.20 K.Uemura start	GA2002
			if((COM0.NO301 == 700)||(COM0.NO301 == 701)||(COM0.NO301 == 710)||(COM0.NO301 == 711)){
//			if((COM0.NO301 == 700)||(COM0.NO301 == 701)){
// chg 2016.10.20 K.Uemura end
				if(SEQ.FLAG.BIT.MEASUREMENT == MEASURE_RUNNING){
					COM0.NO310.BIT.RDY = 0;									// READYのﾋﾞｯﾄをOFFにし状態番号をｾｯﾄする
				}
			}else{
				if(COM0.NO301 > 111){
					COM0.NO310.BIT.RDY = 0;									// READYのﾋﾞｯﾄをOFFにし状態番号をｾｯﾄする
// add 2016.07.26 K.Uemura start	G72601
					if(COM0.NO301 == 200){
						SEQ.MSEC_BUFFER[0][1] = SEQ.MSEC_COUNTER;
					}else if(COM0.NO301 == 153){
						SEQ.MSEC_BUFFER[5][1] = SEQ.MSEC_COUNTER;
					}else if(COM0.NO301 == 300){
						SEQ.MSEC_BUFFER[20][1] = SEQ.MSEC_COUNTER;
					}
// add 2016.07.26 K.Uemura end
				}
			}
			
			// ﾌﾟﾛﾌｧｲﾙのとき
			if(SEQ.SELECT.BIT.MEASURE == MODE_PROFILE){
// chg 2016.03.02 K.Uemura start	G30202
				if((COM0.NO311 != 400) && (COM0.NO311 != 401) && (COM0.NO311 != 402)){
//				if(COM0.NO311 != 400){
// chg 2016.03.02 K.Uemura end
					clear_result();											// 結果ｸﾘｱ
					SEQ.PROFILE_BUFFER_COUNT = 0;							// ﾌﾟﾛﾌｧｲﾙﾊﾞｯﾌｧ格納用ｶｳﾝﾄ
				}
			}
			
			SEQ.BUFFER_OUTPUT = 0;										// ﾊﾞｯﾌｧ出力状態
			SEQ.TP_CONTROL_STATUS++;
			break;
			
		// ⑥READYがOFFしていることを確認するとREADYOFF検知をON	(ﾀｯﾁﾊﾟﾈﾙ)
		// ⑦計測結果(現在値)の出力
		// ⑧計測結果(最終値)の出力
		case 3:
			if(COM0.NO300.BIT.EXE){										// 実行のﾋﾞｯﾄがONのとき
				// 工具径(自動)・工具径(4以下)・工具径(4より上　左側)・工具径(4より上　右側)・振れ測定・ﾌﾟﾛﾌｧｲﾙ・焦点合わせ・中心位置設定のとき
				// ※伸び測定除く
				// または、清掃確認(1回)のとき
// chg 2016.06.22 K.Uemura start	G62202
				if(((COM0.NO311 >= 10)&&(COM0.NO311 <= 111)&&(COM0.NO311 != 70))||(COM0.NO311 == 153)){
//				if((COM0.NO311 >= 10)&&(COM0.NO311 <= 111)&&(COM0.NO311 != 70)){
// chg 2016.06.22 K.Uemura end
					// 終了条件にて計測を終了したとき
					if(SEQ.FLAG.BIT.MEASUREMENT == MEASURE_STOP){				// 計測を停止する
						if(SEQ.FLAG6.BIT.HDI_CHECK_COMPLETION == 0){			// HDIﾁｪｯｸ完了ﾌﾗｸﾞが「0」のとき
							if(SEQ.SELECT.BIT.MEASURE == MODE_PROFILE)	SEQ.TP_CONTROL_STATUS = 14;		// ﾌﾟﾛﾌｧｲﾙのとき
							else										SEQ.TP_CONTROL_STATUS = 4;
						}
					}
				}
				//
				
				// ﾊﾞｯﾌｧ取得（転送）のとき
				if((400 <= COM0.NO311) && (COM0.NO311 <= 405)){
					if(SEQ.FLAG.BIT.BUFFER_TRANSFER == 0){
						if(SEQ.BUFFER_OUTPUT == 1){
							if(COM0.NO299 == 0){
								SEQ.BUFFER_OUTPUT = 0;					// ﾊﾞｯﾌｧ出力状態
								COM0.NO310.BIT.RDY = 1;
								COM0.NO310.BIT.FIN = 1;					// 完了
								SEQ.TP_CONTROL_STATUS = 8;
							}
						}
					}
				}

				// 電磁ﾊﾞﾙﾌﾞのとき
				if((COM0.NO301 == 200) || (COM0.NO301 == 201)){
					SEQ.TP_CONTROL_STATUS = 10;
				}

				if((250 <= COM0.NO301)&&(COM0.NO301 <= 301)){
					if(COM0.NO301 == 300){
						COM0.NO310.BIT.PUR = 1;		// 電磁ﾊﾞﾙﾌﾞ2(清掃ｴｱ状態) ON
// add 2016.07.26 K.Uemura start	G72601
						SEQ.MSEC_BUFFER[20][2] = SEQ.MSEC_COUNTER;
// add 2016.07.26 K.Uemura end
					}else if(COM0.NO301 == 301){
						COM0.NO310.BIT.PUR = 0;		// 電磁ﾊﾞﾙﾌﾞ2(清掃ｴｱ状態) OFF
					}
					else if((250 <= COM0.NO301)&&(COM0.NO301 <= 263)){
						switch(COM0.NO301){
							case 250:	SEQ.HDI.BIT.B0 = 1;	break;		// HDI0 ON
							case 251:	SEQ.HDI.BIT.B0 = 0;	break;		// HDI0 OFF
							case 252:	SEQ.HDI.BIT.B1 = 1;	break;		// HDI1 ON
							case 253:	SEQ.HDI.BIT.B1 = 0;	break;		// HDI1 OFF
							case 254:	SEQ.HDI.BIT.B2 = 1;	break;		// HDI2 ON
							case 255:	SEQ.HDI.BIT.B2 = 0;	break;		// HDI2 OFF
							case 256:	SEQ.HDI.BIT.B3 = 1;	break;		// HDI3 ON
							case 257:	SEQ.HDI.BIT.B3 = 0;	break;		// HDI3 OFF
							case 258:	SEQ.HDI.BIT.B4 = 1;	break;		// HDI4 ON
							case 259:	SEQ.HDI.BIT.B4 = 0;	break;		// HDI4 OFF
							case 260:	SEQ.HDI.BIT.B5 = 1;	break;		// HDI5 ON
							case 261:	SEQ.HDI.BIT.B5 = 0;	break;		// HDI5 OFF
							case 262:	SEQ.HDI.BIT.B0 = 1;
										SEQ.HDI.BIT.B1 = 1;
										SEQ.HDI.BIT.B2 = 1;
										SEQ.HDI.BIT.B3 = 1;
										SEQ.HDI.BIT.B4 = 1;
										SEQ.HDI.BIT.B5 = 1;	break;		// HDI0-5 ON
							case 263:	SEQ.HDI.BIT.B0 = 0;
										SEQ.HDI.BIT.B1 = 0;
										SEQ.HDI.BIT.B2 = 0;
										SEQ.HDI.BIT.B3 = 0;
										SEQ.HDI.BIT.B4 = 0;
										SEQ.HDI.BIT.B5 = 0;	break;		// HDI0-5 OFF
						}
					}
					
					SEQ.FIN_COUNT = 0;										// 完了待機ｶｳﾝﾀ
					SEQ.TP_CONTROL_STATUS = 21;
				}

				// 初期化のとき
				if(COM0.NO311 == 150){
					SEQ.TP_CONTROL_STATUS = 4;
				}

				if(COM0.NO300.BIT.ROF){										// READY OFF検知
					SEQ.COM_TIMEOUT_PERIOD = 0;
					// ﾊﾞｯﾌｧ取得（転送）のとき
					if((400 <= COM0.NO311) && (COM0.NO311 <= 405)){
						if(SEQ.BUFFER_OUTPUT == 0){
							if(COM0.NO310.BIT.FIN == 0){					// 完了
								SEQ.BUFFER_OUTPUT = 1;						// ﾊﾞｯﾌｧ出力状態
								SEQ.FLAG.BIT.BUFFER_TRANSFER = 1;			// ﾊﾞｯﾌｧ転送ﾌﾗｸﾞ
							}
						}
					}
				}else{
					// ﾀｲﾑｱｳﾄ時間(通信設定)(ms)がﾀｲﾑｱｳﾄ時間ｾｯﾄ(通信設定)(ms)以上になったとき
					if(SEQ.COM_TIMEOUT_PERIOD >= COM_TIMEOUT_PERIOD_SET){
						SEQ.FLAG.BIT.MEASUREMENT = MEASURE_STOP;					// 計測を停止する
						SEQ.TP_CONTROL_STATUS = 5;
						COM0.NO312 = ERR_TP_TIMEOUT;					// 3003	ﾀｯﾁﾊﾟﾈﾙ通信のﾀｲﾑｱｳﾄ
					}
				}
				
				if(COM0.NO300.BIT.RST){									// 強制ﾘｾｯﾄがONのとき

					// ﾊﾞｯﾌｧ取得（転送）のとき
					if((400 <= COM0.NO311) && (COM0.NO311 <= 405)){
						if(SEQ.BUFFER_OUTPUT == 1){
							SEQ.BUFFER_OUTPUT = 0;						// ﾊﾞｯﾌｧ出力状態
							SEQ.FLAG.BIT.BUFFER_TRANSFER = 1;			// ﾊﾞｯﾌｧ転送ﾌﾗｸﾞ
						}
					}
					COM0.NO311 = 0;										// 何もしていない
					COM0.NO312 = 0;										// 0 ｴﾗｰなし
					
					SEQ.FLAG.BIT.MEASUREMENT = MEASURE_STOP;						// 計測を停止する
					SEQ.TP_CONTROL_STATUS = 7;
				}
				
				if(COM0.NO311 == 600){									// 換算ﾃｰﾌﾞﾙ自動設定のとき
					if(COM0.NO310.BIT.RDY == 0){						// READYのﾋﾞｯﾄがOFFのとき
						SEQ.TP_CONTROL_STATUS = 4;
					}
				}
				
			}else{														// 実行のﾋﾞｯﾄがOFFのとき
				if((400 <= COM0.NO311) && (COM0.NO311 <= 405)){
					if(SEQ.BUFFER_OUTPUT == 1){
						SEQ.BUFFER_OUTPUT = 0;							// ﾊﾞｯﾌｧ出力状態
						SEQ.FLAG.BIT.BUFFER_TRANSFER = 1;				// ﾊﾞｯﾌｧ転送ﾌﾗｸﾞ
					}
				}
				SEQ.FLAG.BIT.MEASUREMENT = MEASURE_STOP;							// 計測を停止する
				SEQ.TP_CONTROL_STATUS = 5;
			}
			
			// ﾌﾟﾛﾌｧｲﾙで測定を停止しているとき
			if(SEQ.SELECT.BIT.MEASURE == MODE_PROFILE){
				if(SEQ.FLAG.BIT.MEASUREMENT == MEASURE_STOP){
// chg 2016.03.02 K.Uemura start	G30202
					if((COM0.NO311 != 400) && (COM0.NO311 != 401) && (COM0.NO311 != 402)){
//					if(COM0.NO311 != 400){
// chg 2016.03.02 K.Uemura end
						SEQ.TP_CONTROL_STATUS = 14;		// ﾌﾟﾛﾌｧｲﾙのとき
					}
				}
				
				if(COM0.NO310.BIT.STR == 1){
// chg 2016.03.02 K.Uemura start	G30202
					if((COM0.NO311 != 400) && (COM0.NO311 != 401) && (COM0.NO311 != 402)){
//					if(COM0.NO311 != 400){
// chg 2016.03.02 K.Uemura end
						SEQ.TP_CONTROL_STATUS = 14;		// ﾌﾟﾛﾌｧｲﾙのとき
					}
				}
			}
			
			break;
			
		// 電磁ﾊﾞﾙﾌﾞのとき
		case 21:
			SEQ.FIN_COUNT++;											// 完了待機ｶｳﾝﾀ
			if(SEQ.FIN_COUNT >= 500){									// READYのﾋﾞｯﾄがONのとき
				SEQ.FIN_COUNT = 0;										// 完了待機ｶｳﾝﾀ
				COM0.NO310.BIT.RDY = 1;									// READYのﾋﾞｯﾄをONにしFINをｾｯﾄする
// add 2016.07.26 K.Uemura start	G72601
				if(COM0.NO301 == 200){
					SEQ.MSEC_BUFFER[0][4] = SEQ.MSEC_COUNTER;
				}else if(COM0.NO301 == 300){
					SEQ.MSEC_BUFFER[20][3] = SEQ.MSEC_COUNTER;
				}
// add 2016.07.26 K.Uemura end
				SEQ.TP_CONTROL_STATUS = 4;
			}
			break;
			
		// ⑨完了信号の出力		(計測終了)
		// READY・完了のﾋﾞｯﾄをONにする
		case 4:
			if(COM0.NO310.BIT.RDY == 1){								// READYのﾋﾞｯﾄがONのとき
				COM0.NO310.BIT.FIN = 1;									// 完了
// add 2016.07.26 K.Uemura start	G72601
				if(COM0.NO301 == 200){
					SEQ.MSEC_BUFFER[0][5] = SEQ.MSEC_COUNTER;
				}else if(COM0.NO301 == 300){
					SEQ.MSEC_BUFFER[20][4] = SEQ.MSEC_COUNTER;
				}
// add 2016.07.26 K.Uemura end
				SEQ.TP_CONTROL_STATUS++;
			}
// chg 2017.03.30 K.Uemura start	
			if(COM0.NO300.BIT.EXE == 0){
				SEQ.FLAG.BIT.MEASUREMENT = MEASURE_STOP;							// 計測を停止する
				SEQ.TP_CONTROL_STATUS = 5;
			}
// chg 2017.03.30 K.Uemura end
			break;
			
		// ﾌﾟﾛﾌｧｲﾙのとき
		// ⑨ACK信号のONを確認後、STROBE信号をOFF
		// STROBE信号をOFFにする
		case 14:
			if(COM0.NO300.BIT.ACK == 1){								// ACKのﾋﾞｯﾄがONのとき
				COM0.NO310.BIT.STR = 0;									// STROBE信号をOFF
				SEQ.TP_CONTROL_STATUS++;
			}
			
			if(COM0.NO300.BIT.EXE == 0){								// 実行
				if(COM0.NO300.BIT.ROF == 0){							// READY OFF検知
					COM0.NO310.BIT.STR = 0;								// STROBE信号をOFF
					COM0.NO310.BIT.RDY = 1;
					SEQ.TP_CONTROL_STATUS = 1;
				}
			}
			
			if(COM0.NO300.BIT.RST){										// 強制ﾘｾｯﾄがONのとき
				COM0.NO311 = 0;											// 何もしていない
				COM0.NO312 = 0;											// 0 ｴﾗｰなし
				
				COM0.NO310.BIT.FIN = 1;									// 完了
				COM0.NO310.BIT.RDY = 1;
				
				COM0.NO310.BIT.STR = 0;									// STROBE信号をOFF
				
				SEQ.FLAG.BIT.MEASUREMENT = MEASURE_STOP;							// 計測を停止する
				SEQ.TP_CONTROL_STATUS = 7;
			}
			break;
			
		// ﾌﾟﾛﾌｧｲﾙのとき
		// ACK信号がOFFのとき
		case 15:
// chg 2016.11.11 K.Uemura start	GB1001
#if	1
			if(COM0.NO300.BIT.ACK == 0){								// ACKのﾋﾞｯﾄがOFFのとき
				SEQ.FLAG6.BIT.PROFILE_PROCESSING = 1;					// ﾌﾟﾛﾌｧｲﾙ処理ﾌﾗｸﾞ
			}

			if(COM0.NO300.BIT.EXE == 0){								// 実行
				SEQ.TP_CONTROL_STATUS = 1;
			}
#else
			if(COM0.NO300.BIT.ACK == 0){								// ACKのﾋﾞｯﾄがOFFのとき
				SEQ.FLAG6.BIT.PROFILE_PROCESSING = 1;					// ﾌﾟﾛﾌｧｲﾙ処理ﾌﾗｸﾞ
			}else{
				if(COM0.NO300.BIT.EXE == 0){// 実行
					SEQ.TP_CONTROL_STATUS = 1;
				}
			}
#endif
// chg 2016.11.11 K.Uemura end
			
			if(COM0.NO300.BIT.RST){										// 強制ﾘｾｯﾄがONのとき
				COM0.NO311 = 0;											// 何もしていない
				COM0.NO312 = 0;											// 0 ｴﾗｰなし
				
				COM0.NO310.BIT.STR = 0;									// STROBE信号をOFF
				
				SEQ.FLAG.BIT.MEASUREMENT = MEASURE_STOP;							// 計測を停止する
				SEQ.TP_CONTROL_STATUS = 7;
			}
			break;
			
		// ⑩完了信号のONを確認すると指令をOFF
		// 司令(実行)がOFFのとき完了のﾋﾞｯﾄをOFFにする
		case 5:
			if(COM0.NO300.BIT.RST){										// 強制ﾘｾｯﾄがONのとき
				COM0.NO312 = 0;											// 0 ｴﾗｰなし
				clear_7seg_led_error_no();								// 7セグLEDのｴﾗｰﾅﾝﾊﾞｰｸﾘｱ
			}
			
			if(COM0.NO312 != 0){
				set_7seg_led_error_no(COM0.NO312);						// 7セグLEDのｴﾗｰﾅﾝﾊﾞｰ設定
			}
			
			if(COM0.NO300.BIT.EXE == 0){								// 実行
				if(COM0.NO300.BIT.ROF == 0){							// READY OFF検知
// add 2017.01.19 K.Uemura start	H11901	
					if((COM0.NO311 == 506) && (SEQ.ERROR_BEFORE != 0)){
						COM0.NO312 = SEQ.ERROR_BEFORE;					// ｴﾗｰ番号(前回)
					}
// add 2017.01.19 K.Uemura end
					COM0.NO310.BIT.FIN = 0;								// 完了
					SEQ.TP_CONTROL_STATUS++;
				}
			}
			break;
			
		// ⑪指令のOFFを確認後READYをON・完了信号をOFF
		// ⑫入力した指令のOFF
		// 司令(実行・READY OFF検知)が共にOFFのときｼｰｹﾝｽを最初に戻す
		case 6:
			if(COM0.NO300.BIT.RST){										// 強制ﾘｾｯﾄがONのとき
				COM0.NO312 = 0;											// 0 ｴﾗｰなし
			}
			
			SEQ.PROFILE_BUFFER_COUNT = 0;								// ﾌﾟﾛﾌｧｲﾙﾊﾞｯﾌｧ格納用ｶｳﾝﾄ
			
			if(COM0.NO300.BIT.EXE == 0){								// 実行のﾋﾞｯﾄがONのとき
				if(COM0.NO300.BIT.ROF == 0){							// READY OFF検知のとき
					SEQ.TP_CONTROL_STATUS = 1;
				}
			}
			break;
			
		// 強制ﾘｾｯﾄ時強制ﾘｾｯﾄがOFFのときREADYをON、完了信号をOFF
		case 7:
			if(COM0.NO300.BIT.RST == 0){								// 強制ﾘｾｯﾄがOFFのとき
				COM0.NO310.BIT.RDY = 1;
				COM0.NO310.BIT.FIN = 0;									// 完了
				SEQ.TP_CONTROL_STATUS = 6;
			}
			break;
			
		case 8:
			if(COM0.NO300.BIT.EXE == 0){								// 実行のﾋﾞｯﾄがOFFのとき
				COM0.NO310.BIT.RDY = 1;
				COM0.NO310.BIT.FIN = 0;									// 完了
				SEQ.TP_CONTROL_STATUS = 6;
			}
			break;

// add 2015.12.22 K.Uemura start	FB2201
		//ｶﾊﾞｰ開閉指令の受付時、ｾﾝｻ状態を監視する
		case 10:
			SEQ.TP_CONTROL_STATUS = 21;
			SEQ.FLAG5.BIT.MOVE_COVER = 0;
// chg 2016.03.21 K.Uemura start	G32101
			retMode = get_cover_status_initilized();
//			retMode = get_cover_status();
// chg 2016.03.21 K.Uemura end

			if(COVER_CONTROL == 1){
				// コントロールボックスでカバー開閉制御

				if(retMode == COVER_STATUS_MOVED){				// ｶﾊﾞｰｾﾝｻ反応無し
					COM0.NO312 = ERR_COVER_MOVED;
				}else if(retMode == COVER_STATUS_BREAK){		// ｶﾊﾞｰｾﾝｻClose／Open状態
					COM0.NO312 = ERR_COVER_BROKEN;
				}else if(retMode == COVER_STATUS_CLOSE){		// ｶﾊﾞｰｾﾝｻClose状態
					if(COM0.NO301 == 200){
						//ｶﾊﾞｰOpen指令
						SEQ.FLAG5.BIT.MOVE_COVER = 1;
						SEQ.TP_CONTROL_STATUS = 11;
						COM0.NO310.BIT.COV = 1;					// 電磁ﾊﾞﾙﾌﾞ1(ｶﾊﾞｰ状態) ON(開 Open)
// add 2016.07.26 K.Uemura start	G72601
						SEQ.MSEC_BUFFER[0][2] = SEQ.MSEC_COUNTER;
// add 2016.07.26 K.Uemura end
					}else{
						//ｶﾊﾞｰClose指令
						if(COM0.NO310.BIT.COV == 1){			// 電磁ﾊﾞﾙﾌﾞ1(ｶﾊﾞｰ状態) ON(開 Open)
							COM0.NO312 = ERR_COVER_CLOSE;
						}
					}
				}else if(retMode == COVER_STATUS_OPEN){			// ｶﾊﾞｰｾﾝｻOpen状態
					if(COM0.NO301 == 200){
						//ｶﾊﾞｰOpen指令
						if(COM0.NO310.BIT.COV == 0){			// 電磁ﾊﾞﾙﾌﾞ1(ｶﾊﾞｰ状態) OFF(閉 Close)
							COM0.NO312 = ERR_COVER_OPEN;
						}
					}else{
						//ｶﾊﾞｰClose指令
						SEQ.FLAG5.BIT.MOVE_COVER = 1;
						SEQ.TP_CONTROL_STATUS = 11;
						COM0.NO310.BIT.COV = 0;					// 電磁ﾊﾞﾙﾌﾞ1(ｶﾊﾞｰ状態) OFF(閉 Close)
					}
				}
			}else{
				SEQ.TP_CONTROL_STATUS = 11;
			}
			break;

		case 11:
			SEQ.COVER_COUNT = 1;							// ｶﾊﾞｰ処理開始
			SEQ.TP_CONTROL_STATUS = 12;
			break;

		case 12:
			// コントロールボックスでカバー制御しない
			// カバー制御する場合は、cover_process関数でシーケンスup
			if(COVER_CONTROL == 0){
				SEQ.TP_CONTROL_STATUS = 13;
			}
			break;

		case 13:
			//エラー発生時、電磁弁出力を戻す
			if(COM0.NO312 != 0){
				if(COM0.NO301 == 200){
					COM0.NO310.BIT.COV = 0;		// 電磁ﾊﾞﾙﾌﾞ1(ｶﾊﾞｰ状態) OFF
				}else{
					COM0.NO310.BIT.COV = 1;		// 電磁ﾊﾞﾙﾌﾞ1(ｶﾊﾞｰ状態) ON
				}
			}

			COM0.NO310.BIT.RDY = 1;									// READYのﾋﾞｯﾄをOFFにし状態番号をｾｯﾄする
// add 2016.07.26 K.Uemura start	G72601
			if(COM0.NO301 == 200){
				SEQ.MSEC_BUFFER[0][4] = SEQ.MSEC_COUNTER;
			}else if(COM0.NO301 == 300){
				SEQ.MSEC_BUFFER[20][3] = SEQ.MSEC_COUNTER;
			}
			SEQ.MSEC_FLAG = 0;
// add 2016.07.26 K.Uemura end
			SEQ.TP_CONTROL_STATUS = 4;
			break;
// add 2015.12.22 K.Uemura end
	}
}

// add 2015.12.22 K.Uemura start	FB2201
// カバー状態
//┌───┬───┬───┐
//│Close │ Open │ 状態 │
//├───┼───┼───┤
//│  0   │  0   │移動中│
//│  0   │  1   │ Open │
//│  1   │  0   │Close │
//│  1   │  1   │ 故障 │
//└───┴───┴───┘
short get_cover_status(void)
{
	short retMode = ARGUMENT_ERROR;

	if((COM0.NO310.BIT.CSC == 0) && (COM0.NO310.BIT.CSO == 0)){
		retMode = COVER_STATUS_MOVED;								//移動中
	}else
	if((COM0.NO310.BIT.CSC == 0) && (COM0.NO310.BIT.CSO == 1)){
		retMode = COVER_STATUS_OPEN;								//Open
	}else
	if((COM0.NO310.BIT.CSC == 1) && (COM0.NO310.BIT.CSO == 0)){
		retMode = COVER_STATUS_CLOSE;								//Close
	}else
	if((COM0.NO310.BIT.CSC == 1) && (COM0.NO310.BIT.CSO == 1)){
		retMode = COVER_STATUS_BREAK;								//故障
	}

	return( retMode );
}
// add 2015.12.22 K.Uemura end

// add 2016.03.21 K.Uemura start	G32101
// カバー状態
//┌───┬───┬───┐
//│Close │ Open │ 状態 │
//├───┼───┼───┤
//│  0   │  0   │移動中│
//│  0   │  1   │ Open │
//│  1   │  0   │Close │
//│  1   │  1   │ 故障 │
//└───┴───┴───┘
short get_cover_status_initilized(void)
{
	short retMode = ARGUMENT_ERROR;

	if((SEQ.COVER_CLOSE_SENSOR == 0) && (SEQ.COVER_OPEN_SENSOR == 0)){
		retMode = COVER_STATUS_MOVED;								//移動中
	}else
	if((SEQ.COVER_CLOSE_SENSOR == 0) && (SEQ.COVER_OPEN_SENSOR == 1)){
		retMode = COVER_STATUS_OPEN;								//Open
	}else
	if((SEQ.COVER_CLOSE_SENSOR == 1) && (SEQ.COVER_OPEN_SENSOR == 0)){
		retMode = COVER_STATUS_CLOSE;								//Close
	}else
	if((SEQ.COVER_CLOSE_SENSOR == 1) && (SEQ.COVER_OPEN_SENSOR == 1)){
		retMode = COVER_STATUS_BREAK;								//故障
	}
	return( retMode );
}
// add 2016.03.21 K.Uemura end

// add 2016.03.21 K.Uemura start	G32101
void argument_number_set(void)
{
	if(SEQ.FLAG.BIT.PORTABLE == OPERATION_AUTO){					// 通信操作の場合
		SEQ.END_CONDITION = COM0.NO300.BIT.ECD;						// 300.11  終了条件
// add 2016.12.06 K.Uemura start	GC0602
		SEQ.FLAG6.BIT.ROUGH_SCAN = COM0.NO300.BIT.RSN;				// 300.13  粗測定bit
// add 2016.12.06 K.Uemura end

		SEQ.SPINDLE_SPEED = ((COM0.NO303 << 16) | COM0.NO304);		// 303&304 回転数
		SEQ.FLUTES = COM0.NO305;									// 305     刃数
		SEQ.RADIUS = COM0.NO306;									// 306     工具径
		SEQ.EXTEND_CYCLE = COM0.NO307;								// 307     引数

		SEQ.COVER_OPEN_SENSOR = COM0.NO310.BIT.CSO;					// 310.12  ｶﾊﾞｰｾﾝｻOpen
		SEQ.COVER_CLOSE_SENSOR = COM0.NO310.BIT.CSC;				// 310.13  ｶﾊﾞｰｾﾝｻClose
	}
}
// add 2016.03.21 K.Uemura end


