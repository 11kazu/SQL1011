/******************************************************************************
* File Name	: hdi_output.c
******************************************************************************/
#include <machine.h>
#include "iodefine.h"
#include "user_define.h"
#include "typedefine.h"
#include "usercopy.h"

//************************************************************/
//				内部関数プロトタイプ宣言					
//************************************************************/
void clk5_pulse(void);						// CLK5ﾊﾟﾙｽ
void hdi_output(void);						// HDI出力

//************************************************************/
//				CLK5ﾊﾟﾙｽ
//************************************************************/
void clk5_pulse(void)
{
	CLK5_OUT = 1;
	CLK5_OUT = 0;
}

//************************************************************/
//				HDI出力
//************************************************************/
void hdi_output(void)
{
	_UBYTE buf, skip_flag;
	_UBYTE hdi0_l_flag, hdi1_l_flag, hdi2_l_flag, hdi3_l_flag, hdi4_l_flag, hdi5_l_flag;
	_UBYTE hdi0_r_flag, hdi1_r_flag, hdi2_r_flag, hdi3_r_flag, hdi4_r_flag, hdi5_r_flag;
// add 2016.03.11 K.Uemura start	G31101
	long focus_L, focus_R;
	_UBYTE hdi0_flag, hdi1_flag, hdi2_flag, hdi3_flag, hdi4_flag, hdi5_flag;
// add 2016.03.11 K.Uemura end
	//_UWORD hdi;
	_UDWORD hdi;
	
// chg 2016.06.22 K.Uemura start	G62202
	if((COM0.NO311 == 152)||(COM0.NO311 == 153)){
//	if(COM0.NO311 == 152){
// chg 2016.06.22 K.Uemura end
		//return;
		hdi = 0;
	}else{

		//hdi = SEQ.FOCUSING_HDI >> 11;		// 11ﾋﾞｯﾄ右にｼﾌﾄ
		//hdi = (SEQ.FOCUSING_HDI >> 11) & 0x0000003F;		// 6ﾋﾞｯﾄ分のﾏｽｸ
		hdi = (SEQ.FOCUSING_HDI >> 11) & 0x00000007;		// 3ﾋﾞｯﾄ分のﾏｽｸ
		
		if(OUT.SUB_STATUS <= 3)		hdi = 0;
		
		// 7:ORIGIN・8:ORIGIN(ｴｯｼﾞ考慮)の場合以外は出力不可
		// 計測していないとき
		if(SEQ.FLAG.BIT.MEASUREMENT == 0){
			SEQ.FLAG3.BIT.HDI_OUTPUT = 0;			// HDI出力ﾌﾗｸﾞ

			// HDI出力確認の場合は、計測していない時でも出力可
			if((COM0.NO301 < 250)&&(263 < COM0.NO301)){
				SEQ.FLAG3.BIT.HDI_OUTPUT = 1;			// HDI出力ﾌﾗｸﾞ
			}
		}
		
		// 計測しているときで、かつHDI出力ﾌﾗｸﾞが「1」のとき
		if((SEQ.FLAG.BIT.MEASUREMENT == 1)&&(SEQ.FLAG3.BIT.HDI_OUTPUT == 1)){
			skip_flag = 0;		// ｽｷｯﾌﾟ出力ﾌﾗｸﾞ
			
			// 「7:ORIGIN」「8:ORIGIN(ｴｯｼﾞ考慮)」のとき
			switch(SEQ.SELECT.BIT.MEASURE){
				case MODE_ORIGIN:
				case MODE_ORIGIN_EDGE:
					skip_flag = 1;		// ｽｷｯﾌﾟ出力ﾌﾗｸﾞ
					
					// 8:ORIGIN(ｴｯｼﾞ考慮)のとき左右のｴｯｼﾞいずれかが手動傾斜画素の設定値以下のときに出力する
					// 上記以外は出力しない
					if(SEQ.SELECT.BIT.MEASURE == MODE_ORIGIN_EDGE){		// 8:ORIGIN(ｴｯｼﾞ考慮)のとき
						if((SEQ.FLAG4.BIT.EDGE_R == 0) && (SEQ.FLAG4.BIT.EDGE_L == 0)){
							hdi = 0;
						}
						
						// 全遮光時、全出力ON
						if(IN.FLAG.BIT.HARDWARE_TYPE == CNC_EDITION){
							if(SEQ.MEASUREMENT_DIRECTION == Z_DIRECTION){
								//if(LED.Z_FOCUSING == 0x000F)	hdi = 0x003F;
								if(LED.Z_FOCUSING == 0x000F)	hdi = 0x0007;
							}else{
								//if(LED.FOCUSING == 0x01FF)		hdi = 0x003F;
								if(LED.FOCUSING == 0x01FF)		hdi = 0x0007;
							}
						}else{
								//if(LED.FOCUSING == 0x07FF)		hdi = 0x003F;
								if(LED.FOCUSING == 0x07FF)		hdi = 0x0007;
						}
					}
					break;

				case MODE_D4_AUTO:	// 「10:工具径(自動)」
				case MODE_D4_LOW:	// 「0:工具径(d≦4)」
				case MODE_D4_LEFT:	// 「1:工具径(d＞4 左側)」
				case MODE_D4_RIGHT:	// 「11:工具径(d＞4 右側)」
				case MODE_RUNOUT:	// 「2:振れ」
				case MODE_PROFILE:	// 「5:ﾌﾟﾛﾌｧｲﾙ」
				case MODE_CENTER:	// 「4:中心位置設定」
				case MODE_GROWTH:	// 「6:伸び測定」
					// ｽｷｯﾌﾟが「1」のとき
					if(SKIP_ENABLE == 1)	skip_flag = 1;		// ｽｷｯﾌﾟ出力ﾌﾗｸﾞ
					break;

				case MODE_FOCUS:
					skip_flag = 2;		// ｽｷｯﾌﾟ出力ﾌﾗｸﾞ
					break;
			}
			
			// 換算ﾃｰﾌﾞﾙ自動設定のときｽｷｯﾌﾟ出力しない
			if((COM0.NO300.BIT.EXE)&&(COM0.NO301 == 600)){
				skip_flag = 0;		// ｽｷｯﾌﾟ出力ﾌﾗｸﾞ
			}
			
			if(skip_flag == 1){		// ｽｷｯﾌﾟ出力ﾌﾗｸﾞ
				if(hdi & 0x01 == 0x01){
					SEQ.HDI.BIT.B0 = 1;
					SEQ.HDI0_HOLD_COUNT = 0;									// HDI0ｽｷｯﾌﾟ保持ｶｳﾝﾄをﾘｾｯﾄ
				}else{
					if(SEQ.HDI0_HOLD_COUNT >= SKIP_RETENTION_TIME){			// HDI0ｽｷｯﾌﾟ保持ｶｳﾝﾄがｽｷｯﾌﾟ保持時間以上になったとき
						SEQ.HDI.BIT.B0 = 0;
					}
				}
				
				if((hdi >> 1) & 0x01 == 0x01){
					SEQ.HDI.BIT.B1 = 1;
					SEQ.HDI1_HOLD_COUNT = 0;									// HDI1ｽｷｯﾌﾟ保持ｶｳﾝﾄをﾘｾｯﾄ
				}else{
					if(SEQ.HDI1_HOLD_COUNT >= SKIP_RETENTION_TIME){			// HDI1ｽｷｯﾌﾟ保持ｶｳﾝﾄがｽｷｯﾌﾟ保持時間以上になったとき
						SEQ.HDI.BIT.B1 = 0;
					}
				}
					
				if((hdi >> 2) & 0x01 == 0x01){
					SEQ.HDI.BIT.B2 = 1;
					SEQ.HDI2_HOLD_COUNT = 0;									// HDI2ｽｷｯﾌﾟ保持ｶｳﾝﾄをﾘｾｯﾄ
				}else{
					if(SEQ.HDI2_HOLD_COUNT >= SKIP_RETENTION_TIME){			// HDI2ｽｷｯﾌﾟ保持ｶｳﾝﾄがｽｷｯﾌﾟ保持時間以上になったとき
						SEQ.HDI.BIT.B2 = 0;
					}
				}
				
				/*
				if((hdi >> 3) & 0x01 == 0x01){
					SEQ.HDI.BIT.B3 = 1;
					SEQ.HDI3_HOLD_COUNT = 0;									// HDI3ｽｷｯﾌﾟ保持ｶｳﾝﾄをﾘｾｯﾄ
				}else{
					if(SEQ.HDI3_HOLD_COUNT >= SKIP_RETENTION_TIME){			// HDI3ｽｷｯﾌﾟ保持ｶｳﾝﾄがｽｷｯﾌﾟ保持時間以上になったとき
						SEQ.HDI.BIT.B3 = 0;
					}
				}
				
				if((hdi >> 4) & 0x01 == 0x01){
					SEQ.HDI.BIT.B4 = 1;
					SEQ.HDI4_HOLD_COUNT = 0;									// HDI4ｽｷｯﾌﾟ保持ｶｳﾝﾄをﾘｾｯﾄ
				}else{
					if(SEQ.HDI4_HOLD_COUNT >= SKIP_RETENTION_TIME){			// HDI4ｽｷｯﾌﾟ保持ｶｳﾝﾄがｽｷｯﾌﾟ保持時間以上になったとき
						SEQ.HDI.BIT.B4 = 0;
					}
				}
				
				if((hdi >> 5) & 0x01 == 0x01){
					SEQ.HDI.BIT.B5 = 1;
					SEQ.HDI5_HOLD_COUNT = 0;									// HDI5ｽｷｯﾌﾟ保持ｶｳﾝﾄをﾘｾｯﾄ
				}else{
					if(SEQ.HDI5_HOLD_COUNT >= SKIP_RETENTION_TIME){			// HDI5ｽｷｯﾌﾟ保持ｶｳﾝﾄがｽｷｯﾌﾟ保持時間以上になったとき
						SEQ.HDI.BIT.B5 = 0;
					}
				}
				*/
				
				//hdi = (SEQ.HDI.BIT.B5 << 5) | (SEQ.HDI.BIT.B4 << 4) | (SEQ.HDI.BIT.B3 << 3) |
					  //(SEQ.HDI.BIT.B2 << 2) | (SEQ.HDI.BIT.B1 << 1) | SEQ.HDI.BIT.B0;
				hdi = (SEQ.HDI.BIT.B2 << 2) | (SEQ.HDI.BIT.B1 << 1) | SEQ.HDI.BIT.B0;
					  
				// ｽｷｯﾌﾟ信号反転が「1」のときﾋﾞｯﾄを反転する
				//if(SKIP_SIGNAL_REVERSE == 1)		hdi = ~hdi & 0x3F;
				if(SKIP_SIGNAL_REVERSE == 1)		hdi = ~hdi & 0x07;
				
			}else if(skip_flag == 2){		// ｽｷｯﾌﾟ出力ﾌﾗｸﾞ「3:焦点」
				hdi0_l_flag = hdi1_l_flag = hdi2_l_flag = hdi3_l_flag = hdi4_l_flag = hdi5_l_flag = 0;
				hdi0_r_flag = hdi1_r_flag = hdi2_r_flag = hdi3_r_flag = hdi4_r_flag = hdi5_r_flag = 0;

				focus_L = (long)(RESULT.FOCUS_LEFT[2] * 10);
				focus_R = (long)(RESULT.FOCUS_RIGHT[2] * 10);

				// 計測方向がX方向のとき
				if(SEQ.MEASUREMENT_DIRECTION == X_DIRECTION){
					// HDI0(L)判定
					if(HDI0_SKIP_L == 0){
						hdi0_l_flag = 1;
					}else{
						if((focus_L != 0) && (focus_L <= HDI0_SKIP_L))	hdi0_l_flag = 1;
					}
					
					// HDI0(R)判定
					if(HDI0_SKIP_R == 0){
						hdi0_r_flag = 1;
					}else{
						if((focus_R != 0) && (focus_R <= HDI0_SKIP_R))	hdi0_r_flag = 1;
					}
					
					// HDI1(L)判定
					if(HDI1_SKIP_L == 0){
						hdi1_l_flag = 1;
					}else{
						if((focus_L != 0) && (focus_L <= HDI1_SKIP_L))	hdi1_l_flag = 1;
					}
					
					// HDI1(R)判定
					if(HDI1_SKIP_R == 0){
						hdi1_r_flag = 1;
					}else{
						if((focus_R != 0) && (focus_R <= HDI1_SKIP_R))	hdi1_r_flag = 1;
					}
					
					// HDI2(L)判定
					if(HDI2_SKIP_L == 0){
						hdi2_l_flag = 1;
					}else{
						if((focus_L != 0) && (focus_L <= HDI2_SKIP_L))	hdi2_l_flag = 1;
					}
					
					// HDI2(R)判定
					if(HDI2_SKIP_R == 0){
						hdi2_r_flag = 1;
					}else{
						if((focus_R != 0) && (focus_R <= HDI2_SKIP_R))	hdi2_r_flag = 1;
					}
					
					/*
					// HDI3(L)判定
					if(HDI3_SKIP_L == 0){
						hdi3_l_flag = 1;
					}else{
						if((focus_L != 0) && (focus_L <= HDI3_SKIP_L))	hdi3_l_flag = 1;
					}
					
					// HDI3(R)判定
					if(HDI3_SKIP_R == 0){
						hdi3_r_flag = 1;
					}else{
						if((focus_R != 0) && (focus_R <= HDI3_SKIP_R))	hdi3_r_flag = 1;
					}
					
					// HDI4(L)判定
					if(HDI4_SKIP_L == 0){
						hdi4_l_flag = 1;
					}else{
						if((focus_L != 0) && (focus_L <= HDI4_SKIP_L))	hdi4_l_flag = 1;
					}
					
					// HDI4(R)判定
					if(HDI4_SKIP_R == 0){
						hdi4_r_flag = 1;
					}else{
						if((focus_R != 0) && (focus_R <= HDI4_SKIP_R))	hdi4_r_flag = 1;
					}
					
					// HDIDELAY(L)判定
					if(HDI5_SKIP_L == 0){
						hdi5_l_flag = 1;
					}else{
						if((focus_L != 0) && (focus_L <= HDI5_SKIP_L))	hdi5_l_flag = 1;
					}
					
					// HDI5(R)判定
					if(HDI5_SKIP_R == 0){
						hdi5_r_flag = 1;
					}else{
						if((focus_R != 0) && (focus_R <= HDI5_SKIP_R))	hdi5_r_flag = 1;
					}
					*/
					
					// L・Rの条件を満たしたとき
	// chg 2016.03.11 K.Uemura start	G31101
					if((hdi0_l_flag == 1)&&(hdi0_r_flag == 1))		hdi0_flag = 1;
					else											hdi0_flag = 0;
					
					if((hdi1_l_flag == 1)&&(hdi1_r_flag == 1))		hdi1_flag = 1;
					else											hdi1_flag = 0;
						
					if((hdi2_l_flag == 1)&&(hdi2_r_flag == 1))		hdi2_flag = 1;
					else											hdi2_flag = 0;
					
					/*
					if((hdi3_l_flag == 1)&&(hdi3_r_flag == 1))		hdi3_flag = 1;
					else											hdi3_flag = 0;
					
					if((hdi4_l_flag == 1)&&(hdi4_r_flag == 1))		hdi4_flag = 1;
					else											hdi4_flag = 0;
					
					if((hdi5_l_flag == 1)&&(hdi5_r_flag == 1))		hdi5_flag = 1;
					else											hdi5_flag = 0;
					*/
					
//					if((hdi0_l_flag == 1)&&(hdi0_r_flag == 1))		SEQ.HDI.BIT.B0 = 1;
//					else											SEQ.HDI.BIT.B0 = 0;
//					
//					if((hdi1_l_flag == 1)&&(hdi1_r_flag == 1))		SEQ.HDI.BIT.B1 = 1;
//					else											SEQ.HDI.BIT.B1 = 0;
//						
//					if((hdi2_l_flag == 1)&&(hdi2_r_flag == 1))		SEQ.HDI.BIT.B2 = 1;
//					else											SEQ.HDI.BIT.B2 = 0;
//					
//					if((hdi3_l_flag == 1)&&(hdi3_r_flag == 1))		SEQ.HDI.BIT.B3 = 1;
//					else											SEQ.HDI.BIT.B3 = 0;
//					
//					if((hdi4_l_flag == 1)&&(hdi4_r_flag == 1))		SEQ.HDI.BIT.B4 = 1;
//					else											SEQ.HDI.BIT.B4 = 0;
//					
//					if((hdi5_l_flag == 1)&&(hdi5_r_flag == 1))		SEQ.HDI.BIT.B5 = 1;
//					else											SEQ.HDI.BIT.B5 = 0;
// chg 2016.03.11 K.Uemura end
					
				// 計測方向がZ方向のとき
				}else{
					// HDI0(R)判定
					if(Z_HDI0_SKIP_R == 0){
						hdi0_r_flag = 1;
					}else{
						if(focus_R <= Z_HDI0_SKIP_R)		hdi0_r_flag = 1;
					}
					
					// HDI1(R)判定
					if(Z_HDI1_SKIP_R == 0){
						hdi1_r_flag = 1;
					}else{
						if(focus_R <= Z_HDI1_SKIP_R)		hdi1_r_flag = 1;
					}
					
					// HDI2(R)判定
					if(Z_HDI2_SKIP_R == 0){
						hdi2_r_flag = 1;
					}else{
						if(focus_R <= Z_HDI2_SKIP_R)		hdi2_r_flag = 1;
					}
					
					/*
					// HDI3(R)判定
					if(Z_HDI3_SKIP_R == 0){
						hdi3_r_flag = 1;
					}else{
						if(focus_R <= Z_HDI3_SKIP_R)		hdi3_r_flag = 1;
					}
					
					// HDI4(R)判定
					if(Z_HDI4_SKIP_R == 0){
						hdi4_r_flag = 1;
					}else{
						if(focus_R <= Z_HDI4_SKIP_R)		hdi4_r_flag = 1;
					}
					
					// HDI5(R)判定
					if(Z_HDI5_SKIP_R == 0){
						hdi5_r_flag = 1;
					}else{
						if(focus_R <= Z_HDI5_SKIP_R)		hdi5_r_flag = 1;
					}
					*/
					
					// 条件を満たしたとき
// chg 2016.03.11 K.Uemura start	G31101
					if(hdi0_r_flag == 1)		hdi0_flag = 1;
					else						hdi0_flag = 0;
					
					if(hdi1_r_flag == 1)		hdi1_flag = 1;
					else						hdi1_flag = 0;
						
					if(hdi2_r_flag == 1)		hdi2_flag = 1;
					else						hdi2_flag = 0;
					
					/*
					if(hdi3_r_flag == 1)		hdi3_flag = 1;
					else						hdi3_flag = 0;
					
					if(hdi4_r_flag == 1)		hdi4_flag = 1;
					else						hdi4_flag = 0;
					
					if(hdi5_r_flag == 1)		hdi5_flag = 1;
					else						hdi5_flag = 0;
					*/
					
//					if(hdi0_r_flag == 1)		SEQ.HDI.BIT.B0 = 1;
//					else						SEQ.HDI.BIT.B0 = 0;
//					
//					if(hdi1_r_flag == 1)		SEQ.HDI.BIT.B1 = 1;
//					else						SEQ.HDI.BIT.B1 = 0;
//						
//					if(hdi2_r_flag == 1)		SEQ.HDI.BIT.B2 = 1;
//					else						SEQ.HDI.BIT.B2 = 0;
//					
//					if(hdi3_r_flag == 1)		SEQ.HDI.BIT.B3 = 1;
//					else						SEQ.HDI.BIT.B3 = 0;
//					
//					if(hdi4_r_flag == 1)		SEQ.HDI.BIT.B4 = 1;
//					else						SEQ.HDI.BIT.B4 = 0;
//					
//					if(hdi5_r_flag == 1)		SEQ.HDI.BIT.B5 = 1;
//					else						SEQ.HDI.BIT.B5 = 0;
// chg 2016.03.11 K.Uemura end
				}

// add 2016.03.11 K.Uemura start	G31101
				if(hdi0_flag == 0){
					if(SEQ.HDI0_HOLD_COUNT >= SKIP_RETENTION_TIME){			// HDI0ｽｷｯﾌﾟ保持ｶｳﾝﾄがｽｷｯﾌﾟ保持時間以上になったとき
						SEQ.HDI.BIT.B0 = 0;
					}
					SEQ.HDI0_DELAY_COUNT=0;
				}else{
					if(SEQ.HDI0_DELAY_COUNT >= SKIP_DELAY_TIME){			// HDI0ｽｷｯﾌﾟ保持ｶｳﾝﾄがｽｷｯﾌﾟ遅延時間未満のとき
						SEQ.HDI.BIT.B0 = 1;
					}
					SEQ.HDI0_HOLD_COUNT=0;
				}

				if(hdi1_flag == 0){
					if(SEQ.HDI1_HOLD_COUNT >= SKIP_RETENTION_TIME){			// HDI1ｽｷｯﾌﾟ保持ｶｳﾝﾄがｽｷｯﾌﾟ保持時間以上になったとき
						SEQ.HDI.BIT.B1 = 0;
					}
					SEQ.HDI1_DELAY_COUNT=0;
				}else{
					if(SEQ.HDI1_DELAY_COUNT >= SKIP_DELAY_TIME){			// HDI1ｽｷｯﾌﾟ保持ｶｳﾝﾄがｽｷｯﾌﾟ遅延時間以上になったとき
						SEQ.HDI.BIT.B1 = 1;
					}
					SEQ.HDI1_HOLD_COUNT=0;
				}

				if(hdi2_flag == 0){
					if(SEQ.HDI2_HOLD_COUNT >= SKIP_RETENTION_TIME){			// HDI2ｽｷｯﾌﾟ保持ｶｳﾝﾄがｽｷｯﾌﾟ保持時間以上になったとき
						SEQ.HDI.BIT.B2 = 0;
					}
					SEQ.HDI2_DELAY_COUNT=0;
				}else{
					if(SEQ.HDI2_DELAY_COUNT >= SKIP_DELAY_TIME){			// HDI2ｽｷｯﾌﾟ保持ｶｳﾝﾄがｽｷｯﾌﾟ遅延時間以上になったとき
						SEQ.HDI.BIT.B2 = 1;
					}
					SEQ.HDI2_HOLD_COUNT=0;
				}
				
				/*
				if(hdi3_flag == 0){
					if(SEQ.HDI3_HOLD_COUNT >= SKIP_RETENTION_TIME){			// HDI3ｽｷｯﾌﾟ保持ｶｳﾝﾄがｽｷｯﾌﾟ保持時間以上になったとき
						SEQ.HDI.BIT.B3 = 0;
					}
					SEQ.HDI3_DELAY_COUNT=0;
				}else{
					if(SEQ.HDI3_DELAY_COUNT >= SKIP_DELAY_TIME){			// HDI3ｽｷｯﾌﾟ保持ｶｳﾝﾄがｽｷｯﾌﾟ遅延時間以上になったとき
						SEQ.HDI.BIT.B3 = 1;
					}
					SEQ.HDI3_HOLD_COUNT=0;
				}

				if(hdi4_flag == 0){
					if(SEQ.HDI4_HOLD_COUNT >= SKIP_RETENTION_TIME){			// HDI4ｽｷｯﾌﾟ保持ｶｳﾝﾄがｽｷｯﾌﾟ保持時間以上になったとき
						SEQ.HDI.BIT.B4 = 0;
					}
					SEQ.HDI4_DELAY_COUNT=0;
				}else{
					if(SEQ.HDI4_DELAY_COUNT >= SKIP_DELAY_TIME){			// HDI4ｽｷｯﾌﾟ保持ｶｳﾝﾄがｽｷｯﾌﾟ遅延時間以上になったとき
						SEQ.HDI.BIT.B4 = 1;
					}
					SEQ.HDI4_HOLD_COUNT=0;
				}

				if(hdi5_flag == 0){
					if(SEQ.HDI5_HOLD_COUNT >= SKIP_RETENTION_TIME){			// HDI5ｽｷｯﾌﾟ保持ｶｳﾝﾄがｽｷｯﾌﾟ保持時間以上になったとき
						SEQ.HDI.BIT.B5 = 0;
					}
					SEQ.HDI5_DELAY_COUNT=0;
				}else{
					if(SEQ.HDI5_DELAY_COUNT >= SKIP_DELAY_TIME){			// HDI5ｽｷｯﾌﾟ保持ｶｳﾝﾄがｽｷｯﾌﾟ遅延時間以上になったとき
						SEQ.HDI.BIT.B5 = 1;
					}
					SEQ.HDI5_HOLD_COUNT=0;
				}
				*/
				
// add 2016.03.11 K.Uemura end
				
				//hdi = (SEQ.HDI.BIT.B5 << 5) | (SEQ.HDI.BIT.B4 << 4) | (SEQ.HDI.BIT.B3 << 3) | (SEQ.HDI.BIT.B2 << 2) | (SEQ.HDI.BIT.B1 << 1) | SEQ.HDI.BIT.B0;
				hdi = (SEQ.HDI.BIT.B2 << 2) | (SEQ.HDI.BIT.B1 << 1) | SEQ.HDI.BIT.B0;
				
				// ｽｷｯﾌﾟ信号反転が「1」のときﾋﾞｯﾄを反転する
				//if(SKIP_SIGNAL_REVERSE == 1)		hdi = ~hdi & 0x3F;
				if(SKIP_SIGNAL_REVERSE == 1)		hdi = ~hdi & 0x07;
				
			}else{
				hdi = 0;
			}
		}else{
			if((250 <= COM0.NO301)&&(COM0.NO301 <= 263)){
				//hdi = (SEQ.HDI.BIT.B5 << 5) | (SEQ.HDI.BIT.B4 << 4) | (SEQ.HDI.BIT.B3 << 3) | (SEQ.HDI.BIT.B2 << 2) | (SEQ.HDI.BIT.B1 << 1) | SEQ.HDI.BIT.B0;
				hdi = (SEQ.HDI.BIT.B2 << 2) | (SEQ.HDI.BIT.B1 << 1) | SEQ.HDI.BIT.B0;
			}else{
				hdi = 0;
			}
		}
	}
	
	// ｴﾗｰ信号出力 ADD 161114
	if(COM0.NO312 == 0){					// ｴﾗｰが無いとき
		if(ERR_SIGNAL_REVERSE == 0){		// ｽｷｯﾌﾟ信号反転が「0」のとき
			SEQ.HDI.BIT.B3 = 0;
		}else{								// ｽｷｯﾌﾟ信号反転が「1」のとき反転
			SEQ.HDI.BIT.B3 = 1;
		}
	}else{									// ｴﾗｰがあるとき
		if(ERR_SIGNAL_REVERSE == 0){		// ｽｷｯﾌﾟ信号反転が「0」のとき
			SEQ.HDI.BIT.B3 = 1;
		}else{								// ｽｷｯﾌﾟ信号反転が「1」のとき反転
			SEQ.HDI.BIT.B3 = 0;
		}
	}
	
	// HDI出力(TPD表示) + ｴﾗｰ信号出力 ADD 161114
	hdi |= (SEQ.HDI.BIT.B3 << 3);
	COM0.NO331 = hdi;
	
	// 電磁ﾊﾞﾙﾌﾞ
	hdi |= (COM0.NO310.BIT.PUR << 7) | (COM0.NO310.BIT.COV << 6);
	
	buf = hdi;
	drv_disp_data_out(buf);
	
	clk5_pulse();					// CLK5ﾊﾟﾙｽ
}
