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
#include <spi.h>
#include <spi.c>
//#include <digitalw.c>       // digitalwrite
//#include <digitalp.c>       // pinmode
#include <delayms.c>

// Printf
#ifdef LEDCONTROLPRINTF
    #include <stdio.c>
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
    u8 m, sda, sck, cs;
    va_list args;
    
    LEDCONTROL_SPI = module;
    
    va_start(args, module);                     // points args after module

    // init SPI communication

    if (LEDCONTROL_SPI == SPISW)
    {
        sda = va_arg(args, u8);                 // get the next arg
        sck = va_arg(args, u8);                 // get the next arg
        cs  = va_arg(args, u8);                 // get the next arg
        SPI_setPin(LEDCONTROL_SPI, sda, sck, cs);
        SPI_setBitOrder(LEDCONTROL_SPI, SPI_MSBFIRST);
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

    gLastDevice = va_arg(args, u8) - 1;         // get the last arg
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
        LedControl_spiTransfer(m, OP_SHUTDOWN, 0);
    }

    va_end(args);                               // cleans up the list
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

void LedControl_setColumn(u8 matrix, u8 col, u8 value)
{
    u8 row;

    //if (matrix >= gLastDevice)
    //    return;

    for (row=0; row<8; row++)
        LedControl_setLed(matrix, row, col & 7, (value >> row) & 0x01);
}

void LedControl_setRow(u8 matrix, u8 row, u8 value)
{
    u8 offset;
    
    //if (matrix >= gLastDevice)
    //    return;

    offset = (matrix << 3) + (row & 7);
    status[offset] = value;
    LedControl_spiTransfer(matrix, row+1, value);
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
    defined(LEDCONTROLPRINTF)

void LedControl_printChar(u8 charIndex)
{
    u8 col;
    
    charIndex = charIndex & 0x7f - 0x20;

    // display char on the active matrix
    for (col=0; col<=7; col++)
        LedControl_setColumn(gActiveDevice, 7-col, font8x8[2+charIndex*8+col]);
        //LedControl_setColumn(gActiveDevice, 7-col, font[charIndex][col]);

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
    u8 sign;
    u8 length;

    s32 i;
    u32 v;                      // absolute value

    u8 tmp[12];
    u8 *tp = tmp;               // pointer on tmp

    u8 string[12];
    u8 *sp = string;            // pointer on string

    if (value == 0)
    {
        LedControl_printChar('0');
        return;
    }
    
    sign = ( (base == 10) && (value < 0) );

    if (sign)
        v = -value;
    else
        v = (u32)value;

    //while (v || tp == tmp)
    while (v)
    {
        i = v % base;
        v = v / base;
        
        if (i < 10)
            *tp++ = i + '0';
        else
            *tp++ = i + 'A' - 10;
    }

    // start of string
    if (sign)
        *sp++ = '-';

    length = sign + tp - tmp + 1;

    // backwards writing 
    while (tp > tmp)
        *sp++ = *--tp;

    // end of string
    *sp = 0;

    LedControl_print(string);
}
#endif

#if defined(LEDCONTROLPRINTFLOAT)
void LedControl_printFloat(float number, u8 digits)
{ 
    u8 i, toPrint;
    u16 int_part;
    float rounding, remainder;

    // Handle negative numbers
    if (number < 0.0)
    {
        LedControl_printChar('-');
        number = -number;
    }

    // Round correctly so that print(1.999, 2) prints as "2.00"  
    rounding = 0.5;
    for (i=0; i<digits; ++i)
        rounding /= 10.0;

    number += rounding;

    // Extract the integer part of the number and print it  
    int_part = (u16)number;
    remainder = number - (float)int_part;
    LedControl_printNumber(int_part, 10);

    // Print the decimal point, but only if there are digits beyond
    if (digits) // > 0)
        LedControl_printChar('.'); 

    // Extract digits from the remainder one at a time
    while (digits--) // > 0)
    {
        remainder *= 10.0;
        toPrint = (unsigned int)remainder; //Integer part without use of math.h lib, I think better! (Fazzi)
        LedControl_printNumber(toPrint, 10);
        remainder -= toPrint; 
    }
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
// d'après : http://breizhmakers.over-blog.com/article-un-peu-d-animation-ou-le-scrolling-a-base-de-max7219-105669349.html
#if defined(LEDCONTROLSCROLL)
void LedControl_scroll(const char * str)
{
    u8 m;                           // current matrix (0 < m < 8)
    u8 r;                           // current row    (0 < r < 8)
    u8 row[8];                      // new char to display
    u8 charIndex;                   // current char index in the font tab
    u8 curchar = gScroll / 8;       // current char to display (first matrix)
    u8 offset = gScroll & 7;        // pixel to scroll (0 < offset < 8)
    u8 len = strlen(str) - 1;       // number of char (from 0)
    u16 scrollmax = 8 * max(gLastDevice+1, len+1);
    
    // for every matrix connected
    for (m = 0; m <= gLastDevice; m++)
    {
        // for every line of the matrix
        for (r = 0; r < 8; r++)
        {
            if (curchar > len)
                charIndex = 0;
            else
                charIndex = str[curchar] & 0x7F - 0x20;
            // shift the current char by offset
            //row[r] = font[charIndex][r] >> offset;
            row[r] = font8x8[2+charIndex*8+r] >> offset;

            // add the next char shifted by (8 - offset)
            // only if offset is not null

            if (offset == 0) continue;
             
            if ((curchar+1) > len)
                charIndex = 0;
            else
                charIndex = str[curchar + 1] & 0x7F - 0x20;
            //row[r] |= font[charIndex][r] << (8-offset);
            row[r] |= font8x8[2+charIndex*8+r] << (8-offset);
        }

        // Display the new matrix
        for (r = 0; r < 8; r++)
            LedControl_setColumn(m, 7 - r, row[r]);     

        curchar++;
    }

    Delayms(5);
    
    // Do we cover the whole scroll area ?
    gScroll = (gScroll + 1) % scrollmax;
}
#endif

#endif /* LEDCONTROL_C */
