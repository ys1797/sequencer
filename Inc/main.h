/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"

#define MAX_CMD_LEN 64 
#define RX_DATA_SIZE (64)
#define RX_FILL (USART_RX_DATA_SIZE/2)

#define CW_MAX_MSG_LENGTH			64
#define FLASH_STORE_ADR				0x8007c00

#define PTT_IN								GPIO_PIN_5	// PTT INPUT PIN (PA5)
#define CW_DOT 								GPIO_PIN_0	// CW keyer DOT input (PF0)
#define CW_DASH 							GPIO_PIN_1	// CW keyer DASH input (PF1)

#define KEYB_IN								GPIO_PIN_7	// keyboard analog input PIN (PA7)
#define SPEED_IN							GPIO_PIN_6	// CW speed regulator analog input PIN (PA6)

#define TX_CH1								GPIO_PIN_0	// TX output channel 1
#define TX_CH2								GPIO_PIN_1	// TX output channel 2
#define TX_CH3								GPIO_PIN_2	// TX output channel 3
#define TX_CH4								GPIO_PIN_3	// TX output channel 4
#define CW_OUT								GPIO_PIN_4	// CW output pin 
#define AUTO_OUT							GPIO_PIN_1	// AUTO indicator output pin  (PB1)
#define TX_LED								GPIO_PIN_13	// AUTO indicator output pin  (PA13)

#define AUTO_ANALOG_PIN				GPIO_PIN_1	// Analog input channel "AUTO"

// Variable macros
#define STRAIGHT 1
#define IAMBIC_B 2
#define IAMBIC_A 3
#define BUG 4
#define ULTIMATIC 5
#define SINGLE_PADDLE 6

#define PADDLE_NORMAL 0
#define PADDLE_REVERSE 1

#define ULTIMATIC_NORMAL 0
#define ULTIMATIC_DIT_PRIORITY 1
#define ULTIMATIC_DAH_PRIORITY 2

#define PRINTCHAR 0
#define NOPRINT 1

#define KEYER_NORMAL 0
#define OMIT_LETTERSPACE 1

#define UNDEFINED_SENDING 0	
#define AUTOMATIC_SENDING 1
#define MANUAL_SENDING 2
#define AUTOMATIC_SENDING_INTERRUPTED 3

#define DYNAMIC_DAH_TO_DIT_RATIO_LOWER_LIMIT_WPM 30
#define DYNAMIC_DAH_TO_DIT_RATIO_LOWER_LIMIT_RATIO 300 // 300 = 3:1 ratio
#define DYNAMIC_DAH_TO_DIT_RATIO_UPPER_LIMIT_WPM 70
#define DYNAMIC_DAH_TO_DIT_RATIO_UPPER_LIMIT_RATIO 240 // 240 = 2.4:1 ratio

#define NO_CLOSURE 0
#define DIT_CLOSURE_DAH_OFF 1
#define DAH_CLOSURE_DIT_OFF 2
#define DIT_CLOSURE_DAH_ON 3
#define DAH_CLOSURE_DIT_ON 4


#define SIDETONE_OFF 0
#define SIDETONE_ON 1
#define SIDETONE_PADDLE_ONLY 2


#define SENDING_NOTHING 0
#define SENDING_DIT 1
#define SENDING_DAH 2

#define SERIAL_SEND_BUFFER_NORMAL 0
#define SERIAL_SEND_BUFFER_TIMED_COMMAND 1
#define SERIAL_SEND_BUFFER_HOLD 2

#define SERIAL_SEND_BUFFER_SPECIAL_START 13
#define SERIAL_SEND_BUFFER_WPM_CHANGE 14        
#define SERIAL_SEND_BUFFER_PTT_ON 15            
#define SERIAL_SEND_BUFFER_PTT_OFF 16           
#define SERIAL_SEND_BUFFER_TIMED_KEY_DOWN 17    
#define SERIAL_SEND_BUFFER_TIMED_WAIT 18        
#define SERIAL_SEND_BUFFER_NULL 19              
#define SERIAL_SEND_BUFFER_PROSIGN 20           
#define SERIAL_SEND_BUFFER_HOLD_SEND 21         
#define SERIAL_SEND_BUFFER_HOLD_SEND_RELEASE 22 
#define SERIAL_SEND_BUFFER_MEMORY_NUMBER 23
#define SERIAL_SEND_BUFFER_TX_CHANGE 24
#define SERIAL_SEND_BUFFER_SPECIAL_END 25


// alter these below to map alternate sidetones for Winkey interface protocol emulation
#ifdef OPTION_WINKEY_2_SUPPORT
	#define WINKEY_SIDETONE_1 3759
	#define WINKEY_SIDETONE_2 1879
	#define WINKEY_SIDETONE_3 1252
	#define WINKEY_SIDETONE_4 940
	#define WINKEY_SIDETONE_5 752
	#define WINKEY_SIDETONE_6 625
	#define WINKEY_SIDETONE_7 535
	#define WINKEY_SIDETONE_8 469
	#define WINKEY_SIDETONE_9 417
	#define WINKEY_SIDETONE_10 375
#else //OPTION_WINKEY_2_SUPPORT
	#define WINKEY_SIDETONE_1 4000
	#define WINKEY_SIDETONE_2 2000
	#define WINKEY_SIDETONE_3 1333
	#define WINKEY_SIDETONE_4 1000
	#define WINKEY_SIDETONE_5 800
	#define WINKEY_SIDETONE_6 666
	#define WINKEY_SIDETONE_7 571
	#define WINKEY_SIDETONE_8 500
	#define WINKEY_SIDETONE_9 444
	#define WINKEY_SIDETONE_10 400
#endif //OPTION_WINKEY_2_SUPPORT


#define WINKEY_1_REPORT_VERSION_NUMBER 10
#define WINKEY_2_REPORT_VERSION_NUMBER 23

// alter these to map to alternate hang time wordspace units
#define WINKEY_HANG_TIME_1_0 1.0
#define WINKEY_HANG_TIME_1_33 1.33
#define WINKEY_HANG_TIME_1_66 1.66
#define WINKEY_HANG_TIME_2_0 2.0

#define WINKEY_RETURN_THIS_FOR_ADMIN_GET_CAL 0x16
#define WINKEY_RETURN_THIS_FOR_ADMIN_PADDLE_A2D 0xEE
#define WINKEY_RETURN_THIS_FOR_ADMIN_SPEED_A2D 0x00


// WINKEY defination
#define WINKEY_NO_COMMAND_IN_PROGRESS 0
#define WINKEY_UNBUFFERED_SPEED_COMMAND 1
#define WINKEY_UNSUPPORTED_COMMAND 2
#define WINKEY_POINTER_COMMAND 3
#define WINKEY_ADMIN_COMMAND 4
#define WINKEY_PAUSE_COMMAND 5
#define WINKEY_KEY_COMMAND 6
#define WINKEY_SETMODE_COMMAND 7
#define WINKEY_SIDETONE_FREQ_COMMAND 8
#define WINKEY_ADMIN_COMMAND_ECHO 9
#define WINKEY_BUFFERED_SPEED_COMMAND 10
#define WINKEY_DAH_TO_DIT_RATIO_COMMAND 11
#define WINKEY_KEYING_COMPENSATION_COMMAND 12
#define WINKEY_FIRST_EXTENSION_COMMAND 13
#define WINKEY_PTT_TIMES_PARM1_COMMAND 14
#define WINKEY_PTT_TIMES_PARM2_COMMAND 15
#define WINKEY_SET_POT_PARM1_COMMAND 16
#define WINKEY_SET_POT_PARM2_COMMAND 17
#define WINKEY_SET_POT_PARM3_COMMAND 18
#define WINKEY_SOFTWARE_PADDLE_COMMAND 19
//#define WINKEY_CANCEL_BUFFERED_SPEED_COMMAND 20
#define WINKEY_BUFFFERED_PTT_COMMMAND 21
#define WINKEY_HSCW_COMMAND 22
#define WINKEY_BUFFERED_HSCW_COMMAND 23
#define WINKEY_WEIGHTING_COMMAND 24
#define WINKEY_KEY_BUFFERED_COMMAND 25
#define WINKEY_WAIT_BUFFERED_COMMAND 26
#define WINKEY_POINTER_01_COMMAND 27
#define WINKEY_POINTER_02_COMMAND 28
#define WINKEY_POINTER_03_COMMAND 29
#define WINKEY_FARNSWORTH_COMMAND 30
#define WINKEY_MERGE_COMMAND 31
#define WINKEY_MERGE_PARM_2_COMMAND 32
#define WINKEY_SET_PINCONFIG_COMMAND 33
#define WINKEY_EXTENDED_COMMAND 34
#define WINKEY_SEND_MSG 35
#define WINKEY_LOAD_SETTINGS_PARM_1_COMMAND 101
#define WINKEY_LOAD_SETTINGS_PARM_2_COMMAND 102
#define WINKEY_LOAD_SETTINGS_PARM_3_COMMAND 103
#define WINKEY_LOAD_SETTINGS_PARM_4_COMMAND 104
#define WINKEY_LOAD_SETTINGS_PARM_5_COMMAND 105
#define WINKEY_LOAD_SETTINGS_PARM_6_COMMAND 106
#define WINKEY_LOAD_SETTINGS_PARM_7_COMMAND 107
#define WINKEY_LOAD_SETTINGS_PARM_8_COMMAND 108
#define WINKEY_LOAD_SETTINGS_PARM_9_COMMAND 109
#define WINKEY_LOAD_SETTINGS_PARM_10_COMMAND 110
#define WINKEY_LOAD_SETTINGS_PARM_11_COMMAND 111
#define WINKEY_LOAD_SETTINGS_PARM_12_COMMAND 112
#define WINKEY_LOAD_SETTINGS_PARM_13_COMMAND 113
#define WINKEY_LOAD_SETTINGS_PARM_14_COMMAND 114
#define WINKEY_LOAD_SETTINGS_PARM_15_COMMAND 115
#define SEND_BUFFER_SIZE 150

#define WINKEY_HOUSEKEEPING 0
#define SERVICE_SERIAL_BYTE 1

#define WINKEY_UNBUFFERED_SPEED 0
#define WINKEY_BUFFERED_SPEED 1

#define winkey_paddle_echo_buffer_decode_timing_factor 0.25

//#define DEBUG_WINKEY 1



#define default_length_letterspace 3
#define default_length_wordspace 7


#define potentiometer_check_interval_ms 150
#define potentiometer_reading_threshold 1 
#define initial_pot_wpm_low_value 13     // Potentiometer WPM fully CCW
#define initial_pot_wpm_high_value 55    // Potentiometer WPM fully CW
#define potentiometer_change_threshold 0.9 // don't change the keyer speed until pot wpm has changed more than this

typedef struct  {
	char 			ch;
	uint16_t	key;
} morse_t;


typedef struct  {
	uint8_t	gpio:4;
	uint8_t	on:4;
	uint32_t timer;
} ch_t;

typedef struct  {
	uint8_t		Ident[4];					// Ident 0x55 0x66 0x55 0x66
	uint8_t current_tx;
  uint16_t	ptt_lead_time;
	
  uint16_t	ptt_tail_time;
	uint16_t		dah_to_dit_ratio;
	
	uint16_t	chdelay[4];				// output channel delay in ms.
	
	uint16_t	chtail[4];				// output channel tail time in ms.
	
	uint8_t		chrev[4];					// output reverse flag.

	uint8_t		cw_reverse;				// CW key dot/dash reverse
	uint8_t		cw_keyreverse;		// CW output reverse

	uint8_t	ptt_hang_time_wordspace_units;
	uint8_t wpm;
	uint8_t wpm_farnsworth;
	uint8_t paddle_mode;
	uint8_t keyer_mode;
	uint8_t pot_activated;
	uint8_t weighting;
	uint8_t length_wordspace;
	uint8_t ptt_buffer_hold_active;
	uint8_t paddle_interruption_quiet_time_element_lengths;
	uint32_t	__padding;				// Gap for flash programming
} settings_t;

typedef struct  {
	uint8_t ptt_line_activated:1;
	uint8_t manual_ptt_invoke:1;
	uint8_t key_state:1;					// 0 = key up, 1 = key down
	uint8_t last_ptt:1;						// last PTT signal state
	uint8_t config_dirty:1;				// setting need to be stored on flash
	uint8_t com_event:1;					// Com port event detected
	uint8_t winkey_command:1;
	uint8_t pause_sending_buffer:1;
	uint8_t loop_element_lengths_breakout_flag:1;
	uint8_t winkey_host_open:1;
	uint8_t winkey_dit_invoke:1;
	uint8_t winkey_dah_invoke:1;
	uint8_t winkey_serial_echo:1;
	uint8_t winkey_paddle_echo_activated:1;
	uint8_t send_winkey_breakin_byte_flag:1;
	uint8_t  winkey_pinconfig_ptt_bit:1;
	uint8_t winkey_interrupted:1;
	uint8_t winkey_breakin_status_byte_inhibit:1;
	uint8_t dot_buffer:1;
	uint8_t dash_buffer:1;
	uint8_t iambic_flag:1;
	uint8_t inCommand:1;
	uint8_t irq_adc:1;
} flags_t;

void StoreSetting(void);
void OnUsbDataRx(uint8_t* dataIn, uint32_t length, uint16_t index);
void USB_print(char *msg);
void WinkeyPortWrite(uint8_t byte_to_send, uint8_t override_filter);

void SetCw(char on);				/* Set CW keyer output*/
void SetCh(uint8_t n, uint8_t on);
void SetTxLed(char on);
void SetAutoled(char on);
void SaveDefaultConfig(void);
void DisplaySettings(void);
void Error_Handler(void);

// From commands.c:
void ExecuteCmd(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
