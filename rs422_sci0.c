/******************************************************************************
* File Name	: rs422_sci0.c
******************************************************************************/
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "iodefine.h"
#include "typedefine.h"
#include "usercopy.h"
#include "user_define.h"
#include "ASC2.h"
// add 2016.10.18 K.Uemura start	GA1801
#include "parameter.h"
// add 2016.10.18 K.Uemura end

void sci0_init(void);														// RS422初期設定
void rs422_master(void);													// ﾒｲﾝｼｰｹﾝｽ関数

void rs422_rxi(void);														// RS422受信
void rs422_txi(void);														// RS422送信
void rs422_tei(void);														// RS422送信終了

void rs422_init(void);														// RS422初期化関数
void rs422_drive(void);														// RS422制御関数

void rs422_send_command2(_UBYTE rw , _UWORD address_num , _UWORD data_num);	// RS422送信ｺﾏﾝﾄﾞ関数
void rs422_receive_read(void);												// RS422受信ﾃﾞｰﾀﾘｰﾄﾞ関数
void rs422_write_command_set2(_UWORD data);									// RS422書き込みｺﾏﾝﾄﾞｾｯﾄ
void rs422_write_address_set2(_UWORD address);								// RS422書き込みｱﾄﾞﾚｽｾｯﾄ
void rs422_write_cr_set2(void);												// RS422書き込みCRｾｯﾄ

void end_condition_set(void);												// 終了条件ｾｯﾄ
_UWORD make_okcount(void);													// 終了条件計算
// add 2016.10.18 K.Uemura start	GA1801
_UWORD verify_DLPM(unsigned short *, unsigned short *);						// DLPM確認
// add 2016.10.18 K.Uemura end

//************************************************************/
//				RS422初期設定(SCI0)
//************************************************************/
void sci0_init(void)
{
	MSTP(SCI0) = 0;			// RIIC0ﾓｼﾞｭｰﾙｽﾄｯﾌﾟ解除
	
	// 割り込み要求禁止
	IEN(SCI0, TEI0) = 0;
	IEN(SCI0, ERI0) = 0;
	IEN(SCI0, TXI0) = 0;
	IEN(SCI0, RXI0) = 0;

	SCI0.SCR.BYTE = 0x00;

	while (0x00 != (SCI0.SCR.BYTE & 0xF0)){}

	// Select an On-chip baud rate generator to the clock source
	SCI0.SCR.BIT.CKE = 0;

	// SMR - Serial Mode Register
	SCI0.SMR.BYTE = 0x00;

	// SCMR - Smart Card Mode Register
	SCI0.SCMR.BYTE = 0xF2;

	// SEMR - Serial Extended Mode Register
	SCI0.SEMR.BYTE = 0x00;
	
	// BRR - Bit Rate Register
	// 115200bps
	//Bit Rate: (48MHz/(64*2^(-1)*115200bps))-1 = 12.0208 ≒ 12
	SCI0.BRR = 12;			// 115200bps
	
	delay_ms(1);			// 1ﾋﾞｯﾄ期間待機
	
	// 割り込み優先ﾚﾍﾞﾙ設定
	IPR(SCI0, TXI0) = 1;
	IPR(SCI0, RXI0) = 1;
	IPR(SCI0, TEI0) = 1;
	IPR(SCI0, ERI0) = 1;
	
	// 割り込み要求ｸﾘｱ
	IR(SCI0, TXI0) = 0;
	IR(SCI0, RXI0) = 0;
	IR(SCI0, TEI0) = 0;
	IR(SCI0, ERI0) = 0;
	
	// 割り込み要求許可
	IEN(SCI0, TXI0) = 1;
	IEN(SCI0, RXI0) = 1;
	IEN(SCI0, ERI0) = 1;
	IEN(SCI0, TEI0) = 1;
}

//************************************************************/
//				ﾒｲﾝｼｰｹﾝｽ関数					
//************************************************************/
void rs422_master(void)
{
	switch(COM0.MASTER_STATUS){
		case RESET_MODE:
			break;
		case INIT_START_MODE:
			rs422_init();
			break;
		case DRV_MODE:
			rs422_drive();
			break;
		case IDLE_MODE:
			break;
		default:
			COM0.MASTER_STATUS = RESET_MODE;
	}
}

/********************************************************/
/*		RS422(SCI0)RXI(受信ﾃﾞｰﾀﾌﾙ割り込み)
/********************************************************/
void rs422_rxi(void)
{
	COM0.RE_BUF[COM0.RE_CONT] = SCI0.RDR;
	
	if(COM0.RE_BUF[COM0.RE_CONT] == CR){		// 受信ﾃﾞｰﾀが「CR」のとき
		SCI0.SCR.BIT.RIE = 0;					// RXIおよびERI割り込み要求を禁止
		SCI0.SCR.BIT.RE = 0;					// ｼﾘｱﾙ受信動作を禁止
		COM0.SUB_STATUS++;
	}
	
	COM0.RE_CONT++;
}

/********************************************************/
/*		RS422(SCI0)TXI(送信ﾃﾞｰﾀｴﾝﾌﾟﾃｨ割り込み)
/********************************************************/
void rs422_txi(void)
{
	SCI0.TDR = COM0.WR_BUF[COM0.WR_CONT];
	
	if(COM0.WR_CONT == COM0.SEND_COUNT){		// 最後まで送信したら
		SCI0.SCR.BIT.TIE = 0;					// TXI割り込み要求を禁止
		SCI0.SCR.BIT.TEIE = 1;					// TEI割り込み要求を許可
	}
	
	COM0.WR_CONT++;
}
	
/********************************************************/
/*		RS422(SCI0)TEI(送信終了割り込み)
/********************************************************/
void rs422_tei(void)
{
	while(SCI0.SSR.BIT.TEND != 1){}				// TENDﾌﾗｸﾞが「1」になるまで待機
	
	SCI0.SCR.BIT.TEIE = 0;						// TEI割り込み要求を禁止
	SCI0.SCR.BIT.TIE = 0;						// TXI割り込み要求を禁止
	SCI0.SCR.BIT.TE = 0;						// ｼﾘｱﾙ送信動作を禁止
	
	SCI0.SCR.BIT.RIE = 1;						// RXIおよびERI割り込み要求を許可
	SCI0.SCR.BIT.RE = 1;						// ｼﾘｱﾙ受信動作を許可
	
	COM0.SUB_STATUS++;
}

//************************************************************/
//				RS422初期化関数
//************************************************************/
void rs422_init(void)
{
	_UWORD temp;
	
	switch(COM0.SUB_STATUS){
		// ﾀｽｸ分散のため、記述を分割する
		// 100usごと
		
		// ﾃﾞｰﾀの読み出し(2032)	////////////////////////////////////////////////////////
		// ｱﾄﾞﾚｽのｾｯﾄ
		case 1:
			// ﾎﾟｰﾘﾝｸﾞするｱﾄﾞﾚｽ
			SCI0.SCR.BIT.RIE = 0;							// RXIおよびERI割り込み要求を禁止
			SCI0.SCR.BIT.RE = 0;							// ｼﾘｱﾙ受信動作を禁止
			COM0.SET_COUNT = 1;
			COM0.SUB_STATUS++;
			break;
		
		// ｱﾄﾞﾚｽのｾｯﾄ
		case 2:
			rs422_send_command2('R', 2032, 1);				// 「読み出し」 開始ｱﾄﾞﾚｽ「2032」 ﾃﾞｰﾀ数「1」
			break;
		
		// ﾀｲﾑｱｳﾄｾｯﾄ
		case 3:
			ctl_uni_timer2((POLLING_CYCLE/10) + 100);		// 通信ﾀｲﾑｱｳﾄﾀｲﾏｰ ﾎﾟｰﾘﾝｸﾞ周期+1s
			COM0.SUB_STATUS++;
			break;
		
		// ｺﾏﾝﾄﾞ送信
		case 4:
			COM0.WR_CONT = 0;
			COM0.RE_CONT = 0;
			
			COM0.SUB_STATUS = 9;
			
			SCI0.SCR.BIT.TIE = 1;							// TIEを「1」
			SCI0.SCR.BIT.TE = 1;							// TEを「1」
			break;
		////////////////////////////////////////////////////////////////////////////////
		
		// (送信ﾃﾞｰﾀｴﾝﾌﾟﾃｨ割り込み)・(受信ﾃﾞｰﾀﾌﾙ割り込み)を通過しｽﾃｰﾀｽが「+2」進む
		
		// ﾃﾞｰﾀの格納(2032)	////////////////////////////////////////////////////////////
		// ｺﾏﾝﾄﾞ受信
		case 11:
			COM0.RECEIVE_COUNT = 1;							// 受信文字数
			COM0.RECEIVE_DATA_COUNT = 1;					// 受信ﾃﾞｰﾀｶｳﾝﾄ
			COM0.SET_COUNT = 1;
			rs422_receive_read();							// RS422受信ﾃﾞｰﾀﾘｰﾄﾞ関数
			COM0.SUB_STATUS++;
			break;
			
		// ｺﾏﾝﾄﾞ受信
		case 12:
			rs422_receive_read();							// RS422受信ﾃﾞｰﾀﾘｰﾄﾞ関数
			break;
		
		// ｺﾏﾝﾄﾞ受信
		case 13:
			temp = COM0.RECEIVE_DATA;
			temp = (temp >> 4) & 0x01;
			// 常時ONﾋﾞｯﾄが「1」であるか
			if(temp == 1){
				COM0.START_ADDRESS = 1001;					// 開始ｱﾄﾞﾚｽ
				COM0.SUB_STATUS = 141;
				//SEQ.FLAG.BIT.MEMORY_CONTROL = 1;
				
			}else{
				COM0.SUB_STATUS = 10;						// ﾀｲﾑｱｳﾄ待ち
			}
			break;
			
		// ﾊﾟﾗﾒｰﾀの書き込み(1001 - 2000)	////////////////////////////////////////////
		// ｱﾄﾞﾚｽをｾｯﾄ
		case 141:
			COM0.SEND_DATA = COM0.START_ADDRESS;
			COM0.SET_COUNT = 1;
			rs422_write_address_set2(COM0.SEND_DATA);		// RS422書き込みｱﾄﾞﾚｽｾｯﾄ
			COM0.SUB_STATUS++;
			break;
				
		case 142:
			rs422_write_address_set2(COM0.SEND_DATA);		// RS422書き込みｱﾄﾞﾚｽｾｯﾄ
			break;
		
		// ﾊﾟﾗﾒｰﾀの書き込み(1001 - 2000)
		case 143:
			COM0.SEND_DATA_COUNT = 1;						// 送信ﾃﾞｰﾀ数
			COM0.SEND_DATA = SEQ.ALL_DATA[COM0.SEND_DATA_COUNT];
			COM0.SET_COUNT = 1;
			rs422_write_command_set2(COM0.SEND_DATA);
			COM0.SUB_STATUS++;
			break;
			
		case 144:
			rs422_write_command_set2(COM0.SEND_DATA);
			break;
			
		// ﾊﾟﾗﾒｰﾀの書き込み(1001 - 2000)
		case 145:
			COM0.SEND_DATA_COUNT++;							// 送信ﾃﾞｰﾀ数
			
			if(COM0.SEND_DATA_COUNT < 1001){
				COM0.SEND_DATA = SEQ.ALL_DATA[COM0.SEND_DATA_COUNT];
				COM0.SET_COUNT = 1;
				rs422_write_command_set2(COM0.SEND_DATA);
				COM0.SUB_STATUS--;
			}else{
				rs422_write_cr_set2();						// RS422書き込みCRｾｯﾄ
				COM0.SUB_STATUS = 148;
			}
			break;
			
		// ﾀｲﾑｱｳﾄｾｯﾄ
		case 148:
			ctl_uni_timer2((POLLING_CYCLE/10) + 100);		// 通信ﾀｲﾑｱｳﾄﾀｲﾏｰ ﾎﾟｰﾘﾝｸﾞ周期+1s
			COM0.SUB_STATUS++;
			break;
			
		// 書き込み
		case 149:
			SCI0.SCR.BIT.RIE = 0;							// RXIおよびERI割り込み要求を禁止
			SCI0.SCR.BIT.RE = 0;							// ｼﾘｱﾙ受信動作を禁止
			
			COM0.WR_CONT = 0;
			COM0.RE_CONT = 0;
			
			COM0.SUB_STATUS++;
			
			SCI0.SCR.BIT.TIE = 1;							// TIEを「1」
			SCI0.SCR.BIT.TE = 1;							// TEを「1」
			break;
			
		// ﾊﾟﾗﾒｰﾀの書き込み(333)	////////////////////////////////////////////////////
		// 起動ﾋﾞｯﾄ
		// ｱﾄﾞﾚｽをｾｯﾄ
		case 151:
			//SEQ.FLAG.BIT.MEMORY_CONTROL = 0;
			COM0.START_ADDRESS = 333;						// 開始ｱﾄﾞﾚｽ
			COM0.SEND_DATA = COM0.START_ADDRESS;
			COM0.SET_COUNT = 1;
			rs422_write_address_set2(COM0.SEND_DATA);		// RS422書き込みｱﾄﾞﾚｽｾｯﾄ
			COM0.SUB_STATUS++;
			break;
				
		case 152:
			rs422_write_address_set2(COM0.SEND_DATA);		// RS422書き込みｱﾄﾞﾚｽｾｯﾄ
			break;
			
		// ﾊﾟﾗﾒｰﾀの書き込み(333)
		case 153:
			//COM0.NO333 = 1;
			COM0.NO333.BIT.LINK = 1;						// LINK UP
			COM0.SEND_DATA = COM0.NO333.WORD;
			COM0.SET_COUNT = 1;
			rs422_write_command_set2(COM0.SEND_DATA);
			COM0.SUB_STATUS++;
			break;
			
		case 154:
			rs422_write_command_set2(COM0.SEND_DATA);
			break;
			
		// ﾊﾟﾗﾒｰﾀの書き込み(333)
		case 155:
			rs422_write_cr_set2();							// RS422書き込みCRｾｯﾄ
			COM0.SUB_STATUS = 158;
			break;
			
		// ﾀｲﾑｱｳﾄｾｯﾄ
		case 158:
			ctl_uni_timer2((POLLING_CYCLE/10) + 100);		// 通信ﾀｲﾑｱｳﾄﾀｲﾏｰ ﾎﾟｰﾘﾝｸﾞ周期+1s
			COM0.SUB_STATUS++;
			break;
			
		// 書き込み
		case 159:
			SCI0.SCR.BIT.RIE = 0;							// RXIおよびERI割り込み要求を禁止
			SCI0.SCR.BIT.RE = 0;							// ｼﾘｱﾙ受信動作を禁止
			
			COM0.WR_CONT = 0;
			COM0.RE_CONT = 0;
			
			COM0.SUB_STATUS++;
			
			SCI0.SCR.BIT.TIE = 1;							// TIEを「1」
			SCI0.SCR.BIT.TE = 1;							// TEを「1」
			break;
			
		// ADD 150729
		// ﾃﾞｰﾀの書き込み(98 - 99)	////////////////////////////////////////////////
		// ｱﾄﾞﾚｽをｾｯﾄ
		case 161:
			COM0.SEND_DATA = 98;							// 書き込み開始ｱﾄﾞﾚｽ
			COM0.SET_COUNT = 1;
			rs422_write_address_set2(COM0.SEND_DATA);		// RS422書き込みｱﾄﾞﾚｽｾｯﾄ
			COM0.SUB_STATUS++;
			break;
				
		case 162:
			rs422_write_address_set2(COM0.SEND_DATA);		// RS422書き込みｱﾄﾞﾚｽｾｯﾄ
			break;
			
		// ﾃﾞｰﾀの書き込み(98 - 99)
		case 163:
			COM0.SEND_DATA_COUNT = 1;						// 送信ﾃﾞｰﾀ数
			//COM0.SEND_DATA = COM0.NO310.WORD;				// 303 状態
			COM0.SEND_DATA = SEQ.FPGA_RIVISION;				// 98 FPGAﾊﾞｰｼﾞｮﾝ
			COM0.SET_COUNT = 1;
			rs422_write_command_set2(COM0.SEND_DATA);
			COM0.SUB_STATUS++;
			break;
			
		case 164:
			rs422_write_command_set2(COM0.SEND_DATA);
			break;
			
		// ﾃﾞｰﾀの書き込み(98 - 99)
		case 165:
			COM0.SEND_DATA_COUNT++;							// 送信ﾃﾞｰﾀ数
			
			switch(COM0.SEND_DATA_COUNT){
				//case 2:		COM0.SEND_DATA = COM0.NO311;				break;		// 99 状態番号
				case 2:		COM0.SEND_DATA = RX_RIVISION;				break;		// 99 RXﾊﾞｰｼﾞｮﾝ
			}
			
			if(COM0.SEND_DATA_COUNT < 3){
				COM0.SET_COUNT = 1;
				rs422_write_command_set2(COM0.SEND_DATA);
				COM0.SUB_STATUS--;
			}else{
				rs422_write_cr_set2();						// RS422書き込みCRｾｯﾄ
				COM0.SUB_STATUS = 169;
			}
			break;
			
		// 書き込み
		case 169:
			SCI0.SCR.BIT.RIE = 0;							// RXIおよびERI割り込み要求を禁止
			SCI0.SCR.BIT.RE = 0;							// ｼﾘｱﾙ受信動作を禁止
		
			COM0.WR_CONT = 0;
			COM0.RE_CONT = 0;
			
			COM0.SUB_STATUS++;
			
			SCI0.SCR.BIT.TIE = 1;							// TIEを「1」
			SCI0.SCR.BIT.TE = 1;							// TEを「1」
			break;
		////////////////////////////////////////////////////////////////////////////////
			
		// 起動ﾋﾞｯﾄ
		//case 161:
		case 171:
			COM0.MASTER_STATUS = DRV_MODE;
			COM0.SUB_STATUS = 201;
			break;
	}
}

/********************************************************/
/*		RS422送信ｺﾏﾝﾄﾞ関数
/********************************************************/
// rw			: 'R'「読み出し」　'W'「書き込み」
// address_num	: 開始ｱﾄﾞﾚｽ
// data_num		: ﾃﾞｰﾀ数

void rs422_send_command2(_UBYTE rw , _UWORD address_num , _UWORD data_num)
{
	_UBYTE buf;
	
	switch(COM0.SET_COUNT){				// ｺﾏﾝﾄﾞｾｯﾄｶｳﾝﾄ
		case 1:
			COM0.SEND_COUNT = 0;
			COM0.WR_BUF[COM0.SEND_COUNT] = ESC;
			COM0.SET_COUNT++;
			break;
			
		case 2:
			COM0.SEND_COUNT++;
			COM0.WR_BUF[COM0.SEND_COUNT] = rw;
			COM0.SET_COUNT++;
			break;
			
		case 3:
			COM0.SEND_COUNT++;
			buf = (address_num >> 12) & 15;
			// 結果が10-15のとき、'A'-'F'に変換する
			if(buf > 9)		COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x37;
			else			COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x30;
			COM0.SET_COUNT++;
			break;
			
		case 4:
			COM0.SEND_COUNT++;
			buf = (address_num >> 8) & 15;
			// 結果が10-15のとき、'A'-'F'に変換する
			if(buf > 9)		COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x37;
			else			COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x30;
			COM0.SET_COUNT++;
			break;
			
		case 5:
			COM0.SEND_COUNT++;
			buf = (address_num >> 4) & 15;
			// 結果が10-15のとき、'A'-'F'に変換する
			if(buf > 9)		COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x37;
			else			COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x30;
			COM0.SET_COUNT++;
			break;
			
		case 6:
			COM0.SEND_COUNT++;
			buf = address_num & 15;
			// 結果が10-15のとき、'A'-'F'に変換する
			if(buf > 9)		COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x37;
			else			COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x30;
			COM0.SET_COUNT++;
			break;
			
		case 7:
			COM0.SEND_COUNT++;
			buf = (data_num >> 12) & 15;
			// 結果が10-15のとき、'A'-'F'に変換する
			if(buf > 9)		COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x37;
			else			COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x30;
			COM0.SET_COUNT++;
			break;
			
		case 8:
			COM0.SEND_COUNT++;
			buf = (data_num >> 8) & 15;
			// 結果が10-15のとき、'A'-'F'に変換する
			if(buf > 9)		COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x37;
			else			COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x30;
			COM0.SET_COUNT++;
			break;
			
		case 9:
			COM0.SEND_COUNT++;
			buf = (data_num >> 4) & 15;
			// 結果が10-15のとき、'A'-'F'に変換する
			if(buf > 9)		COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x37;
			else			COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x30;
			COM0.SET_COUNT++;
			break;
			
		case 10:
			COM0.SEND_COUNT++;
			buf = data_num & 15;
			// 結果が10-15のとき、'A'-'F'に変換する
			if(buf > 9)		COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x37;
			else			COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x30;
			COM0.SET_COUNT++;
			break;
			
		case 11:
			COM0.SEND_COUNT++;
			COM0.WR_BUF[COM0.SEND_COUNT] = CR;
			COM0.SET_COUNT = 1;
			COM0.SUB_STATUS++;		// 次へ
			break;
			
		default:
			COM0.SEND_COUNT = 0;
			COM0.WR_BUF[COM0.SEND_COUNT] = ESC;
			COM0.SET_COUNT = 2;
	}
}

/********************************************************/
/*		RS422受信ﾃﾞｰﾀﾘｰﾄﾞ関数
/********************************************************/
void rs422_receive_read(void)
{
	_UWORD buf;
	
	switch(COM0.SET_COUNT){					// ｺﾏﾝﾄﾞｾｯﾄｶｳﾝﾄ
		case 1:
			COM0.RECEIVE_DATA = 0;			// 受信ﾃﾞｰﾀ
			COM0.RECEIVE_COUNT++;			// 受信文字数
			if(COM0.RE_BUF[COM0.RECEIVE_COUNT] > '9')	buf = COM0.RE_BUF[COM0.RECEIVE_COUNT] - 0x37;
			else										buf = COM0.RE_BUF[COM0.RECEIVE_COUNT] - 0x30;
			COM0.RECEIVE_DATA = buf << 12;
			COM0.SET_COUNT++;
			break;
			
		case 2:
			COM0.RECEIVE_COUNT++;			// 受信文字数
			if(COM0.RE_BUF[COM0.RECEIVE_COUNT] > '9')	buf = COM0.RE_BUF[COM0.RECEIVE_COUNT] - 0x37;
			else										buf = COM0.RE_BUF[COM0.RECEIVE_COUNT] - 0x30;
			COM0.RECEIVE_DATA |= buf << 8;
			COM0.SET_COUNT++;
			break;
			
		case 3:
			COM0.RECEIVE_COUNT++;			// 受信文字数
			if(COM0.RE_BUF[COM0.RECEIVE_COUNT] > '9')	buf = COM0.RE_BUF[COM0.RECEIVE_COUNT] - 0x37;
			else										buf = COM0.RE_BUF[COM0.RECEIVE_COUNT] - 0x30;
			COM0.RECEIVE_DATA |= buf << 4;
			COM0.SET_COUNT++;
			break;
			
		case 4:
			COM0.RECEIVE_COUNT++;			// 受信文字数
			if(COM0.RE_BUF[COM0.RECEIVE_COUNT] > '9')	buf = COM0.RE_BUF[COM0.RECEIVE_COUNT] - 0x37;
			else										buf = COM0.RE_BUF[COM0.RECEIVE_COUNT] - 0x30;
			COM0.RECEIVE_DATA |= buf;
			
			COM0.SET_COUNT = 1;
			COM0.SUB_STATUS++;				// 次へ
			break;
			
		default:
			COM0.SET_COUNT = 1;
	}
}

/********************************************************/
/*		RS422書き込みｺﾏﾝﾄﾞｾｯﾄ
/********************************************************/
// rw			: 'R'「読み出し」　'W'「書き込み」
// address_num	: 開始ｱﾄﾞﾚｽ
// data_num		: ﾃﾞｰﾀ数

void rs422_write_command_set2(_UWORD data)
{
	_UBYTE buf;
	
	switch(COM0.SET_COUNT){				// ｺﾏﾝﾄﾞｾｯﾄｶｳﾝﾄ
		case 1:
			COM0.SEND_COUNT++;
			buf = (data >> 12) & 15;
			// 結果が10-15のとき、'A'-'F'に変換する
			if(buf > 9)		COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x37;
			else			COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x30;
			COM0.SET_COUNT++;
			break;
			
		case 2:
			COM0.SEND_COUNT++;
			buf = (data >> 8) & 15;
			// 結果が10-15のとき、'A'-'F'に変換する
			if(buf > 9)		COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x37;
			else			COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x30;
			COM0.SET_COUNT++;
			break;
			
		case 3:
			COM0.SEND_COUNT++;
			buf = (data >> 4) & 15;
			// 結果が10-15のとき、'A'-'F'に変換する
			if(buf > 9)		COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x37;
			else			COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x30;
			COM0.SET_COUNT++;
			break;
			
		case 4:
			COM0.SEND_COUNT++;
			buf = data & 15;
			// 結果が10-15のとき、'A'-'F'に変換する
			if(buf > 9)		COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x37;
			else			COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x30;
			COM0.SET_COUNT = 1;
			COM0.SUB_STATUS++;		// 次へ
			break;
			
		default:
			COM0.SET_COUNT = 1;
	}
}

/********************************************************/
/*		RS422書き込みｱﾄﾞﾚｽｾｯﾄ
//	1 → 2 → 3 → 4 → 5 → 6  COM0.SUB_STATUS++
/********************************************************/
void rs422_write_address_set2(_UWORD address)
{
	_UBYTE buf;
	
	switch(COM0.SET_COUNT){				// ｺﾏﾝﾄﾞｾｯﾄｶｳﾝﾄ
		case 1:
			COM0.SEND_COUNT = 0;
			COM0.WR_BUF[COM0.SEND_COUNT] = ESC;
			COM0.SET_COUNT++;
			break;
			
		case 2:
			COM0.SEND_COUNT++;
			COM0.WR_BUF[COM0.SEND_COUNT] = 'W';
			COM0.SET_COUNT++;
			break;
			
		case 3:
			COM0.SEND_COUNT++;
			buf = (address >> 12) & 15;
			// 結果が10-15のとき、'A'-'F'に変換する
			if(buf > 9)		COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x37;
			else			COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x30;
			COM0.SET_COUNT++;
			break;
			
		case 4:
			COM0.SEND_COUNT++;
			buf = (address >> 8) & 15;
			// 結果が10-15のとき、'A'-'F'に変換する
			if(buf > 9)		COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x37;
			else			COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x30;
			COM0.SET_COUNT++;
			break;
			
		case 5:
			COM0.SEND_COUNT++;
			buf = (address >> 4) & 15;
			// 結果が10-15のとき、'A'-'F'に変換する
			if(buf > 9)		COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x37;
			else			COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x30;
			COM0.SET_COUNT++;
			break;
			
		case 6:
			COM0.SEND_COUNT++;
			buf = address & 15;
			// 結果が10-15のとき、'A'-'F'に変換する
			if(buf > 9)		COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x37;
			else			COM0.WR_BUF[COM0.SEND_COUNT] = buf + 0x30;
			COM0.SET_COUNT = 1;
			COM0.SUB_STATUS++;		// 次へ
			break;
			
		default:
			COM0.SET_COUNT = 1;
	}
}

/********************************************************/
//		RS422書き込みCRｾｯﾄ
/********************************************************/
void rs422_write_cr_set2(void)
{
	COM0.SEND_COUNT++;
	COM0.WR_BUF[COM0.SEND_COUNT] = CR;
}

//************************************************************/
//				RS422制御関数
//	
//	COM0.NO299 ～ 309：11WORD
//	COM0.NO310 ～ 333：24WORD
//	
//	
//	  1 →   2
//	  3 →   4 →   9
//	 11 →  12
//	 13 →  14 →  21 →  22
//	    →  12
//	 23 →  24
//	 25 →  24
//	    →  29
//	 29 →  30
//	 31 →  32
//	 33 →  34                       rs422_write_command_set2(COM0.SUB_STATUS + 1)
//	 35 →  39 →  40
//	           →  50
//	           →  60
//	           → 101
//	           → 121
//	           → 141
//	           → 161
//	           → 200
//	    →  38
//	 41 →  42
//	 43 →  44
//	 45 →  44
//	    →  49 → 200
//	 51 →  52
//	 53 →  54
//	 55 →  58 →  59 → 200
//	 61 →  62
//	 63 →  64
//	 65 →  64
//	    →  69 → 200
//	101 → 102
//	103 → 104 → 109
//	111 → 112
//	113 → 101
//	    → 114 → 201
//	    → 112
//	121 → 122
//	123 → 124 → 129
//	131 → 132
//	133 → 121
//	    → 134 → 135 → 136 → 201
//	    → 132
//	141 → 142
//	143 → 144
//	145 → 144
//	    → 148 → 149 → 150
//	151 → 201
//	161 → 162
//	163 → 164
//	165 → 164
//	    → 168 → 169
//	171 → 172
//	173 → 174
//	175 → 174
//	    → 178 → 179 → 180
//	181 → 182
//	183 → 184
//	185 → 184
//	    → 188 → 189 → 190
//	191 → 201
//************************************************************/
void rs422_drive(void)
{
	_UWORD i;
	
	switch(COM0.SUB_STATUS){
		// ﾀｽｸ分散のため、記述を分割する
		// 100usごと
		
		// ﾃﾞｰﾀの読み出し(299 - 302)	////////////////////////////////////////////////
		// ｱﾄﾞﾚｽのｾｯﾄ
		case 1:
			// ﾎﾟｰﾘﾝｸﾞするｱﾄﾞﾚｽ
			SCI0.SCR.BIT.RIE = 0;							// RXIおよびERI割り込み要求を禁止
			SCI0.SCR.BIT.RE = 0;							// ｼﾘｱﾙ受信動作を禁止
			COM0.SET_COUNT = 1;
			COM0.SUB_STATUS++;
			break;
		
		// ｱﾄﾞﾚｽのｾｯﾄ
		case 2:
			rs422_send_command2('R', 299, 11);				// 「読み出し」 開始ｱﾄﾞﾚｽ「299」 ﾃﾞｰﾀ数「11」
			break;
			
		// ﾀｲﾑｱｳﾄｾｯﾄ
		case 3:
			ctl_uni_timer2((POLLING_CYCLE/10) + 100);		// 通信ﾀｲﾑｱｳﾄﾀｲﾏｰ ﾎﾟｰﾘﾝｸﾞ周期+1s
			COM0.SUB_STATUS++;
			break;
		
		// ｺﾏﾝﾄﾞ送信
		case 4:
			COM0.WR_CONT = 0;
			COM0.RE_CONT = 0;
			
			COM0.SUB_STATUS = 9;
			
			SCI0.SCR.BIT.TIE = 1;							// TIEを「1」
			SCI0.SCR.BIT.TE = 1;							// TEを「1」
			break;
		////////////////////////////////////////////////////////////////////////////////
			
		// (送信ﾃﾞｰﾀｴﾝﾌﾟﾃｨ割り込み)・(受信ﾃﾞｰﾀﾌﾙ割り込み)を通過しｽﾃｰﾀｽが「+2」進む
		
		// ﾃﾞｰﾀの格納(299 - 309)	////////////////////////////////////////////////////
		// ｺﾏﾝﾄﾞ受信
		case 11:
			COM0.RECEIVE_COUNT = 1;							// 受信文字数
			COM0.RECEIVE_DATA_COUNT = 1;					// 受信ﾃﾞｰﾀｶｳﾝﾄ
			COM0.SET_COUNT = 1;
			rs422_receive_read();							// RS422受信ﾃﾞｰﾀﾘｰﾄﾞ関数
			COM0.SUB_STATUS++;
			break;
			
		// ｺﾏﾝﾄﾞ受信
		case 12:
			rs422_receive_read();							// RS422受信ﾃﾞｰﾀﾘｰﾄﾞ関数
			break;
			
		// ｺﾏﾝﾄﾞ受信
		case 13:
			switch(COM0.RECEIVE_DATA_COUNT){
				case 1:		COM0.NO299		= COM0.RECEIVE_DATA;			break;		// 299 ﾊﾞｯﾌｧ転送のﾊﾝﾄﾞｼｪｲｸｱﾄﾞﾚｽ
// chg 2016.09.02 K.Uemura start	G90201
				case 2:		COM0.Dummy		= COM0.RECEIVE_DATA;			break;		// 300 指令
//				case 2:		COM0.NO300.WORD	= COM0.RECEIVE_DATA;			break;		// 300 指令
// chg 2016.09.02 K.Uemura end
				case 3:		COM0.NO301		= COM0.RECEIVE_DATA;			break;		// 301 指令引数
				case 4:		COM0.NO302		= COM0.RECEIVE_DATA;			break;		// 302 予備
				case 5:		COM0.NO303		= COM0.RECEIVE_DATA;			break;		// 303 回転数
				case 6:		COM0.NO304		= COM0.RECEIVE_DATA;			break;		// 
				case 7:		COM0.NO305		= COM0.RECEIVE_DATA;			break;		// 305 刃数
				case 8:		COM0.NO306		= COM0.RECEIVE_DATA;			break;		// 306 工具径
				case 9:		COM0.NO307		= COM0.RECEIVE_DATA;			break;		// 307 引数
				case 10:	COM0.NO308		= COM0.RECEIVE_DATA;			break;		// 
				
				case 11:	COM0.NO309		= COM0.RECEIVE_DATA;
							COM0.NO300.WORD	= COM0.Dummy;					break;		// 300 指令
				
				/*
				case 11:	COM0.NO309		= COM0.RECEIVE_DATA;			break;		// 
// chg 2016.09.02 K.Uemura start	G90201
				case 12:	COM0.NO300.WORD	= COM0.Dummy;					break;		// 300 指令
				*/
//				case 12:	COM0.receive[0]	= COM0.RECEIVE_DATA;			break;		// 
//				case 13:	COM0.receive[1]	= COM0.RECEIVE_DATA;			break;		// 
//				case 14:	COM0.receive[2]	= COM0.RECEIVE_DATA;			break;		// 
// chg 2016.09.02 K.Uemura end
			}
			COM0.RECEIVE_DATA_COUNT++;						// 受信ﾃﾞｰﾀｶｳﾝﾄ
			
// chg 2016.09.02 K.Uemura start	G90201
//			if(COM0.RECEIVE_DATA_COUNT < 13){
			if(COM0.RECEIVE_DATA_COUNT < 12){
// chg 2016.09.02 K.Uemura end
				COM0.SET_COUNT = 1;
				rs422_receive_read();						// RS422受信ﾃﾞｰﾀﾘｰﾄﾞ関数
				COM0.SUB_STATUS--;
			}else{
				COM0.SUB_STATUS++;
			}
			break;
			
		// ﾘｾｯﾄ処理
		case 14:
			COM0.SUB_STATUS = 21;
			break;
		////////////////////////////////////////////////////////////////////////////////
		
		// ﾃﾞｰﾀの書き込み(310 - 333)	////////////////////////////////////////////////
		// ｱﾄﾞﾚｽをｾｯﾄ
		case 21:
			COM0.SEND_DATA = 310;							// 書き込み開始ｱﾄﾞﾚｽ
			COM0.SET_COUNT = 1;
			rs422_write_address_set2(COM0.SEND_DATA);		// RS422書き込みｱﾄﾞﾚｽｾｯﾄ
			COM0.SUB_STATUS++;
			break;
				
		case 22:
			rs422_write_address_set2(COM0.SEND_DATA);		// RS422書き込みｱﾄﾞﾚｽｾｯﾄ
			break;
			
		// ﾃﾞｰﾀの書き込み(310 - 333)
		case 23:
			COM0.SEND_DATA_COUNT = 10;						// 送信ﾃﾞｰﾀ数
			COM0.SEND_DATA = COM0.NO310.WORD;				// 310 状態
			COM0.SET_COUNT = 1;
			rs422_write_command_set2(COM0.SEND_DATA);
			COM0.SUB_STATUS++;
			break;
			
		case 24:
			rs422_write_command_set2(COM0.SEND_DATA);
			break;
			
		// ﾃﾞｰﾀの書き込み(310 - 333)
		case 25:
			COM0.SEND_DATA_COUNT++;							// 送信ﾃﾞｰﾀ数
			
			switch(COM0.SEND_DATA_COUNT){
				case 11:	COM0.SEND_DATA = COM0.NO311;				break;		// 311 状態番号
				case 12:	COM0.SEND_DATA = COM0.NO312;				break;		// 312 ｴﾗｰ番号
				case 13:	COM0.SEND_DATA = COM0.NO313;				break;		// 313 計測結果 現在値(REAL)	上位
				case 14:	COM0.SEND_DATA = COM0.NO314;				break;		// 314 計測結果 現在値(REAL)	下位
				case 15:	COM0.SEND_DATA = COM0.NO315;				break;		// 315 計測結果 最終値(d)		上位
				case 16:	COM0.SEND_DATA = COM0.NO316;				break;		// 316 計測結果 最終値(d)		下位
				case 17:	COM0.SEND_DATA = COM0.NO317;				break;		// 317 計測結果 最終値(D)		上位
				case 18:	COM0.SEND_DATA = COM0.NO318;				break;		// 318 計測結果 最終値(D)		下位
				case 19:	COM0.SEND_DATA = COM0.NO319;				break;		// 319 計測結果 最終値(ΔX)		上位
				case 20:	COM0.SEND_DATA = COM0.NO320;				break;		// 320 計測結果 最終値(ΔX)		下位
				case 21:	COM0.SEND_DATA = COM0.NO321;				break;		// 321 計測結果 最終値(振れ)	上位
				case 22:	COM0.SEND_DATA = COM0.NO322;				break;		// 322 計測結果 最終値(振れ)	下位
				case 23:	COM0.SEND_DATA = COM0.NO323;				break;		// 323 空き
				case 24:	COM0.SEND_DATA = COM0.NO324;				break;		// 324 空き
				case 25:	COM0.SEND_DATA = COM0.NO325;				break;		// 325 バッファ領域最小
				case 26:	COM0.SEND_DATA = COM0.NO326;				break;		// 326 バッファ領域最大
				case 27:	COM0.SEND_DATA = (SEQ.TOTAL_COUNT>>16);		break;		// 327 計測総数
				case 28:	COM0.SEND_DATA = SEQ.TOTAL_COUNT;			break;		// 328 計測総数
				case 29:	COM0.SEND_DATA = (SEQ.OK_COUNT>>16);		break;		// 329 OK数
				case 30:	COM0.SEND_DATA = SEQ.OK_COUNT;				break;		// 330 OK数
				case 31:	COM0.SEND_DATA = COM0.NO331;				break;		// 331 ｽｷｯﾌﾟ出力
				case 32:	COM0.SEND_DATA = COM0.NO332;				break;		// 332 LED11灯
				case 33:	COM0.SEND_DATA = COM0.NO333.WORD;			break;		// 333 
			}
			
			if(COM0.SEND_DATA_COUNT < 34){
//			if(COM0.SEND_DATA_COUNT < 33){
				COM0.SET_COUNT = 1;
				rs422_write_command_set2(COM0.SEND_DATA);
				COM0.SUB_STATUS--;
			}else{
				rs422_write_cr_set2();						// RS422書き込みCRｾｯﾄ
// chg 2016.07.27 K.Uemura start	G72701
				if(SEQ.ALL_DATA[999]==1){
					COM0.SUB_STATUS = 29;
				}else{
					COM0.SUB_STATUS = 39;
				}
//				COM0.SUB_STATUS = 29;
// chg 2016.07.27 K.Uemura end
			}
			break;
			
		// 書き込み
		case 29:
			SCI0.SCR.BIT.RIE = 0;							// RXIおよびERI割り込み要求を禁止
			SCI0.SCR.BIT.RE = 0;							// ｼﾘｱﾙ受信動作を禁止
		
			COM0.WR_CONT = 0;
			COM0.RE_CONT = 0;
			
			COM0.SUB_STATUS++;
			
			SCI0.SCR.BIT.TIE = 1;							// TIEを「1」
			SCI0.SCR.BIT.TE = 1;							// TEを「1」
			break;
		////////////////////////////////////////////////////////////////////////////////
		
		// ﾃﾞｰﾀの書き込み(デバッグ用：101 - 149)	////////////////////////////////////
		// ｱﾄﾞﾚｽをｾｯﾄ
		case 31:
			//COM0.SEND_DATA = 9001;							// 書き込み開始ｱﾄﾞﾚｽ
			COM0.SEND_DATA = 101;							// 書き込み開始ｱﾄﾞﾚｽ
			COM0.SET_COUNT = 1;
			rs422_write_address_set2(COM0.SEND_DATA);		// RS422書き込みｱﾄﾞﾚｽｾｯﾄ
			COM0.SUB_STATUS++;
			break;
			
		case 32:
			rs422_write_address_set2(COM0.SEND_DATA);		// RS422書き込みｱﾄﾞﾚｽｾｯﾄ
			break;
			
		// ﾃﾞｰﾀの書き込み(デバッグ用：101 - 149)
		case 33:
			COM0.SEND_DATA_COUNT = 1;						// 送信ﾃﾞｰﾀ数
			COM0.SEND_DATA = COM0.NO101;					// 101 左ｴｯｼﾞ最小位置
			COM0.SET_COUNT = 1;
			rs422_write_command_set2(COM0.SEND_DATA);
			COM0.SUB_STATUS++;
			break;
			
		case 34:
			rs422_write_command_set2(COM0.SEND_DATA);
			break;
			
		// ﾃﾞｰﾀの書き込み(デバッグ用：101 - 149)
		case 35:
			COM0.SEND_DATA_COUNT++;									// 送信ﾃﾞｰﾀ数
			
			switch(COM0.SEND_DATA_COUNT){
				case 2:		COM0.SEND_DATA = COM0.NO102;				break;		// 102 右ｴｯｼﾞ最大位置
				case 3:		COM0.SEND_DATA = COM0.NO103;				break;		// 103 左ｴｯｼﾞ傾斜
				case 4:		COM0.SEND_DATA = COM0.NO104;				break;		// 104 右ｴｯｼﾞ傾斜
				case 5:		COM0.SEND_DATA = COM0.NO105;				break;		// 105 ｴｯｼﾞ間の差
				case 6:		COM0.SEND_DATA = COM0.NO106;				break;		// 106 d最大			RESULT.TIR[1]*10	// 最大ｴｯｼﾞ[pix]	RESULT.SMALL_D_MAX[0] * 10	// d最大
				case 7:		COM0.SEND_DATA = COM0.NO107;				break;		// 107 左エッジ最小		RESULT.TIR[2]*10	// 比率乗算結果		RESULT.LARGE_D_MIN[0] * 10	// 左エッジ最小
				case 8:		COM0.SEND_DATA = COM0.NO108;				break;		// 108 右エッジ最大
				case 9:		COM0.SEND_DATA = COM0.NO109;				break;		// 109 左焦点最小
				case 10:	COM0.SEND_DATA = COM0.NO110;				break;		// 110 右焦点最小
				case 11:	COM0.SEND_DATA = COM0.NO111;				break;		// 111 エッジ最小
				case 12:	COM0.SEND_DATA = COM0.NO112;				break;		// 112 エッジ最大
				case 13:	COM0.SEND_DATA = COM0.NO113;				break;		// 113 左焦点最小
				case 14:	COM0.SEND_DATA = COM0.NO114;				break;		// 114 右焦点最小
				case 15:	COM0.SEND_DATA = COM0.NO115;				break;		// 115 左エッジ最大
				case 16:	COM0.SEND_DATA = COM0.NO116;				break;		// 116 右エッジ最大
				case 17:	COM0.SEND_DATA = COM0.NO117;				break;		// 117 左最大工具径(d)
				case 18:	COM0.SEND_DATA = COM0.NO118;				break;		// 118 右最大工具径(d)
				case 19:	COM0.SEND_DATA = COM0.NO119;				break;		// 119 工具中心左最大
				case 20:	COM0.SEND_DATA = COM0.NO120;				break;		// 120 工具中心右最大
				case 21:	COM0.SEND_DATA = COM0.NO121;				break;		// COM0.NO121 = RESULT.EDGE_LEFT_SCALE[SEQ.BUFFER_COUNT] * 10;
				case 22:	COM0.SEND_DATA = COM0.NO122;				break;		// COM0.NO122 = RESULT.EDGE_RIGHT_SCALE[SEQ.BUFFER_COUNT] * 10;
				case 23:	COM0.SEND_DATA = COM0.NO123;				break;		// 右ｴｯｼﾞ最大位置 - 左ｴｯｼﾞ最小位置
				case 24:	COM0.SEND_DATA = COM0.NO124;				break;		// COM0.NO122 - COM0.NO121		RESULT.EDGE_RIGHT_SCALE[SEQ.BUFFER_COUNT] * 10 - RESULT.EDGE_LEFT_SCALE[SEQ.BUFFER_COUNT] * 10
				case 25:	COM0.SEND_DATA = COM0.NO125;				break;		// 125 SEQ.TRIGGER_TIME_PERIOD
				case 26:	COM0.SEND_DATA = COM0.NO126;				break;		// 126 SEQ.TRIGGER_TIME_PERIOD

				case 27:	COM0.SEND_DATA = COM0.NO127;				break;		// 127 回転数[上位]
				case 28:	COM0.SEND_DATA = COM0.NO128;				break;		// 128 回転数[下位]
				case 29:	COM0.SEND_DATA = COM0.NO129;				break;		// 129 刃数
				case 30:	COM0.SEND_DATA = COM0.NO130;				break;		// 130 電池残量(%)
				case 31:	COM0.SEND_DATA = COM0.NO131;				break;		// 131 IN.FLAG.BIT.EXT_POWER
				case 32:	COM0.SEND_DATA = COM0.NO132;				break;		// 132 ｶｳﾝﾄ数(確認用)
				case 33:	COM0.SEND_DATA = COM0.NO133;				break;		// 133 平均値(仮)
				case 34:	COM0.SEND_DATA = COM0.NO134;				break;		// 134 最小値(仮)
				case 35:	COM0.SEND_DATA = COM0.NO135;				break;		// 135 最大値(仮)
				case 36:	COM0.SEND_DATA = COM0.NO136;				break;		// 136 最大ｴｯｼﾞ[pix]
				case 37:	COM0.SEND_DATA = COM0.NO137;				break;		// 137 比率乗算結果
				case 38:	COM0.SEND_DATA = COM0.NO138;				break;		// 138 FPGAﾊﾞｰｼﾞｮﾝ
				case 39:	COM0.SEND_DATA = COM0.NO139;				break;		// 139 RXﾊﾞｰｼﾞｮﾝ
				case 40:	COM0.SEND_DATA = COM0.NO140;				break;		// 
				case 41:	COM0.SEND_DATA = COM0.NO141;				break;		// 
				case 42:	COM0.SEND_DATA = COM0.NO142;				break;		// 
				case 43:	COM0.SEND_DATA = COM0.NO143;				break;		// 
				case 44:	COM0.SEND_DATA = COM0.NO144;				break;		// 
				case 45:	COM0.SEND_DATA = COM0.NO145;				break;		// 
				case 46:	COM0.SEND_DATA = COM0.NO146;				break;		// 
				case 47:	COM0.SEND_DATA = COM0.NO147;				break;		// 
				case 48:	COM0.SEND_DATA = COM0.NO148;				break;		// 
				case 49:	COM0.SEND_DATA = COM0.NO149;				break;		// 
			}
			
//			if(COM0.SEND_DATA_COUNT < 41){
			if(COM0.SEND_DATA_COUNT < 50){
				COM0.SET_COUNT = 1;
				rs422_write_command_set2(COM0.SEND_DATA);
				COM0.SUB_STATUS--;
			}else{
				rs422_write_cr_set2();								// RS422書き込みCRｾｯﾄ
				COM0.SUB_STATUS = 39;
			}
			break;
			
		// 書き込み
		case 39:
			SCI0.SCR.BIT.RIE = 0;									// RXIおよびERI割り込み要求を禁止
			SCI0.SCR.BIT.RE = 0;									// ｼﾘｱﾙ受信動作を禁止
		
			COM0.WR_CONT = 0;
			COM0.RE_CONT = 0;
			
			// この部分で以降の処理を行うか判断する
			// ﾃﾞｰﾀの書き込み(3000 - 3639)があるとき
			if(SEQ.BUFFER_NO_OLD != SEQ.BUFFER_NO_NEW){				// ﾊﾞｯﾌｧﾅﾝﾊﾞｰ旧とﾊﾞｯﾌｧﾅﾝﾊﾞｰ新が異なるとき
				COM0.SUB_STATUS = 40;
				
			// ﾃﾞｰﾀの書き込み(299)があるとき
			}else if(SEQ.FLAG.BIT.BUFFER_TRANSFER == 1){			// ﾊﾞｯﾌｧ転送ﾌﾗｸﾞ
				//SEQ.FLAG.BIT.BUFFER_TRANSFER = 0;					// ﾊﾞｯﾌｧ転送ﾌﾗｸﾞ
				COM0.SUB_STATUS = 50;
			
			// ﾃﾞｰﾀの書き込み(295 - 299)があるとき
			}else if(SEQ.FLAG2.BIT.BLACK_WHITE_COMPLETE == 1){		// BLACKﾁｪｯｸ・WHITEﾁｪｯｸ完了ﾌﾗｸﾞ
				SEQ.FLAG2.BIT.BLACK_WHITE_COMPLETE = 0;				// BLACKﾁｪｯｸ・WHITEﾁｪｯｸ完了ﾌﾗｸﾞ
				COM0.SUB_STATUS = 60;								// 起動処理結果表示
				
			// ﾊﾟﾗﾒｰﾀの読み出し(1001 - 2000)
			}else if(SEQ.FLAG3.BIT.PARA_READ == 1){					// ﾊﾟﾗﾒｰﾀ読み出しﾌﾗｸﾞ(505)
				SEQ.FLAG3.BIT.PARA_READ = 0;
				COM0.START_ADDRESS = 1001;							// 開始ｱﾄﾞﾚｽ
				COM0.SUB_STATUS = 101;
				SEQ.FLAG.BIT.MEMORY_CONTROL = 1;
				COM0.RECEIVE_DATA_COUNT = 1;						// 受信ﾃﾞｰﾀｶｳﾝﾄ
				
			// ﾊﾟﾗﾒｰﾀの読み出し(3000 - 5999)
			}else if(SEQ.FLAG3.BIT.PARA_READ == 2){					// ﾊﾟﾗﾒｰﾀ読み出しﾌﾗｸﾞ(601)
				SEQ.FLAG3.BIT.PARA_READ = 0;
				COM0.START_ADDRESS = 3000;							// 開始ｱﾄﾞﾚｽ
				COM0.SUB_STATUS = 121;
				SEQ.FLAG.BIT.MEMORY_CONTROL = 1;
				COM0.RECEIVE_DATA_COUNT = 1;						// 受信ﾃﾞｰﾀｶｳﾝﾄ
				
			// ﾊﾟﾗﾒｰﾀの書き込み(1001 - 2000)
			}else if(SEQ.FLAG3.BIT.PARA_WRITE == 1){				// ﾊﾟﾗﾒｰﾀ書き込みﾌﾗｸﾞ(506)
				SEQ.FLAG3.BIT.PARA_WRITE = 0;
				COM0.START_ADDRESS = 1001;							// 開始ｱﾄﾞﾚｽ
				COM0.SUB_STATUS = 141;
				SEQ.FLAG.BIT.MEMORY_CONTROL = 1;
				
			// ﾊﾟﾗﾒｰﾀの書き込み(98 - 99)
			}else if(SEQ.FLAG3.BIT.PARA_WRITE == 2){				// ﾊﾟﾗﾒｰﾀ書き込みﾌﾗｸﾞ(602)
				SEQ.FLAG3.BIT.PARA_WRITE = 0;
				COM0.START_ADDRESS = 3000;							// 開始ｱﾄﾞﾚｽ
				COM0.SUB_STATUS = 161;
				SEQ.FLAG.BIT.MEMORY_CONTROL = 1;
				
			}else{
				COM0.SUB_STATUS = 200;								// ﾎﾟｰﾘﾝｸﾞ待機
			}
			
			SCI0.SCR.BIT.TIE = 1;									// TIEを「1」
			SCI0.SCR.BIT.TE = 1;									// TEを「1」
			break;
		////////////////////////////////////////////////////////////////////////////////
		
		// ﾃﾞｰﾀの書き込み(3000 - 3639)最大640個	////////////////////////////////////////
		// ｱﾄﾞﾚｽをｾｯﾄ
		case 41:
			SEQ.BUFFER_NO_OLD++;
			if(SEQ.BUFFER_NO_OLD > BUFFER_NUMBER){					// ﾊﾞｯﾌｧﾅﾝﾊﾞｰ新がﾊﾞｯﾌｧ最大数を超えているとき
				SEQ.BUFFER_NO_OLD = 0;
			}
			
			COM0.SEND_DATA = 3000 + SEQ.BUFFER_NO_OLD;				// 書き込み開始ｱﾄﾞﾚｽ
			COM0.SET_COUNT = 1;
			rs422_write_address_set2(COM0.SEND_DATA);				// RS422書き込みｱﾄﾞﾚｽｾｯﾄ
			COM0.SUB_STATUS++;
			break;
			
		case 42:
			rs422_write_address_set2(COM0.SEND_DATA);				// RS422書き込みｱﾄﾞﾚｽｾｯﾄ
			break;
			
		// ﾃﾞｰﾀの書き込み(3000 - 3639)
		case 43:
			COM0.SEND_DATA_COUNT = SEQ.BUFFER_NO_OLD;				// 送信ﾃﾞｰﾀｶｳﾝﾄ
			COM0.SEND_DATA = COM0.NO3000[COM0.SEND_DATA_COUNT];		// 3000
			COM0.SET_COUNT = 1;
			rs422_write_command_set2(COM0.SEND_DATA);
			COM0.SUB_STATUS++;
			break;
			
		case 44:
			rs422_write_command_set2(COM0.SEND_DATA);
			break;
			
		// ﾃﾞｰﾀの書き込み(3000 - 3639)
		case 45:
			if(COM0.SEND_DATA_COUNT < SEQ.BUFFER_NO_NEW){
				COM0.SEND_DATA_COUNT++;									// 送信ﾃﾞｰﾀ数
				SEQ.BUFFER_NO_OLD = COM0.SEND_DATA_COUNT;
				COM0.SEND_DATA = COM0.NO3000[COM0.SEND_DATA_COUNT];		// 3000
				COM0.SET_COUNT = 1;
				rs422_write_command_set2(COM0.SEND_DATA);
				COM0.SUB_STATUS--;
				
			}else if(COM0.SEND_DATA_COUNT > SEQ.BUFFER_NO_NEW){
				if(COM0.SEND_DATA_COUNT < BUFFER_NUMBER){
					COM0.SEND_DATA_COUNT++;									// 送信ﾃﾞｰﾀ数
					SEQ.BUFFER_NO_OLD = COM0.SEND_DATA_COUNT;
					COM0.SEND_DATA = COM0.NO3000[COM0.SEND_DATA_COUNT];		// 3000
					COM0.SET_COUNT = 1;
					rs422_write_command_set2(COM0.SEND_DATA);
					COM0.SUB_STATUS--;
				}else{
					rs422_write_cr_set2();							// RS422書き込みCRｾｯﾄ
					COM0.SUB_STATUS = 49;
				}
			}else{
				rs422_write_cr_set2();								// RS422書き込みCRｾｯﾄ
				COM0.SUB_STATUS = 49;
			}
			break;
			
		// 書き込み
		case 49:
			SCI0.SCR.BIT.RIE = 0;									// RXIおよびERI割り込み要求を禁止
			SCI0.SCR.BIT.RE = 0;									// ｼﾘｱﾙ受信動作を禁止
		
			COM0.WR_CONT = 0;
			COM0.RE_CONT = 0;
			
			COM0.SUB_STATUS = 200;									// ﾎﾟｰﾘﾝｸﾞ待機
			
			SCI0.SCR.BIT.TIE = 1;									// TIEを「1」
			SCI0.SCR.BIT.TE = 1;									// TEを「1」
			break;
		////////////////////////////////////////////////////////////////////////////////
		
		// ﾃﾞｰﾀの書き込み(299)	////////////////////////////////////////////////////////
		// ｱﾄﾞﾚｽをｾｯﾄ
		case 51:
			COM0.SEND_DATA = 299;									// 書き込み開始ｱﾄﾞﾚｽ
			COM0.SET_COUNT = 1;
			rs422_write_address_set2(COM0.SEND_DATA);				// RS422書き込みｱﾄﾞﾚｽｾｯﾄ
			COM0.SUB_STATUS++;
			break;
			
		case 52:
			rs422_write_address_set2(COM0.SEND_DATA);				// RS422書き込みｱﾄﾞﾚｽｾｯﾄ
			break;
			
		// ﾃﾞｰﾀの書き込み(299)
		case 53:
			// ﾊﾞｯﾌｧ出力状態
			if(SEQ.BUFFER_OUTPUT == 1)		COM0.NO299 = 1;
			else							COM0.NO299 = 0;
			COM0.SEND_DATA = COM0.NO299;							// 299 ﾊﾞｯﾌｧ転送
			COM0.SET_COUNT = 1;
			rs422_write_command_set2(COM0.SEND_DATA);
			COM0.SUB_STATUS++;
			break;
			
		// ﾃﾞｰﾀの書き込み(299)
		case 54:
			rs422_write_command_set2(COM0.SEND_DATA);
			break;
			
		// ﾃﾞｰﾀの書き込み(299)
		case 55:
			rs422_write_cr_set2();									// RS422書き込みCRｾｯﾄ
			COM0.SUB_STATUS = 58;
			break;
			
		// ﾀｲﾑｱｳﾄｾｯﾄ
		case 58:
			ctl_uni_timer2((POLLING_CYCLE/10) + 100);				// 通信ﾀｲﾑｱｳﾄﾀｲﾏｰ ﾎﾟｰﾘﾝｸﾞ周期+1s
			COM0.SUB_STATUS++;
			break;
			
		// 書き込み
		case 59:
			SCI0.SCR.BIT.RIE = 0;									// RXIおよびERI割り込み要求を禁止
			SCI0.SCR.BIT.RE = 0;									// ｼﾘｱﾙ受信動作を禁止
		
			COM0.WR_CONT = 0;
			COM0.RE_CONT = 0;
			
			COM0.SUB_STATUS = 200;									// ﾎﾟｰﾘﾝｸﾞ待機
			
			SCI0.SCR.BIT.TIE = 1;									// TIEを「1」
			SCI0.SCR.BIT.TE = 1;									// TEを「1」
			break;
		////////////////////////////////////////////////////////////////////////////////
		
		// ﾃﾞｰﾀの書き込み(200 - 209)	////////////////////////////////////////////////
		// 起動処理結果表示
		// ｱﾄﾞﾚｽをｾｯﾄ
		case 61:
			COM0.SEND_DATA = 200;									// 書き込み開始ｱﾄﾞﾚｽ
			COM0.SET_COUNT = 1;
			rs422_write_address_set2(COM0.SEND_DATA);				// RS422書き込みｱﾄﾞﾚｽｾｯﾄ
			COM0.SUB_STATUS++;
			break;
			
		case 62:
			rs422_write_address_set2(COM0.SEND_DATA);				// RS422書き込みｱﾄﾞﾚｽｾｯﾄ
			break;
			
		// ﾃﾞｰﾀの書き込み(200 - 209)
		case 63:
			COM0.SEND_DATA_COUNT = 0;								// 送信ﾃﾞｰﾀｶｳﾝﾄ
			COM0.SEND_DATA = SEQ.X_BLACK_AVE;						// 200 ﾌﾞﾗｯｸ平均値(X)(BLACKﾁｪｯｸ時)
			
			COM0.SET_COUNT = 1;
			rs422_write_command_set2(COM0.SEND_DATA);
			COM0.SUB_STATUS++;
			break;
			
		case 64:
			rs422_write_command_set2(COM0.SEND_DATA);
			break;
			
		// ﾃﾞｰﾀの書き込み(200 - 209)
		case 65:
			COM0.SEND_DATA_COUNT++;									// 送信ﾃﾞｰﾀ数
			
			switch(COM0.SEND_DATA_COUNT){
				case 1:		// 201 ﾎﾜｲﾄ平均値(調光動作最終時)
					COM0.SEND_DATA = SEQ.X_WHITE_AVE;					// ﾎﾜｲﾄ平均値(X)
					break;
					
				case 2:		// 202 ﾎﾜｲﾄ最小値(WHITEﾁｪｯｸ時)
					COM0.SEND_DATA = SEQ.X_WHITE_MIN;					// ﾎﾜｲﾄ最小値(X)
					break;

				case 3:		// 203 ﾎﾜｲﾄ最大値(WHITEﾁｪｯｸ時)
// chg 2016.03.08 K.Uemura start	G30702
					COM0.SEND_DATA = (SEQ.X_LED_BRIGHTNESS_WHITE << 8) |SEQ.X_WHITE_MAX;	// LED輝度 ﾎﾜｲﾄ平均値(X) ／ ﾎﾜｲﾄ最大値(X)
//					COM0.SEND_DATA = SEQ.X_WHITE_MAX;					// ﾎﾜｲﾄ最大値(X)
// chg 2016.03.08 K.Uemura end
					break;
					
				case 4:		// 204 調光ﾚﾍﾞﾙ
					COM0.SEND_DATA = SEQ.X_LED_BRIGHTNESS;				// LED輝度(X)
					break;
				
				case 5:		// 205 ﾎﾜｲﾄ平均値(調光動作最終時)
					COM0.SEND_DATA = SEQ.Z_BLACK_AVE;					// ﾌﾞﾗｯｸ平均値(Z)
					break;
					
				case 6:		// 206 ﾎﾜｲﾄ平均値(調光動作最終時)
					COM0.SEND_DATA = SEQ.Z_WHITE_AVE;					// ﾎﾜｲﾄ平均値(Z)
					break;
					
				case 7:		// 207 ﾎﾜｲﾄ最小値(WHITEﾁｪｯｸ時)
					COM0.SEND_DATA = SEQ.Z_WHITE_MIN;					// ﾎﾜｲﾄ最小値(Z)
					break;

				case 8:		// 208 ﾎﾜｲﾄ最大値(WHITEﾁｪｯｸ時)
// chg 2016.03.08 K.Uemura start	G30702
					COM0.SEND_DATA = (SEQ.Z_LED_BRIGHTNESS_WHITE << 8) | SEQ.Z_WHITE_MAX;	// LED輝度 ﾎﾜｲﾄ平均値(Z) ／ ﾎﾜｲﾄ最大値(Z)
//					COM0.SEND_DATA = SEQ.Z_WHITE_MAX;					// ﾎﾜｲﾄ最大値(Z)
// chg 2016.03.08 K.Uemura end
					break;
					
				case 9:		// 209 調光ﾚﾍﾞﾙ
					COM0.SEND_DATA = SEQ.Z_LED_BRIGHTNESS;				// LED輝度(Z)
					break;
			}
			
			if(COM0.SEND_DATA_COUNT < 10){
				COM0.SET_COUNT = 1;
				rs422_write_command_set2(COM0.SEND_DATA);
				COM0.SUB_STATUS--;
			}else{
				rs422_write_cr_set2();								// RS422書き込みCRｾｯﾄ
				COM0.SUB_STATUS = 69;
			}
			break;
			
		// 書き込み
		case 69:
			SCI0.SCR.BIT.RIE = 0;									// RXIおよびERI割り込み要求を禁止
			SCI0.SCR.BIT.RE = 0;									// ｼﾘｱﾙ受信動作を禁止
		
			COM0.WR_CONT = 0;
			COM0.RE_CONT = 0;
			
			COM0.SUB_STATUS = 200;
			
			SCI0.SCR.BIT.TIE = 1;									// TIEを「1」
			SCI0.SCR.BIT.TE = 1;									// TEを「1」
			break;
		////////////////////////////////////////////////////////////////////////////////
		
		// ﾊﾟﾗﾒｰﾀの読み出し(1001 - 2000)	////////////////////////////////////////////
		// ｱﾄﾞﾚｽのｾｯﾄ
		case 101:
			SCI0.SCR.BIT.RIE = 0;									// RXIおよびERI割り込み要求を禁止
			SCI0.SCR.BIT.RE = 0;									// ｼﾘｱﾙ受信動作を禁止
			COM0.SET_COUNT = 1;
			COM0.SUB_STATUS++;
			break;
		
		// ｱﾄﾞﾚｽのｾｯﾄ
		case 102:
			// 1回のﾃﾞｰﾀ読み出し最大数は256個
			rs422_send_command2('R', COM0.START_ADDRESS, 250);		// 「読み出し」 開始ｱﾄﾞﾚｽ「1001」 ﾃﾞｰﾀ数「250」
			break;
			
		// ﾀｲﾑｱｳﾄｾｯﾄ
		case 103:
			ctl_uni_timer2((POLLING_CYCLE/10) + 100);				// 通信ﾀｲﾑｱｳﾄﾀｲﾏｰ ﾎﾟｰﾘﾝｸﾞ周期+1s
			COM0.SUB_STATUS++;
			break;
		
		// ｺﾏﾝﾄﾞ送信
		case 104:
			COM0.WR_CONT = 0;
			COM0.RE_CONT = 0;
			
			COM0.SUB_STATUS = 109;
			
			SCI0.SCR.BIT.TIE = 1;									// TIEを「1」
			SCI0.SCR.BIT.TE = 1;									// TEを「1」
			break;
		////////////////////////////////////////////////////////////////////////////////
		
		// (送信ﾃﾞｰﾀｴﾝﾌﾟﾃｨ割り込み)・(受信ﾃﾞｰﾀﾌﾙ割り込み)を通過しｽﾃｰﾀｽが「+2」進む
		
		// ﾊﾟﾗﾒｰﾀの格納(1001 - 2000)	////////////////////////////////////////////////
		// ｺﾏﾝﾄﾞ受信
		case 111:
			COM0.RECEIVE_COUNT = 1;									// 受信文字数
			rs422_receive_read();									// RS422受信ﾃﾞｰﾀﾘｰﾄﾞ関数
			COM0.SUB_STATUS++;
			break;
			
		// ｺﾏﾝﾄﾞ受信
		case 112:
			rs422_receive_read();									// RS422受信ﾃﾞｰﾀﾘｰﾄﾞ関数
			break;
			
		// ｺﾏﾝﾄﾞ受信
		case 113:
			COM0.MEM_BUF[COM0.RECEIVE_DATA_COUNT] = COM0.RECEIVE_DATA;		// 1001 - 2000
			
			if((COM0.RECEIVE_DATA_COUNT == 250)||(COM0.RECEIVE_DATA_COUNT == 500)||(COM0.RECEIVE_DATA_COUNT == 750)){
				COM0.START_ADDRESS += 250;
				COM0.SUB_STATUS = 101;
			}else if(COM0.RECEIVE_DATA_COUNT == 1000){
				COM0.SUB_STATUS++;
// add 2016.10.18 K.Uemura start	GA1801
				// DLPM確認
				if(verify_DLPM(&COM0.MEM_BUF[0], &i)){
					COM0.NO312 = (4000 + i);
					COM0.SUB_STATUS = 201;

					COM0.NO310.BIT.RDY = 1;			// READYのﾋﾞｯﾄをOFFにし状態番号をｾｯﾄする
					SEQ.TP_CONTROL_STATUS++;
				}
// add 2016.10.18 K.Uemura end
			}else{
				rs422_receive_read();								// RS422受信ﾃﾞｰﾀﾘｰﾄﾞ関数
				COM0.SUB_STATUS--;
			}
			
			COM0.RECEIVE_DATA_COUNT++;								// 受信ﾃﾞｰﾀｶｳﾝﾄ
			break;
		////////////////////////////////////////////////////////////////////////////////
			
		// ROMに書き込み(1001 - 2000)	////////////////////////////////////////////////
		// 
		case 114:
			I2C.WR_BUF[0] = MEM_INITIAL;
			for(i = 1; i<=1000; i++){
				I2C.WR_BUF[4*i-3]	= COM0.MEM_BUF[i] >> 24;
				I2C.WR_BUF[4*i-2]	= COM0.MEM_BUF[i] >> 16;
				I2C.WR_BUF[4*i-1]	= COM0.MEM_BUF[i] >> 8;
				I2C.WR_BUF[4*i]		= COM0.MEM_BUF[i];
			}
			I2C.WR_CONT = 0;
			
			if(RIIC0.ICCR2.BIT.BBSY == 0){
				RIIC0.ICCR2.BIT.ST = 1;								// ｽﾀｰﾄﾋﾞｯﾄ
				I2C.SUB_STATUS = 12;
				
				IR(RIIC0, ICEEI0) = 0;								// 割り込み要求ｸﾘｱ(通信ｴﾗｰ/ｲﾍﾞﾝﾄ発生)
				IEN(RIIC0, ICEEI0) = 1;								// 割り込み要求許可(通信ｴﾗｰ/ｲﾍﾞﾝﾄ発生)
			}
			
			ctl_uni_timer2((POLLING_CYCLE/10) + 100);				// 通信ﾀｲﾑｱｳﾄﾀｲﾏｰ ﾎﾟｰﾘﾝｸﾞ周期+1s
			
			COM0.SUB_STATUS = 201;
			break;
		////////////////////////////////////////////////////////////////////////////////
		
		// ﾊﾟﾗﾒｰﾀの読み出し(3000 - 5999)	////////////////////////////////////////////
		// ｱﾄﾞﾚｽのｾｯﾄ
		case 121:
			SCI0.SCR.BIT.RIE = 0;									// RXIおよびERI割り込み要求を禁止
			SCI0.SCR.BIT.RE = 0;									// ｼﾘｱﾙ受信動作を禁止
			COM0.SET_COUNT = 1;
			COM0.SUB_STATUS++;
			break;
		
		// ｱﾄﾞﾚｽのｾｯﾄ
		case 122:
			// 1回のﾃﾞｰﾀ読み出し最大数は256個
			rs422_send_command2('R', COM0.START_ADDRESS, 250);		// 「読み出し」 開始ｱﾄﾞﾚｽ「3000」 ﾃﾞｰﾀ数「250」
			break;
			
		// ﾀｲﾑｱｳﾄｾｯﾄ
		case 123:
			ctl_uni_timer2((POLLING_CYCLE/10) + 100);				// 通信ﾀｲﾑｱｳﾄﾀｲﾏｰ ﾎﾟｰﾘﾝｸﾞ周期+1s
			COM0.SUB_STATUS++;
			break;
		
		// ｺﾏﾝﾄﾞ送信
		case 124:
			COM0.WR_CONT = 0;
			COM0.RE_CONT = 0;
			
			COM0.SUB_STATUS = 129;
			
			SCI0.SCR.BIT.TIE = 1;									// TIEを「1」
			SCI0.SCR.BIT.TE = 1;									// TEを「1」
			break;
		////////////////////////////////////////////////////////////////////////////////
		
		// (送信ﾃﾞｰﾀｴﾝﾌﾟﾃｨ割り込み)・(受信ﾃﾞｰﾀﾌﾙ割り込み)を通過しｽﾃｰﾀｽが「+2」進む
		
		// ﾊﾟﾗﾒｰﾀの格納(3000 - 5999)	////////////////////////////////////////////////
		// ｺﾏﾝﾄﾞ受信
		case 131:
			COM0.RECEIVE_COUNT = 1;									// 受信文字数
			rs422_receive_read();									// RS422受信ﾃﾞｰﾀﾘｰﾄﾞ関数
			COM0.SUB_STATUS++;
			break;
			
		// ｺﾏﾝﾄﾞ受信
		case 132:
			rs422_receive_read();									// RS422受信ﾃﾞｰﾀﾘｰﾄﾞ関数
			break;
			
		// ｺﾏﾝﾄﾞ受信
		case 133:
			COM0.MEM_BUF[COM0.RECEIVE_DATA_COUNT] = COM0.RECEIVE_DATA;		// 1001 - 3000
			
			if((COM0.RECEIVE_DATA_COUNT == 250)||(COM0.RECEIVE_DATA_COUNT == 500)||(COM0.RECEIVE_DATA_COUNT == 750)||(COM0.RECEIVE_DATA_COUNT == 1000)
				//||(COM0.RECEIVE_DATA_COUNT == 1250)||(COM0.RECEIVE_DATA_COUNT == 1500)||(COM0.RECEIVE_DATA_COUNT == 1750)){
				||(COM0.RECEIVE_DATA_COUNT == 1250)||(COM0.RECEIVE_DATA_COUNT == 1500)||(COM0.RECEIVE_DATA_COUNT == 1750)||(COM0.RECEIVE_DATA_COUNT == 2000)
				||(COM0.RECEIVE_DATA_COUNT == 2250)||(COM0.RECEIVE_DATA_COUNT == 2500)||(COM0.RECEIVE_DATA_COUNT == 2750)){
				COM0.START_ADDRESS += 250;
				COM0.SUB_STATUS = 121;
			//}else if(COM0.RECEIVE_DATA_COUNT == 2000){
			}else if(COM0.RECEIVE_DATA_COUNT == 3000){
				COM0.SUB_STATUS++;
			}else{
				rs422_receive_read();						// RS422受信ﾃﾞｰﾀﾘｰﾄﾞ関数
				COM0.SUB_STATUS--;
			}
			
			COM0.RECEIVE_DATA_COUNT++;						// 受信ﾃﾞｰﾀｶｳﾝﾄ
			break;
		////////////////////////////////////////////////////////////////////////////////
		
		// ROMに書き込み(3000 - 5999)	////////////////////////////////////////////////
		// 
		case 134:
			I2C.WR_BUF[0] = MEM_INITIAL;
			//for(i = 1; i<=2000; i++){
			for(i = 1; i<=3000; i++){
				I2C.WR_BUF[2*i+4094]	= COM0.MEM_BUF[i] >> 8;
				I2C.WR_BUF[2*i+4095]	= COM0.MEM_BUF[i];
			}
			I2C.WR_CONT = 4096;
			I2C.FLAG.BIT.ADDRESS_CHANGE = 1;				// ｱﾄﾞﾚｽ変更ﾌﾗｸﾞ
			
			if(RIIC0.ICCR2.BIT.BBSY == 0){
				RIIC0.ICCR2.BIT.ST = 1;						// ｽﾀｰﾄﾋﾞｯﾄ
				I2C.SUB_STATUS = 12;
				
				IR(RIIC0, ICEEI0) = 0;						// 割り込み要求ｸﾘｱ(通信ｴﾗｰ/ｲﾍﾞﾝﾄ発生)
				IEN(RIIC0, ICEEI0) = 1;						// 割り込み要求許可(通信ｴﾗｰ/ｲﾍﾞﾝﾄ発生)
			}
			
			ctl_uni_timer2((POLLING_CYCLE/10) + 100);		// 通信ﾀｲﾑｱｳﾄﾀｲﾏｰ ﾎﾟｰﾘﾝｸﾞ周期+1s
			
			//COM0.SUB_STATUS = 201;
			COM0.SUB_STATUS++;
			break;
		////////////////////////////////////////////////////////////////////////////////
			
		// ROMに書き込み終了まで待機
		case 135:
			if(I2C.SUB_STATUS >= 40){
				COM0.SUB_STATUS++;
			}
			break;
			
		// ROMに書き込み(1001 - 2000)	////////////////////////////////////////////////
		// 
		//case 114:
		case 136:
			led_layout_x();							// LED割り付け(X)
			led_layout_z();							// LED割り付け(Z)
			led_layout_portable();					// LED割り付け(ﾎﾟｰﾀﾌﾞﾙ)
			
			I2C.WR_BUF[0] = MEM_INITIAL;
			for(i = 1; i<4000; i=i+2){
				I2C.WR_BUF[i] = I2C.RE_BUF[i];
				I2C.WR_BUF[i+1] = I2C.RE_BUF[i+1];
			}
			I2C.WR_CONT = 0;
			
			if(RIIC0.ICCR2.BIT.BBSY == 0){
				RIIC0.ICCR2.BIT.ST = 1;								// ｽﾀｰﾄﾋﾞｯﾄ
				I2C.SUB_STATUS = 12;
				
				IR(RIIC0, ICEEI0) = 0;								// 割り込み要求ｸﾘｱ(通信ｴﾗｰ/ｲﾍﾞﾝﾄ発生)
				IEN(RIIC0, ICEEI0) = 1;								// 割り込み要求許可(通信ｴﾗｰ/ｲﾍﾞﾝﾄ発生)
			}
			
			ctl_uni_timer2((POLLING_CYCLE/10) + 100);				// 通信ﾀｲﾑｱｳﾄﾀｲﾏｰ ﾎﾟｰﾘﾝｸﾞ周期+1s
			
			COM0.SUB_STATUS = 201;
			break;
		////////////////////////////////////////////////////////////////////////////////
		
		// ﾊﾟﾗﾒｰﾀの書き込み(1001 - 2000)	////////////////////////////////////////////
		// ｱﾄﾞﾚｽをｾｯﾄ
		case 141:
			COM0.SEND_DATA = COM0.START_ADDRESS;
			COM0.SET_COUNT = 1;
			rs422_write_address_set2(COM0.SEND_DATA);		// RS422書き込みｱﾄﾞﾚｽｾｯﾄ
			COM0.SUB_STATUS++;
			break;
				
		case 142:
			rs422_write_address_set2(COM0.SEND_DATA);		// RS422書き込みｱﾄﾞﾚｽｾｯﾄ
			break;
		
		// ﾊﾟﾗﾒｰﾀの書き込み(1001 - 2000)
		case 143:
			COM0.SEND_DATA_COUNT = 1;						// 送信ﾃﾞｰﾀ数
			COM0.SEND_DATA = SEQ.ALL_DATA[COM0.SEND_DATA_COUNT];
			COM0.SET_COUNT = 1;
			rs422_write_command_set2(COM0.SEND_DATA);
			COM0.SUB_STATUS++;
			break;
			
		case 144:
			rs422_write_command_set2(COM0.SEND_DATA);
			break;
			
		// ﾊﾟﾗﾒｰﾀの書き込み(1001 - 2000)
		case 145:
			COM0.SEND_DATA_COUNT++;							// 送信ﾃﾞｰﾀ数
			
			if(COM0.SEND_DATA_COUNT < 1001){
				COM0.SEND_DATA = SEQ.ALL_DATA[COM0.SEND_DATA_COUNT];
				COM0.SET_COUNT = 1;
				rs422_write_command_set2(COM0.SEND_DATA);
				COM0.SUB_STATUS--;
			}else{
				rs422_write_cr_set2();						// RS422書き込みCRｾｯﾄ
				COM0.SUB_STATUS = 148;
			}
			break;
			
		// ﾀｲﾑｱｳﾄｾｯﾄ
		case 148:
			ctl_uni_timer2((POLLING_CYCLE/10) + 100);		// 通信ﾀｲﾑｱｳﾄﾀｲﾏｰ ﾎﾟｰﾘﾝｸﾞ周期+1s
			COM0.SUB_STATUS++;
			break;
			
		// 書き込み
		case 149:
			SCI0.SCR.BIT.RIE = 0;							// RXIおよびERI割り込み要求を禁止
			SCI0.SCR.BIT.RE = 0;							// ｼﾘｱﾙ受信動作を禁止
			
			COM0.WR_CONT = 0;
			COM0.RE_CONT = 0;
			
			COM0.SUB_STATUS++;
			
			SCI0.SCR.BIT.TIE = 1;							// TIEを「1」
			SCI0.SCR.BIT.TE = 1;							// TEを「1」
			break;
			
		case 151:
			SEQ.FLAG.BIT.MEMORY_CONTROL = 0;
			COM0.SUB_STATUS = 201;
			
			if(COM0.NO300.BIT.EXE){
				COM0.NO310.BIT.RDY = 1;
				SEQ.TP_CONTROL_STATUS++;
			}
			break;
		////////////////////////////////////////////////////////////////////////////////
		
		// ﾊﾟﾗﾒｰﾀの書き込み(3000 - 5999)	////////////////////////////////////////////
		// ｱﾄﾞﾚｽをｾｯﾄ
		case 161:
			COM0.SEND_DATA = COM0.START_ADDRESS;
			COM0.SET_COUNT = 1;
			rs422_write_address_set2(COM0.SEND_DATA);		// RS422書き込みｱﾄﾞﾚｽｾｯﾄ
			COM0.SUB_STATUS++;
			break;
				
		case 162:
			rs422_write_address_set2(COM0.SEND_DATA);		// RS422書き込みｱﾄﾞﾚｽｾｯﾄ
			break;
		
		// ﾊﾟﾗﾒｰﾀの書き込み(3000 - 5999)
		case 163:
			COM0.SEND_DATA_COUNT = 0;						// 送信ﾃﾞｰﾀ数
			COM0.SEND_DATA = SEQ.TABLE_EDGE_LEFT[COM0.SEND_DATA_COUNT];
			COM0.SET_COUNT = 1;
			rs422_write_command_set2(COM0.SEND_DATA);
			COM0.SUB_STATUS++;
			break;
				
		case 164:
			rs422_write_command_set2(COM0.SEND_DATA);
			break;
			
		// ﾊﾟﾗﾒｰﾀの書き込み(3000 - 5999)
		case 165:
			COM0.SEND_DATA_COUNT++;							// 送信ﾃﾞｰﾀ数
			
			if(COM0.SEND_DATA_COUNT < 1000){
				COM0.SEND_DATA = SEQ.TABLE_EDGE_LEFT[COM0.SEND_DATA_COUNT];
				rs422_write_command_set2(COM0.SEND_DATA);
				COM0.SUB_STATUS--;
			}else{
				rs422_write_cr_set2();						// RS422書き込みCRｾｯﾄ
				COM0.SUB_STATUS = 168;
			}
			break;
			
		// ﾀｲﾑｱｳﾄｾｯﾄ
		case 168:
			ctl_uni_timer2((POLLING_CYCLE/10) + 100);		// 通信ﾀｲﾑｱｳﾄﾀｲﾏｰ ﾎﾟｰﾘﾝｸﾞ周期+1s
			COM0.SUB_STATUS++;
			break;
			
		// 書き込み
		case 169:
			SCI0.SCR.BIT.RIE = 0;							// RXIおよびERI割り込み要求を禁止
			SCI0.SCR.BIT.RE = 0;							// ｼﾘｱﾙ受信動作を禁止
			
			COM0.WR_CONT = 0;
			COM0.RE_CONT = 0;
			
			COM0.SUB_STATUS++;
			
			SCI0.SCR.BIT.TIE = 1;							// TIEを「1」
			SCI0.SCR.BIT.TE = 1;							// TEを「1」
			break;
			
			//
		// ｱﾄﾞﾚｽをｾｯﾄ
		case 171:
			COM0.SEND_DATA = COM0.START_ADDRESS + 1000;
			COM0.SET_COUNT = 1;
			rs422_write_address_set2(COM0.SEND_DATA);		// RS422書き込みｱﾄﾞﾚｽｾｯﾄ
			COM0.SUB_STATUS++;
			break;
				
		case 172:
			rs422_write_address_set2(COM0.SEND_DATA);		// RS422書き込みｱﾄﾞﾚｽｾｯﾄ
			break;
		
		// ﾊﾟﾗﾒｰﾀの書き込み(3000 - 5999)
		case 173:
			COM0.SEND_DATA_COUNT = 0;						// 送信ﾃﾞｰﾀ数
			COM0.SEND_DATA = SEQ.TABLE_EDGE_RIGHT[COM0.SEND_DATA_COUNT];
			COM0.SET_COUNT = 1;
			rs422_write_command_set2(COM0.SEND_DATA);
			COM0.SUB_STATUS++;
			break;
				
		case 174:
			rs422_write_command_set2(COM0.SEND_DATA);
			break;
			
		// ﾊﾟﾗﾒｰﾀの書き込み(3000 - 5999)
		case 175:
			COM0.SEND_DATA_COUNT++;							// 送信ﾃﾞｰﾀ数
			
			if(COM0.SEND_DATA_COUNT < 1000){
				COM0.SEND_DATA = SEQ.TABLE_EDGE_RIGHT[COM0.SEND_DATA_COUNT];
				rs422_write_command_set2(COM0.SEND_DATA);
				COM0.SUB_STATUS--;
			}else{
				rs422_write_cr_set2();						// RS422書き込みCRｾｯﾄ
				COM0.SUB_STATUS = 178;
			}
			break;
			
		// ﾀｲﾑｱｳﾄｾｯﾄ
		case 178:
			ctl_uni_timer2((POLLING_CYCLE/10) + 100);		// 通信ﾀｲﾑｱｳﾄﾀｲﾏｰ ﾎﾟｰﾘﾝｸﾞ周期+1s
			COM0.SUB_STATUS++;
			break;
			
		// 書き込み
		case 179:
			SCI0.SCR.BIT.RIE = 0;							// RXIおよびERI割り込み要求を禁止
			SCI0.SCR.BIT.RE = 0;							// ｼﾘｱﾙ受信動作を禁止
			
			COM0.WR_CONT = 0;
			COM0.RE_CONT = 0;
			
			COM0.SUB_STATUS++;
			
			SCI0.SCR.BIT.TIE = 1;							// TIEを「1」
			SCI0.SCR.BIT.TE = 1;							// TEを「1」
			break;
			//
			
			//
		// ｱﾄﾞﾚｽをｾｯﾄ
		case 181:
			COM0.SEND_DATA = COM0.START_ADDRESS + 2000;
			COM0.SET_COUNT = 1;
			rs422_write_address_set2(COM0.SEND_DATA);		// RS422書き込みｱﾄﾞﾚｽｾｯﾄ
			COM0.SUB_STATUS++;
			break;
				
		case 182:
			rs422_write_address_set2(COM0.SEND_DATA);		// RS422書き込みｱﾄﾞﾚｽｾｯﾄ
			break;
		
		// ﾊﾟﾗﾒｰﾀの書き込み(3000 - 5999)
		case 183:
			COM0.SEND_DATA_COUNT = 0;						// 送信ﾃﾞｰﾀ数
			COM0.SEND_DATA = SEQ.Z_TABLE_EDGE_RIGHT[COM0.SEND_DATA_COUNT];
			COM0.SET_COUNT = 1;
			rs422_write_command_set2(COM0.SEND_DATA);
			COM0.SUB_STATUS++;
			break;
				
		case 184:
			rs422_write_command_set2(COM0.SEND_DATA);
			break;
			
		// ﾊﾟﾗﾒｰﾀの書き込み(3000 - 5999)
		case 185:
			COM0.SEND_DATA_COUNT++;							// 送信ﾃﾞｰﾀ数
			
			if(COM0.SEND_DATA_COUNT < 1000){
				COM0.SEND_DATA = SEQ.Z_TABLE_EDGE_RIGHT[COM0.SEND_DATA_COUNT];
				rs422_write_command_set2(COM0.SEND_DATA);
				COM0.SUB_STATUS--;
			}else{
				rs422_write_cr_set2();						// RS422書き込みCRｾｯﾄ
				COM0.SUB_STATUS = 188;
			}
			break;
			
		// ﾀｲﾑｱｳﾄｾｯﾄ
		case 188:
			ctl_uni_timer2((POLLING_CYCLE/10) + 100);		// 通信ﾀｲﾑｱｳﾄﾀｲﾏｰ ﾎﾟｰﾘﾝｸﾞ周期+1s
			COM0.SUB_STATUS++;
			break;
			
		// 書き込み
		case 189:
			SCI0.SCR.BIT.RIE = 0;							// RXIおよびERI割り込み要求を禁止
			SCI0.SCR.BIT.RE = 0;							// ｼﾘｱﾙ受信動作を禁止
			
			COM0.WR_CONT = 0;
			COM0.RE_CONT = 0;
			
			COM0.SUB_STATUS++;
			
			SCI0.SCR.BIT.TIE = 1;							// TIEを「1」
			SCI0.SCR.BIT.TE = 1;							// TEを「1」
			break;
			//
			
		case 191:
			SEQ.FLAG.BIT.MEMORY_CONTROL = 0;
			COM0.SUB_STATUS = 201;
			
			if(COM0.NO300.BIT.EXE){
				COM0.NO310.BIT.RDY = 1;
				SEQ.TP_CONTROL_STATUS++;
			}
			break;
			
		////////////////////////////////////////////////////////////////////////////////
		
		// ﾎﾟｰﾘﾝｸﾞ待機
		case 201:
			SEQ.FLAG.BIT.BUFFER_TRANSFER = 0;					// ﾊﾞｯﾌｧ転送ﾌﾗｸﾞ
			break;
	}
}

//************************************************************/
//				終了条件ｾｯﾄ
//************************************************************/
void end_condition_set(void)
{
	_UWORD i,temp;
	
	SEQ.OK_COUNT_SET			= 0;			// OKｶｳﾝﾄ数ｾｯﾄ
	SEQ.END_TIMEOUT_PERIOD_SET	= 0;			// ﾀｲﾑｱｳﾄ時間ｾｯﾄ
	SEQ.NG_COUNT_SET			= 0;			// 連続NGｶｳﾝﾄ数ｾｯﾄ
	
	// ﾀｲﾑｱｳﾄ時間・OKｶｳﾝﾄ数・連続NGｶｳﾝﾄ数設定
	// ※ﾎﾟｰﾀﾌﾞﾙ版のときは終了条件を設定しない
	if(IN.FLAG.BIT.HARDWARE_TYPE == CNC_EDITION){
		// 終了条件無効がONのときは終了条件を設定しない
		if(COM0.NO300.BIT.ECD == 0){			// 終了条件無効がOFFのとき	ADD 150720
			for(i = 0; i<=2; i++){
				temp = 0;
				
				// 全処理共通のﾀｲﾑｱｳﾄ時間・OKｶｳﾝﾄ数・連続NGｶｳﾝﾄ数が設定されているとき
				if(SEQ.ALL_DATA[500+i*10] > 0){
// add 2016.03.08 K.Uemura start	G30801
					if((COM0.NO301 >= 10)&&(COM0.NO301 <= 63)){	
//					if((COM0.NO301 >= 10)&&(COM0.NO301 <= 61)){	
// add 2016.03.08 K.Uemura end
						temp = SEQ.ALL_DATA[500+i*10];
					}
					
				// 全処理共通のﾀｲﾑｱｳﾄ時間・OKｶｳﾝﾄ数・連続NGｶｳﾝﾄ数が設定されていないとき
				}else{
					// 工具径(d＞4 自動)のとき
					if(COM0.NO301 == 20){
						if(SEQ.FLAG2.BIT.AUTO_MODE == 1){				// 「1:工具径(d≦4)」
							temp = SEQ.ALL_DATA[501+i*10];				// 工具径(d≦4)
							
						}else if(SEQ.FLAG2.BIT.AUTO_MODE == 2){			// 「2:工具径(d＞4 左側)」
							temp = SEQ.ALL_DATA[502+i*10];				// 工具径(d＞4)
							
						}else if(SEQ.FLAG2.BIT.AUTO_MODE == 3){			// 「3:工具径(d＞4 右側)」
							temp = SEQ.ALL_DATA[502+i*10];				// 工具径(d＞4)
						}
						
					// 工具径(d≦4)のとき
					}else if(COM0.NO301 == 10){
						temp = SEQ.ALL_DATA[501+i*10];					// 工具径(d≦4)
						
					// 工具径(d＞4 左側)または工具径(d＞4 右側)のとき
					}else if((COM0.NO301 == 21)||(COM0.NO301 == 22)){
						temp = SEQ.ALL_DATA[502+i*10];					// 工具径(d＞4)
						
					// 振れ測定(X)または振れ測定(Z)のとき
					}else if((COM0.NO301 == 50)||(COM0.NO301 == 51)){
						temp = SEQ.ALL_DATA[503+i*10];					// 振れ測定
						
					// ﾌﾟﾛﾌｧｲﾙ(X)またはﾌﾟﾛﾌｧｲﾙ(Z)のとき
// chg 2016.03.08 K.Uemura start	G30801
					}else if((COM0.NO301 == 60)||(COM0.NO301 == 61)||(COM0.NO301 == 62)||(COM0.NO301 == 63)){
//					}else if((COM0.NO301 == 60)||(COM0.NO301 == 61)){
// chg 2016.03.08 K.Uemura end
						temp = SEQ.ALL_DATA[504+i*10];					// ﾌﾟﾛﾌｧｲﾙ
						
// chg 2015.08.31 K.Uemura start	工具長の終了条件無効
//					// 工具長(Z)のとき
//					}else if(COM0.NO301 == 30){
//						temp = SEQ.ALL_DATA[505+i*10];					// 工具長(Z)
//						
// chg 2015.08.31 K.Uemura end
					}
				}

// add 2015.10.07 K.Uemura start	OKカウント数自動演算
				if(temp == 0){
					temp = make_okcount();

// add 2016.12.15 K.Uemura start	GC0602	
					// 粗測定bitがONの時、OK数を10倍
					if(SEQ.FLAG6.BIT.ROUGH_SCAN){
						temp *= 10;
					}
// add 2016.12.15 K.Uemura end

				}
// add 2015.10.07 K.Uemura end

				if(i == 0)			SEQ.OK_COUNT_SET			= temp;		// OKｶｳﾝﾄ数ｾｯﾄ
				else if(i == 1)		SEQ.END_TIMEOUT_PERIOD_SET	= temp;		// ﾀｲﾑｱｳﾄ時間ｾｯﾄ
				else if(i == 2)		SEQ.NG_COUNT_SET			= temp;		// 連続NGｶｳﾝﾄ数ｾｯﾄ
			}

// chg 2017.01.06 K.Uemura start	H10601	
			// 1回の測定時間を0.5msと仮定

			// 回転数が「0」の場合、waitしない
			SEQ.WAIT_COUNT_SET = 0;
			if(SEQ.SPINDLE_SPEED != 0){
				// 最低3周回転しなければ測定終了させない
				temp = 30;
//				// n周指定が無い場合は、2周とする
//				temp = (SEQ.ALL_DATA[997] == 0)? 20 : SEQ.ALL_DATA[997];
				// 1周に要する時間[msec] × タクト(0.5ms) × 周数(10倍値)
				SEQ.WAIT_COUNT_SET = (double)(60.0 / SEQ.SPINDLE_SPEED * 1000) * (1/0.5) * (double)(temp/10.0);
			}
// chg 2017.01.06 K.Uemura end
		}
// add 2015.08.31 K.Uemura start	ﾌﾟﾛｱﾌｨﾙ動作時は終了条件を必ず有効にする
		// ﾌﾟﾛﾌｧｲﾙ(X)またはﾌﾟﾛﾌｧｲﾙ(Z)のとき
// chg 2016.03.08 K.Uemura start	G30801
		if((COM0.NO301 == 60)||(COM0.NO301 == 61)||(COM0.NO301 == 62)||(COM0.NO301 == 63)){
//		if((COM0.NO301 == 60)||(COM0.NO301 == 61)){
// chg 2016.03.08 K.Uemura end
			temp = SEQ.ALL_DATA[504];					// OKｶｳﾝﾄ数
			temp = SEQ.ALL_DATA[514];					// ﾀｲﾑｱｳﾄ時間
			temp = SEQ.ALL_DATA[524];					// 連続NGｶｳﾝﾄ数
		}
// add 2015.08.31 K.Uemura end
	}
	
// add 2016.01.21 K.Uemura start	G12102
	SEQ.FLAG4.BIT.OKCOUNT_FLAG = 0;
// add 2016.01.21 K.Uemura end
	SEQ.OK_COUNT			= 0;				// OKｶｳﾝﾄ数
	SEQ.END_TIMEOUT_PERIOD	= 0;				// ﾀｲﾑｱｳﾄ時間
	SEQ.NG_COUNT			= 0;				// 連続NGｶｳﾝﾄ数
	
	SEQ.COM_START_PERIOD	= 0;				// 計測開始時間(ms)
	SEQ.COM_TIMEOUT_PERIOD	= 0;				// ﾀｲﾑｱｳﾄ時間(通信設定)(ms)
}

//************************************************************/
//				終了条件計算
// chg 2017.02.08 K.Uemura	H20801	回転数「0」の場合は、1000回転の条件を適用
//************************************************************/
_UWORD make_okcount(void)
{
	_UWORD temp = 0;
	double modlus;

	switch(COM0.NO301){
		case 10:	// 工具径(d≦3)
		case 20:	// 工具径(自動)
		case 21:	// 工具径(d＞3 左側)
		case 22:	// 工具径(d＞3 右側)
		case 60:	// ﾌﾟﾛﾌｧｲﾙ(X)
		case 61:	// ﾌﾟﾛﾌｧｲﾙ(Z)
// add 2016.03.08 K.Uemura start	G30801
		case 62:	// ﾌﾟﾛﾌｧｲﾙ(X 左側)
		case 63:	// ﾌﾟﾛﾌｧｲﾙ(Z 右側)
// add 2016.03.08 K.Uemura end
			//工具半径の判別
			if(SEQ.RADIUS < 50){
				// Φ1.00(半径：0.5)より小さい
				temp = 1000;

				if(SEQ.SPINDLE_SPEED == 0){
				}else if(SEQ.SPINDLE_SPEED < 100){
					// 100より小さい
// chg 2016.01.21 K.Uemura start	G12101
					modlus = (double)(2.0) / SEQ.SPINDLE_SPEED * 60 / 0.44;
					temp = modlus * 1000;
//					modlus = 2 / SEQ.SPINDLE_SPEED * 60 / 0.44;
// chg 2016.01.21 K.Uemura end
				}else if(SEQ.SPINDLE_SPEED < 1000){
					// 100以上1000より小さい
					modlus = 1.918;
					temp = 2727 - ((SEQ.SPINDLE_SPEED - 100) * modlus);
				}
			}else if(SEQ.RADIUS < 100){
				// Φ2.00(半径：1.0)より小さい
				temp = 500;

				if(SEQ.SPINDLE_SPEED == 0){
				}else if(SEQ.SPINDLE_SPEED < 100){
					// 100より小さい
					temp = 10000 - (SEQ.SPINDLE_SPEED * 80);
				}else if(SEQ.SPINDLE_SPEED < 1000){
					// 100以上1000より小さい
					modlus = 1.666;
					temp = 2000 - ((SEQ.SPINDLE_SPEED - 100) * modlus);
				}
			}else if(SEQ.RADIUS < 150){
				// Φ3.00以下(半径：1.5)より小さい
				temp = 200;

				if(SEQ.SPINDLE_SPEED == 0){
				}else if(SEQ.SPINDLE_SPEED < 100){
					// 100より小さい
					temp = 5000 - (SEQ.SPINDLE_SPEED * 40);
				}else if(SEQ.SPINDLE_SPEED < 1000){
					// 100以上1000より小さい
					modlus = 0.888;
					temp = 1000 - ((SEQ.SPINDLE_SPEED - 100) * modlus);
				}
			}else if(SEQ.RADIUS < 600){
				// Φ12.00(半径：6.0)より小さい
				temp = 50;

				if(SEQ.SPINDLE_SPEED == 0){
				}else if(SEQ.SPINDLE_SPEED < 100){
					// 100より小さい
					temp = 2000 - (SEQ.SPINDLE_SPEED * 10);
				}else if(SEQ.SPINDLE_SPEED < 1000){
					// 100以上1000より小さい
					modlus = 1.055;
					temp = 1000 - ((SEQ.SPINDLE_SPEED - 100) * modlus);
				}
			}else{
				// Φ12(半径：6.0)以上
				temp = 20;

				if(SEQ.SPINDLE_SPEED == 0){
				}else if(SEQ.SPINDLE_SPEED < 100){
					// 100より小さい
					temp = 300 - (SEQ.SPINDLE_SPEED * 2);
				}else if(SEQ.SPINDLE_SPEED < 1000){
					// 100以上1000より小さい
					modlus = 0.088;
					temp = 100 - ((SEQ.SPINDLE_SPEED - 100) * modlus);
				}
			}
			break;
	}

	return( temp );
}

// add 2016.10.18 K.Uemura start	GA1801
/***** HEADER START ************************************************************
	[Function]		verify_DLPM
	[Summary]		DLPMの確認
	[Description]	FROM保存パラメータの確認
					非公開パラメータは、para_iniと比較し設定値に問題がないか確認
	[Caution]		DLPM004～009は、個体差があるため比較しない
	[Arguments]		I	unsigned short		*DLPM先頭アドレス
					O	unsigned short		DLPM相違index
	[Return Code]	0	正常終了
					1	パラメータ異常
					
	[File Name]		rs422_sci0.c
	[History]		2016/10/18	K.Uemura	新規作成
****** HEADER END *************************************************************/
_UWORD verify_DLPM( unsigned short *arry_DLPM, unsigned short *arry_DLPM_idx )
{
	unsigned short	idx, idx_DLPM;

	*arry_DLPM_idx = 0;

	//動作見直し（2016.11.28）
	//設定値変更の際、デフォルト値との相違でエラーとなるため比較条件が厳しすぎる
	//比較処理しないように変更。
	return(0);
	
	if(*(arry_DLPM + 998) == 0){

#if	0
		// DLPM001 ～ DLPM999までの設定値確認
		for(idx = 1 ; idx <= 999 ; idx++ ){
			*arry_DLPM_idx += *(arry_DLPM + idx);

			if( *(arry_DLPM + idx) != 0 ){
				break;
			}
		}

		// パラメータ総和が「0」のため、不適切
		if(*arry_DLPM_idx == 0){
			return( 1 );
		}
#else
		/************************** 
			prameter.hと同一で無ければNG
			※DLPM設定変更での動作確認を行う場合は、DLPM998を1に設定する
		 **************************/
		for(idx = 1 ; idx <= 210 ; idx++ ){

			// インデックス変換
			idx_DLPM = convert_DLPM( idx );

			// 比較を行うDLPM No
			switch(idx + idx_DLPM){
				case 1:
				case 2:
				case 3:
				case 10:
				case 11:
				case 12:
				case 13:
				case 14:
				case 15:
				case 16:
				case 20:
				case 21:
				case 22:
				case 23:
				case 24:
				case 25:
				case 26:
				case 30:
				case 31:
				case 32:
				case 33:
				case 34:
				case 35:
				case 36:
				case 40:
				case 41:
				case 42:
				case 43:
				case 44:
				case 45:
				case 46:
				case 50:
				case 51:
				case 52:
				case 53:
				case 54:
				case 55:
				case 60:
				case 61:
				case 62:
				case 63:
				case 64:
				case 65:
				case 66:
				case 67:
				case 68:
				case 69:
				case 70:
				case 71:
				case 72:
				case 73:
				case 80:
				case 81:
				case 82:
				case 83:
				case 84:
				case 85:
				case 86:
				case 87:
				case 88:
				case 89:
				case 90:
				case 91:
				case 92:
				case 93:
				case 100:
				case 101:
				case 102:
				case 103:
				case 104:
				case 105:
				case 106:
				case 107:
				case 108:
					break;

				// 他パラメータは比較しない
				default:
					continue;
					break;
			}

			if(*(arry_DLPM + idx + idx_DLPM) != para_ini[idx]){

				// デフォルトと相違
				*arry_DLPM_idx = idx;
				return( 1 );
			}
		}
#endif
	}

	return( 0 );
}
// add 2016.10.18 K.Uemura end
