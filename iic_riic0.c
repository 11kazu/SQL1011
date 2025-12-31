/******************************************************************************
* File Name	: iic_riic0.c
******************************************************************************/
#include "iodefine.h"
#include "typedefine.h"
#include "usercopy.h"
#include "user_define.h"
#include "parameter.h"

void riic0_init(void);
void i2c_master(void);
void i2c_reset(void);
void i2c_init(void);
void i2c_eei(void);
void i2c_rxi(void);
void i2c_tei(void);

void i2c_init_para(void);			// IIC初期ﾊﾟﾗﾒｰﾀ
// add 2016.10.18 K.Uemura start	GA1801
unsigned short convert_DLPM( unsigned short );
// add 2016.10.18 K.Uemura end

void led_layout_x(void);			// LED割り付け(X)
void led_layout_z(void);			// LED割り付け(Z)
void led_layout_portable(void);		// LED割り付け(ﾎﾟｰﾀﾌﾞﾙ)

/********************************************************/
/*		IIC初期設定(RIIC0)								*/
/********************************************************/
void riic0_init(void)
{
	// ﾃﾞｰﾀﾃﾞｨﾚｸｼｮﾝﾚｼﾞｽﾀ(DDR)	0:入力	1:出力	X:未使用(1)	-:予約ﾋﾞｯﾄ(0)
	PORT1.DDR.BYTE		= 0x70;			// 0X1X00--	// P17入力、P15出力、P12・13通信(I2C)、その他無し
	// 入力ﾊﾞｯﾌｧｺﾝﾄﾛｰﾙﾚｼﾞｽﾀ(ICR)
	PORT1.ICR.BYTE		= 0x0C;			// 00001100	// P12・13通信(I2C)
	
	MSTP(RIIC0) = 0;					// RIIC0ﾓｼﾞｭｰﾙｽﾄｯﾌﾟ解除
	
	// SCI初期設定
	// 割り込み要求を禁止する
	IEN(RIIC0, ICTEI0) = 0;
	IEN(RIIC0, ICEEI0) = 0;
	IEN(RIIC0, ICTXI0) = 0;
	IEN(RIIC0, ICRXI0) = 0;

	RIIC0.ICCR1.BIT.ICE = 0;			// SCL,SDA端子非駆動状態
	
	RIIC0.ICCR1.BIT.IICRST = 1;			// RIICﾘｾｯﾄ
	
	RIIC0.ICCR1.BIT.IICRST = 0;			// RIICﾘｾｯﾄ解除
	
	RIIC0.SARL0.BYTE = 0xA0;			// ｽﾚｰﾌﾞｱﾄﾞﾚｽﾚｼﾞｽﾀL(1010000 0)
	RIIC0.SARU0.BYTE = 0x00;			// ｽﾚｰﾌﾞｱﾄﾞﾚｽﾚｼﾞｽﾀU(0000000 0)
	
	RIIC0.ICSER.BYTE = 0x01;			// I2Cﾊﾞｽｽﾃｰﾀｽｲﾈｰﾌﾞﾙﾚｼﾞｽﾀ
	
	// 転送速度	= 1 / {[ (ICBRH+1) ＋ (ICBRL+1) ] / IICφ ＋ SCLnﾗｲﾝ立ち上がり時間[tr] ＋SCLnﾗｲﾝ立ち下がり時間[tf] }
	// 			= 1 / {[(7+1)+(16+1)]/(48000000/2)+0.0000003+0.0000003}
	// 			= 609.14kbps
	RIIC0.ICMR1.BIT.CKS = 1;			// 内部基準ｸﾛｯｸ選択ﾋﾞｯﾄ(24MHz/2ｸﾛｯｸ)
	RIIC0.ICBRL.BYTE = 0xF0;			// I2Cﾊﾞｽﾋﾞｯﾄﾚｰﾄﾛｰﾚﾍﾞﾙﾚｼﾞｽﾀ(16)
	RIIC0.ICBRH.BYTE = 0xE7;			// I2Cﾊﾞｽﾋﾞｯﾄﾚｰﾄﾊｲﾚﾍﾞﾙﾚｼﾞｽﾀ( 7)
	
	RIIC0.ICIER.BYTE = 0xEC;			// I2Cﾊﾞｽｲﾝﾀﾗﾌﾟﾄｲﾈｰﾌﾞﾙﾚｼﾞｽﾀ
	
	RIIC0.ICMR3.BIT.ACKWP = 1;			// ACKBTﾋﾞｯﾄへの書き込み許可
	
	RIIC0.ICCR1.BIT.ICE = 1;			// 内部ﾘｾｯﾄ
	
	// 割り込み優先ﾚﾍﾞﾙ設定ﾋﾞｯﾄ「1」にする
	IPR(RIIC0, ICTEI0) = 1;
	IPR(RIIC0, ICEEI0) = 1;
	IPR(RIIC0, ICTXI0) = 1;
	IPR(RIIC0, ICRXI0) = 1;
	
	// 割り込み要求を許可する
	IEN(RIIC0, ICRXI0) = 1;				// 割り込み要求許可(受信ﾃﾞｰﾀﾌﾙ)
}

//************************************************************/
//				ﾒｲﾝｼｰｹﾝｽ関数					
//************************************************************/
void i2c_master(void)
{
	switch(I2C.MASTER_STATUS){
		case RESET_MODE:
			i2c_reset();
			break;
		case INIT_START_MODE:
			i2c_init();
			break;
		case DRV_MODE:
			break;
		case IDLE_MODE:
			break;
		case READ_MODE:
			break;
		case WRITE_MODE:
			break;
		default:
			I2C.MASTER_STATUS = RESET_MODE;
	}
}

//************************************************************/
//				IICﾘｾｯﾄ関数
//************************************************************/
// ﾘｾｯﾄを行うためにｽﾀｰﾄを9回送信する(汎用出力ﾎﾟｰﾄとして扱う)
void i2c_reset(void)
{
	switch(I2C.SUB_STATUS){
		case 1:
			// SCLを「L」 SDAを「H」
			SCL_OUT = 1, SDA_OUT = 0;
			I2C.RESET_COUNT = 0;					// ｿﾌﾄｳｪｱﾘｾｯﾄｶｳﾝﾄ
			I2C.SUB_STATUS++;
			break;
			
		// 9回繰り返す
		case 2:
			// SCLを「H」
			SCL_OUT = 0, SDA_OUT = 0;
			I2C.SUB_STATUS++;
			break;
			
		case 3:
			// SDAを「L」
			SCL_OUT = 0, SDA_OUT = 1;
			I2C.SUB_STATUS++;
			break;
			
		case 4:
			// SCLを「L」
			SCL_OUT = 1, SDA_OUT = 1;
			I2C.SUB_STATUS++;
			break;
			
		case 5:
			// SDAを「H」
			SCL_OUT = 1, SDA_OUT = 0;
			I2C.RESET_COUNT++;						// ｿﾌﾄｳｪｱﾘｾｯﾄｶｳﾝﾄ
			if(I2C.RESET_COUNT < 9){
				I2C.SUB_STATUS = 2;
			}else{
				I2C.SUB_STATUS++;
			}
			break;
			
		case 6:
			SCL_OUT = 0, SDA_OUT = 0;
			riic0_init();							// IIC初期設定
			I2C.MASTER_STATUS = INIT_START_MODE;
			I2C.SUB_STATUS = 1;
			break;
	}
}

//************************************************************/
//				初期化関数
//************************************************************/
void i2c_init(void)
{
	switch(I2C.SUB_STATUS){
		// ﾃﾞｰﾀの読み出し(ｽﾀｰﾄﾋﾞｯﾄ)
		case 1:
			if(RIIC0.ICSR2.BIT.STOP == 1){
				RIIC0.ICSR2.BIT.NACKF = 0;
				RIIC0.ICSR2.BIT.STOP = 0;
			}
			
			if(RIIC0.ICCR2.BIT.BBSY == 0){
				RIIC0.ICCR2.BIT.ST = 1;				// ｽﾀｰﾄﾋﾞｯﾄ
				I2C.SUB_STATUS++;
				
				IR(RIIC0, ICEEI0) = 0;				// 割り込み要求ｸﾘｱ(通信ｴﾗｰ/ｲﾍﾞﾝﾄ発生)
				IEN(RIIC0, ICEEI0) = 1;				// 割り込み要求許可(通信ｴﾗｰ/ｲﾍﾞﾝﾄ発生)
			}
			break;
		
		// ﾃﾞｰﾀの書き込み(初期値 0 - 127)
		case 8:
			if(I2C.RE_BUF[0] != MEM_INITIAL){		// 初期値が書き込まれていないとき
				if(RIIC0.ICSR2.BIT.STOP == 1){
					RIIC0.ICSR2.BIT.NACKF = 0;
					RIIC0.ICSR2.BIT.STOP = 0;
				}
				i2c_init_para();		// IIC初期ﾊﾟﾗﾒｰﾀ
				
				
				I2C.WR_CONT = 0;
				
				if(RIIC0.ICCR2.BIT.BBSY == 0){
					RIIC0.ICCR2.BIT.ST = 1;			// ｽﾀｰﾄﾋﾞｯﾄ
					I2C.SUB_STATUS = 12;
					
					IR(RIIC0, ICEEI0) = 0;			// 割り込み要求ｸﾘｱ(通信ｴﾗｰ/ｲﾍﾞﾝﾄ発生)
					IEN(RIIC0, ICEEI0) = 1;			// 割り込み要求許可(通信ｴﾗｰ/ｲﾍﾞﾝﾄ発生)
				}
			}else{									// 初期値が書き込まれているとき
				if(RIIC0.ICSR2.BIT.STOP == 1){
					RIIC0.ICSR2.BIT.NACKF = 0;
					RIIC0.ICSR2.BIT.STOP = 0;
				}
				SEQ.FLAG.BIT.MEMORY_CONTROL = 0;
				I2C.SUB_STATUS = 39;
			}
			break;
			
		// 書き込み後待機(EEPROMが最大5msｺﾏﾝﾄﾞを受け付けないため)
		case 17:
			I2C.SUB_STATUS++;
			break;
			
		// ﾃﾞｰﾀの書き込み(初期値 128以降)
		case 18:
			if(RIIC0.ICSR2.BIT.STOP == 1){
				RIIC0.ICSR2.BIT.NACKF = 0;
				RIIC0.ICSR2.BIT.STOP = 0;
			}
			
			if(RIIC0.ICCR2.BIT.BBSY == 0){
				RIIC0.ICCR2.BIT.ST = 1;			// ｽﾀｰﾄﾋﾞｯﾄ
				I2C.SUB_STATUS = 22;
				
				IR(RIIC0, ICEEI0) = 0;			// 割り込み要求ｸﾘｱ(通信ｴﾗｰ/ｲﾍﾞﾝﾄ発生)
				IEN(RIIC0, ICEEI0) = 1;			// 割り込み要求許可(通信ｴﾗｰ/ｲﾍﾞﾝﾄ発生)
			}
			break;
			
		// 書き込み後待機(EEPROMが最大5msｺﾏﾝﾄﾞを受け付けないため)
		case 27:
			I2C.SUB_STATUS++;
			break;
			
		// ﾃﾞｰﾀの再読み出し(ｽﾀｰﾄﾋﾞｯﾄ)
		case 28:
			if(RIIC0.ICSR2.BIT.STOP == 1){
				RIIC0.ICSR2.BIT.NACKF = 0;
				RIIC0.ICSR2.BIT.STOP = 0;
			}
			
			I2C.FLAG.BIT.ADDRESS_CHANGE = 0;		// ｱﾄﾞﾚｽ変更ﾌﾗｸﾞ
			
			if(RIIC0.ICCR2.BIT.BBSY == 0){
				RIIC0.ICCR2.BIT.ST = 1;				// ｽﾀｰﾄﾋﾞｯﾄ
				I2C.SUB_STATUS = 32;

				IR(RIIC0, ICEEI0) = 0;				// 割り込み要求ｸﾘｱ(通信ｴﾗｰ/ｲﾍﾞﾝﾄ発生)
				IEN(RIIC0, ICEEI0) = 1;				// 割り込み要求許可(通信ｴﾗｰ/ｲﾍﾞﾝﾄ発生)
			}
			break;
			
		case 38:
			if(RIIC0.ICSR2.BIT.STOP == 1){
				RIIC0.ICSR2.BIT.NACKF = 0;
				RIIC0.ICSR2.BIT.STOP = 0;
			}
			
			if(COM0.SUB_STATUS != 135){
				SEQ.FLAG.BIT.MEMORY_CONTROL = 0;
			}
			
			if(COM0.NO300.BIT.EXE){// 実行
				if(COM0.SUB_STATUS != 135){
					COM0.NO310.BIT.RDY = 1;			// READYのﾋﾞｯﾄをOFFにし状態番号をｾｯﾄする
					SEQ.TP_CONTROL_STATUS++;
				}
			}
			
			I2C.SUB_STATUS++;
			break;
			
		case 39:
			//COM0.MASTER_STATUS = COM0_INIT_START_MODE;
			if(COM0.SUB_STATUS != 135){
				COM0.SUB_STATUS = 1;
			}
			I2C.SUB_STATUS++;
			break;
			
		// ﾃﾞｰﾀの書き込み(2ﾃﾞｰﾀ分)
		case 48:
			if(RIIC0.ICSR2.BIT.STOP == 1){
				RIIC0.ICSR2.BIT.NACKF = 0;
				RIIC0.ICSR2.BIT.STOP = 0;
			}
			
			if(RIIC0.ICCR2.BIT.BBSY == 0){
				RIIC0.ICCR2.BIT.ST = 1;			// ｽﾀｰﾄﾋﾞｯﾄ
				I2C.SUB_STATUS = 52;
				
				IR(RIIC0, ICEEI0) = 0;			// 割り込み要求ｸﾘｱ(通信ｴﾗｰ/ｲﾍﾞﾝﾄ発生)
				IEN(RIIC0, ICEEI0) = 1;			// 割り込み要求許可(通信ｴﾗｰ/ｲﾍﾞﾝﾄ発生)
			}
			break;
	}
}

/********************************************************/
/*		IICｲﾍﾞﾝﾄ発生(EEI)
/********************************************************/
void i2c_eei(void)
{
	switch(I2C.SUB_STATUS){
		// ﾃﾞｰﾀの読み出し(ｽﾚｰﾌﾞｱﾄﾞﾚｽ+書き込み)
		case 2:
			while(RIIC0.ICSR2.BIT.NACKF != 0){}
			while(RIIC0.ICSR2.BIT.TDRE != 1){}
			
			RIIC0.ICDRT = MEM_DATA_WR;				// ｽﾚｰﾌﾞｱﾄﾞﾚｽ+書き込み
			
			I2C.SUB_STATUS++;
			
			break;
		
		// ﾃﾞｰﾀの読み出し(ｱﾄﾞﾚｽ上位)
		case 3:
			while(RIIC0.ICMR3.BIT.ACKBR != 0){}
			while(RIIC0.ICSR2.BIT.NACKF != 0){}
			while(RIIC0.ICSR2.BIT.TDRE != 1){}
				
			RIIC0.ICDRT = 0x00;						// ｱﾄﾞﾚｽ
			I2C.SUB_STATUS++;
			break;
			
		// ﾃﾞｰﾀの読み出し(ｱﾄﾞﾚｽ下位)
		case 4:
			while(RIIC0.ICMR3.BIT.ACKBR != 0){}
			while(RIIC0.ICSR2.BIT.NACKF != 0){}
			while(RIIC0.ICSR2.BIT.TDRE != 1){}
				
			RIIC0.ICDRT = 0x00;						// ｱﾄﾞﾚｽ
			
			if(RIIC0.ICSR2.BIT.START == 1){
				RIIC0.ICSR2.BIT.START = 0;
			}
			
			IEN(RIIC0, ICEEI0) = 0;					// 割り込み要求禁止(通信ｴﾗｰ/ｲﾍﾞﾝﾄ発生)
			IR(RIIC0, ICTEI0) = 0;					// 割り込み要求ｸﾘｱ(送信終了)
			
			I2C.SUB_STATUS++;
			IEN(RIIC0, ICTEI0) = 1;					// 割り込み要求許可(送信終了)
			break;
			
		// ﾃﾞｰﾀの読み出し(ｽﾚｰﾌﾞｱﾄﾞﾚｽ+読み出し)
		case 6:
			while(RIIC0.ICMR3.BIT.ACKBR != 0){}
			while(RIIC0.ICSR2.BIT.NACKF != 0){}
			while(RIIC0.ICSR2.BIT.TDRE != 1){}
			
			I2C.RE_CONT = 0;
			
			IR(RIIC0, ICRXI0) = 0;					// 割り込み要求ｸﾘｱ(受信ﾃﾞｰﾀﾌﾙ)
			
			RIIC0.ICDRT = MEM_DATA_RE;				// ｽﾚｰﾌﾞｱﾄﾞﾚｽ+書き込み
				
			IEN(RIIC0, ICEEI0) = 0;					// 割り込み要求禁止(通信ｴﾗｰ/ｲﾍﾞﾝﾄ発生)
				
			I2C.SUB_STATUS++;
				
			while(RIIC0.ICSR2.BIT.RDRF != 1){}
			while(RIIC0.ICSR2.BIT.NACKF != 0){}
			I2C.DUMMY = RIIC0.ICDRR;				// ﾀﾞﾐｰﾘｰﾄﾞ
			IEN(RIIC0, ICRXI0) = 1;					// 割り込み要求許可(受信ﾃﾞｰﾀﾌﾙ)
			break;
			
		// ﾃﾞｰﾀの書き込み(ｽﾚｰﾌﾞｱﾄﾞﾚｽ+書き込み) 0 ～ 127
		case 12:
			while(RIIC0.ICSR2.BIT.NACKF != 0){}
			while(RIIC0.ICSR2.BIT.TDRE != 1){}
			
			RIIC0.ICDRT = MEM_DATA_WR;				// ｽﾚｰﾌﾞｱﾄﾞﾚｽ+書き込み
			
			I2C.SUB_STATUS++;
			
			break;
			
		// ﾃﾞｰﾀの書き込み(ｱﾄﾞﾚｽ上位)
		case 13:
			while(RIIC0.ICMR3.BIT.ACKBR != 0){}
			while(RIIC0.ICSR2.BIT.NACKF != 0){}
			while(RIIC0.ICSR2.BIT.TDRE != 1){}
			
			RIIC0.ICDRT = I2C.WR_CONT >> 8;			// 上位ｱﾄﾞﾚｽ
			
			I2C.SUB_STATUS++;
			
			break;
			
		// ﾃﾞｰﾀの書き込み(ｱﾄﾞﾚｽ下位)
		case 14:
			while(RIIC0.ICMR3.BIT.ACKBR != 0){}
			while(RIIC0.ICSR2.BIT.NACKF != 0){}
			while(RIIC0.ICSR2.BIT.TDRE != 1){}
			
			RIIC0.ICDRT = I2C.WR_CONT;				// 下位ｱﾄﾞﾚｽ
			
			I2C.LAST_ADDRESS = I2C.WR_CONT + 127;	// 最終ｱﾄﾞﾚｽのﾃﾞｰﾀ
			
			I2C.SUB_STATUS++;
			
			break;
			
		// ﾃﾞｰﾀの書き込み(ﾃﾞｰﾀ+ｽﾄｯﾌﾟﾋﾞｯﾄ)
		case 15:
			while(RIIC0.ICSR2.BIT.NACKF != 0){}
			while(RIIC0.ICSR2.BIT.TDRE != 1){}
			
			RIIC0.ICDRT = I2C.WR_BUF[I2C.WR_CONT];	// 書き込み
			I2C.WR_CONT++;
			
			if(I2C.WR_CONT == (I2C.LAST_ADDRESS + 1)){
				while(RIIC0.ICSR2.BIT.TEND != 1){}
				
				IEN(RIIC0, ICEEI0) = 0;				// 割り込み要求禁止(通信ｴﾗｰ/ｲﾍﾞﾝﾄ発生)
			
				if(RIIC0.ICSR2.BIT.STOP == 1){
					RIIC0.ICSR2.BIT.STOP = 0;
				}
				RIIC0.ICCR2.BIT.SP = 1;
				
				while(RIIC0.ICSR2.BIT.STOP != 1){}
				
				RIIC0.ICSR2.BIT.NACKF = 0;
				RIIC0.ICSR2.BIT.STOP = 0;
				
				I2C.SUB_STATUS = 17;
			}
			break;
			
		// ﾃﾞｰﾀの書き込み(ｽﾚｰﾌﾞｱﾄﾞﾚｽ+書き込み) 128以降
		case 22:
			while(RIIC0.ICSR2.BIT.NACKF != 0){}
			while(RIIC0.ICSR2.BIT.TDRE != 1){}
			
			RIIC0.ICDRT = MEM_DATA_WR;				// ｽﾚｰﾌﾞｱﾄﾞﾚｽ+書き込み
			
			I2C.SUB_STATUS++;
			
			break;
			
		// ﾃﾞｰﾀの書き込み(ｱﾄﾞﾚｽ上位)
		case 23:
			while(RIIC0.ICMR3.BIT.ACKBR != 0){}
			while(RIIC0.ICSR2.BIT.NACKF != 0){}
			while(RIIC0.ICSR2.BIT.TDRE != 1){}
			
			RIIC0.ICDRT = I2C.WR_CONT >> 8;			// 上位ｱﾄﾞﾚｽ
			
			I2C.SUB_STATUS++;
			
			break;
			
		// ﾃﾞｰﾀの書き込み(ｱﾄﾞﾚｽ下位)
		case 24:
			while(RIIC0.ICMR3.BIT.ACKBR != 0){}
			while(RIIC0.ICSR2.BIT.NACKF != 0){}
			while(RIIC0.ICSR2.BIT.TDRE != 1){}
			
			RIIC0.ICDRT = I2C.WR_CONT;				// 下位ｱﾄﾞﾚｽ
			
			I2C.LAST_ADDRESS = I2C.WR_CONT + 127;	// 最終ｱﾄﾞﾚｽのﾃﾞｰﾀ
			
			if(I2C.FLAG.BIT.ADDRESS_CHANGE == 1){	// ｱﾄﾞﾚｽ変更ﾌﾗｸﾞ
				// 最終ｱﾄﾞﾚｽが「10095」以上のとき、最終ｱﾄﾞﾚｽを「10095」にする
				if(I2C.LAST_ADDRESS >= 10095)	I2C.LAST_ADDRESS = 10095;
			}else{
				// 最終ｱﾄﾞﾚｽが「4000」以上のとき、最終ｱﾄﾞﾚｽを「4000」にする
				if(I2C.LAST_ADDRESS >= 4000)	I2C.LAST_ADDRESS = 4000;
			}
			
			I2C.SUB_STATUS++;
			
			break;
			
		// ﾃﾞｰﾀの書き込み(ﾃﾞｰﾀ+ｽﾄｯﾌﾟﾋﾞｯﾄ)
		case 25:
			while(RIIC0.ICSR2.BIT.NACKF != 0){}
			while(RIIC0.ICSR2.BIT.TDRE != 1){}
			
			RIIC0.ICDRT = I2C.WR_BUF[I2C.WR_CONT];	// 書き込み
			I2C.WR_CONT++;
			
			if(I2C.WR_CONT == (I2C.LAST_ADDRESS + 1)){
				while(RIIC0.ICSR2.BIT.TEND != 1){}
				
				IEN(RIIC0, ICEEI0) = 0;				// 割り込み要求禁止(通信ｴﾗｰ/ｲﾍﾞﾝﾄ発生)
			
				if(RIIC0.ICSR2.BIT.STOP == 1){
					RIIC0.ICSR2.BIT.STOP = 0;
				}
				RIIC0.ICCR2.BIT.SP = 1;
				
				while(RIIC0.ICSR2.BIT.STOP != 1){}
				
				RIIC0.ICSR2.BIT.NACKF = 0;
				RIIC0.ICSR2.BIT.STOP = 0;
				
				if(I2C.FLAG.BIT.ADDRESS_CHANGE == 1){		// ｱﾄﾞﾚｽ変更ﾌﾗｸﾞ
					// 最終ｱﾄﾞﾚｽが「10095」以上のとき
					if(I2C.LAST_ADDRESS >= 10095){
						I2C.SUB_STATUS = 27;
					}else{
						I2C.SUB_STATUS = 17;
					}
				}else{
					// 最終ｱﾄﾞﾚｽが「4000」以上のとき
					if(I2C.LAST_ADDRESS >= 4000){
						I2C.SUB_STATUS = 27;
					}else{
						I2C.SUB_STATUS = 17;
					}
				}
			}
			break;
			
		// ﾃﾞｰﾀの再読み出し(ｽﾚｰﾌﾞｱﾄﾞﾚｽ+書き込み)
		case 32:
			while(RIIC0.ICSR2.BIT.NACKF != 0){}
			while(RIIC0.ICSR2.BIT.TDRE != 1){}
			
			RIIC0.ICDRT = MEM_DATA_WR;				// ｽﾚｰﾌﾞｱﾄﾞﾚｽ+書き込み
			
			I2C.SUB_STATUS++;
			
			break;
			
		// ﾃﾞｰﾀの再読み出し(ｱﾄﾞﾚｽ上位)
		case 33:
			while(RIIC0.ICMR3.BIT.ACKBR != 0){}
			while(RIIC0.ICSR2.BIT.NACKF != 0){}
			while(RIIC0.ICSR2.BIT.TDRE != 1){}
				
			RIIC0.ICDRT = 0x00;						// ｱﾄﾞﾚｽ
			I2C.SUB_STATUS++;
			break;
			
		// ﾃﾞｰﾀの再読み出し(ｱﾄﾞﾚｽ下位)
		case 34:
			while(RIIC0.ICMR3.BIT.ACKBR != 0){}
			while(RIIC0.ICSR2.BIT.NACKF != 0){}
			while(RIIC0.ICSR2.BIT.TDRE != 1){}
				
			RIIC0.ICDRT = 0x00;						// ｱﾄﾞﾚｽ
			
			if(RIIC0.ICSR2.BIT.START == 1){
				RIIC0.ICSR2.BIT.START = 0;
			}
			
			IEN(RIIC0, ICEEI0) = 0;					// 割り込み要求禁止(通信ｴﾗｰ/ｲﾍﾞﾝﾄ発生)
			IR(RIIC0, ICTEI0) = 0;					// 割り込み要求ｸﾘｱ(送信終了)
			
			I2C.SUB_STATUS++;
			IEN(RIIC0, ICTEI0) = 1;					// 割り込み要求許可(送信終了)
			break;
			
		// ﾃﾞｰﾀの再読み出し(ｽﾚｰﾌﾞｱﾄﾞﾚｽ+読み出し)
		case 36:
			while(RIIC0.ICMR3.BIT.ACKBR != 0){}
			while(RIIC0.ICSR2.BIT.NACKF != 0){}
			while(RIIC0.ICSR2.BIT.TDRE != 1){}
			
			I2C.RE_CONT = 0;
			
			IR(RIIC0, ICRXI0) = 0;					// 割り込み要求ｸﾘｱ(受信ﾃﾞｰﾀﾌﾙ)
			
			RIIC0.ICDRT = MEM_DATA_RE;				// ｽﾚｰﾌﾞｱﾄﾞﾚｽ+書き込み
				
			IEN(RIIC0, ICEEI0) = 0;					// 割り込み要求禁止(通信ｴﾗｰ/ｲﾍﾞﾝﾄ発生)
				
			I2C.SUB_STATUS++;
				
			while(RIIC0.ICSR2.BIT.RDRF != 1){}
			while(RIIC0.ICSR2.BIT.NACKF != 0){}
			I2C.DUMMY = RIIC0.ICDRR;				// ﾀﾞﾐｰﾘｰﾄﾞ
			IEN(RIIC0, ICRXI0) = 1;					// 割り込み要求許可(受信ﾃﾞｰﾀﾌﾙ)
			break;
			
			
		// ﾃﾞｰﾀの書き込み(ｽﾚｰﾌﾞｱﾄﾞﾚｽ+書き込み) 0 ～ 1
		case 52:
			while(RIIC0.ICSR2.BIT.NACKF != 0){}
			while(RIIC0.ICSR2.BIT.TDRE != 1){}
			
			RIIC0.ICDRT = MEM_DATA_WR;				// ｽﾚｰﾌﾞｱﾄﾞﾚｽ+書き込み
			
			I2C.SUB_STATUS++;
			
			break;
			
		// ﾃﾞｰﾀの書き込み(ｱﾄﾞﾚｽ上位)
		case 53:
			while(RIIC0.ICMR3.BIT.ACKBR != 0){}
			while(RIIC0.ICSR2.BIT.NACKF != 0){}
			while(RIIC0.ICSR2.BIT.TDRE != 1){}
			
			RIIC0.ICDRT = I2C.WR_CONT >> 8;			// 上位ｱﾄﾞﾚｽ
			
			I2C.SUB_STATUS++;
			
			break;
			
		// ﾃﾞｰﾀの書き込み(ｱﾄﾞﾚｽ下位)
		case 54:
			while(RIIC0.ICMR3.BIT.ACKBR != 0){}
			while(RIIC0.ICSR2.BIT.NACKF != 0){}
			while(RIIC0.ICSR2.BIT.TDRE != 1){}
			
			RIIC0.ICDRT = I2C.WR_CONT;				// 下位ｱﾄﾞﾚｽ
			
			I2C.LAST_ADDRESS = I2C.WR_CONT + 1;		// 最終ｱﾄﾞﾚｽのﾃﾞｰﾀ
			
			I2C.SUB_STATUS++;
			
			break;
			
		// ﾃﾞｰﾀの書き込み(ﾃﾞｰﾀ+ｽﾄｯﾌﾟﾋﾞｯﾄ)
		case 55:
			while(RIIC0.ICSR2.BIT.NACKF != 0){}
			while(RIIC0.ICSR2.BIT.TDRE != 1){}
			
			RIIC0.ICDRT = I2C.WR_BUF[I2C.WR_CONT];	// 書き込み
			I2C.WR_CONT++;
			
			if(I2C.WR_CONT == (I2C.LAST_ADDRESS + 1)){
				while(RIIC0.ICSR2.BIT.TEND != 1){}
				
				IEN(RIIC0, ICEEI0) = 0;				// 割り込み要求禁止(通信ｴﾗｰ/ｲﾍﾞﾝﾄ発生)
			
				if(RIIC0.ICSR2.BIT.STOP == 1){
					RIIC0.ICSR2.BIT.STOP = 0;
				}
				RIIC0.ICCR2.BIT.SP = 1;
				
				while(RIIC0.ICSR2.BIT.STOP != 1){}
				
				RIIC0.ICSR2.BIT.NACKF = 0;
				RIIC0.ICSR2.BIT.STOP = 0;
				
				I2C.SUB_STATUS = 57;
			}
			break;
			
	}
}

/********************************************************/
/*		IIC受信ﾃﾞｰﾀﾌﾙ(RXI)
/********************************************************/
// EEPROMの全てのﾃﾞｰﾀを読み出す
// I2C.SUB_STATUS	「7」「37」
void i2c_rxi(void)
{
	_UINT i, j;
	
	// ﾃﾞｰﾀの読み出し(ﾃﾞｰﾀ受信)
	_UWORD last_data;
	last_data = 10095;				// 最終ﾃﾞｰﾀ
	
	while(RIIC0.ICSR2.BIT.RDRF != 1){}
	
	if(I2C.RE_CONT == (last_data - 2)){
		RIIC0.ICMR3.BIT.WAIT = 1;
	}else if(I2C.RE_CONT == (last_data - 1)){
		RIIC0.ICMR3.BIT.ACKBT = 1;
	}else if(I2C.RE_CONT == last_data){
		
		IEN(RIIC0, ICRXI0) = 0;		// 割り込み要求禁止(受信ﾃﾞｰﾀﾌﾙ)
		
		if(RIIC0.ICSR2.BIT.STOP == 1){
			RIIC0.ICSR2.BIT.STOP = 0;
		}
		RIIC0.ICCR2.BIT.SP = 1;		// ｽﾄｯﾌﾟｺﾝﾃﾞｨｼｮﾝ発行要求
	}
	
	if(I2C.RE_CONT <= last_data){
		I2C.RE_BUF[I2C.RE_CONT] = RIIC0.ICDRR;
		I2C.RE_CONT++;
	}
	
	if(I2C.RE_CONT == (last_data + 1)){
		RIIC0.ICMR3.BIT.ACKBT = 1;
		RIIC0.ICMR3.BIT.WAIT = 0;
		
		while(RIIC0.ICSR2.BIT.STOP != 1){}
		
		RIIC0.ICMR3.BIT.RDRFS = 0;
		RIIC0.ICMR3.BIT.ACKBT = 0;
		RIIC0.ICSR2.BIT.NACKF = 0;
		RIIC0.ICSR2.BIT.STOP = 0;
		
		// 読み出し用ﾊﾞｯﾌｧから全ﾒﾓﾘﾃﾞｰﾀを格納(ﾊﾟﾗﾒｰﾀﾅﾝﾊﾞｰ)
		j = 0;
		
		// ﾊﾟﾗﾒｰﾀ
		for(i = 1; i<=1000; i++){
			SEQ.ALL_DATA[i]	= (I2C.RE_BUF[4*(i+j)-3] << 24) + (I2C.RE_BUF[4*(i+j)-2] << 16)
								+ (I2C.RE_BUF[4*(i+j)-1] << 8) + I2C.RE_BUF[4*(i+j)];
		}
		
		// 換算ﾃｰﾌﾞﾙ
		for(i = 1; i<=100; i++){
			// 4096から
			SEQ.TABLE_EDGE_LEFT[i-1]	= (I2C.RE_BUF[4094+2*i] << 8) + I2C.RE_BUF[4095+2*i];
			SEQ.TABLE_EDGE_RIGHT[i-1]	= (I2C.RE_BUF[6094+2*i] << 8) + I2C.RE_BUF[6095+2*i];
			
			SEQ.Z_TABLE_EDGE_RIGHT[i-1]	= (I2C.RE_BUF[8094+2*i] << 8) + I2C.RE_BUF[8095+2*i];
		}
		
		// ﾛｸﾞ周期(周期ﾄﾘｶﾞ時間)
		SEQ.TRIGGER_TIME_PERIOD = (60000 / PARAM_SPINDLE_SPEED) / PARAM_FLUTES;
		
		// 分割数・補正間隔より距離のﾃｰﾌﾞﾙを作成
		if(IN.FLAG.BIT.HARDWARE_TYPE == CNC_EDITION){		// 計測ﾀｲﾌﾟがCNC版のとき
			SEQ.TABLE_EDGE_DISTANCE[0] = 0;					// 距離(実寸換算ﾃｰﾌﾞﾙ)
			if(X_DIVISION_NUMBER > 1){						// 無限ﾙｰﾌﾟ回避のため
				for(i = 1; i<=X_DIVISION_NUMBER-1; i++){
					SEQ.TABLE_EDGE_DISTANCE[i] = SEQ.TABLE_EDGE_DISTANCE[i-1] + X_CORRECTION_INTERVAL;			// 距離(実寸換算ﾃｰﾌﾞﾙ)
					if(i >= 99)		break;					// 配列数のｵｰﾊﾞｰ防止
				}
			}
			
			SEQ.Z_TABLE_EDGE_DISTANCE[0] = 0;				// 距離(実寸換算ﾃｰﾌﾞﾙ)
			if(Z_DIVISION_NUMBER > 1){						// 無限ﾙｰﾌﾟ回避のため
				for(i = 1; i<=Z_DIVISION_NUMBER-1; i++){
					SEQ.Z_TABLE_EDGE_DISTANCE[i] = SEQ.Z_TABLE_EDGE_DISTANCE[i-1] + Z_CORRECTION_INTERVAL;		// 距離(実寸換算ﾃｰﾌﾞﾙ)
					if(i >= 99)		break;					// 配列数のｵｰﾊﾞｰ防止
				}
			}
			
		}else{												// 計測ﾀｲﾌﾟがﾎﾟｰﾀﾌﾞﾙ版のとき
			SEQ.TABLE_EDGE_DISTANCE[0] = 0;					// 距離(実寸換算ﾃｰﾌﾞﾙ)
			if(PORTABLE_DIVISION_NUMBER > 1){				// 無限ﾙｰﾌﾟ回避のため
				for(i = 1; i<=PORTABLE_DIVISION_NUMBER-1; i++){
					SEQ.TABLE_EDGE_DISTANCE[i] = SEQ.TABLE_EDGE_DISTANCE[i-1] + PORTABLE_CORRECTION_INTERVAL;			// 距離(実寸換算ﾃｰﾌﾞﾙ)
					if(i >= 99)		break;					// 配列数のｵｰﾊﾞｰ防止
				}
			}
		}
		
		// EEPROMの値を一時ﾃﾞｰﾀにｺﾋﾟｰする
		SEQ.SPINDLE_SPEED		= PARAM_SPINDLE_SPEED;		// 回転数
		SEQ.FLUTES				= PARAM_FLUTES;				// 刃数
		SEQ.START_DELAY_TIME	= PARAM_START_DELAY_TIME;	// 計測開始遅延時間
		
		SEQ.FLAG6.BIT.HDI_UPDATE = 1;						// HDI更新ﾌﾗｸﾞ	ADD 170204
		I2C.SUB_STATUS++;
	}
}

/********************************************************/
/*		IIC送信終了(TEI)
/********************************************************/
// I2C.SUB_STATUS	「5」「35」
void i2c_tei(void)
{
	// ﾃﾞｰﾀの読み出し(ﾘｽﾀｰﾄﾋﾞｯﾄ)
	RIIC0.ICCR2.BIT.RS = 1;		// ﾘｽﾀｰﾄｺﾝﾃﾞｨｼｮﾝ発行要求
	
	while(RIIC0.ICSR2.BIT.TEND != 1){}
	
	IEN(RIIC0, ICTEI0) = 0;		// 割り込み要求禁止(送信終了)
	IR(RIIC0, ICEEI0) = 0;		// 割り込み要求ｸﾘｱ(通信ｴﾗｰ/ｲﾍﾞﾝﾄ発生)
	
	SEQ.FLAG6.BIT.HDI_UPDATE = 1;	// HDI更新ﾌﾗｸﾞ	ADD 170204
	
	I2C.SUB_STATUS++;
	IEN(RIIC0, ICEEI0) = 1;		// 割り込み要求許可(通信ｴﾗｰ/ｲﾍﾞﾝﾄ発生)
}

/********************************************************/
/*		IIC初期ﾊﾟﾗﾒｰﾀ
/********************************************************/
void i2c_init_para(void)
{
	_SDWORD i, j;
	_SWORD temp;
	signed long loop_number = 4000;
	
	I2C.WR_BUF[0] = MEM_INITIAL;
	
	// 書き込み用ﾊﾞｯﾌｧをﾘｾｯﾄする
	for(i = 1; i<loop_number; i=i+2){
		I2C.WR_BUF[i] = 0;
		I2C.WR_BUF[i+1] = 0;
	}
	
	// 初期ﾃﾞｰﾀを書き込み用ﾊﾞｯﾌｧに格納する
	// 160622更新(G62202)
	for(i = 1; i<=210; i++){
// chg 2016.10.18 K.Uemura start	GA1801
#if	1
		// インデックス変換
		j = convert_DLPM( i );
#else
		// 設定
		if(i <= 16)			j = 0;			// 露光時間			  1 -  16
		//汚れ率
											// X
											// Z
		//初期チェック
											// X
		else if(i <= 23)	j = 3;			// Z				 20 -  26
		else if(i <= 30)	j = 6;			// NORMAL			 30 -  36
		else if(i <= 37)	j = 9;			// ECO				 40 -  46
		// 計測設定
		else if(i <= 43)	j = 12;			// 共通				 50 -  55
		else if(i <= 57)	j = 16;			// X				 60 -  73
		else if(i <= 71)	j = 22;			// Z				 80 -  93
		// 換算TBL
		else if(i <= 80)	j = 28;			// X				100 - 102
											// Z				103 - 105
											// ﾎﾟｰﾀﾌﾞﾙ			106 - 108
		// 通信設定
		else if(i <= 87)	j = 69;			// 通信設定			150 - 156
		else if(i <= 89)	j = 72;			// 通信設定			160 - 161
		// SKIP
		else if(i <= 93)	j = 110;		// 出力設定			200 - 203
		else if(i <= 105)	j = 116;		// ORIGIN-X			210 - 221
		else if(i <= 117)	j = 124;		// ORIGIN-Z			230 - 241
		else if(i <= 129)	j = 132;		// 焦点合わせ-X		250 - 261
		else if(i <= 135)	j = 140;		// 焦点合わせ-Z		270 - 275
		// 表示/出力
		else if(i <= 139)	j = 165;		// 表示				301 - 304
		else if(i <= 144)	j = 170;		// 伸び				310 - 312
											// 焦点合わせ		313
											// 中心位置設定		314
		// LED点灯範囲
		else if(i <= 162)	j = 255;		// X				400 - 417
		else if(i <= 170)	j = 267;		// Z				430 - 437
		else if(i <= 192)	j = 289;		// ﾎﾟｰﾀﾌﾞﾙ			460 - 481
		// 終了条件
		else if(i <= 198)	j = 307;		// ﾀｲﾑｱｳﾄ時間		500 - 505
		else if(i <= 204)	j = 311;		// OKｶｳﾝﾄ数			510 - 515
		else if(i <= 210)	j = 315;		// 連続NGｶｳﾝﾄ数		520 - 525
#endif
// chg 2016.10.18 K.Uemura end
		
		temp = para_ini[i];
		
		I2C.WR_BUF[4*(i+j)-3]	= temp >> 24;
		I2C.WR_BUF[4*(i+j)-2]	= temp >> 16;
		I2C.WR_BUF[4*(i+j)-1]	= temp >> 8;
		I2C.WR_BUF[4*(i+j)]		= temp;
	}
}

// add 2016.10.18 K.Uemura start	GA1801
/***** HEADER START ************************************************************
	[Function]		関数名
	[Summary]		処理名
	[Description]	概要
	[Caution]		注意
	[Arguments]		引数(入力:Input，出力:Output)
					I/O	
	[Return Code]	戻り値(ARGU_ERR:引数エラーなど)
					0(固定)
	[File Name]		iic_riic0.c
	[History]		2016/10/18	K.Uemura	新規作成
****** HEADER END *************************************************************/
unsigned short convert_DLPM( unsigned short index )
{
	unsigned short DLPM_no = 0;

	// 設定
	if(index <= 16)			DLPM_no = 0;		// 露光時間			  1 -  16
	//汚れ率
												// X
												// Z
	//初期チェック
												// X
	else if(index <= 23)	DLPM_no = 3;		// Z				 20 -  26
	else if(index <= 30)	DLPM_no = 6;		// NORMAL			 30 -  36
	else if(index <= 37)	DLPM_no = 9;		// ECO				 40 -  46
	// 計測設定
	else if(index <= 43)	DLPM_no = 12;		// 共通				 50 -  55
	else if(index <= 57)	DLPM_no = 16;		// X				 60 -  73
	else if(index <= 71)	DLPM_no = 22;		// Z				 80 -  93
	// 換算TBL
	else if(index <= 80)	DLPM_no = 28;		// X				100 - 102
												// Z				103 - 105
												// ﾎﾟｰﾀﾌﾞﾙ			106 - 108
	// 通信設定
	else if(index <= 87)	DLPM_no = 69;		// 通信設定			150 - 156
	else if(index <= 89)	DLPM_no = 72;		// 通信設定			160 - 161
	// SKIP
	else if(index <= 94)	DLPM_no = 110;		// 出力設定			200 - 204
	else if(index <= 106)	DLPM_no = 115;		// ORIGIN-X			210 - 221
	else if(index <= 118)	DLPM_no = 123;		// ORIGIN-Z			230 - 241
	else if(index <= 130)	DLPM_no = 131;		// 焦点合わせ-X		250 - 261
	else if(index <= 136)	DLPM_no = 139;		// 焦点合わせ-Z		270 - 275
	// 表示/出力
	else if(index <= 140)	DLPM_no = 164;		// 表示				301 - 304
	else if(index <= 145)	DLPM_no = 169;		// 伸び				310 - 312
												// 焦点合わせ		313
												// 中心位置設定		314

// add 2016.12.06 K.Uemura start	GC0602
	else if(index <= 146)	DLPM_no = 174;		// 測定精度			320
// add 2016.12.06 K.Uemura end

	// LED点灯範囲
	else if(index <= 164)	DLPM_no = 253;		// X				400 - 417
	else if(index <= 172)	DLPM_no = 265;		// Z				430 - 437
	else if(index <= 194)	DLPM_no = 287;		// ﾎﾟｰﾀﾌﾞﾙ			460 - 481
	// 終了条件
	else if(index <= 200)	DLPM_no = 305;		// ﾀｲﾑｱｳﾄ時間		500 - 505
	else if(index <= 206)	DLPM_no = 309;		// OKｶｳﾝﾄ数			510 - 515
	else if(index <= 212)	DLPM_no = 313;		// 連続NGｶｳﾝﾄ数		520 - 525

	return( DLPM_no );
}
// add 2016.10.18 K.Uemura end

//************************************************************/
//				LED割り付け(X)
//************************************************************/
void led_layout_x(void)
{
	_UWORD i, j, temp, green_led_divison, table_upper, table_lower, table_center;
	float percentage;
	
	table_center = X_TABLE_CENTER;		// 換算ﾃｰﾌﾞﾙで中央にする配列のｲﾝﾃﾞｯｸｽ(X)
	
	// 無効ｴﾘｱの設定
	// LED1 開始・終了		// 換算ﾃｰﾌﾞﾙの(最小値の平均値)を四捨五入した値
	X_LED1_START	= X_LED1_END	= ((SEQ.TABLE_EDGE_LEFT[0] + SEQ.TABLE_EDGE_RIGHT[0])/2 + 5) / 10;
	// LED9 開始・終了		// 換算ﾃｰﾌﾞﾙの(最大値の平均値)を四捨五入した値
	X_LED9_START	= X_LED9_END	= ((SEQ.TABLE_EDGE_LEFT[X_DIVISION_NUMBER-1] + SEQ.TABLE_EDGE_RIGHT[X_DIVISION_NUMBER-1])/2 + 5) / 10;
	
	// 中央の設定
	// LED5 開始・終了		// 換算ﾃｰﾌﾞﾙの(中央の値)を四捨五入した値
	X_LED5_START	= X_LED5_END	= ((SEQ.TABLE_EDGE_LEFT[table_center] + SEQ.TABLE_EDGE_RIGHT[table_center])/2 + 5) / 10;
	
	// 無効ｴﾘｱ・中央の隣接部分の設定
	X_LED2_START	= X_LED1_END + 1;				// LED2 開始
	X_LED4_END		= X_LED5_START - 1;				// LED4 終了
	X_LED6_START	= X_LED5_END + 1;				// LED6 開始
	X_LED8_END		= X_LED9_START - 1;				// LED8 終了
	
	// 緑LEDの分割数
	green_led_divison = ((X_DIVISION_NUMBER-1)*X_CORRECTION_INTERVAL)/6;
	
	j = 1;
	for(i=1; i<=X_DIVISION_NUMBER-1; i++){
		if(i*X_CORRECTION_INTERVAL > j*green_led_divison){
			percentage	= (float)(j*green_led_divison - (i-1)*X_CORRECTION_INTERVAL) / X_CORRECTION_INTERVAL;
			table_upper	= (SEQ.TABLE_EDGE_LEFT[i] + SEQ.TABLE_EDGE_RIGHT[i])/2;
			table_lower	= (SEQ.TABLE_EDGE_LEFT[i-1] + SEQ.TABLE_EDGE_RIGHT[i-1])/2;
			// 換算ﾃｰﾌﾞﾙの(上段の平均値)-(下段の平均値)*ﾊﾟｰｾﾝﾃｰｼﾞ+(下段の平均値)を四捨五入した値
			// 10倍の値なので1/10倍して元に戻す
			temp		= ((table_upper - table_lower) * percentage + table_lower + 5) / 10;
			
			if(j == 1){
				X_LED2_END		= temp;			// LED2 終了
				X_LED3_START	= temp + 1;		// LED3 開始
			}else if(j == 2){
				X_LED3_END		= temp;			// LED3 終了
				X_LED4_START	= temp + 1;		// LED4 開始
				j++;
			}else if(j == 4){
				X_LED6_END		= temp;			// LED6 終了
				X_LED7_START	= temp + 1;		// LED7 開始
			}else if(j == 5){
				X_LED7_END		= temp;			// LED7 終了
				X_LED8_START	= temp + 1;		// LED8 開始
			}
			
			if(j < 5)	j++;
			else		break;
		}
	}
	
	j = 0;
	
	for(i=400; i<=417; i++){
		I2C.RE_BUF[4*(i+j)-3]	= SEQ.ALL_DATA[i] >> 24;
		I2C.RE_BUF[4*(i+j)-2]	= SEQ.ALL_DATA[i] >> 16;
		I2C.RE_BUF[4*(i+j)-1]	= SEQ.ALL_DATA[i] >> 8;
		I2C.RE_BUF[4*(i+j)]		= SEQ.ALL_DATA[i];
	}
}

//************************************************************/
//				LED割り付け(Z)
//************************************************************/
void led_layout_z(void)
{
	_UWORD i, j, table_center;
	
	table_center = Z_TABLE_CENTER;		// 換算ﾃｰﾌﾞﾙで中央にする配列のｲﾝﾃﾞｯｸｽ(Z)
	
	// 無効ｴﾘｱの設定
	// LED1 開始			// 換算ﾃｰﾌﾞﾙの(最小値の平均値)を四捨五入した値
	Z_LED1_START					= (SEQ.Z_TABLE_EDGE_RIGHT[0] + 5) / 10;
	// LED4 開始・終了		// 換算ﾃｰﾌﾞﾙの(最大値の平均値)を四捨五入した値
	Z_LED4_START	= Z_LED4_END	= (SEQ.Z_TABLE_EDGE_RIGHT[Z_DIVISION_NUMBER-1] + 5) / 10;
	
	// 中央の設定
	// LED2 開始・終了		// 換算ﾃｰﾌﾞﾙの(中央に設定した配列値)を四捨五入した値
	Z_LED2_START	= Z_LED2_END	= (SEQ.Z_TABLE_EDGE_RIGHT[table_center] + 5) / 10;
	
	// 無効ｴﾘｱ・中央の隣接部分の設定
	Z_LED1_END		= Z_LED2_START - 1;			// LED1 終了
	Z_LED3_START	= Z_LED2_END + 1;			// LED3 開始
	Z_LED3_END		= Z_LED4_START - 1;			// LED3 終了
	
	j = 0;
	
	for(i=430; i<=437; i++){
		I2C.RE_BUF[4*(i+j)-3]	= SEQ.ALL_DATA[i] >> 24;
		I2C.RE_BUF[4*(i+j)-2]	= SEQ.ALL_DATA[i] >> 16;
		I2C.RE_BUF[4*(i+j)-1]	= SEQ.ALL_DATA[i] >> 8;
		I2C.RE_BUF[4*(i+j)]		= SEQ.ALL_DATA[i];
	}
}

//************************************************************/
//				LED割り付け(ﾎﾟｰﾀﾌﾞﾙ)
//************************************************************/
void led_layout_portable(void)
{
	_UWORD i, j, temp, green_led_divison, table_upper, table_lower, table_center;
	float percentage;
	
	table_center = PORTABLE_TABLE_CENTER;		// 換算ﾃｰﾌﾞﾙで中央にする配列のｲﾝﾃﾞｯｸｽ(X)
	
	// 無効ｴﾘｱの設定
	// LED1 開始・終了		// 換算ﾃｰﾌﾞﾙの(最小値の平均値)を四捨五入した値
	LED1_START	= LED1_END	= ((SEQ.TABLE_EDGE_LEFT[0] + SEQ.TABLE_EDGE_RIGHT[0])/2 + 5) / 10;
	// LED11 開始・終了		// 換算ﾃｰﾌﾞﾙの(最大値の平均値)を四捨五入した値
	LED11_START	= LED11_END	= ((SEQ.TABLE_EDGE_LEFT[PORTABLE_DIVISION_NUMBER-1] + SEQ.TABLE_EDGE_RIGHT[PORTABLE_DIVISION_NUMBER-1])/2 + 5) / 10;
	
	// 中央の設定
	// LED6 開始・終了		// 換算ﾃｰﾌﾞﾙの(中央の値)を四捨五入した値
	LED6_START	= LED6_END	= ((SEQ.TABLE_EDGE_LEFT[table_center] + SEQ.TABLE_EDGE_RIGHT[table_center])/2 + 5) / 10;
	
	// 無効ｴﾘｱ・中央の隣接部分の設定
	LED2_START	= LED1_END + 1;				// LED2 開始
	LED5_END	= LED6_START - 1;			// LED5 終了
	LED7_START	= LED6_END + 1;				// LED7 開始
	LED10_END	= LED11_START - 1;			// LED10 終了
	
	// 緑LEDの分割数
	green_led_divison = ((PORTABLE_DIVISION_NUMBER-1)*PORTABLE_CORRECTION_INTERVAL)/8;
	
	j = 1;
	for(i=1; i<=PORTABLE_DIVISION_NUMBER-1; i++){
		if(i*PORTABLE_CORRECTION_INTERVAL > j*green_led_divison){
			percentage	= (float)(j*green_led_divison - (i-1)*PORTABLE_CORRECTION_INTERVAL) / PORTABLE_CORRECTION_INTERVAL;
			table_upper	= (SEQ.TABLE_EDGE_LEFT[i] + SEQ.TABLE_EDGE_RIGHT[i])/2;
			table_lower	= (SEQ.TABLE_EDGE_LEFT[i-1] + SEQ.TABLE_EDGE_RIGHT[i-1])/2;
			// 換算ﾃｰﾌﾞﾙの(上段の平均値)-(下段の平均値)*ﾊﾟｰｾﾝﾃｰｼﾞ+(下段の平均値)を四捨五入した値
			// 10倍の値なので1/10倍して元に戻す
			temp		= ((table_upper - table_lower) * percentage + table_lower + 5) / 10;
			
			if(j == 1){
				LED2_END	= temp;			// LED2 終了
				LED3_START	= temp + 1;		// LED3 開始
			}else if(j == 2){
				LED3_END	= temp;			// LED3 終了
				LED4_START	= temp + 1;		// LED4 開始
			}else if(j == 3){
				LED4_END	= temp;			// LED4 終了
				LED5_START	= temp + 1;		// LED5 開始
				j++;
			}else if(j == 5){
				LED7_END	= temp;			// LED7 終了
				LED8_START	= temp + 1;		// LED8 開始
			}else if(j == 6){
				LED8_END	= temp;			// LED8 終了
				LED9_START	= temp + 1;		// LED9 開始
			}else if(j == 7){
				LED9_END	= temp;			// LED9 終了
				LED10_START	= temp + 1;		// LED10 開始
			}
			
			if(j < 7)	j++;
			else		break;
		}
	}
	
	j = 0;
	
	for(i=460; i<=481; i++){
		I2C.RE_BUF[4*(i+j)-3]	= SEQ.ALL_DATA[i] >> 24;
		I2C.RE_BUF[4*(i+j)-2]	= SEQ.ALL_DATA[i] >> 16;
		I2C.RE_BUF[4*(i+j)-1]	= SEQ.ALL_DATA[i] >> 8;
		I2C.RE_BUF[4*(i+j)]		= SEQ.ALL_DATA[i];
	}
}
