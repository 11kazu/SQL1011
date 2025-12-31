/******************************************************************************
* File Name	: user.h
******************************************************************************/

/****************************/
/*		グローバル変数定義	*/
/****************************/

/*	ﾒｲﾝｼｰｹﾝｽ制御用構造体	*/
struct {
		union{
			unsigned short	WORD;
			struct{
				unsigned short				:4;
				unsigned short	TP_SHOT		:1;			// ﾀｯﾁﾊﾟﾈﾙ関数ﾌﾗｸﾞ
				unsigned short	F_SHOT		:1;			// FPGA関数ﾌﾗｸﾞ
				unsigned short	R_SHOT		:1;			// ROM関数処理
				unsigned short	I_SHOT		:1;			// 入力関数ﾌﾗｸﾞ
				unsigned short	A_SHOT		:1;			// ｱﾅﾛｸﾞ入力関数ﾌﾗｸﾞ
				unsigned short	O_SHOT		:1;			// 出力関数ﾌﾗｸﾞ
				unsigned short	T_SHOT		:1;			// ﾀｲﾏｰ関数ﾌﾗｸﾞ
				unsigned short	M25_SHOT	:1;			// M25関数ﾌﾗｸﾞ
				unsigned short	HDI_SHOT	:1;			// HDI関数ﾌﾗｸﾞ
				unsigned short	DEB_SHOT	:1;			// ﾃﾞﾊﾞｯｸﾞ出力関数ﾌﾗｸﾞ
				unsigned short	SEC_SHOT	:1;			// 1sﾌﾗｸﾞ
				unsigned short	MSEC_SHOT	:1;			// 1msﾌﾗｸﾞ
				}	BIT;
			}	FLAG1;

		union{
			unsigned short	WORD;
			struct{
				unsigned short   					:1;
				unsigned short   CHANGE_MODE		:1;			// 計測ﾓｰﾄﾞ切り替えﾌﾗｸﾞ
				unsigned short   POWER				:1;			// 電源		0:OFF	1:ON
				unsigned short   MEASUREMENT		:1;			// 計測		0:STOP	1:START
				unsigned short   ECO				:1;			// ECO		0:OFF	1:ON
				unsigned short   POWER_ON			:1;			// 電源ONﾌﾗｸﾞ
				unsigned short   POWER_OFF			:1;			// 電源OFFﾌﾗｸﾞ
				unsigned short   MEMORY_CONTROL		:1;			// ﾒﾓﾘ制御ﾌﾗｸﾞ
				unsigned short   BUFFER_RESET		:1;			// ﾊﾞｯﾌｧﾘｾｯﾄﾌﾗｸﾞ
				unsigned short   AFTER_STOPPING		:1;			// 計測停止後1ｻｲｸﾙ取得ﾌﾗｸﾞ
				unsigned short   SWING_OUTPUT		:1;			// 振れ出力ﾌﾗｸﾞ
				unsigned short   BUFFER_TRANSFER	:1;			// ﾊﾞｯﾌｧ転送ﾌﾗｸﾞ
				unsigned short   PORTABLE			:1;			// ﾎﾟｰﾀﾌﾞﾙ操作ﾌﾗｸﾞ
				unsigned short   PARAM_INITIAL		:1;			// ﾊﾟﾗﾒｰﾀ初期化ﾌﾗｸﾞ
				unsigned short   CNC_INITIAL		:1;			// CNC電源起動時初期化ﾌﾗｸﾞ
				
				unsigned short   GROWTH_OUTPUT		:1;			//伸び結果出力ﾌﾗｸﾞ
				}	BIT;
			}	FLAG;
			
		union{
			unsigned short	WORD;
			struct{
				unsigned short	DIMMER					:1;			// ﾃﾞｨﾏｰ
				unsigned short	BLACK_WHITE_COMPLETE	:1;			// BLACKﾁｪｯｸ・WHITEﾁｪｯｸ完了ﾌﾗｸﾞ
				unsigned short	MEMORY_RESET			:1;			// 記憶ﾘｾｯﾄﾌﾗｸﾞ
				unsigned short	OK						:1;			// OKﾌﾗｸﾞ
				unsigned short	AUTO_RIGHT				:1;			// 自動判別(右)ﾌﾗｸﾞ
				unsigned short	AUTO_LEFT				:1;			// 自動判別(左)ﾌﾗｸﾞ
				unsigned short	AUTO_BOTH				:1;			// 自動判別(両端)ﾌﾗｸﾞ
				unsigned short	AUTO_UNDETECTED			:1;			// 自動判別(未検出)ﾌﾗｸﾞ
				unsigned short	AUTO_MODE				:3;			// 自動判別ﾓｰﾄﾞ	0:なし	1:工具径(d≦4)	2:工具径(d＞4 左側)	3:工具径(d＞4 右側)
				unsigned short	EDGE_LEFT_NG			:1;			// ｴｯｼﾞ 左NGﾌﾗｸﾞ
				unsigned short	EDGE_RIGHT_NG			:1;			// ｴｯｼﾞ 右NGﾌﾗｸﾞ
				unsigned short	FOCUS_LEFT_NG			:1;			// 焦点 左NGﾌﾗｸﾞ
				unsigned short	FOCUS_RIGHT_NG			:1;			// 焦点 右NGﾌﾗｸﾞ
				unsigned short	PROFILE_AUTO			:1;			// ﾌﾟﾛﾌｧｲﾙ自動判別済ﾌﾗｸﾞ
				}	BIT;
			}	FLAG2;
			
		union{
			unsigned short	WORD;
			struct{
				unsigned short	PARA_READ				:2;			// ﾊﾟﾗﾒｰﾀ読み出しﾌﾗｸﾞ
				unsigned short	PARA_WRITE				:2;			// ﾊﾟﾗﾒｰﾀ書き込みﾌﾗｸﾞ
				unsigned short	BOOT_LED				:1;			// BOOTLED点滅ﾌﾗｸﾞ
				unsigned short	BLINK_LED				:1;			// LED点滅ﾌﾗｸﾞ
				unsigned short	AUTO_ERROR				:1;			// 自動ｴﾗｰﾌﾗｸﾞ
				unsigned short	TUNING_ERROR			:1;			// ﾁｭｰﾆﾝｸﾞｴﾗｰ
				unsigned short	EDGE_L_ERROR			:1;			// ｴｯｼﾞLｴﾗｰﾌﾗｸﾞ
				unsigned short	EDGE_R_ERROR			:1;			// ｴｯｼﾞRｴﾗｰﾌﾗｸﾞ
				unsigned short	SWING_RISE				:2;			// 振れ上昇ﾌﾗｸﾞ
				unsigned short	SWING_RESET				:1;			// 振れｶｳﾝﾄﾘｾｯﾄﾌﾗｸﾞ
				unsigned short	HDI_OUTPUT				:1;			// HDI出力ﾌﾗｸﾞ
				unsigned short	PEAKHOLD_ENABLE			:1;			// ﾋﾟｰｸﾎｰﾙﾄﾞ有効ﾌﾗｸﾞ
				unsigned short	PEAKHOLD_RESET			:1;			// ﾋﾟｰｸﾎｰﾙﾄﾞﾘｾｯﾄﾌﾗｸﾞ
				}	BIT;
			}	FLAG3;
			
		union{
			unsigned short	WORD;
			struct{
// chg 2016.01.21 K.Uemura start	G12102
				unsigned short							:6;
				unsigned short	OKCOUNT_FLAG			:1;			// OKｶｳﾝﾄﾌﾗｸﾞ(結果達成)
//				unsigned short							:7;
// chg 2016.01.21 K.Uemura end
				unsigned short	EDGE_L					:1;			// ｴｯｼﾞLﾌﾗｸﾞ(現結果)
				unsigned short	EDGE_R					:1;			// ｴｯｼﾞRﾌﾗｸﾞ(現結果)
				unsigned short	EDGE_LR					:1;			// ｴｯｼﾞLRﾌﾗｸﾞ(現結果)
				unsigned short	EDGE_L_POLLING			:1;			// ｴｯｼﾞLﾌﾗｸﾞ(ﾎﾟｰﾘﾝｸﾞ間)
				unsigned short	EDGE_R_POLLING			:1;			// ｴｯｼﾞRﾌﾗｸﾞ(ﾎﾟｰﾘﾝｸﾞ間)
				unsigned short	EDGE_LR_POLLING			:1;			// ｴｯｼﾞLRﾌﾗｸﾞ(ﾎﾟｰﾘﾝｸﾞ間)
				unsigned short	EDGE_L_SCAN				:1;			// ｴｯｼﾞLﾌﾗｸﾞ(計測開始～)
				unsigned short	EDGE_R_SCAN				:1;			// ｴｯｼﾞRﾌﾗｸﾞ(計測開始～)
				unsigned short	EDGE_LR_SCAN			:1;			// ｴｯｼﾞLRﾌﾗｸﾞ(計測開始～)
				}	BIT;
			}	FLAG4;
			
		union{
			unsigned short	WORD;
			struct{
// chg 2015.05.13 K.Uemura start	
				unsigned short							:3;
// add 2015.12.22 K.Uemura start	FB2201
				unsigned short	MOVE_COVER				:1;			// カバー移動中
// add 2015.12.22 K.Uemura end
				unsigned short	DISP_SOFTVERSION		:1;			// ソフトバージョン
//				unsigned short							:6;
// chg 2015.05.13 K.Uemura end
				unsigned short	BOOTUP_PROCESS			:1;			// 起動プロセス(起動処理が完了すると1)
// add 2015.11.04 K.Uemura start	
				unsigned short	INITIALIZE_FLAG			:1;			// 起動プロセス(初期化処理が完了すると1)
// add 2015.11.04 K.Uemura end
				unsigned short	LOW_BATTERY				:1;			// ﾊﾞｯﾃﾘｰ残量
				unsigned short	LOW_BATTERY_FLAG		:1;
				unsigned short	VOLTAGE_ERROR			:1;			// 電圧不足
				unsigned short	DEB_SIOOUT_FLAG			:1;			// 232c出力フラグ
				unsigned short	DEB_SIOOUT_TIME			:4;			// 232c出力カウント数
				unsigned short	DEB_RESET_FLAG			:1;			// 再起動フラグ
				}	BIT;
			}	FLAG5;
			
		union{
			unsigned short	WORD;
			struct{
				unsigned short							:4;
				unsigned short	START_DELAY_TIME		:1;			// 計測開始遅延時間ﾌﾗｸﾞ
				unsigned short	VERIFY_ERROR			:1;			// ﾊﾟﾗﾒｰﾀ確認ｴﾗｰﾌﾗｸﾞ
				unsigned short	PIXEL_END				:1;			// 特定画素終了ﾌﾗｸﾞ
				unsigned short	HDI_CHECK_OK			:1;			// HDIﾁｪｯｸOKﾌﾗｸﾞ
				unsigned short	HDI_CHECK_COMPLETION	:1;			// HDIﾁｪｯｸ完了ﾌﾗｸﾞ
				unsigned short	DATA_TRANSMISSION		:1;			// ﾃﾞｰﾀ送信ﾌﾗｸﾞ
				unsigned short	PROFILE_PROCESSING		:1;			// ﾌﾟﾛﾌｧｲﾙ処理ﾌﾗｸﾞ
				unsigned short	ROUGH_SCAN				:1;			// 粗測定処理ﾌﾗｸﾞ
				
				unsigned short	CLEANING				:1;			// 清掃実行ﾌﾗｸﾞ
				unsigned short	ADD_DATA				:1;			// 追加ﾃﾞｰﾀﾌﾗｸﾞ(平均・最小・最大) ADD 160226
				
				unsigned short	DEBUG_LEVEL				:1;			// ｾﾝｻﾚﾍﾞﾙﾌﾗｸﾞ	ADD 161025
				unsigned short	HDI_UPDATE				:1;			// HDI更新ﾌﾗｸﾞ	ADD 170204
				}	BIT;
			}	FLAG6;
			
		unsigned long	DEB_RESET_TIME;					// 再起動フラグ※秒で指定すること(15[min]≒54,000[sec])

		union{
			unsigned short	WORD;
			struct{
				unsigned short					:2;
				unsigned short	MEASURE_BEFORE	:6;
				unsigned short	MEASURE			:6;			// 0:工具径(d≦4)	1:工具径(d＞4 左側)	2:振れ	3:焦点	4:中心位置設定	5:ﾌﾟﾛﾌｧｲﾙ	6:光軸調整	7:ORIGIN	8:ORIGIN(ｴｯｼﾞ考慮)	10:工具径(自動)	11:工具径(d＞4 右側)
				unsigned short	PEAKHOLD		:2;			// 0:Off	1:On	2:5s	3:10s
				}	BIT;
			}	SELECT;
			
		unsigned short	AUTO_RIGHT_COUNT;				// 自動判別(右)ｶｳﾝﾀ
		unsigned short	AUTO_LEFT_COUNT;				// 自動判別(左)ｶｳﾝﾀ
		unsigned short	AUTO_BOTH_COUNT;				// 自動判別(両端)ｶｳﾝﾀ
		unsigned short	AUTO_UNDETECTED_COUNT;			// 自動判別(未検出)ｶｳﾝﾀ
		
		unsigned short	FIN_COUNT;						// 完了待機ｶｳﾝﾀ
		
		unsigned short	PEAKHOLD_COUNT;					// ﾋﾟｰｸﾎｰﾙﾄﾞｶｳﾝﾀ
		
		union{
			unsigned char	BYTE;
			struct{
				unsigned char	B0				:1;		// 
				unsigned char	B1				:1;		// 
				unsigned char	B2				:1;		// 
				unsigned char	B3				:1;		// 
				unsigned char	B4				:1;		// 
				unsigned char	B5				:1;		// 
				unsigned char					:2;		// 
				}	BIT;
			}	HDI;
		
		unsigned char	AUTO_COUNT;						// 方向判別ｶｳﾝﾄ
		unsigned char	TUNING_COUNT;					// ﾁｭｰﾆﾝｸﾞｶｳﾝﾄ
		
		unsigned char	BUFFER_OUTPUT;					// ﾊﾞｯﾌｧ出力状態
		
		unsigned char	BATTERY_POWER;					// 電池残量(%)
		
		unsigned short	MASTER_STATUS;					// ﾒｲﾝｼｰｹﾝｽ用ｽﾃｰﾀｽ変数
		unsigned char	MASTER_COUNT;					// ﾏｽﾀｰｼｰｹﾝｽｽﾚｯﾄﾞｶｳﾝﾀ
		
		unsigned short	TIM_COUNT;						// 1msﾀｲﾏｶｳﾝﾀ
		
		unsigned long	COMPARE_DBUS_LONG[4];			// DBUSの値比較用ﾃﾞｰﾀ
		
		unsigned long	BLACK_WHITE_CHECK[4];			// BLACKﾁｪｯｸ・WHITEﾁｪｯｸのﾃﾞｰﾀ
		unsigned char	WHITE_CHECK_COUNT;				// WHITEﾁｪｯｸｶｳﾝﾀ
		
		unsigned char	CHECK_RETRY_COUNT;				// BLACKﾁｪｯｸ・WHITEﾁｪｯｸ再送ｶｳﾝﾀ
		
		unsigned char	DIM_RETRY_COUNT;				// 調光ﾘﾄﾗｲｶｳﾝﾄ
		unsigned short	DIM_LEVEL;						// 調光ﾚﾍﾞﾙ
		
		long			INPUT_DBUS_LONG;				// DBUSの入力ﾃﾞｰﾀ
			
		float			INPUT_DBUS;						// DBUSの入力ﾃﾞｰﾀ
		float			INPUT_DBUS_BEFORE[3];			// DBUSの入力ﾃﾞｰﾀ(1つ前のﾃﾞｰﾀ)
		
		float			INPUT_CBUS;						// CBUSの入力ﾃﾞｰﾀ
		
		unsigned short	CORRECTION_TABLE_NUMBER;		// 補正ﾃｰﾌﾞﾙ番号
		
		float			TABLE_TEMP_DATA[1001];			// ﾃｰﾌﾞﾙ一時ﾃﾞｰﾀ
		unsigned short	TABLE_TEMP_COUNT;				// ﾃｰﾌﾞﾙ一時ﾃﾞｰﾀｶｳﾝﾄ
		
		unsigned long	TABLE_EDGE_DISTANCE[100];		// 距離(実寸換算ﾃｰﾌﾞﾙ)
		unsigned long	TABLE_EDGE_LEFT[100];			// ｴｯｼﾞ 左(実寸換算ﾃｰﾌﾞﾙ)
		unsigned long	TABLE_EDGE_RIGHT[100];			// ｴｯｼﾞ 右(実寸換算ﾃｰﾌﾞﾙ)
		
		unsigned long	Z_TABLE_EDGE_DISTANCE[100];		// 距離(実寸換算ﾃｰﾌﾞﾙ)
		unsigned long	Z_TABLE_EDGE_RIGHT[100];		// ｴｯｼﾞ 右(Z)(実寸換算ﾃｰﾌﾞﾙ)
		
		// ﾁｭｰﾆﾝｸﾞ処理
		float			TUNING_EDGE_SMALL_D_LOWER;			// dの最大下限(ﾁｭｰﾆﾝｸﾞ)
		float			TUNING_EDGE_SMALL_D_UPPER;			// dの最大上限(ﾁｭｰﾆﾝｸﾞ)
		float			TUNING_EDGE_LEFT_MIN_LOWER;			// ｴｯｼﾞ 左最小下限(ﾁｭｰﾆﾝｸﾞ)
		float			TUNING_EDGE_LEFT_MIN_UPPER;			// ｴｯｼﾞ 左最小上限(ﾁｭｰﾆﾝｸﾞ)
		float			TUNING_EDGE_RIGHT_MAX_LOWER;		// ｴｯｼﾞ 右最大下限(ﾁｭｰﾆﾝｸﾞ)
		float			TUNING_EDGE_RIGHT_MAX_UPPER;		// ｴｯｼﾞ 右最大上限(ﾁｭｰﾆﾝｸﾞ)
		
		float			TUNING_FOCUS_LEFT_MIN;				// 焦点 左最小(ﾁｭｰﾆﾝｸﾞ)
		float			TUNING_FOCUS_RIGHT_MIN;				// 焦点 右最小(ﾁｭｰﾆﾝｸﾞ)
		
		//
		
		float			PEAKHOLD_DATA1[3];				// 
		float			PEAKHOLD_DATA2[3];				// 
		
		unsigned char	SWING_UP_COUNT;					// 振れ上昇ｶｳﾝﾄ
		unsigned char	SWING_DOWN_COUNT;				// 振れ下降ｶｳﾝﾄ
		
		unsigned short	SWING_BUFFER_COUNT;				// 振れﾊﾞｯﾌｧ格納用ｶｳﾝﾄ
		unsigned short	SWING_BUFFER_COUNT2;			// 振れﾊﾞｯﾌｧ格納用ｶｳﾝﾄ
		unsigned short	PROFILE_BUFFER_COUNT;			// ﾌﾟﾛﾌｧｲﾙﾊﾞｯﾌｧ格納用ｶｳﾝﾄ
		
		unsigned short	BUFFER_NO_OLD;					// ﾊﾞｯﾌｧﾅﾝﾊﾞｰ旧
		unsigned short	BUFFER_NO_NEW;					// ﾊﾞｯﾌｧﾅﾝﾊﾞｰ新
		
		unsigned short	START_DELAY_TIME;				// 計測開始遅延時間
		
// add 2016.03.21 K.Uemura start	G32101
		unsigned char	END_CONDITION;					// 終了条件
// add 2016.03.21 K.Uemura end
		unsigned long	SPINDLE_SPEED;					// 回転数
		unsigned short	FLUTES;							// 刃数
		unsigned short	RADIUS;							// 工具径
// add 2016.03.10 K.Uemura start	G31002
		unsigned short	EXTEND_CYCLE;					// 伸び測定ｻｲｸﾙ
// add 2016.03.10 K.Uemura end
// add 2016.03.21 K.Uemura start	G32101
		unsigned char	COVER_OPEN_SENSOR;				// ｶﾊﾞｰｾﾝｻOpen
		unsigned char	COVER_CLOSE_SENSOR;				// ｶﾊﾞｰｾﾝｻClose
// add 2016.03.21 K.Uemura end
		
		unsigned short	IDLE_COUNT;						// 空回しｶｳﾝﾄ
		unsigned short	IDLE_COUNT_SET;					// 空回しｶｳﾝﾄｾｯﾄ
		
		//
		unsigned short	TUNING_ENABLE;					// 傾斜ﾁｭｰﾆﾝｸﾞ有無
		unsigned short	TUNING_SECONDS;					// ﾁｭｰﾆﾝｸﾞ秒数
		unsigned short	TUNING_OFFSET;					// ﾁｭｰﾆﾝｸﾞｵﾌｾｯﾄ
		unsigned short	TUNING_RATIO_LOWER_LIMIT;		// ﾁｭｰﾆﾝｸﾞ比率下限
		unsigned short	TUNING_RATIO_UPPER_LIMIT;		// ﾁｭｰﾆﾝｸﾞ比率上限
		unsigned short	MANUAL_GRADIENT;				// 手動傾斜画素
		unsigned short	FOCUSING_GRADIENT;				// 焦点合わせ傾斜
		unsigned short	ORIGIN_THRESHOLD;				// ORIGIN(ｴｯｼﾞ考慮)しきい値
		//
		
		unsigned short	BUFFER_COUNT;					// ﾊﾞｯﾌｧｶｳﾝﾄ
		unsigned short	STROBE_COUNT;					// STROBEﾊﾟﾙｽｶｳﾝﾄ
// add 2015.09.02 K.Uemura start	
		unsigned short	INITIAL_COUNT;					// 初期化ﾊﾟﾙｽｶｳﾝﾄ
// add 2015.09.02 K.Uemura end
// add 2015.09.02 K.Uemura start	
		unsigned short	COVER_COUNT;					// ｶﾊﾞｰﾊﾟﾙｽｶｳﾝﾄ
// add 2015.09.02 K.Uemura end
// add 2016.02.18 K.Uemura start	G21804
		unsigned short	CLEANING_CYCLE;					// 清掃ｶｳﾝﾄ
		unsigned short	CLEANING_COUNT_TOTAL;			// 清掃ﾊﾟﾙｽｶｳﾝﾄ(合計)
		unsigned short	CLEANING_COUNT_PASS;			// 清掃ﾊﾟﾙｽｶｳﾝﾄ(正常)
// add 2016.02.18 K.Uemura end
		//
		
		// ADD 160226
		unsigned short	AVE_AVE;						// 平均値(平均)
		unsigned short	AVE_MIN;						// 平均値(最小)
		unsigned short	AVE_MAX;						// 平均値(最大)
		unsigned short	AVE_RATE;						// 平均値(率)
		
		unsigned short	MIN_AVE;						// 最小値(平均)
		unsigned short	MIN_MIN;						// 最小値(最小)
		unsigned short	MIN_MAX;						// 最小値(最大)
		unsigned short	MIN_RATE;						// 最小値(率)
		
		unsigned short	MAX_AVE;						// 最大値(平均)
		unsigned short	MAX_MIN;						// 最大値(最小)
		unsigned short	MAX_MAX;						// 最大値(最大)
		unsigned short	MAX_RATE;						// 最大値(率)
		
		unsigned long	AVE_TOTAL;						// 平均値(合計)
		unsigned long	MIN_TOTAL;						// 最小値(合計)
		unsigned long	MAX_TOTAL;						// 最大値(合計)
		//
		
		unsigned long	INPUT_DBUS213;					// 左ｴｯｼﾞ傾斜の入力ﾃﾞｰﾀ
		unsigned long	INPUT_DBUS214;					// 右ｴｯｼﾞ傾斜の入力ﾃﾞｰﾀ
		
		unsigned long	INPUT_DBUS224;					// 左ｴｯｼﾞﾃﾞｰﾀ
		unsigned long	INPUT_DBUS225;					// 右ｴｯｼﾞﾃﾞｰﾀ
		
		unsigned short	BUFFER_COUNT2;					// ﾊﾞｯﾌｧｶｳﾝﾄ2(動作確認用)
		
		unsigned short	CBUS_NUMBER;					// CBUSﾅﾝﾊﾞｰ
		unsigned short	LAST_CBUS_NUMBER;				// CBUSﾅﾝﾊﾞｰ(1ｻｲｸﾙ最終)
		
		unsigned short	FPGA_RESTART_COUNT;				// FPGA再ｽﾀｰﾄｶｳﾝﾄ		
		unsigned char	FPGA_SEND_COUNT;				// FPGA再送信ｶｳﾝﾄ
		
		unsigned char	FPGA_SEND_STATUS;				// FPGA送信ｽﾃｰﾀｽ
		unsigned char	FPGA_RECEIVE_STATUS;			// FPGA受信ｽﾃｰﾀｽ
		
		unsigned char	FPGA_PARAM_COUNT;				// FPGAﾊﾟﾗﾒｰﾀ再送信ｶｳﾝﾄ
		
		unsigned char	TP_CONTROL_STATUS;				// ﾀｯﾁﾊﾟﾈﾙ制御ｽﾃｰﾀｽ
		
		unsigned char	POWER_STATUS;					// 電源ｽｲｯﾁｽﾃｰﾀｽ
		unsigned char	POWER_COUNT;					// 電源制御ｶｳﾝﾄ
		unsigned char	POWER_SEQUENCE_COUNT;			// POWERｼｰｹﾝｽｶｳﾝﾄ
		
		unsigned int	JUNK;							// ﾀﾞﾐｰﾘｰﾄﾞ
		
		union UFloatLong{
			long lLong;
			float fFloat;
		};
		
		unsigned short	STARTUP_COUNT;					// 起動待ち時間
		
		unsigned short	LED_BRIGHTNESS;					// LED輝度
		unsigned short	X_LED_BRIGHTNESS;				// LED輝度(X)
		unsigned short	Z_LED_BRIGHTNESS;				// LED輝度(Z)
// add 2016.03.08 K.Uemura start	G30702
		unsigned short	X_LED_BRIGHTNESS_WHITE;			// LED輝度 ﾎﾜｲﾄ平均値(X)
		unsigned short	Z_LED_BRIGHTNESS_WHITE;			// LED輝度 ﾎﾜｲﾄ平均値(Z)
// add 2016.03.08 K.Uemura end
		
		unsigned short	X_BLACK_AVE;					// ﾌﾞﾗｯｸ平均値(X)
		unsigned short	X_WHITE_AVE;					// ﾎﾜｲﾄ平均値(X)
		unsigned short	X_WHITE_MAX;					// ﾎﾜｲﾄ最大値(X)
		unsigned short	X_WHITE_MIN;					// ﾎﾜｲﾄ最小値(X)
		
		unsigned short	Z_BLACK_AVE;					// ﾌﾞﾗｯｸ平均値(Z)
		unsigned short	Z_WHITE_AVE;					// ﾎﾜｲﾄ平均値(Z)
		unsigned short	Z_WHITE_MAX;					// ﾎﾜｲﾄ最大値(Z)
		unsigned short	Z_WHITE_MIN;					// ﾎﾜｲﾄ最小値(Z)

		char			SIO_BUF[512];					// シリアル送信バッファ
		unsigned char	SIO_IDX;						// シリアルバッファインデックス
		long			PARA_DATA[50];					// ﾊﾟﾗﾒｰﾀﾃﾞｰﾀ(FPGAに対して送信するｺﾏﾝﾄﾞ)
		long			ALL_DATA[1001];					// 全ﾒﾓﾘﾃﾞｰﾀ(ﾊﾟﾗﾒｰﾀﾅﾝﾊﾞｰ)
//		long			HDI_DATA[13];					// HDIﾃﾞｰﾀ
//		unsigned char	HDI_DATA_COUNT;					// HDIﾃﾞｰﾀｶｳﾝﾀ
		long			X_HDI_DATA[13];					// HDIﾃﾞｰﾀ(X)
		long			Z_HDI_DATA[13];					// HDIﾃﾞｰﾀ(Z)
		
		unsigned int	END_TIMEOUT_PERIOD_SET;			// ﾀｲﾑｱｳﾄ時間ｾｯﾄ(終了条件)(s)
		unsigned int	OK_COUNT_SET;					// OKｶｳﾝﾄ数ｾｯﾄ
		unsigned int	NG_COUNT_SET;					// 連続NGｶｳﾝﾄ数ｾｯﾄ
// chg 2017.01.06 K.Uemura start	H10601	
		unsigned int	WAIT_COUNT_SET;					// WAITｶｳﾝﾄ数ｾｯﾄ
// chg 2017.01.06 K.Uemura end
		
		unsigned long	END_TIMEOUT_PERIOD;				// ﾀｲﾑｱｳﾄ時間(終了条件)(ms)
		unsigned long	OK_COUNT;						// OKｶｳﾝﾄ数
		unsigned long	NG_COUNT;						// 連続NGｶｳﾝﾄ数
		
		unsigned long	TOTAL_COUNT;					// 計測総数
		
		unsigned int	COM_START_PERIOD;				// 計測開始時間(ms)
		unsigned int	COM_TIMEOUT_PERIOD;				// ﾀｲﾑｱｳﾄ時間(通信設定)(ms)
		
		unsigned int	MEMORY_TIME;					// 記憶時間(ms)
		
		unsigned int	POLLING_COUNT;					// ﾎﾟｰﾘﾝｸﾞ周期ｶｳﾝﾄ
		
// add 2016.03.11 K.Uemura start	G31101
		unsigned short	HDI0_DELAY_COUNT;				// HDI0ｽｷｯﾌﾟ遅延ｶｳﾝﾄ
		unsigned short	HDI1_DELAY_COUNT;				// HDI1ｽｷｯﾌﾟ遅延ｶｳﾝﾄ
		unsigned short	HDI2_DELAY_COUNT;				// HDI2ｽｷｯﾌﾟ遅延ｶｳﾝﾄ
		unsigned short	HDI3_DELAY_COUNT;				// HDI3ｽｷｯﾌﾟ遅延ｶｳﾝﾄ
		unsigned short	HDI4_DELAY_COUNT;				// HDI4ｽｷｯﾌﾟ遅延ｶｳﾝﾄ
		unsigned short	HDI5_DELAY_COUNT;				// HDI5ｽｷｯﾌﾟ遅延ｶｳﾝﾄ
// add 2016.03.11 K.Uemura end
		
		unsigned short	HDI0_HOLD_COUNT;				// HDI0ｽｷｯﾌﾟ保持ｶｳﾝﾄ
		unsigned short	HDI1_HOLD_COUNT;				// HDI1ｽｷｯﾌﾟ保持ｶｳﾝﾄ
		unsigned short	HDI2_HOLD_COUNT;				// HDI2ｽｷｯﾌﾟ保持ｶｳﾝﾄ
		unsigned short	HDI3_HOLD_COUNT;				// HDI3ｽｷｯﾌﾟ保持ｶｳﾝﾄ
		unsigned short	HDI4_HOLD_COUNT;				// HDI4ｽｷｯﾌﾟ保持ｶｳﾝﾄ
		unsigned short	HDI5_HOLD_COUNT;				// HDI5ｽｷｯﾌﾟ保持ｶｳﾝﾄ
		
		unsigned long	FPGA_RIVISION;					// FPGAﾊﾞｰｼﾞｮﾝ
		
		unsigned long	FOCUSING_HDI;					// 焦点合わせ・HDI(FPGAからのﾃﾞｰﾀ)
		
		unsigned long	PROCESS_STATUS;					// 動作状態
		
		unsigned long	START_DELAY_TIME_COUNT;			// 計測開始遅延時間ｶｳﾝﾀ
		//unsigned char	START_DELAY_TIME_FLAG;			// 計測開始遅延時間ﾌﾗｸﾞ
		
		unsigned char   CHANGE_FPGA;					// FPGA処理切り替えﾌﾗｸﾞ
		
		unsigned char	MEASUREMENT_DIRECTION;			// 計測方向
		unsigned char	MEASUREMENT_DIRECTION_BEFORE;	// 計測方向(直前)
		
		unsigned char	READY_PULSE_TIME;				// READYのﾊﾟﾙｽ時間
		//unsigned char	VERIFY_ERROR_FLAG;				// ﾊﾟﾗﾒｰﾀ確認ｴﾗｰﾌﾗｸﾞ
		
		//unsigned char	PIXEL_END_FLAG;					// 特定画素終了ﾌﾗｸﾞ
		unsigned short	PIXEL_END_COUNT;				// 特定画素終了ｶｳﾝﾄ
		
		unsigned short	HDI_CHECK_COUNT;				// HDIﾁｪｯｸｶｳﾝﾄ
		
		unsigned short	TRIGGER_TIME_PERIOD;			// 周期ﾄﾘｶﾞ時間
		
		unsigned short	GROWTH_INTERVAL_COUNT;			// 伸び計測間隔ｶｳﾝﾄ
		unsigned long	GROWTH_OUTPUT_COUNT;			// 伸び計測出力ｶｳﾝﾄ
// add 2015.08.21 K.Uemura start	
		long			GROWTH_MIN;						// 伸び最小
		long			GROWTH_MAX;						// 伸び最大
// add 2015.08.21 K.Uemura end
// add 2015.08.19 K.Uemura start	プロファイル最大／最小出力
		unsigned long	PROFILE_OUTPUT_COUNT;			// ﾌﾟﾛﾌｧｲﾙ計測出力ｶｳﾝﾄ
// add 2015.08.19 K.Uemura end
// add 2016.10.20 K.Uemura start	GA2001
		unsigned short	VERIFY_COUNT;					// ﾊﾟﾗﾒｰﾀ確認回数
// add 2016.10.20 K.Uemura end
// add 2016.07.26 K.Uemura start	G72601
		unsigned long	MSEC_COUNTER;					// msecｶｳﾝﾀ(約429万秒 ≒ 49日)
		unsigned char	MSEC_FLAG;						// msecﾌﾗｸﾞ
		unsigned char	MSEC_INDEX;						// msecｲﾝﾃﾞｯｸｽ
		unsigned long	MSEC_BUFFER[30][10];			// msec変数(EXE → RDY OFF ≒ lap)
														//          EXE → RDY OFF → FIN
// add 2016.07.26 K.Uemura end
// add 2017.01.19 K.Uemura start	H11901	
		unsigned short	ERROR_BEFORE;					// ｴﾗｰ番号(前回)
// add 2017.01.19 K.Uemura end
}	SEQ;

/*	RESULT制御用構造体	*/
struct {
		float			EDGE_LEFT_PIXEL[5];				// ｴｯｼﾞ 左(画素)
		float			EDGE_RIGHT_PIXEL[5];			// ｴｯｼﾞ 右(画素)
		
		float			EDGE_LEFT_SCALE[5];				// ｴｯｼﾞ 左
		float			EDGE_RIGHT_SCALE[5];			// ｴｯｼﾞ 右
		
		float			FOCUS_LEFT[5];					// 焦点 左
		float			FOCUS_RIGHT[5];					// 焦点 右
		
		float			WORK_EDGE_DIFF[5];				// WORK ｴｯｼﾞ左右の差
		float			WORK_CENTER[5];					// WORK 中央
		
		float			SMALL_D_MAX[5];					// d 最大
		
		float			LARGE_D_MIN[5];					// D 最小
		float			LARGE_D_MAX[5];					// D 最大
		float			LARGE_D_DIFF[5];				// D 最大最小の差
		
		float			SWING_MIN[2];					// 振れ 最小	[0] 刃数0以外	[1] 刃数0
		float			SWING_MAX[2];					// 振れ 最大	[0] 刃数0以外	[1] 刃数0
		float			SWING_DIFF[2];					// 振れ 最大最小の差
		
		float			DELTA_X_DIFF[5];				// ΔX 差
		float			DELTA_X_DIFF_MIN[5];			// ΔX 最小
		float			DELTA_X_DIFF_MAX[5];			// ΔX 最大
		
		// 左
		float			GREATER_D_DIFF[5];				// D>4 右-中央
		float			GREATER_D_DIFF_MIN[5];			// D>4 最小
		float			GREATER_D_DIFF_MAX[5];			// D>4 最大
		
		// 右
		float			GREATER_D_DIFF_RIGHT[5];		// D>4 右-中央(右側)
		float			GREATER_D_DIFF_RIGHT_MIN[5];	// D>4 最小(右側)
		float			GREATER_D_DIFF_RIGHT_MAX[5];	// D>4 最大(右側)
		
		float			FOCUS_LEFT_MIN[5];				// 焦点 左最小
		float			FOCUS_RIGHT_MIN[5];				// 焦点 右最小
		
		float			TIR[7];							// 振れ(ﾊﾞｯﾌｧ出力用)
														// [0]最大(周期)      [実寸]
														// [1]最大(ﾁｭｰﾆﾝｸﾞ時) [画素]
														// [2]振れ比率乗算結果[画素]
														// [3]最大(検査時)    [実寸]
														// [4]最小(検査時)    [実寸]
														// [5]差(最大－最小)  [実寸]
// add 2015.07.29 K.Uemura start	
														// [6]最小(周期)      [実寸]
// add 2015.07.29 K.Uemura end
// add 2015.08.06 K.Uemura start	
		float			TIR_MAX[10];					// 各刃の最大(No1～9)
// add 2015.08.06 K.Uemura end

		float			LEFT_L[3];						// 
		float			LEFT_R[3];						// 
		float			RIGHT_L[3];						// 
		float			RIGHT_R[3];						// 

		float			GROWTH[3];						// 伸び(ﾊﾞｯﾌｧ出力用)

// add 2015.09.30 K.Uemura start	093001
		short			RUNOUT_MIN;
		short			RUNOUT_MAX;
		short			PROFILE_MIN;
		short			PROFILE_MAX;
// add 2015.09.30 K.Uemura end
// add 2016.06.22 K.Uemura start	G62202
		short			SENSOR_LEVEL_X;					// ｾﾝｻﾚﾍﾞﾙ X
		short			SENSOR_LEVEL_Z;					// ｾﾝｻﾚﾍﾞﾙ Z
// add 2016.06.22 K.Uemura end
}	RESULT;

/*	debug用構造体	*/
struct {
		unsigned short	DEBUG_COUNT;					// ﾃﾞﾊﾞｯｸﾞ出力用ｶｳﾝﾀ
		long			DEBUG[4096];					// ﾃﾞﾊﾞｯｸﾞ出力用ﾃﾞｰﾀ
		unsigned char	DEBUG_OUTPUT;					// ﾃﾞﾊﾞｯｸﾞ出力用出力ﾌｫｰﾏｯﾄ
														// 0 整数3桁
														// 1 符号・整数1桁・小数点3桁
														// 2 整数3桁・小数点3桁

#ifdef	DEBUG_EACHSCAN
// deb 2015.07.29 K.Uemura start	
		char			DEBUG_FLAG;
		unsigned short	DEBUG_EDGE[8000];				// エッジ座標
//		unsigned short	DEBUG_EDGE2[4000];				// エッジ座標
		unsigned char	DEBUG_FOCUS[8000];				// 焦点数値
		unsigned short	DEBUG_TIR_TIME;					// 計測時間[ms]
		unsigned short	DEBUG_NO[20];					// エッジ要素
		unsigned char	DEBUG_NO_COUNT;					// エッジ要素カウント数
// deb 2015.07.29 K.Uemura end
#endif
}	DEBUG_STR;

/*	ﾀｲﾏｰ制御用構造体	*/
struct {
		union{
			unsigned short	WORD;
			struct{
				unsigned short  UNI_ST1	:1;				//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ1ｽﾀｰﾄﾋﾞｯﾄ
				unsigned short  UNI_UP1	:1;				//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ1完了ﾋﾞｯﾄ
				unsigned short  UNI_ST2	:1;				//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ2ｽﾀｰﾄﾋﾞｯﾄ
				unsigned short  UNI_UP2	:1;				//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ2完了ﾋﾞｯﾄ
				unsigned short  UNI_ST3	:1;				//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ3ｽﾀｰﾄﾋﾞｯﾄ
				unsigned short  UNI_UP3	:1;				//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ3完了ﾋﾞｯﾄ
				unsigned short  UNI_ST4	:1;				//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ4ｽﾀｰﾄﾋﾞｯﾄ
				unsigned short  UNI_UP4	:1;				//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ4完了ﾋﾞｯﾄ
				unsigned short  UNI_ST5	:1;				//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ5ｽﾀｰﾄﾋﾞｯﾄ
				unsigned short  UNI_UP5	:1;				//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ5完了ﾋﾞｯﾄ
				unsigned short  UNI_ST6	:1;				//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ6ｽﾀｰﾄﾋﾞｯﾄ
				unsigned short  UNI_UP6	:1;				//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ6完了ﾋﾞｯﾄ
				unsigned short  UNI_ST7	:1;				//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ7ｽﾀｰﾄﾋﾞｯﾄ
				unsigned short  UNI_UP7	:1;				//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ7完了ﾋﾞｯﾄ
				unsigned short  UNI_ST8	:1;				//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ8ｽﾀｰﾄﾋﾞｯﾄ
				unsigned short  UNI_UP8	:1;				//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ8完了ﾋﾞｯﾄ
				}	BIT;
			}	MSEC_10;
		unsigned short	UNI_CONT1	;					//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ1用ｶｳﾝﾀ
		unsigned short	UNI_CONT2	;					//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ2用ｶｳﾝﾀ
		unsigned short	UNI_CONT3	;					//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ3用ｶｳﾝﾀ
		unsigned short	UNI_CONT4	;					//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ4用ｶｳﾝﾀ
		unsigned short	UNI_CONT5	;					//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ5用ｶｳﾝﾀ
		unsigned short	UNI_CONT6	;					//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ6用ｶｳﾝﾀ
		unsigned short	UNI_CONT7	;					//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ7用ｶｳﾝﾀ
		unsigned short	UNI_CONT8	;					//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ8用ｶｳﾝﾀ
		unsigned short	UNI_COMP1	;					//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ1用比較ｶｳﾝﾀ
		unsigned short	UNI_COMP2	;					//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ2用比較ｶｳﾝﾀ
		unsigned short	UNI_COMP3	;					//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ3用比較ｶｳﾝﾀ
		unsigned short	UNI_COMP4	;					//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ4用比較ｶｳﾝﾀ
		unsigned short	UNI_COMP5	;					//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ5用比較ｶｳﾝﾀ
		unsigned short	UNI_COMP6	;					//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ6用比較ｶｳﾝﾀ
		unsigned short	UNI_COMP7	;					//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ7用比較ｶｳﾝﾀ
		unsigned short	UNI_COMP8	;					//ﾕﾆﾊﾞｰｻﾙｶｳﾝﾀ8用比較ｶｳﾝﾀ
		
		unsigned char	MASTER_COUNT_1US;				// 1usﾏｽﾀｰｼｰｹﾝｽｽﾚｯﾄﾞｶｳﾝﾀ
		unsigned char	MASTER_COUNT_10US;				// 10usﾏｽﾀｰｼｰｹﾝｽｽﾚｯﾄﾞｶｳﾝﾀ
		unsigned char	MASTER_COUNT_100US;				// 100usﾏｽﾀｰｼｰｹﾝｽｽﾚｯﾄﾞｶｳﾝﾀ
		
}	TIM1;

/*	入力処理用構造体	*/
struct {
		union{
			unsigned short	WORD;
			struct{
				unsigned short					:1;		// 
				unsigned short	IN_SHOT			:1;		// 外部入力許可
				unsigned short	POWER_SW		:1;		// 電源ｽｲｯﾁ
				unsigned short	START_SW		:1;		// ｽﾀｰﾄｽｲｯﾁ
				unsigned short	CLEAR_SW		:1;		// ｸﾘｱｽｲｯﾁ
				unsigned short	SELECT_SW		:1;		// ｾﾚｸﾄｽｲｯﾁ
				unsigned short	PEAKHOLD_SW		:1;		// ﾋﾟｰｸﾎｰﾙﾄﾞｽｲｯﾁ
				unsigned short	MODE_SW			:1;		// ﾓｰﾄﾞｽｲｯﾁ
				unsigned short	UP_SW			:1;		// 上ｽｲｯﾁ
				unsigned short	DOWN_SW			:1;		// 下ｽｲｯﾁ
				unsigned short	ECO_SW			:1;		// ECOｽｲｯﾁ
				unsigned short	EXT_POWER		:1;		// 外部電源
				unsigned short	HARDWARE_TYPE	:1;		// CNC/ﾎﾟｰﾀﾌﾞﾙ切替
				unsigned short	BOOT_SW			:1;		// BOOTｽｲｯﾁ
				unsigned short	COVER_OPEN		:1;		// ｶﾊﾞｰｽｲｯﾁ開
				unsigned short	COVER_CLOSE		:1;		// ｶﾊﾞｰｽｲｯﾁ閉
				}	BIT;
			}	FLAG;
			
		unsigned char	JUMP_STATUS;					// ｲﾝﾌﾟｯﾄｶｳﾝﾀ
		
		// ｽｲｯﾁ入力
		// 電源ｽｲｯﾁ
		union{
			unsigned short	WORD;
			struct{
				unsigned short	ON		:8;				// 入力あり
				unsigned short	OFF		:8;				// 入力なし
				}	BIT;
			}	POWER_SW;
		
		// ｽﾀｰﾄｽｲｯﾁ
		union{
			unsigned char	BYTE;
			struct{
				unsigned char	ON		:4;				// 入力あり
				unsigned char	OFF		:4;				// 入力なし
				}	BIT;
			}	START_SW;
			
		// ｸﾘｱｽｲｯﾁ
		union{
			unsigned char	BYTE;
			struct{
				unsigned char	ON		:4;				// 入力あり
				unsigned char	OFF		:4;				// 入力なし
				}	BIT;
			}	CLEAR_SW;
			
		// ｾﾚｸﾄｽｲｯﾁ
		union{
			unsigned long	LONG;
			struct{
				unsigned long	ON		:16;			// 入力あり
				unsigned long	OFF		:16;			// 入力なし
				}	BIT;
			}	SELECT_SW;
			
		// ﾋﾟｰｸﾎｰﾙﾄﾞｽｲｯﾁ
		union{
			unsigned char	BYTE;
			struct{
				unsigned char	ON		:4;				// 入力あり
				unsigned char	OFF		:4;				// 入力なし
				}	BIT;
			}	PEAKHOLD_SW;
			
		// ﾓｰﾄﾞｽｲｯﾁ
		union{
			unsigned long	LONG;
			struct{
				unsigned long	ON		:16;			// 入力あり
				unsigned long	OFF		:16;			// 入力なし
				}	BIT;
			}	MODE_SW;
			
		// 上ｽｲｯﾁ
		union{
			unsigned long	LONG;
			struct{
				unsigned long	ON		:16;			// 入力あり
				unsigned long	OFF		:16;			// 入力なし
				}	BIT;
			}	UP_SW;
			
		// 下ｽｲｯﾁ
		union{
			unsigned long	LONG;
			struct{
				unsigned long	ON		:16;			// 入力あり
				unsigned long	OFF		:16;			// 入力なし
				}	BIT;
			}	DOWN_SW;
			
		// ECOｽｲｯﾁ
		union{
			unsigned short	WORD;
			struct{
				unsigned short	ON		:8;				// 入力あり
				unsigned short	OFF		:8;				// 入力なし
				}	BIT;
			}	ECO_SW;
			
		// ｶﾊﾞｰｽｲｯﾁ開
		union{
			unsigned char	BYTE;
			struct{
				unsigned char	ON		:4;				// 入力あり
				unsigned char	OFF		:4;				// 入力なし
				}	BIT;
			}	COVER_OPEN;
			
		// ｶﾊﾞｰｽｲｯﾁ閉
		union{
			unsigned char	BYTE;
			struct{
				unsigned char	ON		:4;				// 入力あり
				unsigned char	OFF		:4;				// 入力なし
				}	BIT;
			}	COVER_CLOSE;
}	IN;

/*	出力処理用構造体	*/
struct {
		unsigned char	LED_STATUS;						// LEDｽﾃｰﾀｽ
		unsigned char	LED_STATUS_BEFORE;				// LEDｽﾃｰﾀｽ(直前)
		unsigned char	JUMP_STATUS;					// ｲﾝﾌﾟｯﾄｶｳﾝﾀ
		unsigned char	MASTER_STATUS;					// 制御ｽﾃｰﾀｽ
		unsigned char	SUB_STATUS;						// 制御ｽﾃｰﾀｽ
		unsigned char	BOOT_LED_BLINK;					// BOOTLED点滅回数
		
		unsigned short	BOOT_LED_COUNT;					// BOOTLED1msｶｳﾝﾄ
}	OUT;

/*	通信処理用構造体	*/
// RS422
struct {
		unsigned char	MASTER_STATUS;				// 制御ｽﾃｰﾀｽ
		unsigned char	SUB_STATUS;					// 制御ｽﾃｰﾀｽ
		unsigned char	WR_BUF[5000];				// 送信ﾊﾞｯﾌｧ
		unsigned char	RE_BUF[1010];				// 受信ﾊﾞｯﾌｧ
		
		unsigned short	WR_CONT;					// 書き込みｶｳﾝﾀ
		unsigned short	RE_CONT;					// 読み出し回数ｶｳﾝﾀ
		unsigned short	STARTUP_COUNT;				// 起動処理ｶｳﾝﾄ
		unsigned short	SEND_COUNT;					// 送信文字数
		unsigned short	RECEIVE_COUNT;				// 受信文字数
		unsigned short	START_ADDRESS;				// 開始ｱﾄﾞﾚｽ
		
		unsigned short	SET_COUNT;					// ｺﾏﾝﾄﾞｾｯﾄｶｳﾝﾄ
		unsigned short	SEND_DATA;					// 送信ﾃﾞｰﾀ
		unsigned short	SEND_DATA_COUNT;			// 送信ﾃﾞｰﾀｶｳﾝﾄ
		unsigned short	RECEIVE_DATA;				// 受信ﾃﾞｰﾀ
		unsigned short	RECEIVE_DATA_COUNT;			// 受信ﾃﾞｰﾀｶｳﾝﾄ
		
		unsigned short	receive[10];				// 受信ﾃﾞｰﾀ
		unsigned short	send[10];					// 送信ﾃﾞｰﾀ
		
		/*
		unsigned short	NO290;						// BLACKしきい値(Z)
		unsigned short	NO291;						// WHITEしきい値(Z)
		unsigned short	NO292;						// ﾗｲﾝｾﾝｻﾁｪｯｸ上限(Z)
		unsigned short	NO293;						// ﾗｲﾝｾﾝｻﾁｪｯｸ下限(Z)
		unsigned short	NO294;						// 調光ﾚﾍﾞﾙ(Z)
		
// add 2014.08.07 K.Uemura start	起動処理結果表示
		unsigned short	NO295;						// BLACK
		unsigned short	NO296;						// WHITE
		unsigned short	NO297;						// 上限
		unsigned short	NO298;						// 下限
		*/
		unsigned short	NO299;						// 調光レベル
// add 2014.08.07 K.Uemura end
		
		union{
			unsigned short	WORD;
			struct{
				unsigned short	EXE			:1;		// 実行
				unsigned short	ROF			:1;		// READY OFF検知
				unsigned short				:1;		//
				unsigned short	ACK			:1;		// ACK
				unsigned short				:4;		//
				unsigned short	RST			:1;		// 強制ﾘｾｯﾄ
				unsigned short				:2;		//
				unsigned short	ECD			:1;		// 終了条件無効
// chg 2016.12.06 K.Uemura start	GC0602
				unsigned short				:1;		//
				unsigned short	RSN			:1;		// 粗測定
//				unsigned short				:2;		//
// chg 2016.12.06 K.Uemura end
				unsigned short	COM			:1;		// TPD通信無効
				unsigned short	DIS			:1;		// TPD無効
				}	BIT;
			}	NO300;								// 指令
		
		unsigned short	NO301;						// 指令引数
		unsigned short	NO302;						// 予備
		unsigned short	NO303;						// 回転数[上位]
		unsigned short	NO304;						// 回転数[下位]
		unsigned short	NO305;						// 刃数
		unsigned short	NO306;						// 工具径
		unsigned short	NO307;						// 引数
		unsigned short	NO308;						// 
		unsigned short	NO309;						// 
			
		union{
			unsigned short	WORD;
			struct{
				unsigned short	POW			:1;		// POWER
				unsigned short	RDY			:1;		// READY
				unsigned short	FIN			:1;		// 完了
				unsigned short	STR			:1;		// STROBE
				unsigned short				:3;		// ﾀﾞﾐｰﾌﾗｸﾞ
				unsigned short	LED			:1;		// LEDﾌﾗｸﾞ
				unsigned short	DIR			:2;		// 走査方向(0:未検出 1:右ｴｯｼﾞ 2:左ｴｯｼﾞ 3:両ｴｯｼﾞ)
				unsigned short				:2;		// ﾀﾞﾐｰﾌﾗｸﾞ
				unsigned short	CSO			:1;		// 近接ｾﾝｻ1(ｶﾊﾞｰOPEN)
				unsigned short	CSC			:1;		// 近接ｾﾝｻ2(ｶﾊﾞｰCLOSE)
				unsigned short	COV			:1;		// 電磁ﾊﾞﾙﾌﾞ1(ｶﾊﾞｰ状態)
				unsigned short	PUR			:1;		// 電磁ﾊﾞﾙﾌﾞ2(清掃ｴｱ状態)
				}	BIT;
			}	NO310;								// 状態
			
		unsigned short	NO311;						// 状態番号
		unsigned short	NO312;						// ｴﾗｰ番号
		
		unsigned short	NO313;						// 計測結果 現在値(REAL)	上位
		unsigned short	NO314;						// 計測結果 現在値(REAL)	下位
		unsigned short	NO315;						// 計測結果 最終値(d)		上位
		unsigned short	NO316;						// 計測結果 最終値(d)		下位
		unsigned short	NO317;						// 計測結果 最終値(D)		上位
		unsigned short	NO318;						// 計測結果 最終値(D)		下位
		unsigned short	NO319;						// 計測結果 最終値(ΔX)		上位
		unsigned short	NO320;						// 計測結果 最終値(ΔX)		下位
		unsigned short	NO321;						// 計測結果 最終値(振れ)	上位
		unsigned short	NO322;						// 計測結果 最終値(振れ)	下位
		
		unsigned short	NO323;						// 空き
		unsigned short	NO324;						// 空き
		unsigned short	NO325;						// 空き
		unsigned short	NO326;						// 空き
		
		//unsigned short	NO327;						// 計測総数(上位)
		//unsigned short	NO328;						// 計測総数(下位)
		//unsigned short	NO329;						// OK数(上位)
		//unsigned short	NO330;						// OK数(下位)
		
		unsigned short	NO331;						// HDI(SKIP 6点)
		unsigned short	NO332;						// LED11灯
		//unsigned short	NO333;						// 接続状態
		
		union{
			unsigned short	WORD;
			struct{
				unsigned short	LINK		:1;		// LINK UP
				unsigned short	POW			:1;		// POWER
				unsigned short				:14;	// ﾀﾞﾐｰﾌﾗｸﾞ
				}	BIT;
			}	NO333;								// 接続状態
		
// add 2015.08.06 K.Uemura start	
		unsigned short	NO2981[10];					// 振れ：各刃の最大
		unsigned short	NO2998;						// 振れ：最小
		unsigned short	NO2999;						// 振れ：最大
// add 2015.08.06 K.Uemura end
		unsigned short	NO3000[640];				// 
		
		/**/
		/*
		unsigned short	NO9001;						// 左ｴｯｼﾞ最小位置
		unsigned short	NO9002;						// 右ｴｯｼﾞ最大位置
		unsigned short	NO9003;						// 左ｴｯｼﾞ傾斜
		unsigned short	NO9004;						// 右ｴｯｼﾞ傾斜
		unsigned short	NO9005;						// ｴｯｼﾞ間の差
		
#if	1
		unsigned short	NO9006;						// 3005：最大
		unsigned short	NO9007;						// 3005：最小
		unsigned short	NO9008;						// 3003：最小
		unsigned short	NO9009;						// 3003：最大
		unsigned short	NO9010;						// 3004：最小
		unsigned short	NO9011;						// 3004：最大
		unsigned short	NO9012;						// 
		unsigned short	NO9013;						// 
		unsigned short	NO9014;						// 
		unsigned short	NO9015;						// 
		unsigned short	NO9016;						// 
		unsigned short	NO9017;						// 
		unsigned short	NO9018;						// 
		unsigned short	NO9019;						// 
		unsigned short	NO9020;						// d
		unsigned short	NO9021;						// D
		unsigned short	NO9022;						// ΔX
		unsigned short	NO9023;						// 振れ(TIR)
		unsigned short	NO9024;						// 
		unsigned short	NO9025;						// 
		unsigned short	NO9026;						// 
		unsigned short	NO9027;						// 
		unsigned short	NO9028;						// 
		unsigned short	NO9029;						// 
		unsigned short	NO9030;						// real
		unsigned short	NO9031;						// 
		unsigned short	NO9032;						// d
		unsigned short	NO9033;						// 
		unsigned short	NO9034;						// D
		unsigned short	NO9035;						// 
		unsigned short	NO9036;						// ΔX
		unsigned short	NO9037;						// 
		unsigned short	NO9038;						// 振れ
		unsigned short	NO9039;						// 
#endif

*/
		
		unsigned short	NO101;						// 左ｴｯｼﾞ最小位置
		unsigned short	NO102;						// 右ｴｯｼﾞ最大位置
		unsigned short	NO103;						// 左ｴｯｼﾞ傾斜
		unsigned short	NO104;						// 右ｴｯｼﾞ傾斜
		unsigned short	NO105;						// ｴｯｼﾞ間の差
		unsigned short	NO106;						// 3005：最大
		unsigned short	NO107;						// 3005：最小
		unsigned short	NO108;						// 3003：最小
		unsigned short	NO109;						// 3003：最大
		unsigned short	NO110;						// 3004：最小
		unsigned short	NO111;						// 3004：最大
		unsigned short	NO112;						// 
		unsigned short	NO113;						// 
		unsigned short	NO114;						// 
		unsigned short	NO115;						// 
		unsigned short	NO116;						// 
		unsigned short	NO117;						// 
		unsigned short	NO118;						// 
		unsigned short	NO119;						// 
		unsigned short	NO120;						// d
		unsigned short	NO121;						// D
		unsigned short	NO122;						// ΔX
		unsigned short	NO123;						// 振れ(TIR)
		unsigned short	NO124;						// 
		unsigned short	NO125;						// 
		unsigned short	NO126;						// 
		unsigned short	NO127;						// 
		unsigned short	NO128;						// 
		unsigned short	NO129;						// 
		unsigned short	NO130;						// real
		unsigned short	NO131;						// 
		unsigned short	NO132;						// d
		unsigned short	NO133;						// 
		unsigned short	NO134;						// D
		unsigned short	NO135;						// 
		unsigned short	NO136;						// ΔX
		unsigned short	NO137;						// 
		unsigned short	NO138;						// 振れ
		unsigned short	NO139;						// 
		unsigned short	NO140;						// 
		unsigned short	NO141;						// 
		unsigned short	NO142;						// 
		unsigned short	NO143;						// 
		unsigned short	NO144;						// 
		unsigned short	NO145;						// 
		unsigned short	NO146;						// 
		unsigned short	NO147;						// 
		unsigned short	NO148;						// 
		unsigned short	NO149;						// 
// add 2016.09.05 K.Uemura start	G90501
		unsigned short	Dummy;						// ﾀﾞﾐｰ
// add 2016.09.05 K.Uemura end

		unsigned short	MEM_BUF[3001];				// ﾒﾓﾘﾊﾞｯﾌｧ
		
}	COM0;

// RS232C
struct {
		unsigned char	MASTER_STATUS;				// 制御ｽﾃｰﾀｽ
		unsigned char	SUB_STATUS;					// 制御ｽﾃｰﾀｽ
		unsigned char	RE_BUF[10];					// 受信ﾊﾞｯﾌｧ
		
		unsigned short	WR_CONT;					// 書き込みｶｳﾝﾀ
		unsigned short	RE_CONT;					// 読み出し回数ｶｳﾝﾀ
}	COM2;

// CFG
struct {
		unsigned char	MASTER_STATUS;				// 制御ｽﾃｰﾀｽ
		unsigned char	SUB_STATUS;					// 制御ｽﾃｰﾀｽ
		unsigned char	WR_BUF[500];				// 送信ﾊﾞｯﾌｧ
		unsigned char	RE_BUF[100];				// 受信ﾊﾞｯﾌｧ
		
		unsigned short	STARTUP_COUNT;				// 起動処理ｶｳﾝﾄ
		unsigned short	WR_CONT;					// 書き込みｶｳﾝﾀ
		unsigned short	RE_CONT;					// 読み出し回数ｶｳﾝﾀ
		unsigned short	SEND_COUNT;					// 送信文字数
}	COM3;

// M25
struct {
		unsigned char	MASTER_STATUS;				// 制御ｽﾃｰﾀｽ
		unsigned char	SUB_STATUS;					// 制御ｽﾃｰﾀｽ
		unsigned char	DATA_SELECT;				// ﾃﾞｰﾀ選択
		unsigned char	WR_BUF[300];				// 送信ﾊﾞｯﾌｧ
		unsigned char	RE_BUF[520];				// 受信ﾊﾞｯﾌｧ
		
		unsigned short	STARTUP_COUNT;				// 起動処理ｶｳﾝﾄ
		unsigned short	WR_CONT;					// 書き込みｶｳﾝﾀ
		unsigned short	RE_CONT;					// 読み出し回数ｶｳﾝﾀ
		unsigned short	SEND_COUNT;					// 送信文字数
		
		unsigned long	START_ADDRESS;				// 開始ｱﾄﾞﾚｽ
}	COM6;

// LED
struct {
		unsigned char   SEG_BUF[15]	;					// 7ｾｸﾞ表示用ﾊﾞｯﾌｧ
		unsigned char	ON_COUNT;						// ON時間
		
		union{
			unsigned char	BYTE;
			struct{
				unsigned char	L				:1;		// 焦点L
				unsigned char	R				:1;		// 焦点R
				unsigned char	Z				:1;		// 焦点Z
				unsigned char					:5;		// 
				}	BIT;
			}	FOCUS;
			
		unsigned char	Z_FOCUSING;						// Z軸4灯LED
		unsigned short	FOCUSING;						// X軸11灯LED
		
		union{
			unsigned short	WORD;
			struct{
				// Measure
				unsigned short	LESS			:1;		// 「≦」小なりｲｺｰﾙ	(less than or equal)
				unsigned short	GREATER			:1;		// 「＞」大なり		(greater than)
				unsigned short	RUN_OUT			:1;		// Run Out
				// Setting
				unsigned short	FOCUS			:1;		// Focus
				unsigned short	CENTER			:1;		// Center
				// Peak Hold
				unsigned short	OFF				:1;		// Off
				unsigned short	ON				:1;		// On
				unsigned short	S5				:1;		// 5s
				unsigned short	S10				:1;		// 10s
				
				unsigned short	POWER			:1;		// POWER
				unsigned short	ECO				:1;		// ECO
				unsigned short	SPARE			:1;		// 予備
				// Logo
				unsigned short					:4;		// 
				}	BIT;
			}	MSP;
}	LED;

/*	I2C通信処理用構造体	*/
struct {
		union{
			unsigned char	BYTE;
			struct{
				unsigned char					:7;
				unsigned char	ADDRESS_CHANGE	:1;		// ｱﾄﾞﾚｽ変更ﾌﾗｸﾞ
				}	BIT;
			}	FLAG;
			
		// ﾒﾓﾘｲﾆｼｬﾙ[1] + ﾊﾟﾗﾒｰﾀ[4000] + X左ｴｯｼﾞ(偶数)・X右ｴｯｼﾞ(奇数)[4000] + Z右ｴｯｼﾞ(奇数)[2000]
		unsigned char   WR_BUF[10100];			// 送信ﾊﾞｯﾌｧ
		unsigned char   RE_BUF[10100];			// 受信ﾊﾞｯﾌｧ
		unsigned char   DUMMY		;			//受信ﾀﾞﾐｰ
		unsigned char	MASTER_STATUS	;		//制御ｽﾃｰﾀｽ
		unsigned char	SUB_STATUS	;			//制御ｽﾃｰﾀｽ
		unsigned char   WR_COMP		;			//書き込み比較ｶｳﾝﾀ
		unsigned char   SLAVE_ADD	;			//ｽﾚｲﾌﾞｱﾄﾞﾚｽ
		unsigned char   ADD_HI		;			//ﾒﾓﾘｱﾄﾞﾚｽ上位
		unsigned char   ADD_LOW		;			//ﾒﾓﾘｱﾄﾞﾚｽ下位
		unsigned char   ADD_NO_HI		;		//ﾒﾓﾘｱﾄﾞﾚｽﾅﾝﾊﾞｰ上位
		unsigned char   ADD_NO_LOW		;		//ﾒﾓﾘｱﾄﾞﾚｽﾅﾝﾊﾞｰ下位
		unsigned char   RESET_COUNT		;		// ｿﾌﾄｳｪｱﾘｾｯﾄｶｳﾝﾄ
		
		unsigned short  WR_CONT		;			//書き込みｶｳﾝﾀ
		unsigned short	RE_CONT		;			//読み出し回数ｶｳﾝﾀ
		unsigned short  LAST_ADDRESS;			// 最終ﾃﾞｰﾀ
		unsigned short  STATUS		;			//ｽﾃｰﾀｽ
}	I2C;

/*	AD入力処理制御用構造体	*/
struct {
		union{
			unsigned char	BYTE;
			struct{
				unsigned char			:6;		//ﾀﾞﾐｰﾋﾞｯﾄ
				unsigned char	ST		:1;		//AD入力処理ｽﾀｰﾄﾌﾗｸﾞ
				unsigned char	FIN		:1;		//AD入力処理ｽﾀｰﾄﾌﾗｸﾞ
				}	BIT;
			}	FLAG;
			
		unsigned char	COUNT		;			//入力処理積算ｶｳﾝﾀ
		unsigned char	STATUS		;			//入力処理積算ｶｳﾝﾀ
		
		unsigned short	ADIN0[21]	;			//入力ﾃﾞｰﾀ格納変数
		unsigned short	ADIN0MAX	;			//入力ﾃﾞｰﾀ格納変数(最大値)
		unsigned short	ADIN0MIN	;			//入力ﾃﾞｰﾀ格納変数(最小値)
		unsigned short	V_BATT	;				// 電源電圧
		unsigned short	INTERVAL	;			//入力処理ｲﾝﾀｰﾊﾞﾙｶｳﾝﾀ
		unsigned short	INPUT		;			//入力ﾕﾆｯﾄ
}	ADCOV;
