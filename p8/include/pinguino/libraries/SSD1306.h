/*	--------------------------------------------------------------------
	FILE:			SSD1306.c
	PROJECT:		pinguino
	PURPOSE:		Drive 0.96" 128x64 Oled display (SSD1306 controller)
	PROGRAMER:		regis blanchot <rblanchot@gmail.com>
	FIRST RELEASE:	17 Oct. 2013
	LAST RELEASE:	25 Mar. 2014
	----------------------------------------------------------------------------
    * Done : 8-BIT 68XX/80XX PARALLEL
    * Todo : 3-/4-WIRE SPI
    * Todo : I2C
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

#ifndef __SSD1306_H
#define __SSD1306_H

#include <typedef.h>

/**	--------------------------------------------------------------------
    Display interfaces
    ------------------------------------------------------------------*/

#if defined(SSD1306USEPMP6800) || defined(SSD1306USEPMP8080)
#if defined(__18f46j53) || defined(__18f47j53) || defined(__PIC32MX__)
    #include <pmp.h>
    #define SSD1306_PMP6800             1    // Parallel port
    #define SSD1306_PMP8080             2    // Parallel port
#endif
#endif

#if defined(SSD1306USEPORT6800) || defined(SSD1306USEPORT8080)
    #define SSD1306_PORT6800            1
    #define SSD1306_PORT8080            2 
#endif

#if defined(SSD1306USEI2C1) || defined(SSD1306USEI2C2)
    #include <i2c.h>
    #define SSD1306_I2C1                I2C1
    #define SSD1306_I2C2                I2C2
#endif

#if defined(SSD1306USESPISW) ||defined(SSD1306USESPI1) ||defined(SSD1306USESPI2)
    #include <spi.h>
    #define SSD1306_SPISW               SPISW
    #define SSD1306_SPI1                SPI1
    #define SSD1306_SPI2                SPI2
    #ifdef __PIC32MX__
    #define SSD1306_SPI3                SPI3
    #define SSD1306_SPI4                SPI4
    #endif
#endif

/**	--------------------------------------------------------------------
    Display sizes
    ------------------------------------------------------------------*/

    //#define SSD1306_128X32              1
    #define SSD1306_128X64              2
    
/**	--------------------------------------------------------------------
    Display commands
    ------------------------------------------------------------------*/

    #define SSD1306_CMD_SINGLE          0x80
    #define SSD1306_CMD_STREAM          0x00
    #define SSD1306_DATA_STREAM         0x40

    #define SSD1306_SETCONTRAST         0x81
    #define SSD1306_DISPLAYALLON_RESUME 0xA4
    #define SSD1306_DISPLAYALLON        0xA5
    #define SSD1306_NORMALDISPLAY       0xA6
    #define SSD1306_INVERTDISPLAY       0xA7
    #define SSD1306_DISPLAYOFF          0xAE
    #define SSD1306_DISPLAYON           0xAF

    #define SSD1306_SETDISPLAYOFFSET    0xD3
    #define SSD1306_SETCOMPINS          0xDA

    #define SSD1306_SETVCOMDETECT       0xDB

    #define SSD1306_SETDISPLAYCLOCKDIV  0xD5
    #define SSD1306_SETPRECHARGE        0xD9

    #define SSD1306_SETMULTIPLEX        0xA8

    #define SSD1306_SETLOWCOLUMN        0x00
    #define SSD1306_SETHIGHCOLUMN       0x10

    #define SSD1306_SETSTARTLINE        0x40

    #define SSD1306_MEMORYMODE          0x20

    #define SSD1306_COMSCANINC          0xC0
    #define SSD1306_COMSCANDEC          0xC8

    #define SSD1306_SEGREMAP            0xA0
    #define SSD1306_SEGREMAPINV         0xA1

    #define SSD1306_CHARGEPUMP          0x8D

    #define SSD1306_EXTERNALVCC         0x1
    #define SSD1306_SWITCHCAPVCC        0x2

    // Scrolling #defines
    #define SSD1306_ACTIVATE_SCROLL     0x2F
    #define SSD1306_DEACTIVATE_SCROLL   0x2E
    #define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3
    #define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26
    #define SSD1306_LEFT_HORIZONTAL_SCROLL 0x27
    #define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
    #define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A

/**	--------------------------------------------------------------------
    Display constants
    ------------------------------------------------------------------*/

    #define DISPLAY_WIDTH               128
    #define DISPLAY_WIDTH_BITS          7
    #define DISPLAY_WIDTH_MASK          0x7F
    
    #ifdef SSD1306_128X64
        #define DISPLAY_ROWS            8
        #define DISPLAY_ROW_BITS        3
        #define DISPLAY_ROW_MASK        0x07
    #else // SSD1306_128X32
        #define DISPLAY_ROWS            4
        #define DISPLAY_ROW_BITS        2
        #define DISPLAY_ROW_MASK        0x03
    #endif

    #define DISPLAY_HEIGHT              (DISPLAY_ROWS * 8)
    #define DISPLAY_SIZE                (DISPLAY_WIDTH * DISPLAY_ROWS)
    #define PORTRAIT                    100
    #define LANDSCAPE                   101

    // only 2 colors available on these type of screen
    #define Blue                        0x001F
    #define Yellow                      0xFFE0

/**	--------------------------------------------------------------------
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
        u8 width;
        u8 height;
        const u8 *address;
    } font_t;

    typedef struct
    {
        u8 r;			// 8/8/8 representation
        u8 g;
        u8 b;
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
        rect_t screen;
        color_t bcolor;
        color_t color;
        coord_t cursor;
        font_t font;
    } lcd_t;

/**	--------------------------------------------------------------------
    Prototypes
    ------------------------------------------------------------------*/

void SSD1306_init(u16, ...);
void SSD1306_sendCommand(u16, u8);
void SSD1306_sendData(u16, u8);
void SSD1306_displayOff(u16);
void SSD1306_displayOn(u16);
void SSD1306_sleep(u16);
void SSD1306_wake(u16);
void SSD1306_setInverse(u16, u8);
void SSD1306_setDisplayOffset(u16, u8);
void SSD1306_setContrast(u16, u8);
void SSD1306_setDisplayStartLine(u16, u8);
void SSD1306_setSegmentRemap(u16, u8);
void SSD1306_setMultiplexRatio(u16, u8);
void SSD1306_setComOutputScanDirection(u16, u8);
void SSD1306_setComPinsHardwareConfiguration(u16, u8, u8);
void SSD1306_startHorizontalScroll(u16, u8, u8, u8, u16); 
void SSD1306_startVerticalAndHorizontalScroll(u16, u8, u8, u8, u16, u8);
void SSD1306_stopScroll(u16);
void SSD1306_pamSetStartAddress(u16, u8);
void SSD1306_setMemoryAddressingMode(u16, u8);
void SSD1306_hvSetColumnAddress(u16, u8, u8);
void SSD1306_hvSetPageAddress(u16, u8, u8);
void SSD1306_pamSetPageStart(u16, u8);
void SSD1306_setDisplayClockRatioAndFrequency(u16, u8, u8);
void SSD1306_setPrechargePeriod(u16, u8, u8);
void SSD1306_setVcomhDeselectLevel(u16, u8);
void SSD1306_nop(u16);
void SSD1306_setChargePumpEnable(u16, u8);
void SSD1306_refresh(u16);
void SSD1306_clearScreen(u16);
void SSD1306_scrollRight(u16);
void SSD1306_scrollLeft(u16);
void SSD1306_scrollUp(u16);
void SSD1306_scrollDown(u16);
void SSD1306_setFont(u16, const u8 *);
void SSD1306_printChar(u16, u8);
void SSD1306_printChar2(u8);
void SSD1306_print(u16, u8 *);
void SSD1306_println(u16, u8 *);
void SSD1306_printNumber(u16, long, u8);
void SSD1306_printFloat(u16, float, u8);
void SSD1306_printf(u16, const u8 *, ...);
void SSD1306_setCursor(u16, u8, u8);
void SSD1306_drawPixel(u16, u8, u8);
void SSD1306_clearPixel(u16, u8, u8);
u8 SSD1306_getColor(u16, u8, u8);
void SSD1306_drawBitmap(u16, u16, u16, u16, u16, u16*);
void drawPixel(u16, u16, u16);
void SSD1306_drawCircle(u16, u16, u16, u16);
void SSD1306_fillCircle(u16, u16, u16, u16);
void SSD1306_drawLine(u16, u16, u16, u16, u16);

/**	--------------------------------------------------------------------
    Basic functions
    ------------------------------------------------------------------*/

#define SSD1306_displayOff(module)                  SSD1306_sendCommand(module, 0xAE)
#define SSD1306_displayOn(module)                   SSD1306_sendCommand(module, 0xAF)
#define SSD1306_sleep(module)                       SSD1306_sendCommand(module, 0xAE)
#define SSD1306_wake(module)                        SSD1306_sendCommand(module, 0xAF)
#define SSD1306_setInverse(module, value)           SSD1306_sendCommand(module, value ? 0xA7 : 0xA6)
#define SSD1306_setDisplayOffset(module, value) {   SSD1306_sendCommand(module, 0xD3); SSD1306_sendCommand(module, value & 0x3F);}
#define SSD1306_setContrast(module, value)      {   SSD1306_sendCommand(module, 0x81); SSD1306_sendCommand(module, value);}
#define SSD1306_setDisplayStartLine(module, value)  SSD1306_sendCommand(module, 0x40 | value)
#define SSD1306_setSegmentRemap(module, value)      SSD1306_sendCommand(module, value ? 0xA1 : 0xA0)
#define SSD1306_setMultiplexRatio(module, value){   SSD1306_sendCommand(module, 0xA8); SSD1306_sendCommand(module, value & 0x3F);}
#define SSD1306_setComOutputScanDirection(module, value) SSD1306_sendCommand(module, value ? 0xC8 : 0xC0)
#define SSD1306_setComPinsHardwareConfiguration(module, sequential, lr_remap) {SSD1306_sendCommand(module, 0xDA); SSD1306_sendCommand(module, 0x02 | ((sequential & 1) << 4) | ((lr_remap & 1) << 5));}
#define SSD1306_pamSetStartAddress(module, address){SSD1306_sendCommand(module, address & 0x0F); SSD1306_sendCommand(module, (address << 4) & 0x0F);}
#define SSD1306_setMemoryAddressingMode(module, mode){SSD1306_sendCommand(module, 0x20); SSD1306_sendCommand(module, mode & 0x3);}
#define SSD1306_hvSetColumnAddress(module, start, end){SSD1306_sendCommand(module, 0x21); SSD1306_sendCommand(module, start & DISPLAY_WIDTH_MASK); SSD1306_sendCommand(module, end & DISPLAY_WIDTH_MASK);}
#define SSD1306_hvSetPageAddress(module, start, end){ SSD1306_sendCommand(module, 0x22); SSD1306_sendCommand(module, start & DISPLAY_ROW_MASK); SSD1306_sendCommand(module, end & DISPLAY_ROW_MASK);}
#define SSD1306_pamSetPageStart(module, address)    SSD1306_sendCommand(module, 0xB0 | (address & DISPLAY_ROW_MASK))
#define SSD1306_setDisplayClockRatioAndFrequency(module, ratio, frequency) {SSD1306_sendCommand(module, 0xD5); SSD1306_sendCommand(module, (ratio & 0x0F) | ((frequency & 0x0F) << 4));}
#define SSD1306_setPrechargePeriod(module, phase1, phase2){SSD1306_sendCommand(module, 0xD9); SSD1306_sendCommand(module, (phase1 & 0x0F) | ((phase2 & 0x0F ) << 4));}
#define SSD1306_setVcomhDeselectLevel(module, level){SSD1306_sendCommand(module, 0xDB); SSD1306_sendCommand(module, (level & 0x03) << 4);}
#define SSD1306_nop(module)                         SSD1306_sendCommand(module, 0xE3)
#define SSD1306_setChargePumpEnable(module, enable){SSD1306_sendCommand(module, 0x8D); SSD1306_sendCommand(module, enable ? 0x14 : 0x10);}
#define SSD1306_getFontWidth(m)  (SSD1306.font.width)
#define SSD1306_getFontHeight(m) (SSD1306.font.height)
#define SSD1306_invertDisplay(m) SSD1306_sendCommand(m, SSD1306_INVERTDISPLAY)
#define SSD1306_normalDisplay(m) SSD1306_sendCommand(m, SSD1306_NORMALDISPLAY)

/**	--------------------------------------------------------------------
    Globals
    ------------------------------------------------------------------*/

// One display at a time
lcd_t SSD1306;

/*
#if defined(__18f47j53)
// SPISW, SPI1, SPI2, I2C, PMP6800, PMP8080, PORTD
lcd_t SSD1306[7];
#else
// SPISW, SPI1, I2C, PMP6800, PMP8080, PORTD
lcd_t SSD1306[6];
#endif
*/

/**	--------------------------------------------------------------------
    Pin Assignment
                                ----------PINGUINO 47J53----------
            SSD1306             6800 w/PMP              6800 wo/PMP
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

    #ifndef __PIC32MX__
    #define Low(x)              digitalwrite(x, LOW)
    #define High(x)             digitalwrite(x, HIGH)
    #else
    #define Low(x)              low(x)
    #define High(x)             high(x)
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

#endif /* __SSD1306_H */
