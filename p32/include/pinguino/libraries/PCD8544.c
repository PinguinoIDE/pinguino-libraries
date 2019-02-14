/*  --------------------------------------------------------------------
    File          : PCD8544.c
    Project       : Pinguino
    Description   : Pinguino C library for Monochrome Nokia LCD
                    with PCD8544 controler
    Author        : Thomas Missonier (sourcezax@users.sourceforge.net)
                    Régis Blanchot (rblanchot@gmail.com)
    First release : March 2014
    --------------------------------------------------------------------
    CHANGELOG:
    04 Feb. 2016 - Régis Blanchot - added Pinguino SPI library support
    22 Oct. 2016 - Régis Blanchot - fixed graphics functions
    23 Mar. 2017 - Régis Blanchot - fixed PIC18F RAM limitations
    --------------------------------------------------------------------
    TODO:
    * Backlight management
    --------------------------------------------------------------------
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    - Redistributions of source code must retain the above copyright notice,
      this list of conditions, the others licences below, and the following disclaimer.
    - Redistributions in binary form must reproduce the above notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.
    ------------------------------------------------------------------*/

#ifndef __PCD8544_C
#define __PCD8544_C

//#define PCD8544DEBUG

#include <stdarg.h>
#include <const.h>              // false, true, ...
#include <macro.h>              // BitSet, BitClear, ...
#include <typedef.h>            // u8, u16, ...
//#ifndef __SDCC
#include <string.h>             // memset, memcpy
//#endif
#include <PCD8544.h>
#include <spi.c>                // SPI harware and software functions
#include <spi.h>

#ifndef __PIC32MX__
#include <delayms.c>            // Delayms, Delayus
#include <digitalw.c>           // digitalwrite
#include <digitalp.c>           // pinmode
#else
#include <delay.c>              // Delayms, Delayus
#include <digitalw.c>           // digitalwrite
#endif

// Serial debug
#ifdef PCD8544DEBUG
    #define SERIALPRINTF
    #include <serial.c>
#endif
    
// Printf
#if defined(PCD8544PRINTF)
    #include <printFormated.c>
#endif

// PrintFloat
#if defined(PCD8544PRINTFLOAT)
    #include <printFloat.c>
#endif

// PrintNumber
#if defined(PCD8544PRINTNUMBER) || defined(PCD8544PRINTFLOAT)
    #include <printNumber.c>
#endif

// Graphics
#ifdef PCD8544GRAPHICS
#include <graphics.c>           // graphic routines
#endif

//#include <logo/pinguino84x48.h> // Pinguino Logo
//static volatile u8 * PCD8544_buffer = logo;  // screen buffer points on logo[]
//u8 * PCD8544_buffer = logo;  // screen buffer points on logo (84*48/8)

// Screen buffers / 1 buffer per line
// NB : PIC18F RAM bank holds 256 bytes only,
// too small to stock the whole buffer (6 x 84 = 506 bytes)
u8 row0[PCD8544_DISPLAY_WIDTH];
u8 row1[PCD8544_DISPLAY_WIDTH];
u8 row2[PCD8544_DISPLAY_WIDTH];
u8 row3[PCD8544_DISPLAY_WIDTH];
u8 row4[PCD8544_DISPLAY_WIDTH];
u8 row5[PCD8544_DISPLAY_WIDTH];

// Buffers pointers
u8* PCD8544_buffer[PCD8544_DISPLAY_ROWS] = { row0, row1, row2, row3, row4, row5 };

///	--------------------------------------------------------------------
/// Core functions
///	--------------------------------------------------------------------

// DC = LOW => CMD
void PCD8544_command(u8 module, u8 cmd)
{
    digitalwrite(PCD8544[module].pin.dc, LOW);
    SPI_write(module, cmd);
}
 
// DC = HIGH => DATA
#if 0
void PCD8544_data(u8 module, u8 c)
{
    digitalwrite(PCD8544[module].pin.dc, HIGH);
    SPI_write(module, c);
}
#endif

///	--------------------------------------------------------------------
/// Display functions
///	--------------------------------------------------------------------

void PCD8544_init(int module, ...)
{
    u8 row;
    int sdo, sck, cs;
    va_list args;
    
    #ifdef PCD8544DEBUG
    Serial_begin(9600);
    // Disable RX (same pin as SPI SDO)
    RCSTAbits.CREN = 0;
    Delayms(1000);
    Serial_printf("\fInit ...\r\n");
    #endif
        
    PCD8544_SPI = (u8)module;
    
    #ifdef PCD8544DEBUG
    Serial_printf("SPI%d\r\n", PCD8544_SPI);
    #endif

    va_start(args, module); // args points on the argument after module

    PCD8544[PCD8544_SPI].pin.dc = (u8)va_arg(args, int); // get the first arg
    pinmode(PCD8544[PCD8544_SPI].pin.dc, OUTPUT);
    
    #ifdef PCD8544DEBUG
    Serial_printf("DC = pin %d\r\n", PCD8544[PCD8544_SPI].pin.dc);
    #endif

    PCD8544[PCD8544_SPI].pin.rst = (u8)va_arg(args, int); // get the first arg
    pinmode(PCD8544[PCD8544_SPI].pin.rst, OUTPUT);
    digitalwrite(PCD8544[PCD8544_SPI].pin.rst, HIGH);

    #ifdef PCD8544DEBUG
    Serial_printf("RST = pin %d\r\n", PCD8544[PCD8544_SPI].pin.rst);
    #endif

    #ifdef PCD8544DEBUG
    for (row = 0; row < PCD8544_DISPLAY_ROWS; row++)
    Serial_printf("Address row%d = 0x%X\r\n", row, PCD8544_buffer[row]);
    #endif

    // init SPI communication before reset

    if (module == SPISW)
    {
        sdo = va_arg(args, int);         // get the next arg
        sck = va_arg(args, int);         // get the next arg
        cs  = va_arg(args, int);         // get the last arg
        SPI_setBitOrder(PCD8544_SPI, SPI_MSBFIRST);
        SPI_begin(PCD8544_SPI, sdo, sck, cs);
    }
    else
    {
        SPI_setMode(PCD8544_SPI, SPI_MASTER);
        //SPI_setDataMode(PCD8544_SPI, SPI_MODE0);
        SPI_setDataMode(PCD8544_SPI, SPI_MODE1);
        // Maximum baud rate is 4.0 Mbits/s
        #ifndef __PIC32MX__
        SPI_setClockDivider(PCD8544_SPI, SPI_CLOCK_DIV16);
        #else
        SPI_setClockDivider(PCD8544_SPI, SPI_PBCLOCK_DIV4);
        #endif
        SPI_begin(PCD8544_SPI);
    }
    va_end(args);           // cleans up the list
    
    // Init.
    // NB : each command can be sent in any order
    PCD8544_select(PCD8544_SPI);
    // toggle RST low to reset
    // RST may be LOW before VDD goes HIGH.
    digitalwrite(PCD8544[PCD8544_SPI].pin.rst, LOW);
    Delayms(70);// 30 < Delayms < 100
    digitalwrite(PCD8544[PCD8544_SPI].pin.rst, HIGH);
    // get into the extended mode
    PCD8544_command(PCD8544_SPI, PCD8544_FUNCTIONSET | PCD8544_EXTENDEDINSTRUCTION );
    // LCD bias select (4 is optimal?)
    PCD8544_command(PCD8544_SPI, PCD8544_SETBIAS | 4);
    // set VOP
    PCD8544_command(PCD8544_SPI, PCD8544_SETVOP | 0x7F); // default contrast
    // back form extended to normal mode
    PCD8544_command(PCD8544_SPI, PCD8544_FUNCTIONSET);
    // Set display to Normal
    PCD8544_command(PCD8544_SPI, PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL);
    PCD8544_deselect(PCD8544_SPI);

    // GFX properties
    PCD8544[PCD8544_SPI].screen.width  = PCD8544_DISPLAY_WIDTH;
    PCD8544[PCD8544_SPI].screen.height = PCD8544_DISPLAY_HEIGHT;
    PCD8544[PCD8544_SPI].screen.rows   = PCD8544_DISPLAY_ROWS;
    PCD8544[PCD8544_SPI].orientation   = PCD8544_PORTRAIT;
    PCD8544[PCD8544_SPI].pixel.y       = 0;
    PCD8544[PCD8544_SPI].pixel.x       = 0;

    // Push out PCD8544_buffer to the Display
    // Will show the Pinguino logo
    //PCD8544_refresh(module);
}

void PCD8544_refresh(u8 module)
{
    u8 row, x;
    
    PCD8544_select(module);

    // Send command Home
    digitalwrite(PCD8544[module].pin.dc, LOW);
    SPI_write(module, PCD8544_SETYADDR | 0);
    SPI_write(module, PCD8544_SETXADDR | 0);

    // Send data in DDRAM
    // NB : After every data byte, the address counter is incremented
    // automatically.
    digitalwrite(PCD8544[module].pin.dc, HIGH);
    for (row = 0; row < PCD8544_DISPLAY_ROWS; row++)
    {
        for (x = 0; x < PCD8544_DISPLAY_WIDTH; x++)
        {
            //#ifdef __SDCC
            //SPI_write(module, *(PCD8544_buffer[row])++);
            //#else
            SPI_write(module, PCD8544_buffer[row][x]);
            //#endif
        }
    }
    
    PCD8544_deselect(module);
}

void PCD8544_clearScreen(u8 module)
{
    u8 row, x;
    
    for (row = 0; row < PCD8544_DISPLAY_ROWS; row++)
    {
        //#ifdef __SDCC
        //memset(&PCD8544_buffer[row], 0, PCD8544_DISPLAY_WIDTH);
        for (x = 0; x < PCD8544_DISPLAY_WIDTH; x++)
            PCD8544_buffer[row][x] = 0;
            //*(PCD8544_buffer[row])++ = 0;
        //#else  
        //memset(PCD8544_buffer[row], 0, PCD8544_DISPLAY_WIDTH);
        //#endif
    }
    
    // home position
    PCD8544[module].pixel.x = 0;
    PCD8544[module].pixel.y = 0;
}

#ifdef _PCD8544_USE_DISPLAYONOFF
void PCD8544_displayOff(u8 module)
{
    // First, fill RAM with zeroes to ensure minimum specified current consumption
    PCD8544_clearScreen(module);
    PCD8544_refresh(module);
    PCD8544_select(module);
    PCD8544_command(module, PCD8544_FUNCTIONSET|PCD8544_POWERDOWN);
    PCD8544_deselect(module);
}

void PCD8544_displayOn(u8 module)
{
    PCD8544_select(module);
    PCD8544_command(module, PCD8544_FUNCTIONSET);
    PCD8544_deselect(module);
}
#endif

void PCD8544_setContrast(u8 module, u8 val)
{
    PCD8544_select(module);
    PCD8544_command(module, PCD8544_FUNCTIONSET | PCD8544_EXTENDEDINSTRUCTION );
    PCD8544_command(module, PCD8544_SETVOP | (val & 0x7F)); 
    PCD8544_command(module, PCD8544_FUNCTIONSET);
    PCD8544_deselect(module);
}

///	--------------------------------------------------------------------
/// Scroll functions
///	--------------------------------------------------------------------

#if defined(PCD8544PRINTCHAR)   || defined(PCD8544PRINT)      || \
    defined(PCD8544PRINTNUMBER) || defined(PCD8544PRINTFLOAT) || \
    defined(PCD8544PRINTLN)     || defined(PCD8544PRINTF)

// Up handed 1-row scroll
// The display is 16 rows tall.
void PCD8544_scrollUp(u8 module)
{
    u8 row, x;
    u8 bytes = ((PCD8544[module].font.height + 7) / 8);
    u8 lastline = PCD8544_DISPLAY_ROWS - bytes;
    
    // Copy line i+1 in line i
    //void *memcpy(void *dest, const void *src, size_t n)
    for (row=0; row<lastline; row++)
    {
        #ifdef __SDCC
        //memcpy(&PCD8544_buffer[row], &PCD8544_buffer[row+1], PCD8544_DISPLAY_WIDTH);
        for (x = 0; x < PCD8544_DISPLAY_WIDTH; x++)
            PCD8544_buffer[row][x] = PCD8544_buffer[row+1][x];
            //*(PCD8544_buffer[row])++ = *(PCD8544_buffer[row+1])++;
        #else
        memcpy(PCD8544_buffer[row], PCD8544_buffer[row+1], PCD8544_DISPLAY_WIDTH);
        #endif
    }
    // Clear the last lines
    //void *memset(void *str, int c, size_t n)
    for (row=lastline; row<PCD8544_DISPLAY_ROWS; row++)
    {
        #ifdef __SDCC
        //memset(&PCD8544_buffer[row], 0, PCD8544_DISPLAY_WIDTH);
        for (x = 0; x < PCD8544_DISPLAY_WIDTH; x++)
            PCD8544_buffer[row][x] = 0;
            //*(PCD8544_buffer[row])++ = 0;
        #else
        memset(PCD8544_buffer[row], 0, PCD8544_DISPLAY_WIDTH);
        #endif
    }
    PCD8544[module].pixel.y = PCD8544[module].pixel.y - (8 * bytes);
}

#endif

///	--------------------------------------------------------------------
/// Print functions
///	--------------------------------------------------------------------

#ifdef PCD8544SETFONT
void PCD8544_setFont(u8 module, const u8 *font)
{
    PCD8544[module].font.address   = font;
    PCD8544[module].font.width     = font[FONT_WIDTH];
    PCD8544[module].font.height    = font[FONT_HEIGHT];
    PCD8544[module].font.firstChar = font[FONT_FIRST_CHAR];
    PCD8544[module].font.charCount = font[FONT_CHAR_COUNT];
}
#endif

#if defined(PCD8544PRINTCHAR)   || defined(PCD8544PRINT)      || \
    defined(PCD8544PRINTNUMBER) || defined(PCD8544PRINTFLOAT) || \
    defined(PCD8544PRINTLN)     || defined(PCD8544PRINTF)

void printChar(u8 c)
{
    PCD8544_printChar(PCD8544_SPI, c);
}

void PCD8544_printChar(u8 module, u8 c)
{
    u8  x, y;
    u8  i, j, k, h;
    u8  dat, page, tab;
    u8  width = 0;
    u8  bytes = (PCD8544[module].font.height + 7) / 8;
    u16 index = 0;

    if ((PCD8544[module].pixel.x + PCD8544[module].font.width) > PCD8544[module].screen.width)
    {
        PCD8544[module].pixel.x = 0;
        PCD8544[module].pixel.y = PCD8544[module].pixel.y + (bytes << 3); // *8
    }

    if ((PCD8544[module].pixel.y + PCD8544[module].font.height) > PCD8544[module].screen.height)
    {
        //PCD8544[module].pixel.y = 0;
        PCD8544_scrollUp(module);
    }

    switch (c)
    {
        case '\n':
            PCD8544[module].pixel.y = PCD8544[module].pixel.y + (bytes << 3); // *8
            break;
            
        case '\r':
            PCD8544[module].pixel.x = 0;
            break;
            
        case '\t':
            tab = PCD8544_TABSIZE * PCD8544[module].font.width;
            PCD8544[module].pixel.x += (PCD8544[module].pixel.x + tab) % tab;
            break;
            
        default:
            if (c < PCD8544[module].font.firstChar || c >= (PCD8544[module].font.firstChar + PCD8544[module].font.charCount))
                c = ' ';
            c = c - PCD8544[module].font.firstChar;

            // fixed width font
            if (PCD8544[module].font.address[FONT_LENGTH]==0 && PCD8544[module].font.address[FONT_LENGTH+1]==0)
            {
                width = PCD8544[module].font.width; 
                index = FONT_OFFSET + c * bytes * width;
            }

            // variable width font
            else
            {
                width = PCD8544[module].font.address[FONT_WIDTH_TABLE + c];
                for (i=0; i<c; i++)
                    index += PCD8544[module].font.address[FONT_WIDTH_TABLE + i];
                index = FONT_WIDTH_TABLE + PCD8544[module].font.charCount + index * bytes;
            }

            // save the coordinates
            x = PCD8544[module].pixel.x;
            y = PCD8544[module].pixel.y;

            // draw the character
            for (i=0; i<bytes; i++)
            {
                page = i * width;
                for (j=0; j<width; j++)
                {
                    dat = PCD8544[module].font.address[index + page + j];
                    // if char. takes place on more than 1 line (8 bits)
                    if (PCD8544[module].font.height > 8)
                    {
                        k = ((i+1)<<3);
                        if (PCD8544[module].font.height < k)
                            dat >>= k - PCD8544[module].font.height;
                    }
                    // Write the byte
                    for (h = 0; h < 8; h++)
                    {
                        if (dat & 1)
                            PCD8544_drawPixel(module,  x + j, y + h);
                        else
                            PCD8544_clearPixel(module, x + j, y + h);
                        dat >>= 1;
                    }
                }

                // 1px gap between chars
                for (h = 0; h < 8; h++)
                    PCD8544_clearPixel(module, x + width, y + h);

                // Next part of the current char will be one line under
                y += 8;
            }
            // Next char location
            PCD8544[module].pixel.x = x + width + 1;
            break;
    }
}

#endif

#if defined(PCD8544PRINT)       || defined(PCD8544PRINTLN)    || \
    defined(PCD8544PRINTNUMBER) || defined(PCD8544PRINTFLOAT)
void PCD8544_print(u8 module, const u8 *string)
{
    while (*string != 0)
        PCD8544_printChar(module, *string++);
}
#endif

#if defined(PCD8544PRINTLN)
void PCD8544_println(u8 module, const u8 *string)
{
    PCD8544_print(module, string);
    PCD8544_print(module, (const u8*)"\n\r");
}
#endif

#if defined(PCD8544PRINTCENTER)
void PCD8544_printCenter(u8 module, const u8 *string)
{
    PCD8544[module].pixel.x = (PCD8544[module].screen.width - PCD8544_stringWidth(module, string)) / 2;
    
    // write string
    while (*string != 0)
        PCD8544_printChar(module, *string++);
}

u8 PCD8544_charWidth(u8 module, u8 c)
{
    // fixed width font
    if (PCD8544[module].font.address[FONT_LENGTH]==0 && PCD8544[module].font.address[FONT_LENGTH+1]==0)
        return (PCD8544[module].font.width + 1); 

    // variable width font
    if (c < PCD8544[module].font.firstChar || c > (PCD8544[module].font.firstChar + PCD8544[module].font.charCount))
        c = ' ';
    c = c - PCD8544[module].font.firstChar;
    return (PCD8544[module].font.address[FONT_WIDTH_TABLE + c] + 1);
}

u16 PCD8544_stringWidth(u8 module, const u8* str)
{
    u16 width = 0;

    while(*str != 0)
        width += PCD8544_charWidth(module, *str++);

    return width;
}
#endif

#if defined(PCD8544PRINTNUMBER) || defined(PCD8544PRINTFLOAT)
void PCD8544_printNumber(u8 module, s32 value, u8 base)
{  
    PCD8544_SPI = module;
    printNumber(printChar, value, base);
}
#endif

#if defined(PCD8544PRINTFLOAT)
void PCD8544_printFloat(u8 module, float number, u8 digits)
{ 
    PCD8544_SPI = module;
    printFloat(printChar, number, digits);
}
#endif

#if defined(PCD8544PRINTF)
void PCD8544_printf(u8 module, const u8 *fmt, ...)
{
    static u8 buffer[25];
    u8 *c=(char*)&buffer;
    va_list	args;

    va_start(args, fmt);
    psprintf2(buffer, fmt, args);
    va_end(args);

    while (*c != 0)
        PCD8544_printChar(module, *c++);
}
#endif

///	--------------------------------------------------------------------
/// Graphic functions
///	--------------------------------------------------------------------

//Draw Pixel on the buffer
//Also called from printChar

void PCD8544_drawPixel(u8 module, u8 x, u8 y)
{
    //if (x >= PCD8544[module].screen.width)  return;
    //if (y >= PCD8544[module].screen.height) return;

    //PCD8544_buffer[x + (y >> 3) * PCD8544[module].screen.width] |=  1 << (y % 8);
    //#ifdef __SDCC
    //*(PCD8544_buffer[y >> 3] + x) |= 1 << (y % 8);
    //#else
    PCD8544_buffer[y >> 3][x] |= 1 << (y % 8);
    //#endif
} 

//Clear Pixel on the buffer
//Also called from printChar

void PCD8544_clearPixel(u8 module, u8 x, u8 y)
{
    //if (x >= PCD8544[module].screen.width)  return;
    //if (y >= PCD8544[module].screen.height) return;

    //PCD8544_buffer[x + (y >> 3) * PCD8544[module].screen.width] &=  ~(1 << (y % 8));
    //#ifdef __SDCC
    //*(PCD8544_buffer[y >> 3] + x) &= ~(1 << (y % 8));
    //#else
    PCD8544_buffer[y >> 3][x] &= ~(1 << (y % 8));
    //#endif
} 


#ifdef PCD8544GRAPHICS

// defined as extern void drawPixel(u16 x, u16 y); in graphics.c
// *********************************************************************
void drawPixel(u16 x, u16 y)
{
    PCD8544_drawPixel(PCD8544_SPI, (u8)x, (u8)y);
}
// *********************************************************************

/*
u8 PCD8544_getPixel(u8 module, u8 x, u8 y)
{
    if (x >= PCD8544[module].screen.width)  return 0;
    if (y >= PCD8544[module].screen.height) return 0;

    return (PCD8544_buffer[x+ (y>>3) * PCD8544[module].screen.width] >> (y%8)) & 0x1;  
}
*/

void PCD8544_drawLine(u8 module, u8 x0, u8 y0, u8 x1, u8 y1)
{
    PCD8544_SPI = module;
    drawLine(x0, y0, x1, y1);
}

void PCD8544_drawRect(u8 module, u8 x1, u8 y1, u8 x2, u8 y2)
{
    PCD8544_SPI = module;
    drawRect(x1, y1, x2, y2);
}

void PCD8544_drawRoundRect(u8 module, u8 x1, u8 y1, u8 x2, u8 y2)
{
    PCD8544_SPI = module;
    drawRoundRect(x1, y1, x2, y2);
}

void PCD8544_drawCircle(u8 module, u8 x, u8 y, u8 radius)
{
    PCD8544_SPI = module;
    drawCircle(x, y, radius);
}

void PCD8544_fillCircle(u8 module, u8 x, u8 y, u8 radius)
{
    PCD8544_SPI = module;
    fillCircle(x, y, radius);
}

void PCD8544_fillRect(u8 module, u8 x1, u8 y1, u8 x2, u8 y2)
{
    PCD8544_SPI = module;
    fillRect(x1, y1, x2, y2);
}

void PCD8544_fillRoundRect(u8 module, u8 x1, u8 y1, u8 x2, u8 y2)
{
    PCD8544_SPI = module;
    fillRoundRect(x1, y1, x2, y2);
}

#ifdef  _PCD8544_USE_BITMAP
void PCD8544_drawBitmap(u8 module1, u8 module2, const u8* filename, u16 x, u16 y)
{
    PCD8544_SPI = module1;
    drawBitmap(module2, filename, x, y);
}
/*
void PCD8544_drawBitmap(u8 x, u8 y, const u8 *bitmap,u8 w, u8 h, u8 color)
{
    u8 i, j, byteWidth;
    byteWidth= (w + 7) / 8;

    for(j=0; j<h; j++) 
      {
          for(i=0; i<w; i++ ) 
          {
                  if((bitmap[(j * byteWidth + i / 8)]) & (128 >> (i & 7))) 
                  {
                    PCD8544_drawPixel(x+i, y+j, color);
                  }
          }
      }
}
*/
#endif

#endif //PCD8544GRAPHICS

#ifdef _PCD8544_USE_HOME
void PCD8544_home(u8 module)
{
    PCD8544[module].pixel.x = 0;
    PCD8544[module].pixel.y = 0;
}
#endif

#ifdef _PCD8544_USE_SETCURSOR
void PCD8544_setCursor(u8 module, u8 x, u8 y)
{
    PCD8544[module].pixel.x = x * (PCD8544[module].font.width+1);
    PCD8544[module].pixel.y = y * (PCD8544[module].font.height+1);
}
#endif

#ifdef _PCD8544_USE_ORIENTATION
void PCD8544_setOrientation(u8 module, u8 x)
{
    PCD8544[module].orientation = (x & 3);
    switch(PCD8544[module].orientation) 
    {
        case 0:
        case 2:
            PCD8544[module].screen.width  = PCD8544_DISPLAY_WIDTH;
            PCD8544[module].screen.height = PCD8544_DISPLAY_HEIGHT;
            PCD8544[module].screen.rows   = PCD8544[module].screen.height / 8;
            break;
        case 1:
        case 3:
            PCD8544[module].screen.width  = PCD8544_DISPLAY_HEIGHT;
            PCD8544[module].screen.height = PCD8544_DISPLAY_WIDTH;
            PCD8544[module].screen.rows   = PCD8544[module].screen.height / 8;
            break;
    }
}
#endif

#ifdef  _PCD8544_USE_INVERT
void PCD8544_invertDisplay(u8 module)
{
    PCD8544_select(module);
    PCD8544_command(module, PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYINVERTED);
    PCD8544_deselect(module);
}

void PCD8544_normalDisplay(u8 module)
{
    PCD8544_select(module);
    PCD8544_command(module, PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL);
    PCD8544_deselect(module);
}
#endif

#endif /* __PCD8544_C */
