/*  -------------------------------------------------------------------
    FILE:           ST7565.h
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


#ifndef __ST7565_H
#define __ST7565_H

#include <typedef.h>

//#define enablePartialUpdate

#define ST7565_STARTBYTES 1

/**	--------------------------------------------------------------------
    Color definitions
    ------------------------------------------------------------------*/

#define BLACK 1
#define WHITE 0

/**	--------------------------------------------------------------------
    Display size (default orientation is portrait)
    ------------------------------------------------------------------*/

#define ST7565_WIDTH        128
#define ST7565_HEIGHT       64
#define ST7565_SIZE         (ST7565_WIDTH * ST7565_HEIGHT)
#define ST7565_TABSIZE      4

/**	--------------------------------------------------------------------
    Display commands
    ------------------------------------------------------------------*/

#define ST7565_DISPLAY_OFF                 0xAE
#define ST7565_DISPLAY_ON                  0xAF

#define ST7565_SET_DISP_START_LINE         0x40
#define ST7565_SET_PAGE                    0xB0

#define ST7565_SET_COLUMN_UPPER            0x10
#define ST7565_SET_COLUMN_LOWER            0x00

#define ST7565_SET_ADC_NORMAL              0xA0
#define ST7565_SET_ADC_REVERSE             0xA1

#define ST7565_SET_DISP_NORMAL             0xA6
#define ST7565_SET_DISP_REVERSE            0xA7

#define ST7565_SET_ALLPTS_NORMAL           0xA4
#define ST7565_SET_ALLPTS_ON               0xA5
#define ST7565_SET_BIAS_9                  0xA2 
#define ST7565_SET_BIAS_7                  0xA3

#define ST7565_RMW                         0xE0
#define ST7565_RMW_CLEAR                   0xEE
#define ST7565_INTERNAL_RESET              0xE2
#define ST7565_SET_COM_NORMAL              0xC0
#define ST7565_SET_COM_REVERSE             0xC8
#define ST7565_SET_POWER_CONTROL           0x28
#define ST7565_SET_RESISTOR_RATIO          0x20
#define ST7565_SET_VOLUME_FIRST            0x81
#define ST7565_SET_VOLUME_SECOND           0
#define ST7565_SET_STATIC_OFF              0xAC
#define ST7565_SET_STATIC_ON               0xAD
#define ST7565_SET_STATIC_REG              0x0
#define ST7565_SET_BOOSTER_FIRST           0xF8
#define ST7565_SET_BOOSTER_234             0
#define ST7565_SET_BOOSTER_5               1
#define ST7565_SET_BOOSTER_6               3
#define ST7565_NOP                         0xE3
#define ST7565_TEST                        0xF0

/**	--------------------------------------------------------------------
    Typedef.
    ------------------------------------------------------------------*/

    typedef struct
    {
        u16 val;
        u8 v[2];
        struct
        {
            u8 LB;
            u8 HB;
        } byte;
    } byte_t;
    
    typedef struct
    {
        u8 dc;
        u8 cs;
        u8 sda;
        u8 sck;
    } pin_t;

    typedef struct
    {
        u16 x;
        u16 y;
        u16 page;
        u16 xmax;
        u16 ymax;
    } coord_t;

    typedef struct
    {
        const u8 *address;
        u8 width;
        u8 height;
        u8 firstChar;
        u8 charCount;
    } font_t;

    typedef struct
    {
        u8  r;			// 8/8/8 representation
        u8  g;
        u8  b;
        u16 c;			// 5/6/5 representation
    } color_t;

    typedef struct
    {
        u16 startx;
        u16 starty;
        u16 endx;
        u16 endy;
        u16 width;
        u16 height;
    } rect_t;

    typedef union
    {
        u16 w;
        struct
        {
            u8 l8;
            u8 h8;
        };
    } word_t;

    typedef struct
    {
        u8 orientation;
        pin_t pin;
        rect_t screen;
        color_t bcolor;
        color_t color;
        coord_t pixel;
        font_t font;
    } lcd_t;

/**	--------------------------------------------------------------------
    Prototypes
    ------------------------------------------------------------------*/

void ST7565_sendCommand(u8, u8);
void ST7565_sendData(u8, u8);

//void ST7565_init(u8, u8, u8, u8, u8);
void ST7565_init(int module, ...);

void ST7565_setOrientation(u8, s16);
void ST7565_setWindow(u8, u8, u8, u8, u8);

void ST7565_setColor(u8, u16);
void ST7565_setBackgroundColor(u8, u16);
color_t *ST7565_getColor(u8, u8, u8);
color_t *ST7565_packColor(u8, u8, u8);
void ST7565_unpackColor(color_t*);

void ST7565_clearScreen(u8);
void ST7565_clearWindow(u8, u8, u8, u8, u8);

//TODO
//void ST7565_scrollRight();
//void ST7565_scrollLeft();
//void ST7565_scrollUp();
//void ST7565_scrollDown();

void ST7565_setFont(u8, const u8*);
void ST7565_printChar(u8, u8);
void ST7565_print(u8, const u8*);
void ST7565_println(u8, const u8*);
void ST7565_printCenter(u8, const u8*);
u8 ST7565_charWidth(u8, u8);
u16 ST7565_stringWidth(u8, const u8*);
void ST7565_printNumber(u8, long, u8);
void ST7565_printFloat(u8, float, u8);
void ST7565_printf(u8, const u8*, ...);
void ST7565_setCursor(u8, u8, u8);

void ST7565_drawPixel(u8, u8, u8);
void ST7565_clearPixel(u8, u8, u8);
void ST7565_drawBitmap(u8, u8, const u8*, u16, u16);
void ST7565_drawCircle(u8, u16, u16, u16);
void ST7565_fillCircle(u8, u16, u16, u16);
void ST7565_drawLine(u8, u16, u16, u16, u16);
void ST7565_drawVLine(u8, u16, u16, u16);
void ST7565_drawHLine(u8, u16, u16, u16);

void setWindow(u8, u8, u8, u8);
void drawPixel(u16, u16);
void setColor(u8, u8, u8);
void drawVLine(u16, u16, u16);
void drawHLine(u16, u16, u16);
extern void drawBitmap(u8, const u8 *, u16, u16);

#ifdef enablePartialUpdate
static void updateBoundingBox(u8, u8, u8, u8);
#endif

/**	--------------------------------------------------------------------
    Macros
    ------------------------------------------------------------------*/

#define ST7565_getFontWidth(m)   (ST7565.font.width)
#define ST7565_getFontHeight(m)  (ST7565.font.height)
#define ST7565_invertDisplay(m)  ST7565_sendCommand(m, ST7565_SET_DISP_REVERSE)
#define ST7565_normalDisplay(m)  ST7565_sendCommand(m, ST7565_SET_DISP_NORMAL)
#define ST7565_displayOn(m)      ST7565_sendCommand(m, ST7565_DISPLAY_ON)
#define ST7565_displayOff(m)     ST7565_sendCommand(m, ST7565_DISPLAY_OFF)
#define ST7565_reset(m)          ST7565_sendCommand(m, ST7565_INTERNAL_RESET)
#ifndef __PIC32MX__
#define ST7565_low(x)            digitalwrite(x, 0)
#define ST7565_high(x)           digitalwrite(x, 1)
#else
#define ST7565_low(x)            low(x)
#define ST7565_high(x)           high(x)
#endif
#define ST7565_select(m)         SPI_select(m)
#define ST7565_deselect(m)       SPI_deselect(m)

/**	--------------------------------------------------------------------
    Globals
    ------------------------------------------------------------------*/

u8 ST7565_SPI;
lcd_t ST7565;

#endif /* __ST7565_H */
