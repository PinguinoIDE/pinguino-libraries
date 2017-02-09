/*
 *    LedControl.h - A library for controling Leds with a MAX7219/MAX7221
 *    Copyright (c) 2007 Eberhard Fahle
 * 
 *    Permission is hereby granted, free of charge, to any person
 *    obtaining a copy of this software and associated documentation
 *    files (the "Software"), to deal in the Software without
 *    restriction, including without limitation the rights to use,
 *    copy, modify, merge, publish, distribute, sublicense, and/or sell
 *    copies of the Software, and to permit persons to whom the
 *    Software is furnished to do so, subject to the following
 *    conditions:
 * 
 *    This permission notice shall be included in all copies or 
 *    substantial portions of the Software.
 * 
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *    OTHER DEALINGS IN THE SOFTWARE.
 */
 
 /**
  *  2014-02-07 - Régis Blanchot -  Adapted to Pinguino
  *                                 Added a new 8x8 font
  *                                 Added scroll function
  */

#ifndef LEDCONTROL_H
#define LEDCONTROL_H

#include <typedef.h>
#include <stdarg.h>

//the opcodes for the MAX7221 and MAX7219
#define OP_NOOP         0
#define OP_DIGIT0       1
#define OP_DIGIT1       2
#define OP_DIGIT2       3
#define OP_DIGIT3       4
#define OP_DIGIT4       5
#define OP_DIGIT5       6
#define OP_DIGIT6       7
#define OP_DIGIT7       8
#define OP_DECODEMODE   9
#define OP_INTENSITY    10
#define OP_SCANLIMIT    11
#define OP_SHUTDOWN     12
#define OP_DISPLAYTEST  15

// Led-Matrix 8x8

#if defined(LEDCONTROLPRINTCHAR)   || defined(LEDCONTROLPRINT)      || \
    defined(LEDCONTROLPRINTNUMBER) || defined(LEDCONTROLPRINTFLOAT) || \
    defined(LEDCONTROLPRINTF)      || defined(LEDCONTROLSCROLL)
    #include <fonts/font8x8.h>
    const u8 *font_address;
    u8 font_width;
    u8 font_height;
    u8 font_firstchar;
    u8 font_charcount;
    /*
    #define FONT_HEIGHT             0
    #define FONT_WIDTH              1
    #define FONT_OFFSET             2
    */
    #define FONT_LENGTH             0
    #define FONT_FIXED_WIDTH        2
    #define FONT_WIDTH              2
    #define FONT_HEIGHT             3
    #define FONT_FIRST_CHAR         4
    #define FONT_CHAR_COUNT         5
    #define FONT_WIDTH_TABLE        6
    #define FONT_OFFSET             6
#endif

// 7-Segment Displays
 
#if defined(LEDCONTROLSETDIGIT) || defined(LEDCONTROLSETCHAR)
const static u8 charTable[128] = {
    0b01111110,0b00110000,0b01101101,0b01111001,0b00110011,0b01011011,0b01011111,0b01110000,
    0b01111111,0b01111011,0b01110111,0b00011111,0b00001101,0b00111101,0b01001111,0b01000111,
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00000000,0b10000000,0b00000001,0b10000000,0b00000000,
    0b01111110,0b00110000,0b01101101,0b01111001,0b00110011,0b01011011,0b01011111,0b01110000,
    0b01111111,0b01111011,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b01110111,0b00011111,0b00001101,0b00111101,0b01001111,0b01000111,0b00000000,
    0b00110111,0b00000000,0b00000000,0b00000000,0b00001110,0b00000000,0b00000000,0b00000000,
    0b01100111,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00001000,
    0b00000000,0b01110111,0b00011111,0b00001101,0b00111101,0b01001111,0b01000111,0b00000000,
    0b00110111,0b00000000,0b00000000,0b00000000,0b00001110,0b00000000,0b00000000,0b00000000,
    0b01100111,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,
    0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000,0b00000000
};
#endif

/// private:

// SPI bus, either SPISW, SPI1 or SPI2, ...
u8 LEDCONTROL_SPI;
// We keep track of the led-status for all 8 max. devices in this array
u8 status[64];
// The maximum number of devices we use (max. 8)
u8 gLastDevice;
// The current active matrix
u8 gActiveDevice;
// Stores how many pixel the text has been scrolled
// 0 < gScroll < scrollmax
#if defined(LEDCONTROLSCROLL)
u16 gScroll = 0;
#endif

/* Send out a single command to the device */
//void LedControl_shiftOut(u8, u8, u8);
void LedControl_spiTransfer(u8, u8, u8);

/// public:

/* 
 * Create a new controler 
 * Params :
 * dataPin		pin on the Pinguino where data gets shifted out
 * clockPin		pin for the clock
 * csPin		pin for selecting the device 
 * numDevices	maximum number of devices that can be controled
 */

void LedControl_init(u8 module, ...);
//void LedControl_init(u8 dataPin, u8 clkPin, u8 csPin, u8 numDevices);

/* 
 * Set the shutdown (power saving) mode for the device
 * Params :
 * matrix	The address of the display to control
 * status	If true the device goes into power-down mode. Set to false
 *		for normal operation.
 */

#if defined(LEDCONTROLSHUTDOWN)
void LedControl_shutdown(u8);
#endif

/* 
 * Set the number of digits (or rows) to be displayed.
 * See datasheet for sideeffects of the scanlimit on the brightness
 * of the display.
 * Params :
 * matrix	address of the display to control
 * limit	number of digits to be displayed (1..8)
 */

#if defined(LEDCONTROLSETSCANLIMIT)
void LedControl_setScanLimit(u8);
#endif

/* 
 * Set the brightness of the display.
 * Params:
 * matrix		the address of the display to control
 * intensity	the brightness of the display. (0..15)
 */

#if defined(LEDCONTROLSETINTENSITY)
void LedControl_setIntensity(u8);
#endif

/* 
 * Switch all Leds on the display off. 
 * Params:
 * matrix	address of the display to control
 */

#if defined(LEDCONTROLCLEARDISPLAY) || defined(LEDCONTROLCLEARALL)
void LedControl_clearDisplay(u8);
#endif

#if defined(LEDCONTROLCLEARALL)
void LedControl_clearAll();
#endif

/* 
 * Set the status of a single Led.
 * Params :
 * matrix	address of the display 
 * row	the row of the Led (0..7)
 * col	the column of the Led (0..7)
 * state	If true the led is switched on, 
 *		if false it is switched off
 */

void LedControl_setLed(u8, u8, u8, boolean);

/* 
 * Set all 8 Led's in a row to a new state
 * Params:
 * matrix	address of the display
 * row	row which is to be set (0..7)
 * value	each bit set to 1 will light up the
 *		corresponding Led.
 */

void LedControl_setRow(u8, u8, u8);

/* 
 * Set all 8 Led's in a column to a new state
 * Params:
 * matrix	address of the display
 * col	column which is to be set (0..7)
 * value	each bit set to 1 will light up the
 *		corresponding Led.
 */

void LedControl_setColumn(u8, u8, u8);

/* 
 * Display a hexadecimal digit on a 7-Segment Display
 * Params:
 * matrix	address of the display
 * digit	the position of the digit on the display (0..7)
 * value	the value to be displayed. (0x00..0x0F)
 * dp	sets the decimal point.
 */

#if defined(LEDCONTROLSETDIGIT)
void LedControl_setDigit(u8, u8, u8, boolean);
#endif

/* 
 * Display a character on a 7-Segment display.
 * There are only a few characters that make sense here :
 *  '0','1','2','3','4','5','6','7','8','9','0',
 *  'A','b','c','d','E','F','H','L','P',
 *  '.','-','_',' ' 
 * Params:
 * matrix	address of the display
 * digit	the position of the character on the display (0..7)
 * value	the character to be displayed. 
 * dp	sets the decimal point.
 */

#if defined(LEDCONTROLSETCHAR)
void LedControl_setChar(u8, u8, char, boolean);
#endif

void LedControl_setFont(const u8*);
void LedControl_printChar(u8);
void LedControl_print(const char *);
#if defined(LEDCONTROLPRINTNUMBER)
void LedControl_printNumber(long, u8);
#endif
#if defined(LEDCONTROLPRINTFLOAT)
void LedControl_printFloat(float, u8);
#endif
#if defined(LEDCONTROLPRINTF)
void LedControl_printf(const u8 *, ...);
#endif
#if defined(LEDCONTROLSCROLL)
u16 LedControl_scroll(const char *);
#endif

#endif	//LEDCONTROL_H
