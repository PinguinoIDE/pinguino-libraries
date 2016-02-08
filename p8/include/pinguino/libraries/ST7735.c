/*	--------------------------------------------------------------------
    FILE:			ST7735.c
    PROJECT:		pinguino
    PURPOSE:		Drive 1.8" 128x160 TFT display
    PROGRAMER:		regis blanchot <rblanchot@gmail.com>
    FIRST RELEASE:	11 Dec. 2014
    LAST RELEASE:	29 Jan. 2016
    ------------------------------------------------------------------------
    http://w8bh.net/pi/TFT1.pdf to TFT5.pdf
    ------------------------------------------------------------------------
    CHANGELOG
    * 01 Oct. 2015  RB  fixed ST7735_setOrientation()
    * 03 Oct. 2015  RB  added new function ST7735_printCenter()
    * 27 Jan. 2016  RB  replaced ST7735_WIDTH and ST7735_HEIGHT with
                        ST7735[module].screen.width and ST7735[module].screen.height
    ------------------------------------------------------------------------
    TODO
    * scroll functions
    ------------------------------------------------------------------------
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
	------------------------------------------------------------------*/

/**	--------------------------------------------------------------------
    Pin Assignment

            ST7735             PIC32MX     32MX250
    1       LED                A1
    2       SCK                SCK
    3       SDA                SDO
    4       A0                 DC
    5       RESET              VSS         VSS
    6       CS                 SS          SS
    7       GND                GND         GND
    8       VCC                VCC         VCC
    ------------------------------------------------------------------*/

#ifndef __ST7735_C
#define __ST7735_C

#include <compiler.h>
#include <typedef.h>
#include <macro.h>
#include <stdarg.h>
#include <spi.h>
#include <ST7735.h>
#include <digitalw.c>
#include <digitalp.c>
#include <delayms.c>
#include <spi.c>

// Printf
#ifdef ST7735PRINTF
    #include <printFormated.c>
#endif

// PrintFloat
#if defined(ST7735PRINTFLOAT)
    #include <printFloat.c>
#endif

// PrintNumber
#if defined(ST7735PRINTNUMBER) || defined(ST7735PRINTFLOAT)
    #include <printNumber.c>
#endif

// Graphics Library
#if defined(ST7735GRAPHICS) || defined(ST7735DRAWBITMAP)
    #ifdef ST7735DRAWBITMAP
    #define DRAWBITMAP
    #endif
    #include <graphics.c>
#endif

///	--------------------------------------------------------------------
/// Core functions
///	--------------------------------------------------------------------

void ST7735_sendCommand(u8 module, u8 val)
{
    ST7735_low(ST7735[module].pin.dc);            // COMMAND = 0
    SPI_write(module, val);
}

void ST7735_sendData(u8 module, u8 val)
{
    ST7735_high(ST7735[module].pin.dc);           // DATA = 1
    SPI_write(module, val);
}

/*  --------------------------------------------------------------------
    DESCRIPTION:
        Initialize the graphical display
    PARAMETERS:
    RETURNS:
    REMARKS:
        Orientation sets to portrait by default
    ------------------------------------------------------------------*/

//void ST7735_init(u8 module, u8 cs, u8 dc, u8 sda, u8 sck)
void ST7735_init(u8 module, ...)
{
    u8 sda, sck, cs;
    va_list args;
    
    ST7735_SPI = module;
    
    va_start(args, module); // args points on the argument after module

    ST7735[module].pin.dc = va_arg(args, u8); // get the first arg
    pinmode(ST7735[module].pin.dc, OUTPUT);
    
    // init SPI communication

    if (module == SPISW)
    {
        sda = va_arg(args, u8);         // get the next arg
        sck = va_arg(args, u8);         // get the next arg
        cs  = va_arg(args, u8);         // get the last arg
        SPI_setBitOrder(module, SPI_MSBFIRST);
        SPI_begin(module, sda, sck, cs);
    }
    else
    {
        SPI_setMode(ST7735_SPI, SPI_MASTER);
        SPI_setDataMode(ST7735_SPI, SPI_MODE1);
        //maximum baud rate possible = FPB = FOSC/4
        SPI_setClockDivider(ST7735_SPI, SPI_CLOCK_DIV4);
        SPI_begin(ST7735_SPI);
    }
    va_end(args);           // cleans up the list
    
    // default Screen Values

    ST7735[module].cursor.x      = 0;
    ST7735[module].cursor.y      = 0;
    ST7735[module].cursor.page   = 0;
    ST7735[module].screen.startx = 0;
    ST7735[module].screen.starty = 0;
    ST7735[module].screen.endx   = ST7735_WIDTH  - 1;
    ST7735[module].screen.endy   = ST7735_HEIGHT - 1;
    ST7735[module].screen.width  = ST7735_WIDTH;
    ST7735[module].screen.height = ST7735_HEIGHT;
    //ST7735[module].font.width    = 1;
    //ST7735[module].font.height   = 1;

    // Software reset and minimal init.
    
    //ST7735_low(ST7735[module].pin.cs);
    ST7735_select(module);                         // Chip select
    
    ST7735_sendCommand(module,(u8)ST7735_SWRESET); // software reset, puts display into sleep
    Delayms(150);
    ST7735_sendCommand(module,(u8)ST7735_SLPOUT);  // wake from sleep
    Delayms(500);
    ST7735_sendCommand(module,(u8)ST7735_COLMOD);  // set color mode to 16-bit
    ST7735_sendData(module,0x05);
    Delayms(10);
    ST7735_sendCommand(module,(u8)ST7735_MADCTL);  // RGB
    ST7735_sendData(module, ST7735_MADCTL_RGB);    // portrait
    Delayms(10);
    ST7735_sendCommand(module,(u8)ST7735_DISPON);  // display on!
    Delayms(100);
    ST7735_sendCommand(module,(u8)ST7735_NORON);   // normal display
    Delayms(10);
    
    //ST7735_high(ST7735[module].pin.cs);
    ST7735_deselect(module);                       // Chip deselected

    // Default colors and orientation
    
    //ST7735_setOrientation(module, 90);             // landscape orientation
    ST7735_setBackgroundColor(module, ST7735_BLACK);
    ST7735_setColor(module, ST7735_WHITE);
    ST7735_clearScreen(module);
}

///	--------------------------------------------------------------------
/// Set the display orientation to 0, 90, 180, or 270 degrees
///	--------------------------------------------------------------------

void ST7735_setOrientation(u8 module, s16 degrees)
{
    u8 arg;
    
    if (degrees < 0)
        degrees = 360 + degrees;
        
    ST7735[module].orientation = (u16)degrees;

    switch (degrees)
    {
        case  90:// OK
            ST7735[module].screen.endx   = ST7735_HEIGHT - 1;
            ST7735[module].screen.endy   = ST7735_WIDTH  - 1;
            ST7735[module].screen.width  = ST7735_HEIGHT;
            ST7735[module].screen.height = ST7735_WIDTH;
            arg = ST7735_MADCTL_MX | ST7735_MADCTL_MV | ST7735_MADCTL_RGB; // 0x60;
            break;
        case 180:// OK
            ST7735[module].screen.endx   = ST7735_WIDTH - 1;
            ST7735[module].screen.endy   = ST7735_HEIGHT  - 1;
            ST7735[module].screen.width  = ST7735_WIDTH;
            ST7735[module].screen.height = ST7735_HEIGHT;
            arg = ST7735_MADCTL_MX | ST7735_MADCTL_MY | ST7735_MADCTL_RGB; // 0xC0;
            break;
        case 270:// OK
            ST7735[module].screen.endx   = ST7735_HEIGHT - 1;
            ST7735[module].screen.endy   = ST7735_WIDTH  - 1;
            ST7735[module].screen.width  = ST7735_HEIGHT;
            ST7735[module].screen.height = ST7735_WIDTH;
            arg = ST7735_MADCTL_MY | ST7735_MADCTL_MV | ST7735_MADCTL_RGB; // 0xA0;
            break;
        case 0:// OK
        default:
            ST7735[module].screen.endx   = ST7735_WIDTH  - 1;
            ST7735[module].screen.endy   = ST7735_HEIGHT - 1;
            ST7735[module].screen.width  = ST7735_WIDTH;
            ST7735[module].screen.height = ST7735_HEIGHT;
            arg = ST7735_MADCTL_RGB; // 0x00;
            break;
    }
    
    //ST7735_low(ST7735[module].pin.cs);                     // Chip select
    ST7735_select(module);

    ST7735_sendCommand(module,ST7735_MADCTL);
    ST7735_sendData(module,arg);

    //ST7735_high(ST7735[module].pin.cs);                    // Chip deselected
    ST7735_deselect(module);
    
    ST7735[module].cursor.xmax  = ST7735[module].screen.width / ST7735[module].font.width;
    ST7735[module].cursor.ymax  = ST7735[module].screen.height / ST7735[module].font.height;
}

///	--------------------------------------------------------------------
/// Sets a rectangular display window into which pixel data is placed
///	--------------------------------------------------------------------

void ST7735_setWindow(u8 module, u8 x0, u8 y0, u8 x1, u8 y1)
{
    //ST7735_low(ST7735[module].pin.cs);       // Chip select
    ST7735_select(module);
    
    ST7735_low(ST7735[module].pin.dc);       // COMMAND = 0
    SPI_write(module, ST7735_CASET);         // set column range (x0,x1)

    ST7735_high(ST7735[module].pin.dc);      // DATA = 1
    SPI_write(module,0);
    SPI_write(module,x0);
    SPI_write(module,0);
    SPI_write(module,x1);

    ST7735_low(ST7735[module].pin.dc);       // COMMAND = 0
    SPI_write(module,ST7735_RASET);          // set row range (y0,y1)

    ST7735_high(ST7735[module].pin.dc);      // DATA = 1
    SPI_write(module,0);
    SPI_write(module,y0);
    SPI_write(module,0);
    SPI_write(module,y1);

    //ST7735_high(ST7735[module].pin.cs);      // Chip deselected
    ST7735_deselect(module);
}

///	--------------------------------------------------------------------
/// Sets current color
///	--------------------------------------------------------------------

void ST7735_setColor(u8 module, u16 c)
{
    ST7735[module].color.c = c;
}

///	--------------------------------------------------------------------
/// Sets background color
///	--------------------------------------------------------------------

void ST7735_setBackgroundColor(u8 module, u16 c)
{
    ST7735[module].bcolor.c = c;
}

///	--------------------------------------------------------------------
/// Gets pixel color
///	--------------------------------------------------------------------

color_t *ST7735_getColor(u8 module, u8 x, u8 y)
{
    color_t *color = NULL;
    u8 ch, cl;
    
    if ( x >= ST7735[module].screen.width || y >= ST7735[module].screen.height ) return 0;

    //ST7735_low(ST7735[module].pin.cs);       // Chip select
    ST7735_select(module);
    
    ST7735_low(ST7735[module].pin.dc);       // COMMAND = 0
    SPI_write(module,ST7735_CASET);          // set column range (x0,x1)
    
    ST7735_high(ST7735[module].pin.dc);      // DATA = 1
    SPI_write(module,0x00);
    SPI_write(module,x);
    SPI_write(module,0x00);
    SPI_write(module,x);
    
    ST7735_low(ST7735[module].pin.dc);       // COMMAND = 0
    SPI_write(module,ST7735_RASET);          // set row range (y0,y1)
    
    ST7735_high(ST7735[module].pin.dc);      // DATA = 1
    SPI_write(module,0x00);
    SPI_write(module,y);
    SPI_write(module,0x00);
    SPI_write(module,y);
    
    ST7735_low(ST7735[module].pin.dc);       // COMMAND = 0
    SPI_write(module,ST7735_RAMRD);          // Read RAM
    
    ST7735_high(ST7735[module].pin.dc);      // DATA = 1
    ch = SPI_read(module);
    cl = SPI_read(module);
    (*color).c = make16(ch, cl);
    
    //ST7735_high(ST7735[module].pin.cs);      // Chip deselected
    ST7735_deselect(module);
    
    return color;
}

///	--------------------------------------------------------------------
/// Packs individual red,green,blue color components into 16bit color
/// 16-bit color format is rrrrrggg.gggbbbbb
/// Max values: Red 31, Green 63, Blue 31
///	--------------------------------------------------------------------

color_t *ST7735_packColor(u8 r, u8 g, u8 b)
{
    color_t *color = NULL;

    //(*color).c = ((r & 0x1F) << 11 ) + ((g & 0x3F) << 5) + (b & 0x1F);
    (*color).c = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    return color;
}

///	--------------------------------------------------------------------
/// Reduces 16-bit color into component r,g,b values
///	--------------------------------------------------------------------

void ST7735_unpackColor(color_t *color)
{
    (*color).r =  (*color).c >> 11;
    (*color).g = ((*color).c & 0x07E0)>>5;
    (*color).b =  (*color).c & 0x001F;
}

///	--------------------------------------------------------------------
/// Clear the display
///	--------------------------------------------------------------------

void ST7735_clearScreen(u8 module)
{
    u16 i;
    u8 ch = ST7735[module].bcolor.c >> 8;
    u8 cl = ST7735[module].bcolor.c & 0xFF;
    
    //ST7735_low(ST7735[module].pin.cs);             // Chip select
    ST7735_select(module);

    ST7735_low(ST7735[module].pin.dc);             // COMMAND = 0
    SPI_write(module,ST7735_CASET);    // set column range (x0,x1)
    
    ST7735_high(ST7735[module].pin.dc);            // DATA = 1
    SPI_write(module,0x00);
    SPI_write(module,0x00);
    SPI_write(module,0x00);
    SPI_write(module,ST7735[module].screen.endx);
    
    ST7735_low(ST7735[module].pin.dc);             // COMMAND = 0
    SPI_write(module,ST7735_RASET);    // set row range (y0,y1)
    
    ST7735_high(ST7735[module].pin.dc);            // DATA = 1
    SPI_write(module,0x00);
    SPI_write(module,0x00);
    SPI_write(module,0x00);
    SPI_write(module,ST7735[module].screen.endy);
    
    ST7735_low(ST7735[module].pin.dc);             // COMMAND = 0
    SPI_write(module,ST7735_RAMWR);    // Write to RAM
        
    ST7735_high(ST7735[module].pin.dc);            // DATA = 1
    for (i = 0; i < ST7735_SIZE; i++)
    {
        SPI_write(module,ch);
        SPI_write(module,cl);
    }

    //ST7735_high(ST7735[module].pin.cs);            // Chip deselected
    ST7735_deselect(module);

    ST7735[module].cursor.x    = 0;
    ST7735[module].cursor.y    = 0;
}

///	--------------------------------------------------------------------
/// Clear the window
///	--------------------------------------------------------------------

void ST7735_clearWindow(u8 module, u8 x0, u8 y0, u8 x1, u8 y1)
{
    u8 x, y;
    u8 ch = ST7735[module].bcolor.c >> 8;
    u8 cl = ST7735[module].bcolor.c & 0xFF;
    
    //ST7735_low(ST7735[module].pin.cs);             // Chip select
    ST7735_select(module);
    
    ST7735_low(ST7735[module].pin.dc);             // COMMAND = 0
    SPI_write(module,ST7735_CASET);    // set column range (x0,x1)

    ST7735_high(ST7735[module].pin.dc);            // DATA = 1
    SPI_write(module,0x00);
    SPI_write(module,x0);
    SPI_write(module,0x00);
    SPI_write(module,x1);
    
    ST7735_low(ST7735[module].pin.dc);             // COMMAND = 0
    SPI_write(module,ST7735_RASET);    // set row range (y0,y1)

    ST7735_high(ST7735[module].pin.dc);            // DATA = 1
    SPI_write(module,0x00);
    SPI_write(module,y0);
    SPI_write(module,0x00);
    SPI_write(module,y1);
    
    ST7735_low(ST7735[module].pin.dc);             // COMMAND = 0
    SPI_write(module,ST7735_RAMWR);                // Write to RAM
        
    ST7735_high(ST7735[module].pin.dc);            // DATA = 1
    for (x = x0; x < x1; x++)
    {
        for (y = y0; y < y1; y++)
        {
            SPI_write(module,ch);
            SPI_write(module,cl);
        }
    }

    //ST7735_high(ST7735[module].pin.cs);            // Chip deselected
    ST7735_deselect(module);

    ST7735[module].cursor.x = 0;
    ST7735[module].cursor.y = 0;
}

///	--------------------------------------------------------------------
/// Print functions
///	--------------------------------------------------------------------

void ST7735_setFont(u8 module, const u8 *font)
{
    ST7735[module].font.address = font;
    ST7735[module].font.width   = font[0];
    ST7735[module].font.height  = font[1];
    ST7735[module].cursor.xmax  = ST7735[module].screen.width / ST7735[module].font.width;
    ST7735[module].cursor.ymax  = ST7735[module].screen.height / ST7735[module].font.height;
}

/*	--------------------------------------------------------------------
    DESCRIPTION:
        write a char at (ST7735[module].cursor.x, ST7735[module].cursor.y)
    PARAMETERS:
        * c ascii code of the character to print
    RETURNS:
    REMARKS:
------------------------------------------------------------------*/

void printChar(u8 c)
{
    ST7735_printChar(ST7735_SPI, c);
}

void ST7735_printChar(u8 module, u8 c)
{
    u8 h, w, b;
    u8 tx, ty;

    if (c > 0x7F)
        c = 0x20;

    while (ST7735[module].cursor.x >= ST7735[module].cursor.xmax)
    {
        ST7735[module].cursor.x = 0;
        ST7735[module].cursor.y++;            
    }

    while (ST7735[module].cursor.y > ST7735[module].cursor.ymax)
    {
        ST7735[module].cursor.y = 0;
        //ST7735_scrollUp();
        ST7735_clearScreen(module);
    }

    switch (c)
    {
        case '\n':
            ST7735[module].cursor.y++;
            break;
            
        case '\r':
            ST7735[module].cursor.x = 0;
            break;
            
        case '\t':
            ST7735[module].cursor.x += (ST7735[module].cursor.x + ST7735_TABSIZE) % ST7735_TABSIZE;
            break;
            
        default:
            tx = ST7735[module].cursor.x * ST7735[module].font.width;
            ty = ST7735[module].cursor.y * ST7735[module].font.height;
            for (h = 0; h < (ST7735[module].font.height-2); h++)
            {
                b = ST7735[module].font.address[2 + (c - 32) * ST7735[module].font.width + h];
                for (w = 0; w < (ST7735[module].font.width+2); w++)
                {
                    if (b & 1)
                        ST7735_drawPixel(module,  tx + h, ty + w);
                    else
                        ST7735_clearPixel(module, tx + h, ty + w);
                    b >>= 1;
                }
            }
            ST7735[module].cursor.x++;
    }
}

/*	--------------------------------------------------------------------
    DESCRIPTION:
        write a formated string at curent cursor position
    PARAMETERS:
        *fmt pointer on a formated string
    RETURNS:
    REMARKS:
------------------------------------------------------------------*/

#if defined(ST7735PRINT)       || defined(ST7735PRINTLN)    || \
    defined(ST7735PRINTNUMBER) || defined(ST7735PRINTFLOAT) || \
    defined(ST7735PRINTCENTER)
void ST7735_print(u8 module, const u8 *string)
{
    while (*string != 0)
        ST7735_printChar(module, *string++);
}
#endif

#if defined(ST7735PRINTLN)
void ST7735_println(u8 module, const u8 *string)
{
    ST7735_print(module, string);
    ST7735_print(module, (u8*)"\n\r");
}
#endif

#if defined(ST7735PRINTCENTER)
void ST7735_printCenter(u8 module, const u8 *string)
{
    u8 strlen, nbspace;
    const u8 *p;

    for (p = string; *p; ++p);
    strlen = p - string;

    nbspace = (ST7735[module].screen.width / ST7735[module].font.width - strlen) / 2;
    
    // write spaces before
    while(nbspace--)
        ST7735_printChar(module, 32);

    // write string
    ST7735_print(module, string);
    ST7735_print(module, (u8*)"\n\r");
}
#endif

#if defined(ST7735PRINTNUMBER) || defined(ST7735PRINTFLOAT)
void ST7735_printNumber(u8 module, long value, u8 base)
{
    ST7735_SPI = module;
    printNumber(value, base);
}
#endif

#if defined(ST7735PRINTFLOAT)
void ST7735_printFloat(u8 module, float number, u8 digits)
{ 
    ST7735_SPI = module;
    printFloat(number, digits);
}
#endif

#if defined(ST7735PRINTF)
void ST7735_printf(u8 module, const u8 *fmt, ...)
{
    static u8 buffer[25];
    u8 *c=(char*)&buffer;
    va_list	args;

    va_start(args, fmt);
    psprintf2(buffer, fmt, args);
    va_end(args);

    while (*c)
        ST7735_printChar(module, *c++);
}
#endif

///	--------------------------------------------------------------------
/// Graphic functions
///	--------------------------------------------------------------------

/*	--------------------------------------------------------------------
    DESCRIPTION:
        Sets the cursor to the specified x,y position
    PARAMETERS:
        x,y coord.
    RETURNS:
    REMARKS:
------------------------------------------------------------------*/

void ST7735_setCursor(u8 module, u8 x, u8 y)
{
    if (x >= ST7735[module].screen.width)  return;
    if (y >= ST7735[module].screen.height) return;

    ST7735[module].cursor.x = x;
    ST7735[module].cursor.y = y;
}

/*	--------------------------------------------------------------------
    DESCRIPTION:
        Draws a pixel with current color.
    PARAMETERS:
        x,y coord.
    RETURNS:
    REMARKS:
------------------------------------------------------------------*/

void ST7735_drawPixel(u8 module, u8 x, u8 y)
{
    if (x >= ST7735[module].screen.width)  return;
    if (y >= ST7735[module].screen.height) return;

    //ST7735_low(ST7735[module].pin.cs);           // Chip select
    ST7735_select(module);
    
    ST7735_low(ST7735[module].pin.dc);           // COMMAND = 0
    SPI_write(module,ST7735_CASET);            // set column range (x0,x1)
    
    ST7735_high(ST7735[module].pin.dc);          // DATA = 1
    SPI_write(module,0x00);
    SPI_write(module,x);
    //SPI_write(module,0x00);
    //SPI_write(module,x);
    
    ST7735_low(ST7735[module].pin.dc);           // COMMAND = 0
    SPI_write(module,ST7735_RASET);            // set row range (y0,y1)
    
    ST7735_high(ST7735[module].pin.dc);          // DATA = 1
    SPI_write(module,0x00);
    SPI_write(module,y);
    //SPI_write(module,0x00);
    //SPI_write(module,y);
    
    ST7735_low(ST7735[module].pin.dc);           // COMMAND = 0
    SPI_write(module,ST7735_RAMWR);
    
    ST7735_high(ST7735[module].pin.dc);          // DATA = 1
    SPI_write(module,ST7735[module].color.c >> 8);
    SPI_write(module,ST7735[module].color.c & 0xFF);
    
    //ST7735_high(ST7735[module].pin.cs);          // Chip deselected
    ST7735_deselect(module);
}

void ST7735_clearPixel(u8 module, u8 x, u8 y)
{
    if (x >= ST7735[module].screen.width)  return;
    if (y >= ST7735[module].screen.height) return;
    
    //ST7735_low(ST7735[module].pin.cs);           // Chip select
    ST7735_select(module);
    
    ST7735_low(ST7735[module].pin.dc);           // COMMAND = 0
    SPI_write(module,ST7735_CASET);            // set column range (x0,x1)

    ST7735_high(ST7735[module].pin.dc);          // DATA = 1
    SPI_write(module,0);
    SPI_write(module,x);
    SPI_write(module,0);
    SPI_write(module,x);

    ST7735_low(ST7735[module].pin.dc);           // COMMAND = 0
    SPI_write(module,ST7735_RASET);            // set row range (y0,y1)

    ST7735_high(ST7735[module].pin.dc);          // DATA = 1
    SPI_write(module,0);
    SPI_write(module,y);
    SPI_write(module,0);
    SPI_write(module,y);

    ST7735_low(ST7735[module].pin.dc);           // COMMAND = 0
    SPI_write(module,ST7735_RAMWR);

    ST7735_high(ST7735[module].pin.dc);          // DATA = 1
    SPI_write(module,ST7735[module].bcolor.c >> 8);
    SPI_write(module,ST7735[module].bcolor.c & 0xFF);
    
    //ST7735_high(ST7735[module].pin.cs);          // Chip deselected
    ST7735_deselect(module);
}

#if defined(ST7735GRAPHICS) || defined(ST7735DRAWBITMAP)

void ST7735_drawVLine(u8 module, u16 x, u16 y, u16 h)
{
    u8 ch = ST7735[module].color.c >> 8;
    u8 cl = ST7735[module].color.c & 0xFF;

    if (x >= ST7735[module].screen.width)  return;
    if (y >= ST7735[module].screen.height) return;

    if ((y+h-1) >= ST7735[module].screen.height)
        h = ST7735[module].screen.height - y;
        
    //ST7735_low(ST7735[module].pin.cs);             // Chip select
    ST7735_select(module);
    
    ST7735_low(ST7735[module].pin.dc);             // COMMAND = 0
    SPI_write(module,ST7735_CASET);    // set column range (x0,x1)
    
    ST7735_high(ST7735[module].pin.dc);            // DATA = 1
    SPI_write(module,0x00);
    SPI_write(module,x);
    SPI_write(module,0x00);
    SPI_write(module,x);
    
    ST7735_low(ST7735[module].pin.dc);             // COMMAND = 0
    SPI_write(module,ST7735_RASET);    // set row range (y0,y1)
    
    ST7735_high(ST7735[module].pin.dc);            // DATA = 1
    SPI_write(module,0x00);
    SPI_write(module,y);
    SPI_write(module,0x00);
    SPI_write(module,y+h-1);
    
    ST7735_low(ST7735[module].pin.dc);             // COMMAND = 0
    SPI_write(module,ST7735_RAMWR);
    
    ST7735_high(ST7735[module].pin.dc);            // DATA = 1
    while (h--)
    {
        SPI_write(module,ch);
        SPI_write(module,cl);
    }
    
    //ST7735_high(ST7735[module].pin.cs);            // Chip deselected
    ST7735_deselect(module);
}

void ST7735_drawHLine(u8 module, u16 x, u16 y, u16 w)
{
    u8 ch = ST7735[module].color.c >> 8;
    u8 cl = ST7735[module].color.c & 0xFF;

    if (x >= ST7735[module].screen.width)  return;
    if (y >= ST7735[module].screen.height) return;

    if ((x+w-1) >= ST7735[module].screen.width)
        w = ST7735[module].screen.width - x;
        
    //ST7735_low(ST7735[module].pin.cs);             // Chip select
    ST7735_select(module);
    
    ST7735_low(ST7735[module].pin.dc);             // COMMAND = 0
    SPI_write(module,ST7735_CASET);    // set column range (x0,x1)
    
    ST7735_high(ST7735[module].pin.dc);            // DATA = 1
    SPI_write(module,0x00);
    SPI_write(module,x);
    SPI_write(module,0x00);
    SPI_write(module,x+w-1);
    
    ST7735_low(ST7735[module].pin.dc);             // COMMAND = 0
    SPI_write(module,ST7735_RASET);    // set row range (y0,y1)
    
    ST7735_high(ST7735[module].pin.dc);            // DATA = 1
    SPI_write(module,0x00);
    SPI_write(module,y);
    SPI_write(module,0x00);
    SPI_write(module,y);
    
    ST7735_low(ST7735[module].pin.dc);             // COMMAND = 0
    SPI_write(module,ST7735_RAMWR);
    
    ST7735_high(ST7735[module].pin.dc);            // DATA = 1
    while (w--)
    {
        SPI_write(module,ch);
        SPI_write(module,cl);
    }
    
    //ST7735_high(ST7735[module].pin.cs);            // Chip deselected
    ST7735_deselect(module);
}
#endif

/*	--------------------------------------------------------------------
    DESCRIPTION:
        Graphic routines based on drawPixel in graphics.c
    PARAMETERS:
    RETURNS:
    REMARKS:
------------------------------------------------------------------*/

#if defined(ST7735GRAPHICS) || defined(ST7735DRAWBITMAP)

void setWindow(u8 x0, u8 y0, u8 x1, u8 y1)
{
    ST7735_setWindow(ST7735_SPI, x0, y0, x1, y1);
}

void drawPixel(u16 x, u16 y)
{
    ST7735_drawPixel(ST7735_SPI, x, y);
}

void setColor(u8 r, u8 g, u8 b)
{
    /*
    u16 c;
    r &= 0x1F;
    g &= 0x3F;
    b &= 0x1F;
    c = (r<<11) + (g<<5) + b;
    */
    ST7735_setColor(ST7735_SPI, ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

void drawHLine(u16 x, u16 y, u16 w)
{
    ST7735_drawHLine(ST7735_SPI, x, y, w);
}

void drawVLine(u16 x, u16 y, u16 h)
{
    ST7735_drawVLine(ST7735_SPI, x, y, h);
}

void ST7735_drawLine(u8 module, u16 x0, u16 y0, u16 x1, u16 y1)
{
    ST7735_SPI = module;
    drawLine(x0, y0, x1, y1);
}

void ST7735_drawRect(u8 module, u16 x1, u16 y1, u16 x2, u16 y2)
{
    ST7735_SPI = module;
    drawRect(x1, y1, x2, y2);
}

void ST7735_drawRoundRect(u8 module, u16 x1, u16 y1, u16 x2, u16 y2)
{
    ST7735_SPI = module;
    drawRoundRect(x1, y1, x2, y2);
}

void ST7735_drawCircle(u8 module, u16 x, u16 y, u16 radius)
{
    ST7735_SPI = module;
    drawCircle(x, y, radius);
}

void ST7735_fillCircle(u8 module, u16 x, u16 y, u16 radius)
{
    ST7735_SPI = module;
    fillCircle(x, y, radius);
}

void ST7735_fillRect(u8 module, u16 x1, u16 y1, u16 x2, u16 y2)
{
    ST7735_SPI = module;
    fillRect(x1, y1, x2, y2);
}

void ST7735_fillRoundRect(u8 module, u16 x1, u16 y1, u16 x2, u16 y2)
{
    ST7735_SPI = module;
    fillRoundRect(x1, y1, x2, y2);
}

#ifdef ST7735DRAWBITMAP
void ST7735_drawBitmap(u8 module1, u8 module2, const u8* filename, u16 x, u16 y)
{
    ST7735_SPI = module1;
    drawBitmap(module2, filename, x, y);
}
#endif

#endif // ST7735GRAPHICS || ST7735DRAWBITMAP

#endif // __ST7735_C
