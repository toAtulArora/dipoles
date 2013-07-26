/* vim: set sw=8 ts=8 si : */
/****************************************************************************
Title	:   HD44780U LCD library
Authors:   
Based on Volker Oth's lcd library (http://members.xoom.com/volkeroth)
modified by Peter Fleury's (http://jump.to/fleury). Flexible pin
configuration by Markus Ermert. Adapted for the linuxfocus LCD display
by Guido Socher.

Software:  AVR-GCC with AVR-AS
Target:    any AVR device
Copyright: GPL
       
*****************************************************************************/
#include <avr/io.h>
#include <avr/pgmspace.h>
#define F_CPU 8000000UL  // 1 MHz
#include <util/delay.h>
#include "lcd_hw.h"
#include "lcd.h"

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif

#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

/* 
** constants/macros 
*/

#define lcd_e_high()    LCD_E_PORT|=(1<<LCD_E_PIN)
#define lcd_e_low()     LCD_E_PORT&=~(1<<LCD_E_PIN)
#define lcd_e_toggle()  toggle_e()

#define lcd_cmd_mode()	cbi(LCD_RS_PORT, LCD_RS_PIN)	/* RS=0  command mode   */
#define lcd_data_mode()	sbi(LCD_RS_PORT, LCD_RS_PIN)	/* RS=1  data mode      */
#define lcd_wr_mode()		cbi(LCD_RW_PORT, LCD_RW_PIN)	/* RW=0  write mode     */
#define lcd_rd_mode()		sbi(LCD_RW_PORT, LCD_RW_PIN)	/* RW=1  read mode      */
#define lcd_data_port_in()	{	/* defines all data pins as input */ \
					cbi(LCD_DATA_DDR_D7,LCD_DATA_PIN_D7);\
					cbi(LCD_DATA_DDR_D6,LCD_DATA_PIN_D6);\
					cbi(LCD_DATA_DDR_D5,LCD_DATA_PIN_D5);\
					cbi(LCD_DATA_DDR_D4,LCD_DATA_PIN_D4);\
				}
#define lcd_data_port_out()	{	/* defines all data pins as output */ \
					sbi(LCD_DATA_DDR_D7,LCD_DATA_PIN_D7);\
					sbi(LCD_DATA_DDR_D6,LCD_DATA_PIN_D6);\
				 	sbi(LCD_DATA_DDR_D5,LCD_DATA_PIN_D5);\
				 	sbi(LCD_DATA_DDR_D4,LCD_DATA_PIN_D4);\
				}

#if LCD_LINES==1
#define LCD_FUNCTION_DEFAULT    LCD_FUNCTION_4BIT_1LINE
#else
#define LCD_FUNCTION_DEFAULT    LCD_FUNCTION_4BIT_2LINES
#endif


/* 
** function prototypes 
*/
static void toggle_e(void);
static void lcd_out_high(char d);
static void lcd_out_low(char d);

/*
** local functions
*/

void lcd_delay_ms(unsigned int ms)
/* delay for a minimum of <ms> */
{
        // we use a calibrated macro. This is more
        // accurate and not so much compiler dependent
        // as self made code.
	//
        while(ms){
                _delay_ms(0.96);
                ms--;
        }
}

static void lcd_out_low(char d)
{	/* output low nibble */
	if (d&0x08)  sbi(LCD_DATA_PORT_D7,LCD_DATA_PIN_D7);
		else cbi(LCD_DATA_PORT_D7,LCD_DATA_PIN_D7);
	if (d&0x04)  sbi(LCD_DATA_PORT_D6,LCD_DATA_PIN_D6);
		else cbi(LCD_DATA_PORT_D6,LCD_DATA_PIN_D6);
	if (d&0x02)  sbi(LCD_DATA_PORT_D5,LCD_DATA_PIN_D5);
		else cbi(LCD_DATA_PORT_D5,LCD_DATA_PIN_D5);
	if (d&0x01)  sbi(LCD_DATA_PORT_D4,LCD_DATA_PIN_D4);
		else cbi(LCD_DATA_PORT_D4,LCD_DATA_PIN_D4); 
}
static void lcd_out_high(char d)
{	/* output high nibble */ 
	if (d&0x80)  sbi(LCD_DATA_PORT_D7,LCD_DATA_PIN_D7);
		else cbi(LCD_DATA_PORT_D7,LCD_DATA_PIN_D7);
	if (d&0x40)  sbi(LCD_DATA_PORT_D6,LCD_DATA_PIN_D6);
		else cbi(LCD_DATA_PORT_D6,LCD_DATA_PIN_D6);
	if (d&0x20)  sbi(LCD_DATA_PORT_D5,LCD_DATA_PIN_D5);
		else cbi(LCD_DATA_PORT_D5,LCD_DATA_PIN_D5);
	if (d&0x10)  sbi(LCD_DATA_PORT_D4,LCD_DATA_PIN_D4);
		else cbi(LCD_DATA_PORT_D4,LCD_DATA_PIN_D4); 
}

static void toggle_e(void)
/* toggle Enable Pin */
{
	lcd_e_high();
	_delay_us(2);
	lcd_e_low();
}


static void lcd_write(char data, uint8_t rs)
{
	/* configure data pins as output */
	lcd_data_port_out();

	/* output high nibble first */

	lcd_out_high(data);

	if (rs)
		lcd_data_mode();	/* RS=1: write data            */
	else
		lcd_cmd_mode();	/* RS=0: write instruction     */
	lcd_wr_mode();		/* RW=0  write mode         */
	lcd_e_toggle();

	/* output low nibble */
	lcd_out_low(data);

	if (rs)
		lcd_data_mode();	/* RS=1: write data            */
	else
		lcd_cmd_mode();	/* RS=0: write instruction     */
	lcd_wr_mode();		/* RW=0  write mode         */

	lcd_e_toggle();

	/* all data pins high (inactive) */
	lcd_data_port_in();
}


static uint8_t lcd_read(uint8_t rs)
{
	register uint8_t data;

	/* configure data pins as input */
	lcd_data_port_in();

	if (rs)
		lcd_data_mode();	/* RS=1: read data      */
	else
		lcd_cmd_mode();	/* RS=0: read busy flag */
	lcd_rd_mode();		/* RW=1  read mode      */


	lcd_e_high();
	_delay_us(2);

	/* read high nibble first */


	data =
	    bit_is_set(LCD_DATA_PINR_D7, LCD_DATA_PIN_D7) ? 0x80 : 0;
	data |=
	    bit_is_set(LCD_DATA_PINR_D6, LCD_DATA_PIN_D6) ? 0x40 : 0;
	data |=
	    bit_is_set(LCD_DATA_PINR_D5, LCD_DATA_PIN_D5) ? 0x20 : 0;
	data |=
	    bit_is_set(LCD_DATA_PINR_D4, LCD_DATA_PIN_D4) ? 0x10 : 0;


	lcd_e_low();
	_delay_us(2);		/* Enable low       */

	lcd_e_high();
	_delay_us(2);

	/* read low nibble        */


	data |=
	    bit_is_set(LCD_DATA_PINR_D7, LCD_DATA_PIN_D7) ? 0x08 : 0;
	data |=
	    bit_is_set(LCD_DATA_PINR_D6, LCD_DATA_PIN_D6) ? 0x04 : 0;
	data |=
	    bit_is_set(LCD_DATA_PINR_D5, LCD_DATA_PIN_D5) ? 0x02 : 0;
	data |=
	    bit_is_set(LCD_DATA_PINR_D4, LCD_DATA_PIN_D4) ? 0x01 : 0;


	lcd_e_low();

	return (data);
}


static unsigned char lcd_waitbusy(void)
/* loops while lcd is busy, reads address counter */
{
	register unsigned char c;

	while ((c = lcd_read(0)) & (1 << LCD_BUSY)) {
	}

	return (c); // return address counter=position
}




/*
** PUBLIC FUNCTIONS 
*/

void lcd_command(char cmd)
/* send commando <cmd> to LCD */
{
	lcd_waitbusy();
	lcd_write(cmd, 0);
}


void lcd_gotoxy(uint8_t x, uint8_t y)
/* goto position (x,y) */
{
#if LCD_LINES==1
	lcd_command((1 << LCD_DDRAM) + LCD_START_LINE1 + x);
#endif
#if LCD_LINES==2
	if (y == 0)
		lcd_command((1 << LCD_DDRAM) + LCD_START_LINE1 + x);
	else
		lcd_command((1 << LCD_DDRAM) + LCD_START_LINE2 + x);
#endif
#if LCD_LINES==3
	if (y == 0)
		lcd_command((1 << LCD_DDRAM) + LCD_START_LINE1 + x);
	else if (y == 1)
		lcd_command((1 << LCD_DDRAM) + LCD_START_LINE2 + x);
	else if (y == 2)
		lcd_command((1 << LCD_DDRAM) + LCD_START_LINE3 + x);
#endif
#if LCD_LINES==4
	if (y == 0)
		lcd_command((1 << LCD_DDRAM) + LCD_START_LINE1 + x);
	else if (y == 1)
		lcd_command((1 << LCD_DDRAM) + LCD_START_LINE2 + x);
	else if (y == 2)
		lcd_command((1 << LCD_DDRAM) + LCD_START_LINE3 + x);
	else			/* y==3 */
		lcd_command((1 << LCD_DDRAM) + LCD_START_LINE4 + x);
#endif

}				/* lcd_gotoxy */


void lcd_clrscr(void)
/* clear lcd and set cursor to home position */
{
	lcd_command(1 << LCD_CLR);
}


void lcd_home(void)
/* set cursor to home position */
{
	lcd_command(1 << LCD_HOME);
}


void lcd_putc(char c)
/* print character at current cursor position */
{
	lcd_waitbusy();	// read busy-flag and address counter
	lcd_write(c, 1);
}

void rolltext_lcd_p(prog_char *text, int shiftlen){
        unsigned char i;
        unsigned char j;
        unsigned char textlen=0;
        while (pgm_read_byte(text+textlen)){
                textlen++;
        }
        if (shiftlen <= ROLLLEN){
		i=0;
                j=ROLLLEN-shiftlen + 1;
                while(i < j){
                        lcd_putc(' ');
                        i++;
                }
		j=0;
        }else{
                j=shiftlen-ROLLLEN;
		i=0;
        }
        if (j <= textlen) {
                while(i < ROLLLEN){
                        if (pgm_read_byte(text+j)=='\0') break;
                        lcd_putc(pgm_read_byte(text+j));
                        j++;
                        i++;
                }
        }
}

void lcd_puts(const char *s)
/* print string on lcd  */
{
	while (*s) {
		lcd_putc(*s);
		s++;
	}

}


void lcd_puts_p(const prog_char *progmem_s)
/* print string from program memory on lcd  */
{
	register char c;

	while ((c = pgm_read_byte(progmem_s++))) {
		lcd_putc(c);
	}

}


void lcd_init(uint8_t dispAttr)
/* initialize display and select type of cursor */
/* dispAttr: LCD_DISP_OFF, LCD_DISP_ON, LCD_DISP_ON_CURSOR, LCD_DISP_CURSOR_BLINK */
{
    /*------ Initialize lcd to 4 bit i/o mode -------*/

	lcd_data_port_out();	/* all data port bits as output */
	sbi(LCD_RS_DDR, LCD_RS_PIN);	/* RS pin as output */
	sbi(LCD_RW_DDR, LCD_RW_PIN);	/* RW pin as output */
	sbi(LCD_E_DDR, LCD_E_PIN);	/* E  pin as output */

	lcd_delay_ms(16);	/* wait 16ms or more after power-on       */

	/* initial write to lcd is 8bit */
	lcd_out_high(LCD_FUNCTION_8BIT_1LINE);
	lcd_e_toggle();
	lcd_delay_ms(6);	/* delay, busy flag can't be checked here */

	lcd_out_high(LCD_FUNCTION_8BIT_1LINE);
	lcd_e_toggle();
	lcd_delay_ms(6);	/* delay, busy flag can't be checked here */

	lcd_out_high(LCD_FUNCTION_8BIT_1LINE);
	lcd_e_toggle();
	lcd_delay_ms(6);	/* delay, busy flag can't be checked here */

	lcd_out_high(LCD_FUNCTION_4BIT_1LINE);	/* set IO mode to 4bit */
	lcd_e_toggle();

	/* from now the lcd only accepts 4 bit I/O, we can use lcd_command() */
	lcd_command(LCD_FUNCTION_DEFAULT);	/* function set: display lines  */
	lcd_command(LCD_DISP_OFF);	/* display off                  */
	lcd_clrscr();		/* display clear                */
	lcd_command(LCD_MODE_DEFAULT);	/* set entry mode               */
	lcd_command(dispAttr);	/* display/cursor control       */

}
