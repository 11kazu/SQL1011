/******************************************************************************
* File Name	: ctl_timer_func.c
******************************************************************************/
#include <machine.h>
#include "iodefine.h"
#include "typedefine.h"
#include "usercopy.h"
#include "user_define.h"

//************************************************************/
//				内部関数プロトタイプ宣言					
//************************************************************/

void ctl_uni_timer1(_UWORD time);		// 
void ctl_drv_timer1(void);
void ctl_uni_timer2(_UWORD time);		// RS422ﾀｲﾑｱｳﾄ
void ctl_drv_timer2(void);
void ctl_uni_timer3(_UWORD time);		// RS232Cﾀｲﾑｱｳﾄ
void ctl_drv_timer3(void);
void ctl_uni_timer4(_UWORD time);		// 
void ctl_drv_timer4(void);
void ctl_uni_timer5(_UWORD time);		// 
void ctl_drv_timer5(void);
void ctl_uni_timer6(_UWORD time);		// 
void ctl_drv_timer6(void);
void ctl_uni_timer7(_UWORD time);		// 
void ctl_drv_timer7(void);
void ctl_uni_timer8(_UWORD time);		// 
void ctl_drv_timer8(void);

//************************************************************/
//				広域変数宣言					
//************************************************************/

//************************************************************/
//				ﾕﾆｰﾊﾞｰｻﾙﾀｲﾏ1起動関数					
//************************************************************/
/*引数timeにﾃﾞｰﾀを与えると×10msのﾀｲﾏｰ値でﾀｲﾏｰ動作をｽﾀｰﾄする
　ﾀｲﾏｰ本体はctl_drv_timer1()でｶｳﾝﾄｱｯﾌﾟするとTIM.FLAG.BIT.UNI_UP1
　のﾌﾗｸﾞが立つ*/
void ctl_uni_timer1(_UWORD time)
{
	TIM1.MSEC_10.BIT.UNI_UP1	= 0;
	TIM1.UNI_CONT1				= 0;
	TIM1.UNI_COMP1				= time;
	TIM1.MSEC_10.BIT.UNI_ST1	= 1;
}
//************************************************************/
//				ﾕﾆｰﾊﾞｰｻﾙﾀｲﾏ1駆動関数					
//************************************************************/
void ctl_drv_timer1(void)
{
	if(TIM1.MSEC_10.BIT.UNI_ST1){
		TIM1.UNI_CONT1 ++;
		if(TIM1.UNI_CONT1==TIM1.UNI_COMP1){
			TIM1.MSEC_10.BIT.UNI_ST1	= 0;
			TIM1.MSEC_10.BIT.UNI_UP1	= 1;
		}
	}
}
//************************************************************/
//				ﾕﾆｰﾊﾞｰｻﾙﾀｲﾏ2起動関数					
//************************************************************/
void ctl_uni_timer2(_UWORD time)
{
	TIM1.MSEC_10.BIT.UNI_UP2	= 0;
	TIM1.UNI_CONT2				= 0;
	TIM1.UNI_COMP2				= time;
	TIM1.MSEC_10.BIT.UNI_ST2	= 1;
}
//************************************************************/
//				ﾕﾆｰﾊﾞｰｻﾙﾀｲﾏ2駆動関数					
//************************************************************/
void ctl_drv_timer2(void)
{
	if(TIM1.MSEC_10.BIT.UNI_ST2){
		TIM1.UNI_CONT2 ++;
		if(TIM1.UNI_CONT2 == TIM1.UNI_COMP2){
			TIM1.MSEC_10.BIT.UNI_ST2	= 0;
			TIM1.MSEC_10.BIT.UNI_UP2	= 1;
		}
	}
}
//************************************************************/
//				ﾕﾆｰﾊﾞｰｻﾙﾀｲﾏ3起動関数					
//************************************************************/
void ctl_uni_timer3(_UWORD time)
{
	TIM1.MSEC_10.BIT.UNI_UP3	= 0;
	TIM1.UNI_CONT3				= 0;
	TIM1.UNI_COMP3				= time;
	TIM1.MSEC_10.BIT.UNI_ST3	= 1;
}
//************************************************************/
//				ﾕﾆｰﾊﾞｰｻﾙﾀｲﾏ3駆動関数					
//************************************************************/
void ctl_drv_timer3(void)
{
	if(TIM1.MSEC_10.BIT.UNI_ST3){
		TIM1.UNI_CONT3 ++;
		if(TIM1.UNI_CONT3 == TIM1.UNI_COMP3){
			TIM1.MSEC_10.BIT.UNI_ST3	= 0;
			TIM1.MSEC_10.BIT.UNI_UP3	= 1;
		}
	}
}
//************************************************************/
//				ﾕﾆｰﾊﾞｰｻﾙﾀｲﾏ4起動関数					
//************************************************************/
void ctl_uni_timer4(_UWORD time)
{
	TIM1.MSEC_10.BIT.UNI_UP4	= 0;
	TIM1.UNI_CONT4				= 0;
	TIM1.UNI_COMP4				= time;
	TIM1.MSEC_10.BIT.UNI_ST4	= 1;
}
//************************************************************/
//				ﾕﾆｰﾊﾞｰｻﾙﾀｲﾏ4駆動関数					
//************************************************************/
void ctl_drv_timer4(void)
{
	if(TIM1.MSEC_10.BIT.UNI_ST4){
		TIM1.UNI_CONT4 ++;
		if(TIM1.UNI_CONT4 == TIM1.UNI_COMP4){
			TIM1.MSEC_10.BIT.UNI_ST4	= 0;
			TIM1.MSEC_10.BIT.UNI_UP4	= 1;
		}
	}
}
//************************************************************/
//				ﾕﾆｰﾊﾞｰｻﾙﾀｲﾏ5起動関数					
//************************************************************/
void ctl_uni_timer5(_UWORD time)
{
	TIM1.MSEC_10.BIT.UNI_UP5	= 0;
	TIM1.UNI_CONT5				= 0;
	TIM1.UNI_COMP5				= time;
	TIM1.MSEC_10.BIT.UNI_ST5	= 1;
}
//************************************************************/
//				ﾕﾆｰﾊﾞｰｻﾙﾀｲﾏ5駆動関数					
//************************************************************/
void ctl_drv_timer5(void)
{
	if(TIM1.MSEC_10.BIT.UNI_ST5){
		TIM1.UNI_CONT5 ++;
		if(TIM1.UNI_CONT5 == TIM1.UNI_COMP5){
			TIM1.MSEC_10.BIT.UNI_ST5	= 0;
			TIM1.MSEC_10.BIT.UNI_UP5	= 1;
		}
	}
}
//************************************************************/
//				ﾕﾆｰﾊﾞｰｻﾙﾀｲﾏ6起動関数					
//************************************************************/
void ctl_uni_timer6(_UWORD time)
{
	TIM1.MSEC_10.BIT.UNI_UP6	= 0;
	TIM1.UNI_CONT6				= 0;
	TIM1.UNI_COMP6				= time;
	TIM1.MSEC_10.BIT.UNI_ST6	= 1;
}
//************************************************************/
//				ﾕﾆｰﾊﾞｰｻﾙﾀｲﾏ6駆動関数					
//************************************************************/
void ctl_drv_timer6(void)
{
	if(TIM1.MSEC_10.BIT.UNI_ST6){
		TIM1.UNI_CONT6 ++;
		if(TIM1.UNI_CONT6 == TIM1.UNI_COMP6){
			TIM1.MSEC_10.BIT.UNI_ST6	= 0;
			TIM1.MSEC_10.BIT.UNI_UP6	= 1;
		}
	}
}
//************************************************************/
//				ﾕﾆｰﾊﾞｰｻﾙﾀｲﾏ7起動関数					
//************************************************************/
void ctl_uni_timer7(_UWORD time)
{
	TIM1.MSEC_10.BIT.UNI_UP7	= 0;
	TIM1.UNI_CONT7				= 0;
	TIM1.UNI_COMP7				= time;
	TIM1.MSEC_10.BIT.UNI_ST7	= 1;
}
//************************************************************/
//				ﾕﾆｰﾊﾞｰｻﾙﾀｲﾏ7駆動関数					
//************************************************************/
void ctl_drv_timer7(void)
{
	if(TIM1.MSEC_10.BIT.UNI_ST7){
		TIM1.UNI_CONT7 ++;
		if(TIM1.UNI_CONT7 == TIM1.UNI_COMP7){
			TIM1.MSEC_10.BIT.UNI_ST7	= 0;
			TIM1.MSEC_10.BIT.UNI_UP7	= 1;
		}
	}
}
//************************************************************/
//				ﾕﾆｰﾊﾞｰｻﾙﾀｲﾏ8起動関数					
//************************************************************/
void ctl_uni_timer8(_UWORD time)
{
	TIM1.MSEC_10.BIT.UNI_UP8	= 0;
	TIM1.UNI_CONT8				= 0;
	TIM1.UNI_COMP8				= time;
	TIM1.MSEC_10.BIT.UNI_ST8	= 1;
}
//************************************************************/
//				ﾕﾆｰﾊﾞｰｻﾙﾀｲﾏ8駆動関数					
//************************************************************/
void ctl_drv_timer8(void)
{
	if(TIM1.MSEC_10.BIT.UNI_ST8){
		TIM1.UNI_CONT8 ++;
		if(TIM1.UNI_CONT8 == TIM1.UNI_COMP8){
			TIM1.MSEC_10.BIT.UNI_ST8	= 0;
			TIM1.MSEC_10.BIT.UNI_UP8	= 1;
		}
	}
}
