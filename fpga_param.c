/******************************************************************************
* File Name	: fpga_param.c
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

void send_to_fpga(void);				// FPGAへのﾃﾞｰﾀ送信関数
void parameter_for_tpd(void);			// ﾀｯﾁﾊﾟﾈﾙ用ﾊﾟﾗﾒｰﾀ
void set_hdi_data(void);				// HDIﾃﾞｰﾀｾｯﾄ	ADD 170204
void set_parameter_to_dbus(void);		// DBUSにﾊﾟﾗﾒｰﾀをｾｯﾄ

//************************************************************/
//				FPGAへのﾃﾞｰﾀ送信関数(ﾊﾟﾗﾒｰﾀ)
//	1 → ・・・ → 8 → 9 → 3 → 
//	                 → 11 → 12 → 13 → 14 [12]ベリファイ(send_to_fpga_verify)
//	                       → 1 → 
//************************************************************/
void send_to_fpga(void)
{
	switch(SEQ.FPGA_SEND_STATUS){
		// RX → FPGAにﾃﾞｰﾀ送信
		// C_PRIOを「H」にする
		case 1:
			//if(F_PRIO_IN == 0){
				SEQ.FPGA_PARAM_COUNT = 0;				// FPGAﾊﾟﾗﾒｰﾀ再送信ｶｳﾝﾄ add 150508
				
				C_PRIO_OUT	= 1;					// C_PRIO
				SEQ.CBUS_NUMBER		= 0;			// CBUSﾅﾝﾊﾞｰをﾘｾｯﾄ
				SEQ.FPGA_SEND_STATUS++;
			//}
			break;
			
		// ﾎﾟｰﾄを出力に設定する
		case 2:
			// ここで方向/ﾀｲﾌﾟ別のﾊﾟﾗﾒｰﾀ設定を行う
			parameter_for_tpd();					// ﾀｯﾁﾊﾟﾈﾙ用ﾊﾟﾗﾒｰﾀ
			
			// HDI更新ﾌﾗｸﾞが「1」のとき	ADD 170204
			if(SEQ.FLAG6.BIT.HDI_UPDATE == 1)	set_hdi_data();
			
			//SEQ.HDI_DATA_COUNT = 0;					// HDIﾃﾞｰﾀｶｳﾝﾀ
			bus_to_out();							// ﾊﾞｽ出力変更関数
			
			send_to_cbus_zero();						// ｺﾏﾝﾄﾞﾅﾝﾊﾞｰ出力関数0
			send_to_dbus_zero();						// ﾃﾞｰﾀ出力関数0
			
			SEQ.FPGA_SEND_STATUS++;
			break;
			
		// ｺﾏﾝﾄﾞﾊﾞｽ・ﾃﾞｰﾀﾊﾞｽを設定する
		case 3:
			SEQ.CBUS_NUMBER++;						// CBUSﾅﾝﾊﾞｰ
			set_parameter_to_dbus();				// DBUSにﾊﾟﾗﾒｰﾀをｾｯﾄ
			send_to_cbus(SEQ.CBUS_NUMBER);
			send_to_dbus(SEQ.INPUT_DBUS_LONG);
			SEQ.FPGA_SEND_STATUS++;
			break;
			
		// C_ACKを「H」にする
		case 4:
			C_ACK_OUT	= 1;							// C_ACK
			SEQ.FPGA_SEND_STATUS++;
			break;
			
		// C_ACKを「L」にする
		case 5:
			C_ACK_OUT	= 0;							// C_ACK
			SEQ.FPGA_SEND_STATUS++;
			break;
			
		// ｺﾏﾝﾄﾞﾊﾞｽ・ﾃﾞｰﾀﾊﾞｽを「L」にする
		case 6:
			send_to_cbus_zero();						// ｺﾏﾝﾄﾞﾅﾝﾊﾞｰ出力関数0
			send_to_dbus_zero();						// ﾃﾞｰﾀ出力関数0
			SEQ.FPGA_SEND_STATUS++;
			if(SEQ.CBUS_NUMBER < 380){					// CBUSﾅﾝﾊﾞｰが「380」未満のとき
				SEQ.FPGA_SEND_STATUS++;
			}
			break;
			
		// ﾎﾟｰﾄを入力に設定する
		case 7:
			bus_to_in();								// ﾊﾞｽ入力変更関数
			SEQ.FPGA_SEND_STATUS++;
			break;
			
		// C_PRIOを「L」にする
		case 8:
			C_PRIO_OUT	= 0;							// C_PRIO
			if(SEQ.CBUS_NUMBER < 380){					// CBUSﾅﾝﾊﾞｰが「380」未満のとき
				SEQ.FPGA_SEND_STATUS++;
			}else{
				SEQ.FPGA_SEND_STATUS = 11;
			}
			break;
			
		// C_PRIOを「H」にする
		case 9:
			C_PRIO_OUT	= 1;							// C_PRIO
			//if(SEQ.CBUS_NUMBER == 41){					// CBUSﾅﾝﾊﾞｰが「41」のとき
			if(SEQ.CBUS_NUMBER == 42){					// CBUSﾅﾝﾊﾞｰが「42」のとき
				SEQ.CBUS_NUMBER = 379;
			}
			SEQ.FPGA_SEND_STATUS = 3;
			break;
			
		// FPGA → RXにﾃﾞｰﾀ送信
		// F_PRIO_INが「H」になったらC_ACKを「H」にする
		case 11:
			if(C_ACK_OUT == 1){
				C_ACK_OUT	= 0;
			}else{
				if(F_PRIO_IN == 1){
					C_ACK_OUT	= 1;				// C_ACK
					C_ACK_OUT	= 0;				// C_ACK
					SEQ.FPGA_SEND_STATUS++;
				}
				// add 150508
				// F_PRIO_INが1にならないときはｺﾏﾝﾄﾞを再送する
				else{
					SEQ.FPGA_PARAM_COUNT++;				// FPGAﾊﾟﾗﾒｰﾀ再送信ｶｳﾝﾄ
					if(SEQ.FPGA_PARAM_COUNT >= 10){		// ｶｳﾝﾀが10になったとき
						SEQ.FPGA_SEND_STATUS = 1;
					}
				}
				//
			}
			break;
			
		case 12:	// 割り込み	
			if(SEQ.CBUS_NUMBER == 200){				// 初期ﾊﾟﾗﾒｰﾀｾｯﾄ完了のとき
				//SEQ.CBUS_NUMBER = 81;				// ﾌﾞﾗｯｸ固定値設定
				//DA.DADR0 = 0;						// DA0
				SEQ.FPGA_SEND_COUNT = 0;			// FPGA再送信ｶｳﾝﾄをﾘｾｯﾄ
			}else if(SEQ.CBUS_NUMBER == 201){		// ﾌﾞﾗｯｸ固定値設定完了のとき
				if(SEQ.FPGA_SEND_COUNT < 5){ 		// 5回送信する
					SEQ.CBUS_NUMBER = 381;			// ﾌﾞﾗｯｸ固定値設定
					DA.DADR0 = 0;					// DA0
					SEQ.FPGA_SEND_COUNT++;
				}else{
					SEQ.CBUS_NUMBER = 383;			// ﾎﾜｲﾄ固定値設定
					DA.DADR0 = SEQ.LED_BRIGHTNESS;	// DA0
					SEQ.FPGA_SEND_COUNT = 0;
				}
			}else if(SEQ.CBUS_NUMBER == 203){		// ﾎﾜｲﾄ固定値設定完了のとき
				if(SEQ.FPGA_SEND_COUNT < 5){ 		// 5回送信する
					SEQ.CBUS_NUMBER = 383;			// ﾎﾜｲﾄ固定値設定
					DA.DADR0 = SEQ.LED_BRIGHTNESS;	// DA0
					SEQ.FPGA_SEND_COUNT = 0;
				}else{
					SEQ.CBUS_NUMBER = 390;			// 
					DA.DADR0 = 0;					// DA0
					SEQ.FPGA_SEND_COUNT = 0;
				}
			}
			SEQ.FPGA_SEND_STATUS++;
			break;
			
		// C_ACKを「H」にする
		case 13:
			SEQ.FPGA_SEND_STATUS++;
			C_ACK_OUT	= 1;						// C_ACK
			break;
			
		// F_PRIO_INが「L」になったらﾎﾟｰﾄを入力に設定する
		case 14:
			C_ACK_OUT	= 0;						// C_ACK
// chg 2016.02.18 K.Uemura start	G21804
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
			}else{
				DA.DADR0 = 0;						// DA0
			}

//			DA.DADR0 = 0;							// DA0
// chg 2016.02.18 K.Uemura end
			
			SEQ.CHANGE_FPGA = 12;					// ﾊﾟﾗﾒｰﾀ比較
			SEQ.FPGA_SEND_STATUS = 1;
			break;
			
		// C_PRIOを「H」にする
		case 15:
			if(F_PRIO_IN == 0){
				C_PRIO_OUT	= 1;					// C_PRIO
				SEQ.FPGA_SEND_STATUS++;
			}
			break;
			
		// ﾎﾟｰﾄを出力に設定する
		case 16:
			bus_to_out();							// ﾊﾞｽ出力変更関数
			SEQ.FPGA_SEND_STATUS++;
			break;
			
		// ｺﾏﾝﾄﾞﾊﾞｽ・ﾃﾞｰﾀﾊﾞｽを設定する
		case 17:
			send_to_cbus(SEQ.CBUS_NUMBER);
			send_to_dbus_zero();						// ﾃﾞｰﾀ出力関数0
			SEQ.FPGA_SEND_STATUS++;
			
			break;
			
		// C_ACKを「H」にする
		case 18:
			C_ACK_OUT	= 1;						// C_ACK
			SEQ.FPGA_SEND_STATUS++;
			break;
			
		// C_ACKを「L」にする
		case 19:
			C_ACK_OUT	= 0;						// C_ACK
			SEQ.FPGA_SEND_STATUS++;
			break;
			
		// C_PRIO・ｺﾏﾝﾄﾞﾊﾞｽ・ﾃﾞｰﾀﾊﾞｽを「L」にする
		case 20:
			send_to_cbus_zero();						// ｺﾏﾝﾄﾞﾅﾝﾊﾞｰ出力関数0
			send_to_dbus_zero();						// ﾃﾞｰﾀ出力関数0
			SEQ.FPGA_SEND_STATUS++;
			break;
			
		case 21:
			bus_to_in();							// ﾊﾞｽ入力変更関数
			SEQ.FPGA_SEND_STATUS++;
			break;
			
		// ﾎﾟｰﾄを入力に設定する
		case 22:
			C_PRIO_OUT	= 0;						// C_PRIO
			if(SEQ.CBUS_NUMBER != 390){
				SEQ.FPGA_SEND_STATUS = 11;
			}else{
				SEQ.FPGA_SEND_STATUS++;
			}
			break;
			
		case 23:
			SEQ.CHANGE_FPGA = 0;
			DA.DADR0 = 0;							// DA0
			break;
	}
}

//************************************************************/
//				ﾀｯﾁﾊﾟﾈﾙ用ﾊﾟﾗﾒｰﾀ
//************************************************************/
// 計測方向/ﾀｲﾌﾟ別にﾊﾟﾗﾒｰﾀ設定を行う
void parameter_for_tpd(void)
{
	// 計測方向
	SEQ.PARA_DATA[1]	= SEQ.MEASUREMENT_DIRECTION;		// 0:X方向		1:Z方向
	
	// 計測ﾀｲﾌﾟ
	SEQ.PARA_DATA[2]	= IN.FLAG.BIT.HARDWARE_TYPE;		// 0:ﾎﾟｰﾀﾌﾞﾙ	1:CNC
	
	// 露光時間
	// 計測ﾀｲﾌﾟがﾎﾟｰﾀﾌﾞﾙ版でECOのとき
	if((IN.FLAG.BIT.HARDWARE_TYPE == PORTABLE_EDITION)&&(SEQ.FLAG.BIT.ECO == 1)){
		SEQ.PARA_DATA[3]	= ECO_EXPOSURE_TIME;			// 露光時間(ECO)
		
	// 計測ﾀｲﾌﾟがCNC版で計測方向がZ方向のとき
	}else if((IN.FLAG.BIT.HARDWARE_TYPE == CNC_EDITION)&&(SEQ.MEASUREMENT_DIRECTION == Z_DIRECTION)){
		SEQ.PARA_DATA[3]	= Z_EXPOSURE_TIME;				// 露光時間(Z)
		
	// 上記以外
	}else{
		SEQ.PARA_DATA[3]	= X_EXPOSURE_TIME;				// 露光時間(X)
	}
	
	// 黒比率傾斜・黒比率傾斜小ﾚﾍﾞﾙ・SKIP係数・荒ｴｯｼﾞｻｰﾁ係数・HDI
	// 計測ﾀｲﾌﾟがCNC版で計測方向がZ方向のとき
	if((IN.FLAG.BIT.HARDWARE_TYPE == CNC_EDITION)&&(SEQ.MEASUREMENT_DIRECTION == Z_DIRECTION)){
		SEQ.PARA_DATA[4]	= Z_BLACK_RATIO_LARGE;			// 黒比率傾斜大ﾚﾍﾞﾙ
		SEQ.PARA_DATA[5]	= Z_BLACK_RATIO_SMALL;			// 黒比率傾斜小ﾚﾍﾞﾙ
		SEQ.PARA_DATA[6]	= Z_SKIP_COEFFICIENT;			// SKIP係数
		SEQ.PARA_DATA[7]	= Z_ROUGH_EDGE_COEFFICIENT;		// 荒ｴｯｼﾞｻｰﾁ係数
		
		SEQ.PARA_DATA[8]	= Z_HDI0_START;					// HDI0 開始
		SEQ.PARA_DATA[9]	= Z_HDI0_END;					// HDI0 開始
		SEQ.PARA_DATA[10]	= Z_HDI1_START;					// HDI1 開始
		SEQ.PARA_DATA[11]	= Z_HDI1_END;					// HDI1 終了
		SEQ.PARA_DATA[12]	= Z_HDI2_START;					// HDI2 開始
		SEQ.PARA_DATA[13]	= Z_HDI2_END;					// HDI2 終了
		SEQ.PARA_DATA[14]	= Z_HDI3_START;					// HDI3 開始
		SEQ.PARA_DATA[15]	= Z_HDI3_END;					// HDI3 終了
		SEQ.PARA_DATA[16]	= Z_HDI4_START;					// HDI4 開始
		SEQ.PARA_DATA[17]	= Z_HDI4_END;					// HDI4 終了
		SEQ.PARA_DATA[18]	= Z_HDI5_START;					// HDI5 開始
		SEQ.PARA_DATA[19]	= Z_HDI5_END;					// HDI5 終了
		
	// 上記以外
	}else{
		SEQ.PARA_DATA[4]	= X_BLACK_RATIO_LARGE;			// 黒比率傾斜大ﾚﾍﾞﾙ
		SEQ.PARA_DATA[5]	= X_BLACK_RATIO_SMALL;			// 黒比率傾斜小ﾚﾍﾞﾙ
		SEQ.PARA_DATA[6]	= X_SKIP_COEFFICIENT;			// SKIP係数
		SEQ.PARA_DATA[7]	= X_ROUGH_EDGE_COEFFICIENT;		// 荒ｴｯｼﾞｻｰﾁ係数
		
		SEQ.PARA_DATA[8]	= X_HDI0_START;					// HDI0 開始
		SEQ.PARA_DATA[9]	= X_HDI0_END;					// HDI0 開始
		SEQ.PARA_DATA[10]	= X_HDI1_START;					// HDI1 開始
		SEQ.PARA_DATA[11]	= X_HDI1_END;					// HDI1 終了
		SEQ.PARA_DATA[12]	= X_HDI2_START;					// HDI2 開始
		SEQ.PARA_DATA[13]	= X_HDI2_END;					// HDI2 終了
		SEQ.PARA_DATA[14]	= X_HDI3_START;					// HDI3 開始
		SEQ.PARA_DATA[15]	= X_HDI3_END;					// HDI3 終了
		SEQ.PARA_DATA[16]	= X_HDI4_START;					// HDI4 開始
		SEQ.PARA_DATA[17]	= X_HDI4_END;					// HDI4 終了
		SEQ.PARA_DATA[18]	= X_HDI5_START;					// HDI5 開始
		SEQ.PARA_DATA[19]	= X_HDI5_END;					// HDI5 終了
	}
	
	// LED
	// 計測ﾀｲﾌﾟがCNC版のとき
	if(IN.FLAG.BIT.HARDWARE_TYPE == CNC_EDITION){
		// 計測方向がZ方向のとき
		if(SEQ.MEASUREMENT_DIRECTION == Z_DIRECTION){
			SEQ.PARA_DATA[20]	= Z_LED1_START;				// LED1 開始
			SEQ.PARA_DATA[21]	= Z_LED1_END;				// LED1 終了
			SEQ.PARA_DATA[22]	= Z_LED2_START;				// LED2 開始
			SEQ.PARA_DATA[23]	= Z_LED2_END;				// LED2 終了
			SEQ.PARA_DATA[24]	= Z_LED3_START;				// LED3 開始
			SEQ.PARA_DATA[25]	= Z_LED3_END;				// LED3 終了
			SEQ.PARA_DATA[26]	= Z_LED4_START;				// LED4 開始
			SEQ.PARA_DATA[27]	= Z_LED4_END;				// LED4 終了
			
			SEQ.PARA_DATA[28]	= 0xFFFF;					// LED5 開始
			SEQ.PARA_DATA[29]	= 0xFFFF;					// LED5 終了
			SEQ.PARA_DATA[30]	= 0xFFFF;					// LED6 開始
			SEQ.PARA_DATA[31]	= 0xFFFF;					// LED6 終了
			SEQ.PARA_DATA[32]	= 0xFFFF;					// LED7 開始
			SEQ.PARA_DATA[33]	= 0xFFFF;					// LED7 終了
			SEQ.PARA_DATA[34]	= 0xFFFF;					// LED8 開始
			SEQ.PARA_DATA[35]	= 0xFFFF;					// LED8 終了
			SEQ.PARA_DATA[36]	= 0xFFFF;					// LED9 開始
			SEQ.PARA_DATA[37]	= 0xFFFF;					// LED9 終了
			SEQ.PARA_DATA[38]	= 0xFFFF;					// LED10 開始
			SEQ.PARA_DATA[39]	= 0xFFFF;					// LED10 終了
			SEQ.PARA_DATA[40]	= 0xFFFF;					// LED11 開始
			SEQ.PARA_DATA[41]	= 0xFFFF;					// LED11 終了
			
		}else{
			SEQ.PARA_DATA[20]	= X_LED1_START;				// LED1 開始
			SEQ.PARA_DATA[21]	= X_LED1_END;				// LED1 終了
			SEQ.PARA_DATA[22]	= X_LED2_START;				// LED2 開始
			SEQ.PARA_DATA[23]	= X_LED2_END;				// LED2 終了
			SEQ.PARA_DATA[24]	= X_LED3_START;				// LED3 開始
			SEQ.PARA_DATA[25]	= X_LED3_END;				// LED3 終了
			SEQ.PARA_DATA[26]	= X_LED4_START;				// LED4 開始
			SEQ.PARA_DATA[27]	= X_LED4_END;				// LED4 終了
			SEQ.PARA_DATA[28]	= X_LED5_START;				// LED5 開始
			SEQ.PARA_DATA[29]	= X_LED5_END;				// LED5 終了
			SEQ.PARA_DATA[30]	= X_LED6_START;				// LED6 開始
			SEQ.PARA_DATA[31]	= X_LED6_END;				// LED6 終了
			SEQ.PARA_DATA[32]	= X_LED7_START;				// LED7 開始
			SEQ.PARA_DATA[33]	= X_LED7_END;				// LED7 終了
			SEQ.PARA_DATA[34]	= X_LED8_START;				// LED8 開始
			SEQ.PARA_DATA[35]	= X_LED8_END;				// LED8 終了
			SEQ.PARA_DATA[36]	= X_LED9_START;				// LED9 開始
			SEQ.PARA_DATA[37]	= X_LED9_END;				// LED9 終了
			
			SEQ.PARA_DATA[38]	= 0xFFFF;					// LED10 開始
			SEQ.PARA_DATA[39]	= 0xFFFF;					// LED10 終了
			SEQ.PARA_DATA[40]	= 0xFFFF;					// LED11 開始
			SEQ.PARA_DATA[41]	= 0xFFFF;					// LED11 終了
		}
	}else{
		SEQ.PARA_DATA[20]	= LED1_START;					// LED1 開始
		SEQ.PARA_DATA[21]	= LED1_END;						// LED1 終了
		SEQ.PARA_DATA[22]	= LED2_START;					// LED2 開始
		SEQ.PARA_DATA[23]	= LED2_END;						// LED2 終了
		SEQ.PARA_DATA[24]	= LED3_START;					// LED3 開始
		SEQ.PARA_DATA[25]	= LED3_END;						// LED3 終了
		SEQ.PARA_DATA[26]	= LED4_START;					// LED4 開始
		SEQ.PARA_DATA[27]	= LED4_END;						// LED4 終了
		SEQ.PARA_DATA[28]	= LED5_START;					// LED5 開始
		SEQ.PARA_DATA[29]	= LED5_END;						// LED5 終了
		SEQ.PARA_DATA[30]	= LED6_START;					// LED6 開始
		SEQ.PARA_DATA[31]	= LED6_END;						// LED6 終了
		SEQ.PARA_DATA[32]	= LED7_START;					// LED7 開始
		SEQ.PARA_DATA[33]	= LED7_END;						// LED7 終了
		SEQ.PARA_DATA[34]	= LED8_START;					// LED8 開始
		SEQ.PARA_DATA[35]	= LED8_END;						// LED8 終了
		SEQ.PARA_DATA[36]	= LED9_START;					// LED9 開始
		SEQ.PARA_DATA[37]	= LED9_END;						// LED9 終了
		SEQ.PARA_DATA[38]	= LED10_START;					// LED10 開始
		SEQ.PARA_DATA[39]	= LED10_END;					// LED10 終了
		SEQ.PARA_DATA[40]	= LED11_START;					// LED11 開始
		SEQ.PARA_DATA[41]	= LED11_END;					// LED11 終了
	}
	
	// 特定画素値の出力のとき
	if((COM0.NO301 == 700)||(COM0.NO301 == 701)){
		SEQ.PARA_DATA[42]	= COM0.NO302;					// 0 - 4095
	}else{
		SEQ.PARA_DATA[42]	= 0x1000;						// 4096
	}
	
	// 計測方向がZ方向のとき
	if(SEQ.MEASUREMENT_DIRECTION == Z_DIRECTION){
		SEQ.IDLE_COUNT_SET				= Z_IDLE_COUNT;					// 空回しｶｳﾝﾄｾｯﾄ
		SEQ.TUNING_ENABLE				= Z_TUNING_ENABLE;				// 傾斜ﾁｭｰﾆﾝｸﾞ有無
		SEQ.TUNING_SECONDS				= Z_TUNING_SECONDS;				// ﾁｭｰﾆﾝｸﾞ秒数
		SEQ.TUNING_OFFSET				= Z_TUNING_OFFSET;				// ﾁｭｰﾆﾝｸﾞｵﾌｾｯﾄ
		SEQ.TUNING_RATIO_LOWER_LIMIT	= Z_TUNING_RATIO_LOWER_LIMIT;	// ﾁｭｰﾆﾝｸﾞ比率下限
		SEQ.TUNING_RATIO_UPPER_LIMIT	= Z_TUNING_RATIO_UPPER_LIMIT;	// ﾁｭｰﾆﾝｸﾞ比率上限
		SEQ.MANUAL_GRADIENT				= Z_MANUAL_GRADIENT;			// 手動傾斜画素
		SEQ.FOCUSING_GRADIENT			= Z_FOCUSING_GRADIENT;			// 焦点合わせ傾斜
		SEQ.ORIGIN_THRESHOLD			= Z_ORIGIN_THRESHOLD;			// ORIGIN(ｴｯｼﾞ考慮)しきい値
		
		SEQ.LED_BRIGHTNESS				= SEQ.Z_LED_BRIGHTNESS;			// LED輝度値を設定
		
	// 計測方向がX方向のとき
	}else{
		SEQ.IDLE_COUNT_SET				= X_IDLE_COUNT;					// 空回しｶｳﾝﾄｾｯﾄ
		SEQ.TUNING_ENABLE				= X_TUNING_ENABLE;				// 傾斜ﾁｭｰﾆﾝｸﾞ有無
		SEQ.TUNING_SECONDS				= X_TUNING_SECONDS;				// ﾁｭｰﾆﾝｸﾞ秒数
		SEQ.TUNING_OFFSET				= X_TUNING_OFFSET;				// ﾁｭｰﾆﾝｸﾞｵﾌｾｯﾄ
		SEQ.TUNING_RATIO_LOWER_LIMIT	= X_TUNING_RATIO_LOWER_LIMIT;	// ﾁｭｰﾆﾝｸﾞ比率下限
		SEQ.TUNING_RATIO_UPPER_LIMIT	= X_TUNING_RATIO_UPPER_LIMIT;	// ﾁｭｰﾆﾝｸﾞ比率上限
		SEQ.MANUAL_GRADIENT				= X_MANUAL_GRADIENT;			// 手動傾斜画素
		SEQ.FOCUSING_GRADIENT			= X_FOCUSING_GRADIENT;			// 焦点合わせ傾斜
		SEQ.ORIGIN_THRESHOLD			= X_ORIGIN_THRESHOLD;			// ORIGIN(ｴｯｼﾞ考慮)しきい値
		
		SEQ.LED_BRIGHTNESS				= SEQ.X_LED_BRIGHTNESS;			// LED輝度値を設定
	}
}

//************************************************************/
//				HDIﾃﾞｰﾀｾｯﾄ	ADD 170204
//************************************************************/
void set_hdi_data(void)
{
	_UBYTE i;
	float temp;
	
	SEQ.FLAG6.BIT.HDI_UPDATE = 0;	// HDI更新ﾌﾗｸﾞ	ADD 170204
	
	// 計測方向がX方向のとき
	// 換算ﾃｰﾌﾞﾙの「最大値の平均値」-「最小値の平均値」を算出する
	temp = ((SEQ.TABLE_EDGE_LEFT[X_DIVISION_NUMBER-1] + SEQ.TABLE_EDGE_RIGHT[X_DIVISION_NUMBER-1]) - (SEQ.TABLE_EDGE_LEFT[0] + SEQ.TABLE_EDGE_RIGHT[0]))/2;
	for(i=1;i<=12;i++){
		// HDI = 「中央値の平均値(四捨五入) / 10倍」-「設定値 * temp / ((分割数-1) * 補正間隔 * 10倍)」
		SEQ.X_HDI_DATA[i] = (short)((float)(((SEQ.TABLE_EDGE_LEFT[X_TABLE_CENTER]+SEQ.TABLE_EDGE_RIGHT[X_TABLE_CENTER])/2)+5)/10)
								+ ((float)((short)SEQ.ALL_DATA[i+209]*temp/((X_DIVISION_NUMBER-1)*X_CORRECTION_INTERVAL*10)));			// HDIﾃﾞｰﾀ(X)
								//+ ((float)((short)SEQ.PARA_DATA[i+7]*temp/((X_DIVISION_NUMBER-1)*X_CORRECTION_INTERVAL*10)));			// HDIﾃﾞｰﾀ(X)
	}
	
	// 計測方向がZ方向のとき
	// 換算ﾃｰﾌﾞﾙの「最大値」-「最小値」を算出する
	temp = (SEQ.Z_TABLE_EDGE_RIGHT[Z_DIVISION_NUMBER-1] - SEQ.Z_TABLE_EDGE_RIGHT[0]);
	for(i=1;i<=12;i++){
		// HDI = 「中央値(四捨五入) / 10倍」+「設定値 * temp / ((分割数-1) * 補正間隔 * 10倍)」
		SEQ.Z_HDI_DATA[i] = (short)((float)(SEQ.Z_TABLE_EDGE_RIGHT[Z_TABLE_CENTER]+5)/10)
								- ((float)((short)SEQ.ALL_DATA[i+229]*temp/((Z_DIVISION_NUMBER-1)*Z_CORRECTION_INTERVAL*10)));			// HDIﾃﾞｰﾀ(Z)
								//- ((float)((short)SEQ.PARA_DATA[i+7]*temp/((Z_DIVISION_NUMBER-1)*Z_CORRECTION_INTERVAL*10)));			// HDIﾃﾞｰﾀ(Z)
	}
}

//************************************************************/
//				DBUSにﾊﾟﾗﾒｰﾀをｾｯﾄ
//************************************************************/
void set_parameter_to_dbus(void)
{
	//float temp;
	union UFloatLong Variable;
	
	switch(SEQ.CBUS_NUMBER){
		
		// 整数値 //
		case 1:		// 方向
		case 2:		// ﾀｲﾌﾟ
		case 3:		// 露光時間
		
		case 20:	// LED1 開始
		case 21:	// LED1 終了
		case 22:	// LED2 開始
		case 23:	// LED2 終了
		case 24:	// LED3 開始
		case 25:	// LED3 終了
		case 26:	// LED4 開始
		case 27:	// LED4 終了
		case 28:	// LED5 開始
		case 29:	// LED5 終了
		case 30:	// LED6 開始
		case 31:	// LED6 終了
		case 32:	// LED7 開始
		case 33:	// LED7 終了
		case 34:	// LED8 開始
		case 35:	// LED8 終了
		case 36:	// LED9 開始
		case 37:	// LED9 終了
		case 38:	// LED10 開始
		case 39:	// LED10 終了
		case 40:	// LED11 開始
		case 41:	// LED11 終了
		case 42:	// 特定画素値の出力
			SEQ.INPUT_DBUS_LONG = SEQ.PARA_DATA[SEQ.CBUS_NUMBER];
			break;
		
		// 浮動小数点変換値 //
		// 1/100倍 /////////////////////////////////////////////////////////////////////
		case 4:	// 黒比率傾斜大ﾚﾍﾞﾙ
		case 5:	// 黒比率傾斜小ﾚﾍﾞﾙ
		case 6:	// ｽｷｯﾌﾟ係数
		case 7:	// 荒ｴｯｼﾞｻｰﾁ係数
		
			// LONG型をFLOAT型に変更する
			Variable.fFloat = SEQ.PARA_DATA[SEQ.CBUS_NUMBER];
			// ﾒﾓﾘ保存値を変換する
			Variable.fFloat = Variable.fFloat / 100;
			SEQ.INPUT_DBUS_LONG = Variable.lLong;
			break;
		////////////////////////////////////////////////////////////////////////////////
		
		// 整数値 //
		// ｽｷｯﾌﾟ制御
		// 設定値を画素変換して送信
		// XとZで符号が異なる
		case 8:		// HDI0　開始
		case 9:		// HDI0　終了
		case 10:	// HDI1　開始
		case 11:	// HDI1　終了
		case 12:	// HDI2　開始
		case 13:	// HDI2　終了
		case 14:	// HDI3　開始
		case 15:	// HDI3　終了
		case 16:	// HDI4　開始
		case 17:	// HDI4　終了
		case 18:	// HDI5　開始
		case 19:	// HDI5　終了
			// 計測方向がZ方向のとき
			if(SEQ.MEASUREMENT_DIRECTION == Z_DIRECTION)	SEQ.INPUT_DBUS_LONG = SEQ.Z_HDI_DATA[SEQ.CBUS_NUMBER-7];
			// 計測方向がX方向のとき
			else											SEQ.INPUT_DBUS_LONG = SEQ.X_HDI_DATA[SEQ.CBUS_NUMBER-7];
			
			//SEQ.PARA_DATA[SEQ.CBUS_NUMBER] = SEQ.HDI_DATA[SEQ.CBUS_NUMBER-7];		// HDIﾃﾞｰﾀ
			
			// ﾊﾟﾗﾒｰﾀ確認用
			//SEQ.HDI_DATA_COUNT++;										// HDIﾃﾞｰﾀｶｳﾝﾀ
			//SEQ.HDI_DATA[SEQ.HDI_DATA_COUNT] = SEQ.INPUT_DBUS_LONG;		// HDIﾃﾞｰﾀ
			break;
		
		case 380:
			SEQ.INPUT_DBUS_LONG = 0;
			break;
	}
}
