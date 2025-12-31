/******************************************************************************
* File Name	: fpga.c
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

void send_to_fpga3(void);				// ﾌﾞﾗｯｸﾁｪｯｸ・ﾎﾜｲﾄﾁｪｯｸ・ﾗｲﾝｾﾝｻﾁｪｯｸ・調光
void bus_to_in(void);					// ﾊﾞｽ入力変更関数
void bus_to_out(void);					// ﾊﾞｽ出力変更関数
_UWORD from_cbus(void);					// ｺﾏﾝﾄﾞﾅﾝﾊﾞｰ入力関数
_UDWORD from_dbus(void);				// ﾃﾞｰﾀ入力関数
void send_to_cbus_zero(void);			// ｺﾏﾝﾄﾞﾅﾝﾊﾞｰ出力関数0
void send_to_dbus_zero(void);			// ﾃﾞｰﾀ出力関数0
void send_to_cbus(_UWORD w_data);		// ｺﾏﾝﾄﾞﾅﾝﾊﾞｰ出力関数
void send_to_dbus(_UDWORD w_data);		// ﾃﾞｰﾀ出力関数

//************************************************************/
//				FPGAへのﾃﾞｰﾀ送信関数(ﾌﾞﾗｯｸ・ﾎﾜｲﾄ)
//************************************************************/
// ﾌﾞﾗｯｸﾁｪｯｸ・ﾎﾜｲﾄﾁｪｯｸ・ﾗｲﾝｾﾝｻﾁｪｯｸ・調光
void send_to_fpga3(void)
{
	switch(SEQ.FPGA_SEND_STATUS){
		// RX → FPGAにﾃﾞｰﾀ送信
		// C_PRIOを「H」にする
		case 1:
			//if(F_PRIO_IN == 0){						// F_PRIO_INが「L」のとき
				C_PRIO_OUT	= 1;						// C_PRIO
				SEQ.FPGA_SEND_STATUS++;					// 次へ
			//}
			break;
			
		// ﾎﾟｰﾄを出力に設定する
		case 2:
			bus_to_out();								// ﾊﾞｽを出力に設定
			send_to_cbus_zero();						// ｺﾏﾝﾄﾞﾅﾝﾊﾞｰ出力関数0
			SEQ.FPGA_SEND_STATUS++;						// 次へ
			break;
			
		// ｺﾏﾝﾄﾞﾊﾞｽ・ﾃﾞｰﾀﾊﾞｽを設定する
		case 3:
			send_to_cbus(SEQ.CBUS_NUMBER);				// ｺﾏﾝﾄﾞﾅﾝﾊﾞｰ出力
			send_to_dbus_zero();						// ﾃﾞｰﾀ出力関数0
			
			SEQ.FPGA_SEND_STATUS++;						// 次へ
			break;
			
		// C_ACKを「H」にする
		case 4:
			C_ACK_OUT	= 1;							// C_ACK
			SEQ.FPGA_SEND_STATUS++;						// 次へ
			break;
			
		// C_ACKを「L」にする
		case 5:
			C_ACK_OUT	= 0;							// C_ACK
			SEQ.FPGA_SEND_STATUS++;						// 次へ
			break;
			
		// ｺﾏﾝﾄﾞﾊﾞｽ・ﾃﾞｰﾀﾊﾞｽを「L」にする
		case 6:
			send_to_cbus_zero();						// ｺﾏﾝﾄﾞﾅﾝﾊﾞｰ出力関数0
			send_to_dbus_zero();						// ﾃﾞｰﾀ出力関数0
			SEQ.FPGA_SEND_STATUS++;						// 次へ
			break;
			
		// ﾎﾟｰﾄを入力に設定する
		case 7:
			bus_to_in();								// ﾊﾞｽを入力に設定
			SEQ.FPGA_SEND_STATUS++;						// 次へ
			break;
			
		// C_PRIOを「L」にする
		case 8:
			C_PRIO_OUT	= 0;							// C_PRIO
			SEQ.FPGA_SEND_STATUS = 11;
			break;
			
		// FPGA → RXにﾃﾞｰﾀ送信
		// F_PRIO_INが「H」になったらC_ACKを「H」にする
		case 11:
			if(F_PRIO_IN == 1){
				SEQ.FPGA_SEND_STATUS++;
				C_ACK_OUT	= 1;						// C_ACK
			}
			break;
			
		case 12:
			SEQ.FPGA_SEND_STATUS++;						// 次へ
			C_ACK_OUT	= 0;							// C_ACK
			break;
			
		case 13:	// 割り込み
			SEQ.FPGA_SEND_STATUS++;						// 次へ
			break;
			
			
		// C_INT_INが「H」になったら、LED輝度信号返信のとき値の判定を行う
		case 14:
			if(C_INT_IN == 0){							// C_INT_INが「L」のとき
				SEQ.FPGA_SEND_STATUS = 14;
				
			}else{										// C_INT_INが「H」のとき
				if(SEQ.CBUS_NUMBER == 201){				// ﾌﾞﾗｯｸ固定値設定完了のとき
					SEQ.WHITE_CHECK_COUNT = 0;			// WHITEﾁｪｯｸｶｳﾝﾀ
					SEQ.BLACK_WHITE_CHECK[SEQ.WHITE_CHECK_COUNT] = (SEQ.INPUT_DBUS_LONG & 0xFF);	// BLACKﾁｪｯｸ・WHITEﾁｪｯｸのﾃﾞｰﾀ(下位8ﾋﾞｯﾄ)
					
				}else if((SEQ.CBUS_NUMBER >= 204)&&(SEQ.CBUS_NUMBER <= 206)){						// ﾎﾜｲﾄ固定値設定完了のとき
					SEQ.BLACK_WHITE_CHECK[SEQ.WHITE_CHECK_COUNT] = (SEQ.INPUT_DBUS_LONG & 0xFF);	// BLACKﾁｪｯｸ・WHITEﾁｪｯｸのﾃﾞｰﾀ(下位8ﾋﾞｯﾄ)
				}
				SEQ.FPGA_SEND_STATUS = 16;
			}
			C_ACK_OUT	= 1;							// C_ACK
			C_ACK_OUT	= 0;							// C_ACK
			break;
			
			/*
		case 15:
			SEQ.FPGA_SEND_STATUS++;						// 次へ(割り込みがすぐに入る可能性があるため、あらかじめｽﾃｰﾀｽを進めておく)
			//C_ACK_OUT	= 1;							// C_ACK
			break;
			*/
			
		case 16:
			//C_ACK_OUT	= 0;							// C_ACK
			if(SEQ.CBUS_NUMBER == 201){					// ﾌﾞﾗｯｸ固定値設定完了のとき
				SEQ.FPGA_SEND_STATUS++;
				SEQ.CHANGE_FPGA = 0;
				SEQ.WHITE_CHECK_COUNT = 1;				// WHITEﾁｪｯｸｶｳﾝﾀ
				SEQ.POWER_STATUS++;
			}else if(SEQ.CBUS_NUMBER == 203){			// 
				SEQ.FPGA_SEND_STATUS++;
				SEQ.CHANGE_FPGA = 0;
				SEQ.POWER_STATUS++;
			}else if((SEQ.CBUS_NUMBER == 204)||(SEQ.CBUS_NUMBER == 205)){		// ﾎﾜｲﾄ平均値・ﾎﾜｲﾄﾁｪｯｸ最大値
				SEQ.FPGA_SEND_STATUS = 11;
				SEQ.WHITE_CHECK_COUNT++;				// WHITEﾁｪｯｸｶｳﾝﾀ
			}else if(SEQ.CBUS_NUMBER == 206){			// ﾎﾜｲﾄ最小値
				SEQ.FPGA_SEND_STATUS++;
				SEQ.CHANGE_FPGA = 0;
				SEQ.POWER_STATUS++;
			}
			break;
	}
}

//************************************************************/
//				ﾊﾞｽ入力変更関数
//************************************************************/
void bus_to_in(void)
{
	PORT9.DDR.BIT.B3	= 0;			// P93入力
	PORTA.DDR.BYTE		= 0x00;			// PA0-A7入力
	PORTB.DDR.BYTE		= 0x00;			// PB0-B7入力
	PORTC.DDR.BYTE		= 0x00;			// PC0-C7入力
	PORTD.DDR.BYTE		= 0x00;			// PD0-D7入力
	PORTE.DDR.BYTE		= 0x00;			// PE0-E7入力
	
	PORT9.ICR.BIT.B3	= 1;		// 00001001
	PORTA.ICR.BYTE		= 0xFF;		// 11111111
	PORTB.ICR.BYTE		= 0xFF;		// 11111111
	PORTC.ICR.BYTE		= 0xFF;		// 11111111
	PORTD.ICR.BYTE		= 0xFF;		// 11111111
	PORTE.ICR.BYTE		= 0xFF;		// 11111111
}

//************************************************************/
//				ﾊﾞｽ出力変更関数
//************************************************************/
void bus_to_out(void)
{
	PORT9.ICR.BIT.B3	= 0;		// 00001001
	PORTA.ICR.BYTE		= 0x00;		// 11111111
	PORTB.ICR.BYTE		= 0x00;		// 11111111
	PORTC.ICR.BYTE		= 0x00;		// 11111111
	PORTD.ICR.BYTE		= 0x00;		// 11111111
	PORTE.ICR.BYTE		= 0x00;		// 11111111
	
	PORT9.DDR.BIT.B3	= 1;			// P93出力
	PORTA.DDR.BYTE		= 0xFF;			// PA0-A7出力
	PORTB.DDR.BYTE		= 0xFF;			// PB0-B7出力
	PORTC.DDR.BYTE		= 0xFF;			// PC0-C7出力
	PORTD.DDR.BYTE		= 0xFF;			// PD0-D7出力
	PORTE.DDR.BYTE		= 0xFF;			// PE0-E7出力
}

//************************************************************/
//				ｺﾏﾝﾄﾞﾅﾝﾊﾞｰ入力関数
//************************************************************/
_UWORD from_cbus(void)
{
	_UWORD w_data;
	
	/*
	w_data = (CBUS8_IN << 8) + (CBUS7_IN << 7) + (CBUS6_IN << 6) + (CBUS5_IN << 5)
			+ (CBUS4_IN << 4) + (CBUS3_IN << 3) + (CBUS2_IN << 2) + (CBUS1_IN << 1) + CBUS0_IN;
			*/
	w_data = (CBUS8_IN << 8) + PORTE.PORT.BYTE;
			
	return(w_data);
}

//************************************************************/
//				ﾃﾞｰﾀ入力関数
//************************************************************/
_UDWORD from_dbus(void)
{
	_UDWORD w_data;
	
	/*
	w_data = (DBUS31_IN << 31) + (DBUS30_IN << 30) + (DBUS29_IN << 29)
			+ (DBUS28_IN << 28) + (DBUS27_IN << 27) + (DBUS26_IN << 26) + (DBUS25_IN << 25)
			+ (DBUS24_IN << 24) + (DBUS23_IN << 23) + (DBUS22_IN << 22) + (DBUS21_IN << 21)
			+ (DBUS20_IN << 20) + (DBUS19_IN << 19) + (DBUS18_IN << 18) + (DBUS17_IN << 17)
			+ (DBUS16_IN << 16) + (DBUS15_IN << 15) + (DBUS14_IN << 14) + (DBUS13_IN << 13)
			+ (DBUS12_IN << 12) + (DBUS11_IN << 11) + (DBUS10_IN << 10) + (DBUS09_IN << 9)
			+ (DBUS08_IN << 8) + (DBUS07_IN << 7) + (DBUS06_IN << 6) + (DBUS05_IN << 5)
			+ (DBUS04_IN << 4) + (DBUS03_IN << 3) + (DBUS02_IN << 2) + (DBUS01_IN << 1) + DBUS00_IN;
			*/
			
	w_data = (PORTD.PORT.BYTE << 24) + (PORTC.PORT.BYTE << 16) + (PORTB.PORT.BYTE << 8) + PORTA.PORT.BYTE;
			
	return(w_data);
}

//************************************************************/
//				ｺﾏﾝﾄﾞﾅﾝﾊﾞｰ出力関数0
//************************************************************/
void send_to_cbus_zero(void)
{
	/*
	// ｺﾏﾝﾄﾞﾊﾞｽ出力
	CBUS0_OUT = 0;
	CBUS1_OUT = 0;
	CBUS2_OUT = 0;
	CBUS3_OUT = 0;
	CBUS4_OUT = 0;
	CBUS5_OUT = 0;
	CBUS6_OUT = 0;
	CBUS7_OUT = 0;
	CBUS8_OUT = 0;
	*/
	
	PORTE.DR.BYTE = 0x00;			// CBUS0出力
	CBUS8_OUT = 0;
}

//************************************************************/
//				ﾃﾞｰﾀ出力関数0
//************************************************************/
void send_to_dbus_zero(void)
{
	/*
	// ﾃﾞｰﾀ出力
	DBUS00_OUT = 0;
	DBUS01_OUT = 0;
	DBUS02_OUT = 0;
	DBUS03_OUT = 0;
	DBUS04_OUT = 0;
	DBUS05_OUT = 0;
	DBUS06_OUT = 0;
	DBUS07_OUT = 0;
	DBUS08_OUT = 0;
	DBUS09_OUT = 0;

	DBUS10_OUT = 0;
	DBUS11_OUT = 0;
	DBUS12_OUT = 0;
	DBUS13_OUT = 0;
	DBUS14_OUT = 0;
	DBUS15_OUT = 0;
	DBUS16_OUT = 0;
	DBUS17_OUT = 0;
	DBUS18_OUT = 0;
	DBUS19_OUT = 0;
	
	DBUS20_OUT = 0;
	DBUS21_OUT = 0;
	DBUS22_OUT = 0;
	DBUS23_OUT = 0;
	DBUS24_OUT = 0;
	DBUS25_OUT = 0;
	DBUS26_OUT = 0;
	DBUS27_OUT = 0;
	DBUS28_OUT = 0;
	DBUS29_OUT = 0;
	
	DBUS30_OUT = 0;
	DBUS31_OUT = 0;
	*/
	
	PORTA.DR.BYTE = 0x00;
	PORTB.DR.BYTE = 0x00;
	PORTC.DR.BYTE = 0x00;
	PORTD.DR.BYTE = 0x00;
}

//************************************************************/
//				ｺﾏﾝﾄﾞﾅﾝﾊﾞｰ出力関数
//************************************************************/
void send_to_cbus(_UWORD w_data)
{
	/*
	// ｺﾏﾝﾄﾞﾊﾞｽ出力
	CBUS0_OUT = w_data & 0x01;
	CBUS1_OUT = (w_data >> 1) & 0x01;
	CBUS2_OUT = (w_data >> 2) & 0x01;
	CBUS3_OUT = (w_data >> 3) & 0x01;
	CBUS4_OUT = (w_data >> 4) & 0x01;
	CBUS5_OUT = (w_data >> 5) & 0x01;
	CBUS6_OUT = (w_data >> 6) & 0x01;
	CBUS7_OUT = (w_data >> 7) & 0x01;
	CBUS8_OUT = (w_data >> 8) & 0x01;
	*/
	
	PORTE.DR.BYTE = w_data & 0xFF;			// CBUS0出力
	CBUS8_OUT = (w_data >> 8) & 0x01;
}

//************************************************************/
//				ﾃﾞｰﾀ出力関数
//************************************************************/
void send_to_dbus(_UDWORD w_data)
{
	/*
	// ﾃﾞｰﾀ出力
	DBUS00_OUT = w_data & 0x01;
	DBUS01_OUT = (w_data >> 1) & 0x01;
	DBUS02_OUT = (w_data >> 2) & 0x01;
	DBUS03_OUT = (w_data >> 3) & 0x01;
	DBUS04_OUT = (w_data >> 4) & 0x01;
	DBUS05_OUT = (w_data >> 5) & 0x01;
	DBUS06_OUT = (w_data >> 6) & 0x01;
	DBUS07_OUT = (w_data >> 7) & 0x01;
	DBUS08_OUT = (w_data >> 8) & 0x01;
	DBUS09_OUT = (w_data >> 9) & 0x01;

	DBUS10_OUT = (w_data >> 10) & 0x01;
	DBUS11_OUT = (w_data >> 11) & 0x01;
	DBUS12_OUT = (w_data >> 12) & 0x01;
	DBUS13_OUT = (w_data >> 13) & 0x01;
	DBUS14_OUT = (w_data >> 14) & 0x01;
	DBUS15_OUT = (w_data >> 15) & 0x01;
	DBUS16_OUT = (w_data >> 16) & 0x01;
	DBUS17_OUT = (w_data >> 17) & 0x01;
	DBUS18_OUT = (w_data >> 18) & 0x01;
	DBUS19_OUT = (w_data >> 19) & 0x01;
	
	DBUS20_OUT = (w_data >> 20) & 0x01;
	DBUS21_OUT = (w_data >> 21) & 0x01;
	DBUS22_OUT = (w_data >> 22) & 0x01;
	DBUS23_OUT = (w_data >> 23) & 0x01;
	DBUS24_OUT = (w_data >> 24) & 0x01;
	DBUS25_OUT = (w_data >> 25) & 0x01;
	DBUS26_OUT = (w_data >> 26) & 0x01;
	DBUS27_OUT = (w_data >> 27) & 0x01;
	DBUS28_OUT = (w_data >> 28) & 0x01;
	DBUS29_OUT = (w_data >> 29) & 0x01;
	
	DBUS30_OUT = (w_data >> 30) & 0x01;
	DBUS31_OUT = (w_data >> 31) & 0x01;
	*/
	
	PORTA.DR.BYTE = w_data & 0xFF;
	PORTB.DR.BYTE = (w_data >> 8) & 0xFF;
	PORTC.DR.BYTE = (w_data >> 16) & 0xFF;
	PORTD.DR.BYTE = (w_data >> 24) & 0xFF;
}
