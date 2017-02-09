/*  --------------------------------------------------------------------
    FILE:           ks0108.c
    PROJECT:        Pinguino32
    PURPOSE:        Pinguino's Graphic LCD functions
    PROGRAMER:      Fabian Maximilian Thiele (me@apetech.de)
    FIRST RELEASE:  ???
    --------------------------------------------------------------------
    CHANGELOG:
    * 20??-??-??    Marcus Fazzi (anunakin@ieee.org) - Pinguino 32 pPort
    * 2016-10-17    Régis Blanchot - Added use of Print libraries
    * 2016-11-24    Régis Blanchot - Complete re-write
    --------------------------------------------------------------------
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
    ------------------------------------------------------------------*/

#ifndef KS0108_C
#define KS0108_C

#include <typedef.h>                    // u8, u16, ...
#include <stdarg.h>
#include <const.h>                      // false, true, ...
#include <macro.h>                      // BitSet, BitClear, ...
#include <ks0108.h>
#include <digitalw.c>                   // digitalwrite

#ifndef __PIC32MX__
#ifndef KS0108_FAST
#include <digitalr.c>                   // digitalread
#endif
#include <digitalp.c>                   // pinmode
#endif

#ifdef KS0108_DEBUG
#define SERIALPRINTF
#include <serial.c>
#endif

// Printf
#if defined(KS0108PRINTF)
    #include <printFormated.c>
#endif

// PrintFloat
#if defined(KS0108PRINTFLOAT)
    #include <printFloat.c>
#endif

// PrintNumber
#if defined(KS0108PRINTNUMBER) || defined(KS0108PRINTFLOAT)
    #include <printNumber.c>
#endif

// Graphics
#ifdef KS0108GRAPHICS
#include <graphics.c>                   // graphic routines
#endif

///	--------------------------------------------------------------------
/// Core functions
///	--------------------------------------------------------------------

#define GLCD_high(pin)      digitalwrite(pin, 1)
#define GLCD_low(pin)       digitalwrite(pin, 0)
#define GLCD_delay()        Nop()
#define GLCD_selectChip1()  {GLCD_low(KS0108.pin.cs2); GLCD_high(KS0108.pin.cs1);}
#define GLCD_selectChip2()  {GLCD_low(KS0108.pin.cs1); GLCD_high(KS0108.pin.cs2);}
#define GLCD_home()         GLCD_goto(0,0)

u8 gStartLine;

/*  --------------------------------------------------------------------
    GLCD_enable
    ------------------------------------------------------------------*/

void GLCD_enable()
{
    GLCD_high(KS0108.pin.en);
    GLCD_delay();                       // min. 450ns
    GLCD_low(KS0108.pin.en);
}

/*  --------------------------------------------------------------------
    GLCD_dataIn
    ------------------------------------------------------------------*/

void GLCD_dataIn()
{
    #ifdef KS0108_FAST
    KS0108_TRIS = 0xFF;
    #else
    u8 i;
    for (i=0; i<8; i++)
        pinmode(KS0108.pin.d[i], INPUT);
    #endif
}

/*  --------------------------------------------------------------------
    GLCD_dataOut
    ------------------------------------------------------------------*/

void GLCD_dataOut()
{
    #ifdef KS0108_FAST
    KS0108_TRIS = 0x00;
    #else
    u8 i;
    for (i=0; i<8; i++)
        pinmode(KS0108.pin.d[i], OUTPUT);
    #endif
}

/*  --------------------------------------------------------------------
    GLCD_send
    --------------------------------------------------------------------
    Send a byte on data bus.
    ------------------------------------------------------------------*/

void GLCD_send(u8 d)
{
    if (KS0108.screen.invert)
        d = ~d;
    #ifdef KS0108_FAST
    KS0108_LAT = d;
    #else
    u8 i;
    for (i=0; i<8; i++)
        digitalwrite(KS0108.pin.d[i], (d >> i) & 0x01);
    #endif
    GLCD_enable();
}

/*  --------------------------------------------------------------------
    GLCD_get
    --------------------------------------------------------------------
    Get a byte from the data bus.
    ------------------------------------------------------------------*/

u8 GLCD_get()
{
    u8 i,r=0;
    
    GLCD_high(KS0108.pin.en);
    GLCD_delay();
    #ifdef KS0108_FAST
    r = KS0108_PORT;
    #else
    for (i=0; i<8; i++)
        r = r | (digitalread(KS0108.pin.d[i]) << i);
    #endif
    GLCD_low(KS0108.pin.en);
    return r;
}

/*  --------------------------------------------------------------------
    GLCD_selectChip
    --------------------------------------------------------------------
    Select the right chip depending on x value:
    Right : 000 < x < 063 : chip 1
    Left  : 064 < x < 127 : chip 2
    ------------------------------------------------------------------*/

void GLCD_selectChip(u8 x)
{
    switch (x >> 6)                 // x/64
    {
        case 0: GLCD_selectChip1(); break;
        case 1: GLCD_selectChip2(); break;
    }
    /*
    GLCD_dataOut();
    GLCD_low(KS0108.pin.rw);        // Write
    GLCD_low(KS0108.pin.rs);        // Command
    GLCD_send(KS0108_SET_ADD|(x%64));
    */
}

/*  --------------------------------------------------------------------
    GLCD_displayOn/Off
    --------------------------------------------------------------------
    This function turns the display on.
    This can be done by sending the command 3Fh to both the controllers.
    So, while sending this command, both CS1 and CS2 must be pulled low.
    Similarly the RS pin should be low too as the byte sent is an instruction.
    ------------------------------------------------------------------*/

void GLCD_displayOn()
{
    GLCD_dataOut();
    GLCD_low(KS0108.pin.rw);            // Write
    GLCD_low(KS0108.pin.rs);            // Command

    GLCD_selectChip1();                 // Select chip 1
    GLCD_send(KS0108_ON);

    GLCD_selectChip2();                 // Select chip 2
    GLCD_send(KS0108_ON);
}

void GLCD_displayOff()
{
    GLCD_dataOut();
    GLCD_low(KS0108.pin.rw);            // Write
    GLCD_low(KS0108.pin.rs);            // Command

    GLCD_selectChip1();                 // Select chip 1
    GLCD_send(KS0108_OFF);

    GLCD_selectChip2();                 // Select chip 2
    GLCD_send(KS0108_OFF);
}

/*  --------------------------------------------------------------------
    GLCD_setStartLine
    --------------------------------------------------------------------
    This function changes the line number to be displayed at the top of
    the screen. You can set it to be any number between 0 to 63. It does
    not affect the data in the display RAM, it just scrolls the display
    up and down.
    ------------------------------------------------------------------*/
/*
void GLCD_setStartLine(u8 line)
{
    if (line < KS0108.screen.height)
    {
        GLCD_dataOut();
        GLCD_low(KS0108.pin.rw);        // Write
        GLCD_low(KS0108.pin.rs);        // Command

        GLCD_selectChip1();             // Select chip 1
        GLCD_send(KS0108_START_LINE | line);
        GLCD_selectChip2();             // Select chip 2
        GLCD_send(KS0108_START_LINE | line);
    }
}
*/
/*  --------------------------------------------------------------------
    GLCD_goto
    --------------------------------------------------------------------
    Moves the cursor to specified row and column
    0 <= x < 128
    0 <= y < 64
    ------------------------------------------------------------------*/

void GLCD_set(u8 x, u8 line)
{
    GLCD_dataOut();
    GLCD_low(KS0108.pin.rw);            // Write
    GLCD_low(KS0108.pin.rs);            // Commands

    GLCD_selectChip(x);                 // Select Chip
    GLCD_send(KS0108_SET_PAGE|line);
    GLCD_send(KS0108_SET_ADD |(x%64));
}

void GLCD_goto(u8 x, u8 y)
{
    if (x < KS0108.screen.width && y < KS0108.screen.height)
    {
        KS0108.pixel.x = x;
        KS0108.pixel.y = y;
        GLCD_set(x, y >> 3);
    }
}

/*  --------------------------------------------------------------------
    GLCD_readStatus()
    --------------------------------------------------------------------
    Read Status from specified controller.
    ------------------------------------------------------------------*/
/*
u8 GLCD_readStatus()
{
    GLCD_dataIn();
    GLCD_high(KS0108.pin.rw);           // Read
    GLCD_low(KS0108.pin.rs);            // Command
    GLCD_selectChip(KS0108.pixel.x);    // Select Chip
    return GLCD_get();                  // Get the actual status
}
*/
/*  --------------------------------------------------------------------
    GLCD_readData()
    --------------------------------------------------------------------
    Returns a byte read from the current display location.
    The first read operation is a dummy read during which the data is
    fetched from the display RAM is latched in to the output register
    of KS0108B.
    In the second read, the microcontroller can get the actual data.
    ------------------------------------------------------------------*/
    
u8 GLCD_readData(u8 x, u8 line)
{
    GLCD_dataOut();
    GLCD_low(KS0108.pin.rw);            // Write
    GLCD_low(KS0108.pin.rs);            // Commands

    GLCD_selectChip(x);                 // Select Chip
    GLCD_send(KS0108_SET_PAGE|line);
    GLCD_send(KS0108_SET_ADD |(x%64));
    
    GLCD_dataIn();
    GLCD_high(KS0108.pin.rw);           // Read
    GLCD_high(KS0108.pin.rs);           // Data
    GLCD_selectChip(x);                 // Select Chip
    GLCD_get();                         // Dummy read
    GLCD_selectChip(x);                 // Select Chip
    return GLCD_get();                  // Get the actual data
}

/*  --------------------------------------------------------------------
    GLCD_writeCommand()
    --------------------------------------------------------------------
    Writes a byte of command
    ------------------------------------------------------------------*/

void GLCD_writeCommand(u8 b)
{
    GLCD_selectChip(KS0108.pixel.x);    // Select Chip
    GLCD_dataOut();
    GLCD_low(KS0108.pin.rw);            // Write
    GLCD_low(KS0108.pin.rs);            // Command
    GLCD_send(b);
}

/*  --------------------------------------------------------------------
    GLCD_writeData()
    --------------------------------------------------------------------
    Writes a byte of data to the current location
    After operation x is increased by 1 automatically
    ------------------------------------------------------------------*/

void GLCD_write(u8 b, u8 x, u8 line)
{
    GLCD_dataOut();

    GLCD_low(KS0108.pin.rw);            // Write
    GLCD_low(KS0108.pin.rs);            // Commands

    GLCD_selectChip(x);                 // Select Chip
    GLCD_send(KS0108_SET_PAGE|line);
    GLCD_send(KS0108_SET_ADD |(x%64));

    GLCD_low(KS0108.pin.rw);            // Write
    GLCD_high(KS0108.pin.rs);           // Data

    GLCD_selectChip(x);                 // Select Chip
    GLCD_send(b);
}

void GLCD_writeData(u8 b)
{
    u8 dat;
    u8 x = KS0108.pixel.x;
    u8 line = KS0108.pixel.y >> 3;
    u8 yOffset = KS0108.pixel.y % 8;

    if (yOffset == 0)                   // No offset,
    {                                   // The byte is on 1 line
        GLCD_write(b, x, line);
        KS0108.pixel.x++;
    }
    else                                // Offset is positive
    {                                   // The byte is on 2 lines
        // first line
        dat = GLCD_readData(x, line);
        GLCD_write(dat | (b << yOffset), x, line);

        // second line
        dat = GLCD_readData(x, line + 1);
        GLCD_write(dat | (b >> (8 - yOffset)), x, line + 1);

        // Back to the first line, one byte further
        //GLCD_set(x + 1, line);
        KS0108.pixel.x++;
    }
}

/*  --------------------------------------------------------------------
    GLCD_clearPage
    --------------------------------------------------------------------
    Clears 1 line
    0 <= line < 8
    ------------------------------------------------------------------*/

void GLCD_clearline(u8 line)
{
    u8 x;

    GLCD_dataOut();
    GLCD_low(KS0108.pin.rw);            // Write
    GLCD_high(KS0108.pin.cs1);          // Select chip 1
    GLCD_high(KS0108.pin.cs2);          // Select chip 2

    GLCD_low(KS0108.pin.rs);            // Command
    GLCD_send(KS0108_SET_PAGE|line);    // Select page
        
    GLCD_high(KS0108.pin.rs);           // Data
    for (x = 0; x < 64; x++)            // Clear all the 64 pixels
        GLCD_send(KS0108.screen.bcolor);
}

/*  --------------------------------------------------------------------
    GLCD_clearScreen
    --------------------------------------------------------------------
    Clears the whole screen (all 8 pages)
    ------------------------------------------------------------------*/

void GLCD_clearScreen()
{
    u8 x, y;

    GLCD_dataOut();
    GLCD_low(KS0108.pin.rw);            // Write
    GLCD_high(KS0108.pin.cs1);          // Select chip 1
    GLCD_high(KS0108.pin.cs2);          // Select chip 2

    // Loop through all the 8 lines
    for (y = 0; y < 8; y++)
    {
        GLCD_low(KS0108.pin.rs);        // Command
        GLCD_send(KS0108_SET_PAGE|y);   // Select Line
        
        GLCD_high(KS0108.pin.rs);       // Data
        for (x = 0; x < 64; x++)        // Clear all the 64 pixels
            GLCD_send(KS0108.screen.bcolor);
    }

    //GLCD_goto(0, 0);
    GLCD_low(KS0108.pin.rw);            // Write
    GLCD_low(KS0108.pin.rs);            // Commands
    GLCD_high(KS0108.pin.cs1);          // Select chip 1
    GLCD_high(KS0108.pin.cs2);          // Select chip 2
    GLCD_send(KS0108_SET_PAGE|0);
    GLCD_send(KS0108_SET_ADD|0);
    KS0108.pixel.y = 0;
    KS0108.pixel.x = 0;
    
}

/*  --------------------------------------------------------------------
    GLCD_setColor
    --------------------------------------------------------------------
    Sets current color
    ------------------------------------------------------------------*/

#define GLCD_setColor(co)               KS0108.screen.color = co

/*  --------------------------------------------------------------------
    GLCD_setBackgroundColor
    --------------------------------------------------------------------
    Sets background color
    ------------------------------------------------------------------*/

#define GLCD_setBackgroundColor(co)     KS0108.screen.bcolor = co;

/*  --------------------------------------------------------------------
    GLCD_init
    --------------------------------------------------------------------
    Start up the display
    ------------------------------------------------------------------*/

void GLCD_init(u8 rs, u8 rw, u8 en, u8 cs1, u8 cs2, u8 rst,
               u8 d0, u8 d1, u8 d2, u8 d3,  u8 d4,  u8 d5, u8 d6, u8 d7)
{
    KS0108.pin.rs = rs;     GLCD_high(KS0108.pin.rs);   pinmode(rs, OUTPUT);
    KS0108.pin.rw = rw;     GLCD_high(KS0108.pin.rw);   pinmode(rw, OUTPUT);
    KS0108.pin.en = en;     GLCD_high(KS0108.pin.en);   pinmode(en, OUTPUT);
    
    KS0108.pin.cs1 = cs1;   GLCD_high(KS0108.pin.cs1);  pinmode(cs1, OUTPUT);
    KS0108.pin.cs2 = cs2;   GLCD_high(KS0108.pin.cs2);  pinmode(cs2, OUTPUT);
    KS0108.pin.rst = rst;   GLCD_high(KS0108.pin.rst);  pinmode(rst, OUTPUT);

    #ifndef KS0108_FAST
    KS0108.pin.d[0] = d0;
    KS0108.pin.d[1] = d1;
    KS0108.pin.d[2] = d2;
    KS0108.pin.d[3] = d3;
    KS0108.pin.d[4] = d4;
    KS0108.pin.d[5] = d5;
    KS0108.pin.d[6] = d6;
    KS0108.pin.d[7] = d7;
    #endif
    
    GLCD_dataOut();
    
    // GFX properties
    KS0108.screen.width  = KS0108_DISPLAY_WIDTH - 1;
    KS0108.screen.height = KS0108_DISPLAY_HEIGHT - 1;
    KS0108.screen.color  = KS0108_WHITE;
    KS0108.screen.bcolor = KS0108_BLACK;
    KS0108.screen.invert = 0;
    KS0108.pixel.y       = 0;
    KS0108.pixel.x       = 0;

    // Reset the LCD
    GLCD_low(KS0108.pin.rst);
    GLCD_delay();
    GLCD_high(KS0108.pin.rst);
    
    // power on
    GLCD_displayOn();

    // clear display
    GLCD_clearScreen();

    #ifdef KS0108_DEBUG
    Serial_begin(9600);
    #endif
}

#ifdef KS0108INVERTDISPLAY
void GLCD_invertDisplay()
{
    KS0108.screen.color  == KS0108_BLACK;
    KS0108.screen.bcolor == KS0108_WHITE;
    KS0108.screen.invert = 1;
    GLCD_clearScreen();
}
#endif

#ifdef KS0108NORMALDISPLAY
void GLCD_normalDisplay()
{
    KS0108.screen.color  == KS0108_WHITE;
    KS0108.screen.bcolor == KS0108_BLACK;
    KS0108.screen.invert = 0;
    GLCD_clearScreen();
}
#endif

#ifdef KS0108SETFONT
///	--------------------------------------------------------------------
/// Print functions
///	--------------------------------------------------------------------

void GLCD_setFont(const u8* font)
{
    KS0108.font.address   = font;
    //KS0108.font.size      = font[FONT_LENGTH] << 8 + font[FONT_LENGTH+1];
    KS0108.font.width     = font[FONT_WIDTH];
    KS0108.font.height    = font[FONT_HEIGHT];
    KS0108.font.firstChar = font[FONT_FIRST_CHAR];
    KS0108.font.charCount = font[FONT_CHAR_COUNT];
}

#if defined(KS0108PRINTCHAR)   || defined(KS0108PRINT)      || \
    defined(KS0108PRINTNUMBER) || defined(KS0108PRINTFLOAT) || \
    defined(KS0108PRINTLN)     || defined(KS0108PRINTF)     || \
    defined(KS0108PRINTCENTER)

void GLCD_setCursor(u8 x, u8 y)
{ 
    GLCD_goto(x * (KS0108.font.width+1), y * (KS0108.font.height+1));
}

/*  --------------------------------------------------------------------
    GLCD_scrollUp
    --------------------------------------------------------------------
    Scroll up 1 character row (8 pixel lines)   
    ------------------------------------------------------------------*/

void GLCD_scrollUp()
{   
    u8 x, line, byte, d;

    // Move up the 8 lines, each line n+1 is copied in line n
    for (byte=0; byte<((KS0108.font.height + 7) / 8); byte++)
    {
        for(line=0; line<7; line++)
        {
            for (x=0; x<128; x++)
            {
                d = GLCD_readData(x, line + 1);
                GLCD_write(d, x, line);
            }
        }
    }

    // Clear the bottom line
    //for (line=0; line<bytes; line++)
    //    GLCD_clearline(7-line);
    GLCD_clearline(7);

    // Go back to the last line
    GLCD_goto(0, KS0108.pixel.y - (KS0108.font.height + 1));
}

/*  --------------------------------------------------------------------
    GLCD_printChar
    --------------------------------------------------------------------
    Print a character on screen
    ------------------------------------------------------------------*/

void GLCD_printChar(u8 c)
{
    u8  x, y;
    u8  i, j, page;
    u8  dat, tab;
    u8  width = 0;
    u8  bytes = (KS0108.font.height + 7) / 8;
    u16 index = 0;

    switch (c)
    {
        case '\n':
            #ifdef KS0108_DEBUG
            Serial_printf("\n");
            #endif
            KS0108.pixel.y += KS0108.font.height + 1;
            break;

        case '\r':
            #ifdef KS0108_DEBUG
            Serial_printf("\r");
            #endif
            KS0108.pixel.x = 0;
            break;

        case '\t':
            tab = KS0108_TABSIZE * KS0108.font.width;
            KS0108.pixel.x += (KS0108.pixel.x + tab) % tab;
            break;

        default:

            if (c < KS0108.font.firstChar || c >= (KS0108.font.firstChar + KS0108.font.charCount))
                c = ' ';
            
            #ifdef KS0108_DEBUG
            Serial_printf("%c", c);
            #endif
            c = c - KS0108.font.firstChar;

            // fixed width font
            if (KS0108.font.address[FONT_LENGTH]==0 && KS0108.font.address[FONT_LENGTH+1]==0)
            {
                width = KS0108.font.width; 
                index = FONT_OFFSET + c * bytes * width;
            }

            // variable width font
            else
            {
                width = KS0108.font.address[FONT_WIDTH_TABLE + c];
                for (i=0; i<c; i++)
                    index += KS0108.font.address[FONT_WIDTH_TABLE + i];
                index = FONT_WIDTH_TABLE + KS0108.font.charCount + index * bytes;
            }

            // save the coordinates
            x = KS0108.pixel.x;
            y = KS0108.pixel.y;

            // draw the character
            #ifdef KS0108_DEBUG
            Serial_printf("\tx,y=%d,%d\r\n", KS0108.pixel.x, KS0108.pixel.y);
            #endif
            for (i=0; i<bytes; i++)
            {
                page = i * width;
                for (j=0; j<width; j++)
                {
                    dat = KS0108.font.address[index + page + j];
                    // if char. takes more than 1 line
                    if (KS0108.font.height > 8)
                        if (KS0108.font.height < ((i+1)*8))
                            dat >>= (i+1) * 8 - KS0108.font.height;

                    GLCD_writeData(dat);
                }
                
                // 1px gap between chars
                GLCD_writeData(0);

                // Next part of the current char will be at :
                // - the same position on the x-axis
                // - one line under
                GLCD_goto(x, y + 8);
            }
            // Next char will be at :
            // - last char pos + last char width + 1px gap
            // - on the same line
            GLCD_goto(x + width + 1, y);
            return;
    }

    if ((KS0108.pixel.x + KS0108.font.width) > KS0108.screen.width)
    {
        KS0108.pixel.x = 0;
        KS0108.pixel.y += KS0108.font.height + 1;
    }

    if ((KS0108.pixel.y + KS0108.font.height) > KS0108.screen.height)
    {
        GLCD_scrollUp();
        return;
    }

    GLCD_goto(KS0108.pixel.x, KS0108.pixel.y);
}
#endif

#if defined(KS0108PRINT)       || defined(KS0108PRINTLN)    || \
    defined(KS0108PRINTNUMBER) || defined(KS0108PRINTFLOAT) || \
    defined(KS0108PRINTCENTER)
    
void GLCD_print(const u8 *string)
{
    while (*string != 0)
        GLCD_printChar(*string++);
}

#endif

#if defined(KS0108PRINTLN)

void GLCD_println(const u8 *string)
{
    GLCD_print(string);
    GLCD_print((const u8*)"\n\r");
}

#endif

#if defined(KS0108PRINTCENTER)

void GLCD_printCenter(const u8 *string)
{
    u8 x = (KS0108.screen.width - GLCD_stringWidth(string)) / 2;
    
    // write string
    GLCD_goto(x, KS0108.pixel.y);
    GLCD_print(string);
}

#endif

#if defined(KS0108PRINTF)

void GLCD_printf(const u8 *fmt, ...)
{
    static u8 buffer[25];
    u8 *c=(char*)&buffer;
    va_list	args;

    va_start(args, fmt);
    psprintf2(buffer, fmt, args);
    va_end(args);

    while (*c)
        GLCD_printChar(*c++);
}

#endif

// Print a integer number on LCD
#if defined(KS0108PRINTNUMBER) || defined(KS0108PRINTFLOAT)

void GLCD_printNumber(long n, u8 base)
{
    printNumber(GLCD_printChar, n, base);
}

#endif

// Print a float number to LCD
#if defined(KS0108PRINTFLOAT)

void GLCD_printFloat(float number, u8 digits)
{ 
    printFloat(GLCD_printChar, number, digits);
}

#endif

u8 GLCD_charWidth(u8 c)
{
    // fixed width font
    //if (KS0108.font.size == 0)
    if (KS0108.font.address[FONT_LENGTH]==0 && KS0108.font.address[FONT_LENGTH+1]==0)
        return (KS0108.font.width + 1); 

    // variable width font
    if (c < KS0108.font.firstChar || c > (KS0108.font.firstChar + KS0108.font.charCount))
        c = ' ';
    c = c - KS0108.font.firstChar;
    return (KS0108.font.address[FONT_WIDTH_TABLE + c] + 1);
}

u16 GLCD_stringWidth(u8* str)
{
    u16 width = 0;

    while(*str != 0)
        width += GLCD_charWidth(*str++);

    return width;
}

#endif // KS0108SETFONT

///	--------------------------------------------------------------------
/// Graphic functions
///	--------------------------------------------------------------------

#ifdef KS0108GRAPHICS

//draw Pixel on the buffer
void GLCD_drawPixel(u8 x, u8 y)
{
    u8 d, line;

    if (x > KS0108.screen.width || y > KS0108.screen.height)
        return;

    line = y >> 3;
    
    // Read data from display memory
    d = GLCD_readData(x, line); 
    
    // Set or clear dot
    if (KS0108.screen.color == KS0108_WHITE)
        d |=  (1 << (y % 8));
    else
        d &= ~(1 << (y % 8));

    // Write data back to display
    GLCD_write(d, x, line);
}

// defined as extern void drawPixel(u16 x, u16 y); in graphics.c
void drawPixel(u16 x, u16 y)
{
    GLCD_drawPixel((u8)x, (u8)y);
}

void GLCD_drawLine(u8 x1, u8 y1, u8 x2, u8 y2)
{
    drawLine(x1, y1, x2, y2);
}

void GLCD_drawRect(u8 x, u8 y, u8 width, u8 height)
{
    drawRect(x, y, width, height);
}

void GLCD_drawRoundRect(u8 x1, u8 y1, u8 x2, u8 y2)
{
    drawRoundRect(x1, y1, x2, y2);
}

void GLCD_fillRect(u8 x, u8 y, u8 width, u8 height)
{
    fillRect(x, y, width, height);
}

void GLCD_fillRoundRect(u16 x1, u16 y1, u16 x2, u16 y2)
{
    fillRoundRect(x1, y1, x2, y2);
}

void GLCD_drawCircle(u8 x, u8 y, u8 radius)
{
    drawCircle(x, y, radius);
}

void GLCD_fillCircle(u8 x, u8 y, u8 radius)
{
    fillCircle(x, y, radius);
}

/** Loads a Bitmap to GLCD */

#ifdef _KS0108_USE_BITMAP

void GLCD_drawBitmap(const u8 * bitmap, u8 x, u8 y)
{
    u8 width, height;
    u8 i, j;
    u8 displayData;

    width = *(bitmap++); 
    height = *(bitmap++);
    for(j = 0; j < height / 8; j++)
    {
        GLCD_goto(x, y + (j*8) );
        for(i = 0; i < width; i++)
        {
            displayData = *(bitmap++);
            GLCD_writeData(displayData);
        }
    }
}

#endif /* _KS0108_USE_BITMAP */

#endif /* KS0108GRAPHICS */

#endif /* KS0108_C */
