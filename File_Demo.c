#include <stdio.h>                      
#include <ctype.h>                      
#include <string.h>                    
#include "cmsis_os.h"                   
#include "rl_fs.h"      
#include "rl_net.h"
#include "board.h"

// Text file that contains stock info
#define DATA_FILE "FTP_root\\stocks.txt"

// Used for pause()
#define DATA_DELAY		2
#define SCROLL_DELAY 	5000000

// Used in digital_write()
#define CS		0 // PTD0
#define CLK 	1 // PTD1
#define DIN 	2 // PTD2
#define LOW 	0
#define HIGH	1

/*
 * Struct for displaying characters
 * c = char represented, len = # of columns needed for display, cols = data to pass to led matrix
 */
typedef struct {
	char c;
  int len;
  int cols[8];
} char_t;

// Functions
void dhcp_client_notify (uint32_t if_num, dhcpClientOption opt, const uint8_t *val, uint32_t len);
static char *get_drive (char *src, char *dst, uint32_t dst_sz);
static void cmd_format (char *par);
static void init_filesystem (void);
void digital_write(int port, int direction);
void pause(int i);
void write_byte(int data_byte);
void write(int address, int data);
void clear_display(void);
void setup(void);
void shift(void);
void scroll(char_t c);
char_t char2char_t(char c);
void display_string(int s[], int len);
void display_stocks(void);

// Global Variables
int current_text[1000]; // current text being displayed on matrix
int current_text_length = 0; // current length of current_text
int update_occured = 1; // set to 1 when update occurs, set to 0 once current_text has been updated
int STATE[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}; // current display state of the led matrix

/*
 * Drives specified gpio pin low or high
 * port: CS = 0, CLK = 1, DIN = 2
 * direction: low = 0, high = 1
 */
void digital_write(int port, int direction) {
	if (direction) {
		PTD->PSOR |= (1 << port);
	} else {
		PTD->PCOR |= (1 << port);
	}
}

/*
 * Busy wait
 */
void pause(int i) {
	int j;
	for (j = 0; j < i; j++);
}

/*
 * Writes a byte of data through DIN
 */
void write_byte(int data_byte) {
	int i;
	digital_write(CS, LOW);
	for (i = 0; i < 8; i++) {
		digital_write(CLK, LOW);
		digital_write(DIN, data_byte & 0x80);
		data_byte = data_byte << 1;
		pause(DATA_DELAY);
		digital_write(CLK, HIGH);
	}
}

/*
 * Writes data to an address of the led matrix
 */
void write(int address, int data) {
	digital_write(CS, LOW);
	write_byte(address);
	write_byte(data);
	digital_write(CS, HIGH);
}

/*
 * Resets led matrix display
 */
void clear_display() {
	int i;
	for (i = 1; i <= 8; i++){
		write(i, 0);
	}
}

/*
 * Setup for ports and led matrix
 */
void setup() {
	// ports
	SIM->SCGC5 |= SIM_SCGC5_PORTD_MASK;
	PORTD->PCR[CLK] = PORT_PCR_MUX(001);
	PORTD->PCR[CS] = PORT_PCR_MUX(001);
	PORTD->PCR[DIN] = PORT_PCR_MUX(001);
	GPIOD_PDDR |= (1 << CLK);
	GPIOD_PDDR |= (1 << CS); 
	GPIOD_PDDR |= (1 << DIN);
	digital_write(CS, LOW);
	
	// led matrix
	write(9, 0); // decoding
	write(11, 7); // scanlimit
	write(12, 1); // normal power-down mode
	clear_display();
}

/*
 * Move display one unit left, leaving right most column empty
 */
void shift() {
	int i;
	for (i = 7; i >= 1; i--) {
		STATE[i] = STATE[i-1];
		write(i+1, STATE[i]);
	}
	STATE[0] = 0;
	write(1, STATE[0]);
}

/*
 * Scrolls c accross the display
 */
void scroll(char_t c) {
	int i;
	for (i = 0; i < c.len; i++) {
		shift();
		STATE[0] = c.cols[i];
		write(1, STATE[0]);
		pause(SCROLL_DELAY);
	}
	shift();
	pause(SCROLL_DELAY);
}

/*
 * Library of char_t
 */
int char_t_library_len = 41;
char_t char_t_library[] = {
	{'0', 3, {0x3E,0x22,0x3E,0x00,0x00,0x00,0x00,0x00}},
	{'1', 1, {0x3E,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
  {'2', 3, {0x2E,0x2A,0x3A,0x00,0x00,0x00,0x00,0x00}},
	{'3', 3, {0x2A,0x2A,0x3E,0x00,0x00,0x00,0x00,0x00}},
	{'4', 3, {0x38,0x08,0x3E,0x00,0x00,0x00,0x00,0x00}},
	{'5', 3, {0x3A,0x2A,0x2E,0x00,0x00,0x00,0x00,0x00}},
	{'6', 3, {0x3E,0x2A,0x2E,0x00,0x00,0x00,0x00,0x00}},
	{'7', 3, {0x20,0x20,0x3E,0x00,0x00,0x00,0x00,0x00}},
	{'8', 3, {0x3E,0x2A,0x3E,0x00,0x00,0x00,0x00,0x00}},
	{'9', 3, {0x38,0x28,0x3E,0x00,0x00,0x00,0x00,0x00}},
	{'A', 3, {0x3E,0x28,0x3E,0x00,0x00,0x00,0x00,0x00}},
	{'B', 3, {0x3E,0x2A,0x36,0x00,0x00,0x00,0x00,0x00}},
	{'C', 3, {0x1C,0x22,0x22,0x00,0x00,0x00,0x00,0x00}},
	{'D', 3, {0x3E,0x22,0x1C,0x00,0x00,0x00,0x00,0x00}},
	{'E', 3, {0x3E,0x2A,0x22,0x00,0x00,0x00,0x00,0x00}},
	{'F', 3, {0x3E,0x28,0x20,0x00,0x00,0x00,0x00,0x00}},
	{'G', 3, {0x1C,0x22,0x2E,0x00,0x00,0x00,0x00,0x00}},
	{'H', 3, {0x3E,0x8,0x3E,0x00,0x00,0x00,0x00,0x00}},
	{'I', 3, {0x22,0x3E,0x22,0x00,0x00,0x00,0x00,0x00}},
	{'J', 3, {0x24,0x22,0x3E,0x00,0x00,0x00,0x00,0x00}},
	{'K', 4, {0x3E,0x8,0x14,0x22,0x00,0x00,0x00,0x00}},
	{'L', 3, {0x3E,0x2,0x2,0x00,0x00,0x00,0x00,0x00}},
	{'M', 5, {0x3E,0x10,0x8,0x10,0x3E,0x00,0x00,0x00}},
	{'N', 5, {0x3E,0x10,0xC,0x2,0x3E,0x00,0x00,0x00}},
	{'O', 3, {0x3E,0x22,0x3E,0x00,0x00,0x00,0x00,0x00}},
	{'P', 3, {0x3E,0x28,0x38,0x00,0x00,0x00,0x00,0x00}},
	{'Q', 4, {0x1C,0x22,0x24,0x1A,0x00,0x00,0x00,0x00}},
	{'R', 3, {0x3E,0x28,0x36,0x00,0x00,0x00,0x00,0x00}},
	{'S', 3, {0x1A,0x2A,0x2C,0x00,0x00,0x00,0x00,0x00}},
	{'T', 3, {0x20,0x3E,0x20,0x00,0x00,0x00,0x00,0x00}},
	{'U', 3, {0x3E,0x2,0x3E,0x00,0x00,0x00,0x00,0x00}},
	{'V', 3, {0x3C,0x2,0x3C,0x00,0x00,0x00,0x00,0x00}},
	{'W', 5, {0x3E,0x4,0x8,0x4,0x3E,0x00,0x00,0x00}},
	{'X', 3, {0x36,0x8,0x36,0x00,0x00,0x00,0x00,0x00}},
	{'Y', 3, {0x30,0xE,0x30,0x00,0x00,0x00,0x00,0x00}},
	{'Z', 3, {0x26,0x2A,0x32,0x00,0x00,0x00,0x00,0x00}},
	{' ', 2, {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
	{'.', 1, {0x2,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
	{'%', 5, {0x32,0x34,0x8,0x16,0x26,0x00,0x00,0x00}},
	{'+', 5, {0x10,0x20,0x7E,0x20,0x10,0x00,0x00,0x00}},
	{'-', 5, {0x8,0x4,0x7E,0x4,0x8,0x00,0x00,0x00}}
};
 
/*
 * Converts char to char_t
 */
char_t char2char_t(char c) {
	int i;
	for (i = 0; i < char_t_library_len; i++) {
		
		if (c == char_t_library[i].c || c == toupper(char_t_library[i].c)) {
			return char_t_library[i];
		}
	}
	// no match, return space
	return (char_t) {' ', 2, {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}};
}

/*
 * Displays s on the led matrix
 */
void display_string(int s[], int len) {
	int i = 0;
	for (i = 0; i < len; i++) {
		scroll(char2char_t(s[i]));
	}
}

// updates current_text, displays current_text
void display_stocks() {
	if (update_occured) {
		FILE * f;
		f = fopen(DATA_FILE, "r");
		update_occured = 0;
		if (f == NULL) {
			fclose(f);
			return;
		}
		int ch;
		int i = 0;
		ch = fgetc(f);
		while (ch != EOF) {
			current_text[i] = ch;
			i++;
			ch = fgetc(f);
		}
		current_text_length = i;
		update_occured = 0;
	}
	display_string(current_text, current_text_length);
}

// Alert for when data file is updated
void netFTPs_Notify (netFTPs_Event event) {
	switch (event) {
		case netFTPs_EventUpload:
			update_occured = 1;
			break;
		default:
			return;
	}
}

/*-----------------------------------------------------------------------------
 * Helper functions definitions from 
 * https://www.keil.com/pack/doc/mw/Network/html/_f_t_p__server__example.html
 *----------------------------------------------------------------------------*/
/// IP address change notification
bool MSGupdate; char msg_text[64];
void dhcp_client_notify (uint32_t if_num,
                         dhcpClientOption opt, const uint8_t *val, uint32_t len) {
  if (opt == dhcpClientIPaddress) {
    // IP address has changed
    sprintf (msg_text,"IP address: %s\n", ip4_ntoa (val));
    MSGupdate = true;
  }
}
// Defining constants											 
#define CMD_COUNT   (sizeof (cmd) / sizeof (cmd[0]))
#ifdef __RTX
extern uint32_t os_time;
uint32_t HAL_GetTick(void) { 
  return os_time; 
}
#endif
/*------------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
 * Helper functions from 
 * https://www.keil.com/pack/doc/mw/Network/html/_f_t_p__server__example.html
 *----------------------------------------------------------------------------*/
// Extract drive specification from the input string
static char *get_drive (char *src, char *dst, uint32_t dst_sz) {
  uint32_t i, n;
  i = 0;
  n = 0;
  while (!n && src && src[i] && (i < dst_sz)) {
    dst[i] = src[i];
    if (dst[i] == ':') {
      n = i + 1;
    }
    i++;
  }
  if (n == dst_sz) {
    n = 0;
  }
  dst[n] = '\0';
  return (src + n);
}
// Format device
static void cmd_format (char *par) {
  char  label[12];
  char  drive[4];
  int   retv;
  par = get_drive (par, drive, 4);
  printf ("\nProceed with Format [Y/N]\n");
  retv = getchar();
  if (retv == 'y' || retv == 'Y') {
    /* Format the drive */
    if (fformat (drive, par) == fsOK) {
      printf ("Format completed.\n");
      if (fvol (drive, label, NULL) == 0) {
        if (label[0] != '\0') {
          printf ("Volume label is \"%s\"\n", label);
        }
      }
    }
    else {
      printf ("Formatting failed.\n");
    }
  }
  else {
    printf ("Formatting canceled.\n");
  }
}
// Initialize a flash memory card
static void init_filesystem (void) {
  fsStatus stat;
  printf ("Initializing and mounting enabled drives...\n\n");
  /* Initialize and mount drive "M0" */
  stat = finit ("M0:");
  if (stat == fsOK) {
    stat = fmount ("M0:");
    if (stat == fsOK) {
      printf ("Drive M0 ready!\n");
    }
    else if (stat == fsNoFileSystem) {
      /* Format the drive */
      printf ("Drive M0 not formatted!\n");
      cmd_format ("M0:");
    }
    else {
      printf ("Drive M0 mount failed with error code %d\n", stat);
    }
  }
  else {
    printf ("Drive M0 initialization failed!\n");
  }
  printf ("\nDone!\n");
}
/*------------------------------------------------------------------------------*/


int main() {
	// initializations
	setup();
	hardware_init();
	init_filesystem ();
	net_initialize();
	if (finit("") == fsOK) {
    fmount("");
  }
	
	// main loop
	while(1) {
		net_main();
		display_stocks();
	}
}


