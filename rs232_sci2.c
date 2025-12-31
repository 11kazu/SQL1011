/******************************************************************************
* File Name	: rs232_sci2.c
******************************************************************************/
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "iodefine.h"
#include "typedefine.h"
#include "usercopy.h"
#include "user_define.h"
#include "ASC2.h"
#include <stdio.h>
#include "lut.h"

void sci2_init(void);						// RS232C初期設定

void rs232c_master(void);					// ﾒｲﾝｼｰｹﾝｽ関数

void rs232c_rxi(void);						// RS232C(SCI2)RXI(受信ﾃﾞｰﾀﾌﾙ割り込み)
void rs232c_txi(void);						// RS232C(SCI2)TXI(送信ﾃﾞｰﾀｴﾝﾌﾟﾃｨ割り込み)
void rs232c_tei(void);						// RS232C(SCI2)TEI(送信終了割り込み)

void rs232c_drive(void);					// RS232C制御関数
void rs232c_drive_debug(void);				// RS232C制御関数(デバッグ出力)
unsigned char get_number(unsigned long, unsigned char);
void sci_puts(char);
unsigned short get_voltage(unsigned short);

//************************************************************/
//				RS232C初期設定(SCI2)
//************************************************************/
void sci2_init(void)
{
	MSTP(SCI2) = 0;			// RIIC0ﾓｼﾞｭｰﾙｽﾄｯﾌﾟ解除
	
	// 割り込み要求禁止
	IEN(SCI2, TEI2) = 0;
	IEN(SCI2, ERI2) = 0;
	IEN(SCI2, TXI2) = 0;
	IEN(SCI2, RXI2) = 0;

	IOPORT.PFFSCI.BIT.SCI2S = 1;		// P50-52をｼﾘｱﾙ端子として設定
	
	SCI2.SCR.BYTE = 0x00;

	while (0x00 != (SCI2.SCR.BYTE & 0xF0)){}

	// Select an On-chip baud rate generator to the clock source
	SCI2.SCR.BIT.CKE = 0;

	// SMR - Serial Mode Register
	SCI2.SMR.BYTE = 0x00;		// 38400bps
	//SCI2.SMR.BYTE = 0x02;		// 9600bps

	// SCMR - Smart Card Mode Register
	SCI2.SCMR.BYTE = 0xF2;

	// SEMR - Serial Extended Mode Register
	SCI2.SEMR.BYTE = 0x00;
	
	// BRR - Bit Rate Register
	// 115200bps
	//Bit Rate: (48MHz/(64*2^(-1)*115200bps))-1 = 12.0208 ≒ 12
	SCI2.BRR = 12;			// 115200bps
	//SCI2.BRR = 38;			// 38400bps
	
	delay_ms(1);			// 1ﾋﾞｯﾄ期間待機
	
	// 割り込み優先ﾚﾍﾞﾙ設定
	IPR(SCI2, TXI2) = 1;
	IPR(SCI2, RXI2) = 1;
	IPR(SCI2, TEI2) = 1;
	IPR(SCI2, ERI2) = 1;
	
	// 割り込み要求ｸﾘｱ
	IR(SCI2, TXI2) = 0;
	IR(SCI2, RXI2) = 0;
	IR(SCI2, TEI2) = 0;
	IR(SCI2, ERI2) = 0;
	
	// 割り込み要求許可
	IEN(SCI2, TXI2) = 1;
	IEN(SCI2, RXI2) = 1;
	IEN(SCI2, ERI2) = 1;
	IEN(SCI2, TEI2) = 1;
}

//************************************************************/
//				ﾒｲﾝｼｰｹﾝｽ関数					
//************************************************************/
void rs232c_master(void)
{
	switch(COM2.MASTER_STATUS){
		case RESET_MODE:
			break;
		case INIT_START_MODE:
			break;
		case DRV_MODE:
			rs232c_drive();
			break;
		case IDLE_MODE:
			break;
		case READ_MODE:
			break;
		case WRITE_MODE:
			break;
		case DEBUG_MODE:
			rs232c_drive_debug();
			break;
		default:
			COM2.MASTER_STATUS = RESET_MODE;
	}
}

//************************************************************/
//				RS232C(SCI2)RXI(受信ﾃﾞｰﾀﾌﾙ割り込み)
//************************************************************/
void rs232c_rxi(void)
{
	_UBYTE temp;
	
	temp = SCI2.RDR;
	// 「STX」を受信したらｶｳﾝﾀを「0」にする
	if(temp == STX)		COM2.RE_CONT = 0;
	
	COM2.RE_BUF[COM2.RE_CONT] = temp;
	
	if(temp != ETX){				// 「ETX」以外を受信したらｶｳﾝﾀを+1する
		COM2.RE_CONT++;
	}else{							// 「ETX」を受信したら受信動作を停止する
		SCI2.SCR.BIT.RIE = 0;		// RXIおよびERI割り込み要求を禁止
		SCI2.SCR.BIT.RE = 0;		// ｼﾘｱﾙ受信動作を禁止
		COM2.SUB_STATUS++;			// 次へ
	}
}

//************************************************************/
//				RS232C(SCI2)TXI(送信ﾃﾞｰﾀｴﾝﾌﾟﾃｨ割り込み)
//************************************************************/
void rs232c_txi(void)
{
	long temp;
#ifdef	__INITIAL_SEQUENCE
	short i;
#endif
	temp = DEBUG_STR.DEBUG[DEBUG_STR.DEBUG_COUNT];
	
	switch(DEBUG_STR.DEBUG_OUTPUT){
		case 0:			// ﾃﾞｼﾞﾀﾙ値・ﾌﾞﾗｯｸ値		// 0 整数3桁
			switch(COM2.WR_CONT){							// 書き込みｶｳﾝﾀ(内部)
				case 1:		// 百の位
					SCI2.TDR = ((DEBUG_STR.DEBUG[DEBUG_STR.DEBUG_COUNT] / 100) % 10 + 0x30);
					COM2.WR_CONT++;
					break;
					
				case 2:		// 十の位
					SCI2.TDR = ((DEBUG_STR.DEBUG[DEBUG_STR.DEBUG_COUNT] / 10) % 10 + 0x30);
					COM2.WR_CONT++;
					break;
					
				case 3:		// 一の位
					SCI2.TDR = ((DEBUG_STR.DEBUG[DEBUG_STR.DEBUG_COUNT]) % 10 + 0x30);
					COM2.WR_CONT++;
					break;
					
				case 4:		// ｶﾝﾏまたはCR
					if(DEBUG_STR.DEBUG_COUNT < 4095){		// 4095未満のとき
						SCI2.TDR = ',';
						DEBUG_STR.DEBUG_COUNT++;
					}else{
						SCI2.TDR = CR;
						SCI2.SCR.BIT.TIE = 0;				// TXI割り込み要求を禁止
						SCI2.SCR.BIT.TEIE = 1;				// TEI割り込み要求を許可
					}
					COM2.WR_CONT = 1;						// 書き込みｶｳﾝﾀ
					break;
			}
			break;

		case 1:			// 処理済みﾃﾞｰﾀ				// 1 符号・整数1桁・小数点3桁
			if(temp < 0)	temp *= -1;						// 値がﾏｲﾅｽのとき一時ﾃﾞｰﾀを反転する
			switch(COM2.WR_CONT){							// 書き込みｶｳﾝﾀ(内部)
				case 1:		// 符号
					if(DEBUG_STR.DEBUG[DEBUG_STR.DEBUG_COUNT] < 0){
						SCI2.TDR = '-';
					}else{
						SCI2.TDR = '+';
						//SCI2.TDR = ((DEBUG_STR.DEBUG[DEBUG_STR.DEBUG_COUNT]) % 10 + 0x30);
						//COM2.WR_CONT++;
					}
					COM2.WR_CONT++;
					break;
					
				case 2:		// 一の位
					SCI2.TDR = ((temp / 1000) % 10 + 0x30);
					COM2.WR_CONT++;
					break;
					
				case 3:		// 小数点
					SCI2.TDR = '.';
					COM2.WR_CONT++;
					break;
					
				case 4:		// 小数第一位
					SCI2.TDR = ((temp / 100) % 10 + 0x30);
					COM2.WR_CONT++;
					break;
					
				case 5:		// 小数第二位
					SCI2.TDR = ((temp / 10) % 10 + 0x30);
					COM2.WR_CONT++;
					break;
					
				case 6:		// 小数第三位
					SCI2.TDR = ((temp) % 10 + 0x30);
					COM2.WR_CONT++;
					break;
					
				case 7:		// ｶﾝﾏまたはCR
					if(DEBUG_STR.DEBUG_COUNT < 4095){		// 4095未満のとき
						SCI2.TDR = ',';
						DEBUG_STR.DEBUG_COUNT++;
					}else{
						SCI2.TDR = CR;
						SCI2.SCR.BIT.TIE = 0;				// TXI割り込み要求を禁止
						SCI2.SCR.BIT.TEIE = 1;				// TEI割り込み要求を許可
					}
					COM2.WR_CONT = 1;						// 書き込みｶｳﾝﾀ
					break;
			}
			break;

		case 2:			// ﾎﾜｲﾄ値					// 2 整数3桁・小数点3桁
			switch(COM2.WR_CONT){							// 書き込みｶｳﾝﾀ(内部)
				case 1:		// 百の位
					SCI2.TDR = ((temp / 100000) % 10 + 0x30);
					COM2.WR_CONT++;
					break;
					
				case 2:		// 十の位
					SCI2.TDR = ((temp / 10000) % 10 + 0x30);
					COM2.WR_CONT++;
					break;
					
				case 3:		// 一の位
					SCI2.TDR = ((temp / 1000) % 10 + 0x30);
					COM2.WR_CONT++;
					break;
					
				case 4:		// 小数点
					SCI2.TDR = '.';
					COM2.WR_CONT++;
					break;
					
				case 5:		// 小数第一位
					SCI2.TDR = ((temp / 100) % 10 + 0x30);
					COM2.WR_CONT++;
					break;
					
				case 6:		// 小数第二位
					SCI2.TDR = ((temp / 10) % 10 + 0x30);
					COM2.WR_CONT++;
					break;
					
				case 7:		// 小数第三位
					SCI2.TDR = ((temp) % 10 + 0x30);
					COM2.WR_CONT++;
					break;
					
				case 8:		// ｶﾝﾏまたはCR
					if(DEBUG_STR.DEBUG_COUNT < 4095){		// 4095未満のとき
						SCI2.TDR = ',';
						DEBUG_STR.DEBUG_COUNT++;
					}else{
						SCI2.TDR = CR;
						SCI2.SCR.BIT.TIE = 0;				// TXI割り込み要求を禁止
						SCI2.SCR.BIT.TEIE = 1;				// TEI割り込み要求を許可
					}
					COM2.WR_CONT = 1;						// 書き込みｶｳﾝﾀ
					break;
			}
			break;

		case 10:
			// デバッグデータ出力
			switch(COM2.WR_CONT){							// 書き込みｶｳﾝﾀ(内部)
				// 電圧
				case 1:
					// 出力文字列の生成
					memset((void*)&SEQ.SIO_BUF[0],0,sizeof(SEQ.SIO_BUF));

					// 電圧
					sprintf((char*)&SEQ.SIO_BUF[strlen((char*)SEQ.SIO_BUF)],"%03d,",ADCOV.V_BATT);
					sprintf((char*)&SEQ.SIO_BUF[strlen((char*)SEQ.SIO_BUF)],"%04d,",get_voltage(ADCOV.V_BATT));

					// 輝度
					sprintf((char*)&SEQ.SIO_BUF[strlen((char*)SEQ.SIO_BUF)],"%04d,",DA.DADR0);

#ifdef	___LOGGING_VOLTAGE_RESULT
					// 工具径
					sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"%05d,",DEBUG_STR.DEBUG[10]);	// 工具径
					sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"%05d,",DEBUG_STR.DEBUG[23]);	// エッジ(左)最小
					sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"%05d,",DEBUG_STR.DEBUG[21]);	// エッジ(左)最大
					sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"%05d,",DEBUG_STR.DEBUG[24]);	// エッジ(右)最小
					sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"%05d,",DEBUG_STR.DEBUG[22]);	// エッジ(右)最大
					sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"%04d,",DEBUG_STR.DEBUG[33]);	// 傾き(左)最小
					sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"%04d,",DEBUG_STR.DEBUG[31]);	// 傾き(左)最大
					sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"%04d,",DEBUG_STR.DEBUG[34]);	// 傾き(右)最小
					sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"%04d,",DEBUG_STR.DEBUG[32]);	// 傾き(右)最大
//					sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"%06.1f,",DEBUG_STR.DEBUG[10]);
//					sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"%05.2f,",DEBUG_STR.DEBUG[31]);
//					sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"%05.2f,",DEBUG_STR.DEBUG[32]);
					sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],",,");
					sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"%02d,",SEQ.SELECT.BIT.MEASURE);
					sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"%03d,",COM0.NO311);
					sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"%d,",SEQ.SELECT.BIT.PEAKHOLD);
#endif

#ifdef	___LOGGING_INTERVAL_RESET
					// 初期パラメータ
					sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"%03d,",COM0.NO295);
					sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"%03d,",COM0.NO296);
					sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"%03d,",COM0.NO298);
					sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"%03d,",COM0.NO297);
					sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"%03d,",COM0.NO299);
#endif

// add 2016.07.14 K.Uemura start	
#ifdef	__LONGGING_SENSOR_LEVEL
					for(temp=0; temp<24; temp++){
						sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"%03d,",COM0.NO3000[temp]);
					}
#endif
// add 2016.07.14 K.Uemura end

#ifdef	__INITIAL_SEQUENCE
					sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"\n");

					// 初期パラメータ(繰り返し)
					/*
					if(SEQ.FLAG.BIT.ECO == 0){
						sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"NORMAL %3d / %2d:(%3d - %3d)\n",STD_LDV_TARGET, STD_LDV_RANGE, STD_LDV_TARGET-STD_LDV_RANGE, STD_LDV_TARGET+STD_LDV_RANGE);
					}else{
						sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"ECO    %3d / %2d:(%3d - %3d)\n",ECO_LDV_TARGET, ECO_LDV_RANGE, STD_LDV_TARGET-STD_LDV_RANGE, STD_LDV_TARGET+STD_LDV_RANGE);
					}
					*/
					if(SEQ.FLAG.BIT.ECO == 0){
						if(DEBUG_STR.DEBUG[99] == Z_DIRECTION){
							sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"NORMAL[Z] %3d / %2d:(%3d - %3d)\n",X_LDV_TARGET, X_LDV_RANGE, X_LDV_TARGET-X_LDV_RANGE, X_LDV_TARGET+X_LDV_RANGE);
						}else{
							sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"NORMAL[X] %3d / %2d:(%3d - %3d)\n",X_LDV_TARGET, X_LDV_RANGE, X_LDV_TARGET-X_LDV_RANGE, X_LDV_TARGET+X_LDV_RANGE);
						}
					}else{
						if(DEBUG_STR.DEBUG[99] == Z_DIRECTION){
							sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"ECO[Z]    %3d / %2d:(%3d - %3d)\n",ECO_LDV_TARGET, ECO_LDV_RANGE, X_LDV_TARGET-X_LDV_RANGE, X_LDV_TARGET+X_LDV_RANGE);
						}else{
							sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"ECO[X]    %3d / %2d:(%3d - %3d)\n",ECO_LDV_TARGET, ECO_LDV_RANGE, X_LDV_TARGET-X_LDV_RANGE, X_LDV_TARGET+X_LDV_RANGE);
						}
					}
					
					for(i=0;i<20;i++){
						//	DEBUG_STR.DEBUG[100+(0*5)+0]
						//	DEBUG_STR.DEBUG[100+(0*5)+3]
						sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"%04d,",DEBUG_STR.DEBUG[100+(i*2)+0]);	// SEQ.DIM_LEVEL;
						sprintf(&SEQ.SIO_BUF[strlen(SEQ.SIO_BUF)],"%04d\n",DEBUG_STR.DEBUG[100+(i*2)+1]);	// SEQ.BLACK_WHITE_CHECK[1];
					}
#endif

					SEQ.SIO_IDX = 0;
					COM2.WR_CONT++;
					SCI2.TDR = SEQ.SIO_BUF[SEQ.SIO_IDX++];
					break;

				case 2:
					if(strlen(SEQ.SIO_BUF) <= (SEQ.SIO_IDX+1)){
						COM2.WR_CONT = 10;
					}
					sci_puts(SEQ.SIO_BUF[SEQ.SIO_IDX]);
					SEQ.SIO_IDX++;
					break;

				// CR
				case 10:
					SCI2.TDR = CR;
					SCI2.SCR.BIT.TIE = 0;				// TXI割り込み要求を禁止
					SCI2.SCR.BIT.TEIE = 1;				// TEI割り込み要求を許可
					COM2.WR_CONT = 1;					// 書き込みｶｳﾝﾀ
					COM2.SUB_STATUS++;					// 次へ
					DEBUG_STR.DEBUG[10] = DEBUG_STR.DEBUG[21] = DEBUG_STR.DEBUG[22] = DEBUG_STR.DEBUG[31] = DEBUG_STR.DEBUG[32] = INITIAL_MIN;
					DEBUG_STR.DEBUG[23] = DEBUG_STR.DEBUG[24] = DEBUG_STR.DEBUG[33] = DEBUG_STR.DEBUG[34] = INITIAL_MAX;
					SEQ.FLAG5.BIT.DEB_SIOOUT_FLAG = 0;
					break;
			}
			break;
	}
}

void sci_puts(char str)
{
	if(str == 0x0a){
		SCI2.TDR = CR;
	}else{
		SCI2.TDR = str;
	}

	return;
}

//************************************************************/
//				RS232C(SCI2)TEI(送信終了割り込み)
//************************************************************/
void rs232c_tei(void)
{
	while(SCI2.SSR.BIT.TEND != 1){}				// TENDﾌﾗｸﾞが「1」になるまで待機
	
	SCI2.SCR.BIT.TEIE = 0;						// TEI割り込み要求を禁止
	SCI2.SCR.BIT.TIE = 0;						// TXI割り込み要求を禁止
	SCI2.SCR.BIT.TE = 0;						// ｼﾘｱﾙ送信動作を禁止
	
	SCI2.SCR.BIT.RIE = 1;						// RXIおよびERI割り込み要求を許可
	SCI2.SCR.BIT.RE = 1;						// ｼﾘｱﾙ受信動作を許可
	
#ifdef	OUTPUT232C
	COM2.SUB_STATUS = 1;
#else
	COM2.SUB_STATUS = 2;
#endif
}

//************************************************************/
//				RS232C制御関数
//************************************************************/
void rs232c_drive(void)
{
	_UWORD i;
	_UBYTE array[8] = {STX, 'M', 'O', 'D', 'E', '0', 0, ETX};	// 受信ﾃﾞｰﾀ確認用配列
	
	switch(COM2.SUB_STATUS){
		// 受信動作の開始
		case 1:
			SCI2.SCR.BIT.TEIE = 0;						// TEI割り込み要求を禁止
			SCI2.SCR.BIT.TIE = 0;						// TXI割り込み要求を禁止
			SCI2.SCR.BIT.TE = 0;						// ｼﾘｱﾙ送信動作を禁止
			
			SCI2.SCR.BIT.RIE = 1;						// RXIおよびERI割り込み要求を許可
			SCI2.SCR.BIT.RE = 1;						// ｼﾘｱﾙ受信動作を許可
			
			COM2.SUB_STATUS++;							// 次へ
			break;
			
		// 受信動作の割り込み
		case 2:
			break;
			
		// 受信ｺﾏﾝﾄﾞに応じてFPGAに対しﾃﾞｰﾀ送信要求を行う
		case 3:
			// 受信ｺﾏﾝﾄﾞの確認
			for(i = 0; i<=7; i++){
				if(i != 6){								// iが「6」以外のとき
					if(COM2.RE_BUF[i] == array[i]){		// 受信内容が正しいとき
						if(i == 7){						// 受信ｺﾏﾝﾄﾞが全て正しいとき
							COM2.SUB_STATUS++;			// 次へ
							if(SEQ.CHANGE_FPGA == 0){
// add 2016.10.20 K.Uemura start	GA2002
								SEQ.BUFFER_NO_OLD = SEQ.BUFFER_NO_NEW = 0;
// add 2016.10.20 K.Uemura end
								SEQ.CHANGE_FPGA = 10;
								SEQ.FPGA_SEND_STATUS = 1;
								DEBUG_STR.DEBUG_COUNT = 0;	// ﾃﾞﾊﾞｯｸﾞ出力用ｶｳﾝﾀ
							}
						}
					}else{								// 受信内容が異なるとき確認を終了する
						COM2.SUB_STATUS = 1;
						break;
					}
				}else{									// iが「6」のとき
					if(COM2.RE_BUF[i] == '1'){			// ﾃﾞｼﾞﾀﾙ値
						SEQ.CBUS_NUMBER = 405;			// 演算前ﾃﾞｰﾀ読み出し
						DEBUG_STR.DEBUG_OUTPUT = 0;			// ﾃﾞﾊﾞｯｸﾞ出力用出力ﾌｫｰﾏｯﾄ		// 0 整数3桁
					}else if(COM2.RE_BUF[i] == '2'){	// 処理済みﾃﾞｰﾀ
						SEQ.CBUS_NUMBER = 406;			// 演算後ﾃﾞｰﾀ読み出し
						DEBUG_STR.DEBUG_OUTPUT = 1;			// ﾃﾞﾊﾞｯｸﾞ出力用出力ﾌｫｰﾏｯﾄ		// 1 符号・整数1桁・小数点3桁
					}else if(COM2.RE_BUF[i] == '4'){	// ﾌﾞﾗｯｸ値
						SEQ.CBUS_NUMBER = 403;			// ﾌﾞﾗｯｸﾃﾞｰﾀ読み出し
						DEBUG_STR.DEBUG_OUTPUT = 0;			// ﾃﾞﾊﾞｯｸﾞ出力用出力ﾌｫｰﾏｯﾄ		// 0 整数3桁
					}else if(COM2.RE_BUF[i] == '8'){	// ﾎﾜｲﾄ値
						SEQ.CBUS_NUMBER = 404;			// ﾎﾜｲﾄﾃﾞｰﾀ読み出し
						DEBUG_STR.DEBUG_OUTPUT = 2;			// ﾃﾞﾊﾞｯｸﾞ出力用出力ﾌｫｰﾏｯﾄ		// 2 整数3桁・小数点3桁
					}else{								// その他
						COM2.SUB_STATUS = 1;
						break;
					}
				}
			}
			break;
			
		// FPGAからﾃﾞｰﾀ受信中
		case 4:
			break;
			
		case 5:
			DEBUG_STR.DEBUG_COUNT = 0;
			COM2.WR_CONT = 1;							// 書き込みｶｳﾝﾀ
			
			COM2.SUB_STATUS++;							// 次へ
			
			SCI2.SCR.BIT.TIE = 1;						// TIEを「1」
			SCI2.SCR.BIT.TE = 1;						// TEを「1」
			break;
			
		// ﾃﾞｰﾀ送信中
		case 6:
			break;
	}
}

//************************************************************/
//				RS232C制御関数
//************************************************************/
void rs232c_drive_debug(void)
{
	if(SEQ.FLAG5.BIT.DEB_SIOOUT_FLAG){
		switch(COM2.SUB_STATUS){
			// 受信動作の開始
			case 1:
				SCI2.SCR.BIT.TEIE = 0;						// TEI割り込み要求を禁止
				SCI2.SCR.BIT.TIE = 0;						// TXI割り込み要求を禁止
				SCI2.SCR.BIT.TE = 0;						// ｼﾘｱﾙ送信動作を禁止
				
				SCI2.SCR.BIT.RIE = 1;						// RXIおよびERI割り込み要求を許可
				SCI2.SCR.BIT.RE = 1;						// ｼﾘｱﾙ受信動作を許可
				
				COM2.SUB_STATUS++;							// 次へ
				break;
				
			// 受信動作の割り込み
			case 2:
				COM2.RE_CONT = 0;
				COM2.RE_BUF[COM2.RE_CONT] = 123;

				SCI2.SCR.BIT.RIE = 0;		// RXIおよびERI割り込み要求を禁止
				SCI2.SCR.BIT.RE = 0;		// ｼﾘｱﾙ受信動作を禁止

				COM2.SUB_STATUS++;			// 次へ
				break;
				
			// 受信ｺﾏﾝﾄﾞに応じてFPGAに対しﾃﾞｰﾀ送信要求を行う
			case 3:
				DEBUG_STR.DEBUG_OUTPUT = 10;

				DEBUG_STR.DEBUG_COUNT = 0;
				DEBUG_STR.DEBUG[DEBUG_STR.DEBUG_COUNT] = ADCOV.V_BATT;
				COM2.SUB_STATUS++;							// 次へ
				break;
				
			case 4:
				COM2.SUB_STATUS++;							// 次へ
				break;
				
			// 送信データ生成待ち（rs232c_txi関数　DEBUG_STR.DEBUG_OUTPUT = 10）
			case 5:
				DEBUG_STR.DEBUG_COUNT = 0;
				COM2.WR_CONT = 1;							// 書き込みｶｳﾝﾀ
				
				COM2.SUB_STATUS++;							// 次へ
				
				SCI2.SCR.BIT.TIE = 1;						// TIEを「1」
				SCI2.SCR.BIT.TE = 1;						// TEを「1」
				break;
				
			// ﾃﾞｰﾀ送信中
			case 6:
				break;
		}
	}
}

//************************************************************/
//				RS232C制御関数
//************************************************************/
unsigned char get_number(unsigned long number, unsigned char digit)
{
	unsigned char number_digit;
	unsigned long multiplier;			// 乗数
	unsigned char i;


	multiplier = 0;

	if(1 <= (digit-1)){
		multiplier = 1;
		for(i=0;i<(digit-1);i++){
			multiplier *= 10;
		}
	}

	if(multiplier != 0){
		number_digit = ((number / multiplier) % 10 + 0x30);
	}else{
		number_digit = ((number) % 10 + 0x30);
	}

	return(number_digit);
}

//************************************************************/
//				電圧取得（adcov → voltage）
//************************************************************/
unsigned short get_voltage(unsigned short adcov)
{
	unsigned char idx;

	if(adcov < LUT_VOLTAGE_MIN){
		return(0);
	}else if(LUT_VOLTAGE_MAX < adcov){
		return(9999);
	}else{
		idx = adcov - LUT_VOLTAGE_MIN;
		return(LUT_VOLTAGE[idx]);
	}
}
