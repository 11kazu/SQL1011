/******************************************************************************
* File Name	: user_define.h
******************************************************************************/

// RXｿﾌﾄｳｪｱﾊﾞｰｼﾞｮﾝ
#define RX_RIVISION			202						// RXﾊﾞｰｼﾞｮﾝ 170328

#define EXHIBITION			0						// ピークホールド繰り返し(0:しない 1:する)

/************************************************************/
/*					ｸﾞﾛｰﾊﾞﾙ定数定義							*/
/************************************************************/
// 入力
// 入力ﾎﾟｰﾄ定義
#define CNC_CONTROL_IN		PORT0.PORT.BIT.B7		// CNC/ﾎﾟｰﾀﾌﾞﾙ切替

#define EXT_POWER_IN		PORT1.PORT.BIT.B7		// 外部電源
#define AWAKE_IN			PORT5.PORT.BIT.B6		// AWAKE
#define C_INT_IN			PORT3.PORT.BIT.B3		// C_INT

// ｽｲｯﾁ入力
#define POWER_SW_IN			PORT4.PORT.BIT.B7		// 電源ｽｲｯﾁ			S1
#define START_SW_IN			PORT6.PORT.BIT.B1		// ｽﾀｰﾄｽｲｯﾁ			S2
#define CLEAR_SW_IN			PORT6.PORT.BIT.B2		// ｸﾘｱｽｲｯﾁ			S3
#define SELECT_SW_IN		PORT6.PORT.BIT.B3		// ｾﾚｸﾄｽｲｯﾁ			S4
#define PEAKHOLD_SW_IN		PORT6.PORT.BIT.B4		// ﾋﾟｰｸﾎｰﾙﾄﾞｽｲｯﾁ	S5
#define MODE_SW_IN			PORT6.PORT.BIT.B5		// ﾓｰﾄﾞｽｲｯﾁ			S6
#define UP_SW_IN			PORT4.PORT.BIT.B5		// 上ｽｲｯﾁ			S7
#define DOWN_SW_IN			PORT4.PORT.BIT.B6		// 下ｽｲｯﾁ			S8
#define ECO_SW_IN			PORT6.PORT.BIT.B0		// ECOｽｲｯﾁ			S9

#define BOOT_SW_IN			PORT6.PORT.BIT.B6		// BOOTｽｲｯﾁ

#define COVER_OPEN_IN		PORT2.PORT.BIT.B5		// ｶﾊﾞｰｽｲｯﾁ開
#define COVER_CLOSE_IN		PORT2.PORT.BIT.B2		// ｶﾊﾞｰｽｲｯﾁ閉

#define F_PRIO_IN			PORT9.PORT.BIT.B0		// F_PRIO入力

//
#define CFG_INT_IN			PORT4.PORT.BIT.B2		// CFG_INT
#define CFG_DONE_IN			PORT4.PORT.BIT.B3		// CFG_DONE
//

// ﾃﾞｰﾀﾊﾞｽ入力
#define DBUS00_IN			PORTA.PORT.BIT.B0		// DBUS00入力
#define DBUS01_IN			PORTA.PORT.BIT.B1		// DBUS01入力
#define DBUS02_IN			PORTA.PORT.BIT.B2		// DBUS02入力
#define DBUS03_IN			PORTA.PORT.BIT.B3		// DBUS03入力
#define DBUS04_IN			PORTA.PORT.BIT.B4		// DBUS04入力
#define DBUS05_IN			PORTA.PORT.BIT.B5		// DBUS05入力
#define DBUS06_IN			PORTA.PORT.BIT.B6		// DBUS06入力
#define DBUS07_IN			PORTA.PORT.BIT.B7		// DBUS07入力

#define DBUS08_IN			PORTB.PORT.BIT.B0		// DBUS08入力
#define DBUS09_IN			PORTB.PORT.BIT.B1		// DBUS09入力
#define DBUS10_IN			PORTB.PORT.BIT.B2		// DBUS10入力
#define DBUS11_IN			PORTB.PORT.BIT.B3		// DBUS11入力
#define DBUS12_IN			PORTB.PORT.BIT.B4		// DBUS12入力
#define DBUS13_IN			PORTB.PORT.BIT.B5		// DBUS13入力
#define DBUS14_IN			PORTB.PORT.BIT.B6		// DBUS14入力
#define DBUS15_IN			PORTB.PORT.BIT.B7		// DBUS15入力

#define DBUS16_IN			PORTC.PORT.BIT.B0		// DBUS16入力
#define DBUS17_IN			PORTC.PORT.BIT.B1		// DBUS17入力
#define DBUS18_IN			PORTC.PORT.BIT.B2		// DBUS18入力
#define DBUS19_IN			PORTC.PORT.BIT.B3		// DBUS19入力
#define DBUS20_IN			PORTC.PORT.BIT.B4		// DBUS20入力
#define DBUS21_IN			PORTC.PORT.BIT.B5		// DBUS21入力
#define DBUS22_IN			PORTC.PORT.BIT.B6		// DBUS22入力
#define DBUS23_IN			PORTC.PORT.BIT.B7		// DBUS23入力

#define DBUS24_IN			PORTD.PORT.BIT.B0		// DBUS24入力
#define DBUS25_IN			PORTD.PORT.BIT.B1		// DBUS25入力
#define DBUS26_IN			PORTD.PORT.BIT.B2		// DBUS26入力
#define DBUS27_IN			PORTD.PORT.BIT.B3		// DBUS27入力
#define DBUS28_IN			PORTD.PORT.BIT.B4		// DBUS28入力
#define DBUS29_IN			PORTD.PORT.BIT.B5		// DBUS29入力
#define DBUS30_IN			PORTD.PORT.BIT.B6		// DBUS30入力
#define DBUS31_IN			PORTD.PORT.BIT.B7		// DBUS31入力

// ｺﾏﾝﾄﾞﾊﾞｽ入力
#define CBUS0_IN			PORTE.PORT.BIT.B0		// CBUS0入力
#define CBUS1_IN			PORTE.PORT.BIT.B1		// CBUS1入力
#define CBUS2_IN			PORTE.PORT.BIT.B2		// CBUS2入力
#define CBUS3_IN			PORTE.PORT.BIT.B3		// CBUS3入力
#define CBUS4_IN			PORTE.PORT.BIT.B4		// CBUS4入力
#define CBUS5_IN			PORTE.PORT.BIT.B5		// CBUS5入力
#define CBUS6_IN			PORTE.PORT.BIT.B6		// CBUS6入力
#define CBUS7_IN			PORTE.PORT.BIT.B7		// CBUS7入力
#define CBUS8_IN			PORT9.PORT.BIT.B3		// CBUS8入力

// 出力
// 出力ﾎﾟｰﾄ定義
#define MONITOR_OUT			PORT1.DR.BIT.B5			// ﾗﾝﾓﾆﾀ
#define SUSPEND_OUT			PORT5.DR.BIT.B5			// ｻｽﾍﾟﾝﾄﾞ

#define BOOT_LED_OUT		PORT3.DR.BIT.B2			// BOOTLED

//
#define CFG_PROG_OUT		PORT4.DR.BIT.B1			// CFG_PROG
#define M25_CS_OUT			PORT4.DR.BIT.B4			// M25_CS
//

// ﾃﾞｨｽﾌﾟﾚｲ出力
#define DISP0_OUT			PORT7.DR.BIT.B0			// DISP0出力
#define DISP1_OUT			PORT7.DR.BIT.B1			// DISP1出力
#define DISP2_OUT			PORT7.DR.BIT.B2			// DISP2出力
#define DISP3_OUT			PORT7.DR.BIT.B3			// DISP3出力
#define DISP4_OUT			PORT7.DR.BIT.B4			// DISP4出力
#define DISP5_OUT			PORT7.DR.BIT.B5			// DISP5出力
#define DISP6_OUT			PORT7.DR.BIT.B6			// DISP6出力
#define DISP7_OUT			PORT7.DR.BIT.B7			// DISP7出力
#define DISP8_OUT			PORT8.DR.BIT.B0			// DISP8出力

// ｸﾛｯｸ出力
#define CLK1_OUT			PORT8.DR.BIT.B1			// CLK1出力
#define CLK2_OUT			PORT8.DR.BIT.B2			// CLK2出力
#define CLK3_OUT			PORT8.DR.BIT.B3			// CLK3出力
#define CLK4_OUT			PORT5.DR.BIT.B1			// CLK4出力
#define CLK5_OUT			PORT5.DR.BIT.B4			// CLK5出力

// ﾃﾞｰﾀﾊﾞｽ出力
#define DBUS00_OUT			PORTA.DR.BIT.B0			// DBUS00出力
#define DBUS01_OUT			PORTA.DR.BIT.B1			// DBUS01出力
#define DBUS02_OUT			PORTA.DR.BIT.B2			// DBUS02出力
#define DBUS03_OUT			PORTA.DR.BIT.B3			// DBUS03出力
#define DBUS04_OUT			PORTA.DR.BIT.B4			// DBUS04出力
#define DBUS05_OUT			PORTA.DR.BIT.B5			// DBUS05出力
#define DBUS06_OUT			PORTA.DR.BIT.B6			// DBUS06出力
#define DBUS07_OUT			PORTA.DR.BIT.B7			// DBUS07出力

#define DBUS08_OUT			PORTB.DR.BIT.B0			// DBUS08出力
#define DBUS09_OUT			PORTB.DR.BIT.B1			// DBUS09出力
#define DBUS10_OUT			PORTB.DR.BIT.B2			// DBUS10出力
#define DBUS11_OUT			PORTB.DR.BIT.B3			// DBUS11出力
#define DBUS12_OUT			PORTB.DR.BIT.B4			// DBUS12出力
#define DBUS13_OUT			PORTB.DR.BIT.B5			// DBUS13出力
#define DBUS14_OUT			PORTB.DR.BIT.B6			// DBUS14出力
#define DBUS15_OUT			PORTB.DR.BIT.B7			// DBUS15出力

#define DBUS16_OUT			PORTC.DR.BIT.B0			// DBUS16出力
#define DBUS17_OUT			PORTC.DR.BIT.B1			// DBUS17出力
#define DBUS18_OUT			PORTC.DR.BIT.B2			// DBUS18出力
#define DBUS19_OUT			PORTC.DR.BIT.B3			// DBUS19出力
#define DBUS20_OUT			PORTC.DR.BIT.B4			// DBUS20出力
#define DBUS21_OUT			PORTC.DR.BIT.B5			// DBUS21出力
#define DBUS22_OUT			PORTC.DR.BIT.B6			// DBUS22出力
#define DBUS23_OUT			PORTC.DR.BIT.B7			// DBUS23出力

#define DBUS24_OUT			PORTD.DR.BIT.B0			// DBUS24出力
#define DBUS25_OUT			PORTD.DR.BIT.B1			// DBUS25出力
#define DBUS26_OUT			PORTD.DR.BIT.B2			// DBUS26出力
#define DBUS27_OUT			PORTD.DR.BIT.B3			// DBUS27出力
#define DBUS28_OUT			PORTD.DR.BIT.B4			// DBUS28出力
#define DBUS29_OUT			PORTD.DR.BIT.B5			// DBUS29出力
#define DBUS30_OUT			PORTD.DR.BIT.B6			// DBUS30出力
#define DBUS31_OUT			PORTD.DR.BIT.B7			// DBUS31出力

// ｺﾏﾝﾄﾞﾊﾞｽ出力
#define CBUS0_OUT			PORTE.DR.BIT.B0			// CBUS0出力
#define CBUS1_OUT			PORTE.DR.BIT.B1			// CBUS1出力
#define CBUS2_OUT			PORTE.DR.BIT.B2			// CBUS2出力
#define CBUS3_OUT			PORTE.DR.BIT.B3			// CBUS3出力
#define CBUS4_OUT			PORTE.DR.BIT.B4			// CBUS4出力
#define CBUS5_OUT			PORTE.DR.BIT.B5			// CBUS5出力
#define CBUS6_OUT			PORTE.DR.BIT.B6			// CBUS6出力
#define CBUS7_OUT			PORTE.DR.BIT.B7			// CBUS7出力
#define CBUS8_OUT			PORT9.DR.BIT.B3			// CBUS8出力

// EEPROM通信
#define SCL_OUT				PORT1.DR.BIT.B2			// SCL
#define SDA_OUT				PORT1.DR.BIT.B3			// SDA

// ﾊﾞｽ通信
#define C_PRIO_OUT			PORT9.DR.BIT.B1			// C_PRIO
#define C_ACK_OUT			PORT9.DR.BIT.B2			// C_ACK

// アナログ値
// FPGAからのデータを「0 - 255」から「155 - 775」に変換する
// ((775-155)/255) * x + 155 = y
// (x:FPGAのデータ y:マイコンのデータ)
//#define DIODES_MIN			155U					// DIODES最小値(0.5V)
//#define DIODES_MAX			775U					// DIODES最大値(2.5V)

#define	LINESENSOR_START		0						// ﾗｲﾝｾﾝｻ開始端
#define	LINESENSOR_END			4095					// ﾗｲﾝｾﾝｻ終了端
//#define	LINESENSOR_MEASURE_MIN	7						// ﾗｲﾝｾﾝｻ開始画素
//#define	LINESENSOR_MEASURE_MAX	4089					// ﾗｲﾝｾﾝｻ終了画素

#define	LINESENSOR_X_MEASURE_MIN	607						// ﾗｲﾝｾﾝｻ開始画素(X)
#define	LINESENSOR_X_MEASURE_MAX	3489					// ﾗｲﾝｾﾝｻ終了画素(X)
#define	LINESENSOR_Z_MEASURE_MIN	7						// ﾗｲﾝｾﾝｻ開始画素(Z)
#define	LINESENSOR_Z_MEASURE_MAX	1293					// ﾗｲﾝｾﾝｻ終了画素(Z)

#define	X_TABLE_CENTER				(X_DIVISION_NUMBER-1)/2		// 換算ﾃｰﾌﾞﾙで中央にする配列のｲﾝﾃﾞｯｸｽ(X)
#define	Z_TABLE_CENTER				4							// 換算ﾃｰﾌﾞﾙで中央にする配列のｲﾝﾃﾞｯｸｽ(Z)
#define	PORTABLE_TABLE_CENTER		(PORTABLE_DIVISION_NUMBER-1)/2		// 換算ﾃｰﾌﾞﾙで中央にする配列のｲﾝﾃﾞｯｸｽ(ﾎﾟｰﾀﾌﾞﾙ)

//#define	DL_CENTER				(unsigned long)(TABLE_CENTER * CORRECTION_INTERVAL)			//	補正TBLの中心を画面中心とする
//#define	DL_CENTER10				(unsigned long)(DL_CENTER * 10)

#define	DL_X_CENTER				(unsigned long)(X_TABLE_CENTER * X_CORRECTION_INTERVAL)			//	補正TBLの中心を画面中心とする
#define	DL_Z_CENTER				(unsigned long)(Z_TABLE_CENTER * Z_CORRECTION_INTERVAL)			//	補正TBLの中心を画面中心とする

#define	DL_MIN					(unsigned long)(0)												//	0を計測範囲最小
#define	DL_MAX					(unsigned long)((X_DIVISION_NUMBER-1) * X_CORRECTION_INTERVAL)	//	補正TBLの最大を計測範囲最大
#define	DL_Z_MAX				(unsigned long)((Z_DIVISION_NUMBER-1) * Z_CORRECTION_INTERVAL)	//	補正TBLの最大を計測範囲最大

//#define	DL_CENTER10			((SEQ.TABLE_EDGE_LEFT[25]+SEQ.TABLE_EDGE_RIGHT[25]) / 2.0)	//	ｴｯｼﾞ左右(実寸換算ﾃｰﾌﾞﾙ)の補正中心を画面中心とする
//#define	DL_CENTER			(DL_CENTER10 / 10.0)

// 伸び計測間隔(= 伸び計測時間 / ﾎﾟｰﾘﾝｸﾞ周期)	(単位は整数とする)
#define	GROWTH_INTERVAL			(unsigned short)(GROWTH_MEASUREMENT_TIME / POLLING_CYCLE)

#define	INITIAL_MIN			-1000000				// 結果初期化(最小値)
#define	INITIAL_MAX			1000000					// 結果初期化(最大値)
#define	FOCUS_MAX			99.9999

// ﾓｰﾄﾞ番号
#define	MODE_ORIGIN				7			// ORIGIN
#define	MODE_ORIGIN_EDGE		8			// ORIGIN(ｴｯｼﾞ考慮)
#define	MODE_D4_AUTO			10			// d＞4 自動
#define	MODE_D4_LOW				0			// d≦4
#define	MODE_D4_LEFT			1			// d＞4 左
#define	MODE_D4_RIGHT			11			// d＞4 右
#define	MODE_RUNOUT				2			// 振れ
#define	MODE_FOCUS				3			// 焦点合わせ
#define	MODE_CENTER				4			// 中心位置設定
#define	MODE_PROFILE			5			// ﾌﾟﾛﾌｧｲﾙ
#define	MODE_GROWTH				6			// 伸び測定
// add 2016.02.18 K.Uemura start	G21804
#define	MODE_CLEANING			20			// 清掃確認
// add 2016.02.18 K.Uemura end

//#define	MODE_CONVERSION			20			// 換算TBL
//#define	MODE_BUFFER_TRANSFER	30			// バッファ転送
//#define	MODE_IO_OUTPUT			40			// IO出力

#define	MODE_D4_AUTO_LOW	1				// d≦4
#define	MODE_D4_AUTO_LEFT	2				// d＞4 左
#define	MODE_D4_AUTO_RIGHT	3				// d＞4 右

#define	EDGE_OR				0				// 一方ｴｯｼﾞ
#define	EDGE_LEFT			1				// 左ｴｯｼﾞ
#define	EDGE_RIGHT			2				// 右ｴｯｼﾞ
#define	EDGE_AND			3				// 両方ｴｯｼﾞ

#define	UPPER				0				// 上
#define	LOWER				1				// 下

#define	TOOL_LEFT			0				// 工具位置左側（右エッジ）
#define	TOOL_RIGHT			1				// 工具位置右側（左エッジ）

#define	TUNING_NO			0				// ﾁｭｰﾆﾝｸﾞ実施しない
#define	TUNING_YES			1				// ﾁｭｰﾆﾝｸﾞ実施する

#define	RESULT_NOW			0				// 結果格納領域 現在
#define	RESULT_MAX			1				// 結果格納領域 最大
#define	RESULT_MIN			2				// 結果格納領域 最小

#define	RESULT_AREA_MAX		5				// 結果ｴﾘｱ数(306～314)

#define	PEAKHOLD_OFF		0				// ﾋﾟｰｸﾎｰﾙﾄﾞ OFF(しない)
#define	PEAKHOLD_ON			1				// ﾋﾟｰｸﾎｰﾙﾄﾞ ON(する)
#define	PEAKHOLD_5S			2				// ﾋﾟｰｸﾎｰﾙﾄﾞ  5sec
#define	PEAKHOLD_10S		3				// ﾋﾟｰｸﾎｰﾙﾄﾞ 10sec

#define	SPINDLE_SPEED_MIN	20				// 回転数下限
#define	SPINDLE_SPEED_MAX	150				// 回転数上限


#define	ARGUMENT_ERROR		-1				// 引数ｴﾗｰ

// 方向種別
#define	X_DIRECTION			0				// X方向
#define	Z_DIRECTION			1				// Z方向

// ハード種別
#define	PORTABLE_EDITION	0				// ﾎﾟｰﾀﾌﾞﾙ版
#define	CNC_EDITION			1				// CNC版

// 電源種別
#define	POWER_SUPPLY_ADAPTER	0			// 外部電源(アダプタ)
#define	POWER_SUPPLY_BATTERY	1			// バッテリー(電池)

// バッテリー動作状況
#define	PORTABLE_DRIVE_NORMAL	0			// 通常(シャッタ通常)
#define	PORTABLE_DRIVE_ECO		1			// ECO(シャッタ低速)

#define	BATTERY_MAX				543			// 9.0V
#define	BATTERY_WARNING			456			// 7.6V(バッテリー警告＠電池残量警告)
#define	BATTERY_ERROR			418			// 7.0V(バッテリーエラー＠計測不可)
#define	BATTERY_MIN_DISP		443			// 7.4V(LED発光に伴い0.4Vの電圧損失が発生するため)

// 計測開始の司令方法
#define	OPERATION_AUTO		0				// 外部司令（タッチパネル）
#define	OPERATION_MANUAL	1				// 手動操作（Start-SW）

// ピークホールド
#define	PEAK_HOLD_OFF		0				// 0:Off
#define	PEAK_HOLD_ON		1				// 1:On
#define	PEAK_HOLD_5S		2				// 2:5s
#define	PEAK_HOLD_10S		3				// 3:10s

// 検出結果
#define	RESULT_NO			0				// 結果なし
#define	RESULT_YES			1				// 結果あり

// 計測状態
#define	MEASURE_STOP		0				// 停止中
#define	MEASURE_RUNNING		1				// 実行中

// add 2015.12.22 K.Uemura start	FB2201
// カバー状態
#define	COVER_STATUS_MOVED				0		// 00 移動中（応答なし）
#define	COVER_STATUS_OPEN				1		// 01 OPEN
#define	COVER_STATUS_CLOSE				2		// 10 CLOSE
#define	COVER_STATUS_BREAK				3		// 11 故障（CLOSE／OPEN）
// add 2015.12.22 K.Uemura end

// add 2016.02.18 K.Uemura start	G21804
#define	SENSOR_INITIAL_BRIGHT			900		// 初期化輝度
												// ※CNC版のみ。ﾎﾟｰﾀﾌﾞﾙ版はpower_sequence.c内で記述
// add 2016.02.18 K.Uemura end
#define	SENSOR_INITIAL_BRIGHT2			500		// 初期化輝度

// add 2017.02.09 K.Uemura start	H20901
#define	SENSOR_OUT_LEVEL_BRIGHT			500		// ｾﾝｻ出力ﾚﾍﾞﾙ用輝度
// add 2017.02.09 K.Uemura end


// ｴﾗｰ番号		////////////////////////////////////////////////////////////////
#define	ERR_NONE						0		// 0	ｴﾗｰなし

#define	ERR_UNDERVOLTAGE				100		// 100	電圧不足
#define	ERR_PARAMETER_VERIFY			200		// 200	verifyｴﾗｰ(FPGAﾊﾟﾗﾒｰﾀが不正)

//※Z方向は+100する
#define	ERR_BLACK_TIMEOUT				1001	// 1001	BLACKﾀｲﾑｱｳﾄ(FPGAとのﾃﾞｰﾀ送受信)
#define	ERR_WHITE_TIMEOUT				1002	// 1002	WHITEﾀｲﾑｱｳﾄ(FPGAとのﾃﾞｰﾀ送受信)
#define	ERR_BRIGHTNESS_TIMEOUT			1003	// 1003	調光ﾀｲﾑｱｳﾄ(FPGAとのﾃﾞｰﾀ送受信)
#define	ERR_WHITE_SETTING_TIMEOUT		1004	// 1004	WHITE設定ﾀｲﾑｱｳﾄ(FPGAとのﾃﾞｰﾀ送受信)

#define	ERR_BLACK_NG					1010	// 1011	BLACK値NG
#define	ERR_WHITE_NG					1020	// 1012	WHITE値NG
#define	ERR_BRIGHTNESS_NG				1030	// 1013	調光値NG リトライ
#define	ERR_BRIGHTNESS_UPPER			1031	// 1014	調光値NG 上限到達(輝度上げれない)
#define	ERR_BRIGHTNESS_LOWER			1032	// 1015	調光値NG 下限到達(輝度下けれない)
#define	ERR_NOLED						1200	// 1200 LED点灯しない
// add 2016.02.18 K.Uemura start	G21802
#define	ERR_INITIALIZE					1300	// 1300 初期化未設定
// add 2016.02.18 K.Uemura end
// add 2016.06.22 K.Uemura start	G62202
#define	ERR_SENSOR_LEVEL				1400	// 1400 ｾﾝｻ出力ﾚﾍﾞﾙが低い(DLPM055に達しない)
// add 2016.06.22 K.Uemura end

#define	ERR_TP_TIMEOUT					2300	// 2300	ﾀｯﾁﾊﾟﾈﾙ通信のﾀｲﾑｱｳﾄ

// add 2015.12.22 K.Uemura start	FB2201
#define	ERR_COVER_TIMEOUT				2500	// 2500	カバー開閉の指令後、センサ完了状態に遷移しない
#define	ERR_COVER_BROKEN				2501	// 2501	センサ故障
#define	ERR_COVER_MOVED					2502	// 2502	センサ移動中
												// 		カバー指令コマンドとカバーの状態が一致しない
#define	ERR_COVER_OPEN					2503	// 2503	カバーOPEN状態だがOPEN信号が出力されていない
#define	ERR_COVER_CLOSE					2504	// 2504	カバーCLOSE状態だがCLOSE信号が出力されていない
// add 2015.12.22 K.Uemura end

#define	ERR_NOMODE						3000	// 3000	存在しない動作モード

#define	ERR_ZERO_SPINDLE_SPEED			3100	// 6000	「振れ」のとき回転数「0」
#define	ERR_OUT_OF_SPINDLE_SPEED		3101	// 6001	「振れ」のとき回転数範囲外
#define	ERR_OUT_OF_FLUTES				3102	// 6002	「振れ」のとき刃数範囲外

// add 2016.03.24 K.Uemura start	G32401
#define	ERR_MEASURE_START				5000	// 5000 測定開始できない
// add 2016.03.24 K.Uemura end
#define	ERR_ONE_EDGE					5004	// 5004	自動判別ﾓｰﾄﾞでどちらか一方ｴｯｼﾞがあり判別不可能

#define	ERR_NO_EDGE						5005	// 5005	自動判別ﾓｰﾄﾞでｴｯｼﾞがなく判別不可能
#define	ERR_BOTH_EDGE					5006	// 5006	自動判別ﾓｰﾄﾞで両方ｴｯｼﾞがあり判別不可能
#define	ERR_TUNING_NO_RESULT			5007	// 5007	ﾁｭｰﾆﾝｸﾞ結果なし
#define	ERR_TRRIGER_PEAK				5008	// 5008	TIRﾄﾘｶﾞﾋﾟｰｸが得られない
// add 2016.03.08 K.Uemura start	G30802
#define	ERR_NO_RESULT					5009	// 5009	測定結果が得られない
// add 2016.03.08 K.Uemura end
#define	ERR_END_TIMEOUT_PERIOD			5010	// 5010	ﾀｲﾑｱｳﾄ設定時間に到達
#define	ERR_NG_COUNT					5011	// 5011	連続NGｶｳﾝﾄ数に到達

#define	ERR_OUT_OF_PARAMETER			9001	// 9001	ﾊﾟﾗﾒｰﾀ範囲外

////////////////////////////////////////////////////////////////////////////////

// 読出値

// TPDﾊﾟﾗﾒｰﾀ	////////////////////////////////////////////////////////////////
// 初期値
// 計測設定
// 共通
#define INIT_START_DELAY_TIME			para_ini[33]		// 計測開始遅延時間
#define INIT_SPINDLE_SPEED				para_ini[34]		// 回転数
#define INIT_FLUTES						para_ini[35]		// 刃数
////////////////////////////////////////////////////////////////////////////////

// TPDﾊﾟﾗﾒｰﾀ	////////////////////////////////////////////////////////////////
// 設定
// 露光時間
#define X_EXPOSURE_TIME					SEQ.ALL_DATA[1]		// 露光時間(X)
#define ECO_EXPOSURE_TIME				SEQ.ALL_DATA[2]		// 露光時間(ECO)
#define Z_EXPOSURE_TIME					SEQ.ALL_DATA[3]		// 露光時間(Z)

// add 2016.02.18 K.Uemura start	G21804
#define X_DIRTY_BASE_AVE				SEQ.ALL_DATA[4]		// 汚れﾁｪｯｸ基準平均(X)
#define X_DIRTY_BASE_MIN				SEQ.ALL_DATA[5]		// 汚れﾁｪｯｸ基準最小(X)
#define X_DIRTY_BASE_MAX				SEQ.ALL_DATA[6]		// 汚れﾁｪｯｸ基準最大(X)
#define Z_DIRTY_BASE_AVE				SEQ.ALL_DATA[7]		// 汚れﾁｪｯｸ基準平均(Z)
#define Z_DIRTY_BASE_MIN				SEQ.ALL_DATA[8]		// 汚れﾁｪｯｸ基準最小(Z)
#define Z_DIRTY_BASE_MAX				SEQ.ALL_DATA[9]		// 汚れﾁｪｯｸ基準最大(Z)
// add 2016.02.18 K.Uemura end

// 初期ﾁｪｯｸ
// X
#define X_BLACK_CHECK					SEQ.ALL_DATA[10]	// BLACKﾁｪｯｸ(X)
#define X_WHITE_CHECK					SEQ.ALL_DATA[11]	// WHITEﾁｪｯｸ(X)
#define X_SENSOR_CHECK_LOWER			SEQ.ALL_DATA[12]	// ﾗｲﾝｾﾝｻﾁｪｯｸ下限(X)
#define X_SENSOR_CHECK_UPPER			SEQ.ALL_DATA[13]	// ﾗｲﾝｾﾝｻﾁｪｯｸ上限(X)
#define X_LDV_TARGET					SEQ.ALL_DATA[14]	// LDVﾀｰｹﾞｯﾄ(X)
#define X_LDV_RANGE						SEQ.ALL_DATA[15]	// LDV有効範囲(X)
#define X_RETRY_COUNT					SEQ.ALL_DATA[16]	// LDVﾘﾄﾗｲ回数(X)
// Z
#define Z_BLACK_CHECK					SEQ.ALL_DATA[20]	// BLACKﾁｪｯｸ(Z)
#define Z_WHITE_CHECK					SEQ.ALL_DATA[21]	// WHITEﾁｪｯｸ(Z)
#define Z_SENSOR_CHECK_LOWER			SEQ.ALL_DATA[22]	// ﾗｲﾝｾﾝｻﾁｪｯｸ下限(Z)
#define Z_SENSOR_CHECK_UPPER			SEQ.ALL_DATA[23]	// ﾗｲﾝｾﾝｻﾁｪｯｸ上限(Z)
#define Z_LDV_TARGET					SEQ.ALL_DATA[24]	// LDVﾀｰｹﾞｯﾄ(Z)
#define Z_LDV_RANGE						SEQ.ALL_DATA[25]	// LDV有効範囲(Z)
#define Z_RETRY_COUNT					SEQ.ALL_DATA[26]	// LDVﾘﾄﾗｲ回数(Z)
// NORMAL
#define NORMAL_BLACK_CHECK				SEQ.ALL_DATA[30]	// BLACKﾁｪｯｸ(NORMAL)
#define NORMAL_WHITE_CHECK				SEQ.ALL_DATA[31]	// WHITEﾁｪｯｸ(NORMAL)
#define NORMAL_SENSOR_CHECK_LOWER		SEQ.ALL_DATA[32]	// ﾗｲﾝｾﾝｻﾁｪｯｸ下限(NORMAL)
#define NORMAL_SENSOR_CHECK_UPPER		SEQ.ALL_DATA[33]	// ﾗｲﾝｾﾝｻﾁｪｯｸ上限(NORMAL)
#define NORMAL_LDV_TARGET				SEQ.ALL_DATA[34]	// LDVﾀｰｹﾞｯﾄ(NORMAL)
#define NORMAL_LDV_RANGE				SEQ.ALL_DATA[35]	// LDV有効範囲(NORMAL)
#define NORMAL_RETRY_COUNT				SEQ.ALL_DATA[36]	// LDVﾘﾄﾗｲ回数(NORMAL)
// ECO
#define ECO_BLACK_CHECK					SEQ.ALL_DATA[40]	// BLACKﾁｪｯｸ(ECO)
#define ECO_WHITE_CHECK					SEQ.ALL_DATA[41]	// WHITEﾁｪｯｸ(ECO)
#define ECO_SENSOR_CHECK_LOWER			SEQ.ALL_DATA[42]	// ﾗｲﾝｾﾝｻﾁｪｯｸ下限(ECO)
#define ECO_SENSOR_CHECK_UPPER			SEQ.ALL_DATA[43]	// ﾗｲﾝｾﾝｻﾁｪｯｸ上限(ECO)
#define ECO_LDV_TARGET					SEQ.ALL_DATA[44]	// LDVﾀｰｹﾞｯﾄ(ECO)
#define ECO_LDV_RANGE					SEQ.ALL_DATA[45]	// LDV有効範囲(ECO)
#define ECO_RETRY_COUNT					SEQ.ALL_DATA[46]	// LDVﾘﾄﾗｲ回数(ECO)

// 計測設定
// 共通
#define BRIGHTNESS_ADJUSTMENT			SEQ.ALL_DATA[50]	// LED輝度調整
#define MANUAL_BRIGHTNESS				SEQ.ALL_DATA[51]	// LED輝度値
#define PARAM_START_DELAY_TIME			SEQ.ALL_DATA[52]	// 計測開始遅延時間
#define PARAM_SPINDLE_SPEED				SEQ.ALL_DATA[53]	// 回転数
#define PARAM_FLUTES					SEQ.ALL_DATA[54]	// 刃数
// add 2016.06.22 K.Uemura start	G62202
#define SENSOR_LEVEL					SEQ.ALL_DATA[55]	// 清掃確認ﾚﾍﾞﾙ
// add 2016.06.22 K.Uemura end
// X
#define X_IDLE_COUNT					SEQ.ALL_DATA[60]	// FPGAｱｲﾄﾞﾙ数(ﾎﾟｰﾘﾝｸﾞ周期[DLPM156]×COUNT)
#define X_BLACK_RATIO_LARGE				SEQ.ALL_DATA[61]	// 黒比率傾斜大ﾚﾍﾞﾙ
#define X_BLACK_RATIO_SMALL				SEQ.ALL_DATA[62]	// 黒比率傾斜小ﾚﾍﾞﾙ
#define X_SKIP_COEFFICIENT				SEQ.ALL_DATA[63]	// SKIP係数
#define X_ROUGH_EDGE_COEFFICIENT		SEQ.ALL_DATA[64]	// 荒ｴｯｼﾞｻｰﾁ係数
#define X_TUNING_ENABLE					SEQ.ALL_DATA[65]	// 傾斜ﾁｭｰﾆﾝｸﾞ有無
#define X_TUNING_SECONDS				SEQ.ALL_DATA[66]	// ﾁｭｰﾆﾝｸﾞ秒数
#define X_TUNING_OFFSET					SEQ.ALL_DATA[67]	// ﾁｭｰﾆﾝｸﾞｵﾌｾｯﾄ
#define X_TUNING_RATIO_LOWER_LIMIT		SEQ.ALL_DATA[68]	// ﾁｭｰﾆﾝｸﾞ比率下限
#define X_TUNING_RATIO_UPPER_LIMIT		SEQ.ALL_DATA[69]	// ﾁｭｰﾆﾝｸﾞ比率上限
#define X_MANUAL_GRADIENT				SEQ.ALL_DATA[70]	// 手動傾斜画素
#define X_FOCUSING_GRADIENT				SEQ.ALL_DATA[71]	// 焦点合わせ傾斜
#define X_ORIGIN_THRESHOLD				SEQ.ALL_DATA[72]	// ORIGIN(ｴｯｼﾞ考慮)しきい値
#define X_RATIO_SWING					SEQ.ALL_DATA[73]	// 振れ比率
// Z
#define Z_IDLE_COUNT					SEQ.ALL_DATA[80]	// FPGAｱｲﾄﾞﾙ数(ﾎﾟｰﾘﾝｸﾞ周期[DLPM156]×COUNT)
#define Z_BLACK_RATIO_LARGE				SEQ.ALL_DATA[81]	// 黒比率傾斜大ﾚﾍﾞﾙ
#define Z_BLACK_RATIO_SMALL				SEQ.ALL_DATA[82]	// 黒比率傾斜小ﾚﾍﾞﾙ
#define Z_SKIP_COEFFICIENT				SEQ.ALL_DATA[83]	// SKIP係数
#define Z_ROUGH_EDGE_COEFFICIENT		SEQ.ALL_DATA[84]	// 荒ｴｯｼﾞｻｰﾁ係数
#define Z_TUNING_ENABLE					SEQ.ALL_DATA[85]	// 傾斜ﾁｭｰﾆﾝｸﾞ有無
#define Z_TUNING_SECONDS				SEQ.ALL_DATA[86]	// ﾁｭｰﾆﾝｸﾞ秒数
#define Z_TUNING_OFFSET					SEQ.ALL_DATA[87]	// ﾁｭｰﾆﾝｸﾞｵﾌｾｯﾄ
#define Z_TUNING_RATIO_LOWER_LIMIT		SEQ.ALL_DATA[88]	// ﾁｭｰﾆﾝｸﾞ比率下限
#define Z_TUNING_RATIO_UPPER_LIMIT		SEQ.ALL_DATA[89]	// ﾁｭｰﾆﾝｸﾞ比率上限
#define Z_MANUAL_GRADIENT				SEQ.ALL_DATA[90]	// 手動傾斜画素
#define Z_FOCUSING_GRADIENT				SEQ.ALL_DATA[91]	// 焦点合わせ傾斜
#define Z_ORIGIN_THRESHOLD				SEQ.ALL_DATA[92]	// ORIGIN(ｴｯｼﾞ考慮)しきい値
// chg 2016.06.22 K.Uemura start	G62201
#define Z_RATIO_SWING					SEQ.ALL_DATA[93]	// 振れ比率
// chg 2016.06.22 K.Uemura end

// 換算TBL
// X
#define X_DIVISION_NUMBER				SEQ.ALL_DATA[100]	// 分割数(X)
#define X_CORRECTION_INTERVAL			SEQ.ALL_DATA[101]	// 補正間隔(X)
#define X_AVERAGE_NUMBER				SEQ.ALL_DATA[102]	// 平均数(X)
// Z
#define Z_DIVISION_NUMBER				SEQ.ALL_DATA[103]	// 分割数(Z)
#define Z_CORRECTION_INTERVAL			SEQ.ALL_DATA[104]	// 補正間隔(Z)
#define Z_AVERAGE_NUMBER				SEQ.ALL_DATA[105]	// 平均数(Z)
// ﾎﾟｰﾀﾌﾞﾙ
#define PORTABLE_DIVISION_NUMBER		SEQ.ALL_DATA[106]	// 分割数(ﾎﾟｰﾀﾌﾞﾙ)
#define PORTABLE_CORRECTION_INTERVAL	SEQ.ALL_DATA[107]	// 補正間隔(ﾎﾟｰﾀﾌﾞﾙ)
#define PORTABLE_AVERAGE_NUMBER			SEQ.ALL_DATA[108]	// 平均数(ﾎﾟｰﾀﾌﾞﾙ)

// 通信設定
#define COM_DEVICE						SEQ.ALL_DATA[150]	// ﾃﾞﾊﾞｲｽ
#define COM_TOP_ADDRESS					SEQ.ALL_DATA[151]	// 通信TOP(CNC)
#define BUF_TOP_ADDRESS					SEQ.ALL_DATA[152]	// ﾊﾞｯﾌｧTOP(CNC)
#define BUFFER_NUMBER					SEQ.ALL_DATA[153]	// ﾊﾞｯﾌｧ数
#define COM_TIMEOUT_PERIOD_SET			SEQ.ALL_DATA[154]	// ﾀｲﾑｱｳﾄ時間ｾｯﾄ(通信設定)
#define STROBE_ON_TIME					SEQ.ALL_DATA[155]	// STROBE信号ON時間
#define POLLING_CYCLE					SEQ.ALL_DATA[156]	// ﾎﾟｰﾘﾝｸﾞ周期
#define COVER_DRIVE_TIME				SEQ.ALL_DATA[160]	// ｶﾊﾞｰｾﾝｻ監視時間
#define COVER_CONTROL					SEQ.ALL_DATA[161]	// ｶﾊﾞｰ制御

// SKIP
// 出力設定
#define SKIP_ENABLE						SEQ.ALL_DATA[200]	// ｽｷｯﾌﾟ
#define SKIP_SIGNAL_REVERSE				SEQ.ALL_DATA[201]	// ｽｷｯﾌﾟ信号反転
#define SKIP_RETENTION_TIME				SEQ.ALL_DATA[202]	// ｽｷｯﾌﾟ保持時間
// chg 2016.03.11 K.Uemura start	G31101
#define SKIP_DELAY_TIME					SEQ.ALL_DATA[203]	// ｽｷｯﾌﾟ遅延時間
// chg 2016.03.11 K.Uemura end
// add 2016.12.08 K.Uemura start	
#define ERR_SIGNAL_REVERSE				SEQ.ALL_DATA[204]	// ｴﾗｰ信号反転
// add 2016.12.08 K.Uemura end
// ORIGIN-X
#define X_HDI0_START					SEQ.ALL_DATA[210]	// HDI0 開始
#define X_HDI0_END						SEQ.ALL_DATA[211]	// HDI0 開始
#define X_HDI1_START					SEQ.ALL_DATA[212]	// HDI1 開始
#define X_HDI1_END						SEQ.ALL_DATA[213]	// HDI1 終了
#define X_HDI2_START					SEQ.ALL_DATA[214]	// HDI2 開始
#define X_HDI2_END						SEQ.ALL_DATA[215]	// HDI2 終了
#define X_HDI3_START					SEQ.ALL_DATA[216]	// HDI3 開始
#define X_HDI3_END						SEQ.ALL_DATA[217]	// HDI3 終了
#define X_HDI4_START					SEQ.ALL_DATA[218]	// HDI4 開始
#define X_HDI4_END						SEQ.ALL_DATA[219]	// HDI4 終了
#define X_HDI5_START					SEQ.ALL_DATA[220]	// HDI5 開始
#define X_HDI5_END						SEQ.ALL_DATA[221]	// HDI5 終了
// ORIGIN-Z
#define Z_HDI0_START					SEQ.ALL_DATA[230]	// HDI0 開始
#define Z_HDI0_END						SEQ.ALL_DATA[231]	// HDI0 開始
#define Z_HDI1_START					SEQ.ALL_DATA[232]	// HDI1 開始
#define Z_HDI1_END						SEQ.ALL_DATA[233]	// HDI1 終了
#define Z_HDI2_START					SEQ.ALL_DATA[234]	// HDI2 開始
#define Z_HDI2_END						SEQ.ALL_DATA[235]	// HDI2 終了
#define Z_HDI3_START					SEQ.ALL_DATA[236]	// HDI3 開始
#define Z_HDI3_END						SEQ.ALL_DATA[237]	// HDI3 終了
#define Z_HDI4_START					SEQ.ALL_DATA[238]	// HDI4 開始
#define Z_HDI4_END						SEQ.ALL_DATA[239]	// HDI4 終了
#define Z_HDI5_START					SEQ.ALL_DATA[240]	// HDI5 開始
#define Z_HDI5_END						SEQ.ALL_DATA[241]	// HDI5 終了
// 焦点合わせ-X
#define HDI0_SKIP_L						SEQ.ALL_DATA[250]	// HDI0ｽｷｯﾌﾟしきい値(L)
#define HDI0_SKIP_R						SEQ.ALL_DATA[251]	// HDI0ｽｷｯﾌﾟしきい値(R)
#define HDI1_SKIP_L						SEQ.ALL_DATA[252]	// HDI1ｽｷｯﾌﾟしきい値(L)
#define HDI1_SKIP_R						SEQ.ALL_DATA[253]	// HDI1ｽｷｯﾌﾟしきい値(R)
#define HDI2_SKIP_L						SEQ.ALL_DATA[254]	// HDI2ｽｷｯﾌﾟしきい値(L)
#define HDI2_SKIP_R						SEQ.ALL_DATA[255]	// HDI2ｽｷｯﾌﾟしきい値(R)
#define HDI3_SKIP_L						SEQ.ALL_DATA[256]	// HDI3ｽｷｯﾌﾟしきい値(L)
#define HDI3_SKIP_R						SEQ.ALL_DATA[257]	// HDI3ｽｷｯﾌﾟしきい値(R)
#define HDI4_SKIP_L						SEQ.ALL_DATA[258]	// HDI4ｽｷｯﾌﾟしきい値(L)
#define HDI4_SKIP_R						SEQ.ALL_DATA[259]	// HDI4ｽｷｯﾌﾟしきい値(R)
#define HDI5_SKIP_L						SEQ.ALL_DATA[260]	// HDI5ｽｷｯﾌﾟしきい値(L)
#define HDI5_SKIP_R						SEQ.ALL_DATA[261]	// HDI5ｽｷｯﾌﾟしきい値(R)
// 焦点合わせ-Z
#define Z_HDI0_SKIP_R					SEQ.ALL_DATA[270]	// HDI0ｽｷｯﾌﾟしきい値
#define Z_HDI1_SKIP_R					SEQ.ALL_DATA[271]	// HDI1ｽｷｯﾌﾟしきい値
#define Z_HDI2_SKIP_R					SEQ.ALL_DATA[272]	// HDI2ｽｷｯﾌﾟしきい値
#define Z_HDI3_SKIP_R					SEQ.ALL_DATA[273]	// HDI3ｽｷｯﾌﾟしきい値
#define Z_HDI4_SKIP_R					SEQ.ALL_DATA[274]	// HDI4ｽｷｯﾌﾟしきい値
#define Z_HDI5_SKIP_R					SEQ.ALL_DATA[275]	// HDI5ｽｷｯﾌﾟしきい値

// 表示/出力
// 表示
#define RESULT_SIGN						SEQ.ALL_DATA[301]	// 符号
#define DIAMETER_OUTPUT					SEQ.ALL_DATA[302]	// 出力Φ
#define DIAMETER_OUTPUT_TPD				SEQ.ALL_DATA[303]	// 出力Φ(TPD)
#define LANGUAGE						SEQ.ALL_DATA[304]	// 言語
// 伸び
#define GROWTH_MEASUREMENT_TIME			SEQ.ALL_DATA[310]	// 伸び計測時間
#define GROWTH_DRAWING_PERIOD			SEQ.ALL_DATA[311]	// 伸びｸﾞﾗﾌ描画周期
#define GROWTH_STARTING_POINT			SEQ.ALL_DATA[312]	// 伸び計測始点
// 焦点合わせ
#define FOCUS_MEMORY_TIME_SET			SEQ.ALL_DATA[313]	// 焦点記憶時間
// 中心位置設定
#define CENTER_MEMORY_TIME_SET			SEQ.ALL_DATA[314]	// 中心位置設定記憶時間
// chg 2016.12.06 K.Uemura start	GC0602
#define ROUGH_SCAN_RATIO				SEQ.ALL_DATA[320]	// 粗測定ﾊﾟﾗﾒｰﾀ
// chg 2016.12.06 K.Uemura end

// LED点灯範囲
// X
#define X_LED1_START					SEQ.ALL_DATA[400]	// LED1 開始
#define X_LED1_END						SEQ.ALL_DATA[401]	// LED1 終了
#define X_LED2_START					SEQ.ALL_DATA[402]	// LED2 開始
#define X_LED2_END						SEQ.ALL_DATA[403]	// LED2 終了
#define X_LED3_START					SEQ.ALL_DATA[404]	// LED3 開始
#define X_LED3_END						SEQ.ALL_DATA[405]	// LED3 終了
#define X_LED4_START					SEQ.ALL_DATA[406]	// LED4 開始
#define X_LED4_END						SEQ.ALL_DATA[407]	// LED4 終了
#define X_LED5_START					SEQ.ALL_DATA[408]	// LED5 開始
#define X_LED5_END						SEQ.ALL_DATA[409]	// LED5 終了
#define X_LED6_START					SEQ.ALL_DATA[410]	// LED6 開始
#define X_LED6_END						SEQ.ALL_DATA[411]	// LED6 終了
#define X_LED7_START					SEQ.ALL_DATA[412]	// LED7 開始
#define X_LED7_END						SEQ.ALL_DATA[413]	// LED7 終了
#define X_LED8_START					SEQ.ALL_DATA[414]	// LED8 開始
#define X_LED8_END						SEQ.ALL_DATA[415]	// LED8 終了
#define X_LED9_START					SEQ.ALL_DATA[416]	// LED9 開始
#define X_LED9_END						SEQ.ALL_DATA[417]	// LED9 終了
// Z
#define Z_LED1_START					SEQ.ALL_DATA[430]	// LED1 開始
#define Z_LED1_END						SEQ.ALL_DATA[431]	// LED1 終了
#define Z_LED2_START					SEQ.ALL_DATA[432]	// LED2 開始
#define Z_LED2_END						SEQ.ALL_DATA[433]	// LED2 終了
#define Z_LED3_START					SEQ.ALL_DATA[434]	// LED3 開始
#define Z_LED3_END						SEQ.ALL_DATA[435]	// LED3 終了
#define Z_LED4_START					SEQ.ALL_DATA[436]	// LED4 開始
#define Z_LED4_END						SEQ.ALL_DATA[437]	// LED4 終了
// ﾎﾟｰﾀﾌﾞﾙ
#define LED1_START						SEQ.ALL_DATA[460]	// LED1 開始
#define LED1_END						SEQ.ALL_DATA[461]	// LED1 終了
#define LED2_START						SEQ.ALL_DATA[462]	// LED2 開始
#define LED2_END						SEQ.ALL_DATA[463]	// LED2 終了
#define LED3_START						SEQ.ALL_DATA[464]	// LED3 開始
#define LED3_END						SEQ.ALL_DATA[465]	// LED3 終了
#define LED4_START						SEQ.ALL_DATA[466]	// LED4 開始
#define LED4_END						SEQ.ALL_DATA[467]	// LED4 終了
#define LED5_START						SEQ.ALL_DATA[468]	// LED5 開始
#define LED5_END						SEQ.ALL_DATA[469]	// LED5 終了
#define LED6_START						SEQ.ALL_DATA[470]	// LED6 開始
#define LED6_END						SEQ.ALL_DATA[471]	// LED6 終了
#define LED7_START						SEQ.ALL_DATA[472]	// LED7 開始
#define LED7_END						SEQ.ALL_DATA[473]	// LED7 終了
#define LED8_START						SEQ.ALL_DATA[474]	// LED8 開始
#define LED8_END						SEQ.ALL_DATA[475]	// LED8 終了
#define LED9_START						SEQ.ALL_DATA[476]	// LED9 開始
#define LED9_END						SEQ.ALL_DATA[477]	// LED9 終了
#define LED10_START						SEQ.ALL_DATA[478]	// LED10 開始
#define LED10_END						SEQ.ALL_DATA[479]	// LED10 終了
#define LED11_START						SEQ.ALL_DATA[480]	// LED11 開始
#define LED11_END						SEQ.ALL_DATA[481]	// LED11 終了

// 終了条件
// ﾀｲﾑｱｳﾄ時間
// OKｶｳﾝﾄ数
// 連続NGｶｳﾝﾄ数

////////////////////////////////////////////////////////////////////////////////

//出力定義
//ﾊﾟﾈﾙLED定義
#define OUT_SEG_0			0x3F					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「0」
#define OUT_SEG_1			0x06					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「1」
#define OUT_SEG_2			0x5B					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「2」
#define OUT_SEG_3			0x4F					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「3」
#define OUT_SEG_4			0x66					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「4」
#define OUT_SEG_5			0x6D					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「5」
#define OUT_SEG_6			0x7D					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「6」
#define OUT_SEG_7			0x27					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「7」
#define OUT_SEG_8			0x7F					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「8」
#define OUT_SEG_9			0x6F					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「9」

#define OUT_SEG_0_DOT		0xBF					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「0.」
#define OUT_SEG_1_DOT		0x86					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「1.」
#define OUT_SEG_2_DOT		0xDB					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「2.」
#define OUT_SEG_3_DOT		0xCF					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「3.」
#define OUT_SEG_4_DOT		0xE6					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「4.」
#define OUT_SEG_5_DOT		0xED					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「5.」
#define OUT_SEG_6_DOT		0xFD					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「6.」
#define OUT_SEG_7_DOT		0xA7					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「7.」
#define OUT_SEG_8_DOT		0xFF					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「8.」
#define OUT_SEG_9_DOT		0xEF					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「9.」

#define OUT_SEG_A			0x77					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「A」
#define OUT_SEG_b			0x7C					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「b」
#define OUT_SEG_C			0x39					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「C」
#define OUT_SEG_d			0x5E					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「d」
#define OUT_SEG_E			0x79					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「E」
#define OUT_SEG_F			0x71					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「F」

#define OUT_SEG_H			0x76					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「H」
#define OUT_SEG_i			0x04					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「i」

#define OUT_SEG_L			0x38					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「L」
#define OUT_SEG_M			0x40					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「M」
#define OUT_SEG_n			0x54					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「n」
#define OUT_SEG_o			0x5C					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「o」
#define OUT_SEG_P			0x73					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「p」

#define OUT_SEG_r			0x50					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「r」

#define OUT_SEG_U			0x3E					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「U」

#define OUT_SEG_MI			0x40					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「-」
#define OUT_SEG_DOT			0x80					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「.」
#define OUT_SEG_BL			0x00					// ｾｸﾞﾒﾝﾄ出力ﾃﾞｰﾀ「 」

#define	OUT_SEG_NO1			0x01					// Z方向走査「-」の縦スクロール  
#define	OUT_SEG_NO2			0x41
#define	OUT_SEG_NO3			0x49

#define RESET_MODE			5U						// ﾘｾｯﾄﾓｰﾄﾞ
#define INIT_START_MODE		10U						// 初期ﾓｰﾄﾞ
#define DRV_MODE			20U						// 通常ﾓｰﾄﾞ
#define IDLE_MODE			30U						// 待機ﾓｰﾄﾞ
#define READ_MODE			40U						// 読み出しﾓｰﾄﾞ
#define WRITE_MODE			50U						// 書き込みﾓｰﾄﾞ
#define	DEBUG_MODE			100U					// ﾃﾞﾊﾞｯｸﾞﾓｰﾄﾞ

//EEPROM関係定義
//書込用
#define MEM_DATA_WR			0xA0					// ﾒﾓﾘｰﾗｲﾄｺﾏﾝﾄﾞ
//読出用
#define MEM_DATA_RE			0xA1					// ﾒﾓﾘｰﾘｰﾄﾞｺﾏﾝﾄﾞ

//書込済み確認用数値
#define MEM_INITIAL			0x07					//書込済み確認用数値(v201:0x07)
//#define MEM_INITIAL		0x00					//書込済み確認用数値(v100:0x00)
//#define MEM_INITIAL		0x01					//書込済み確認用数値(v103:0x01)
//#define MEM_INITIAL		0x02					//書込済み確認用数値(v104:0x02)
//#define MEM_INITIAL		0x03					//書込済み確認用数値(v105:0x03)
//#define MEM_INITIAL		0x04					//書込済み確認用数値(v107:0x04)
//#define MEM_INITIAL		0x05					//書込済み確認用数値(v110:0x05)
//#define MEM_INITIAL		0x06					//書込済み確認用数値(v200:0x06)


#define	OUTPUT232C									// 232C出力

//#define	__DEBUG										// デバッグ動作を行う場合、有効にする

#ifdef	__DEBUG
//#define	DEBUG_EACHSCAN								// 毎回結果(0.44msごと)
#define	DEBUG_OUTPUT232C							// デバッグ出力

//#define	__LOGGING									// ロギングを行う場合、有効にする
//#define	___LOGGING_VOLTAGE_RESULT					// 電圧と計測結果
//#define	___LOGGING_INTERVAL_RESET					// 一定周期のリセット
//#define	__INITIAL_SEQUENCE							// 起動時出力
#define	__LONGGING_SENSOR_LEVEL						// センサ出力レベル

#ifdef	__LOGGING
#define	DEBUG_OUTPUT_1SEC							// 1sec出力
//#define	DEBUG_OUTPUT_100MSEC						// 100msec出力
#endif

#ifdef	___LOGGING_INTERVAL_RESET
#define	DEBUG_OUTPUT_1SEC							// 1sec出力
#define	DEBUG_AUTOSTART								// 再計測
#define	DEBUG_ONTIME		25						// ON時間
#define	DEBUG_OFFTIME		5						// OFF時間
#endif

#endif
