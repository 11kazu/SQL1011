/******************************************************************************
* File Name	: common_function.c
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

unsigned short get_reset_cycle(void);		// リセット周期の取得(回転数/刃数)

//************************************************************/
//				FPGAへのﾃﾞｰﾀ送信関数(計測)
//************************************************************/
// 計測
unsigned short get_reset_cycle(void)
{
	unsigned short reset_cycle;

	// 単位変換：60s→msec
	reset_cycle = (60000 / (SEQ.SPINDLE_SPEED / SEQ.FLUTES));

	return( reset_cycle );
}


