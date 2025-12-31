/******************************************************************************
* File Name	: fpga_conversion.c
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

void send_to_fpga_conversion(void);					// FPGAへのﾃﾞｰﾀ送信関数(換算ﾃﾞｰﾀ取得)



//************************************************************/
//				FPGAへのﾃﾞｰﾀ送信関数(換算ﾃﾞｰﾀ取得)
//	以下１番の処理だとタクトが追いつかないため２番を採用
//	１．毎回結果格納→総和算出→平均計算
//	２．毎回結果の総和→平均計算
//************************************************************/
// 換算ﾃﾞｰﾀ取得
void send_to_fpga_conversion(void)
{
	long temp, ave_number;
// add 2016.07.15 K.Uemura start	G71501
	_UBYTE	table_No;		//	テーブルNo
// add 2016.07.15 K.Uemura end

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
			send_to_dbus_zero();						// ﾃﾞｰﾀ出力関数0
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
			if(SEQ.CBUS_NUMBER == 390){				// 指令ｺﾏﾝﾄﾞが待機のとき
				SEQ.FLAG.BIT.AFTER_STOPPING = 1;	// 計測停止後1ｻｲｸﾙ取得ﾌﾗｸﾞ
				SEQ.CHANGE_FPGA = 9;				// 計測停止
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
				C_ACK_OUT	= 1;					// C_ACK
				C_ACK_OUT	= 0;					// C_ACK
			}
			break;
			
// chg 2016.12.05 K.Uemura start	GC0502
#if	0
		case 14:	// 割り込み後
			if(SEQ.CBUS_NUMBER == 211){									// 左ｴｯｼﾞ最小位置
				// 浮動小数点のﾃﾞｰﾀに変換し値を表示する
				Variable.lLong = SEQ.INPUT_DBUS_LONG;
				SEQ.INPUT_DBUS = Variable.fFloat;
				COM0.NO101 = SEQ.INPUT_DBUS * 10;
				if(SEQ.MEASUREMENT_DIRECTION == X_DIRECTION){			// 計測方向がX方向のとき
					// ﾃｰﾌﾞﾙのｱﾄﾞﾚｽが偶数のとき
					if((COM0.NO302 & 0x0001) == 0){							// 「2」で割った余りが「0」のとき
						COM0.NO314 = SEQ.INPUT_DBUS * 10;
						RESULT.LARGE_D_MIN[0] = (COM0.NO314 < RESULT.LARGE_D_MIN[0]) ? COM0.NO314 : RESULT.LARGE_D_MIN[0];
						COM0.NO316 = RESULT.LARGE_D_MIN[0];
						RESULT.LARGE_D_MAX[0] = (RESULT.LARGE_D_MAX[0] < COM0.NO314) ? COM0.NO314 : RESULT.LARGE_D_MAX[0];
						COM0.NO318 = RESULT.LARGE_D_MAX[0];
						COM0.NO320 = RESULT.LARGE_D_MAX[0] - RESULT.LARGE_D_MIN[0];
						if(SEQ.TABLE_TEMP_COUNT < X_AVERAGE_NUMBER){	// ｶｳﾝﾄが平均数未満のとき
							SEQ.TABLE_TEMP_COUNT++;						// ﾃｰﾌﾞﾙ一時ﾃﾞｰﾀｶｳﾝﾄ
							SEQ.TABLE_TEMP_DATA[SEQ.TABLE_TEMP_COUNT] = SEQ.TABLE_TEMP_DATA[SEQ.TABLE_TEMP_COUNT-1] + SEQ.INPUT_DBUS;			// ﾃｰﾌﾞﾙ一時ﾃﾞｰﾀ
						}
					}
				}
				
			}else if(SEQ.CBUS_NUMBER == 212){							// 右ｴｯｼﾞ最大位置
				// 浮動小数点のﾃﾞｰﾀに変換し値を表示する
				Variable.lLong = SEQ.INPUT_DBUS_LONG;
				SEQ.INPUT_DBUS = Variable.fFloat;
				COM0.NO102 = SEQ.INPUT_DBUS * 10;
				
				// 計測方向がX方向のとき
				if(SEQ.MEASUREMENT_DIRECTION == X_DIRECTION){
					// ﾃｰﾌﾞﾙのｱﾄﾞﾚｽが奇数のとき
					if((COM0.NO302 & 0x0001) == 1){							// 「2」で割った余りが「1」のとき
						COM0.NO314 = SEQ.INPUT_DBUS * 10;
						RESULT.LARGE_D_MIN[0] = (COM0.NO314 < RESULT.LARGE_D_MIN[0]) ? COM0.NO314 : RESULT.LARGE_D_MIN[0];
						COM0.NO316 = RESULT.LARGE_D_MIN[0];
						RESULT.LARGE_D_MAX[0] = (RESULT.LARGE_D_MAX[0] < COM0.NO314) ? COM0.NO314 : RESULT.LARGE_D_MAX[0];
						COM0.NO318 = RESULT.LARGE_D_MAX[0];
						COM0.NO320 = RESULT.LARGE_D_MAX[0] - RESULT.LARGE_D_MIN[0];
						if(SEQ.TABLE_TEMP_COUNT < X_AVERAGE_NUMBER){		// ｶｳﾝﾄが平均数未満のとき
							SEQ.TABLE_TEMP_COUNT++;							// ﾃｰﾌﾞﾙ一時ﾃﾞｰﾀｶｳﾝﾄ
							SEQ.TABLE_TEMP_DATA[SEQ.TABLE_TEMP_COUNT] = SEQ.TABLE_TEMP_DATA[SEQ.TABLE_TEMP_COUNT-1] + SEQ.INPUT_DBUS;			// ﾃｰﾌﾞﾙ一時ﾃﾞｰﾀ
						}
					}
					
				// 計測方向がZ方向のとき
				}else{
					COM0.NO314 = SEQ.INPUT_DBUS * 10;
					RESULT.LARGE_D_MIN[0] = (COM0.NO314 < RESULT.LARGE_D_MIN[0]) ? COM0.NO314 : RESULT.LARGE_D_MIN[0];
					COM0.NO316 = RESULT.LARGE_D_MIN[0];
					RESULT.LARGE_D_MAX[0] = (RESULT.LARGE_D_MAX[0] < COM0.NO314) ? COM0.NO314 : RESULT.LARGE_D_MAX[0];
					COM0.NO318 = RESULT.LARGE_D_MAX[0];
					COM0.NO320 = RESULT.LARGE_D_MAX[0] - RESULT.LARGE_D_MIN[0];
					if(SEQ.TABLE_TEMP_COUNT < Z_AVERAGE_NUMBER){			// ｶｳﾝﾄが平均数未満のとき
						SEQ.TABLE_TEMP_COUNT++;								// ﾃｰﾌﾞﾙ一時ﾃﾞｰﾀｶｳﾝﾄ
						SEQ.TABLE_TEMP_DATA[SEQ.TABLE_TEMP_COUNT] = SEQ.TABLE_TEMP_DATA[SEQ.TABLE_TEMP_COUNT-1] + SEQ.INPUT_DBUS;			// ﾃｰﾌﾞﾙ一時ﾃﾞｰﾀ
					}
				}
				
			}else if(SEQ.CBUS_NUMBER == 223){				// ｽｷｯﾌﾟ出力
				set_result_skip();							// SKIP結果の設定
			}
#else
		case 14:	// 割り込み後
			// 浮動小数点のﾃﾞｰﾀに変換し値を表示する
			Variable.lLong = SEQ.INPUT_DBUS_LONG;
			SEQ.INPUT_DBUS = Variable.fFloat;

			switch(SEQ.CBUS_NUMBER){
				case 211:		// 左ｴｯｼﾞ座標
					COM0.NO101 = SEQ.INPUT_DBUS * 10;
					COM0.NO103 = 99999;
					break;

				case 212:		// 右ｴｯｼﾞ座標
					COM0.NO102 = SEQ.INPUT_DBUS * 10;
					COM0.NO104 = 99999;
					break;

				case 213:		// 左ｴｯｼﾞ傾斜
					RESULT.FOCUS_LEFT[0] = SEQ.INPUT_DBUS;
					SEQ.INPUT_DBUS_LONG = SEQ.INPUT_DBUS * 1000;
					COM0.NO103 = SEQ.INPUT_DBUS_LONG;
					break;

				case 214:		// 右ｴｯｼﾞ傾斜
					RESULT.FOCUS_RIGHT[0] = SEQ.INPUT_DBUS;
					SEQ.INPUT_DBUS_LONG = SEQ.INPUT_DBUS * 1000;
					COM0.NO104 = SEQ.INPUT_DBUS_LONG;
					break;

				case 216:		// 平均値・最小値・最大値
					break;

				case 223:		// ｽｷｯﾌﾟ出力
					set_result_skip();							// SKIP結果の設定
					COM0.NO314 = 0;

// add 2016.12.05 K.Uemura start	GC0501
					// 傾き「5」未満の場合エッジ座標格納
					if((SEQ.MEASUREMENT_DIRECTION == X_DIRECTION) && ((COM0.NO302 & 0x0001) == 0)){
						// 計測方向がX方向で、ﾃｰﾌﾞﾙのｱﾄﾞﾚｽが偶数（「2」で割った余りが「0」）のとき
						if((COM0.NO103 != 0) && (COM0.NO103 < 5000)){
							COM0.NO314 = COM0.NO101;
						}
					}else{
						if((COM0.NO104 != 0) && (COM0.NO104 < 5000)){
							COM0.NO314 = COM0.NO102;
						}
					}
// add 2016.12.15 K.Uemura end

					SEQ.TOTAL_COUNT++;					// 計測総数

					// 傾き判定
					if(COM0.NO314 != 0){
						SEQ.OK_COUNT++;					// OKｶｳﾝﾄ数

						RESULT.LARGE_D_MIN[0] = (COM0.NO314 < RESULT.LARGE_D_MIN[0]) ? COM0.NO314 : RESULT.LARGE_D_MIN[0];
						COM0.NO316 = RESULT.LARGE_D_MIN[0];
						RESULT.LARGE_D_MAX[0] = (RESULT.LARGE_D_MAX[0] < COM0.NO314) ? COM0.NO314 : RESULT.LARGE_D_MAX[0];
						COM0.NO318 = RESULT.LARGE_D_MAX[0];
						COM0.NO320 = RESULT.LARGE_D_MAX[0] - RESULT.LARGE_D_MIN[0];

						if(SEQ.TABLE_TEMP_COUNT < X_AVERAGE_NUMBER){		// ｶｳﾝﾄが平均数未満のとき
							if(SEQ.TABLE_TEMP_COUNT == 0){
								SEQ.TABLE_TEMP_DATA[SEQ.TABLE_TEMP_COUNT] = COM0.NO314;
							}else{
#if	0
								// 平均計算
								//	１．毎回結果格納→総和算出→平均計算
								SEQ.TABLE_TEMP_DATA[SEQ.TABLE_TEMP_COUNT] = COM0.NO314;			// ﾃｰﾌﾞﾙ一時ﾃﾞｰﾀ
#else
								// 平均計算
								//	２．毎回結果の総和→平均計算
								SEQ.TABLE_TEMP_DATA[SEQ.TABLE_TEMP_COUNT] = SEQ.TABLE_TEMP_DATA[SEQ.TABLE_TEMP_COUNT-1] + COM0.NO314;			// ﾃｰﾌﾞﾙ一時ﾃﾞｰﾀ
#endif
							}
							SEQ.TABLE_TEMP_COUNT++;
						}
					}
					break;
			}
#endif
// chg 2016.12.05 K.Uemura end

			SEQ.FPGA_SEND_STATUS++;
			break;
			
		// C_ACKを「H」にする
		case 15:
			SEQ.FPGA_SEND_STATUS++;
			C_ACK_OUT	= 1;								// C_ACK
			break;
			
		// C_ACKを「L」にする
		case 16:
			C_ACK_OUT	= 0;								// C_ACK
			SEQ.FPGA_SEND_STATUS = 11;
			
			//if(SEQ.CBUS_NUMBER == 215){					// 右ｴｯｼﾞﾃﾞｰﾀのとき
			if(SEQ.CBUS_NUMBER == SEQ.LAST_CBUS_NUMBER){	// CBUSﾅﾝﾊﾞｰ(1ｻｲｸﾙ最終)のとき
			
				SEQ.FPGA_SEND_STATUS = 21;
				
				// 100ms間隔でﾃﾞｰﾀ送信	add 140424
				if(SEQ.FLAG.BIT.BUFFER_RESET == 1){			// ﾊﾞｯﾌｧｶｳﾝﾄが「0」のとき
					SEQ.FLAG.BIT.BUFFER_RESET = 0;			// ﾊﾞｯﾌｧﾘｾｯﾄﾌﾗｸﾞ
				}
				//
				
				if(SEQ.MEASUREMENT_DIRECTION == X_DIRECTION)	ave_number = X_AVERAGE_NUMBER;		// 計測方向がX方向のとき
				else											ave_number = Z_AVERAGE_NUMBER;		// 計測方向がZ方向のとき
				
				if(SEQ.TABLE_TEMP_COUNT >= ave_number){		// ｶｳﾝﾄが平均数のとき
				
					OUT.SUB_STATUS = 5;						// START実行中表示の停止
// chg 2016.12.05 K.Uemura start	GC0502
#if	0
					// 平均計算
					//	１．毎回結果格納→総和算出→平均計算
					temp = 0;
					for(idx=0 ; idx < ave_number; idx++ ){
						temp += SEQ.TABLE_TEMP_DATA[idx];
					}
					temp /= ave_number;
#else
					// 平均計算
					//	２．毎回結果の総和→平均計算
					temp = (SEQ.TABLE_TEMP_DATA[SEQ.TABLE_TEMP_COUNT-1]) / ave_number;
#endif
// chg 2016.12.05 K.Uemura end
					
					COM0.NO321 = temp >> 16;
					COM0.NO322 = temp;
					
					set_7seg_led_upper( temp, 1 );			// 7ｾｸﾞ表示(上段)
					set_7seg_lower_no_data();				// 7ｾｸﾞ表示(-----)(下段)

// add 2016.07.15 K.Uemura start	G71501
					// ﾃｰﾌﾞﾙNo
					table_No = (COM0.NO302 >> 0x01) & 0x0FFF;		//	ﾃｰﾌﾞﾙNo(※下位1bitは利用しない)
// add 2016.07.15 K.Uemura end

					// ﾒﾓﾘへﾃﾞｰﾀの書き込み
					// 計測方向がX方向のとき
					if(SEQ.MEASUREMENT_DIRECTION == X_DIRECTION){
						if((COM0.NO302 & 0x0001) == 0){										// 「2」で割った余りが「0」のとき
							// ｴｯｼﾞ 左(実寸換算ﾃｰﾌﾞﾙ)	(ROM:4096 - 6095)	(ﾒﾓﾘﾘﾝｸ:3000-3999)	(ﾃｰﾌﾞﾙ:0,2,4,-,1998)
// chg 2016.07.15 K.Uemura start	G71501
							SEQ.TABLE_EDGE_LEFT[table_No]	= temp;
//							SEQ.TABLE_EDGE_LEFT[(COM0.NO302 / 2)]	= temp;
// chg 2016.07.15 K.Uemura end
							I2C.WR_CONT = COM0.NO302 + 4096;
						}else{
							// ｴｯｼﾞ 右(実寸換算ﾃｰﾌﾞﾙ)	(ROM:6096 - 8095)	(ﾒﾓﾘﾘﾝｸ:4000-4999)	(ﾃｰﾌﾞﾙ:1,3,5,-,1999)
// chg 2016.07.15 K.Uemura start	G71501
							SEQ.TABLE_EDGE_RIGHT[table_No]	= temp;
//							SEQ.TABLE_EDGE_RIGHT[(COM0.NO302 / 2)]	= temp;
// chg 2016.07.15 K.Uemura end
							I2C.WR_CONT = COM0.NO302 + 6095;
						}
					// 計測方向がZ方向のとき
					}else{
						// ｴｯｼﾞ 右(実寸換算ﾃｰﾌﾞﾙ)	(ROM:8096 - 10095)	(ﾒﾓﾘﾘﾝｸ:5000-5999)	(ﾃｰﾌﾞﾙ:32769,32771,-)
// chg 2016.07.15 K.Uemura start	G71501
						SEQ.Z_TABLE_EDGE_RIGHT[table_No]	= temp;
						I2C.WR_CONT = table_No*2 + 8096;
//						SEQ.Z_TABLE_EDGE_RIGHT[((COM0.NO302 - 32768) / 2)]	= temp;
//						I2C.WR_CONT = ((COM0.NO302 - 32768) / 2)*2 + 8096;
// chg 2016.07.15 K.Uemura end
					}
					
					I2C.WR_BUF[I2C.WR_CONT]		= temp >> 8;
					I2C.WR_BUF[I2C.WR_CONT + 1]	= temp;
					
					I2C.RE_BUF[I2C.WR_CONT]		= I2C.WR_BUF[I2C.WR_CONT];
					I2C.RE_BUF[I2C.WR_CONT + 1]	= I2C.WR_BUF[I2C.WR_CONT + 1];
					
					SEQ.FLAG.BIT.MEASUREMENT = 0;
					I2C.SUB_STATUS = 48;					// 1ﾃﾞｰﾀ分書き込み
					//
				}
			}
			break;
			
		// C_ACKを「H」にする
		case 17:
			SEQ.FPGA_SEND_STATUS = 13;
			C_ACK_OUT	= 1;								// C_ACK
			break;
			
		// F_PRIO_INが「L」になっていることを確認する
		case 21:
			//if(F_PRIO_IN == 0){							// F_PRIO_INが「L」のとき
				SEQ.FPGA_SEND_STATUS = 11;
			//}
			
			if(SEQ.FLAG.BIT.AFTER_STOPPING == 0){
				if(F_PRIO_IN == 0){							// F_PRIO_INが「L」のとき
					if(SEQ.FLAG.BIT.MEASUREMENT == 0){
						// 待機を送信(390)
						C_PRIO_OUT	= 1;					// C_PRIO
						SEQ.CBUS_NUMBER = 390;
						SEQ.FPGA_SEND_STATUS = 2;
					}
				}
			}else{
				SEQ.FLAG.BIT.AFTER_STOPPING = 0;
				SEQ.FPGA_SEND_STATUS = 71;
				SEQ.CHANGE_FPGA = 0;
				DA.DADR0 = 0;								// DA0
				
// chg 2016.08.09 K.Uemura start	G80901
#if	1
				if(COM0.NO300.BIT.RST){					// 強制ﾘｾｯﾄがONのとき
					COM0.NO310.BIT.FIN = 1;				// 完了
				}else{
					COM0.NO310.BIT.RDY = 1;				// READY
					COM0.NO310.BIT.FIN = 1;
				}
#else
//				// ﾌﾟﾛﾌｧｲﾙのとき
//				if(SEQ.SELECT.BIT.MEASURE == MODE_PROFILE){
//					if(COM0.NO300.BIT.EXE){					// 実行
//						COM0.NO310.BIT.STR = 1;				// STROBE
//						SEQ.INPUT_DBUS_LONG = RESULT.SMALL_D_MAX[0] * 10.0;
//						COM0.NO3000[SEQ.PROFILE_BUFFER_COUNT] = SEQ.INPUT_DBUS_LONG;
//						
//						SEQ.BUFFER_NO_NEW = SEQ.PROFILE_BUFFER_COUNT;	// ﾊﾞｯﾌｧﾅﾝﾊﾞｰ新
//						SEQ.PROFILE_BUFFER_COUNT++;						// ﾌﾟﾛﾌｧｲﾙﾊﾞｯﾌｧ格納用ｶｳﾝﾄ
//						if(SEQ.PROFILE_BUFFER_COUNT >= BUFFER_NUMBER)	SEQ.PROFILE_BUFFER_COUNT = 0;
//						
//					}else{
//						COM0.NO310.BIT.RDY = 1;				// READY
//					}
//				}else{
//					if(COM0.NO300.BIT.RST){					// 強制ﾘｾｯﾄがONのとき
//						COM0.NO310.BIT.FIN = 1;				// 完了
//					}else{
//						COM0.NO310.BIT.RDY = 1;				// READY
//						COM0.NO310.BIT.FIN = 1;
//					}
//				}
#endif
// chg 2016.08.09 K.Uemura end
			}
			break;
			
		case 22:
			if(SEQ.FLAG.BIT.BUFFER_RESET == 1){				// ﾊﾞｯﾌｧｶｳﾝﾄが「0」のとき
				max_min_reset();
				SEQ.FLAG.BIT.BUFFER_RESET = 0;				// ﾊﾞｯﾌｧﾘｾｯﾄﾌﾗｸﾞ
				SEQ.FPGA_SEND_STATUS = 11;
			}
			break;
	}
}
