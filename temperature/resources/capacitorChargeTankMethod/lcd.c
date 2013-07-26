//
// Title        : LCD driver and other stuff
// Author       : Lars Pontoppidan Larsen
// Date         : Jan 2006
// Version      : 1.00
// Target MCU   : Atmel AVR Series
//
// DESCRIPTION:
// This module implements sending chars and strings to a HD44780 compatible LCD,
// and various other helpfull functions are present.
//
// Display initialization:
//   void lcd_init()           
//
// Sending a zero-terminated string (from s-ram) at pos (0-31):
//   void lcd_string(char *p, unsigned char pos);  
//
// Clear display:
//   void lcd_clear(void);
//
// OTHER STUFF:
//   void ms_spin(unsigned short ms);  
//   void hex2ascii(char *target, long value, char pointplace);
//   char long2ascii(char *target, unsigned long value);
//
// DISCLAIMER:
// The author is in no way responsible for any problems or damage caused by
// using this code. Use at your own risk.
//
// LICENSE:
// This code is distributed under the GNU Public License
// which can be found at http://www.gnu.org/licenses/gpl.txt
//


#include <avr/io.h>

#include <stdlib.h>
#include <util/delay.h>
#include "lcd.h"


#define DISP_ON      0x0C //0b00001100  //LCD control constants
#define DISP_OFF     0x08 //0b00001000  //
#define CLR_DISP     0x01 //0b00000001  //
#define CUR_HOME     0x02 //0b00000010  //
#define DD_RAM_ADDR  0x80 //0b10000000  //
#define DD_RAM_ADDR2 0xC0 //0b11000000  //  
#define DD_RAM_ADDR3 0x28 //40    //      
#define CG_RAM_ADDR  0x40 //0b01000000  //      


#define SWAP_NIBBLES(x) asm volatile("swap %0" : "=r" (x) : "0" (x))

/* Use these defines to specify lcd port and RS, EN pin */
#define PORT PORTB  
#define DDR DDRB

#define RS_BIT 5
#define EN_BIT 4


/* DELAY FUNCTIONS */

#define LOOPS_PER_MS (F_CPU/4000)
void ms_spin(unsigned short ms)
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


/* spin for ms milliseconds */
/*
void ms_spin(unsigned short ms)
{
        if (!ms)
                return;

        // the inner loop takes 4 cycles per iteration 
        __asm__ __volatile__ (
                "1:                     \n"
                "       ldi r26, %3     \n"
                "       ldi r27, %2     \n"
                "2:     sbiw r26, 1     \n"
                "       brne 2b         \n"
                "       sbiw %0, 1      \n"
                "       brne 1b         \n"
                : "=w" (ms)
                : "w" (ms), "i" (LOOPS_PER_MS >> 8), "i" (0xff & LOOPS_PER_MS)
                );
}*/


/* 8-bit count, 3 cycles/loop */
/*
static inline void
_delay_loop_1(unsigned char __count)
{
        if (!__count)
                return;

        asm volatile (
                "1: dec %0" "\n\t"
                "brne 1b"
                : "=r" (__count)
                : "0" (__count)
        );
}
*/

void lcd_putchar(unsigned char rs, unsigned char data )
{
  // must set LCD-mode before calling this function!
  // RS = 1 LCD in character-mode
  // RS = 0 LCD in command-mode
  
  if (rs)
    rs = (1<<RS_BIT);
  
  /* High nibble, rsbit and EN low */
  PORT = (0x0F & (data >> 4)) | rs ;
  
  /* Clock cyclus */
  PORT |= (1<<EN_BIT);
  
  _delay_loop_1(5);
  
  PORT &= ~(1<<EN_BIT);
  
  /* Wait a little */
  ms_spin(2);
  
  /* Low nibble, rsbit and EN low*/
  PORT = (data & 0x0F) | rs;
  
  /* Clock cyclus */
  PORT |= (1<<EN_BIT);
  
  _delay_loop_1(5);
  
  PORT &= ~(1<<EN_BIT);
  
  /* Wait a little */
  ms_spin(2);
}


void lcd_init( void ) // must be run once before using the display
{
  /* Set ddr all out */
  PORT = 0; 
  DDR = 0xFF;
	
  /* Power on wait */

  ms_spin(50);   
  
  
  /* Configure 4 bit access */
  
  lcd_putchar(0, 0x33);
  lcd_putchar(0, 0x32);
  
  
  /* Setup lcd */
  
  lcd_putchar(0, 0x28);
  lcd_putchar(0, 0x08);
  lcd_putchar(0, 0x0c);
  lcd_putchar(0, 0x01);
  lcd_putchar(0, 0x06);
  
}

void lcd_clear(void)
{
  /* Display clear  */
  lcd_putchar(0, CLR_DISP);  

}

void lcd_string(char *p, unsigned char pos)
{
    
    // place cursor
    if (pos < 16) {
      lcd_putchar(0, DD_RAM_ADDR + pos);
    } 
    else if (pos < 32) {
        lcd_putchar(0, DD_RAM_ADDR2 + (pos-16));
    } 
    else
       return;
    
    // Write text
    while (*p) {
      if (pos > 31)
        break;
        
      lcd_putchar(1, *(p++));
      
      if (++pos == 16)
        lcd_putchar(0, DD_RAM_ADDR2);
      
    }
}

/* String functions */

/*
   Writes value as hexadecimals in target. 9 characters will be written.
   pointplace puts a point in the number, example:
   
   0123.4567  (pointplace = 2)
   89ABCD.EF  (pointplace = 1)
*/
void hex2ascii(char *target, long value, char pointplace)
{
   int i;
   unsigned char hex;
   
   for (i=3; i>=0; i--) {
     
     hex = value>>24;   /* Get msbyte */
     SWAP_NIBBLES(hex); /* Get high nibble */
     hex &= 0x0F;
     
     *(target++) = ((hex < 0x0A) ? (hex + '0') : (hex + ('A' - 0x0A)));
     
     hex = value>>24;   /* Get msbyte */     
     hex &= 0x0F;       /* Get low nibble */
     
     *(target++) = ((hex < 0x0A) ? (hex + '0') : (hex + ('A' - 0x0A)));
     
     value <<= 8;
     
     if (i == pointplace) 
       *(target++) = '.';
      
   }
   
   
}


// /*
//    Writes a unsigned long as 13 ascii decimals:
//    
//    x.xxx.xxx.xxx
// */

// unsigned long tenths_tab[10] = {1000000000,100000000,10000000,1000000,100000,10000,1000,100,10,1};
// void long2ascii(char *target, unsigned long value)
// {
//    unsigned char p, pos=0;
//    unsigned char numbernow=0;
//    
//    for (p=0;p<10;p++) {
//      
//      if ((p==1) || (p==4) || (p==7)) {
//        if (numbernow)
//          target[pos] = '.';  
//        else
//          target[pos] = ' ';  
//          
//        pos++;
//      }
//      
//      if (value < tenths_tab[p]) {
//        if (numbernow)
//          target[pos] = '0';  
//        else
//          target[pos] = ' ';   
//      } 
//      else {
//        target[pos] = '0';
//        while (value >= tenths_tab[p]) {
//          target[pos]++;
//          value -= tenths_tab[p];
//        }
//        numbernow = 1;
//     }
//     pos++;
//   }
//   
// }

/*
   Writes a unsigned long as 4 ascii decimals + a dot. Always writes 5 ascii chars.
   Returns dot place.
   
   examples:      returns:
  "a.aaa"         3
    "aaa.a"       2
     "aa.aa"      2
          "a.aaa" 1
          "  aaa" 0 
          "    a" 0 
   x.xxx.xxx.xxx
*/

unsigned long tenths_tab[10] = {1000000000,100000000,10000000,1000000,100000,10000,1000,100,10,1};
char long2ascii(char *target, unsigned long value)
{
  unsigned char p, pos=0;
  unsigned char numbernow=0;
  char ret=0;
  
  for (p=0;(p<10) && (pos<5);p++) {
    
    if (numbernow) {
      /* Eventually place dot */
      /* Notice the nice fallthrough construction. */
      switch(p) {
      case 1: 
        ret++;
      case 4: 
        ret++;
      case 7: 
        ret++;
        target[pos] = '.';  
        pos++;
      }      
    }
    
    if (value < tenths_tab[p]) {
      if (numbernow) {
        /* Inside number, put a zero */
        target[pos] = '0';  
        pos++;
      }
      else {
        /* Check if we need to pad with spaces */
        if (p>=6) {
          target[pos] = ' ';
          pos++;
        }
        
        if (p==6) {
          /* We also need to place a space instead of . */
          target[pos] = ' ';
          pos++;
        }
      }
    } 
    else {
      target[pos] = '0';
      while (value >= tenths_tab[p]) {
        target[pos]++;
        value -= tenths_tab[p];
      }
      pos++;
      numbernow = 1;
    }
  }
  
  return ret;
}

