/*  ----------------------------------------------------------------------------
    FILE:          lcdi2c.h
    PROJECT:       Pinguino - http://www.pinguino.cc/
    PURPOSE:       Driving lcd display through i2c pcf8574 i/o expander
    PROGRAMER:     Regis Blanchot <rblanchot@gmail.com>
    FIRST RELEASE: 29 Jul. 2008
    LAST RELEASE:  24 Apr. 2016
    ----------------------------------------------------------------------------
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
    --------------------------------------------------------------------------*/

/*  ----------------------------------------------------------------------------
    ---------- LCD 2x16 (GDM1602A with build-in Samsung KS0066/S6A0069)
    ----------------------------------------------------------------------------

    01 - VSS (GRND)
    02 - VDD (+5V)
    03 - Vo (R = 1KOhm à la masse)
    04 - RS
    05 - RW (reliée à la masse donc RW = 0 = écriture)
    06 - EN
    07 a 10 - D0 a D3 du LCD doivent rester « en l'air » (non connectées ou à la masse).
    11 a 16 - D4 a D7 du LCD sont reliées au PIC ou PCF8574
    15 - LED+ ???
    16 - LED- ???
    --------------------------------------------------------------------------*/

/*  ----------------------------------------------------------------------------
    ---------- PCF8574P
    ----------------------------------------------------------------------------

    +5V       A0        -|o |-        VDD       +5V
    +5V       A1        -|  |-        SDA       pull-up 1K8 au +5V
    +5V       A2        -|  |-        SCL       pull-up 1K8 au +5V
    LCD_BL    P0        -|  |-        INT
    LCD_RS    P1        -|  |-        P7        LCD_D7
    LCD_RW    P2        -|  |-        P6        LCD_D6
    LCD_EN    P3        -|  |-        P5        LCD_D5
    GRND      VSS       -|  |-        P4        LCD_D4

    SYMBOL    PIN    DESCRIPTION                NB
    A0        1      address input 0            adress = 0 1 0 0 A2 A1 A0 0
    A1        2      address input 1            A0, A1 et A2 connected to +5V
    A2        3      address input 2            then address is 01001110 = 0x4E
    P0        4      quasi-bidirectional I/O 0  LCD_BL
    P1        5      quasi-bidirectional I/O 1  LCD_RS
    P2        6      quasi-bidirectional I/O 2  LCD_RW
    P3        7      quasi-bidirectional I/O 3  LCD_EN
    VSS       8      supply ground              VSS
    P4        9      quasi-bidirectional I/O 4  LCD_D4
    P5        10     quasi-bidirectional I/O 5  LCD_D5
    P6        11     quasi-bidirectional I/O 6  LCD_D6
    P7        12     quasi-bidirectional I/O 7  LCD_D7
    INT       13     interrupt output (active LOW)
    SCL       14     serial clock line          uC_SCL
    SDA       15     serial data line           uC_SDA
    VDD       16     supply voltage             VDD
    --------------------------------------------------------------------------*/

#ifndef __LCDI2C_H
#define __LCDI2C_H

#include <typedef.h>
#include <const.h>              // I2C1, I2C2

#define LCD_MASK                0b11110000              // we only use D7 to D4

#define LCD_WRITE               0
#define LCD_READ                1
#define LCD_DATA                1
#define LCD_CMD                 0

#define LCD_DISPLAY_CLEAR       0b00000001     // 0x01
#define LCD_CURSOR_HOME         0b00000010     // 0x02
#define LCD_ENTRY_MODE_SET      0b00000110     // 0x06 Increment + Display not shifted
// Display : 00001DCB  ; D=Display, C=Cursor et B=Blinking
#define LCD_DISPLAY_ON          0b00001100     // 0x0C Display ON + Cursor OFF + Blinking OFF
#define LCD_DISPLAY_OFF         0b00001000     // 0x08 Display OFF + Cursor OFF + Blinking OFF
#define LCD_SYSTEM_SET_4BITS    0b00101000     // 0x28 Mode 4 bits - 2 lignes - 5x7
#define LCD_SYSTEM_SET_8BITS    0b00111000     // 0x38 Mode 8 bits - 2 lignes - 5x7
#define LCD_ADRESS_LINE1        0b10000000     // DB7=1 pour adresser DD-RAM + Adresse Ligne 1 (0x00) = 0x80
#define LCD_ADRESS_LINE2        0b11000000     // DB7=1 pour adresser DD-RAM + Adresse Ligne 2 (0x40) = 0xC0
#define LCD_ADRESS_NUMBER       0b00110000     // Adresse RAM du code ASCII pour les chiffres
#define LCD_ADRESS_CGRAM        0b01000000     // 0x40 Adresse CGRAM

// commands
#define LCD_CLEARDISPLAY        0x01
#define LCD_RETURNHOME          0x02
#define LCD_ENTRYMODESET        0x04
#define LCD_DISPLAYCONTROL      0x08
#define LCD_CURSORSHIFT         0x10
#define LCD_FUNCTIONSET         0x20
#define LCD_SETCGRAMADDR        0x40
#define LCD_SETDDRAMADDR        0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT          0x00
#define LCD_ENTRYLEFT           0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON           0x04
#define LCD_DISPLAYOFF          0x00
#define LCD_CURSORON            0x02
#define LCD_CURSOROFF           0x00
#define LCD_BLINKON             0x01
#define LCD_BLINKOFF            0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE         0x08
#define LCD_CURSORMOVE          0x00
#define LCD_MOVERIGHT           0x04
#define LCD_MOVELEFT            0x00

// flags for function set
#define LCD_8BITMODE            0x10
#define LCD_4BITMODE            0x00
#define LCD_2LINE               0x08
#define LCD_1LINE               0x00
#define LCD_5x10DOTS            0x04
#define LCD_5x8DOTS             0x00

#define LCD_CENTER              101
#define LCD_RIGHT               102
#define LCD_LEFT                103

#define DEGREE                  0b11011111     // ASCII for degree
#define SIGMA                   0b11100101     // ASCII for sigma
#define MICRO                   0b11100100     // ASCII for micro
#define INFINITE                0b11110011     // ASCII for infinite
#define SPACE                   0x20           // ASCII for space

/*
#define HEXADECIMAL             16
#define DECIMAL                 10
#define OCTAL                   8
#define BINAIRE                 2
*/

#define ACIRC                   0        // â
#define AGRAVE                  1        // à

#define CCEDIL                  2        // ç

#define EACUTE                  3        // é
#define EGRAVE                  4        // è
#define ECIRC                   5        // ê
#define ETREMA                  6        // ë
#define EURO                    7        // €

#define ICIRC                   8        // î
#define ITREMA                  9        // ï

#define OCIRC                   10        // ô

#define UGRAVE                  11        // ù
#define UCIRC                   12        // û

/// VARIABLES GLOBALES

/*
extern const u8 car0[8];
extern const u8 car1[8];
extern const u8 car2[8];
extern const u8 car3[8];
extern const u8 car4[8];
extern const u8 car5[8];
extern const u8 car6[8];
extern const u8 car7[8];
extern const u8 car8[8];
extern const u8 car9[8];
extern const u8 car10[8];
extern const u8 car11[8];
extern const u8 car12[8];
*/
typedef struct
{
    u8 en;
    u8 rw;
    u8 rs;
    u8 d4;
    u8 d5;
    u8 d6;
    u8 d7;
    u8 bl;
} PinOut_t;

typedef struct
{
    u8 width;           // from 0 to 15 = 16
    u8 height;          // from 0 to 1 = 2
    u8 backlight;       // backlight status
    u8 polarity;        // backlight polarity
    u8 address;         // PCF8574_address;
    u8 data;
    PinOut_t pin;
} LCDI2C_t;

/// PROTOTYPES

// private
static void lcdi2c_send4(u8, u8, u8);
static void lcdi2c_send8(u8, u8, u8);

// public
void lcdi2c_init(u8, u8, u8, u8, u8, u8, u8, u8, u8, u8, u8, u8, u8);
void lcdi2c_clearLine(u8, u8);
void lcdi2c_setCursor(u8, u8, u8);
void lcdi2c_printChar(u8, u8);
void lcdi2c_print(u8, const u8 *);
//void lcdi2c_println(u8, const u8 *);
void lcdi2c_printCenter(u8, const u8 *);
void lcdi2c_printNumber(u8, s32, u8);
void lcdi2c_printFloat(u8, float, u8);

#if defined(LCDI2CPRINTF)
//void lcdi2c_printf(u8, char*, ...);
void lcdi2c1_printf(char*, ...);
void lcdi2c2_printf(char*, ...);
#endif
void lcdi2c_newchar(u8, const u8 *, u8);
//void lcdi2c_newpattern(u8);

#define lcdi2c1_init(a,b,c,d,e,f,g,h)   lcdi2c_init(I2C1, a,b,c,d,e,f,g,h)
#define lcdi2c1_clearLine(x)            lcdi2c_clearLine(I2C1, x)
#define lcdi2c1_setCursor(x, y)         lcdi2c_setCursor(I2C1, x, y)
#define lcdi2c1_printChar(x)            lcdi2c_printChar(I2C1, x)
#define lcdi2c1_print(s)                lcdi2c_print(I2C1, s)
//#define lcdi2c1_println(s)            lcdi2c_println(I2C1, s)
#define lcdi2c1_printCenter(s)          lcdi2c_printCenter(I2C1, s)
#define lcdi2c1_printNumber(x, y)       lcdi2c_printNumber(I2C1, x, y)
#define lcdi2c1_printFloat(x, y)        lcdi2c_printFloat(I2C1, x, y)
#define lcdi2c1_newchar(s, x)           lcdi2c_newchar(I2C1, s, x)
//#define lcdi2c1_newpattern()          lcdi2c_newpattern(I2C1)

#define lcdi2c2_init(a,b,c,d,e,f,g,h)   lcdi2c_init(I2C2, a,b,c,d,e,f,g,h)
#define lcdi2c2_clearLine(x)            lcdi2c_clearLine(I2C2, x)
#define lcdi2c2_setCursor(x, y)         lcdi2c_setCursor(I2C2, x, y)
#define lcdi2c2_printChar(x)            lcdi2c_printChar(I2C2, x)
#define lcdi2c2_print(s)                lcdi2c_print(I2C2, s)
//#define lcdi2c2_println(s)            lcdi2c_println(I2C2, s)
#define lcdi2c2_printCenter(s)          lcdi2c_printCenter(I2C2, s)
#define lcdi2c2_printNumber(x, y)       lcdi2c_printNumber(I2C2, x, y)
#define lcdi2c2_printFloat(x, y)        lcdi2c_printFloat(I2C2, x, y)
#define lcdi2c2_newchar(s, x)           lcdi2c_newchar(I2C2, s, x)
//#define lcdi2c2_newpattern()          lcdi2c_newpattern(I2C2)

#if defined(LCDI2CBACKLIGHT) || defined (LCDI2CNOBACKLIGHT)
#define lcdi2c1_backlight()            lcdi2c_backlight(I2C1)
#define lcdi2c2_backlight()            lcdi2c_backlight(I2C2)
#define lcdi2c1_noBacklight()          lcdi2c_noBacklight(I2C1)
#define lcdi2c2_noBacklight()          lcdi2c_noBacklight(I2C2)
#endif

#if defined(LCDI2CCLEAR)
#define lcdi2c_clearScreen(m)           do { lcdi2c_send8(m, LCD_DISPLAY_CLEAR, LCD_CMD); Delayms(2); } while(0)
#define lcdi2c1_clearScreen()           do { lcdi2c_send8(I2C1, LCD_DISPLAY_CLEAR, LCD_CMD); Delayms(2); } while(0)
#define lcdi2c2_clearScreen()           do { lcdi2c_send8(I2C2, LCD_DISPLAY_CLEAR, LCD_CMD); Delayms(2); } while(0)
#endif

#if defined(LCDI2CHOME)
#define lcdi2c_home(m)                  do { lcdi2c_send8(m, LCD_CURSOR_HOME, LCD_CMD); Delayms(2); } while(0)
#define lcdi2c1_home()                  do { lcdi2c_send8(I2C1, LCD_CURSOR_HOME, LCD_CMD); Delayms(2); } while(0)
#define lcdi2c2_home()                  do { lcdi2c_send8(I2C2, LCD_CURSOR_HOME, LCD_CMD); Delayms(2); } while(0)
#endif

#if defined(LCDI2CNOAUTOSCROLL)
#define lcdi2c_noAutoscroll(m)          lcdi2c_send8(m, LCD_ENTRYSHIFTDECREMENT, LCD_CMD)
#define lcdi2c1_noAutoscroll()          lcdi2c_send8(I2C1, LCD_ENTRYSHIFTDECREMENT, LCD_CMD)
#define lcdi2c2_noAutoscroll()          lcdi2c_send8(I2C2, LCD_ENTRYSHIFTDECREMENT, LCD_CMD)
#endif

#if defined(LCDI2CAUTOSCROLL)
#define lcdi2c_autoscroll(m)            lcdi2c_send8(m, LCD_ENTRYSHIFTINCREMENT, LCD_CMD)
#define lcdi2c1_autoscroll()            lcdi2c_send8(I2C1, LCD_ENTRYSHIFTINCREMENT, LCD_CMD)
#define lcdi2c2_autoscroll()            lcdi2c_send8(I2C2, LCD_ENTRYSHIFTINCREMENT, LCD_CMD)
#endif

#if defined(LCDI2CRIGHTTOLEFT)
#define lcdi2c_rightToLeft(m)           lcdi2c_send8(m, LCD_ENTRYRIGHT, LCD_CMD)
#define lcdi2c1_rightToLeft()           lcdi2c_send8(I2C1, LCD_ENTRYRIGHT, LCD_CMD)
#define lcdi2c2_rightToLeft()           lcdi2c_send8(I2C2, LCD_ENTRYRIGHT, LCD_CMD)
#endif

#if defined(LCDI2CLEFTTORIGHT)
#define lcdi2c_leftToRight(m)           lcdi2c_send8(m, LCD_ENTRYLEFT, LCD_CMD)
#define lcdi2c1_leftToRight()           lcdi2c_send8(I2C1, LCD_ENTRYLEFT, LCD_CMD)
#define lcdi2c2_leftToRight()           lcdi2c_send8(I2C2, LCD_ENTRYLEFT, LCD_CMD)
#endif

#if defined(LCDI2CSCROLLDISPLAYRIGHT)
#define lcdi2c_scrollDisplayRight(m)    lcdi2c_send8(m, LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT, LCD_CMD)
#define lcdi2c1_scrollDisplayRight()    lcdi2c_send8(I2C1, LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT, LCD_CMD)
#define lcdi2c2_scrollDisplayRight()    lcdi2c_send8(I2C2, LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT, LCD_CMD)
#endif

#if defined(LCDI2CSCROLLDISPLAYLEFT)
#define lcdi2c_scrollDisplayLeft(m)     lcdi2c_send8(m, LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT, LCD_CMD)
#define lcdi2c1_scrollDisplayLeft()     lcdi2c_send8(I2C1, LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT, LCD_CMD)
#define lcdi2c2_scrollDisplayLeft()     lcdi2c_send8(I2C2, LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT, LCD_CMD)
#endif

#if defined(LCDI2CBLINK)
#define lcdi2c_blink(m)                 lcdi2c_send8(m, LCD_BLINKON, LCD_CMD)
#define lcdi2c1_blink()                 lcdi2c_send8(I2C1, LCD_BLINKON, LCD_CMD)
#define lcdi2c2_blink()                 lcdi2c_send8(I2C2, LCD_BLINKON, LCD_CMD)
#endif

#if defined(LCDI2CNOBLINK)
#define lcdi2c_noBlink(m)               lcdi2c_send8(m, LCD_BLINKOFF, LCD_CMD)
#define lcdi2c1_noBlink()               lcdi2c_send8(I2C1, LCD_BLINKOFF, LCD_CMD)
#define lcdi2c2_noBlink()               lcdi2c_send8(I2C2, LCD_BLINKOFF, LCD_CMD)
#endif

#if defined(LCDI2CCURSOR)
#define lcdi2c_cursor(m)                lcdi2c_send8(m, LCD_CURSORON, LCD_CMD)
#define lcdi2c1_cursor()                lcdi2c_send8(I2C1, LCD_CURSORON, LCD_CMD)
#define lcdi2c2_cursor()                lcdi2c_send8(I2C2, LCD_CURSORON, LCD_CMD)
#endif

#if defined(LCDI2CNOCURSOR)
#define lcdi2c_noCursor(m)              lcdi2c_send8(m, LCD_CURSOROFF, LCD_CMD)
#define lcdi2c1_noCursor()              lcdi2c_send8(I2C1, LCD_CURSOROFF, LCD_CMD)
#define lcdi2c2_noCursor()              lcdi2c_send8(I2C2, LCD_CURSOROFF, LCD_CMD)
#endif

#if defined(LCDI2CDISPLAY)
#define lcdi2c_display(m)               lcdi2c_send8(m, LCD_DISPLAYON, LCD_CMD)
#define lcdi2c1_display()               lcdi2c_send8(I2C1, LCD_DISPLAYON, LCD_CMD)
#define lcdi2c2_display()               lcdi2c_send8(I2C2, LCD_DISPLAYON, LCD_CMD)
#endif

#if defined(LCDI2CNODISPLAY)
#define lcdi2c_noDisplay(m)             lcdi2c_send8(m, LCD_DISPLAYOFF, LCD_CMD)
#define lcdi2c1_noDisplay()             lcdi2c_send8(I2C1, LCD_DISPLAYOFF, LCD_CMD)
#define lcdi2c2_noDisplay()             lcdi2c_send8(I2C2, LCD_DISPLAYOFF, LCD_CMD)
#endif

#endif // __LCDI2C_H
