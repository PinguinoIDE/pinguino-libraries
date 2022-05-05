/*  --------------------------------------------------------------------
    FILE:        oled.c
    PROJECT:     Pinguino
    PURPOSE:     Oled display driver
    PROGRAMER:   Regis blanchot <rblanchot@gmail.com>
    --------------------------------------------------------------------
    http://mbed.org/users/Byrn/code/SSD1306/file/1d9df877c90a/ssd1306.cpp
    --------------------------------------------------------------------
    CHANGELOG
    17 Oct. 2013 - Régis Blanchot - first release (SSD1306 controller)
    23 Mar. 2014 - Régis Blanchot - added 8-BIT 68XX/80XX PARALLEL mode support
    24 Mar. 2014 - Régis Blanchot - added 3-/4-WIRE SPI mode support
    25 Mar. 2014 - Régis Blanchot - added I2C mode support
    05 Feb. 2016 - Régis Blanchot - replaced print functions
                                    by call to the print libraries
    05 Feb. 2016 - Régis Blanchot - added new graphics functions
    16 Jun. 2016 - Régis Blanchot - updated for 32-bit
    30 Nov. 2016 - Régis Blanchot - replaced basic functions with #define
    05 Dec. 2016 - Régis Blanchot - fixed I2C part
    07 Dec. 2016 - Régis Blanchot - optimized (x2) the refresh fonction for I2C
    08 Dec. 2016 - Régis Blanchot - added variable width font support
    12 Dec. 2016 - Régis Blanchot - fixed SPI part
    13 Dec. 2016 - Régis Blanchot - fixed Low RAM PIC support
    22 Nov. 2017 - Régis Blanchot - fixed printCenter to support different fonts
    06 Feb. 2018 - Regis Blanchot - added SSH1106 support
                                  - renamed SSD1306 to OLED
    16 Mar. 2018 - Regis Blanchot - added printx function
    ------------------------------------------------------------------------
    TODO:
    * Manage screen's size in OLED_init
    * Add support to SSD1309, SSD1327, SSD1331 and SH1106 support
    * Rename OLED.xxx functions to Oled.xxx functions
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

/**
Pin Assignment

1   VCC             |   2       GND
3   CS              |   4       RES
5   D/C             |   6       R/W
7   E/RD            |   8       D0 (or SCK)
9   D1 (or SDI)     |   10      D2
11  D3              |   12      D4
13  D5              |   14      D6
15  D7              |   16      NC

**/

/**
Bus Interface

        I2C     6800    8080    4-SPI   3-SPI
BS0     0       0       0       0       1
BS1     1       0       1       0       0
BS2     0       1       1       0       0

**/

#ifndef __OLED_C
#define __OLED_C

// Pinguino standards
#include <typedef.h>
#include <const.h>
#include <macro.h>
#include <stdarg.h>
#include <string.h>         // memset, memcpy
#include <oled.h>

#if !defined(__PIC32MX__)
#include <digitalw.c>
#include <digitalp.c>
#include <delayms.c>
#include <delayus.c>
#else
#include <digitalw.c>
#include <delay.c>
#endif

// Parallel port
#if defined(OLEDUSEPMP6800) || defined(OLEDUSEPMP8080)
#if defined(__18f46j53) || defined(__18f47j53) || defined(__PIC32MX__)
    #include <pmp.c>
#endif
#endif

// I2C
#if defined(OLEDUSEI2C1) || defined(OLEDUSEI2C2)
    #include <i2c.c>
#endif

// SPI
#if defined(OLEDUSESPISW) || \
    defined(OLEDUSESPI1)  || defined(OLEDUSESPI2)
    //defined(OLEDUSESPI3)  || defined(OLEDUSESPI4)
    #include <spi.c>
#endif

// Printf
#ifdef OLEDPRINTF
    #include <printFormated.c>
#endif

// PrintFloat
#if defined(OLEDPRINTFLOAT) || defined(OLEDPRINTX)
    #include <printFloat.c>
#endif

// PrintNumber
#if defined(OLEDPRINTNUMBER) || defined(OLEDPRINTFLOAT) || defined(OLEDPRINTX)
    #include <printNumber.c>
#endif

// Graphics Library
#if defined(OLEDGRAPHICS) || defined(OLEDDRAWBITMAP)
    #ifdef OLEDDRAWBITMAP
    #define DRAWBITMAP
    #endif
    #include <graphics.c>
#endif

///	--------------------------------------------------------------------
/// Globals
///	--------------------------------------------------------------------

// Interface (I2C, SPI, etc...)
u8 gInterface;

// Screen buffers / 1 buffer per line
#if (OLED_DISPLAY_HEIGHT >= 16)
u8 gLine1[OLED_DISPLAY_WIDTH]; // 8
u8 gLine2[OLED_DISPLAY_WIDTH]; // 16
#endif
#if (OLED_DISPLAY_HEIGHT >= 32)
u8 gLine3[OLED_DISPLAY_WIDTH]; // 24
u8 gLine4[OLED_DISPLAY_WIDTH]; // 32
#endif
#if (OLED_DISPLAY_HEIGHT >= 48)
u8 gLine5[OLED_DISPLAY_WIDTH]; // 40
u8 gLine6[OLED_DISPLAY_WIDTH]; // 48
#endif
#if (OLED_DISPLAY_HEIGHT >= 64)
u8 gLine7[OLED_DISPLAY_WIDTH]; // 56
u8 gLine8[OLED_DISPLAY_WIDTH-1]; // 64   *** XC8 bssBank5 issue's workaround ***
#endif

// Buffers pointers
u8 *OLED_buffer[OLED_DISPLAY_ROWS] = {
    #if (OLED_DISPLAY_HEIGHT >= 16)
    gLine1, gLine2,
    #endif
    #if (OLED_DISPLAY_HEIGHT >= 32)
    gLine3, gLine4,
    #endif
    #if (OLED_DISPLAY_HEIGHT >= 48)
    gLine5, gLine6,
    #endif
    #if (OLED_DISPLAY_HEIGHT >= 64)
    gLine7, gLine8
    #endif
};

// Config. (model, size) = [multiplex, displayclockdiv, compins, displayoffset]
// OLED_config[OLED_SH1106][OLED_128X64]  = {0x3F, 0xFF, 0xFF, 0x00};
// OLED_config[OLED_SH1106][OLED_128X32]  = {0x20, 0xFF, 0xFF, 0x0F};
// OLED_config[OLED_SSD1306][OLED_128X64] = {0x3F, 0x80, 0x12, 0xFF};
// OLED_config[OLED_SSD1306][OLED_128X32] = {0x1F, 0x80, 0x02, 0xFF};
// OLED_config[OLED_SSD1306][OLED_96X16]  = {0x0F, 0x60, 0x02, 0xFF};
// OLED_config[OLED_SSD1306][OLED_64X48]  = {0x2F, 0x80, 0x12, 0xFF};
// OLED_config[OLED_SSD1306][OLED_64X32]  = {0x1F, 0x80, 0x12, 0xFF};
// u8 OLED_config[][]
            
// Pins
#if   defined(OLEDUSEI2C1)  || defined(OLEDUSEI2C2)
    u8 OLED_I2CADDR;
#else
    u8 pRST, pDC;
#endif

/// --------------------------------------------------------------------
/// Core functions
/// --------------------------------------------------------------------

/**
        The parallel 6800-series interface consists of :
        * 8 bi-directional data pins (D[7:0])
        * R/W#, D/C#, E and CS#.
        A LOW in R/W# indicates WRITE operation
        A HIGH in R/W# indicates READ operation.
        A LOW in D/C# indicates COMMAND read/write
        A HIGH in D/C# indicates DATA read/write.
        The E input serves as data latch signal while CS# is LOW.
        Data is latched at the falling edge of E signal.

        The parallel 8080-series interface consists of :
        * 8 bi-directional data pins (DB[7:0])
        * RD#, WR#, DC# and CS#.
        A LOW  in DC# indicates COMMAND read/write
        A HIGH in DC# indicates DATA read/write.
        A rising edge of RD# input serves as a data READ latch signal while CS# is kept LOW.
        A rising edge of WR# input serves as a data/command WRITE latch signal while CS# is kept LOW.
**/

void OLED_sendCommand(u8 module, u8 val)
{
    ///-----------------------------------------------------------------

    #if defined(SSD1606USEPORT6800) || defined(SSD1606USEPORT8080)
    // TODO

    ///-----------------------------------------------------------------

    #elif defined(SSD1606USEPMP6800) || defined(SSD1606USEPMP8080)
    #if defined(__18f46j53) || defined(__18f47j53) || defined(__PIC32MX__)
    low(pDC); // COMMAND
    PMP_setAddress(0);
    PMP_write(val);
    #else
    #error "No PMP available for your CPU"
    #endif

    ///-----------------------------------------------------------------

    #elif defined(OLEDUSEI2C1) || defined(OLEDUSEI2C2)
    I2C_start(module);
    //if (I2C_write(module, (OLED_I2CADDR << 1) | I2C_WRITE))
    if (I2C_write(module, OLED_I2CADDR))
    {
        I2C_write(module, OLED_CMD_STREAM); // Co = 0, D/C = 0
        I2C_write(module, val);
    }
    I2C_stop(module);

    ///-----------------------------------------------------------------

    #elif defined(OLEDUSESPISW) ||defined(OLEDUSESPI1) ||defined(OLEDUSESPI2)
    SPI_select(module);
    low(pDC);             // COMMAND read/write
    SPI_write(module, val);
    SPI_deselect(module);

    ///-----------------------------------------------------------------

    #else
    #error "Unknown Protocol of Communication"
    #endif
}

void OLED_sendData(u8 module, u8 val)
{
    ///-----------------------------------------------------------------

    #if defined(SSD1606USEPORT6800) || defined(SSD1606USEPORT8080)
    // TODO

    ///-----------------------------------------------------------------

    #elif defined(SSD1606USEPMP6800) || defined(SSD1606USEPMP8080)
    #if defined(__18f46j53) || defined(__18f47j53) || defined(__PIC32MX__)
    high(pDC);    // DATA
    PMP_setAddress(0);
    PMP_write(val);
    #endif

    ///-----------------------------------------------------------------

    #elif defined(OLEDUSEI2C1) || defined(OLEDUSEI2C2)
    I2C_start(module);
    //if (I2C_write(module, (OLED_I2CADDR << 1) | I2C_WRITE))
    if (I2C_write(module, OLED_I2CADDR))
    {
        I2C_write(module, OLED_DATA_STREAM); // Co = 0, D/C = 1
        I2C_write(module, val);
    }
    I2C_stop(module);

    ///-----------------------------------------------------------------

    #elif defined(OLEDUSESPISW) ||defined(OLEDUSESPI1) ||defined(OLEDUSESPI2)
    SPI_select(module);
    high(pDC);           // DATA read/write
    SPI_write(module, val);
    SPI_deselect(module);

    ///-----------------------------------------------------------------

    #else
    #error "Unknown Protocol of Communication"
    #endif
}

/*  --------------------------------------------------------------------
    DESCRIPTION:
        Initialize the graphical display
    PARAMETERS:
    RETURNS:
    REMARKS:
    --------------------------------------------------------------------
    if MODULE = OLED_PMP6800
            OLED CS#    connected to GND
            OLED RES#   connected to any GPIO (res)
            OLED D/C#   connected to Pinguino PMA[0:15] (pDC)
            OLED W/R#   connected to Pinguino PMRD/PMWR
            OLED E/RD#  connected to GND
            OLED D[7:0] connected to Pinguino PMD[7:0]
            OLED.init(PMP6800, pRST, pDC);
            
    if MODULE = OLED_PMP8080
            OLED CS#    connected to GND
            OLED RES#   connected to any GPIO (res)
            OLED D/C#   connected to Pinguino PMA1
            OLED W/R#   connected to Pinguino PMWR
            OLED E/RD#  connected to GND
            OLED D[7:0] connected to Pinguino PMD[7:0]
            OLED.init(PMP6800, pRST);
            
    if MODULE = OLED_PORTB
            OLED CS#    connected to GND
            OLED RES#   connected to any GPIO
            OLED D/C#   connected to any GPIO
            OLED W/R#   connected to any GPIO
            OLED E/RD#  connected to GND
            OLED D[7:0] connected to Pinguino D[0:7]
            
    if MODULE = OLED_PORTD 
            OLED CS#    connected to GND
            OLED RES#   connected to any GPIO (D0)
            OLED D/C#   connected to any GPIO (D1)
            OLED W/R#   connected to any GPIO (D2)
            OLED E/RD#  connected to GND
            OLED D[7:0] connected to Pinguino D[31:24]
            
    if MODULE = OLED_I2Cx (I2C1, I2C2, ...)
            OLED.init(I2Cx, address);
    
    if MODULE = OLED_SPISW
            OLED.init(SPISW, pSDO, pSCK, pCS, pDC, pRST);
    
    if MODULE = OLED_SPIx (SPI1, SPI2, ...)
            OLED.init(SPIx, pDC, pRST);
    --------------------------------------------------------------------
    Important :
    I didn't add support to the 3-wire SPI mode because in this mode we
    don't use the DC pin and thus we have to send an extra bit to
    simulate this pin. Microchip doesn't provide support to that 9-bit
    SPI mode.
    ------------------------------------------------------------------*/

void OLED_init(u8 module, ...)
{
    #if defined(OLEDUSESPISW)
    u8 pSDO, pSCK, pCS;
    #endif

    va_list args;
    
    gInterface = module;
    
    va_start(args, module); // args points on the argument after module

    ///-----------------------------------------------------------------

    #if defined(SSD1606USEPORT6800) || defined(SSD1606USEPORT8080)
    //void OLED_init(PORT6800, pRST, pDC)
        dDATA = 0x00;           // output
        dCMD  = 0x00;           // output
        DATA  = 0x00;
        CMD   = 0xFF;           // every pin HIGH

    ///-----------------------------------------------------------------

    #elif defined(SSD1606USEPMP6800) || defined(SSD1606USEPMP8080)
    #if defined(__18f46j53) || defined(__18f47j53) || defined(__PIC32MX__)
    //OLED.init(PMP6800, pRST, pDC);
    pRST = va_arg(args, int);
    pDC  = va_arg(args, int);
    high(pRST);
    pinmode(pRST, OUTPUT);              // output
    if   (module == OLED_PMP6800)
    {
        PMP_setMode(PMP_MODE_MASTER1);  // Master mode 1 : PMCS1, PMRD/PMWR, PMENB 
        PMP_setControl(PMRD);           // PMRD-PMWR as WR
    }
    else
    {
        PMP_setMode(PMP_MODE_MASTER2);  // Master mode 2 : PMCS1, PMRD, PMWR
        PMP_setControl(PMWR);           // PMWR as WR
    }
    PMP_setWidth(PMP_MODE_8BIT);        // PMD<7:0>
    PMP_setMux(PMP_MUX_OFF);            // PMA and PMD on separate lines
    PMP_setPolarity(PMP_ACTIVE_LOW);
    PMP_setAddress(pDC);                // pDC
    PMP_autoIncrement(0);
    PMP_setWaitStates(4, 16, 4);        // WR strobe must be at least 60ns
    PMP_init();
    #endif

    ///-----------------------------------------------------------------

    #elif defined(OLEDUSEI2C1) ||defined(OLEDUSEI2C2)
    //OLED.init(I2C, address);
    OLED_I2CADDR = va_arg(args, int);
        #if defined(__18f2550) || defined(__18f4550)
        //I2C_init(module, I2C_MASTER_MODE, I2C_100KHZ); // OK
        I2C_init(module, I2C_MASTER_MODE, I2C_400KHZ); // OK
        #else
        I2C_init(module, I2C_MASTER_MODE, I2C_1MHZ); // OK
        #endif

    ///-----------------------------------------------------------------

    #elif defined(OLEDUSESPI1) ||defined(OLEDUSESPI2)
    //OLED.init(SPIx, pDC, pRST);
    pDC  = va_arg(args, int);       // get the next arg
    pRST = va_arg(args, int);       // get the next arg
    SPI_setMode(module, SPI_MASTER);
    SPI_setDataMode(module, SPI_MODE1);
    #if defined(__PIC32MX__)
    //maximum baud rate possible = FPB/2
    SPI_setClockDivider(module, SPI_PBCLOCK_DIV2);
    #else
    //maximum baud rate possible = FPB = FOSC/4
    SPI_setClockDivider(module, SPI_CLOCK_DIV4);
    #endif
    SPI_begin(module);

    ///-----------------------------------------------------------------

    #elif defined(OLEDUSESPISW)
    //OLED.init(SPISW, pSDO, pSCK, pCS, pDC, pRST);
    pSDO = va_arg(args, int); // get the next arg
    pSCK = va_arg(args, int); // get the next arg
    pCS  = va_arg(args, int); // get the last arg
    pDC  = va_arg(args, int); // get the next arg
    pRST = va_arg(args, int); // get the next arg
    SPI_setBitOrder(module, SPI_MSBFIRST);
    SPI_begin(module, pSDO, pSCK, pCS);

    ///-----------------------------------------------------------------

    #else
    #error "Unknown Interface"
    #endif

    ///-----------------------------------------------------------------

    va_end(args);           // cleans up the list
    
    // default Screen Values

    //OLED.orientation   = PORTRAIT;
    OLED.pixel.x       = 0;
    OLED.pixel.y       = 0;
    OLED.screen.startx = 0;
    OLED.screen.starty = 0;
    OLED.screen.endx   = OLED_DISPLAY_WIDTH  - 1;
    OLED.screen.endy   = OLED_DISPLAY_HEIGHT - 1;
    OLED.screen.width  = OLED_DISPLAY_WIDTH;
    OLED.screen.height = OLED_DISPLAY_HEIGHT;

    /** reset device
    When pRST input is low, the chip is initialized with the following status:
        1. Display is OFF.
        2. 128x64 MUX Display Mode.
        3. Normal segment and display data column address and row address mapping
           (SEG0 mapped to address 00h and COM0 mapped to address 00h).
        4. Shift register data clear in serial interface.
        5. Display start line is set at display RAM address 0.
        6. Column address counter is set at 0.
        7. Normal scan direction of the COM outputs.
        8. Contrast control register is set at 7Fh.
        9. Normal display mode (Equivalent to A4h command)
    **/

    // Reset
    #if !defined(OLEDUSEI2C1) && !defined(OLEDUSEI2C2)
    low(pRST);              // initialized the chip
    Delayus(50);            // for at least 10us
    high(pRST);             // release the reset state
    Delayus(10);            // for at least 2us
    #endif

    //OLED_displayOn(module);
    //Delayms(100);                                       // wait for SEG/COM to be ON

    // Config.
    OLED_displayOff(module);
    OLED_setDisplayClock(module, 8, 0);                 // Fosc+50%, Div. 1
    OLED_setMultiplexRatio(module, 0x3F);               // 1/64 duty
    OLED_setDisplayOffset(module, 0);
    OLED_setDisplayStartLine(module, 0);
    OLED_setChargePumpEnable(module, 1);                  // 1=0x14, 0=0x10
    OLED_setMemoryAddressingMode(module, 0);              // horizontal addressing mode; across then down
    OLED_setSegmentRemap(module, 1);                      // rotate screen 180
    OLED_setComPinsHardwareConfiguration(module, 1, 0); // 0x12
    OLED_setDirection(module, 1);
    OLED_setContrast(module, 0xCF);
    OLED_setPrechargePeriod(module, 0x0F, 0x01);        // 0xF1
    OLED_setVcomhDeselectLevel(module, 1);
    OLED_sendCommand(module, OLED_DISPLAYALLON_RESUME);
    OLED_normalDisplay(module);                           // normal display
    OLED_displayOn(module);
    Delayms(150);                                         // Typically, 150ms delay is recommended to wait

    //OLED_hvSetColumnAddress(module, 0, 127);
    //OLED_hvSetPageAddress(module, 0, 7);

    //OLED_pamSetStartAddress(module, 0);
    //OLED_pamSetPageStart(module, 0);
}

///	--------------------------------------------------------------------
/// Update the display
///	--------------------------------------------------------------------

void OLED_refresh(u8 module)
{
    u8 i, j;

    #if !defined(OLED_132X64) // SH1106 doesn't have these commands
    OLED_hvSetPageAddress(module, 0, OLED_DISPLAY_HEIGHT - 1);
    OLED_hvSetColumnAddress(module, 0, OLED_DISPLAY_WIDTH - 1);
    #else
    u8 x;
    #endif

    #if defined(OLEDUSEI2C1) || defined(OLEDUSEI2C2)

        I2C_start(module);
        //if (I2C_write(module, (OLED_I2CADDR << 1) | I2C_WRITE))
        I2C_write(module, OLED_I2CADDR);
        I2C_write(module, OLED_DATA_STREAM);
        for (i=0; i<OLED_DISPLAY_ROWS; i++)
        {
            for (j=0; j<OLED_DISPLAY_WIDTH; j++)
                #ifdef __SDCC
                I2C_write(module, *(OLED_buffer[i])++);
                #else
                I2C_write(module, OLED_buffer[i][j]);
                #endif
        }
        I2C_stop(module);

    #elif defined(OLEDUSESPISW) ||defined(OLEDUSESPI1) ||defined(OLEDUSESPI2)

        #if !defined(OLED_132X64)
        SPI_select(module);
        high(pDC);                               // DATA
        #endif
        for (i=0; i<OLED_DISPLAY_ROWS; i++)
        {
            #if defined(OLED_132X64)
            OLED_setPageAddress(module, i);
            OLED_setLowColumn(module,  OLED_RAM_OFFSET);  // set 4 lower bits column address
            OLED_setHighColumn(module, OLED_RAM_OFFSET);  // set 4 higher bits column address
            SPI_select(module);
            high(pDC);  // DATA
            #endif
            for (j=0; j<OLED_DISPLAY_WIDTH; j++)
            {
                #ifdef __SDCC
                SPI_write(module, *(OLED_buffer[i])++);
                #else
                SPI_write(module, OLED_buffer[i][j]);
                #endif
            }
        }
        SPI_deselect(module);

    #else

        for (i=0; i<OLED_DISPLAY_ROWS; i++)
        {
            for (j=0; j<OLED_DISPLAY_WIDTH; j++)
                #ifdef __SDCC
                OLED_sendData(module, *(OLED_buffer[i])++);
                #else
                OLED_sendData(module, OLED_buffer[i][j]);
                #endif
        }

    #endif
}

///	--------------------------------------------------------------------
/// Clear the buffers
/// NB : void *memset(void *str, int c, size_t n)
///	--------------------------------------------------------------------

void OLED_clearScreen(u8 module)
{
    u8 i, j;

    for (i=0; i<OLED_DISPLAY_ROWS; i++)
    {
        #if defined(__PIC32MX__) || defined(__XC8__)
        memset( OLED_buffer[i], 0, OLED_DISPLAY_WIDTH);
        #else  
        memset(&OLED_buffer[i], 0, OLED_DISPLAY_WIDTH);
        #endif
    }

    OLED.pixel.x = 0;
    OLED.pixel.y = 0;
}

///	--------------------------------------------------------------------
/// Scroll functions
///	--------------------------------------------------------------------

void OLED_startHorizontalScroll(u8 module, u8 direction, u8 start, u8 end, u16 interval) 
{
    // Before issuing this command the horizontal scroll must be deactivated (2Eh).
    // Otherwise, RAM content may be corrupted.
    // RB : this datasheet recommandation doesn't work
    //OLED_sendCommand(module, OLED_DEACTIVATE_SCROLL);

    OLED_sendCommand(module, direction);// ? 0x27 : 0x26);
    // Dummy byte (???)
    OLED_sendCommand(module, 0x00);
    // Start page
    OLED_sendCommand(module, start & OLED_DISPLAY_ROW_MASK);

    // Time interval between each scroll step
    switch (interval)
    {
        case   2: OLED_sendCommand(module, 0x07); break; // 111b
        case   3: OLED_sendCommand(module, 0x04); break; // 100b
        case   4: OLED_sendCommand(module, 0x05); break; // 101b
        case   5: OLED_sendCommand(module, 0x00); break; // 000b
        case  25: OLED_sendCommand(module, 0x06); break; // 110b
        case  64: OLED_sendCommand(module, 0x01); break; // 001b
        case 128: OLED_sendCommand(module, 0x02); break; // 010b
        case 256: OLED_sendCommand(module, 0x03); break; // 011b
        default:  OLED_sendCommand(module, 0x07); break; // default to 2 frame interval
    }
    
    // End page
    //OLED_sendCommand(module, 0x00);
    OLED_sendCommand(module, end & OLED_DISPLAY_ROW_MASK);

    // ???
    OLED_sendCommand(module, 0x00);
    OLED_sendCommand(module, 0xFF);

    // activate scroll
    OLED_sendCommand(module, OLED_ACTIVATE_SCROLL);
}

void OLED_startVerticalAndHorizontalScroll(u8 module, u8 direction, u8 start, u8 end, u16 interval, u8 vertical_offset)
{
    // Before issuing this command the horizontal scroll must be deactivated (2Eh).
    // Otherwise, RAM content may be corrupted.
    // RB : this datasheet recommandation doesn't work
    //OLED_sendCommand(module, OLED_DEACTIVATE_SCROLL);

    OLED_sendCommand(module, direction); // ? 0x2A : 0x29);
    // Dummy byte (???)
    OLED_sendCommand(module, 0x00);
    // Start page
    OLED_sendCommand(module, start & OLED_DISPLAY_ROW_MASK);

    // Time interval between each scroll step
    switch (interval)
    {
        case   2: OLED_sendCommand(module, 0x07); break; // 111b
        case   3: OLED_sendCommand(module, 0x04); break; // 100b
        case   4: OLED_sendCommand(module, 0x05); break; // 101b
        case   5: OLED_sendCommand(module, 0x00); break; // 000b
        case  25: OLED_sendCommand(module, 0x06); break; // 110b
        case  64: OLED_sendCommand(module, 0x01); break; // 001b
        case 128: OLED_sendCommand(module, 0x02); break; // 010b
        case 256: OLED_sendCommand(module, 0x03); break; // 011b
        default:  OLED_sendCommand(module, 0x07); break; // default to 2 frame interval
    }

    // End page
    OLED_sendCommand(module, end & OLED_DISPLAY_ROW_MASK);

    OLED_sendCommand(module, vertical_offset);    

    // activate scroll
    OLED_sendCommand(module, OLED_ACTIVATE_SCROLL);
}

// Up handed 1-row scroll
// The display is 16 rows tall.
void OLED_scrollUp(u8 module)
{
    u8 i;
    u8 bytes = ((OLED.font.height + 7) / 8);
    u8 lastline = OLED_DISPLAY_ROWS - bytes;

    // Copy line y in Line y-1
    //void *memcpy(void *dest, const void *src, size_t n)
    for (i=0; i<lastline; i++)
        #if defined(__PIC32MX__) || defined(__XC8__)
        memcpy( OLED_buffer[i], OLED_buffer[i + 1], OLED_DISPLAY_WIDTH);
        #else
        memcpy(&OLED_buffer[i], OLED_buffer[i + 1], OLED_DISPLAY_WIDTH);
        #endif

    // Clear the last lines
    //void *memset(void *str, int c, size_t n)
    for (i=lastline; i<OLED_DISPLAY_ROWS; i++)
        #if defined(__PIC32MX__) || defined(__XC8__)
        memset( OLED_buffer[i], 0, OLED_DISPLAY_WIDTH);
        #else
        memset(&OLED_buffer[i], 0, OLED_DISPLAY_WIDTH);
        #endif
    
    OLED.pixel.y = OLED.pixel.y - (8 * bytes);
}

///	--------------------------------------------------------------------
/// Print functions
///	--------------------------------------------------------------------

void OLED_setFont(u8 module, const u8 *font)
{
    OLED.font.address   = font;
    OLED.font.width     = font[FONT_WIDTH];
    OLED.font.height    = font[FONT_HEIGHT];
    OLED.font.firstChar = font[FONT_FIRST_CHAR];
    OLED.font.charCount = font[FONT_CHAR_COUNT];
}

/*  --------------------------------------------------------------------
    DESCRIPTION:
        Sets the cursor to the specified x,y position
    PARAMETERS:
        0 < x < 15
        0 < y < 7
    RETURNS:
    REMARKS:
    TODO:
        check x and y
    ------------------------------------------------------------------*/
    void OLED_setXY(u8 module, u8 x, u8 y)
{
    if ( x >= OLED_DISPLAY_WIDTH || y >= (OLED_DISPLAY_HEIGHT) ) return;

    OLED.pixel.x = x;
    OLED.pixel.y = y;
}


#ifdef OLEDSETCURSOR
void OLED_setCursor(u8 module, u8 x, u8 y)
{
    if ( x >= OLED_DISPLAY_WIDTH || y >= (OLED_DISPLAY_HEIGHT) ) return;

    OLED.pixel.x = x * (OLED.font.width+1);
    OLED.pixel.y = y * (OLED.font.height+1);
}
#endif
/*  --------------------------------------------------------------------
    DESCRIPTION:
        write a char at (OLED.cursor.x, OLED.cursor.y)
    PARAMETERS:
        * c ascii code of the character to print
    RETURNS:
    REMARKS:
    ------------------------------------------------------------------*/

void OLED_printChar2(u8 c)
{
    OLED_printChar(gInterface, c);
}

void OLED_printChar(u8 module, u8 c)
{
    u8  x, y;
    u8  i, j, page;
    u8  dat, tab, k;
    u8  width = 0;
    u8  bytes = (OLED.font.height + 7) / 8;
    u16 index = 0;

    if ((OLED.pixel.x + OLED.font.width) > OLED.screen.width)
    {
        OLED.pixel.x = 0;
        OLED.pixel.y = OLED.pixel.y + bytes * 8; //OLED.font.height;
    }

    if ((OLED.pixel.y + OLED.font.height) > OLED.screen.height)
    {
        OLED_scrollUp(module);            
        //OLED.pixel.y = 0;
    }

    switch (c)
    {
        case '\n':
            OLED.pixel.y = OLED.pixel.y + bytes * 8; //OLED.font.height;
            break;
            
        case '\r':
            OLED.pixel.x = 0;
            break;
            
        case '\t':
            tab = OLED_TABSIZE * OLED.font.width;
            OLED.pixel.x += (OLED.pixel.x + tab) % tab;
            break;
            
        default:
            if (c < OLED.font.firstChar || c >= (OLED.font.firstChar + OLED.font.charCount))
                c = ' ';
            c = c - OLED.font.firstChar;

            // fixed width font
            if (OLED.font.address[FONT_LENGTH] == 0 && OLED.font.address[FONT_LENGTH + 1] == 0)
            {
                width = OLED.font.width; 
                index = FONT_OFFSET + c * bytes * width;
            }

            // variable width font
            else
            {
                width = OLED.font.address[FONT_WIDTH_TABLE + c];
                for (i=0; i<c; i++)
                    index += OLED.font.address[FONT_WIDTH_TABLE + i];
                index = FONT_WIDTH_TABLE + OLED.font.charCount + index * bytes;
            }

            // save the coordinates
            x = OLED.pixel.x;
            y = OLED.pixel.y;

            // draw the character
            for (i=0; i<bytes; i++)
            {
                page = i * width;
                for (j=0; j<width; j++)
                {
                    dat = OLED.font.address[index + page + j];
                    // if char. takes place on more than 1 line (8 bits)
                    if (OLED.font.height > 8)
                    {
                        k = ((i+1)<<3);
                        if (OLED.font.height < k)
                            dat >>= k - OLED.font.height;
                    }
                    // Write the byte
                    #ifdef __SDCC
                    *(OLED_buffer[y >> 3] + x + j) = dat;
                    #else
                    OLED_buffer[y >> 3][x + j] = dat;
                    #endif
                }

                // 1px gap between chars
                #ifdef __SDCC
                *(OLED_buffer[y >> 3] + x + width) = 0;
                #else
                OLED_buffer[y >> 3][x + width] = 0;
                #endif

                // Next part of the current char will be one line under
                y += 8;
            }
            // Next char location
            OLED.pixel.x = x + width + 1;
            break;
    }
}

/*  --------------------------------------------------------------------
    DESCRIPTION:
        write a formated string at curent cursor position
    PARAMETERS:
        *fmt pointer on a formated string
    RETURNS:
    REMARKS:
    ------------------------------------------------------------------*/

#if defined(OLEDPRINT)       || defined(OLEDPRINTLN)    || \
    defined(OLEDPRINTNUMBER) || defined(OLEDPRINTFLOAT) || \
    defined(OLEDPRINTCENTER) || defined(OLEDPRINTX)
void OLED_print(u8 module, u8 *string)
{
    while (*string != 0)
        OLED_printChar(module, *string++);
}
#endif

#if defined(OLEDPRINTLN)
void OLED_println(u8 module, u8 *string)
{
    OLED_print(module, string);
    OLED_print(module, "\n\r");
}
#endif

#if defined(OLEDPRINTCENTER)
void OLED_printCenter(u8 module, u8 *string)
{
    OLED.pixel.x = (OLED.screen.width - OLED_stringWidth(module, string)) / 2;
    
    // write string
    while (*string != 0)
        OLED_printChar(module, *string++);
}

u8 OLED_charWidth(u8 module, u8 c)
{
    // fixed width font
    if (OLED.font.address[FONT_LENGTH]==0 && OLED.font.address[FONT_LENGTH+1]==0)
        return (OLED.font.width + 1); 

    // variable width font
    if (c < OLED.font.firstChar || c > (OLED.font.firstChar + OLED.font.charCount))
        c = ' ';
    c = c - OLED.font.firstChar;
    return (OLED.font.address[FONT_WIDTH_TABLE + c] + 1);
}

u16 OLED_stringWidth(u8 module, u8* str)
{
    u16 width = 0;

    while(*str != 0)
        width += OLED_charWidth(module, *str++);

    return width;
}

#endif

#if defined(OLEDPRINTNUMBER) || defined(OLEDPRINTFLOAT) || defined(OLEDPRINTX)
void OLED_printNumber(u8 module, long value, u8 base)
{  
    gInterface = module;
    printNumber(OLED_printChar2, value, base);
}
#endif

#if defined(OLEDPRINTFLOAT) || defined(OLEDPRINTX)
void OLED_printFloat(u8 module, float number, u8 digits)
{ 
    gInterface = module;
    printFloat(OLED_printChar2, number, digits);
}
#endif

/***********************************************************************
 * printX routine (OLED.printX)
 * added by Regis Blanchot on 2018/03/16
 * useful mixed print function with a smaller footprint than printf
 * writes a string followed by a number and jump to the next line
 **********************************************************************/

#if defined(OLEDPRINTX)
void OLED_printx(u8 module, const char *s, s32 value, u8 base)
{
    OLED_print(module, s);
    if (base == BIN)
        OLED_print(module, (const char *)"0b");
    if (base == HEX)
        OLED_print(module, (const char *)"0x");
    if (base == FLOAT)
        OLED_printFloat(module, (float)value, 2);
    else
        OLED_printNumber(module, value, base);
    OLED_print(module, (const char *)"\n\r");
}
#endif

#ifdef OLEDPRINTF
void OLED_printf(u8 module, const u8 *fmt, ...)
{
    static u8 buffer[25];
    u8 *c=(char*)&buffer;
    va_list	args;

    va_start(args, fmt);
    psprintf2(buffer, fmt, args);
    va_end(args);

    while (*c != 0)
        OLED_printChar(module, *c++);
}
#endif

///	--------------------------------------------------------------------
/// Graphic functions
///	--------------------------------------------------------------------

#ifdef OLEDGRAPHICS
/*  --------------------------------------------------------------------
    DESCRIPTION:
        Draws a pixel with current color.
    PARAMETERS:
        x,y coord.
    RETURNS:
    REMARKS:
    ------------------------------------------------------------------*/

void OLED_drawPixel(u8 module, u8 x, u8 y)
{
    if (x < OLED_DISPLAY_WIDTH && y < OLED_DISPLAY_HEIGHT)
        #ifdef __SDCC
        *(OLED_buffer[y >> 3] + x) |= 1 << (y % OLED_DISPLAY_ROWS);
        #else
        OLED_buffer[y >> 3][x] |= 1 << (y % OLED_DISPLAY_ROWS);
        #endif
}

void OLED_clearPixel(u8 module, u8 x, u8 y)
{
    if (x < OLED_DISPLAY_WIDTH && y < OLED_DISPLAY_HEIGHT)
        #ifdef __SDCC
        *(OLED_buffer[y >> 3] + x) &= ~(1 << (y % OLED_DISPLAY_ROWS));
        #else
        OLED_buffer[y >> 3][x] &= ~(1 << (y % OLED_DISPLAY_ROWS));
        #endif
}

/*  --------------------------------------------------------------------
    DESCRIPTION:
        Returns pixel color at x,y position.
    PARAMETERS:
        x,y coord.
    RETURNS:
        color
    REMARKS:
    ------------------------------------------------------------------*/
#if 0
u8 OLED_getColor(u8 module, u8 x, u8 y)
{
    u8 column,temp,bit_mask;//page;
    unsigned int page;
    u8* GDptr = OLED_buffer;
   
    // Convert coordinates
    column = x & OLED_DISPLAY_WIDTH_MASK; // 0-128
    //page = (y>>OLED_DISPLAY_ROW_BITS)&OLED_DISPLAY_ROW_MASK; // 0-8
    temp = y & OLED_DISPLAY_ROW_MASK;
    bit_mask = 1<<temp;

    //GDptr+= column+(OLED_DISPLAY_WIDTH*page);  

    // Optimise
    page = (y << (OLED_DISPLAY_WIDTH_BITS - OLED_DISPLAY_ROW_BITS)); // 0-8 * 128
    GDptr += column+page;

    return (*GDptr & bit_mask ? 1:0);
}
#endif

/*  --------------------------------------------------------------------
    DESCRIPTION:
        Graphic routines based on drawPixel in graphics.c
    PARAMETERS:
    RETURNS:
    REMARKS:
    ------------------------------------------------------------------*/

void drawPixel(u16 x, u16 y)
{
    //OLED_drawPixel(gInterface, (u8)x, (u8)y);
    if (x < OLED_DISPLAY_WIDTH && y < OLED_DISPLAY_HEIGHT)
        #ifdef __SDCC
        *(OLED_buffer[y >> 3] + x) |= 1 << (y % OLED_DISPLAY_ROWS);
        #else
        OLED_buffer[y >> 3][x] |= 1 << (y % OLED_DISPLAY_ROWS);
        #endif
}

void OLED_drawLine(u8 module, u16 x0, u16 y0, u16 x1, u16 y1)
{
    gInterface = module;
    drawLine(x0, y0, x1, y1);
}

void OLED_drawRect(u8 module, u16 x1, u16 y1, u16 x2, u16 y2)
{
    gInterface = module;
    drawRect(x1, y1, x2, y2);
}

void OLED_drawRoundRect(u8 module, u16 x1, u16 y1, u16 x2, u16 y2)
{
    gInterface = module;
    drawRoundRect(x1, y1, x2, y2);
}

void OLED_fillRect(u8 module, u16 x1, u16 y1, u16 x2, u16 y2)
{
    gInterface = module;
    fillRect(x1, y1, x2, y2);
}

void OLED_fillRoundRect(u8 module, u16 x1, u16 y1, u16 x2, u16 y2)
{
    gInterface = module;
    fillRoundRect(x1, y1, x2, y2);
}

void OLED_drawCircle(u8 module, u16 x, u16 y, u16 radius)
{
    gInterface = module;
    drawCircle(x, y, radius);
}

void OLED_fillCircle(u8 module, u16 x, u16 y, u16 radius)
{
    gInterface = module;
    fillCircle(x, y, radius);
}

#ifdef OLEDDRAWBITMAP
void OLED_drawBitmap(u8 module1, u16 module2, const u8* filename, u16 x, u16 y)
{
    gInterface = module1;
    drawBitmap(module2, filename, x, y);
}
#endif

#endif // OLEDGRAPHICS

#endif /* __OLED_C */
