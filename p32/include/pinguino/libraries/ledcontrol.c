/*
 *    LedControl.cpp - A library for controling Leds with a MAX7219/MAX7221
 *    Copyright (c) 2007 Eberhard Fahle
 * 
 *    09 Feb. 2014 - Regis Blanchot - adapted to Pinguino 
 *    11 Feb. 2014 - Regis Blanchot - added scroll functions
 *    12 Jan. 2016 - Regis Blanchot - added poweron and poweroff functions
 *    12 Jan. 2016 - Regis Blanchot - added printNumber, printFloat and printf functions
 *    12 Jan. 2016 - Regis Blanchot - improved the code size
 *    12 Jan. 2016 - Regis Blanchot - fixed display bugs in scroll function
 *    13 Jan. 2016 - Regis Blanchot - added SPI library support
 *    13 Jan. 2016 - Regis Blanchot - added better cascadind devices management (using OP_NOOP)
 *    24 May  2016 - Regis Blanchot - scroll function returns actual position
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

#ifndef LEDCONTROL_C
#define LEDCONTROL_C

#include <typedef.h>
#include <const.h>
#include <macro.h>
#include <ledcontrol.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <spi.h>
#include <spi.c>

/*
#ifndef __PIC32MX__
#include <digitalw.c>       // digitalwrite
#include <digitalp.c>       // pinmode
#include <delayms.c>
#else
#include <digitalw.c>
#include <delay.c>
#endif
*/

// max
/*
#ifndef __PIC32MX__
#ifndef __XC8__
#if defined(LEDCONTROLSCROLL)
    #include <mathlib.c>
#endif
#endif
#endif
*/

// Printf
#ifdef LEDCONTROLPRINTF
    #include <printFormated.c>
#endif

// PrintFloat
#if defined(LEDCONTROLPRINTFLOAT)
    #include <printFloat.c>
#endif

// PrintNumber
#if defined(LEDCONTROLPRINTNUMBER) || defined(LEDCONTROLPRINTFLOAT)
    #include <printNumber.c>
#endif

void LedControl_spiTransfer(u8 matrix, u8 opcode, u8 data)
{
    u8 m;

    // Enable the line 
    SPI_select(LEDCONTROL_SPI);

    for (m=0; m<=gLastDevice; m++)
    {
        if (m == matrix)
        {
            SPI_write(LEDCONTROL_SPI, opcode);
            SPI_write(LEDCONTROL_SPI, data);
        }
        else
        {
            SPI_write(LEDCONTROL_SPI, OP_NOOP);
            SPI_write(LEDCONTROL_SPI, 0);
        }
    }

    // Latch the data onto the display
    SPI_deselect(LEDCONTROL_SPI);
}

/*  --------------------------------------------------------------------
    On initial power-up, all control registers are reset, the
    display is blanked, and the MAX7219/MAX7221 enter shutdown mode
    ------------------------------------------------------------------*/

void LedControl_init(u8 module, ...)
{
    u8 m;
    int sdo, sck, cs;
    va_list args;
    
    LEDCONTROL_SPI = module;
    
    va_start(args, module);                     // points args after module

    // init SPI communication

    if (LEDCONTROL_SPI == SPISW)
    {
        sdo = va_arg(args, int);                 // get the next arg
        sck = va_arg(args, int);                 // get the next arg
        cs  = va_arg(args, int);                 // get the next arg
        SPI_setBitOrder(LEDCONTROL_SPI, SPI_MSBFIRST);
        SPI_begin(LEDCONTROL_SPI, sdo, sck, cs);
    }
    else
    {
        SPI_setMode(LEDCONTROL_SPI, SPI_MASTER);
        SPI_setDataMode(LEDCONTROL_SPI, SPI_MODE2);
        // MAX72xx have theorical 10MHz maximum rate
        // but seems to work at max 4MHz (48MHz/16)
        SPI_setClockDivider(LEDCONTROL_SPI, SPI_CLOCK_DIV16);
        SPI_begin(LEDCONTROL_SPI);
    }

    gLastDevice = va_arg(args, int) - 1;         // get the last arg
    if (gLastDevice > 7)
        gLastDevice = 7;

    // Active Matrix is the first one
    gActiveDevice = 0;
    //gActiveDevice = gLastDevice - 1;

    // Reset the chained-matrix buffer
    for (m=0; m<64; m++) 
        status[m] = 0;
    
    // Init. all the matrix
    for (m=0; m<=gLastDevice; m++)
    {
        // Normal operation mode
        LedControl_spiTransfer(m, OP_DISPLAYTEST, 0);
        // scanlimit is set to max on startup
        LedControl_spiTransfer(m, OP_SCANLIMIT, 7);
        // No decode, it is done in source
        LedControl_spiTransfer(m, OP_DECODEMODE, 0);
        // Set medium intensity
        LedControl_spiTransfer(m, OP_INTENSITY, 8);
        // we go into shutdown-mode on startup
        //LedControl_spiTransfer(m, OP_SHUTDOWN, 0);
        // we start power on
        LedControl_spiTransfer(m, OP_SHUTDOWN, 1);
    }

    va_end(args);                               // cleans up the list

    #if defined(LEDCONTROLPRINTCHAR)   || defined(LEDCONTROLPRINT)      || \
        defined(LEDCONTROLPRINTNUMBER) || defined(LEDCONTROLPRINTFLOAT) || \
        defined(LEDCONTROLPRINTF)      || defined(LEDCONTROLSCROLL)
    LedControl_setFont(font8x8);
    #endif
}

//#define LedControl_getDeviceCount() (gLastDevice)
#if defined(LEDCONTROLSHUTDOWN) || \
    defined(LEDCONTROLPOWERON)  || defined(LEDCONTROLPOWEROFF)

void LedControl_shutdown(u8 b)
{
    u8 m;
    
    for (m=0; m<=gLastDevice; m++)
        LedControl_spiTransfer(m, OP_SHUTDOWN, b);
}

#define LedControl_poweron()  LedControl_shutdown(1)
#define LedControl_poweroff() LedControl_shutdown(0)
#endif

#if defined(LEDCONTROLSETSCANLIMIT)
void LedControl_setScanLimit(u8 limit)
{
    u8 m;

    for (m=0; m<=gLastDevice; m++)
        LedControl_spiTransfer(m, OP_SCANLIMIT, limit & 7);
}
#endif

#if defined(LEDCONTROLSETINTENSITY)
void LedControl_setIntensity(u8 intensity)
{
    u8 m;

    for (m=0; m<=gLastDevice; m++)
        LedControl_spiTransfer(m, OP_INTENSITY, intensity & 15);
}
#endif

#if defined(LEDCONTROLCLEARDISPLAY) || defined(LEDCONTROLCLEARALL)
void LedControl_clearDisplay(u8 matrix)
{
    u8 offset, i;
    
    //if (matrix >= gLastDevice)
    //    return;
        
    offset = matrix << 3; //* 8;
    for (i=0; i<8; i++)
    {
        status[offset + i] = 0;
        LedControl_spiTransfer(matrix, i+1, 0);
    }
}
#endif

#if defined(LEDCONTROLCLEARALL)
void LedControl_clearAll()
{
    u8 m;
    
    for (m=0; m<=gLastDevice; m++)
        LedControl_clearDisplay(m);

    gActiveDevice = 0;
}
#endif

void LedControl_setLed(u8 matrix, u8 row, u8 column, boolean state)
{
    u8 offset, val;

    //if (matrix >= gLastDevice)
    //    return;
    
    offset = (matrix << 3) + (row & 7);
    val = 0x80 >> (column & 7);

    if (state)
        status[offset] |= val;
    else
        status[offset] &= ~val;

    LedControl_spiTransfer(matrix, row+1, status[offset]);
}

void LedControl_setRow(u8 matrix, u8 row, u8 value)
{
    u8 col;

    //if (matrix >= gLastDevice)
    //    return;

    for (col=0; col<8; col++)
        LedControl_setLed(matrix, col, row & 7, (value >> col) & 0x01);
}

void LedControl_setColumn(u8 matrix, u8 col, u8 value)
{
    u8 offset;
    
    //if (matrix >= gLastDevice)
    //    return;

    offset = (matrix << 3) + (col & 7);
    status[offset] = value;
    LedControl_spiTransfer(matrix, col+1, value);
}

#if defined(LEDCONTROLSETDIGIT) || defined(LEDCONTROLSETCHAR)
void LedControl_setDigit(u8 matrix, u8 digit, u8 value, boolean dp)
{
    u8 offset, v;

    //if (matrix >= gLastDevice)
    //    return;

    if (digit > 7 || value > 15)
        return;

    offset = matrix << 3; //* 8;
    v = charTable[value];
    if (dp)
        v |= 0b10000000;
    status[offset+digit] = v;
    LedControl_spiTransfer(matrix, digit+1, v);
    
}
#define LedControl_setChar(a,b,c,d) LedControl_setDigit(a,b,c,d)
#endif

#if defined(LEDCONTROLPRINTCHAR)   || defined(LEDCONTROLPRINT)      || \
    defined(LEDCONTROLPRINTNUMBER) || defined(LEDCONTROLPRINTFLOAT) || \
    defined(LEDCONTROLPRINTF)      || defined(LEDCONTROLSCROLL)

void LedControl_setFont(const u8* font)
{
    font_address   = font;
    font_width     = font[FONT_WIDTH];
    font_height    = font[FONT_HEIGHT];
    font_firstchar = font[FONT_FIRST_CHAR];
    font_charcount = font[FONT_CHAR_COUNT];
}

void LedControl_printChar(u8 charIndex)
{
    u8 col;
    
    //charIndex = charIndex & 0x7f - 0x20;
    charIndex = charIndex & font_charcount - font_firstchar;

    // display char on the active matrix
    for (col=0; col<font_width; col++)
        LedControl_setRow(gActiveDevice, 7-col, font_address[FONT_OFFSET+charIndex*font_width+col]);
        //LedControl_setColumn(gActiveDevice, col, font_address[FONT_OFFSET+charIndex*font_width+col]);
        //LedControl_setColumn(gActiveDevice, 7-col, font_address[FONT_OFFSET+charIndex*8+col]);

    // point to the next matrix
    if (gActiveDevice++ >= gLastDevice)
        gActiveDevice = 0;

    //Delayms(5);
}
#endif

#if defined(LEDCONTROLPRINT) || defined(LEDCONTROLPRINTNUMBER)
void LedControl_print(const char * str)
{
    while (*str)
        LedControl_printChar(*str++);

    //Delayms(5);
}
#endif

#if defined(LEDCONTROLPRINTNUMBER) || defined(LEDCONTROLPRINTFLOAT)
void LedControl_printNumber(s32 value, u8 base)
{  
    printNumber(LedControl_printChar, value, base);
}
#endif

#if defined(LEDCONTROLPRINTFLOAT)
void LedControl_printFloat(float number, u8 digits)
{ 
    printFloat(LedControl_printChar, number, digits);
}
#endif

#if defined(LEDCONTROLPRINTF)
void LedControl_printf(const u8 *fmt, ...)
{
    static u8 buffer[25];
    u8 *str=(char*)&buffer;
    va_list	args;

    va_start(args, fmt);
    psprintf2(buffer, fmt, args);
    va_end(args);

    while (*str)
        LedControl_printChar(*str++);

    //Delayms(5);
}
#endif

// Scroll Message
// from : http://breizhmakers.over-blog.com/article-un-peu-d-animation-ou-le-scrolling-a-base-de-max7219-105669349.html
#if defined(LEDCONTROLSCROLL)
u16 LedControl_scroll(const char * string)
{
    u8 m;                           // current matrix (0 < m < 8)
    u8 r;                           // current row    (0 < r < 8)
    u8 row[8];                      // new char to display
    u8 charIndex;                   // current char index in the font tab
    u8 curchar = gScroll / 8;       // current char to display (first matrix)
    u8 offset = gScroll & 7;        // pixel to scroll (0 < offset < 8)
    //u8 len = strlen(str) - 1;       // number of char (from 0)
    u8 len;
    u8 str[64];
    u16 scrollmax = 8 * max(gLastDevice+1, len+1);

    // fill the string with leading spaces, one for each matrix
    for (m = 0; m <= gLastDevice; m++)
        str[m] = ' ';
    str[m] = '\0';
    strcat(str, string);
    len = strlen(str) - 1;

    // for every matrix connected
    for (m = 0; m <= gLastDevice; m++)
    {
        // for every line of the matrix
        for (r = 0; r < 8; r++)
        {
            if (curchar > len)
                charIndex = 0;
            else
                //charIndex = str[curchar] & 0x7F - 0x20;
                charIndex = str[curchar] & font_charcount - font_firstchar;
            // shift the current char by offset
            //row[r] = font[charIndex][r] >> offset;
            row[r] = font_address[FONT_OFFSET+charIndex*8+r] >> offset;

            // add the next char shifted by (8 - offset)
            // only if offset is not null

            if (offset == 0) continue;
             
            if ((curchar+1) > len)
                charIndex = 0;
            else
                charIndex = str[curchar + 1] & font_charcount - font_firstchar;
            //row[r] |= font[charIndex][r] << (8-offset);
            row[r] |= font_address[FONT_OFFSET+charIndex*8+r] << (8-offset);
        }

        // Display the new matrix
        for (r = 0; r < 8; r++)
            LedControl_setRow(m, 7 - r, row[r]);     

        curchar++;
    }

    //Delayms(5);
    
    // Do we cover the whole scroll area ?
    gScroll = (gScroll + 1) % scrollmax;
    return gScroll;
}
#endif

#endif /* LEDCONTROL_C */
