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
	CMD_CHHEAD,
	CMD_CHTAIL,
	CMD_CHREV,
	CMD_CWREV,
	CMD_KEYREVERSE,
	CMD_DOTRATIO,
	CMD_WPM,
	CMD_HANGTIME,
	CMD_KEYMODE,
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
	" Cmds:\r\n"
	"DEFAULT\r\n"
  "HEAD x x ms\r\n"
  "TAIL x x ms\r\n"
  "REV x xn\r\n"
  "CW_REV x\r\n"
	"KEYREVERSE x\r\n"
  "DOTRATIO x\r\n"
	"WPM x\r\n"
	"HANGTIME x\r\n"
  "KEYMODE s\r\n"
	"D\r\n\r\n";

/**
 *	Command strings - match command with command ID 
 */
const struct cmd_st cmd_tbl [] = {
	{ "DEFAULT", 		CMD_DEFAULT, },
	{ "HEAD", 			CMD_CHHEAD,  },
	{ "TAIL", 			CMD_CHTAIL,  },
	{ "REV", 				CMD_CHREV,  },
	{ "CW_REV", 		CMD_CWREV,  },
	{ "KEYREVERSE", CMD_KEYREVERSE,},
	{ "DOTRATIO",		CMD_DOTRATIO, },
	{ "WPM",				CMD_WPM, },
	{ "HANGTIME",		CMD_HANGTIME, },
	{ "KEYMODE",		CMD_KEYMODE, },
	{ "D",					CMD_D, },
};

#define CMD_TBL_LEN (sizeof (cmd_tbl) / sizeof (cmd_tbl [0]))

extern settings_t setting;
extern flags_t flags;							// One bit flags arrray
extern uint32_t Rx_ptr_in;

/** Globals */
extern char Rx_Buffer[RX_DATA_SIZE];


static void cmd_default(char *argstr_buf);
static void cmd_chhead(char *argstr_buf);
static void cmd_chtail(char *argstr_buf);
static void cmd_chrev(char *argstr_buf);
static void cmd_cwrev(char *argstr_buf);
static void cmd_keyreverse(char *argstr_buf);
static void cmd_dotratio(char *argstr_buf);
static void cmd_wpm(char *argstr_buf);
static void cmd_hangtime(char *argstr_buf);
static void cmd_keymode(char *argstr_buf);
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


/**
  * @brief Print string to USB console interface
  * @param string to send
  * @retval None
  */
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

/**
  * @brief Process command from USB console interface.
  * @param None
  * @retval None
  */
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
		case CMD_CHHEAD:
			cmd_chhead(argstr_buf);
			break;
		case CMD_CHTAIL:
			cmd_chtail(argstr_buf);
			break;
		case CMD_CHREV:
			cmd_chrev(argstr_buf);
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
		case CMD_KEYMODE:
			cmd_keymode(argstr_buf);
			break;
		case CMD_D:
			DisplaySettings();
			break;
		case CMD_LAST:
			cmd_unknown(Rx_Buffer);
			break;
	}
}

/**
  * @brief 
  * @param None
  * @retval None
  */
static void cmd_default(char *argstr_buf)
{
	USB_print("Loading default..\r\n");
	SaveDefaultConfig();
}

/**
  * @brief 
  * @param None
  * @retval None
  */
static void cmd_chhead(char *argstr_buf)
{
char param[32], *p;
	int ch, x;
	
	p = strchr (argstr_buf, ' ');
	if (p == NULL) return;
	*p = '\0';
	p++;
	ch = atoi(argstr_buf) - 1;
	if (ch < 0 && ch > 3)  return;
	x = atoi(p);
	USB_print(p);	
	if ((x>=0) & (x<501)) {
		setting.chhead[ch] = x;
		flags.config_dirty=1;
	}
	snprintf(param, 32, "Ch#%d head: %d ms\n\r", ch+1, setting.chhead[ch]);
	USB_print(param);
}

static void cmd_chtail(char *argstr_buf)
{
char param[32], *p;
	int ch, x;
	
	p = strchr (argstr_buf, ' ');
	if (p == NULL) return;
	*p = '\0';
	p++;
	ch = atoi(argstr_buf) - 1;
	if (ch < 0 && ch > 3)  return;
	
	x = atoi(p);
	if ((x>=0) & (x<501)) {
		setting.chtail[ch] = x;
		flags.config_dirty=1;
	}
	snprintf(param, 32, "Ch#%d tail: %d ms\n\r", ch+1, setting.chtail[ch]);
	USB_print(param);
}


/**
  * @brief 
  * @param None
  * @retval None
  */
static void cmd_chrev(char *argstr_buf)
{
	char param[32], *p;
	int ch, x;
	
	p = strchr (argstr_buf, ' ');
	if (p == NULL) return;
	*p = '\0';
	p++;
	ch = atoi(argstr_buf) - 1;
	x = atoi(p);
	if (ch < 0 && ch > 3)  return;
	
	if (x) setting.chrev[ch] = 1;
	else setting.chrev[ch] = 0;
	flags.config_dirty=1;
	snprintf(param, 32, "Ch#%d reverse: %s\n\r", ch+1, setting.chrev[ch] ? "ON":"OFF");
	USB_print(param);
}


/**
  * @brief 
  * @param None
  * @retval None
  */
static void cmd_cwrev(char *argstr_buf)
{
	char param[32];
	int x;
	if (argstr_buf) {
		x = atoi(argstr_buf);
		if (x) setting.paddle_mode = PADDLE_REVERSE;
		else setting.paddle_mode = PADDLE_NORMAL;
		flags.config_dirty=1;
	}
	snprintf(param, 32, "CW reverse: %s\n\r", setting.paddle_mode ? "ON":"OFF");
	USB_print(param);
}

/**
  * @brief 
  * @param None
  * @retval None
  */
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

/**
  * @brief 
  * @param None
  * @retval None
  */
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

/**
  * @brief 
  * @param None
  * @retval None
  */
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

/**
  * @brief 
  * @param None
  * @retval None
  */
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


/**
  * @brief 
  * @param None
  * @retval None
  */
static void cmd_keymode(char *argstr_buf)
{
	if (argstr_buf) {
		if (!strcasecmp(argstr_buf, "STRAIGHT")) setting.keyer_mode = STRAIGHT;
		else if (!strcasecmp(argstr_buf, "IAMBIC_B")) setting.keyer_mode = IAMBIC_B;
		else if (!strcasecmp(argstr_buf, "IAMBIC_A")) setting.keyer_mode = IAMBIC_A;
		else if (!strcasecmp(argstr_buf, "BUG")) setting.keyer_mode = BUG;
		else if (!strcasecmp(argstr_buf, "ULTIMATIC")) setting.keyer_mode = ULTIMATIC;
		else if (!strcasecmp(argstr_buf, "SINGLE")) setting.keyer_mode = SINGLE_PADDLE;
	
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
		flags.config_dirty=1;
	}
}

/**
  * @brief 
  * @param None
  * @retval None
  */
static void cmd_unknown(char *argstr_buf)
{
	USB_print(helptext);
}

