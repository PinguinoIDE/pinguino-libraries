/*  -------------------------------------------------------------------
    FILE:           ST7565.c
    PROJECT:        Pinguino
    PURPOSE:        Drive 128x64 Monochrome TFT display (ST7565 controller)
    PROGRAMER:      Regis Blanchot <rblanchot@gmail.com>
    --------------------------------------------------------------------
    31 Jan. 2017    Regis Blanchot - first release
    --------------------------------------------------------------------
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

#ifndef __ST7565_C
#define __ST7565_C

#ifndef __PIC32MX__
#include <compiler.h>
#endif
#include <typedef.h>        // u8
#include <macro.h>          // swap
#include <stdarg.h>
#include <string.h>         // memset
#include <ST7565.h>
#include <spi.h>
#include <spi.c>
#include <digitalw.c>       // pinmode, digitalwrite
#ifndef __PIC32MX__
#include <digitalp.c>
#include <delayms.c>        // Delayms
#else
#include <delay.c>
#endif

// Printf
#ifdef ST7565PRINTF
    #include <printFormated.c>
#endif

// PrintFloat
#if defined(ST7565PRINTFLOAT)
    #include <printFloat.c>
#endif

// PrintNumber
#if defined(ST7565PRINTNUMBER) || defined(ST7565PRINTFLOAT)
    #include <printNumber.c>
#endif

// Graphics Library
#if defined(ST7565GRAPHICS) || defined(ST7565DRAWBITMAP)
    #ifdef ST7565DRAWBITMAP
    #define DRAWBITMAP
    #endif
    #include <graphics.c>
#endif

///	--------------------------------------------------------------------
/// Core functions
///	--------------------------------------------------------------------

// a handy reference to where the pages are on the screen
const u8 pagemap[] = { 3, 2, 1, 0, 7, 6, 5, 4 };

u8 ST7565_buffer[1024];

// reduces how much is refreshed, which speeds it up!
// originally derived from Steve Evans/JCW's mod but cleaned up and
// optimized

#ifdef enablePartialUpdate
static u8 xUpdateMin, xUpdateMax, yUpdateMin, yUpdateMax;

static void updateBoundingBox(u8 xmin, u8 ymin, u8 xmax, u8 ymax)
{
    if (xmin < xUpdateMin) xUpdateMin = xmin;
    if (xmax > xUpdateMax) xUpdateMax = xmax;
    if (ymin < yUpdateMin) yUpdateMin = ymin;
    if (ymax > yUpdateMax) yUpdateMax = ymax;
}
#endif

void ST7565_sendCommand(u8 module, u8 val)
{
    ST7565_low(ST7565.pin.dc);            // COMMAND = 0
    SPI_write(module, val);
}

void ST7565_sendData(u8 module, u8 val)
{
    ST7565_high(ST7565.pin.dc);           // DATA = 1
    SPI_write(module, val);
}

void ST7565_setBrightness(u8 module, u8 val) 
{
    ST7565_sendCommand(module, ST7565_SET_VOLUME_FIRST);
    ST7565_sendCommand(module, ST7565_SET_VOLUME_SECOND | (val & 0x3f));
}

void ST7565_refresh(u8 module) 
{
    u8 col, maxcol, p;
    u16 tmp;

    // For each page
    for (p = 0; p < 8; p++)
    {
        #ifdef enablePartialUpdate
        // check if this page is part of update
        if ( yUpdateMin >= ((p+1)*8) )
            continue;   // nope, skip it!
        if (yUpdateMax < p*8)
            break;
        #endif

        ST7565_sendCommand(module, ST7565_SET_PAGE | pagemap[p]);

        #ifdef enablePartialUpdate
        col = xUpdateMin;
        maxcol = xUpdateMax;
        #else
        // start at the beginning of the row
        col = 0;
        maxcol = ST7565_WIDTH-1;
        #endif

        ST7565_sendCommand(module, ST7565_SET_COLUMN_LOWER | ((col+ST7565_STARTBYTES) & 0xf));
        ST7565_sendCommand(module, ST7565_SET_COLUMN_UPPER | (((col+ST7565_STARTBYTES) >> 4) & 0x0F));
        ST7565_sendCommand(module, ST7565_RMW);

        tmp = p << 7;                                   // 128 * p;
        ST7565_high(ST7565.pin.dc);             // DATA = 1
        for(; col <= maxcol; col++)
            //ST7565_sendData(ST7565_buffer[tmp+col]);
            SPI_write(module, ST7565_buffer[tmp+col]);
    }

    #ifdef enablePartialUpdate
    xUpdateMin = ST7565_WIDTH - 1;
    xUpdateMax = 0;
    yUpdateMin = ST7565_HEIGHT-1;
    yUpdateMax = 0;
    #endif
}

// clear everything
void ST7565_clearScreen(u8 module) 
{
    memset(ST7565_buffer, 0, 1024);
    #ifdef enablePartialUpdate
    updateBoundingBox(0, 0, ST7565_WIDTH-1, ST7565_HEIGHT-1);
    #endif
}

// this doesnt touch the buffer, just clears the display RAM - might be handy
void ST7565_clearDisplay(u8 module) 
{
    u8 p, c;

    for(p = 0; p < 8; p++)
    {
        ST7565_sendCommand(module, ST7565_SET_PAGE | p);
        for(c = 0; c < 129; c++)
        {
            ST7565_sendCommand(module, ST7565_SET_COLUMN_LOWER | (c & 0xf));
            ST7565_sendCommand(module, ST7565_SET_COLUMN_UPPER | ((c >> 4) & 0xf));
            ST7565_sendData(module, 0x0);
        }
    }
}

void ST7565_init(int module, ...)
{
    int sdo, sck, cs;
    va_list args;
    
    ST7565_SPI = module;
    
    va_start(args, module); // args points on the argument after module

    ST7565.pin.dc = va_arg(args, int); // get the first arg
    pinmode(ST7565.pin.dc, OUTPUT);

    // init SPI communication
    if (ST7565_SPI == SPISW)
    {
        sdo = va_arg(args, int);         // get the next arg
        sck = va_arg(args, int);         // get the next arg
        cs  = va_arg(args, int);         // get the last arg
        SPI_setBitOrder(ST7565_SPI, SPI_MSBFIRST);
        SPI_begin(ST7565_SPI, sdo, sck, cs);
    }
    else
    {
        SPI_setMode(ST7565_SPI, SPI_MASTER);
        SPI_setDataMode(ST7565_SPI, SPI_MODE1);
        #ifndef __PIC32MX__
        //maximum baud rate possible = FPB = FOSC/4
        SPI_setClockDivider(ST7565_SPI, SPI_CLOCK_DIV4);
        #else
        //maximum baud rate possible = FPB/2
        SPI_setClockDivider(ST7565_SPI, SPI_PBCLOCK_DIV2);
        #endif
        //SPI_begin(ST7565_SPI, NULL);
        SPI_begin(ST7565_SPI);
    }

    va_end(args);                       // cleans up the list
    
    // default Screen Values

    ST7565.pixel.x       = 0;
    ST7565.pixel.y       = 0;
    ST7565.pixel.page    = 0;
    ST7565.screen.startx = 0;
    ST7565.screen.starty = 0;
    ST7565.screen.endx   = ST7565_WIDTH  - 1;
    ST7565.screen.endy   = ST7565_HEIGHT - 1;
    ST7565.screen.width  = ST7565_WIDTH;
    ST7565.screen.height = ST7565_HEIGHT;

    // Software reset and minimal init.
    SPI_select(module);

    ST7565_sendCommand(module, ST7565_INTERNAL_RESET);
    //digitalwrite(rst, LOW);
    Delayms(500);
    //digitalwrite(rst, HIGH);

    // LCD bias select
    ST7565_sendCommand(module, ST7565_SET_BIAS_7);
    // ADC select
    ST7565_sendCommand(module, ST7565_SET_ADC_NORMAL);
    // SHL select
    ST7565_sendCommand(module, ST7565_SET_COM_NORMAL);
    // Initial display line
    ST7565_sendCommand(module, ST7565_SET_DISP_START_LINE);
    // turn on voltage converter (VC=1, VR=0, VF=0)
    ST7565_sendCommand(module, ST7565_SET_POWER_CONTROL | 0x4);
    // wait for 50% rising
    Delayms(50);
    // turn on voltage regulator (VC=1, VR=1, VF=0)
    ST7565_sendCommand(module, ST7565_SET_POWER_CONTROL | 0x6);
    // wait >=50ms
    Delayms(50);
    // turn on voltage follower (VC=1, VR=1, VF=1)
    ST7565_sendCommand(module, ST7565_SET_POWER_CONTROL | 0x7);
    // wait
    Delayms(10);
    // set lcd operating voltage (regulator resistor, ref voltage resistor)
    ST7565_sendCommand(module, ST7565_SET_RESISTOR_RATIO | 0x6);

    // initial display line
    // set page address
    // set column address
    // write display data

    // set up a bounding box for screen updates

    #ifdef enablePartialUpdate
    updateBoundingBox(0, 0, ST7565_WIDTH-1, ST7565_HEIGHT-1);
    #endif
    
    ST7565_sendCommand(module, ST7565_DISPLAY_ON);
    ST7565_sendCommand(module, ST7565_SET_ALLPTS_NORMAL);
    ST7565_setBrightness(module, 0x18);
}

///	--------------------------------------------------------------------
/// Print functions
///	--------------------------------------------------------------------

#ifdef ST7565SETFONT

void ST7565_setFont(u8 module, const u8 *font)
{
    ST7565.font.address   = font;
    ST7565.font.width     = font[FONT_WIDTH];
    ST7565.font.height    = font[FONT_HEIGHT];
    ST7565.font.firstChar = font[FONT_FIRST_CHAR];
    ST7565.font.charCount = font[FONT_CHAR_COUNT];
}

/*  --------------------------------------------------------------------
    DESCRIPTION:
        write a char at current position
    PARAMETERS:
        * c ascii code of the character to print
    RETURNS:
    REMARKS:
    ------------------------------------------------------------------*/

void ST7565_printChar2(u8 c)
{
    ST7565_printChar(ST7565_SPI, c);
}

void ST7565_printChar(u8 module, u8 c)
{
    u8  x, y;
    u8  i, j, k, h;
    u8  dat, page, tab;
    u8  width = 0;
    u8  bytes = (ST7565.font.height + 7) / 8;
    u16 index = 0;

    if ((ST7565.pixel.x + ST7565.font.width) > ST7565.screen.width)
    {
        ST7565.pixel.x = 0;
        ST7565.pixel.y = ST7565.pixel.y + bytes*8; //ST7565.font.height;
    }

    if ((ST7565.pixel.y + ST7565.font.height) > ST7565.screen.height)
    {
        ST7565.pixel.y = 0;
        //ST7565_scrollUp(module);            
    }

    switch (c)
    {
        case '\n':
            ST7565.pixel.y = ST7565.pixel.y + bytes*8; //ST7565.font.height;
            break;
            
        case '\r':
            ST7565.pixel.x = 0;
            break;
            
        case '\t':
            tab = ST7565_TABSIZE * ST7565.font.width;
            ST7565.pixel.x += (ST7565.pixel.x + tab) % tab;
            break;
            
        default:
            if (c < ST7565.font.firstChar || c >= (ST7565.font.firstChar + ST7565.font.charCount))
                c = ' ';
            c = c - ST7565.font.firstChar;

            // fixed width font
            if (ST7565.font.address[FONT_LENGTH]==0 && ST7565.font.address[FONT_LENGTH+1]==0)
            {
                width = ST7565.font.width; 
                index = FONT_OFFSET + c * bytes * width;
            }

            // variable width font
            else
            {
                width = ST7565.font.address[FONT_WIDTH_TABLE + c];
                for (i=0; i<c; i++)
                    index += ST7565.font.address[FONT_WIDTH_TABLE + i];
                index = FONT_WIDTH_TABLE + ST7565.font.charCount + index * bytes;
            }

            // save the coordinates
            x = ST7565.pixel.x;
            y = ST7565.pixel.y;

            // draw the character
            for (i=0; i<bytes; i++)
            {
                page = i * width;
                for (j=0; j<width; j++)
                {
                    dat = ST7565.font.address[index + page + j];
                    // if char. takes place on more than 1 line (8 bits)
                    if (ST7565.font.height > 8)
                    {
                        k = ((i+1)<<3);
                        if (ST7565.font.height < k)
                            dat >>= k - ST7565.font.height;
                    }
                    // Write the byte
                    for (h = 0; h < 8; h++)
                    {
                        if (dat & 1)
                            ST7565_drawPixel(module,  x + j, y + h);
                        else
                            ST7565_clearPixel(module, x + j, y + h);
                        dat >>= 1;
                    }
                }

                // 1px gap between chars
                for (h = 0; h < 8; h++)
                    ST7565_clearPixel(module, x + width, y + h);

                // Next part of the current char will be one line under
                y += 8;
            }
            // Next char location
            ST7565.pixel.x = x + width + 1;
            break;

            /*
            tx = ST7565.cursor.x * ST7565.font.width;
            ty = ST7565.cursor.y * ST7565.font.height;
            for (h = 0; h < (ST7565.font.height-2); h++)
            {
                b = ST7565.font.address[2 + (c - 32) * ST7565.font.width + h];
                for (w = 0; w < (ST7565.font.width+2); w++)
                {
                    if (b & 1)
                        ST7565_drawPixel(module,  tx + h, ty + w);
                    else
                        ST7565_clearPixel(module, tx + h, ty + w);
                    b >>= 1;
                }
            }
            ST7565.cursor.x++;
            */
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

#if defined(ST7565PRINT)       || defined(ST7565PRINTLN)    || \
    defined(ST7565PRINTNUMBER) || defined(ST7565PRINTFLOAT) || \
    defined(ST7565PRINTCENTER)
void ST7565_print(u8 module, const u8 *string)
{
    while (*string != 0)
        ST7565_printChar(module, *string++);
}
#endif

#if defined(ST7565PRINTLN)
void ST7565_println(u8 module, const u8 *string)
{
    ST7565_print(module, string);
    ST7565_print(module, (u8*)"\n\r");
}
#endif

#if defined(ST7565PRINTCENTER)
void ST7565_printCenter(u8 module, const u8 *string)
{
    ST7565.pixel.x = (ST7565.screen.width - ST7565_stringWidth(module, string)) / 2;
    
    // write string
    while (*string != 0)
        ST7565_printChar(module, *string++);
}

u8 ST7565_charWidth(u8 module, u8 c)
{
    // fixed width font
    if (ST7565.font.address[FONT_LENGTH]==0 && ST7565.font.address[FONT_LENGTH+1]==0)
        return (ST7565.font.width + 1); 

    // variable width font
    if (c < ST7565.font.firstChar || c > (ST7565.font.firstChar + ST7565.font.charCount))
        c = ' ';
    c = c - ST7565.font.firstChar;
    return (ST7565.font.address[FONT_WIDTH_TABLE + c] + 1);
}

u16 ST7565_stringWidth(u8 module, const u8* str)
{
    u16 width = 0;

    while(*str != 0)
        width += ST7565_charWidth(module, *str++);

    return width;
}

#endif

#if defined(ST7565PRINTNUMBER) || defined(ST7565PRINTFLOAT)
void ST7565_printNumber(u8 module, long value, u8 base)
{
    ST7565_SPI = module;
    printNumber(ST7565_printChar2, value, base);
}
#endif

#if defined(ST7565PRINTFLOAT)
void ST7565_printFloat(u8 module, float number, u8 digits)
{ 
    ST7565_SPI = module;
    printFloat(ST7565_printChar2, number, digits);
}
#endif

#if defined(ST7565PRINTF)
void ST7565_printf(u8 module, const u8 *fmt, ...)
{
    static u8 buffer[25];
    u8 *c=(char*)&buffer;
    va_list	args;

    va_start(args, fmt);
    psprintf2(buffer, fmt, args);
    va_end(args);

    while (*c)
        ST7565_printChar(module, *c++);
}
#endif

#endif // ST7565SETFONT

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

void ST7565_setCursor(u8 module, u8 x, u8 y)
{
    if (x >= ST7565.screen.width)  return;
    if (y >= ST7565.screen.height) return;

    ST7565.pixel.x = x * (ST7565.font.width+1);
    ST7565.pixel.y = y * (ST7565.font.height+1);}

/*	--------------------------------------------------------------------
    DESCRIPTION:
        Draws a pixel with current color.
    PARAMETERS:
        x,y coord.
    RETURNS:
    REMARKS:
------------------------------------------------------------------*/

// the most basic function, set a single pixel
void ST7565_setPixel(u8 module, u8 x, u8 y, u8 color) 
{
    if ((x >= ST7565_WIDTH) || (y >= ST7565_HEIGHT))
        return;

    // x is which column
    if (color) 
        ST7565_buffer[x+ (y/8)*128] |= Bit(7-(y%8));  
    else
        ST7565_buffer[x+ (y/8)*128] &= ~Bit(7-(y%8)); 

    #ifdef enablePartialUpdate
    updateBoundingBox(x,y,x,y);
    #endif
}


void ST7565_drawPixel(u8 module, u8 x, u8 y)
{
    ST7565_setPixel(module, x, y, 1);
}

void ST7565_clearPixel(u8 module, u8 x, u8 y)
{
    ST7565_setPixel(module, x, y, 0);
}

// the most basic function, get a single pixel
u8 ST7565_getPixel(u8 module, u8 x, u8 y) 
{
    if ((x >= ST7565_WIDTH) || (y >= ST7565_HEIGHT))
        return 0;

    return (ST7565_buffer[x+ (y/8)*128] >> (7-(y%8))) & 0x1;  
}

/*	--------------------------------------------------------------------
    DESCRIPTION:
        Graphic routines based on drawPixel in graphics.c
    PARAMETERS:
    RETURNS:
    REMARKS:
------------------------------------------------------------------*/

#if defined(ST7565GRAPHICS) || defined(ST7565DRAWBITMAP)

#if 0
void setWindow(u8 x0, u8 y0, u8 x1, u8 y1)
{
    ST7565_setWindow(ST7565_SPI, x0, y0, x1, y1);
}
#endif

void drawPixel(u16 x, u16 y)
{
    ST7565_drawPixel(ST7565_SPI, x, y);
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
    ST7565_setColor(ST7565_SPI, ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

void ST7565_drawLine(u8 module, u16 x0, u16 y0, u16 x1, u16 y1)
{
    ST7565_SPI = module;
    drawLine(x0, y0, x1, y1);
}

void ST7565_drawRect(u8 module, u16 x1, u16 y1, u16 x2, u16 y2)
{
    ST7565_SPI = module;
    drawRect(x1, y1, x2, y2);
}

void ST7565_drawRoundRect(u8 module, u16 x1, u16 y1, u16 x2, u16 y2)
{
    ST7565_SPI = module;
    drawRoundRect(x1, y1, x2, y2);
}

void ST7565_drawCircle(u8 module, u16 x, u16 y, u16 radius)
{
    ST7565_SPI = module;
    drawCircle(x, y, radius);
}

void ST7565_fillCircle(u8 module, u16 x, u16 y, u16 radius)
{
    ST7565_SPI = module;
    fillCircle(x, y, radius);
}

void ST7565_fillRect(u8 module, u16 x1, u16 y1, u16 x2, u16 y2)
{
    ST7565_SPI = module;
    fillRect(x1, y1, x2, y2);
}

void ST7565_fillRoundRect(u8 module, u16 x1, u16 y1, u16 x2, u16 y2)
{
    ST7565_SPI = module;
    fillRoundRect(x1, y1, x2, y2);
}

#ifdef ST7565DRAWBITMAP
void ST7565_drawBitmap(u8 module1, u8 module2, const u8* filename, u16 x, u16 y)
{
    ST7565_SPI = module1;
    drawBitmap(module2, filename, x, y);
}
#endif

#endif // ST7565GRAPHICS || ST7565DRAWBITMAP

#endif // __ST7565_C
