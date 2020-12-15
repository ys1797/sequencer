/**
  ******************************************************************************
  * @file           : commands.c
  * @brief          : USB command processsing
  ******************************************************************************
  * @attention
  *
	* <h2><center>&copy; COPYRIGHT(c) 2016 S54MTB</center></h2>
  *
  ******************************************************************************
  */

#include "stm32f0xx.h"                  // Device header
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "main.h"
#include <ctype.h>

/*
 * Command identifiers	
 */
enum {
	CMD_DEFAULT,
	CMD_DELAY1,
	CMD_DELAY2,
	CMD_DELAY3,
	CMD_DELAY4,
	CMD_CHREV1,
	CMD_CHREV2,
	CMD_CHREV3,
	CMD_CHREV4,
	CMD_CWREV,
	CMD_KEYREVERSE,
	CMD_DOTRATIO,
	CMD_WPM,
	CMD_HANGTIME,
	CMD_D,
	// Add more 
	CMD_LAST
};


// command table
struct cmd_st {
  const char *cmdstr;
  int id;
};


// Help text 
char *helptext = 
	" Cmds:\n\r"
	"DEFAULT\n\r"
  "DELAY1 ms\n\r"
	"DELAY2 ms\n\r"
	"DELAY3 ms\n\r"
	"DELAY4 ms\n\r"
  "CH1_REV x\n\r"
	"CH2_REV x\n\r"
	"CH3_REV x\n\r"
	"CH4_REV x\n\r"
  "CW_REV x\n\r"
	"KEYREVERSE x\n\r"
  "DOTRATIO x\n\r"
	"WPM x\n\r"
	"HANGTIME x\n\r"
	"D\n\r"
  "\n\r";

/**
 *	Command strings - match command with command ID 
 */
const struct cmd_st cmd_tbl [] = {
	{ "DEFAULT", 		CMD_DEFAULT, },
	{ "DELAY1", 		CMD_DELAY1,  },
	{ "DELAY2", 		CMD_DELAY2,  },
	{ "DELAY3", 		CMD_DELAY3,  },
	{ "DELAY4", 		CMD_DELAY4,  },
	{ "CH1_REV", 		CMD_CHREV1,  },
	{ "CH2_REV", 		CMD_CHREV2,  },
	{ "CH3_REV", 		CMD_CHREV3,  },
	{ "CH4_REV", 		CMD_CHREV4,  },
	{ "CW_REV", 		CMD_CWREV,  },
	{ "KEYREVERSE", CMD_KEYREVERSE,},
	{ "DOTRATIO",		CMD_DOTRATIO, },
	{ "WPM",				CMD_WPM, },
	{ "HANGTIME",		CMD_HANGTIME, },
	{ "D",					CMD_D, },
};

#define CMD_TBL_LEN (sizeof (cmd_tbl) / sizeof (cmd_tbl [0]))

extern settings_t setting;
extern flags_t flags;							// One bit flags arrray
extern uint32_t Rx_ptr_in;

/** Globals */
extern char Rx_Buffer[RX_DATA_SIZE];


static void cmd_default(char *argstr_buf);
static void cmd_delay(char ch, char *argstr_buf);
static void cmd_chrev(char ch, char *argstr_buf);
static void cmd_cwrev(char *argstr_buf);
static void cmd_keyreverse(char *argstr_buf);
static void cmd_dotratio(char *argstr_buf);
static void cmd_wpm(char *argstr_buf);
static void cmd_hangtime(char *argstr_buf);
static void cmd_unknown(char *argstr_buf);


char *strupr (char *src)
{
	char *s;
	for (s = src; *s != '\0'; s++) *s = toupper (*s);
	return (src);
}

static int cmdid_search (char *cmdstr)
{
	const struct cmd_st *ctp;

	for (ctp = cmd_tbl; ctp < &cmd_tbl[CMD_TBL_LEN]; ctp++) {
		if (strcmp (ctp->cmdstr, cmdstr) == 0) return (ctp->id);
	}
	return (CMD_LAST);
}



void USB_print(char *msg)
{
	uint8_t tx_res = 0, cnt = 0, l = strlen(msg);
	do {
		tx_res = CDC_Transmit_FS((uint8_t*)msg, l, 0);
	} while (USBD_BUSY == tx_res && ++cnt < 200);
}


/*********************************************************************
 * Function:        void ExecuteCmd(const char *dataIn, int length)
 * PreCondition:    -
 * Input:           command line  
 * Overview:        This function processes the cmd command.
 ********************************************************************/

void ExecuteCmd(void)
{
	unsigned int id;
	char *argstr_buf;
	USB_print("\r\n");
	
	/* First, copy the command and convert it to all uppercase. */
	strupr (Rx_Buffer);
	Rx_ptr_in = 0;
	flags.inCommand = 0;
	
	if (Rx_Buffer[0] == '\0') return;
	
	/*
	 * Next, find the end of the first thing in the buffer.
	 * Since the command ends with a space, we'll look for that.  NULL-Terminate the command
	 * and keep a pointer to the arguments.
	  */
	argstr_buf = strchr(Rx_Buffer, ' ');
	if (argstr_buf) {
		*argstr_buf = '\0';
		argstr_buf++;
	}

	/* Search for a command ID, then switch on it.  Each function invoked here. */
	id = cmdid_search(Rx_Buffer);
	switch (id) {
		case CMD_DEFAULT:
			cmd_default(argstr_buf);
			break;
		case CMD_DELAY1:
			cmd_delay(0, argstr_buf);
			break;
		case CMD_DELAY2:
			cmd_delay(1, argstr_buf);
			break;
		case CMD_DELAY3:
			cmd_delay(2, argstr_buf);
			break;
		case CMD_DELAY4:
			cmd_delay(3, argstr_buf);
			break;
		case CMD_CHREV1:
			cmd_chrev(0, argstr_buf);
			break;
		case CMD_CHREV2:
			cmd_chrev(1, argstr_buf);
			break;			
		case CMD_CHREV3:
			cmd_chrev(3, argstr_buf);
			break;			
		case CMD_CHREV4:
			cmd_chrev(4, argstr_buf);
			break;			
		case CMD_CWREV:
			cmd_cwrev(argstr_buf);
			break;
		case CMD_KEYREVERSE:
			cmd_keyreverse(argstr_buf);
			break;
		case CMD_DOTRATIO:
			cmd_dotratio(argstr_buf);
			break;
		case CMD_WPM:
			cmd_wpm(argstr_buf);
			break;
		case CMD_HANGTIME:		
			cmd_hangtime(argstr_buf);
			break;		
		case CMD_D:
			DisplaySettings();
			break;
		case CMD_LAST:
			cmd_unknown(Rx_Buffer);
			break;
	}
}


static void cmd_default(char *argstr_buf)
{
	USB_print("Loading default..\r\n");
	SaveDefaultConfig();
}



static void cmd_delay(char ch, char *argstr_buf)
{
	char param[32];
	int x;
	
	if (argstr_buf) {
		x = atoi(argstr_buf);
		if ((x>=0) & (x<501)) {
			setting.chdelay[ch] = x;
			flags.config_dirty=1;
		}
	}
	snprintf(param, 32, "Ch#%d delay: %d ms\n\r", ch+1, setting.chdelay[ch]);
	USB_print(param);
}

static void cmd_chrev(char ch, char *argstr_buf)
{
	char param[32];
	int x;
	if (argstr_buf) {
		x = atoi(argstr_buf);
		if (x) setting.chrev[ch] = 1;
		else setting.chrev[ch] = 0;
		flags.config_dirty=1;
	}
	snprintf(param, 32, "Ch#%d reverse: %s\n\r", ch+1, setting.chrev[ch] ? "ON":"OFF");
	USB_print(param);
}


static void cmd_cwrev(char *argstr_buf)
{
	char param[32];
	int x;
	if (argstr_buf) {
		x = atoi(argstr_buf);
		if (x) setting.cw_reverse = 1;
		else setting.cw_reverse = 0;
		flags.config_dirty=1;
	}
	snprintf(param, 32, "CW reverse: %s\n\r", setting.cw_reverse ? "ON":"OFF");
	USB_print(param);
}

static void cmd_keyreverse(char *argstr_buf)
{
	char param[32];
	int x;
	if (argstr_buf) {
		x = atoi(argstr_buf);
		if (x) setting.cw_keyreverse = 1;
		else setting.cw_keyreverse = 0;
		flags.config_dirty=1;
	}
	snprintf(param, 32, "CW output reverse: %s\n\r", setting.cw_keyreverse ? "ON":"OFF");
	USB_print(param);
}

static void cmd_dotratio(char *argstr_buf)
{
	char param[32];
	uint32_t x;
	if (argstr_buf) {
		x = atoi(argstr_buf);
		if (x>=2 && x<10) setting.dah_to_dit_ratio = x;
		flags.config_dirty=1;
	}
	snprintf(param, 32, "Dot ratio: %d\n\r", setting.dah_to_dit_ratio);
	USB_print(param);
}


static void cmd_wpm(char *argstr_buf)
{
	char param[32];
	uint32_t x;
	if (argstr_buf) {
		x = atoi(argstr_buf);
		if (x>0 && x<100) setting.wpm = x;
		flags.config_dirty=1;
	}
	snprintf(param, 32, "WPM: %d\r\n", setting.wpm);
	USB_print(param);
}


static void cmd_hangtime(char *argstr_buf)
{
	char param[32];
	uint32_t x;
	if (argstr_buf) {
		x = atof(argstr_buf);
		if (x<100) setting.ptt_hang_time_wordspace_units = x;
		flags.config_dirty=1;
	}
	snprintf(param, 32, "Hang: %d\n\r", setting.ptt_hang_time_wordspace_units);
	USB_print(param);
}

static void cmd_unknown(char *argstr_buf)
{
	USB_print(helptext);
}

