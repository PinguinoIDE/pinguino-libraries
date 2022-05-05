/*  --------------------------------------------------------------------
    FILE:           OLED.c
    PROJECT:        Pinguino
    PURPOSE:        Drive 0.96" 128x64 Oled display (OLED controller)
    PROGRAMER:      Regis Blanchot <rblanchot@gmail.com>
    ----------------------------------------------------------------------------
    CHANGELOG:
    17 Oct. 2013    Regis Blanchot - first release
    25 Mar. 2014    Regis Blanchot - added 8-BIT 68XX/80XX PARALLEL support
    02 Dec. 2016    Regis Blanchot - moved Interfaces #define to const.h
    06 Feb. 2018    Regis Blanchot - added SSH1106 support
                                   - renamed SSD1306 to OLED
    ----------------------------------------------------------------------------
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

#ifndef __OLED_H
#define __OLED_H

#include <typedef.h>

/** --------------------------------------------------------------------
    Display interfaces
    ------------------------------------------------------------------*/
/*
    #define OLED_PMP6800              1
    #define OLED_PMP8080              2
    #define OLED_PORT6800             1
    #define OLED_PORT8080             2 
    #define OLED_I2C1                 1
    #define OLED_I2C2                 2
    #define OLED_SPISW                0
    #define OLED_SPI1                 1
    #define OLED_SPI2                 2
    #define OLED_SPI3                 3
    #define OLED_SPI4                 4
*/

/** --------------------------------------------------------------------
    Display models
    ------------------------------------------------------------------*/

    //#define OLED_SH1106
    //#define OLED_SSD1306
    //#define OLED_SSD1309 // not yet supported
    //#define OLED_SSD1327 // not yet supported
    //#define OLED_SSD1331 // not yet supported

/** --------------------------------------------------------------------
    Display sizes
    ------------------------------------------------------------------*/

    #if defined(OLED_SH1106) && defined(OLED_128X64)
        #undef  OLED_128X64
        #define OLED_132X64
    #endif
    
    #if defined(OLED_SH1106) && defined(OLED_128X32)
        #undef  OLED_128X32
        #define OLED_132X32
    #endif
    
    #if !defined(OLED_96X16)  && !defined(OLED_96X64)  && \
        !defined(OLED_128X32) && !defined(OLED_132X32) && \
        !defined(OLED_128X64) && !defined(OLED_132X64)
        #warning "*** No display size specified  ***"
        #warning "*** Default value used instead ***"
        #if defined(__16f1459)  || \
            defined(__18f13k50) || defined(__18f14k50)
            #define OLED_128X32
        #else
            #define OLED_128X64
        #endif
    #endif

/** --------------------------------------------------------------------
    Display constants
    ------------------------------------------------------------------*/

    #define OLED_TABSIZE              4

    // used in OLED_getColor() only
    #define OLED_DISPLAY_WIDTH_BITS   7
    #define OLED_DISPLAY_WIDTH_MASK   0x7F

    #if defined(OLED_132X64)
        // SH1106 panel is 128 pixels wide,
        // controller RAM has space for 132,
        // it's centered so add an offset to RAM address.
        #define OLED_DISPLAY_WIDTH    128
        #define OLED_RAM_OFFSET       2
        #define OLED_DISPLAY_ROWS     8
        #define OLED_DISPLAY_ROW_BITS 3
        #define OLED_DISPLAY_ROW_MASK 0x07
    #elif defined(OLED_132X32)
        // SH1106 panel is 128 pixels wide,
        // controller RAM has space for 132,
        // it's centered so add an offset to RAM address.
        #define OLED_DISPLAY_WIDTH    128
        #define OLED_RAM_OFFSET       2
        #define OLED_DISPLAY_ROWS     4
        #define OLED_DISPLAY_ROW_BITS 2
        #define OLED_DISPLAY_ROW_MASK 0x03
    #elif defined(OLED_128X64)
        #define OLED_DISPLAY_WIDTH    128
        #define OLED_RAM_OFFSET       0
        #define OLED_DISPLAY_ROWS     8
        #define OLED_DISPLAY_ROW_BITS 3
        #define OLED_DISPLAY_ROW_MASK 0x07
    #elif defined(OLED_128X32)
        #define OLED_DISPLAY_WIDTH    128
        #define OLED_RAM_OFFSET       0
        #define OLED_DISPLAY_ROWS     4
        #define OLED_DISPLAY_ROW_BITS 2
        #define OLED_DISPLAY_ROW_MASK 0x03
    #else
        #error "*** No display size specified  ***"
    #endif

    #define OLED_DISPLAY_HEIGHT       (OLED_DISPLAY_ROWS * 8)
    #define OLED_DISPLAY_SIZE         (OLED_DISPLAY_WIDTH * OLED_DISPLAY_ROWS)
    #define OLED_DISPLAY_HALF_SIZE    (OLED_DISPLAY_SIZE / 2)
    #define OLED_PORTRAIT             100
    #define OLED_LANDSCAPE            101

/** --------------------------------------------------------------------
    Display commands
    ------------------------------------------------------------------*/

    #define OLED_CMD_SINGLE           0x80
    #define OLED_CMD_STREAM           0x00
    #define OLED_DATA_STREAM          0x40

    #define OLED_SETCONTRAST          0x81
    #define OLED_DISPLAYALLON_RESUME  0xA4
    #define OLED_DISPLAYALLON         0xA5
    #define OLED_NORMALDISPLAY        0xA6
    #define OLED_INVERTDISPLAY        0xA7
    #define OLED_DISPLAYOFF           0xAE
    #define OLED_DISPLAYON            0xAF

    #define OLED_SETDISPLAYOFFSET     0xD3
    #define OLED_SETCOMPINS           0xDA

    #define OLED_SETVCOMDETECT        0xDB

    #define OLED_SETDISPLAYCLOCKDIV   0xD5
    #define OLED_SETPRECHARGE         0xD9

    #define OLED_SETMULTIPLEX         0xA8

    #define OLED_SETPAGEADDRESS       0xB0
    #define OLED_SETLOWCOLUMN         0x00
    #define OLED_SETHIGHCOLUMN        0x10

    #define OLED_SETSTARTLINE         0x40

    #define OLED_MEMORYMODE           0x20
    #define OLED_COLUMNADDR           0x21
    #define OLED_PAGEADDR             0x22
    
    #define OLED_COMSCANINC           0xC0
    #define OLED_COMSCANDEC           0xC8

    #define OLED_SEGREMAP             0xA0
    #define OLED_SEGREMAPINV          0xA1

    #define OLED_CHARGEPUMP           0x8D

    #define OLED_EXTERNALVCC          0x1
    #define OLED_SWITCHCAPVCC         0x2

    #define OLED_NOP                  0xE3

    // Scrolling #defines
    #define OLED_ACTIVATE_SCROLL                        0x2F
    #define OLED_DEACTIVATE_SCROLL                      0x2E
    #define OLED_SET_VERTICAL_SCROLL_AREA               0xA3
    #define OLED_RIGHT_HORIZONTAL_SCROLL                0x26
    #define OLED_LEFT_HORIZONTAL_SCROLL                 0x27
    #define OLED_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL   0x29
    #define OLED_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL    0x2A

/** --------------------------------------------------------------------
    Typedef.
    ------------------------------------------------------------------*/

    typedef struct
    {
        u16 x;
        u16 y;
        u16 page;
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
        u8 startx;
        u8 starty;
        u8 endx;
        u8 endy;
        u8 width;
        u8 height;
        u8 invert;
        u8 bcolor;
        u8 color;
        u8 orientation;
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
        rect_t screen;
        //coord_t cursor;
        coord_t pixel;
        font_t font;
    } lcd_t;

/** --------------------------------------------------------------------
    Prototypes
    ------------------------------------------------------------------*/

void OLED_init(u8, ...);
void OLED_sendCommand(u8, u8);
void OLED_sendCommand(u8, u8);

void OLED_refresh(u8);
void OLED_clearScreen(u8);

void OLED_setFont(u8, const u8 *);
void OLED_printChar(u8, u8);
void OLED_printChar2(u8);
void OLED_print(u8, u8 *);
void OLED_println(u8, u8 *);
void OLED_printCenter(u8, u8 *);
u8 OLED_charWidth(u8, u8);
u16 OLED_stringWidth(u8, u8*);
void OLED_printNumber(u8, long, u8);
void OLED_printFloat(u8, float, u8);
void OLED_printf(u8, const u8 *, ...);
void OLED_setXY(u8, u8, u8);
void OLED_setCursor(u8, u8, u8);
void OLED_scrollUp(u8);

u8 OLED_getColor(u8, u8, u8);
void drawPixel(u16, u16);
void OLED_drawPixel(u8, u8, u8);
void OLED_clearPixel(u8, u8, u8);
void OLED_drawLine(u8, u16, u16, u16, u16);
void OLED_drawCircle(u8, u16, u16, u16);
void OLED_fillCircle(u8, u16, u16, u16);
void OLED_drawRect(u8, u16, u16, u16, u16);
void OLED_drawRoundRect(u8, u16, u16, u16, u16);
void OLED_fillRect(u8, u16, u16, u16, u16);
void OLED_fillRoundRect(u8 , u16, u16, u16, u16);
void OLED_drawBitmap(u8, u16, u16, u16, u16, u16*);

/** --------------------------------------------------------------------
    Basic functions
    ------------------------------------------------------------------*/

// public
#define OLED_displayOff(m)                  OLED_sendCommand(m, OLED_DISPLAYOFF)
#define OLED_displayOn(m)                   OLED_sendCommand(m, OLED_DISPLAYON)
#define OLED_sleep(m)                       OLED_sendCommand(m, OLED_DISPLAYOFF)
#define OLED_wake(m)                        OLED_sendCommand(m, OLED_DISPLAYON)
#define OLED_setInverse(m, v)               OLED_sendCommand(m, v ? OLED_INVERTDISPLAY : OLED_NORMALDISPLAY)
#define OLED_invertDisplay(m)               OLED_sendCommand(m, OLED_INVERTDISPLAY)
#define OLED_normalDisplay(m)               OLED_sendCommand(m, OLED_NORMALDISPLAY)
#define OLED_setDisplayOffset(m, v)        {OLED_sendCommand(m, OLED_SETDISPLAYOFFSET); OLED_sendCommand(m, v & 0x3F);}
#define OLED_setContrast(m, v)             {OLED_sendCommand(m, OLED_SETCONTRAST); OLED_sendCommand(m, v);}
#define OLED_getFontWidth(m)               (OLED.font.width)
#define OLED_getFontHeight(m)              (OLED.font.height)
#define OLED_getDisplayWidth(m)            (OLED_DISPLAY_WIDTH)
#define OLED_getDisplayHeight(m)           (OLED_DISPLAY_HEIGHT)

// private
#define OLED_setDisplayStartLine(m, v)      OLED_sendCommand(m, OLED_SETSTARTLINE | v)
#define OLED_setSegmentRemap(m, v)          OLED_sendCommand(m, v ? OLED_SEGREMAPINV : OLED_SEGREMAP)
#define OLED_setMultiplexRatio(m, v)       {OLED_sendCommand(m, OLED_SETMULTIPLEX); OLED_sendCommand(m, v & 0x3F);}
#define OLED_setDirection(m,v)              OLED_sendCommand(m, v ? OLED_COMSCANDEC : OLED_COMSCANINC)
#define OLED_pamSetStartAddress(m, a)      {OLED_sendCommand(m, a & 0x0F); OLED_sendCommand(m, (a << 4) & 0x0F);}
#define OLED_setMemoryAddressingMode(m,mo) {OLED_sendCommand(m, OLED_MEMORYMODE); OLED_sendCommand(m, mo & 0x3);}
#define OLED_hvSetColumnAddress(m,s,e)     {OLED_sendCommand(m, OLED_COLUMNADDR); OLED_sendCommand(m, s & OLED_DISPLAY_WIDTH_MASK); OLED_sendCommand(m, e & OLED_DISPLAY_WIDTH_MASK);}
#define OLED_hvSetPageAddress(m,s,e)       {OLED_sendCommand(m, OLED_PAGEADDR); OLED_sendCommand(m, s & OLED_DISPLAY_ROW_MASK); OLED_sendCommand(m, e & OLED_DISPLAY_ROW_MASK);}
#define OLED_pamSetPageStart(m, a)          OLED_sendCommand(m, OLED_SETPAGEADDRESS | (a & OLED_DISPLAY_ROW_MASK))
#define OLED_setPrechargePeriod(m,p1,p2)   {OLED_sendCommand(m, OLED_SETPRECHARGE); OLED_sendCommand(m, (p1 & 0x0F) | ((p2 & 0x0F ) << 4));}
#define OLED_setVcomhDeselectLevel(m,l)    {OLED_sendCommand(m, OLED_SETVCOMDETECT); OLED_sendCommand(m, (l & 0x03) << 4);}
#define OLED_nop(m)                         OLED_sendCommand(m, OLED_NOP)
#define OLED_setChargePumpEnable(m, e)     {OLED_sendCommand(m, OLED_CHARGEPUMP); OLED_sendCommand(m, e ? 0x14 : 0x10);}

//#define OLED_stopScroll(m)                  OLED_sendCommand(m, OLED_DEACTIVATE_SCROLL)
//#define OLED_scrollRight(m)                 OLED_startHorizontalScroll(m, OLED_RIGHT_HORIZONTAL_SCROLL, 0, 7, 2)
//#define OLED_scrollLeft(m)                  OLED_startHorizontalScroll(m, OLED_LEFT_HORIZONTAL_SCROLL, 0, 7, 2)
//#define OLED_scrollDown(m)                  OLED_startVerticalAndHorizontalScroll(m, OLED_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL, 0, 64, 0, 1)

#define OLED_setDisplayClock(m, f, r)      {OLED_sendCommand(m, OLED_SETDISPLAYCLOCKDIV); OLED_sendCommand(m, ((f & 0x0F) << 4) | (r & 0x0F));}
#define OLED_setComPinsHardwareConfiguration(m, sequential, lr_remap) {OLED_sendCommand(m, OLED_SETCOMPINS); OLED_sendCommand(m, 0x02 | ((sequential & 1) << 4) | ((lr_remap & 1) << 5));}
#define OLED_setPageAddress(m,i)            OLED_sendCommand(m, OLED_SETPAGEADDRESS | (i & 0x0F))
#define OLED_setLowColumn(m,x)              OLED_sendCommand(m, OLED_SETLOWCOLUMN   | (x & 0x0F))
#define OLED_setHighColumn(m,x)             OLED_sendCommand(m, OLED_SETHIGHCOLUMN  | (x >> 4))

/** --------------------------------------------------------------------
    Globals
    ------------------------------------------------------------------*/

// One display at a time
lcd_t OLED;

/*
#if defined(__18f47j53)
// SPISW, SPI1, SPI2, I2C, PMP6800, PMP8080, PORTD
lcd_t OLED[7];
#else
// SPISW, SPI1, I2C, PMP6800, PMP8080, PORTD
lcd_t OLED[6];
#endif
*/

/** --------------------------------------------------------------------
    Pin Assignment
                                ----------PINGUINO 47J53----------
            OLED             6800 w/PMP              6800 wo/PMP
    1       VCC (3 to 6V)       VIN                     VIN
    2       GND                 GND                     GND
    3       CS                  GND                     GND
    4       RES                             D3          D3
    5       D/C                 PMA1/B4     D4          D4
    6       R/W                 PMRD/E0     D13         D5
    7       E/RD                GND         GND         D6
    8       D0                  PMD0/D0     D24         D24
    9       D1                  PMD1        D25         D25
    10      D2                  PMD2        D26         D26
    11      D3                  PMD3        D27         D27
    12      D4                  PMD4        D28         D28
    13      D5                  PMD5        D29         D29
    14      D6                  PMD6        D30         D30
    15      D7                  PMD7        D31         D31
    16      NC
     
                                PMBE        D1
    ------------------------------------------------------------------*/

    //#define swap(i, j) {int t = i; i = j; j = t;}

    #if !defined(__PIC32MX__)
    #define low(x)              digitalwrite(x, LOW)
    #define high(x)             digitalwrite(x, HIGH)
    #endif
    /*
    #define Low(x)      do { __asm bcf _LATB,x  __endasm; } while(0)
    #define High(x)     do { __asm bsf _LATB,x  __endasm; } while(0)
    #define Output(x)   do { __asm bcf _TRISB,x __endasm; } while(0)
    #define Input(x)    do { __asm bsf _TRISB,x __endasm; } while(0)
    */

    #define DATA                LATD    // RD0 to RD7
    #define dDATA               TRISD
    #define CMD                 LATB    // RB0 to RB7
    #define dCMD                TRISB

#endif /* __OLED_H */
