/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
#include "main.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"


/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc;
DMA_HandleTypeDef hdma_adc;
TIM_HandleTypeDef htim14;

char Rx_Buffer[RX_DATA_SIZE];
uint32_t Rx_ptr_in;
char winkey_buffer[RX_DATA_SIZE];
uint32_t winkey_ptr_in;

uint8_t send_buffer_array[SEND_BUFFER_SIZE];
uint8_t send_buffer_bytes = 0;

uint16_t winkey_last_unbuffered_speed_wpm = 0;
uint8_t send_buffer_status = SERIAL_SEND_BUFFER_NORMAL;
uint8_t winkey_buffer_counter = 0;
uint8_t winkey_buffer_pointer = 0;
uint8_t winkey_sending = 0;
uint8_t winkey_xoff = 0;
uint8_t winkey_session_ptt_tail = 0;
uint8_t winkey_speed_state = WINKEY_UNBUFFERED_SPEED;
uint8_t first_extension_time = 0;
long winkey_paddle_echo_buffer = 0;
uint8_t  winkey_paddle_echo_activated = 0;
unsigned long winkey_paddle_echo_buffer_decode_time = 0;


uint32_t automatic_sending_interruption_time = 0;
volatile uint64_t micros;
volatile flags_t flags;			// One bit flags arrray
settings_t setting;					// Sequenser setting

uint8_t  sending_mode = UNDEFINED_SENDING; // curent send mode
uint8_t  last_sending_mode = MANUAL_SENDING;
uint8_t  keying_compensation = 0;
uint8_t  ultimatic_mode = ULTIMATIC_NORMAL;
uint8_t  length_letterspace = 3;	
uint8_t being_sent = SENDING_NOTHING;
uint32_t ptt_time = 0; 
uint32_t sequencer_ptt_inactive_time = 0;

volatile uint8_t  cw_sidetone;
volatile uint8_t  tone_state;

ch_t ch[4] = {
	{TX_CH1, 0},
	{TX_CH2, 0},
	{TX_CH3, 0},
	{TX_CH4, 0},
};



/* Flash storage for settings at absolute address */
const __IO settings_t settings_Store __attribute__((at(FLASH_STORE_ADR)));


// ADC speed and key
volatile uint16_t adc[2] = {0,};
uint8_t pot_wpm_low_value;
uint8_t pot_wpm_high_value;
uint8_t last_pot_wpm_read;
int pot_full_scale_reading = 1023;



const static morse_t morse_keys[] =
{
	{'A', 0xfff9},	/// 	11111111 11111001
	{'B', 0xffe8},	/// 	11111111 11101000
	{'C', 0xffea},	/// 	11111111 11101010
	{'D', 0xfff4},	/// 	11111111 11110100
	{'E', 0xfffc},	/// 	11111111 11111100
	{'F', 0xffe2},	/// 	11111111 11100010
	{'G', 0xfff6},	/// 	11111111 11110110
	{'H', 0xffe0},	/// 	11111111 11100000
	{'I', 0xfff8},	/// 	11111111 11111000
	{'J', 0xffe7},	/// 	11111111 11100111
	{'K', 0xfff5},	/// 	11111111 11110101
	{'L', 0xffe4},	/// 	11111111 11100100
	{'M', 0xfffb},	/// 	11111111 11111011
	{'N', 0xfffa},	/// 	11111111 11111010
	{'O', 0xfff7},	/// 	11111111 11110111
	{'P', 0xffe6},	/// 	11111111 11100110
	{'Q', 0xffed},	/// 	11111111 11101101
	{'R', 0xfff2},	/// 	11111111 11110010
	{'S', 0xfff0},	/// 	11111111 11110000
	{'T', 0xfffd},	/// 	11111111 11111101
	{'U', 0xfff1},	/// 	11111111 11110001
	{'V', 0xffe1},	/// 	11111111 11100001
	{'W', 0xfff3},	/// 	11111111 11110011
	{'X', 0xffe9},	/// 	11111111 11101001
	{'Y', 0xffeb},	/// 	11111111 11101011
	{'Z', 0xffec},	/// 	11111111 11101100
	{' ', 0xffef},	/// 	11111111 00000000

	{'0', 0xffdf},	/// 	11111111 11011111
	{'1', 0xffcf},	/// 	11111111 11001111
	{'2', 0xffc7},	/// 	11111111 11000111
	{'3', 0xffc3},	/// 	11111111 11000011
	{'4', 0xffc1},	/// 	11111111 11000001
	{'5', 0xffc0},	/// 	11111111 11000000
	{'6', 0xffd0},	/// 	11111111 11010000
	{'7', 0xffd8},	/// 	11111111 11011000
	{'8', 0xffdc},	/// 	11111111 11011100
	{'9', 0xffde},	/// 	11111111 11011110
		{'.', 0xff95},  ///   11111111 10010101
	{',', 0xffb3},  ///   11111111 10110011
	{':', 0xffb8},  ///   11111111 10111000
	{'?', 0xff8c},  ///   11111111 10001100
	{39,  0xff9e},  ///   11111111 10011110  --- Apostrophe
	{'-', 0xffa1},  ///   11111111 10100001  --- Minus=Hyphen
	{'/', 0xffd2},  ///   11111111 11010010
	{'(', 0xffad},  ///   11111111 10101101
	{')', 0xffad},  ///   11111111 10101101
	{'{', 0xffad},  ///   11111111 10101101
	{'}', 0xffad},  ///   11111111 10101101
	{'[', 0xffad},  ///   11111111 10101101 
	{']', 0xffad},  ///   11111111 10101101 
	{34,  0xff92},  ///   11111111 10010010 --- Quotation "
	{'@', 0xff9a},  ///   11111111 10011010 
	{'=', 0xffd1},  ///   11111111 11010001
	{'<', 0xffff},  ///   special - set char time to 1 dit
	{'>', 0xffff},  ///   special - set char time to 3 dit
	{'|', 0xffff},  ///   special - tone
	{192, 0xfff9}, 	// А 11111111 11111001
	{193, 0xffe8},	// Б 11111111 11101000
	{194, 0xfff3},  // В 11111111 11110011
	{195, 0xfff6},  // Г 11111111 11110110
	{196, 0xfff4},  // Д 11111111 11110100
	{197, 0xfffc},  // Е 11111111 11111100
	{168, 0xfffc},  // Ё 11111111 11111100
	{184, 0xfffc},  // ё 11111111 11111100
	{198, 0xffe1},  // Ж 11111111 11100001
	{199, 0xffec},  // З 11111111 11101100
	{200, 0xfff8},  // И 11111111 11111000
	{201, 0xffe7},  // Й 11111111 11100111
	{202, 0xfff5},  // К 11111111 11110101
	{203, 0xffe4},  // Л 11111111 11100100
	{204, 0xfffb},  // М 11111111 11111011
	{205, 0xfffa},  // Н 11111111 11111010
	{206, 0xfff7},  // О 11111111 11110111
	{207, 0xffe6},  // П 11111111 11100110
	{208, 0xfff2},  // Р 11111111 11110010
	{209, 0xfff0},  // С 11111111 11110000
	{210, 0xfffd},  // Т 11111111 11111101
	{211, 0xfff1},  // У 11111111 11110001
	{212, 0xffe2},  // Ф 11111111 11100010
	{213, 0xffe0},  // Х 11111111 11100000
	{214, 0xffea},  // Ц 11111111 11101010
	{215, 0xffee},  // Ч 11111111 11101110
	{216, 0xffef},  // Ш 11111111 11101111
	{217, 0xffed},  // Щ 11111111 11101101
	{218, 0xffdb},  // Ъ 11111111 11011011
	{219, 0xffeb},  // Ы 11111111 11101011
	{220, 0xffe9},  // Ь 11111111 11101001
	{221, 0xffc4},  // Э 11111111 11000100
	{222, 0xffe3},  // Ю 11111111 11100011
	{223, 0xffe5},  // Я 11111111 11100101
	{255, 0xffe5}   // 11111111 11100101

//	{	CH_special | CW_SPC_AA	, CW_special |	CW_SPC_AA	},  ///      --->   AA, New line	.-.-
//	{	CH_special | CW_SPC_AR	, CW_special |	CW_SPC_AR	},  ///      --->   AR, End of message	.-.-.
//	{	CH_special | CW_SPC_AS	, CW_special |	CW_SPC_AS	},  ///      --->   AS, Wait	.-...
//	{	CH_special | CW_SPC_BK	, CW_special |	CW_SPC_BK	},  ///      --->   BK, Break	-...-.-
//	{	CH_special | CW_SPC_BT	, CW_special |	CW_SPC_BT	},  ///      --->   BT, New paragraph	-...-
//	{	CH_special | CW_SPC_CL	, CW_special |	CW_SPC_CL	},  ///      --->   CL, Going off the air ("clear")	-.-..-..
//	{	CH_special | CW_SPC_CT	, CW_special |	CW_SPC_CT	},  ///      --->   CT, Start copying	-.-.-
//	{	CH_special | CW_SPC_DO	, CW_special |	CW_SPC_DO	},  ///      --->   DO, Change to wabun code	-..---
//	{	CH_special | CW_SPC_KN	, CW_special |	CW_SPC_KN	},  ///      --->   KN, Invite a specific station to transmit	-.--.
//	{	CH_special | CW_SPC_SK	, CW_special |	CW_SPC_SK	},  ///      --->   SK, End of transmission (also VA)	...-.-
//	{	CH_special | CW_SPC_SN	, CW_special |	CW_SPC_SN	},  ///      --->   SN, Understood (also VE)	...-.
//	{	CH_special | CW_SPC_SOS	, CW_special |	CW_SPC_SOS},  ///      --->   SOS, Distress message	...---...
//	{	CH_special | CW_SPC_BRK	, CW_special |	CW_SPC_BRK},  ///      --->   SOS, Distress message	...---...
};
#define CW_NOKEYFOUND		0xffff


/* Private function prototypes -----------------------------------------------*/
void ServiceWinkey(uint8_t action, uint8_t incoming_serial_byte);
void SystemClock_Config(void);

static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC_Init(void);
static void MX_TIM14_Init(void);
	
void CheckPotentiometer(void);
uint16_t PotValueWPM(void);
void SpeedSet(uint16_t wpm_set);
void PttKey(void);
void CheckPttTail(void);

void Delay(uint32_t cycles)
{
		uint32_t c = cycles * 8000;
		while(c--);
}



uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static inline uint8_t lowByte(uint16_t x) { return (uint8_t)((x) & 0xFF); }
static inline uint8_t highByte(uint16_t x) {  return (uint8_t)((x<<8) & 0xFF); }

/** 
  * @brief 
  * @param 
  * @retval 
  */
uint8_t is_visible_character(uint8_t char_in)
{
  if((char_in > 31) || (char_in == 9) || (char_in == 10) || (char_in == 13)) return 1;
  else return 0;
}

/** 
  * @brief 
  * @param 
  * @retval 
  */
uint16_t Morse_CharToKey(char ch)
{
	int i;
	for (i = 0; i<sizeof(morse_keys)/sizeof(morse_t); i++) {
		if (ch == morse_keys[i].ch) return morse_keys[i].key;
	}
	return CW_NOKEYFOUND;	
}

/** 
  * @brief 
  * @param 
  * @retval 
  */
int convert_cw_number_to_ascii (uint32_t number_in)
{

  // number_in:  1 = dit, 2 = dah, 9 = a space

  switch (number_in) {
    case 12: return 65;          // A
    case 2111: return 66;
    case 2121: return 67;
    case 211: return 68;
    case 1: return 69;
    case 1121: return 70;
    case 221: return 71;
    case 1111: return 72;
    case 11: return 73;
    case 1222: return 74;
    case 212: return 75;
    case 1211: return 76;
    case 22: return 77;
    case 21: return 78;
    case 222: return 79;
    case 1221: return 80;
    case 2212: return 81;
    case 121: return 82;
    case 111: return 83;
    case 2: return 84;
    case 112: return 85;
    case 1112: return 86;
    case 122: return 87;
    case 2112: return 88;
    case 2122: return 89;
    case 2211: return 90;     // Z

    case 22222: return 48;     // 0
    case 12222: return 49;
    case 11222: return 50;
    case 11122: return 51;
    case 11112: return 52;
    case 11111: return 53;
    case 21111: return 54;
    case 22111: return 55;
    case 22211: return 56;
    case 22221: return 57;
    case 112211: return '?';   // ?
    case 21121: return 47;    // /
    case 2111212: return '*';  // BK 
//    case 221122: return '!';   // ! sp5iou 20180328
    case 221122: return ',';
    case 121212: return '.';
    case 122121: return '@';
    case 222222: return 92;   // special hack; six dahs = \ (backslash)
    case 21112: return '=';   // BT
    case 211112: return '-'; 
    //case 2222222: return '+'; 
    case 9: return 32;        // special 9 = space

    case 212122: return 33;  // ! //sp5iou
    case 1112112: return 36;  // $ //sp5iou
    case 12111: return 38;  // & // sp5iou
    case 122221: return 39;  // ' // sp5iou
    case 121121: return 34;  // " // sp5iou
    case 112212: return 95;  // _ // sp5iou
    case 212121: return 59;  // ; // sp5iou
    case 222111: return 58;  // : // sp5iou
    case 212212: return 41;  // KK (stored as ascii ) ) // sp5iou
    case 111212: return 62;  // SK (stored as ascii > ) // sp5iou
    case 12121: return 60;  // AR (store as ascii < ) // sp5iou

    case 21221: return 40;  // (KN store as ascii ( ) //sp5iou //aaaaaaa


    // for English/Cyrillic/Western European font LCD controller (HD44780UA02):
    case 12212: return 197;      // 'Å' - AA_capital (OZ, LA, SM)
    //case 12212: return 192;    // 'À' - A accent   
    case 1212: return 198;       // 'Æ' - AE_capital   (OZ, LA)
    //case 1212: return 196;     // 'Ä' - A_umlaut (D, SM, OH, ...)
    case 2222: return 138;       // CH  - (Russian letter symbol)
    case 22122: return 209;      // 'Ñ' - (EA)               
    //case 2221: return 214;     // 'Ö' – O_umlaut  (D, SM, OH, ...)
    //case 2221: return 211;     // 'Ò' - O accent
    case 2221: return 216;       // 'Ø' - OE_capital    (OZ, LA)
    case 1122: return 220;       // 'Ü' - U_umlaut     (D, ...)
    case 111111: return 223;     // beta - double S    (D?, ...)   
    case 21211: return 199;      // Ç
    case 11221: return 208;      // Ð
    case 12112: return 200;      // È
    case 11211: return 201;      // É
    case 221121: return 142;     // Ž
    default: 
      return '*'; 
  }
}


/** 
  * @brief 
  * @param 
  * @retval 
  */
void OnUsbDataRx(uint8_t* dataIn, uint32_t length, uint16_t index)
{
	uint32_t i;
	if (0 == index) {
		dataIn[length] = 0;
		USB_print((char*)dataIn);
		for (i=0; i<length; i++) {
			if (i>=RX_DATA_SIZE) break;
			if (flags.inCommand) break;
			if( '\n' == dataIn[i]) continue;
			if( '\r' == dataIn[i]) {
				Rx_Buffer[Rx_ptr_in] = 0;
				flags.inCommand = 1;
				break;
			}
			Rx_Buffer[Rx_ptr_in] = dataIn[i];
			Rx_ptr_in++;
		
			/* To avoid buffer overflow */
			if (Rx_ptr_in >= RX_DATA_SIZE) {
				Rx_ptr_in = 0;
			}
		}
	} else if (2 == index) {
		// winkey data
		for (i=0; i<length; i++) {
			if (i>=RX_DATA_SIZE) break;
			winkey_buffer[winkey_ptr_in] = dataIn[i];
			winkey_ptr_in++;
			flags.winkey_command = 1;
			/* To avoid buffer overflow */
			if (winkey_ptr_in >= RX_DATA_SIZE) winkey_ptr_in = 0;
		}
	}
}


/** 
  * @brief 
  * @param 
  * @retval 
  */
void WinkeyPortWrite(uint8_t byte_to_send, uint8_t override_filter)
{
	uint8_t tx_res, cnt = 0, msg[1];
	msg[0] = byte_to_send;
#ifdef DEBUG_WINKEY
		char izp[50];
#endif //DEBUG_WINKEY
	
	if (((byte_to_send > 4) && (byte_to_send < 31)) && (!override_filter)){
#ifdef DEBUG_WINKEY
		sprintf(izp, "WK TX FILTERED: 0x%X", byte_to_send);
		USB_print(izp);
#endif //DEBUG_WINKEY
		return;
	}
	
	do {
		tx_res = CDC_Transmit_FS(msg, 1, 2);
	} while (USBD_BUSY == tx_res && ++cnt < 200);
	
#ifdef DEBUG_WINKEY
	sprintf(izp, "WK TX: 0x%x\r\n", byte_to_send);
	USB_print(izp);
#endif //DEBUG_WINKEY
}

/** 
  * @brief 
  * @param 
  * @retval 
  */
void ClearSendBuffer()
{
#ifdef DEBUG
	USB_print("clear_send_buffer\n\r");
#endif //DEBUG
  winkey_xoff=0;
  send_buffer_bytes = 0;
}

/** 
  * @brief 
  * @param 
  * @retval 
  */
void RemoveFromSendBuffer()
{
  if ((send_buffer_bytes < 10) && winkey_xoff && flags.winkey_host_open) {
    winkey_xoff=0;
    WinkeyPortWrite(0xc0|winkey_sending|winkey_xoff,0); //send status /XOFF
  }
  
  if (send_buffer_bytes) {
    send_buffer_bytes--;
  }
  if (send_buffer_bytes) {
    for (int x = 0;x < send_buffer_bytes;x++) {
      send_buffer_array[x] = send_buffer_array[x+1];
    }
    WinkeyPortWrite(0xc0|winkey_sending|winkey_xoff,0);
	}
}


/** 
  * @brief 
  * @param 
  * @retval 
  */
void AddToSendBuffer(uint8_t incoming_serial_byte)
{

  if (send_buffer_bytes < SEND_BUFFER_SIZE) {
    if (incoming_serial_byte != 127) {
      send_buffer_bytes++;
      send_buffer_array[send_buffer_bytes - 1] = incoming_serial_byte;
  
      if ((send_buffer_bytes>20) && flags.winkey_host_open) {
        winkey_xoff=1;
        WinkeyPortWrite(0xc0|winkey_sending|winkey_xoff,0); //send XOFF status         
      }
    } else {  // we got a backspace
      if (send_buffer_bytes){
        send_buffer_bytes--;
      }
    }
  }
}

/** 
  * @brief 
  * @param 
  * @retval 
  */
void WinkeyFarnsworthCommand(uint8_t incoming_serial_byte)
{

	if ((incoming_serial_byte > 9) && (incoming_serial_byte < 100)) {
		setting.wpm_farnsworth = incoming_serial_byte;
	}
}


/** 
  * @brief 
  * @param 
  * @retval 
  */
void WinkeyUnbufferedSpeedCommand(uint8_t incoming_serial_byte)
{
  if (incoming_serial_byte == 0) {
      setting.pot_activated = 1;
	} else {
    setting.wpm = incoming_serial_byte;
    winkey_speed_state = WINKEY_UNBUFFERED_SPEED;
    winkey_last_unbuffered_speed_wpm = setting.wpm;
    flags.config_dirty = 1;
  }
}

/** 
  * @brief 
  * @param 
  * @retval 
  */
void WinkeyKeyingCompensationCommand(uint8_t incoming_serial_byte)
{
	keying_compensation = incoming_serial_byte;
}

/** 
  * @brief 
  * @param 
  * @retval 
  */
void WinkeyFirstExtensionCommand(uint8_t incoming_serial_byte)
{
  first_extension_time = incoming_serial_byte;
}

/** 
  * @brief 
  * @param 
  * @retval 
  */
void WinkeyDashToDotRatioCommand(uint8_t incoming_serial_byte)
{
  if ((incoming_serial_byte > 32) && (incoming_serial_byte < 67)) {
    setting.dah_to_dit_ratio = (300*((uint16_t)(incoming_serial_byte)/50));
    flags.config_dirty = 1;
  }
}

/** 
  * @brief 
  * @param 
  * @retval 
  */
void WinkeyWeightingCommand(uint8_t incoming_serial_byte)
{
  if ((incoming_serial_byte > 9) && (incoming_serial_byte < 91)) {
    setting.weighting = incoming_serial_byte;
  }
}

/** 
  * @brief 
  * @param 
  * @retval 
  */
void WinkeyPttTimesParm1Command(uint8_t incoming_serial_byte)
{
}

/** 
  * @brief 
  * @param 
  * @retval 
  */
void WinkeyPttTimesParm2Command(uint8_t incoming_serial_byte)
{
  setting.ptt_tail_time = (3*(int)(1200/setting.wpm)) + (incoming_serial_byte*10);
  winkey_session_ptt_tail = incoming_serial_byte;
}

/** 
  * @brief 
  * @param 
  * @retval 
  */
void WinkeySetPotParm1Command(uint8_t incoming_serial_byte)
{
  pot_wpm_low_value = incoming_serial_byte;
}

/** 
  * @brief 
  * @param 
  * @retval 
  */
void WinkeySetPotParm2Command(uint8_t incoming_serial_byte)
{
    pot_wpm_high_value = (pot_wpm_low_value + incoming_serial_byte);
}

/** 
  * @brief 
  * @param 
  * @retval 
  */
void WinkeySetPotParm3Command(uint8_t incoming_serial_byte)
{
#ifdef OPTION_WINKEY_2_SUPPORT
	pot_full_scale_reading = 1031;
#else //OPTION_WINKEY_2_SUPPORT
  if (incoming_serial_byte == 255) {
		pot_full_scale_reading = 1031;
  } else {
    if (incoming_serial_byte == 127) {
			pot_full_scale_reading = 515;
		}
	}
#endif //OPTION_WINKEY_2_SUPPORT
	setting.pot_activated = 1;
}

/** 
  * @brief 
  * @param 
  * @retval 
  */
void WinkeySetmodeCommand(uint8_t incoming_serial_byte)
{
  flags.config_dirty = 1;

  if (incoming_serial_byte & 4) {  //serial echo enable
    flags.winkey_serial_echo = 1;
  } else {
    flags.winkey_serial_echo = 0;
  }
  if (incoming_serial_byte & 8) {  //paddle_swap
     setting.paddle_mode = PADDLE_REVERSE;
  } else {
     setting.paddle_mode = PADDLE_NORMAL;
  }
  switch (incoming_serial_byte & 48) {
    case 0: 
			setting.keyer_mode = IAMBIC_B;
      break;
    case 16:
			setting.keyer_mode = IAMBIC_A;
      break;
    case 32: 
			setting.keyer_mode = ULTIMATIC;
      break;
    case 48:
			setting.keyer_mode = BUG;
      break;
  }
#ifdef FEATURE_AUTOSPACE
  if ((incoming_serial_byte & 2) == 2) {  //xxxxxx1x = autospace
     setting.autospace_active = 1;
  } else {
     setting.autospace_active = 0;
  }
#endif
  if ((incoming_serial_byte & 1) == 1) {  //xxxxxxx1 = contest wordspace
     setting.length_wordspace = 6;
  } else {
     setting.length_wordspace = 7;
  }

  if ((incoming_serial_byte & 64) == 64) {  //x1xxxxxx = paddle echo
     flags.winkey_paddle_echo_activated = 1;
  } else {
     flags.winkey_paddle_echo_activated = 0;
  }
}

/** 
  * @brief 
  * @param 
  * @retval 
  */
void WinkeySidetoneFreqCommand(uint8_t incoming_serial_byte)
{
}

/** 
  * @brief 
  * @param 
  * @retval 
  */
void WinkeySetPinconfigCommand(uint8_t incoming_serial_byte)
{
  
  if (incoming_serial_byte & 1) {
    setting.ptt_buffer_hold_active = 1;
    flags.winkey_pinconfig_ptt_bit = 1;
  } else {
    setting.ptt_buffer_hold_active = 0;
#ifdef OPTION_WINKEY_2_SUPPORT
    flags.winkey_pinconfig_ptt_bit = 0;
#endif
  }
  
#ifndef OPTION_NO_ULTIMATIC
  switch (incoming_serial_byte & 192) {
    case 0:  ultimatic_mode = ULTIMATIC_NORMAL; break;
    case 64: ultimatic_mode = ULTIMATIC_DAH_PRIORITY; break;
    case 128: ultimatic_mode = ULTIMATIC_DIT_PRIORITY; break;
  }
#endif

  switch(incoming_serial_byte & 48) {
    case 0:  setting.ptt_hang_time_wordspace_units = WINKEY_HANG_TIME_1_0; break;
    case 16: setting.ptt_hang_time_wordspace_units = WINKEY_HANG_TIME_1_33; break;
    case 32: setting.ptt_hang_time_wordspace_units = WINKEY_HANG_TIME_1_66; break;
    case 48: setting.ptt_hang_time_wordspace_units = WINKEY_HANG_TIME_2_0; break;
  }
}

/** 
  * @brief 
  * @param 
  * @retval 
  */
void WinkeyLoadSettingsCommand(uint8_t winkey_status, uint8_t incoming_serial_byte)
{

  switch(winkey_status) {
     case WINKEY_LOAD_SETTINGS_PARM_1_COMMAND:
#ifdef DEBUG_WINKEY
       USB_print("WINKEY_LOAD_SETTINGS_PARM_1_COMMAND\r\n");
#endif //DEBUG_WINKEY       
       WinkeySetmodeCommand(incoming_serial_byte);
       break;
     case WINKEY_LOAD_SETTINGS_PARM_2_COMMAND:
#ifdef DEBUG_WINKEY
       USB_print("WINKEY_LOAD_SETTINGS_PARM_2_COMMAND\r\n");
#endif //DEBUG_WINKEY       
       WinkeyUnbufferedSpeedCommand(incoming_serial_byte);
       break;
     case WINKEY_LOAD_SETTINGS_PARM_3_COMMAND:
#ifdef DEBUG_WINKEY
       USB_print("WINKEY_LOAD_SETTINGS_PARM_3_COMMAND\r\n");
#endif //DEBUG_WINKEY       
       WinkeySidetoneFreqCommand(incoming_serial_byte);
       break;
     case WINKEY_LOAD_SETTINGS_PARM_4_COMMAND:
#ifdef DEBUG_WINKEY
       USB_print("WINKEY_LOAD_SETTINGS_PARM_4_COMMAND\r\n");
#endif //DEBUG_WINKEY       
       WinkeyWeightingCommand(incoming_serial_byte);
       break;
     case WINKEY_LOAD_SETTINGS_PARM_5_COMMAND:
#ifdef DEBUG_WINKEY
       USB_print("WINKEY_LOAD_SETTINGS_PARM_5_COMMAND\r\n");
#endif //DEBUG_WINKEY       
       WinkeyPttTimesParm1Command(incoming_serial_byte);
       break;
     case WINKEY_LOAD_SETTINGS_PARM_6_COMMAND:
#ifdef DEBUG_WINKEY
       USB_print("WINKEY_LOAD_SETTINGS_PARM_6_COMMAND\r\n");
#endif //DEBUG_WINKEY       
       WinkeyPttTimesParm2Command(incoming_serial_byte);
       break;
     case WINKEY_LOAD_SETTINGS_PARM_7_COMMAND:
#ifdef DEBUG_WINKEY
       USB_print("WINKEY_LOAD_SETTINGS_PARM_7_COMMAND\r\n");
#endif //DEBUG_WINKEY       
       WinkeySetPotParm1Command(incoming_serial_byte);
       break;
     case WINKEY_LOAD_SETTINGS_PARM_8_COMMAND:
#ifdef DEBUG_WINKEY
       USB_print("WINKEY_LOAD_SETTINGS_PARM_8_COMMAND\r\n");
#endif //DEBUG_WINKEY       
       WinkeySetPotParm2Command(incoming_serial_byte);
       break;
     case WINKEY_LOAD_SETTINGS_PARM_9_COMMAND:
#ifdef DEBUG_WINKEY
       USB_print("WINKEY_LOAD_SETTINGS_PARM_9_COMMAND\r\n");
#endif //DEBUG_WINKEY       
       WinkeyFirstExtensionCommand(incoming_serial_byte);
       break;
     case WINKEY_LOAD_SETTINGS_PARM_10_COMMAND:
#ifdef DEBUG_WINKEY
       USB_print("WINKEY_LOAD_SETTINGS_PARM_10_COMMAND\r\n");
#endif //DEBUG_WINKEY       
       WinkeyKeyingCompensationCommand(incoming_serial_byte);
       break;
     case WINKEY_LOAD_SETTINGS_PARM_11_COMMAND:
#ifdef DEBUG_WINKEY
       USB_print("WINKEY_LOAD_SETTINGS_PARM_11_COMMAND\r\n");
#endif //DEBUG_WINKEY       
       WinkeyFarnsworthCommand(incoming_serial_byte);
       break;
     case WINKEY_LOAD_SETTINGS_PARM_12_COMMAND:  // paddle switchpoint - don't need to support
#ifdef DEBUG_WINKEY
       USB_print("WINKEY_LOAD_SETTINGS_PARM_12_COMMAND\r\n");
#endif //DEBUG_WINKEY  
       break;
     case WINKEY_LOAD_SETTINGS_PARM_13_COMMAND:
#ifdef DEBUG_WINKEY
       USB_print("WINKEY_LOAD_SETTINGS_PARM_13_COMMAND\r\n");
#endif //DEBUG_WINKEY       
       WinkeyDashToDotRatioCommand(incoming_serial_byte);
       break;
     case WINKEY_LOAD_SETTINGS_PARM_14_COMMAND:
#ifdef DEBUG_WINKEY
       USB_print("WINKEY_LOAD_SETTINGS_PARM_14_COMMAND\r\n");
#endif //DEBUG_WINKEY       
       WinkeySetPinconfigCommand(incoming_serial_byte);
       break;
     case WINKEY_LOAD_SETTINGS_PARM_15_COMMAND:
#ifdef DEBUG_WINKEY
       USB_print("WINKEY_LOAD_SETTINGS_PARM_15_COMMAND\r\n");
#endif //DEBUG_WINKEY       
       WinkeySetPotParm3Command(incoming_serial_byte);
       break;
  }
}


/** 
  * @brief 
  * @param 
  * @retval 
  */
void WinkeyAdminGetValuesCommand()
{
  uint8_t byte_to_send;

  // 1 - mode register
  byte_to_send = 0;
  if (setting.length_wordspace != default_length_wordspace) {
    byte_to_send = byte_to_send | 1;
  }
  #ifdef FEATURE_AUTOSPACE
  if (configuration.autospace_active) {
    byte_to_send = byte_to_send | 2;
  }
  #endif
  if (flags.winkey_serial_echo) {

    byte_to_send = byte_to_send | 4;
  }
  if (setting.paddle_mode == PADDLE_REVERSE) {
    byte_to_send = byte_to_send | 8;
  }
  switch (setting.keyer_mode) {
    case IAMBIC_A: byte_to_send = byte_to_send | 16; break;
    case ULTIMATIC: byte_to_send = byte_to_send | 32; break;
    case BUG: byte_to_send = byte_to_send | 48; break;
  }
  if (flags.winkey_paddle_echo_activated) {
    byte_to_send = byte_to_send | 64;
  }
  #ifdef FEATURE_DEAD_OP_WATCHDOG
  if (!dead_op_watchdog_active) {
    byte_to_send = byte_to_send | 128;
  }
  #endif //FEATURE_DEAD_OP_WATCHDOG
  WinkeyPortWrite(byte_to_send,1);

  // 2 - speed
  if (setting.wpm > 99) {
    WinkeyPortWrite(99,1);
  } else {
    byte_to_send = setting.wpm;
    WinkeyPortWrite(byte_to_send,1);
  }
  // 3 - sidetone
	WinkeyPortWrite(5,1);

  // 4 - weight
  WinkeyPortWrite(setting.weighting,1);

  // 5 - ptt lead
  WinkeyPortWrite(255,1);

  // 6 - ptt tail
  //if (configuration.ptt_tail_time[configuration.current_tx-1] < 256){
    //WinkeyPortWrite((configuration.ptt_tail_time[configuration.current_tx-1] - (3*int(1200/configuration.wpm)))/10,1);
    WinkeyPortWrite(winkey_session_ptt_tail,1);
  // } else {
  //   WinkeyPortWrite(winkey_port_write(255,1);
  // }

  // 7 - pot min wpm
  WinkeyPortWrite(pot_wpm_low_value,1);

  // 8 - pot wpm range
  WinkeyPortWrite(pot_wpm_high_value - pot_wpm_low_value,1);

  // 9 - 1st extension
  WinkeyPortWrite(first_extension_time,1);

  // 10 - compensation
  WinkeyPortWrite(keying_compensation,1);

	// 11 - farnsworth wpm
 WinkeyPortWrite(setting.wpm_farnsworth,1);

  // 12 - paddle setpoint
  WinkeyPortWrite(50,1);  // default value

  // 13 - dah to dit ratio
  WinkeyPortWrite(50,1);  // TODO -backwards calculate

  // 14 - pin config
#ifdef OPTION_WINKEY_2_SUPPORT
    byte_to_send = 0;
    if (configuration.current_ptt_line != 0) {byte_to_send = byte_to_send | 1;}
    if ((configuration.sidetone_mode == SIDETONE_ON) || (configuration.sidetone_mode == SIDETONE_PADDLE_ONLY)) {byte_to_send = byte_to_send | 2;}
    if (current_tx_key_line == tx_key_line_1) {byte_to_send = byte_to_send | 4;}
    if (current_tx_key_line == tx_key_line_2) {byte_to_send = byte_to_send | 8;}

    #ifndef OPTION_NO_ULTIMATIC
    if (ultimatic_mode == ULTIMATIC_DIT_PRIORITY) {byte_to_send = byte_to_send | 128;}
    if (ultimatic_mode == ULTIMATIC_DAH_PRIORITY) {byte_to_send = byte_to_send | 64;}  
    #endif

    if (ptt_hang_time_wordspace_units == 1.33) {byte_to_send = byte_to_send | 16;}
    if (ptt_hang_time_wordspace_units == 1.66) {byte_to_send = byte_to_send | 32;}
    if (ptt_hang_time_wordspace_units == 2.0) {byte_to_send = byte_to_send | 48;}
    winkey_port_write(byte_to_send,1);
#else
    WinkeyPortWrite(5,1); // default value
#endif

  // 15 - pot range
  #ifdef OPTION_WINKEY_2_SUPPORT
    WinkeyPortWrite(zero,1);
  #else
    WinkeyPortWrite(0xFF,1);
  #endif

}

/** 
  * @brief 
  * @param 
  * @retval 
  */
void WinkeyEepromDownload()
{
  
  uint8_t zero = 0;
  unsigned int x = 0;
  unsigned int bytes_sent = 0;
  
  WinkeyPortWrite(0xA5,1); // 01 magic byte
  WinkeyAdminGetValuesCommand(); // 02-16
  
  WinkeyPortWrite((uint8_t)(setting.wpm),1); // 17 cmdwpm
  bytes_sent = 17;
  
  //pad the rest with zeros    
  for (x = 0;x < (256-bytes_sent); x++) {
    WinkeyPortWrite(zero,1);
  }  
}

/** 
  * @brief 
  * @param 
  * @retval 
  */
void TxKey(int state)
{
#ifdef DEBUG_WINKEY
		char izp[64];
#endif //DEBUG_WINKEY

#ifdef DEBUG
	sprintf(izp, "tx_key: %d %d\r\n", state, flags.key_state);
	USB_print(izp);
#endif //DEBUG_WINKEY
	
	if ((state) && (flags.key_state == 0)) {
      uint8_t previous_ptt_line_activated = flags.ptt_line_activated;
      PttKey();
      flags.key_state = 1;
			if ((first_extension_time) && (previous_ptt_line_activated == 0)) Delay(1);
    } else {
      if ((state == 0) && (flags.key_state)) {
        PttKey();
        flags.key_state = 0;
      }
    }
  CheckPttTail();
}


/** 
  * @brief 
  * @param 
  * @retval 
  */
void ServiceWinkey(uint8_t action, uint8_t incoming_serial_byte)
{
  static uint8_t winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
  static int winkey_parmcount = 0;

  static unsigned long winkey_last_activity;
  uint8_t status_byte_to_send;
  static uint8_t winkey_paddle_echo_space_sent = 1;
#ifdef DEBUG_WINKEY
		char izp[50];
#endif //DEBUG_WINKEY

  if (action == WINKEY_HOUSEKEEPING) {
    if (winkey_last_unbuffered_speed_wpm == 0) {
      winkey_last_unbuffered_speed_wpm = setting.wpm;
    }
    // Winkey interface emulation housekeeping items
    // check to see if we were sending stuff and the buffer is clear
    if (flags.winkey_interrupted) {
			// if Winkey sending was interrupted by the paddle, look at PTT line rather than timing out to send 0xc0
      if (flags.ptt_line_activated == 0) {
#ifdef DEBUG_WINKEY
        USB_print("WK: sending unsolicited status byte due to paddle interrupt\r\n");
#endif //DEBUG_WINKEY       
        winkey_sending = 0;
        ClearSendBuffer();
        flags.winkey_interrupted = 0;
        //winkey_port_write(0xc2|winkey_sending|winkey_xoff);  
        WinkeyPortWrite(0xc6,0);    //<- this alone makes N1MM logger get borked (0xC2 = paddle interrupt)
        WinkeyPortWrite(0xc0,0);    // so let's send a 0xC0 to keep N1MM logger happy 
        winkey_buffer_counter = 0;
        winkey_buffer_pointer = 0;  
      }
		} else { //if (flags.winkey_interrupted)
      //if ((winkey_host_open) && (winkey_sending) && (send_buffer_bytes < 1) && ((millis() - winkey_last_activity) > winkey_c0_wait_time)) {
      if ((flags.winkey_host_open) && (winkey_sending) && (send_buffer_bytes < 1) && ((HAL_GetTick() - winkey_last_activity) > 1)) {
        #ifdef OPTION_WINKEY_SEND_WORDSPACE_AT_END_OF_BUFFER
          SendChar(' ',KEYER_NORMAL);
        #endif
        //add_to_send_buffer(' ');    // this causes a 0x20 to get echoed back to host - doesn't seem to effect N1MM program
#ifdef DEBUG_WINKEY
        USB_print("Sending unsolicited status byte\r\n");
#endif //DEBUG_WINKEY           
        winkey_sending = 0;
        WinkeyPortWrite(0xc0|winkey_sending|winkey_xoff,0);    // tell the host we've sent everything
        winkey_buffer_counter = 0;
        winkey_buffer_pointer = 0;
      }
		}  // if (flags.winkey_interrupted)
		
		// failsafe check - if we've been in some command status for awhile waiting for something, clear things out
    if ((winkey_status != WINKEY_NO_COMMAND_IN_PROGRESS) && ((HAL_GetTick() - winkey_last_activity) > 5000)) {
#ifdef DEBUG_WINKEY
      USB_print("WK: cmd tout!->WINKEY_NO_COMMAND_IN_PROGRESS cmd was: ");
			sprintf(izp, "%d\r\n", winkey_status);
			USB_print(izp);
#endif //DEBUG_WINKEY      
      winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      winkey_buffer_counter = 0;
      winkey_buffer_pointer = 0;
      WinkeyPortWrite(0xc0|winkey_sending|winkey_xoff,0);    //send a status byte back for giggles
    }  

		if ((flags.winkey_host_open) && (winkey_paddle_echo_buffer) && (winkey_paddle_echo_activated) && (HAL_GetTick() > winkey_paddle_echo_buffer_decode_time)) {
#ifdef DEBUG_WINKEY
      USB_print("WK: sending paddle echo char\r\n");
#endif //DEBUG_WINKEY       
      WinkeyPortWrite((uint8_t)(convert_cw_number_to_ascii(winkey_paddle_echo_buffer)),0);
      winkey_paddle_echo_buffer = 0;
      winkey_paddle_echo_buffer_decode_time = HAL_GetTick() + ((uint32_t)(600/setting.wpm)*length_letterspace);
      winkey_paddle_echo_space_sent = 0;
    }

    if ((flags.winkey_host_open) && (winkey_paddle_echo_buffer == 0) && (winkey_paddle_echo_activated) && (HAL_GetTick() > (winkey_paddle_echo_buffer_decode_time + ((uint32_t)(1200/setting.wpm)*(setting.length_wordspace-length_letterspace)))) && (!winkey_paddle_echo_space_sent)) {
#ifdef DEBUG_WINKEY
      USB_print("WK: sending paddle echo space");
#endif //DEBUG_WINKEY        
      WinkeyPortWrite(' ',0);
      winkey_paddle_echo_space_sent = 1;
    }
	}  // if (action == WINKEY_HOUSEKEEPING)

	if (action == SERVICE_SERIAL_BYTE){	
#ifdef DEBUG_WINKEY
		sprintf(izp, "WK RX: 0x%X\r\n", incoming_serial_byte);
		USB_print(izp);
#endif //DEBUG_WINKEY
		winkey_last_activity = HAL_GetTick();
		 
		if (winkey_status == WINKEY_NO_COMMAND_IN_PROGRESS){ 
#if defined(OPTION_WINKEY_EXTENDED_COMMANDS_WITH_DOLLAR_SIGNS)
	#if !defined(OPTION_WINKEY_IGNORE_LOWERCASE)
			if ((incoming_serial_byte > 31) && (incoming_serial_byte != 36)) {  // ascii 36 = '$'
	#else // defined(OPTION_WINKEY_IGNORE_LOWERCASE)
			if ((((incoming_serial_byte > 31) && (incoming_serial_byte < 97)) || (incoming_serial_byte == 124)) && (incoming_serial_byte != 36)) {  // 124 = ascii | = half dit
	#endif // defined(OPTION_WINKEY_IGNORE_LOWERCASE)
          
#else // !OPTION_WINKEY_EXTENDED_COMMANDS_WITH_DOLLAR_SIGNS
	#if !defined(OPTION_WINKEY_IGNORE_LOWERCASE)
			if (incoming_serial_byte > 31) {
	#else // defined(OPTION_WINKEY_IGNORE_LOWERCASE)
          if (((incoming_serial_byte > 31) && (incoming_serial_byte < 97)) || (incoming_serial_byte == 124)) {  // 124 = ascii | = half dit
	#endif // defined(OPTION_WINKEY_IGNORE_LOWERCASE)
#endif // !OPTION_WINKEY_EXTENDED_COMMANDS_WITH_DOLLAR_SIGNS

#if !defined(OPTION_WINKEY_IGNORE_LOWERCASE)
				if ((incoming_serial_byte > 96) && (incoming_serial_byte < 123)){incoming_serial_byte = incoming_serial_byte - 32;}
#endif //!defined(OPTION_WINKEY_IGNORE_LOWERCASE)
      
				uint8_t serial_buffer_position_to_overwrite;
				if (winkey_buffer_pointer > 0) {
          serial_buffer_position_to_overwrite = send_buffer_bytes - (winkey_buffer_counter - winkey_buffer_pointer) - 1;
          if ((send_buffer_bytes) && (serial_buffer_position_to_overwrite < send_buffer_bytes )) {
#ifdef DEBUG_WINKEY
						USB_print("service_winkey:serial_buffer_position_to_overwrite:");
						sprintf(izp, " %d: %d\r\n", serial_buffer_position_to_overwrite, incoming_serial_byte);
						USB_print(izp);
#endif //DEBUG_WINKEY 
            send_buffer_array[serial_buffer_position_to_overwrite] = incoming_serial_byte;
          }
          winkey_buffer_pointer++;
        } else {
          AddToSendBuffer(incoming_serial_byte);
#ifdef DEBUG_WINKEY
          USB_print("WK: add_to_send_buffer:");
					sprintf(izp, " %d: %d\r\n", incoming_serial_byte, send_buffer_bytes);
					USB_print(izp);
#endif //DEBUG_WINKEY 
          winkey_buffer_counter++;
        }

        if (!winkey_sending) {
#ifdef DEBUG_WINKEY
					USB_print("WK: status byte: starting to send\r\n");
#endif //DEBUG_WINKEY          
          winkey_sending=0x04;
#if !defined(OPTION_WINKEY_UCXLOG_SUPRESS_C4_STATUS_BYTE)
					WinkeyPortWrite(0xc4|winkey_sending|winkey_xoff,0);  // tell the client we're starting to send
#endif //OPTION_WINKEY_UCXLOG_SUPRESS_C4_STATUS_BYTE
        }
      } else {
#ifdef OPTION_WINKEY_STRICT_HOST_OPEN
				if ((winkey_host_open) || (incoming_serial_byte == 0)) {
#endif // OPTION_WINKEY_STRICT_HOST_OPEN
				switch (incoming_serial_byte) {
          case 0x00:
            winkey_status = WINKEY_ADMIN_COMMAND;
#ifdef DEBUG_WINKEY
              USB_print("WK: ADMIN_CMD\r\n");
#endif //DEBUG_WINKEY
            break;
          case 0x01:
            winkey_status = WINKEY_SIDETONE_FREQ_COMMAND;
            break;
					
          case 0x02:  // speed command - unbuffered
            winkey_status = WINKEY_UNBUFFERED_SPEED_COMMAND;
#ifdef DEBUG_WINKEY
            USB_print("WINKEY_UNBUFFERED_SPEED_COMMAND\r\n");
#endif //DEBUG_WINKEY               
            break;
          case 0x03:  // weighting
            winkey_status = WINKEY_WEIGHTING_COMMAND;
#ifdef DEBUG_WINKEY
            USB_print("WINKEY_WEIGHTING_COMMAND\r\n");
#endif //DEBUG_WINKEY               
            break;
          case 0x04: // PTT lead and tail time
            winkey_status = WINKEY_PTT_TIMES_PARM1_COMMAND;
#ifdef DEBUG_WINKEY
            USB_print("WINKEY_PTT_TIMES_PARM1_COMMAND\r\n");
#endif //DEBUG_WINKEY               
            break;
          case 0x05:     // speed pot set
#ifdef DEBUG_WINKEY
					  USB_print("WINKEY_SET_POT_PARM1_COMMAND\r\n");
#endif //DEBUG_WINKEY
            winkey_status = WINKEY_SET_POT_PARM1_COMMAND;
            break;
          case 0x06:
            winkey_status = WINKEY_PAUSE_COMMAND;
#ifdef DEBUG_WINKEY
            USB_print("WINKEY_PAUSE_COMMAND\r\n");
#endif //DEBUG_WINKEY               
            break;
          case 0x07:
            WinkeyPortWrite(((PotValueWPM()-pot_wpm_low_value)|128),0);
#ifdef DEBUG_WINKEY
            USB_print("WK: report pot\r\n");
#endif //DEBUG_WINKEY               
            break;
          case 0x08:    // backspace command
            if (send_buffer_bytes) {
              send_buffer_bytes--;
            }
#ifdef DEBUG_WINKEY
            USB_print("WK: backspace\r\n");
#endif //DEBUG_WINKEY               
            break;
          case 0x09:
#ifdef DEBUG_WINKEY
            USB_print("WINKEY_SET_PINCONFIG_COMMAND\r\n");
#endif //DEBUG_WINKEY             
            winkey_status = WINKEY_SET_PINCONFIG_COMMAND;
            break;
          case 0x0a:                 // 0A - clear buffer - no parms
            ClearSendBuffer();
            if (winkey_sending) {
              //clear_send_buffer();
              winkey_sending = 0;
              WinkeyPortWrite(0xc0|winkey_sending|winkey_xoff,0);
            }
            flags.pause_sending_buffer = 0;
            winkey_buffer_counter = 0;
            winkey_buffer_pointer = 0;
            #if !defined(OPTION_DISABLE_SERIAL_PORT_CHECKING_WHILE_SENDING_CW)
              flags.loop_element_lengths_breakout_flag = 0;
            #endif
            sending_mode = AUTOMATIC_SENDING;
            flags.manual_ptt_invoke = 0;
            TxKey(0); 
            winkey_speed_state = WINKEY_UNBUFFERED_SPEED;
            setting.wpm = winkey_last_unbuffered_speed_wpm;
#ifdef DEBUG_WINKEY
          USB_print("WK: 0A clearbuff send_buffer_bytes: ");
					sprintf(izp, "%d\r\n", send_buffer_bytes);
					USB_print(izp);
#endif //DEBUG_WINKEY 
            break;
          case 0x0b:
            winkey_status = WINKEY_KEY_COMMAND;
#ifdef DEBUG_WINKEY
            USB_print("WINKEY_KEY_COMMAND\r\n");
#endif //DEBUG_WINKEY              
            break;
          case 0x0c:
            winkey_status = WINKEY_HSCW_COMMAND;
#ifdef DEBUG_WINKEY
            USB_print("WINKEY_HSCW_COMMAND\r\n");
#endif //DEBUG_WINKEY                 
            break;
          case 0x0d:
            winkey_status = WINKEY_FARNSWORTH_COMMAND;
#ifdef DEBUG_WINKEY
            USB_print("WINKEY_FARNSWORTH_COMMAND\r\n");
#endif //DEBUG_WINKEY                 
            break;
          case 0x0e:
            winkey_status = WINKEY_SETMODE_COMMAND;
#ifdef DEBUG_WINKEY
            USB_print("WINKEY_SETMODE_COMMAND\r\n");
#endif //DEBUG_WINKEY                 
            break;
          case 0x0f:  // bulk load of defaults
            winkey_status = WINKEY_LOAD_SETTINGS_PARM_1_COMMAND;
#ifdef DEBUG_WINKEY
            USB_print("WINKEY_LOAD_SETTINGS_PARM_1_COMMAND\r\n");
#endif //DEBUG_WINKEY                 
            break;
          case 0x10:
            winkey_status = WINKEY_FIRST_EXTENSION_COMMAND;
#ifdef DEBUG_WINKEY
            USB_print("WINKEY_FIRST_EXTENSION_COMMAND\r\n");
#endif //DEBUG_WINKEY                 
            break;
          case 0x11:
            winkey_status = WINKEY_KEYING_COMPENSATION_COMMAND;
#ifdef DEBUG_WINKEY
            USB_print("WINKEY_KEYING_COMPENSATION_COMMAND\r\n");
#endif //DEBUG_WINKEY                 
            break;
          case 0x12:
            winkey_status = WINKEY_UNSUPPORTED_COMMAND;
#ifdef DEBUG_WINKEY
            USB_print("WK: 0x12 unsupport\r\n");
#endif //DEBUG_WINKEY     
            winkey_parmcount = 1;
            break;
          case 0x13:  // NULL command
#ifdef DEBUG_WINKEY
            USB_print("WK: 0x13 null\r\n");
#endif //DEBUG_WINKEY               
            break;
          case 0x14:
            winkey_status = WINKEY_SOFTWARE_PADDLE_COMMAND;
#ifdef DEBUG_WINKEY
              USB_print("WINKEY_SOFTWARE_PADDLE_COMMAND\r\n");
#endif //DEBUG_WINKEY                 
            break;
          case 0x15:  // report status
#ifndef OPTION_WINKEY_IGNORE_FIRST_STATUS_REQUEST //--------------------
              status_byte_to_send = 0xc0|winkey_sending|winkey_xoff;
              if (send_buffer_status == SERIAL_SEND_BUFFER_TIMED_COMMAND) {
                status_byte_to_send = status_byte_to_send | 16;
              }
              WinkeyPortWrite(status_byte_to_send,0);
#ifdef DEBUG_WINKEY
              USB_print("WK: 0x15 rpt status: ");
							sprintf(izp, "%d\r\n", status_byte_to_send);
							USB_print(izp);
#endif //DEBUG_WINKEY  
#else //OPTION_WINKEY_IGNORE_FIRST_STATUS_REQUEST ------------------------
              if (ignored_first_status_request){
                status_byte_to_send = 0xc0|winkey_sending|winkey_xoff;
                if (send_buffer_status == SERIAL_SEND_BUFFER_TIMED_COMMAND) {
                  status_byte_to_send = status_byte_to_send | 16;
                }
                winkey_port_write(status_byte_to_send,0);
	#ifdef DEBUG_WINKEY
                USB_print("WK: 0x15 rpt status:");
  							sprintf(izp, "%d\r\n", status_byte_to_send);
	  						USB_print(izp);
	#endif //DEBUG_WINKEY 
                } else {
                  ignored_first_status_request = 1;
	#ifdef DEBUG_WINKEY
                  USB_print("service_winkey:ignored first 0x15 status request\r\n");
	#endif //DEBUG_WINKEY                
                }
#endif //OPTION_WINKEY_IGNORE_FIRST_STATUS_REQUEST -------------------- 
            break;
          case 0x16:  // Pointer operation
            winkey_status = WINKEY_POINTER_COMMAND;
#ifdef DEBUG_WINKEY
            USB_print("WINKEY_POINTER_COMMAND\r\n");
#endif //DEBUG_WINKEY                 
            break;
          case 0x17:  // dit to dah ratio
            winkey_status = WINKEY_DAH_TO_DIT_RATIO_COMMAND;
#ifdef DEBUG_WINKEY
            USB_print("WINKEY_DAH_TO_DIT_RATIO_COMMAND\r\n");
#endif //DEBUG_WINKEY                 
            break;
          // start of buffered commands ------------------------------
          case 0x18:   //buffer PTT on/off
            winkey_status = WINKEY_BUFFFERED_PTT_COMMMAND;
#ifdef DEBUG_WINKEY
            USB_print("WINKEY_BUFFFERED_PTT_COMMMAND\r\n");
#endif //DEBUG_WINKEY            
            break;
          case 0x19:
            winkey_status = WINKEY_KEY_BUFFERED_COMMAND;
#ifdef DEBUG_WINKEY
            USB_print("WINKEY_KEY_BUFFERED_COMMAND\r\n");
#endif //DEBUG_WINKEY            
            break;
          case 0x1a:
            winkey_status = WINKEY_WAIT_BUFFERED_COMMAND;
#ifdef DEBUG_WINKEY
            USB_print("WINKEY_WAIT_BUFFERED_COMMAND\r\n");
#endif //DEBUG_WINKEY            
            break;
          case 0x1b:
            winkey_status = WINKEY_MERGE_COMMAND;
#ifdef DEBUG_WINKEY
					USB_print("WINKEY_MERGE_COMMAND\r\n");
#endif //DEBUG_WINKEY            
            break;
          case 0x1c:      // speed command - buffered
             winkey_status = WINKEY_BUFFERED_SPEED_COMMAND;
#ifdef DEBUG_WINKEY
					USB_print("WINKEY_BUFFERED_SPEED_COMMAND\r\n");
#endif //DEBUG_WINKEY             
            break;
          case 0x1d:
            winkey_status = WINKEY_BUFFERED_HSCW_COMMAND;
#ifdef DEBUG_WINKEY
            USB_print("WINKEY_BUFFERED_HSCW_COMMAND\r\n");
#endif //DEBUG_WINKEY            
            break;
          case 0x1e:  // cancel buffered speed command - buffered
#ifdef DEBUG_WINKEY
              USB_print("WINKEY_CANCEL_BUFFERED_SPEED_COMMAND\r\n");
#endif //DEBUG_WINKEY     
            if (winkey_speed_state == WINKEY_BUFFERED_SPEED){
              AddToSendBuffer(SERIAL_SEND_BUFFER_WPM_CHANGE);
              AddToSendBuffer(0);
              AddToSendBuffer((uint8_t)winkey_last_unbuffered_speed_wpm);
              winkey_speed_state = WINKEY_UNBUFFERED_SPEED;
#ifdef DEBUG_WINKEY
              USB_print("winkey_speed_state = WINKEY_UNBUFFERED_SPEED\r\n");
#endif //DEBUG_WINKEY 
            } else {
#ifdef DEBUG_WINKEY
              USB_print("WINKEY_CANCEL_BUFFERED_SPEED_COMMAND: no action\r\n");
#endif //DEBUG_WINKEY         
            }       
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
            break;
          case 0x1f:  // buffered NOP - no need to do anything
#ifdef DEBUG_WINKEY
            USB_print("WK: 0x1f NOP\r\n");
#endif //DEBUG_WINKEY          
            break;

#ifdef OPTION_WINKEY_EXTENDED_COMMANDS_WITH_DOLLAR_SIGNS
            case 36:
              winkey_status = WINKEY_EXTENDED_COMMAND;
#ifdef DEBUG_WINKEY
                USB_print("WINKEY_EXTENDED_COMMAND\r\n");
	#endif //DEBUG_WINKEY  
              break;
#endif //OPTION_WINKEY_EXTENDED_COMMANDS_WITH_DOLLAR_SIGNS
        } //switch (incoming_serial_byte)

        #ifdef OPTION_WINKEY_STRICT_HOST_OPEN
        } //if ((winkey_host_open) || (incoming_serial_byte == 0))
        #endif
				}
    } else { //if (winkey_status == WINKEY_NO_COMMAND_IN_PROGRESS) 		
			
      if (winkey_status == WINKEY_UNSUPPORTED_COMMAND) {
        winkey_parmcount--;
#ifdef DEBUG_WINKEY
        USB_print("WINKEY_UNSUPPORTED_COMMAND winkey_parmcount:");
				sprintf(izp, " %d\r\n", winkey_parmcount);
				USB_print(izp);
#endif //DEBUG_WINKEY          
        if (winkey_parmcount == 0) {
          WinkeyPortWrite(0xc0|winkey_sending|winkey_xoff,0);           
          winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
#ifdef DEBUG_WINKEY
          USB_print("WINKEY_UNSUPPORTED_COMMAND: WINKEY_NO_COMMAND_IN_PROGRESS");
					sprintf(izp, " %d\r\n", winkey_parmcount);
					USB_print(izp);
#endif //DEBUG_WINKEY          
        }
      }
			
      //WINKEY_LOAD_SETTINGS_PARM_1_COMMAND IS 101
      if ((winkey_status > 100) && (winkey_status < 116)) {   // Load Settings Command - this has 15 parameters, so we handle it a bit differently
        WinkeyLoadSettingsCommand(winkey_status,incoming_serial_byte);
        winkey_status++;
        if (winkey_status > 115) {
          winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
#ifdef DEBUG_WINKEY
          USB_print("WINKEY_LOAD_SETTINGS_PARM_15->NOCMD\r\n");
#endif //DEBUG_WINKEY            
        }
      }

#ifdef OPTION_WINKEY_EXTENDED_COMMANDS_WITH_DOLLAR_SIGNS
      if (winkey_status == WINKEY_EXTENDED_COMMAND) {  
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }
#endif //OPTION_WINKEY_EXTENDED_COMMANDS_WITH_DOLLAR_SIGNS

      if (winkey_status == WINKEY_SET_PINCONFIG_COMMAND) {
        WinkeySetPinconfigCommand(incoming_serial_byte);
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

      if (winkey_status == WINKEY_MERGE_COMMAND) {
        AddToSendBuffer(SERIAL_SEND_BUFFER_PROSIGN);
        AddToSendBuffer(incoming_serial_byte);
        winkey_status = WINKEY_MERGE_PARM_2_COMMAND;
      } else {
        if (winkey_status == WINKEY_MERGE_PARM_2_COMMAND) {
          AddToSendBuffer(incoming_serial_byte);
          winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
        }
      }

      if (winkey_status == WINKEY_UNBUFFERED_SPEED_COMMAND) {
        WinkeyUnbufferedSpeedCommand(incoming_serial_byte);
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

			if (winkey_status == WINKEY_FARNSWORTH_COMMAND) {
        WinkeyFarnsworthCommand(incoming_serial_byte);
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }
			
      if (winkey_status ==  WINKEY_HSCW_COMMAND) {
        if (incoming_serial_byte == 0) {
            setting.pot_activated = 1;
        } else {
          setting.wpm = ((incoming_serial_byte*100)/5);
          winkey_last_unbuffered_speed_wpm = setting.wpm;
					flags.config_dirty = 1;
        }
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

			if (winkey_status == WINKEY_BUFFERED_SPEED_COMMAND) {
#ifdef DEBUG_WINKEY
        USB_print("WK: BUFFERED_SPEED_CMD:send_buffer_bytes: ");
				sprintf(izp, "%d\r\n", send_buffer_bytes);
				USB_print(izp);
#endif //DEBUG_WINKEY         
        AddToSendBuffer(SERIAL_SEND_BUFFER_WPM_CHANGE);
        AddToSendBuffer(0);
        AddToSendBuffer(incoming_serial_byte);
#ifdef DEBUG_WINKEY
        USB_print("WK: BUFFERED_SPEED_CMD:send_buffer_bytes: ");
				sprintf(izp, "%d\r\n", send_buffer_bytes);
				USB_print(izp);
#endif //DEBUG_WINKEY         
#ifdef DEBUG_WINKEY
        USB_print("WK: WINKEY_BUFFERED_SPEED_COMMAND->NOCMD\r\n");
#endif //DEBUG_WINKEY           
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

			if (winkey_status == WINKEY_BUFFERED_HSCW_COMMAND) {   
        if (incoming_serial_byte > 1){  // the HSCW command is overloaded; 0 = buffered TX 1, 1 = buffered TX 2, > 1 = HSCW WPM
          unsigned int send_buffer_wpm = ((incoming_serial_byte*100)/5);
          AddToSendBuffer(SERIAL_SEND_BUFFER_WPM_CHANGE);
          AddToSendBuffer(highByte(send_buffer_wpm));
          AddToSendBuffer(lowByte(send_buffer_wpm));
        } else {
          AddToSendBuffer(SERIAL_SEND_BUFFER_TX_CHANGE);
          AddToSendBuffer(incoming_serial_byte+1);
        }
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }
			
			if (winkey_status == WINKEY_KEY_BUFFERED_COMMAND) {
        AddToSendBuffer(SERIAL_SEND_BUFFER_TIMED_KEY_DOWN);
        AddToSendBuffer(incoming_serial_byte);
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }
			
      if (winkey_status == WINKEY_WAIT_BUFFERED_COMMAND) {
        AddToSendBuffer(SERIAL_SEND_BUFFER_TIMED_WAIT);
        AddToSendBuffer(incoming_serial_byte);
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }
			
      if (winkey_status == WINKEY_BUFFFERED_PTT_COMMMAND) {
        if (incoming_serial_byte) {
          AddToSendBuffer(SERIAL_SEND_BUFFER_PTT_ON);
        } else {
          AddToSendBuffer(SERIAL_SEND_BUFFER_PTT_OFF);
        }
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

      if (winkey_status == WINKEY_POINTER_01_COMMAND) { // move input pointer to new positon in overwrite mode
        winkey_buffer_pointer = incoming_serial_byte;
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
#ifdef DEBUG_WINKEY
        USB_print("WK: PTR1_CMD->NOCMD winkey_buffer_pointer: ");
				sprintf(izp, "%d\r\n", winkey_buffer_pointer);
				USB_print(izp);
        USB_print("send_buffer_bytes: ");
				sprintf(izp, "%d\r\n", send_buffer_bytes);
				USB_print(izp);

#endif //DEBUG_WINKEY          
      }

			if (winkey_status == WINKEY_POINTER_02_COMMAND) { // move input pointer to new position in append mode
        winkey_buffer_pointer = 0;
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
#ifdef DEBUG_WINKEY
        USB_print("WK: PTR2_CMD->NOCMD send_buffer_bytes: ");
				sprintf(izp, "%d", send_buffer_bytes);
				USB_print(izp);
        USB_print(" winkey_buffer_counter: ");
				sprintf(izp, "%d", winkey_buffer_counter);
				USB_print(izp);
        USB_print(" winkey_buffer_pointer: ");
				sprintf(izp, "%d\r\n", winkey_buffer_pointer);
				USB_print(izp);
#endif //DEBUG_WINKEY          
      }

			if (winkey_status == WINKEY_POINTER_03_COMMAND) { // add multiple nulls to buffer
#ifdef DEBUG_WINKEY
        USB_print("WK: PTR3_CMD send_buffer_bytes: ");
        sprintf(izp, "%d", send_buffer_bytes);
				USB_print(izp);
        USB_print(" winkey_buffer_counter:");
        sprintf(izp, "%d", winkey_buffer_counter);
				USB_print(izp);
        USB_print(" winkey_buffer_pointer: ");
				sprintf(izp, "%d\r\n", winkey_buffer_pointer);
				USB_print(izp);
#endif //DEBUG_WINKEY       
        uint8_t serial_buffer_position_to_overwrite;
        for (uint8_t x = incoming_serial_byte; x > 0; x--) {
          if (winkey_buffer_pointer > 0) {
            serial_buffer_position_to_overwrite = send_buffer_bytes - (winkey_buffer_counter - winkey_buffer_pointer) - 1;
            if ((send_buffer_bytes) && (serial_buffer_position_to_overwrite < send_buffer_bytes )) {
              send_buffer_array[serial_buffer_position_to_overwrite] = SERIAL_SEND_BUFFER_NULL;
            }
            winkey_buffer_pointer++;
          } else {
              AddToSendBuffer(SERIAL_SEND_BUFFER_NULL);
              winkey_buffer_counter++;
          }
        }
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
#ifdef DEBUG_WINKEY
        USB_print("WK: PTR3_CMD->NO_CMD send_buffer_bytes: ");
        sprintf(izp, "%d", send_buffer_bytes);
				USB_print(izp);
        USB_print(" winkey_buffer_counter: ");
        sprintf(izp, "%d", winkey_buffer_counter);
				USB_print(izp);
        USB_print(" winkey_buffer_pointer: ");
				sprintf(izp, "%d\r\n", winkey_buffer_pointer);
				USB_print(izp);
#endif //DEBUG_WINKEY 
      }

			if (winkey_status == WINKEY_POINTER_COMMAND) {
        switch (incoming_serial_byte) {
          case 0x00:
            winkey_buffer_counter = 0;
            winkey_buffer_pointer = 0;
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
#ifdef DEBUG_WINKEY
            USB_print("WK: PTR_CMD->NOCMD send_buffer_bytes: ");
						sprintf(izp, "%d", send_buffer_bytes);
						USB_print(izp);
            USB_print(" winkey_buffer_counter: ");
						sprintf(izp, "%d", winkey_buffer_counter);
						USB_print(izp);
            USB_print(" winkey_buffer_pointer: ");
						sprintf(izp, "%d\r\n", winkey_buffer_pointer);
						USB_print(izp);
#endif //DEBUG_WINKEY               
            break;
          case 0x01:
            winkey_status = WINKEY_POINTER_01_COMMAND;
#ifdef DEBUG_WINKEY
						USB_print("WK: PTR1_CMD\r\n");
#endif //DEBUG_WINKEY              
            break;
          case 0x02:
            winkey_status = WINKEY_POINTER_02_COMMAND;  // move to new position in append mode
#ifdef DEBUG_WINKEY
            USB_print("WK: PTR2_CMD\r\n");
#endif //DEBUG_WINKEY            
            break;
          case 0x03:
            winkey_status = WINKEY_POINTER_03_COMMAND;
#ifdef DEBUG_WINKEY
            USB_print("WK: PTR3_CMD\r\n");
#endif //DEBUG_WINKEY            
            break;
          default:
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
#ifdef DEBUG_WINKEY
            USB_print("WK: PTR_CMD->NOCMD\r\n");
#endif //DEBUG_WINKEY            
            break;
        }
      }
			
#ifdef OPTION_WINKEY_2_SUPPORT
      if (winkey_status == WINKEY_SEND_MSG) {
        if ((incoming_serial_byte > 0) && (incoming_serial_byte < 7)) {
          add_to_send_buffer(SERIAL_SEND_BUFFER_MEMORY_NUMBER);
          add_to_send_buffer(incoming_serial_byte - 1);
        }
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;  
      }
#endif //OPTION_WINKEY_2_SUPPORT

      if (winkey_status == WINKEY_ADMIN_COMMAND) {
        switch (incoming_serial_byte) {
          case 0x00: 
            winkey_status = WINKEY_UNSUPPORTED_COMMAND;
            winkey_parmcount = 1;
#ifdef DEBUG_WINKEY
					USB_print("WK: UNSUPPORTED_COMMAND await 1 parm\r\n");
#endif //DEBUG_WINKEY            
            break;  // calibrate command
          case 0x01:
#ifdef DEBUG_WINKEY
            USB_print("WINKEY_ADMIN_COMMAND 0x01\r\n");
#endif //DEBUG_WINKEY          
            HAL_NVIC_SystemReset();
            break;  // reset command
          case 0x02:  // host open command - send version back to host
#ifdef OPTION_WINKEY_2_SUPPORT
              winkey_port_write(WINKEY_2_REPORT_VERSION_NUMBER,1);
#else //OPTION_WINKEY_2_SUPPORT
              WinkeyPortWrite(WINKEY_1_REPORT_VERSION_NUMBER,1);
#endif //OPTION_WINKEY_2_SUPPORT
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
            flags.manual_ptt_invoke = 0;
            flags.winkey_host_open = 1;
#ifdef DEBUG_WINKEY
					USB_print("WK: hostopen\r\n");
#endif //DEBUG_WINKEY  
            break;
          case 0x03: // host close command
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
            flags.manual_ptt_invoke = 0;
            flags.winkey_host_open = 0;
#ifdef OPTION_WINKEY_SEND_VERSION_ON_HOST_CLOSE
	#ifdef OPTION_WINKEY_2_SUPPORT
            winkey_port_write(WINKEY_2_REPORT_VERSION_NUMBER,1);
  #else //OPTION_WINKEY_2_SUPPORT
            winkey_port_write(WINKEY_1_REPORT_VERSION_NUMBER,1);
  #endif //OPTION_WINKEY_2_SUPPORT 
#endif  //OPTION_WINKEY_SEND_VERSION_ON_HOST_CLOSE           
#ifdef DEBUG_WINKEY
            USB_print("WK: hostclose\r\n");
#endif //DEBUG_WINKEY                  
            flags.config_dirty = 1;
            break;
          case 0x04:  // echo command
            winkey_status = WINKEY_ADMIN_COMMAND_ECHO;
#ifdef DEBUG_WINKEY
            USB_print("WK: ADMIN_CMD_ECHO\r\n");
#endif //DEBUG_WINKEY              
            break;
          case 0x05: // paddle A2D
            WinkeyPortWrite(WINKEY_RETURN_THIS_FOR_ADMIN_PADDLE_A2D,0);
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
#ifdef DEBUG_WINKEY
					USB_print("WK: paddleA2D\r\n");
#endif //DEBUG_WINKEY              
            break;
          case 0x06: // speed A2D
            WinkeyPortWrite(WINKEY_RETURN_THIS_FOR_ADMIN_SPEED_A2D,0);
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
#ifdef DEBUG_WINKEY
            USB_print("WK: speedA2D\r\n");
#endif //DEBUG_WINKEY              
            break;
          case 0x07: // Get values
#ifdef DEBUG_WINKEY
            USB_print("winkey_admin_get_values\r\n");
#endif //DEBUG_WINKEY             
            WinkeyAdminGetValuesCommand();
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
            break;
          case 0x08: // reserved
#ifdef DEBUG_WINKEY
						USB_print("WK: ADMIN_CMD 0x08 reserved-WTF\r\n");
#endif //DEBUG_WINKEY              
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
            break;  
          case 0x09: // get cal
#ifdef DEBUG_WINKEY
            USB_print("WK: ADMIN_CMD getcal\r\n");
#endif //DEBUG_WINKEY           
            WinkeyPortWrite(WINKEY_RETURN_THIS_FOR_ADMIN_GET_CAL,0);
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
            break;
#ifdef OPTION_WINKEY_2_SUPPORT
          case 0x0a: // set wk1 mode (10)
            wk2_mode = 1;
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
#ifdef DEBUG_WINKEY
            USB_print("WK: ADMIN_CMD wk2_mode1\r\n");
#endif //DEBUG_WINKEY              
            break;
          case 0x0b: // set wk2 mode (11)
#ifdef DEBUG_WINKEY
            USB_print("WK: ADMIN_CMD wk2_mode2\r\n");
#endif //DEBUG_WINKEY              
            wk2_mode = 2;
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
            break;            
          case 0x0c: // download EEPPROM 256 bytes (12)
#ifdef DEBUG_WINKEY
            USB_print("WK: ADMIN_CMD winkey_eeprom_download\r\n");
#endif //DEBUG_WINKEY           
            winkey_eeprom_download();
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
            break;   
          case 0x0d: // upload EEPROM 256 bytes (13)
#ifdef DEBUG_WINKEY
            USB_print("WK: ADMIN_CMD uploadEEPROM\r\n");
#endif //DEBUG_WINKEY           
            winkey_status = WINKEY_UNSUPPORTED_COMMAND;  // upload EEPROM 256 bytes
            winkey_parmcount = 256;
            break;       
          case 0x0e: //(14)
#ifdef DEBUG_WINKEY
            USB_print("WK: ADMIN_CMD WINKEY_SEND_MSG\r\n");
#endif //DEBUG_WINKEY          
            winkey_status = WINKEY_SEND_MSG;
            break;
          case 0x0f: // load xmode (15)
            winkey_status = WINKEY_UNSUPPORTED_COMMAND;
            winkey_parmcount = 1;
#ifdef DEBUG_WINKEY
            USB_print("WK: ADMIN_CMD loadxmode\r\n");
#endif //DEBUG_WINKEY            
            break;            
          case 0x10: // reserved (16)
            winkey_port_write(zero,0);
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
            break;
          case 0x11: // set high baud rate (17)
#ifdef DEBUG_WINKEY
            USB_print("WK: ADMIN_CMD sethighbaudrate\r\n");
#endif //DEBUG_WINKEY            
            primary_serial_port->end();
            primary_serial_port->begin(9600);
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
            break;
          case 0x12: // set low baud rate (18)
#ifdef DEBUG_WINKEY
            USB_print("ADMIN_CMD setlowbaudrate\r\n");
#endif //DEBUG_WINKEY           
            primary_serial_port->end();
            primary_serial_port->begin(1200);
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
            break;
#endif //OPTION_WINKEY_2_SUPPORT  

          default:
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
#ifdef DEBUG_WINKEY
            USB_print("ADMIN_CMD->NOCMD\r\n");
#endif //DEBUG_WINKEY             
            break;
          }
      } else {
        if (winkey_status == WINKEY_ADMIN_COMMAND_ECHO) {
#ifdef DEBUG_WINKEY
          USB_print("WK: echoabyte\r\n");
#endif //DEBUG_WINKEY  
          WinkeyPortWrite(incoming_serial_byte,1);
          winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
        }
      }

      if (winkey_status == WINKEY_KEYING_COMPENSATION_COMMAND) {
#ifdef DEBUG_WINKEY
        USB_print("WINKEY_KEYING_COMPENSATION_COMMAND byte\r\n");
#endif //DEBUG_WINKEY         
        keying_compensation = incoming_serial_byte;
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }
			
      if (winkey_status == WINKEY_FIRST_EXTENSION_COMMAND) {
#ifdef DEBUG_WINKEY
        USB_print("WINKEY_FIRST_EXTENSION_COMMAND byte\r\n");
#endif //DEBUG_WINKEY              
        first_extension_time = incoming_serial_byte;
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

      if (winkey_status == WINKEY_PAUSE_COMMAND) {
        if (incoming_serial_byte) {
#ifdef DEBUG_WINKEY
          USB_print("WINKEY_PAUSE_COMMAND pause\r\n");
#endif //DEBUG_WINKEY           
          flags.pause_sending_buffer = 1;
        } else {
#ifdef DEBUG_WINKEY
          USB_print("WINKEY_PAUSE_COMMAND unpause\r\n");
#endif //DEBUG_WINKEY             
          flags.pause_sending_buffer = 0;
        }
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

			if (winkey_status ==  WINKEY_KEY_COMMAND) {
        sending_mode = AUTOMATIC_SENDING;
        if (incoming_serial_byte) {
          TxKey(1);
        } else {
          TxKey(0);
        }
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }
			
			if (winkey_status ==  WINKEY_DAH_TO_DIT_RATIO_COMMAND) {
        WinkeyDashToDotRatioCommand(incoming_serial_byte);
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

      if (winkey_status == WINKEY_WEIGHTING_COMMAND) {
        WinkeyWeightingCommand(incoming_serial_byte);
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }
			
      if (winkey_status == WINKEY_PTT_TIMES_PARM1_COMMAND) {
        WinkeyPttTimesParm1Command(incoming_serial_byte);
        winkey_status = WINKEY_PTT_TIMES_PARM2_COMMAND;
      } else {
        if (winkey_status == WINKEY_PTT_TIMES_PARM2_COMMAND) {
          WinkeyPttTimesParm2Command(incoming_serial_byte);
          winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
        }
      }

			if (winkey_status == WINKEY_SET_POT_PARM1_COMMAND) {
        WinkeySetPotParm1Command(incoming_serial_byte);
        winkey_status = WINKEY_SET_POT_PARM2_COMMAND;
      } else {
        if (winkey_status == WINKEY_SET_POT_PARM2_COMMAND) {
          WinkeySetPotParm2Command(incoming_serial_byte);
          winkey_status = WINKEY_SET_POT_PARM3_COMMAND;
        } else {
          if (winkey_status == WINKEY_SET_POT_PARM3_COMMAND) {  // third parm is max read value from pot, depending on wiring
            WinkeySetPotParm3Command(incoming_serial_byte); // WK2 protocol just ignores this third parm
            winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;      // this is taken care of in winkey_set_pot_parm3()
          }
        }
      }

			if (winkey_status ==  WINKEY_SETMODE_COMMAND) {
        WinkeySetmodeCommand(incoming_serial_byte);
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }
			
			if (winkey_status ==  WINKEY_SOFTWARE_PADDLE_COMMAND) {
        switch (incoming_serial_byte) {
          case 0: flags.winkey_dit_invoke = 0; flags.winkey_dah_invoke = 0; break;
          case 1: flags.winkey_dit_invoke = 0; flags.winkey_dah_invoke = 1; break;
          case 2: flags.winkey_dit_invoke = 1; flags.winkey_dah_invoke = 0; break;
          case 3: flags.winkey_dah_invoke = 1; flags.winkey_dit_invoke = 1; break;
        }
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

			if (winkey_status ==  WINKEY_SIDETONE_FREQ_COMMAND) {
        WinkeySidetoneFreqCommand(incoming_serial_byte);
        winkey_status = WINKEY_NO_COMMAND_IN_PROGRESS;
      }

		} // else (winkey_status == WINKEY_NO_COMMAND_IN_PROGRESS)
	} // if (action == SERVICE_SERIAL_BYTE
	
}

/** 
  * @brief 
  * @param 
  * @retval 
  */
void ServiceWinkeyBreakin(void)
{
	if (flags.send_winkey_breakin_byte_flag){
    WinkeyPortWrite(0xc2|winkey_sending|winkey_xoff,0); // 0xc2 - BREAKIN bit set high
    flags.winkey_interrupted = 1;
    flags.send_winkey_breakin_byte_flag = 0;
#ifdef DEBUG_WINKEY
		USB_print("winkey_interrupted!\r\n");
#endif
  }   
}


/** 
  * @brief 
  * @param 
  * @retval 
  */
void CheckDotPaddle(void)
{
  int8_t pin_value = 0;

  if (setting.paddle_mode == PADDLE_NORMAL) {
			pin_value = HAL_GPIO_ReadPin(GPIOF, CW_DOT);
  } else {
			pin_value = HAL_GPIO_ReadPin(GPIOF, CW_DASH);
  }

#ifdef OPTION_DIT_PADDLE_NO_SEND_ON_MEM_RPT
  if (pin_value && memory_rpt_interrupt_flag) {
    memory_rpt_interrupt_flag = 0;
    sending_mode = MANUAL_SENDING;
    loop_element_lengths(3,0,configuration.wpm);
    dit_buffer = 0;
  }
#endif
  
	if (pin_value == 0) {
		flags.dot_buffer = 1;
    if (!flags.winkey_interrupted && flags.winkey_host_open && !flags.winkey_breakin_status_byte_inhibit){
      flags.send_winkey_breakin_byte_flag = 1;
      // winkey_port_write(0xc2|winkey_sending|winkey_xoff); // 0xc2 - BREAKIN bit set high
      // winkey_interrupted = 1;
      flags.dot_buffer = 0;
    }
		flags.manual_ptt_invoke = 0;
#ifdef FEATURE_MEMORIES
    if (repeat_memory < 255) {
      repeat_memory = 255;
      #ifdef OPTION_DIT_PADDLE_NO_SEND_ON_MEM_RPT
      dit_buffer = 0;
      while (!paddle_pin_read(dit_paddle)) {};
      memory_rpt_interrupt_flag = 1;
      #endif
		}
#endif
		ClearSendBuffer();
	}
}


/** 
  * @brief 
  * @param 
  * @retval 
  */
void CheckDashPaddle(void)
{
  int8_t pin_value;

	if (setting.paddle_mode == PADDLE_NORMAL) {
		pin_value = HAL_GPIO_ReadPin(GPIOF, CW_DASH);			
  } else {
		pin_value = HAL_GPIO_ReadPin(GPIOF, CW_DOT);		
  }

  if (pin_value == 0) {
    flags.dash_buffer = 1;

    if (!flags.winkey_interrupted && flags.winkey_host_open && !flags.winkey_breakin_status_byte_inhibit){
      flags.send_winkey_breakin_byte_flag = 1;
      flags.dash_buffer = 0;
    }   
    #ifdef FEATURE_MEMORIES
      repeat_memory = 255;
    #endif
    ClearSendBuffer();
    flags.manual_ptt_invoke = 0;
  }
}


/** 
  * @brief 
  * @param 
  * @retval 
  */
void CheckPaddles(void)
{
	static int8_t last_closure = NO_CLOSURE;
#ifdef DEBUG
	USB_print("loop: entering check_paddles\r\n");
#endif  
  CheckDotPaddle();
  CheckDashPaddle();  

	if (flags.winkey_dit_invoke) flags.dot_buffer = 1;
	if (flags.winkey_dah_invoke) flags.dash_buffer = 1;

		if (setting.keyer_mode == ULTIMATIC) {
    if (ultimatic_mode == ULTIMATIC_NORMAL) {
			switch (last_closure) {
        case DIT_CLOSURE_DAH_OFF:
          if (flags.dash_buffer) {
            if (flags.dot_buffer) {
              last_closure = DAH_CLOSURE_DIT_ON;
              flags.dot_buffer = 0;
            } else {
              last_closure = DAH_CLOSURE_DIT_OFF;
            }
          } else {
            if (!flags.dot_buffer) {
              last_closure = NO_CLOSURE;
            }
          }
          break;
        case DIT_CLOSURE_DAH_ON:
          if (flags.dot_buffer) {
            if (flags.dash_buffer) {
              flags.dash_buffer = 0;
            } else {
              last_closure = DIT_CLOSURE_DAH_OFF;
            }
          } else {
            if (flags.dash_buffer) {
              last_closure = DAH_CLOSURE_DIT_OFF;
            } else {
              last_closure = NO_CLOSURE;
            }
          }
          break;

        case DAH_CLOSURE_DIT_OFF:
          if (flags.dot_buffer) {
            if (flags.dash_buffer) {
              last_closure = DIT_CLOSURE_DAH_ON;
              flags.dash_buffer = 0;
            } else {
              last_closure = DIT_CLOSURE_DAH_OFF;
            }
          } else {
            if (!flags.dash_buffer) {
              last_closure = NO_CLOSURE;
            }
          }
          break;

        case DAH_CLOSURE_DIT_ON:
          if (flags.dash_buffer) {
            if (flags.dot_buffer) {
              flags.dot_buffer = 0;
            } else {
              last_closure = DAH_CLOSURE_DIT_OFF;
            }
          } else {
            if (flags.dot_buffer) {
              last_closure = DIT_CLOSURE_DAH_OFF;
            } else {
              last_closure = NO_CLOSURE;
            }
          }
          break;

        case NO_CLOSURE:
          if ((flags.dot_buffer) && (!flags.dash_buffer)) {
            last_closure = DIT_CLOSURE_DAH_OFF;
          } else {
            if ((flags.dash_buffer) && (!flags.dot_buffer)) {
              last_closure = DAH_CLOSURE_DIT_OFF;
            } else {
              if ((flags.dot_buffer) && (flags.dash_buffer)) {
                // need to handle dit/dah priority here
                last_closure = DIT_CLOSURE_DAH_ON;
                flags.dash_buffer = 0;
              }
            }
          }
          break;
      }
    } else {  // if (ultimatic_mode == ULTIMATIC_NORMAL)
     if ((flags.dot_buffer) && (flags.dash_buffer)) {   // dit or dah priority mode
       if (ultimatic_mode == ULTIMATIC_DIT_PRIORITY) {
         flags.dash_buffer = 0;
       } else {
         flags.dot_buffer = 0;
       }
     }
    } // if (ultimatic_mode == ULTIMATIC_NORMAL)
  } // if (configuration.keyer_mode == ULTIMATIC)

	if (setting.keyer_mode == SINGLE_PADDLE){
    switch (last_closure) {
      case DIT_CLOSURE_DAH_OFF:
        if (flags.dot_buffer) {
          if (flags.dash_buffer) {
            flags.dash_buffer = 0;
          } else {
            last_closure = DIT_CLOSURE_DAH_OFF;
          }
        } else {
          if (flags.dash_buffer) {
            last_closure = DAH_CLOSURE_DIT_OFF;
          } else {
            last_closure = NO_CLOSURE;
          }
        }
        break;

      case DIT_CLOSURE_DAH_ON:
        if (flags.dash_buffer) {
          if (flags.dot_buffer) {
            last_closure = DAH_CLOSURE_DIT_ON;
            flags.dot_buffer = 0;
          } else {
            last_closure = DAH_CLOSURE_DIT_OFF;
          }
        } else {
          if (!flags.dot_buffer) {
            last_closure = NO_CLOSURE;
          }
        }
        break;

      case DAH_CLOSURE_DIT_OFF:
        if (flags.dash_buffer) {
          if (flags.dot_buffer) {
            flags.dot_buffer = 0;
          } else {
            last_closure = DAH_CLOSURE_DIT_OFF;
          }
        } else {
          if (flags.dot_buffer) {
            last_closure = DIT_CLOSURE_DAH_OFF;
          } else {
            last_closure = NO_CLOSURE;
          }
        }
        break;

      case DAH_CLOSURE_DIT_ON:
        if (flags.dot_buffer) {
          if (flags.dash_buffer) {
            last_closure = DIT_CLOSURE_DAH_ON;
            flags.dash_buffer = 0;
          } else {
            last_closure = DIT_CLOSURE_DAH_OFF;
          }
        } else {
          if (!flags.dash_buffer) {
            last_closure = NO_CLOSURE;
          }
        }
        break;

      case NO_CLOSURE:
        if ((flags.dot_buffer) && (!flags.dash_buffer)) {
          last_closure = DIT_CLOSURE_DAH_OFF;
        } else {
          if ((flags.dash_buffer) && (!flags.dot_buffer)) {
            last_closure = DAH_CLOSURE_DIT_OFF;
          } else {
            if ((flags.dot_buffer) && (flags.dash_buffer)) {
              // need to handle dit/dah priority here
              last_closure = DIT_CLOSURE_DAH_ON;
              flags.dash_buffer = 0;
            }
          }
        }
        break;
    }
  } //if (configuration.keyer_mode == SINGLE_PADDLE)
}

/** 
  * @brief 
  * @param 
  * @retval 
  */
void PttKey(void)
{
  uint32_t ptt_activation_time = HAL_GetTick();
  uint8_t all_delays_satisfied = 0;
  
  if (flags.ptt_line_activated == 0) {   // if PTT is currently deactivated, bring it up and insert PTT lead time delay
		SetTxLed(1); // Set TX led
		sequencer_ptt_inactive_time = 0;

		flags.ptt_line_activated = 1;
//USB_print("PTT ON\r\n");
	while (!all_delays_satisfied){
		uint32_t t = HAL_GetTick();
        if (((t - ptt_activation_time) >= setting.chhead[0]) && !ch[0].on){
					SetCh(0, 1);
					ch[0].on = 1;
        }
				if (((t - ptt_activation_time) >= setting.chhead[1]) && !ch[1].on){
					SetCh(1, 1);
					ch[1].on = 1;
        }          
				if (((t - ptt_activation_time) >= setting.chhead[2]) && !ch[2].on){
					SetCh(2, 1);
					ch[2].on = 1;
        }          
				if (((t - ptt_activation_time) >= setting.chhead[3]) && !ch[3].on){
					SetCh(3, 1);
					ch[3].on = 1;
        }
        if (ch[0].on && ch[1].on && ch[2].on && ch[3].on) {
          all_delays_satisfied = 1;
        }
     } //while (!all_delays_satisfied)

  }
  ptt_time = HAL_GetTick();
}


/** 
  * @brief 
  * @param 
  * @retval 
  */
void CheckSequencerTailTime()
	{
	int i;
	uint32_t t = HAL_GetTick();
  if (sequencer_ptt_inactive_time){
			for (i=0; i<4; i++) {
				if (ch[i].on && ((t - sequencer_ptt_inactive_time) >= setting.chtail[i])){
					SetCh(i, 0);
					ch[i].on = 0;
				}
			}
  }

  if (!ch[0].on && !ch[1].on && !ch[2].on && !ch[3].on){
    sequencer_ptt_inactive_time = 0;
  }


}


/** 
  * @brief 
  * @param 
  * @retval 
  */
void PttUnkey(void)
{
  if (flags.ptt_line_activated) {
//USB_print("PTT OFF\r\n");
		SetTxLed(0); // Reset TX led
		flags.ptt_line_activated = 0;
		sequencer_ptt_inactive_time = HAL_GetTick();
  }
}


/** 
  * @brief 
  * @param 
  * @retval 
  */
void LoopElementLengths(float lengths, float additional_time_ms, uint8_t speed_wpm_in)
{
	float element_length;

	if (lengths <= 0) return;

	if ((lengths == 1) && (speed_wpm_in == 0)){
		element_length = additional_time_ms;
	} else {
		element_length = 1200/speed_wpm_in;   
  }
	
  uint64_t ticks = (uint64_t)(element_length*lengths*1000) + (uint64_t)(additional_time_ms*1000); // improvement from Paul, K1XM
  uint64_t start = micros;
  while (((micros - start) < ticks)){
    CheckPttTail();
    if ((setting.keyer_mode != ULTIMATIC) && (setting.keyer_mode != SINGLE_PADDLE)) {
      if ((setting.keyer_mode == IAMBIC_A) && HAL_GPIO_ReadPin(GPIOF, CW_DOT) == 0 && HAL_GPIO_ReadPin(GPIOF, CW_DASH) == 0) {
        flags.iambic_flag = 1;
      }    
     
#ifndef FEATURE_CMOS_SUPER_KEYER_IAMBIC_B_TIMING
          if (being_sent == SENDING_DIT) {
            CheckDashPaddle();
          } else {
            if (being_sent == SENDING_DAH) {
              CheckDotPaddle();
            } else {
              CheckDashPaddle();
              CheckDotPaddle();                
            }
          }            
        #else ////FEATURE_CMOS_SUPER_KEYER_IAMBIC_B_TIMING
          //if (configuration.cmos_super_keyer_iambic_b_timing_on){
          if ((configuration.cmos_super_keyer_iambic_b_timing_on) && (sending_mode == MANUAL_SENDING)) {  
            if ((float(float(micros-start)/float(ticks))*100) >= configuration.cmos_super_keyer_iambic_b_timing_percent) {
            //if ((float(float(millis()-starttime)/float(starttime-ticks))*100) >= configuration.cmos_super_keyer_iambic_b_timing_percent) {
             if (being_sent == SENDING_DIT) {
                CheckDashPaddle();
              } else {
                if (being_sent == SENDING_DAH) {
                  CheckDotPaddle();
                }
              }     
            } else {
              if (((being_sent == SENDING_DIT) || (being_sent == SENDING_DAH)) && (paddle_pin_read(paddle_left) == LOW ) && (paddle_pin_read(paddle_right) == LOW )) {
                dah_buffer = 0;
                dit_buffer = 0;         
              }              
            }
          } else {
            if (being_sent == SENDING_DIT) {
              CheckDashPaddle();
            } else {
              if (being_sent == SENDING_DAH) {
                CheckDotPaddle();
              } else {
                CheckDashPaddle();
                CheckDotPaddle();                
              }
            }  
          }  
        #endif //FEATURE_CMOS_SUPER_KEYER_IAMBIC_B_TIMING

      } else { //(configuration.keyer_mode != ULTIMATIC)
        if (being_sent == SENDING_DIT) {
          CheckDashPaddle();
        } else {
          if (being_sent == SENDING_DAH) {
            CheckDotPaddle();
          } else {
            CheckDashPaddle();
            CheckDotPaddle();                
          }
        }   
      }
      
#if defined(FEATURE_MEMORIES) && defined(FEATURE_BUTTONS)
    check_the_memory_buttons();
#endif

    // blow out prematurely if we're automatic sending and a paddle gets hit
    if (sending_mode == AUTOMATIC_SENDING && (HAL_GPIO_ReadPin(GPIOF, CW_DOT) == 0 || HAL_GPIO_ReadPin(GPIOF, CW_DASH) == 0 || flags.dot_buffer || flags.dash_buffer)) {
      sending_mode = AUTOMATIC_SENDING_INTERRUPTED;
      automatic_sending_interruption_time = HAL_GetTick(); 
      return;
    }   
  }  //while ((millis() < endtime) && (millis() > 200))
  
  if ((setting.keyer_mode == IAMBIC_A) && (flags.iambic_flag) && HAL_GPIO_ReadPin(GPIOF, CW_DOT) == 1 && HAL_GPIO_ReadPin(GPIOF, CW_DASH) == 1) {
    flags.iambic_flag = 0;
    flags.dot_buffer = 0;
    flags.dash_buffer = 0;
  }    
  
  if ((being_sent == SENDING_DIT) || (being_sent == SENDING_DAH)){
//  if (configuration.dit_buffer_off) {flags.dot_buffer = 0;}
//  if (configuration.dah_buffer_off) {flags.dash_buffer = 0;}
  }  
} //void loop_element_lengths



/** 
  * @brief 
  * @param 
  * @retval 
  */
void SendDot(void)
{
  // notes: key_compensation is a straight x mS lengthening or shortening of the key down time
  //        weighting is

  uint16_t character_wpm = setting.wpm;

  if ((sending_mode == AUTOMATIC_SENDING) && (setting.wpm_farnsworth > setting.wpm)) {
    character_wpm = setting.wpm_farnsworth;
#if defined(DEBUG_FARNSWORTH)
    USB_print("send_dit: farns act\r\n");
	#endif
	} 
#if defined(DEBUG_FARNSWORTH)
	else {
      USB_print("send_dit: farns inact");
	}
#endif
  being_sent = SENDING_DIT;
  TxKey(1);
  SetCw(1);
	LoopElementLengths((1.0*((float)(setting.weighting)/50)),keying_compensation,character_wpm);
	SetCw(0);
  TxKey(0);
  LoopElementLengths((2.0-((float)(setting.weighting)/50)),(-1.0*keying_compensation),character_wpm);

  #ifdef FEATURE_AUTOSPACE

    byte autospace_end_of_character_flag = 0;

    if ((sending_mode == MANUAL_SENDING) && (configuration.autospace_active)) {
      check_paddles();
    }
    if ((sending_mode == MANUAL_SENDING) && (configuration.autospace_active) && (dit_buffer == 0) && (dah_buffer == 0)) {
      loop_element_lengths((float)configuration.autospace_timing_factor/(float)100,0,configuration.wpm);
      autospace_end_of_character_flag = 1;
    }
  #endif


  if ((flags.winkey_host_open) && (winkey_paddle_echo_activated) && (sending_mode == MANUAL_SENDING)) {
    winkey_paddle_echo_buffer = (winkey_paddle_echo_buffer * 10) + 1;
    //winkey_paddle_echo_buffer_decode_time = millis() + (float(winkey_paddle_echo_buffer_decode_time_factor/float(configuration.wpm))*length_letterspace);
    winkey_paddle_echo_buffer_decode_time = HAL_GetTick() + ((float)(winkey_paddle_echo_buffer_decode_timing_factor*1200.0/(float)(setting.wpm))*length_letterspace);
      
      #ifdef FEATURE_AUTOSPACE
        if (autospace_end_of_character_flag){winkey_paddle_echo_buffer_decode_time = 0;}
      #endif //FEATURE_AUTOSPACE    
    }
#ifdef FEATURE_AUTOSPACE
    autospace_end_of_character_flag = 0;
#endif //FEATURE_AUTOSPACE

  being_sent = SENDING_NOTHING;
  last_sending_mode = sending_mode;
  CheckPaddles();
}


/** 
  * @brief 
  * @param 
  * @retval 
  */
void SendDash(void)
{
  unsigned int character_wpm  = setting.wpm;

  if ((sending_mode == AUTOMATIC_SENDING) && (setting.wpm_farnsworth > setting.wpm)) {
    character_wpm = setting.wpm_farnsworth;
  }


  being_sent = SENDING_DAH;
  TxKey(1);
  SetCw(1);
  LoopElementLengths(((float)(setting.dah_to_dit_ratio/100.0)*((float)(setting.weighting)/50)),keying_compensation,character_wpm);
  SetCw(0);
  TxKey(0);

  LoopElementLengths((4.0-(3.0*((float)(setting.weighting)/50))),(-1.0*keying_compensation),character_wpm);

#ifdef FEATURE_AUTOSPACE
  byte autospace_end_of_character_flag = 0;

  if ((sending_mode == MANUAL_SENDING) && (configuration.autospace_active)) {
    CheckPaddles();
  }
  if ((sending_mode == MANUAL_SENDING) && (configuration.autospace_active) && (dit_buffer == 0) && (dah_buffer == 0)) {
    loop_element_lengths(2,0,configuration.wpm);
    autospace_end_of_character_flag = 1;
  }
#endif


  if ((flags.winkey_host_open) && (winkey_paddle_echo_activated) && (sending_mode == MANUAL_SENDING)) {
    winkey_paddle_echo_buffer = (winkey_paddle_echo_buffer * 10) + 2;
    //winkey_paddle_echo_buffer_decode_time = millis() + (float(winkey_paddle_echo_buffer_decode_time_factor/float(configuration.wpm))*length_letterspace);
    winkey_paddle_echo_buffer_decode_time = HAL_GetTick() + ((float)(winkey_paddle_echo_buffer_decode_timing_factor*1200.0/(float)(setting.wpm))*length_letterspace);      
#ifdef FEATURE_AUTOSPACE
    if (autospace_end_of_character_flag){winkey_paddle_echo_buffer_decode_time = 0;}
#endif //FEATURE_AUTOSPACE
  }

 
#ifdef FEATURE_PADDLE_ECHO
  if (sending_mode == MANUAL_SENDING) {
    paddle_echo_buffer = (paddle_echo_buffer * 10) + 2;
    paddle_echo_buffer_decode_time = millis() + (((float)1200.0/(float)configuration.wpm) * ((float)configuration.cw_echo_timing_factor/(float)100));

    #ifdef FEATURE_AUTOSPACE
    if (autospace_end_of_character_flag){paddle_echo_buffer_decode_time = 0;}
    #endif //FEATURE_AUTOSPACE    
  }
#endif //FEATURE_PADDLE_ECHO 

#ifdef FEATURE_AUTOSPACE
    autospace_end_of_character_flag = 0;
  #endif //FEATURE_AUTOSPACE  

  CheckPaddles();
  being_sent = SENDING_NOTHING;
  last_sending_mode = sending_mode;
}


/** 
  * @brief 
  * @param 
  * @retval 
  */
void ServiceDotDashBuffers(void)
{

  if (automatic_sending_interruption_time != 0){
    if ((HAL_GetTick() - automatic_sending_interruption_time) > (setting.paddle_interruption_quiet_time_element_lengths*(1200/setting.wpm))){
      automatic_sending_interruption_time = 0;
      sending_mode = MANUAL_SENDING;
    } else {
      flags.dot_buffer = 0;
      flags.dash_buffer = 0;
      return;        
    }
  }

  static int8_t bug_dah_flag = 0;

  #ifdef FEATURE_PADDLE_ECHO
    static unsigned long bug_dah_key_down_time = 0;
  #endif //FEATURE_PADDLE_ECHO

      
  if ((setting.keyer_mode == IAMBIC_A) || (setting.keyer_mode == IAMBIC_B) || (setting.keyer_mode == ULTIMATIC) || (setting.keyer_mode == SINGLE_PADDLE)) {
    if ((setting.keyer_mode == IAMBIC_A) && (flags.iambic_flag) && (HAL_GPIO_ReadPin(GPIOF, CW_DOT)) && (HAL_GPIO_ReadPin(GPIOF, CW_DASH))) {
      flags.iambic_flag = 0;
      flags.dot_buffer = 0;
      flags.dash_buffer = 0;
    } else {
      if (flags.dot_buffer) {
        flags.dot_buffer = 0;
        sending_mode = MANUAL_SENDING;
        SendDot();
      }
      if (flags.dash_buffer) {
        flags.dash_buffer = 0;
        sending_mode = MANUAL_SENDING;
        SendDash();
      }
    }
  } else {
    if (setting.keyer_mode == BUG) {
      if (flags.dot_buffer) {
        flags.dot_buffer = 0;
        sending_mode = MANUAL_SENDING;
        SendDot();
      }

      if (flags.dash_buffer) {
        flags.dash_buffer = 0;
        if (!bug_dah_flag) {
          sending_mode = MANUAL_SENDING;
          TxKey(1);
          bug_dah_flag = 1; 
        }   
      } else {
        if (bug_dah_flag){
          sending_mode = MANUAL_SENDING;
          TxKey(0);
#ifdef FEATURE_PADDLE_ECHO
            if ((millis() - bug_dah_key_down_time) > (0.5 * (1200.0/configuration.wpm))){
              if ((millis() - bug_dah_key_down_time) > (2 * (1200.0/configuration.wpm))){
                paddle_echo_buffer = (paddle_echo_buffer * 10) + 2;
              } else {
                paddle_echo_buffer = (paddle_echo_buffer * 10) + 1;
              }
              paddle_echo_buffer_decode_time = HAL_GetTick() + (((float)3000.0/(float)configuration.wpm) * ((float)configuration.cw_echo_timing_factor/(float)100));
            }
#endif //FEATURE_PADDLE_ECHO            
          bug_dah_flag = 0;
        }
      }
    } else {
      if (setting.keyer_mode == STRAIGHT) {
        if (flags.dot_buffer) {
          flags.dot_buffer = 0;
          sending_mode = MANUAL_SENDING;
          TxKey(1);
        } else {
          sending_mode = MANUAL_SENDING;
          TxKey(0);
        }
      }
    }
  }
}

/** 
  * @brief 
  * @param 
  * @retval 
  */
void CheckPttTail(void)
{
#ifdef DEBUG
    USB_print("loop: entering check_ptt_tail\r\n");
#endif

  if (HAL_GPIO_ReadPin(GPIOA, PTT_IN) == 0){
		if (!flags.manual_ptt_invoke){
        flags.manual_ptt_invoke = 1;
        PttKey();
        return; 
		}
	} else {
		if (flags.manual_ptt_invoke){
			flags.manual_ptt_invoke = 0;
			if (!flags.key_state) PttUnkey();
		}
	}  
	if (flags.key_state) {
		ptt_time = HAL_GetTick();
	} else {
		if ((flags.ptt_line_activated) && (flags.manual_ptt_invoke == 0)) {
			if (last_sending_mode == MANUAL_SENDING) {
        // PTT Tail Time: N     PTT Hang Time: Y
				if ((HAL_GetTick() - ptt_time) >= ((setting.length_wordspace*setting.ptt_hang_time_wordspace_units)*(uint32_t)(1200/setting.wpm)) ) {
					PttUnkey();
				}          
			} else { // automatic sending
				if (flags.winkey_host_open){
					if (((HAL_GetTick() - ptt_time) > (((int)(winkey_session_ptt_tail) * 10) + (3 * (1200/setting.wpm)))) && ( !setting.ptt_buffer_hold_active || ((!send_buffer_bytes) && setting.ptt_buffer_hold_active) || (flags.pause_sending_buffer))) {
              PttUnkey();
            }
				} else {
					if (((HAL_GetTick() - ptt_time) > setting.ptt_tail_time) && ( !setting.ptt_buffer_hold_active || ((!send_buffer_bytes) && setting.ptt_buffer_hold_active) || (flags.pause_sending_buffer))){
						PttUnkey();
					}            
				}  
			}
		}
	}
}

/** 
  * @brief 
  * @param 
  * @retval 
  */
void SendChar(uint8_t cw_char, uint8_t omit_letterspace)
{
	uint16_t idx = 0x8000, key;
	

//#ifdef DEBUG_SEND_CHAR
	char izp[50];
	
	USB_print("send_char: called with cw_char: ");
	snprintf(izp, sizeof(izp), "%c\r\n", cw_char);
	USB_print(izp);
  if (omit_letterspace) {
    USB_print(" OMIT_LETTERSPACE\r\n");
  }
//#endif

	key = Morse_CharToKey(cw_char);
	if (key != CW_NOKEYFOUND) {
		sending_mode = AUTOMATIC_SENDING;
		
		// find zero bit in key sequence					
		while (key & idx) idx>>=1; 
		idx>>=1; // shift right one to initial dit/dah bit
		
		while(idx) {
			if (key & idx) SendDash();
			else SendDot();
			idx>>=1;
			if ((flags.dot_buffer || flags.dash_buffer || sending_mode == AUTOMATIC_SENDING_INTERRUPTED)){
				// Interrupted
				WinkeyPortWrite(0xc2|winkey_sending|winkey_xoff,0); // 0xc2 - BREAKIN bit set high
				flags.dot_buffer = 0;
				flags.dash_buffer = 0;
				break;
			}
		}
	}
	
  if (omit_letterspace != OMIT_LETTERSPACE) {
    LoopElementLengths((length_letterspace-1),0, setting.wpm); //this is minus one because send_dit and send_dah have a trailing element space
  }

  // Farnsworth Timing : http://www.arrl.org/files/file/Technology/x9004008.pdf
/*
   if (setting.wpm_farnsworth > setting.wpm){
     float additional_intercharacter_time_ms = ((( (1.0 * farnsworth_timing_calibration) * ((60.0 * float(setting.wpm_farnsworth) ) - (37.2 * float(setting.wpm) ))/( (float)(setting.wpm) * (float)(setting.wpm_farnsworth) ))/19.0)*1000.0) - (1200.0/ (float)(setting.wpm_farnsworth) );
     loop_element_lengths(1,additional_intercharacter_time_ms,0);
 }
*/
	return;
}

/** 
  * @brief 
  * @param 
  * @retval 
  */
void ServiceSendBuffer(uint8_t no_print)
{
	  // send one character out of the send buffer
#ifdef DEBUG_LOOP
	USB_print("loop: entering service_send_buffer\r\n");
#endif       

  static unsigned long timed_command_end_time;
  static uint8_t timed_command_in_progress = 0;
	
	if (send_buffer_status == SERIAL_SEND_BUFFER_NORMAL) {
    if ((send_buffer_bytes) && (flags.pause_sending_buffer == 0)) {

			if ((send_buffer_array[0] > SERIAL_SEND_BUFFER_SPECIAL_START) && (send_buffer_array[0] < SERIAL_SEND_BUFFER_SPECIAL_END)) {
#if defined(DEBUG_SERVICE_SEND_BUFFER)
				USB_print("service_send_buffer: SERIAL_SEND_BUFFER_SPECIAL\r\n");
#endif
				if (send_buffer_array[0] == SERIAL_SEND_BUFFER_HOLD_SEND) {
#if defined(DEBUG_SERVICE_SEND_BUFFER)
					USB_print("service_send_buffer: SERIAL_SEND_BUFFER_HOLD_SEND\r\n");
#endif
					send_buffer_status = SERIAL_SEND_BUFFER_HOLD;
          RemoveFromSendBuffer();
				}
				
				if (send_buffer_array[0] == SERIAL_SEND_BUFFER_HOLD_SEND_RELEASE) {
#if defined(DEBUG_SERVICE_SEND_BUFFER)
					USB_print("service_send_buffer: SERIAL_SEND_BUFFER_HOLD_SEND_RELEASE\r\n");
#endif
          RemoveFromSendBuffer();
        }

				if (send_buffer_array[0] == SERIAL_SEND_BUFFER_MEMORY_NUMBER) {
#if defined(DEBUG_SERVICE_SEND_BUFFER)
					USB_print("service_send_buffer: SERIAL_SEND_BUFFER_MEMORY_NUMBER\r\n");
#endif

#ifdef DEBUG_SEND_BUFFER
					USB_print("service_send_buffer: SERIAL_SEND_BUFFER_MEMORY_NUMBER\r\n"));
#endif
          if (winkey_sending && flags.winkey_host_open) {
            WinkeyPortWrite(0xc0|winkey_sending|winkey_xoff,0);
            flags.winkey_interrupted = 1;
          }
          RemoveFromSendBuffer();
          if (send_buffer_bytes) {
            if (send_buffer_array[0] < 5) {
#ifdef FEATURE_MEMORIES
                play_memory(send_buffer_array[0]);
#endif
						}
            RemoveFromSendBuffer();
          }
        }

        if (send_buffer_array[0] == SERIAL_SEND_BUFFER_WPM_CHANGE) {  // two bytes for wpm
          //remove_from_send_buffer();
#if defined(DEBUG_SERVICE_SEND_BUFFER)
          USB_print("service_send_buffer: SERIAL_SEND_BUFFER_WPM_CHANGE\r\n");
#endif
          if (send_buffer_bytes > 2) {
#if defined(DEBUG_SERVICE_SEND_BUFFER)
            USB_print("service_send_buffer: SERIAL_SEND_BUFFER_WPM_CHANGE: send_buffer_bytes>2\r\n");
#endif
            RemoveFromSendBuffer();

              if ((flags.winkey_host_open) && (winkey_speed_state == WINKEY_UNBUFFERED_SPEED)){
                winkey_speed_state = WINKEY_BUFFERED_SPEED;
                #if defined(DEBUG_SERVICE_SEND_BUFFER)
                  debug_serial_port->println("service_send_buffer: winkey_speed_state = WINKEY_BUFFERED_SPEED");
                #endif
                winkey_last_unbuffered_speed_wpm = setting.wpm;
              }
            setting.wpm = send_buffer_array[0] * 256;
            RemoveFromSendBuffer();
            setting.wpm = setting.wpm + send_buffer_array[0];
            RemoveFromSendBuffer();
          } else {
#if defined(DEBUG_SERVICE_SEND_BUFFER)
            USB_print("service_send_buffer:SERIAL_SEND_BUFFER_WPM_CHANGE < 2\r\n");
#endif
          }
#if defined(DEBUG_SERVICE_SEND_BUFFER)
          USB_print("service_send_buffer: SERIAL_SEND_BUFFER_WPM_CHANGE: exit send_buffer_bytes: ");
          USB_print(send_buffer_bytes);
#endif
        }
				
				if (send_buffer_array[0] == SERIAL_SEND_BUFFER_TX_CHANGE) {  // one byte for transmitter #
#if defined(DEBUG_SERVICE_SEND_BUFFER)
          USB_print("service_send_buffer: SERIAL_SEND_BUFFER_TX_CHANGE\r\n");
#endif
          RemoveFromSendBuffer();
          if (send_buffer_bytes > 1) {
            // if ((send_buffer_array[0] > 0) && (send_buffer_array[0] < 7)){
            //   switch_to_tx_silent(send_buffer_array[0]);
            // }
            RemoveFromSendBuffer();          
          }
        }

				if (send_buffer_array[0] == SERIAL_SEND_BUFFER_NULL) {
#if defined(DEBUG_SERVICE_SEND_BUFFER)
          USB_print("service_send_buffer: SERIAL_SEND_BUFFER_NULL\r\n");
#endif
          RemoveFromSendBuffer();
        }

				if (send_buffer_array[0] == SERIAL_SEND_BUFFER_PROSIGN) {
#if defined(DEBUG_SERVICE_SEND_BUFFER)
          USB_print("service_send_buffer: SERIAL_SEND_BUFFER_PROSIGN\r\n");
#endif
          RemoveFromSendBuffer();
          if (send_buffer_bytes) {
            SendChar(send_buffer_array[0],OMIT_LETTERSPACE);
            if (flags.winkey_host_open){
              // Must echo back PROSIGN characters sent  N6TV
              WinkeyPortWrite(0xc4|winkey_sending|winkey_xoff,0);  // N6TV
              WinkeyPortWrite(send_buffer_array[0],0);  // N6TV  
            }          
            RemoveFromSendBuffer();
          }
          if (send_buffer_bytes) {
            SendChar(send_buffer_array[0],KEYER_NORMAL);
              if (flags.winkey_host_open){
                // Must echo back PROSIGN characters sent  N6TV
                WinkeyPortWrite(0xc4|winkey_sending|winkey_xoff,0);  // N6TV
                WinkeyPortWrite(send_buffer_array[0],0);  // N6TV  
              }          
            RemoveFromSendBuffer();
          }
        }

				if (send_buffer_array[0] == SERIAL_SEND_BUFFER_TIMED_KEY_DOWN) {
#if defined(DEBUG_SERVICE_SEND_BUFFER)
          USB_print("service_send_buffer: SERIAL_SEND_BUFFER_TIMED_KEY_DOWN\r\n");
#endif

          RemoveFromSendBuffer();
          if (send_buffer_bytes) {
            send_buffer_status = SERIAL_SEND_BUFFER_TIMED_COMMAND;
            sending_mode = AUTOMATIC_SENDING;
            TxKey(1);
            timed_command_end_time = HAL_GetTick() + (send_buffer_array[0] * 1000);
            timed_command_in_progress = SERIAL_SEND_BUFFER_TIMED_KEY_DOWN;
            RemoveFromSendBuffer();
          }
        }

				if (send_buffer_array[0] == SERIAL_SEND_BUFFER_TIMED_WAIT) {
#if defined(DEBUG_SERVICE_SEND_BUFFER)
          USB_print("service_send_buffer: SERIAL_SEND_BUFFER_TIMED_WAIT\r\n");
#endif
          RemoveFromSendBuffer();
          if (send_buffer_bytes) {
            send_buffer_status = SERIAL_SEND_BUFFER_TIMED_COMMAND;
            timed_command_end_time = HAL_GetTick() + (send_buffer_array[0] * 1000);
            timed_command_in_progress = SERIAL_SEND_BUFFER_TIMED_WAIT;
            RemoveFromSendBuffer();
          }
        }
				
				if (send_buffer_array[0] == SERIAL_SEND_BUFFER_PTT_ON) {
#if defined(DEBUG_SERVICE_SEND_BUFFER)
          USB_print("service_send_buffer: SERIAL_SEND_BUFFER_PTT_ON\r\n");
#endif
          RemoveFromSendBuffer();
          flags.manual_ptt_invoke = 1;
          PttKey();
        }

        if (send_buffer_array[0] == SERIAL_SEND_BUFFER_PTT_OFF) {
#if defined(DEBUG_SERVICE_SEND_BUFFER)
          USB_print("service_send_buffer: SERIAL_SEND_BUFFER_PTT_OFF\r\n");
#endif
          RemoveFromSendBuffer();
          flags.manual_ptt_invoke = 0;
          PttUnkey();
        }
				
      } else {   // if ((send_buffer_array[0] > SERIAL_SEND_BUFFER_SPECIAL_START) && (send_buffer_array[0] < SERIAL_SEND_BUFFER_SPECIAL_END))
        
				SendChar(send_buffer_array[0], KEYER_NORMAL);  //****************
				
				if ((flags.winkey_host_open) && (!no_print)) {
          if ((send_buffer_array[0]!= 0x7C) && (send_buffer_array[0] > 30)) {WinkeyPortWrite(send_buffer_array[0],0);}
        }

				if (!flags.pause_sending_buffer) {
#if defined(DEBUG_SERVICE_SEND_BUFFER)
          USB_print("service_send_buffer: after send_char: remove_from_send_buffer\r\n");
          if (no_bytes_flag){
            USB_print("service_send_buffer: no_bytes_flag\r\n");
          }
#endif

					if (!((send_buffer_array[0] > SERIAL_SEND_BUFFER_SPECIAL_START) && (send_buffer_array[0] < SERIAL_SEND_BUFFER_SPECIAL_END))){ // this is a friggin hack to fix something I can't explain with SO2R - Goody 20191217
            RemoveFromSendBuffer();
#if defined(DEBUG_SERVICE_SEND_BUFFER)
            USB_print("service_send_buffer: after send_char: remove_from_send_buffer\r\n");
            if (no_bytes_flag){
							USB_print("service_send_buffer: no_bytes_flag\r\n");
            }
#endif
          } else {
#if defined(DEBUG_SERVICE_SEND_BUFFER)
						USB_print("service_send_buffer: snagged errant remove_from_send_buffer\r\n");
#endif
					}
					        }
      }
    }  //if ((send_buffer_bytes) && (pause_sending_buffer == 0))
		
  } else {   //if (send_buffer_status == SERIAL_SEND_BUFFER_NORMAL)
		
    if (send_buffer_status == SERIAL_SEND_BUFFER_TIMED_COMMAND) {    // we're in a timed command
      if ((timed_command_in_progress == SERIAL_SEND_BUFFER_TIMED_KEY_DOWN) && (HAL_GetTick() > timed_command_end_time)) {
        sending_mode = AUTOMATIC_SENDING;
        TxKey(0);
        timed_command_in_progress = 0;
        send_buffer_status = SERIAL_SEND_BUFFER_NORMAL;
#if defined(DEBUG_SERVICE_SEND_BUFFER)
        USB_print("service_send_buffer: SERIAL_SEND_BUFFER_TIMED_KEY_DOWN->SERIAL_SEND_BUFFER_NORMAL\r\n");
#endif
      }
			
			if ((timed_command_in_progress == SERIAL_SEND_BUFFER_TIMED_WAIT) && (HAL_GetTick() > timed_command_end_time)) {
        timed_command_in_progress = 0;
        send_buffer_status = SERIAL_SEND_BUFFER_NORMAL;
#if defined(DEBUG_SERVICE_SEND_BUFFER)
        USB_print("service_send_buffer: SERIAL_SEND_BUFFER_TIMED_WAIT->SERIAL_SEND_BUFFER_NORMAL\r\n");
#endif
      }
		}

		if (send_buffer_status == SERIAL_SEND_BUFFER_HOLD) {  // we're in a send hold ; see if there's a SERIAL_SEND_BUFFER_HOLD_SEND_RELEASE in the buffer
      if (send_buffer_bytes == 0) {
        send_buffer_status = SERIAL_SEND_BUFFER_NORMAL;  // this should never happen, but what the hell, we'll catch it here if it ever does happen
      } else {
        for (int z = 0; z < send_buffer_bytes; z++) {
          if (send_buffer_array[z] ==  SERIAL_SEND_BUFFER_HOLD_SEND_RELEASE) {
            send_buffer_status = SERIAL_SEND_BUFFER_NORMAL;
            z = send_buffer_bytes;
          }
        }
      }
    }

	}  //if (send_buffer_status == SERIAL_SEND_BUFFER_NORMAL)
	
	//if the paddles are hit, dump the buffer
  CheckPaddles();
  if (((flags.dot_buffer || flags.dash_buffer) && (send_buffer_bytes)) ) {

#if defined(DEBUG_SERVICE_SEND_BUFFER)
    USB_print("service_send_buffer: buffer dump\r\n");
#endif
    ClearSendBuffer();
    send_buffer_status = SERIAL_SEND_BUFFER_NORMAL;
    flags.dot_buffer = 0;
    flags.dash_buffer = 0;    
#ifdef FEATURE_MEMORIES
    repeat_memory = 255;
#endif
    if (winkey_sending && flags.winkey_host_open) {
      WinkeyPortWrite(0xc2|winkey_sending|winkey_xoff,0); // 0xc2 - BREAKIN bit set high
      flags.winkey_interrupted = 1;
    }
  }
}


/**
  * @brief Output current device setting to the console port.
  * @param None
  * @retval None
  */
void DisplaySettings(void)
{
	char izp[50];
	int i;
	for (i=0; i<4; i++) {
		snprintf(izp, sizeof(izp), "CH#%d head: %dms tail: %dms rev: %d\r\n", i+1, setting.chhead[i], setting.chtail[i], setting.chrev[i]);
		USB_print(izp);
	}
	snprintf(izp, sizeof(izp), "CW key rev: %s, rev: %s\n\r", setting.paddle_mode?"ON":"OFF", setting.cw_keyreverse?"ON":"OFF");
	USB_print(izp);
	snprintf(izp, sizeof(izp), "CW WPM: %d, dot ratio: %d\n\r", setting.wpm, setting.dah_to_dit_ratio/100);
	USB_print(izp);
	snprintf(izp, sizeof(izp), "TX tail time: %d ms\n\r", setting.ptt_tail_time);
	USB_print(izp);
	USB_print("Key mode: ");
	switch(setting.keyer_mode) {
		case STRAIGHT: USB_print("STRAIGHT"); break;
		case IAMBIC_B: USB_print("IAMBIC_B"); break;
		case IAMBIC_A: USB_print("IAMBIC_A"); break;
		case BUG: USB_print("BUG"); break;
		case ULTIMATIC: USB_print("ULTIMATIC"); break;
		case SINGLE_PADDLE: USB_print("SINGLE PADDLE"); break;
	}
	USB_print("\r\n");
}


/**
  * @brief Store settinfgs to flash memory region.
  * @param None
  * @retval None
  */
void StoreSetting(void)
{
#ifdef DEBUG
	char izp[32];
#endif // DEBUG
	HAL_StatusTypeDef FLstatus;
	FLASH_EraseInitTypeDef eraseinit;
	uint32_t PageError;
	uint32_t stadr;
	uint64_t buf;
	uint32_t fladr;
	uint8_t *adr;

	
	FLstatus = HAL_FLASH_Unlock();  

	// erase store 
	eraseinit.NbPages = 1;
	eraseinit.PageAddress = FLASH_STORE_ADR;
	eraseinit.TypeErase = FLASH_TYPEERASE_PAGES;
	FLstatus = HAL_FLASHEx_Erase(&eraseinit, &PageError);   
			
	adr = (uint8_t *)&setting;
	fladr = FLASH_STORE_ADR;
	for (stadr = 0; stadr<(sizeof(settings_t)/8); stadr++) {
		memcpy(&buf, adr, 8); 
		FLstatus = HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, fladr, buf);
		adr+=8;
		fladr+=8;
	}
#ifdef DEBUG
	snprintf(izp, 32, "Writing status: %d\n\r", FLstatus); 
	USB_print(izp);
#endif  //DEBUG
	FLstatus = HAL_FLASH_Lock();  
	flags.config_dirty = 0;
}

/**
  * @brief ADC DMA interrupt processor.
  * @param ADC handler descriptor.
  * @retval None
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* h)
{
	if(h->Instance == ADC1) {
		//check if the interrupt comes from ACD1
		HAL_ADC_Stop_DMA(&hadc);
		flags.irq_adc = 1;
   }
}


/* Set CW keyer output*/
void SetCw(char on)
{
//	cw_sidetone = on;

	if(!setting.cw_keyreverse) {
		if (on) HAL_GPIO_WritePin(GPIOA, CW_OUT, GPIO_PIN_SET);
		else HAL_GPIO_WritePin(GPIOA, CW_OUT, GPIO_PIN_RESET);
	} else {
		if (on) HAL_GPIO_WritePin(GPIOA, CW_OUT, GPIO_PIN_RESET);
		else HAL_GPIO_WritePin(GPIOA, CW_OUT, GPIO_PIN_SET);
	}

}


/**
  * @brief Set selected output squencer output state
  * @param n: Channel number from 0 to 3.
  * @param on: 1 - switch on, 0 - switch off
  * @retval None
  */
void SetCh(uint8_t n, uint8_t on)
{
	
	if(!setting.chrev[n]) {
		if (on) HAL_GPIO_WritePin(GPIOA, ch[n].gpio, GPIO_PIN_SET);
		else HAL_GPIO_WritePin(GPIOA, ch[n].gpio, GPIO_PIN_RESET);
	} else {
		if (on) HAL_GPIO_WritePin(GPIOA, ch[n].gpio, GPIO_PIN_RESET);
		else HAL_GPIO_WritePin(GPIOA, ch[n].gpio, GPIO_PIN_SET);
	}
	ch[n].on = on;
}

/** 
  * @brief Set "TX" led
  * @param bool: 1 - switch on, 0 - switch off
  * @retval None
  */
void SetTxLed(char on)
{
	if (on) HAL_GPIO_WritePin(GPIOA, TX_LED, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(GPIOA, TX_LED, GPIO_PIN_RESET);
}


/**
  * @brief Set "auto" led
  * @param bool: 1 - switch on, 0 - switch off
  * @retval None
  */
void SetAutoLed(uint8_t on)
{
	if (on) HAL_GPIO_WritePin(GPIOB, AUTO_OUT, GPIO_PIN_SET);
	else HAL_GPIO_WritePin(GPIOB, AUTO_OUT, GPIO_PIN_RESET);
}


void SaveDefaultConfig(void)
{
	// Ident missmatch - load default setting
	memset((uint8_t *)&setting, 0, sizeof(settings_t));
	setting.Ident[0]=0x55; setting.Ident[1]=0x66; setting.Ident[2]=0x55; setting.Ident[3]=0x66;
	setting.chtail[0] = 200;
	setting.chtail[1] = 10;
	setting.chtail[2] = 0;
	setting.chtail[3] = 10;
	setting.wpm = 27;
	setting.wpm_farnsworth = 27;
	setting.dah_to_dit_ratio = 300; 		// 300 = 3 / normal 3:1 ratio
  setting.ptt_tail_time = 10;					// PTT tail time in mS
  setting.paddle_interruption_quiet_time_element_lengths = 0;	
	setting.ptt_hang_time_wordspace_units = 1;
  setting.paddle_mode = PADDLE_NORMAL;
  setting.keyer_mode = IAMBIC_B;
	setting.weighting = 50;							// 50 = weighting factor of 1 (normal)
	setting.length_wordspace = 7;
	setting.ptt_buffer_hold_active = 0;
	setting.pot_activated = 1;

	StoreSetting();
	memcpy((uint8_t *)&setting, (const void *)&settings_Store, sizeof(settings_t));
}


/**
  * @brief  The application main entry point.
  * @retval int - not used
  */
int main(void)
{
	int i;
	static uint32_t adc_tik;

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
	MX_DMA_Init();
  MX_ADC_Init();
  MX_USB_DEVICE_Init();
	
	HAL_ADCEx_Calibration_Start(&hadc);
	HAL_ADC_Start_DMA(&hadc, (uint32_t*)&adc, 2); // стартуем АЦП
	MX_TIM14_Init();

	// Start indicator
	for (i = 0; i <5; i++) {
		SetTxLed(1);
		Delay(300);
		SetTxLed(0);
		Delay(300);
	}

	/* Load settinfgs from specified position  */
	memcpy((uint8_t *)&setting, (const void *)&settings_Store, sizeof(settings_t));

	if (setting.Ident[0]!=0x55 || setting.Ident[1]!=0x66 || setting.Ident[2]!=0x55 || setting.Ident[3]!=0x66) {
		SaveDefaultConfig();
	}

	// setup default modes	
  setting.paddle_mode = PADDLE_NORMAL;
  setting.keyer_mode = IAMBIC_B;
  setting.ptt_tail_time = 10;					// PTT tail time in mS
	setting.dah_to_dit_ratio = 300; 		// 300 = 3 / normal 3:1 ratio
	setting.weighting = 50;							// 50 = weighting factor of 1 (normal)
	setting.length_wordspace = 7;
	setting.ptt_buffer_hold_active = 0;
	setting.paddle_interruption_quiet_time_element_lengths = 0;
	setting.pot_activated = 1;
	
	// Init speed potentiometer
	pot_wpm_low_value = initial_pot_wpm_low_value;
	pot_wpm_high_value = initial_pot_wpm_high_value;
  last_pot_wpm_read = PotValueWPM();

	// Display setting
	DisplaySettings();
		
	// Set initial gpio level
	SetCw(0);
	for (i=0; i<4; i++) SetCh(i, 0);
	SetTxLed(0);
	SetAutoLed(0);

	HAL_TIM_Base_Start_IT(&htim14);
	
  /* Infinite loop */
  while (1)
  {
		
		if (flags.com_event) {
			char msg[32];
			flags.com_event = 0;
			sprintf(msg, "T %d %d\n\r", cdcvcp_ctrllines.dtr, cdcvcp_ctrllines.rts);
			USB_print(msg);
		}
		

		if(setting.pot_activated) {
			if (flags.irq_adc) {
				CheckPotentiometer();
				flags.irq_adc = 0;
				adc[0] = adc[1] = 0;
			}
			if (HAL_GetTick()-adc_tik > 200) {
				HAL_ADC_Start_DMA(&hadc, (uint32_t*)&adc, 2); // стартуем АЦП
				adc_tik = HAL_GetTick();
			}
		}
	
		if (flags.inCommand) ExecuteCmd();	// Process serial command, if detected.
		
		if (flags.winkey_command) {
			// USB virtial com port #2 data in from host passed to winkey service.
			for (i=0; i<winkey_ptr_in; i++) ServiceWinkey(SERVICE_SERIAL_BYTE, winkey_buffer[i]);
			flags.winkey_command = 0;
		}
		CheckPaddles();
    ServiceDotDashBuffers();
    ServiceSendBuffer(PRINTCHAR);
    CheckPttTail();
		ServiceWinkey(WINKEY_HOUSEKEEPING, 0);
		ServiceWinkeyBreakin();
		CheckSequencerTailTime();
		if (flags.config_dirty && !flags.ptt_line_activated) {
			StoreSetting();
		}
  }
}



/**
  * @brief Check CW speed from potentiometer, set CW speed if needed and report speed to winkey.
  * @param None
  * @retval None
  */
void CheckPotentiometer(void)
{
	static uint32_t last_pot_check_time = 0;
	
	if (setting.pot_activated && ((HAL_GetTick() - last_pot_check_time) > potentiometer_check_interval_ms)) {
    last_pot_check_time = HAL_GetTick();
		uint16_t pot_value_wpm_read = PotValueWPM();	
		if (((abs(pot_value_wpm_read - last_pot_wpm_read) * 10) > (potentiometer_change_threshold * 10))) {
#ifdef DEBUG
			char izp[50];
			snprintf(izp, 40, "check_potentiometer: %d %d\n\r", adc[0], pot_value_wpm_read);
			USB_print(izp);
#endif // DEBUG
			SpeedSet(pot_value_wpm_read);
			last_pot_wpm_read = pot_value_wpm_read;
			if (flags.winkey_host_open) {
				WinkeyPortWrite(((pot_value_wpm_read-pot_wpm_low_value)|128),0);
				winkey_last_unbuffered_speed_wpm = setting.wpm;
			}
		}
	}
}


/**
  * @brief Read CW speed from potentiometer and map result into WPM value
  * @param None
  * @retval Value in WPM read from potentiometer.
  */
uint16_t PotValueWPM(void)
{
  static uint16_t last_pot_read = 0;
  static uint16_t return_value = 0;
  uint16_t pot_read = adc[0];
  if (abs(pot_read - last_pot_read) > potentiometer_reading_threshold ) {
    return_value = map(pot_read, 0, 4096, pot_wpm_low_value, pot_wpm_high_value);
    last_pot_read = pot_read;
  }
  return return_value;
}


/**
  * @brief Set CW speed in word per seconds
  * @param WPM value to set.
  * @retval None
  */
void SpeedSet(uint16_t wpm_set)
{
	if ((wpm_set > 0) && (wpm_set < 1000)){
    setting.wpm = wpm_set;
    flags.config_dirty = 1;
		
    if ((setting.wpm >= DYNAMIC_DAH_TO_DIT_RATIO_LOWER_LIMIT_WPM) && (setting.wpm <= DYNAMIC_DAH_TO_DIT_RATIO_UPPER_LIMIT_WPM)){
        int dynamicweightvalue = map(setting.wpm,DYNAMIC_DAH_TO_DIT_RATIO_LOWER_LIMIT_WPM,DYNAMIC_DAH_TO_DIT_RATIO_UPPER_LIMIT_WPM,DYNAMIC_DAH_TO_DIT_RATIO_LOWER_LIMIT_RATIO,DYNAMIC_DAH_TO_DIT_RATIO_UPPER_LIMIT_RATIO);
        setting.dah_to_dit_ratio=dynamicweightvalue;
		}
	}
}



/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI14|RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
  RCC_OscInitStruct.HSI14CalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_HSI48;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};
	/** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion) 
  */
  hadc.Instance = ADC1;
  hadc.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc.Init.Resolution = ADC_RESOLUTION_12B;
  hadc.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc.Init.ScanConvMode = ADC_SCAN_DIRECTION_FORWARD;
  hadc.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc.Init.LowPowerAutoWait = DISABLE;
  hadc.Init.LowPowerAutoPowerOff = DISABLE;
  hadc.Init.ContinuousConvMode = ENABLE;
  hadc.Init.DiscontinuousConvMode = DISABLE;
  hadc.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DMAContinuousRequests = ENABLE;
  hadc.Init.Overrun = ADC_OVR_DATA_PRESERVED;

  if (HAL_ADC_Init(&hadc) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel to be converted. 
  */
  sConfig.Channel = ADC_CHANNEL_7;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_41CYCLES_5;

  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    //Error_Handler();
  }
  /** Configure for the selected ADC regular channel to be converted. 
  */
  sConfig.Channel = ADC_CHANNEL_6;
	sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

}





/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{
  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();
  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}



/**
  * @brief GPIO Initialization Function
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 
                          |GPIO_PIN_4|GPIO_PIN_13, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);


  /*Configure GPIO PF0 PF1: DOT AND DASH CW KEYBOARD */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA2 PA3 
                           PA4 PA13 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 
                          |GPIO_PIN_4|GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
  /*Configure GPIO pin PB1:  AUTO indicator */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);


	/* Configure GPIO PA5:  PTT_IN */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	/* Configure GPIO PA6 PA7:  speed and keyboard */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	micros += 10;
/*
	if (cw_sidetone) {
		t++;
		if (t>150) {
				t=0;
				tone_state = !tone_state;
				HAL_GPIO_WritePin(GPIOA, CW_OUT, tone_state);
		}
	}
*/
}

/**
  * @brief TIM14 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM14_Init(void)
{

  htim14.Instance = TIM14;
  htim14.Init.Prescaler = 10;
//	htim14.Init.Prescaler = 79;
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim14.Init.Period = 43;
//	htim14.Init.Period = 999;
  htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
}


void Error_Handler(void)
{
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */


