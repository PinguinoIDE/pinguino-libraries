/*  --------------------------------------------------------------------
    FILE:        SSD1306.c
    PROJECT:     Pinguino
    PURPOSE:     Drive 0.96" 128x64 Oled display (SSD1306 controller)
    PROGRAMER:   Regis blanchot <rblanchot@gmail.com>
    --------------------------------------------------------------------
    http://mbed.org/users/Byrn/code/SSD1306/file/1d9df877c90a/ssd1306.cpp
    --------------------------------------------------------------------
    CHANGELOG
    17 Oct. 2013 - Régis Blanchot - first release
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
    ------------------------------------------------------------------------
    TODO:
    * Manage screen's size in SSD1306_init
    * Add support to SSD1309, SSD1327, SSD1331 and SH1106 support
    * Rename SSD1306.xxx functions to Oled.xxx functions
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

#ifndef __SSD1306_C
#define __SSD1306_C

// Pinguino standards
#include <typedef.h>
#include <const.h>
#include <macro.h>
#include <stdarg.h>
#include <string.h>         // memset, memcpy
#include <SSD1306.h>

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
#if defined(SSD1306USEPMP6800) || defined(SSD1306USEPMP8080)
#if defined(__18f46j53) || defined(__18f47j53) || defined(__PIC32MX__)
    #include <pmp.c>
#endif
#endif

// I2C
#if defined(SSD1306USEI2C1) || defined(SSD1306USEI2C2)
    #include <i2c.c>
#endif

// SPI
#if defined(SSD1306USESPISW) || \
    defined(SSD1306USESPI1)  || defined(SSD1306USESPI2)
    //defined(SSD1306USESPI3)  || defined(SSD1306USESPI4)
    #include <spi.c>
#endif

// Printf
#ifdef SSD1306PRINTF
    #include <printFormated.c>
#endif

// PrintFloat
#if defined(SSD1306PRINTFLOAT)
    #include <printFloat.c>
#endif

// PrintNumber
#if defined(SSD1306PRINTNUMBER) || defined(SSD1306PRINTFLOAT)
    #include <printNumber.c>
#endif

// Graphics Library
#if defined(SSD1306GRAPHICS) || defined(SSD1306DRAWBITMAP)
    #ifdef SSD1306DRAWBITMAP
    #define DRAWBITMAP
    #endif
    #include <graphics.c>
#endif

///	--------------------------------------------------------------------
/// Globals
///	--------------------------------------------------------------------

// Interface (I2C, SPI, etc...)
u8 SSD1306_INTF;

// Screen buffers / 1 buffer per line
u8 logo1[SSD1306_DISPLAY_WIDTH];
u8 logo2[SSD1306_DISPLAY_WIDTH];
u8 logo3[SSD1306_DISPLAY_WIDTH];
u8 logo4[SSD1306_DISPLAY_WIDTH];
#ifdef SSD1306_128X64
u8 logo5[SSD1306_DISPLAY_WIDTH];
u8 logo6[SSD1306_DISPLAY_WIDTH];
u8 logo7[SSD1306_DISPLAY_WIDTH];
u8 logo8[SSD1306_DISPLAY_WIDTH];
#endif

// Buffers pointers
u8 *SSD1306_buffer[SSD1306_DISPLAY_ROWS] = {
    logo1, logo2, logo3, logo4,
    #ifdef SSD1306_128X64
    logo5, logo6, logo7, logo8
    #endif
};

// Pins
#if   defined(SSD1306USEI2C1)  || defined(SSD1306USEI2C2)
    u8 SSD1306_I2CADDR;
#else
    u8 pRST, pDC;
#endif

///	--------------------------------------------------------------------
/// Core functions
///	--------------------------------------------------------------------

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

void SSD1306_sendCommand(u8 module, u8 val)
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
    #endif

    ///-----------------------------------------------------------------

    #elif defined(SSD1306USEI2C1) || defined(SSD1306USEI2C2)
    I2C_start(module);
    //if (I2C_writeChar(module, (SSD1306_I2CADDR << 1) | I2C_WRITE))
    if (I2C_writeChar(module, SSD1306_I2CADDR))
    {
        I2C_writeChar(module, SSD1306_CMD_STREAM); // Co = 0, D/C = 0
        I2C_writeChar(module, val);
    }
    I2C_stop(module);

    ///-----------------------------------------------------------------

    #elif defined(SSD1306USESPISW) ||defined(SSD1306USESPI1) ||defined(SSD1306USESPI2)
    SPI_select(module);
    low(pDC);             // COMMAND read/write
    SPI_write(module, val);
    SPI_deselect(module);

    ///-----------------------------------------------------------------

    #else
    #error "Unknown Protocol of Communication"
    #endif
}

void SSD1306_sendData(u8 module, u8 val)
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

    #elif defined(SSD1306USEI2C1) || defined(SSD1306USEI2C2)
    I2C_start(module);
    //if (I2C_writeChar(module, (SSD1306_I2CADDR << 1) | I2C_WRITE))
    if (I2C_writeChar(module, SSD1306_I2CADDR))
    {
        I2C_writeChar(module, SSD1306_DATA_STREAM); // Co = 0, D/C = 1
        I2C_writeChar(module, val);
    }
    I2C_stop(module);

    ///-----------------------------------------------------------------

    #elif defined(SSD1306USESPISW) ||defined(SSD1306USESPI1) ||defined(SSD1306USESPI2)
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
    if MODULE = PMP6800
            OLED CS#    connected to GND
            OLED RES#   connected to any GPIO (res)
            OLED D/C#   connected to Pinguino PMA[0:15] (pDC)
            OLED W/R#   connected to Pinguino PMRD/PMWR
            OLED E/RD#  connected to GND
            OLED D[7:0] connected to Pinguino PMD[7:0]
            SSD1306.init(PMP6800, pRST, pDC);
            
    if MODULE = PMP8080
            OLED CS#    connected to GND
            OLED RES#   connected to any GPIO (res)
            OLED D/C#   connected to Pinguino PMA1
            OLED W/R#   connected to Pinguino PMWR
            OLED E/RD#  connected to GND
            OLED D[7:0] connected to Pinguino PMD[7:0]
            SSD1306.init(PMP6800, pRST);
            
    if MODULE = PORTB
            OLED CS#    connected to GND
            OLED RES#   connected to any GPIO
            OLED D/C#   connected to any GPIO
            OLED W/R#   connected to any GPIO
            OLED E/RD#  connected to GND
            OLED D[7:0] connected to Pinguino D[0:7]
            
    if MODULE = PORTD 
            OLED CS#    connected to GND
            OLED RES#   connected to any GPIO (D0)
            OLED D/C#   connected to any GPIO (D1)
            OLED W/R#   connected to any GPIO (D2)
            OLED E/RD#  connected to GND
            OLED D[7:0] connected to Pinguino D[31:24]
            
    if MODULE = I2C
            SSD1306.init(I2C, address);
    
    if MODULE = SPISW
            SSD1306.init(SPISW, pSDO, pSCK, pCS, pDC, pRST);
    
    if MODULE = SPIx (SPI1, SPI2, ...)
            SSD1306.init(SPIx, pDC, pRST);
    --------------------------------------------------------------------
    Important :
    I didn't add support to the 3-wire SPI mode because in this mode we
    don't use the DC pin and thus we have to send an extra bit to
    simulate this pin. Microchip doesn't provide support to that 9-bit
    SPI mode.
    ------------------------------------------------------------------*/

void SSD1306_init(u8 module, ...)
{
    #if defined(SSD1306USESPISW)
    u8 pSDO, pSCK, pCS;
    #endif

    va_list args;
    
    SSD1306_INTF = module;
    
    va_start(args, module); // args points on the argument after module

    ///-----------------------------------------------------------------

    #if defined(SSD1606USEPORT6800) || defined(SSD1606USEPORT8080)
    //void SSD1306_init(PORT6800, pRST, pDC)
        dDATA = 0x00;           // output
        dCMD  = 0x00;           // output
        DATA  = 0x00;
        CMD   = 0xFF;           // every pin HIGH

    ///-----------------------------------------------------------------

    #elif defined(SSD1606USEPMP6800) || defined(SSD1606USEPMP8080)
    #if defined(__18f46j53) || defined(__18f47j53) || defined(__PIC32MX__)
    //SSD1306.init(PMP6800, pRST, pDC);
    pRST = va_arg(args, int);
    pDC  = va_arg(args, int);
    high(pRST);
    pinmode(pRST, OUTPUT);              // output
    if   (module == SSD1306_PMP6800)
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

    #elif defined(SSD1306USEI2C1) ||defined(SSD1306USEI2C2)
    //SSD1306.init(I2C, address);
    SSD1306_I2CADDR = va_arg(args, int);
    //I2C_init(module, I2C_MASTER_MODE, I2C_100KHZ); // OK
    //I2C_init(module, I2C_MASTER_MODE, I2C_400KHZ); // OK
    I2C_init(module, I2C_MASTER_MODE, I2C_1MHZ); // OK

    ///-----------------------------------------------------------------

    #elif defined(SSD1306USESPI1) ||defined(SSD1306USESPI2)
    //SSD1306.init(SPIx, pDC, pRST);
    pDC  = va_arg(args, int);       // get the next arg
    pRST = va_arg(args, int);       // get the next arg
    SPI_setMode(module, SPI_MASTER);
    SPI_setDataMode(module, SPI_MODE1);
    #if !defined(__PIC32MX__)
    //maximum baud rate possible = FPB = FOSC/4
    SPI_setClockDivider(module, SPI_CLOCK_DIV4);
    #else
    //maximum baud rate possible = FPB/2
    SPI_setClockDivider(module, SPI_PBCLOCK_DIV2);
    #endif
    SPI_begin(module);

    ///-----------------------------------------------------------------

    #elif defined(SSD1306USESPISW)
    //SSD1306.init(SPISW, pSDO, pSCK, pCS, pDC, pRST);
    pSDO = va_arg(args, int);         // get the next arg
    pSCK = va_arg(args, int);         // get the next arg
    pCS  = va_arg(args, int);         // get the last arg
    pDC  = va_arg(args, int);         // get the next arg
    pRST = va_arg(args, int);         // get the next arg
    SPI_setBitOrder(module, SPI_MSBFIRST);
    SPI_begin(module, pSDO, pSCK, pCS);

    ///-----------------------------------------------------------------

    #else
    #error "Unknown Interface"
    #endif

    ///-----------------------------------------------------------------

    va_end(args);           // cleans up the list
    
    // default Screen Values

    //SSD1306.orientation   = PORTRAIT;
    SSD1306.pixel.x       = 0;
    SSD1306.pixel.y       = 0;
    SSD1306.screen.startx = 0;
    SSD1306.screen.starty = 0;
    SSD1306.screen.endx   = SSD1306_DISPLAY_WIDTH  - 1;
    SSD1306.screen.endy   = SSD1306_DISPLAY_HEIGHT - 1;
    SSD1306.screen.width  = SSD1306_DISPLAY_WIDTH;
    SSD1306.screen.height = SSD1306_DISPLAY_HEIGHT;

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
    #if !defined(SSD1306USEI2C1) && !defined(SSD1306USEI2C2)
    low(pRST);              // initialized the chip
    Delayus(50);            // for at least 3us
    high(pRST);
    #endif

    SSD1306_displayOn(module);
    Delayms(100);               // wait for SEG/COM to be ON
    SSD1306_displayOff(module);

    SSD1306_setDisplayClockRatioAndFrequency(module, 0, 8); // 0x80
    SSD1306_setMultiplexRatio(module, 0x3F); // 1/64 duty
    SSD1306_setDisplayOffset(module, 0);    
    SSD1306_setDisplayStartLine(module, 0);  
    SSD1306_setChargePumpEnable(module, 1);  // 1=0x14, 0=0x10
    SSD1306_setMemoryAddressingMode(module, 0); // horizontal addressing mode; across then down
    SSD1306_setSegmentRemap(module, 1);      // rotate screen 180
    SSD1306_setComPinsHardwareConfiguration(module, 1, 0); // 0x12
    SSD1306_setComOutputScanDirection(module, 1);
    SSD1306_setContrast(module, 0xFF); // or 0xCF
    SSD1306_setPrechargePeriod(module, 0x0F, 0x01); // 0xF1
    SSD1306_setVcomhDeselectLevel(module, 1);
    SSD1306_sendCommand(module, SSD1306_DISPLAYALLON_RESUME);
    SSD1306_setInverse(module, 0); // normal display
    SSD1306_wake(module); // display on

    //SSD1306_hvSetColumnAddress(module, 0, 127);
    //SSD1306_hvSetPageAddress(module, 0, 7);

    //SSD1306_pamSetStartAddress(module, 0);
    //SSD1306_pamSetPageStart(module, 0);
}

///	--------------------------------------------------------------------
/// Update the display
///	--------------------------------------------------------------------

void SSD1306_refresh(u8 module)
{
    u8 i, j;

    SSD1306_hvSetColumnAddress(module, 0, SSD1306.screen.endx);
    SSD1306_hvSetPageAddress(  module, 0, SSD1306.screen.endy);

    #if defined(SSD1306USEI2C1) || defined(SSD1306USEI2C2)

        I2C_start(module);
        //if (I2C_writeChar(module, (SSD1306_I2CADDR << 1) | I2C_WRITE))
        I2C_writeChar(module, SSD1306_I2CADDR);
        I2C_writeChar(module, SSD1306_DATA_STREAM);
        for (i=0; i<SSD1306_DISPLAY_ROWS; i++)
        {
            for (j=0; j<SSD1306_DISPLAY_WIDTH; j++)
                #ifdef __SDCC
                I2C_writeChar(module, *(SSD1306_buffer[i])++);
                #else
                I2C_writeChar(module, SSD1306_buffer[i][j]);
                #endif
        }
        I2C_stop(module);
    
    #elif defined(SSD1306USESPISW) ||defined(SSD1306USESPI1) ||defined(SSD1306USESPI2)

        SPI_select(module);
        high(pDC);
        for (i=0; i<SSD1306_DISPLAY_ROWS; i++)
        {
            for (j=0; j<SSD1306_DISPLAY_WIDTH; j++)
                #ifdef __SDCC
                SPI_write(module, *(SSD1306_buffer[i])++);
                #else
                SPI_write(module, SSD1306_buffer[i][j]);
                #endif
        }
        SPI_deselect(module);
    
    #else
    
        for (i=0; i<SSD1306_DISPLAY_ROWS; i++)
        {
            for (j=0; j<SSD1306_DISPLAY_WIDTH; j++)
                #ifdef __SDCC
                SSD1306_sendData(module, *(SSD1306_buffer[i])++);
                #else
                SSD1306_sendData(module, SSD1306_buffer[i][j]);
                #endif
        }

    #endif
}

///	--------------------------------------------------------------------
/// Clear the buffers
/// NB : void *memset(void *str, int c, size_t n)
///	--------------------------------------------------------------------

void SSD1306_clearScreen(u8 module)
{
    u8 i, j;

    for (i=0; i<SSD1306_DISPLAY_ROWS; i++)
    {
        #if defined(__PIC32MX__) || defined(__XC8__)
        memset( SSD1306_buffer[i], 0, SSD1306_DISPLAY_WIDTH);
        #else  
        memset(&SSD1306_buffer[i], 0, SSD1306_DISPLAY_WIDTH);
        #endif
    }

    SSD1306.pixel.x = 0;
    SSD1306.pixel.y = 0;
}

///	--------------------------------------------------------------------
/// Scroll functions
///	--------------------------------------------------------------------

void SSD1306_startHorizontalScroll(u8 module, u8 direction, u8 start, u8 end, u16 interval) 
{
    // Before issuing this command the horizontal scroll must be deactivated (2Eh).
    // Otherwise, RAM content may be corrupted.
    // RB : this datasheet recommandation doesn't work
    //SSD1306_sendCommand(module, SSD1306_DEACTIVATE_SCROLL);

    SSD1306_sendCommand(module, direction);// ? 0x27 : 0x26);
    // Dummy byte (???)
    SSD1306_sendCommand(module, 0x00);
    // Start page
    SSD1306_sendCommand(module, start & SSD1306_DISPLAY_ROW_MASK);

    // Time interval between each scroll step
    switch (interval)
    {
        case   2: SSD1306_sendCommand(module, 0x07); break; // 111b
        case   3: SSD1306_sendCommand(module, 0x04); break; // 100b
        case   4: SSD1306_sendCommand(module, 0x05); break; // 101b
        case   5: SSD1306_sendCommand(module, 0x00); break; // 000b
        case  25: SSD1306_sendCommand(module, 0x06); break; // 110b
        case  64: SSD1306_sendCommand(module, 0x01); break; // 001b
        case 128: SSD1306_sendCommand(module, 0x02); break; // 010b
        case 256: SSD1306_sendCommand(module, 0x03); break; // 011b
        default:  SSD1306_sendCommand(module, 0x07); break; // default to 2 frame interval
    }
    
    // End page
    //SSD1306_sendCommand(module, 0x00);
    SSD1306_sendCommand(module, end & SSD1306_DISPLAY_ROW_MASK);

    // ???
    SSD1306_sendCommand(module, 0x00);
    SSD1306_sendCommand(module, 0xFF);

    // activate scroll
    SSD1306_sendCommand(module, SSD1306_ACTIVATE_SCROLL);
}

void SSD1306_startVerticalAndHorizontalScroll(u8 module, u8 direction, u8 start, u8 end, u16 interval, u8 vertical_offset)
{
    // Before issuing this command the horizontal scroll must be deactivated (2Eh).
    // Otherwise, RAM content may be corrupted.
    // RB : this datasheet recommandation doesn't work
    //SSD1306_sendCommand(module, SSD1306_DEACTIVATE_SCROLL);

    SSD1306_sendCommand(module, direction); // ? 0x2A : 0x29);
    // Dummy byte (???)
    SSD1306_sendCommand(module, 0x00);
    // Start page
    SSD1306_sendCommand(module, start & SSD1306_DISPLAY_ROW_MASK);

    // Time interval between each scroll step
    switch (interval)
    {
        case   2: SSD1306_sendCommand(module, 0x07); break; // 111b
        case   3: SSD1306_sendCommand(module, 0x04); break; // 100b
        case   4: SSD1306_sendCommand(module, 0x05); break; // 101b
        case   5: SSD1306_sendCommand(module, 0x00); break; // 000b
        case  25: SSD1306_sendCommand(module, 0x06); break; // 110b
        case  64: SSD1306_sendCommand(module, 0x01); break; // 001b
        case 128: SSD1306_sendCommand(module, 0x02); break; // 010b
        case 256: SSD1306_sendCommand(module, 0x03); break; // 011b
        default:  SSD1306_sendCommand(module, 0x07); break; // default to 2 frame interval
    }

    // End page
    SSD1306_sendCommand(module, end & SSD1306_DISPLAY_ROW_MASK);

    SSD1306_sendCommand(module, vertical_offset);    

    // activate scroll
    SSD1306_sendCommand(module, SSD1306_ACTIVATE_SCROLL);
}

// Up handed 1-row scroll
// The display is 16 rows tall.
void SSD1306_scrollUp(u8 module)
{
    u8 i;
    u8 bytes = ((SSD1306.font.height + 7) / 8);
    u8 lastline = SSD1306_DISPLAY_ROWS - bytes;

    // Copy line y in Line y-1
    //void *memcpy(void *dest, const void *src, size_t n)
    for (i=0; i<lastline; i++)
        #if defined(__PIC32MX__) || defined(__XC8__)
        memcpy( SSD1306_buffer[i], SSD1306_buffer[i + 1], SSD1306_DISPLAY_WIDTH);
        #else
        memcpy(&SSD1306_buffer[i], SSD1306_buffer[i + 1], SSD1306_DISPLAY_WIDTH);
        #endif

    // Clear the last lines
    //void *memset(void *str, int c, size_t n)
    for (i=lastline; i<SSD1306_DISPLAY_ROWS; i++)
        #if defined(__PIC32MX__) || defined(__XC8__)
        memset( SSD1306_buffer[i], 0, SSD1306_DISPLAY_WIDTH);
        #else
        memset(&SSD1306_buffer[i], 0, SSD1306_DISPLAY_WIDTH);
        #endif
    
    SSD1306.pixel.y = SSD1306.pixel.y - (8 * bytes);
}

///	--------------------------------------------------------------------
/// Print functions
///	--------------------------------------------------------------------

void SSD1306_setFont(u8 module, const u8 *font)
{
    SSD1306.font.address   = font;
    SSD1306.font.width     = font[FONT_WIDTH];
    SSD1306.font.height    = font[FONT_HEIGHT];
    SSD1306.font.firstChar = font[FONT_FIRST_CHAR];
    SSD1306.font.charCount = font[FONT_CHAR_COUNT];
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
#ifdef SSD1306SETCURSOR
void SSD1306_setCursor(u8 module, u8 x, u8 y)
{
    if ( x >= SSD1306_DISPLAY_WIDTH || y >= (SSD1306_DISPLAY_HEIGHT) ) return;

    SSD1306.pixel.x = x * (SSD1306.font.width+1);
    SSD1306.pixel.y = y * (SSD1306.font.height+1);
}
#endif
/*  --------------------------------------------------------------------
    DESCRIPTION:
        write a char at (SSD1306.cursor.x, SSD1306.cursor.y)
    PARAMETERS:
        * c ascii code of the character to print
    RETURNS:
    REMARKS:
    ------------------------------------------------------------------*/

void SSD1306_printChar2(u8 c)
{
    SSD1306_printChar(SSD1306_INTF, c);
}

void SSD1306_printChar(u8 module, u8 c)
{
    u8  x, y;
    u8  i, j, page;
    u8  dat, tab, k;
    u8  width = 0;
    u8  bytes = (SSD1306.font.height + 7) / 8;
    u16 index = 0;

    if ((SSD1306.pixel.x + SSD1306.font.width) > SSD1306.screen.width)
    {
        SSD1306.pixel.x = 0;
        SSD1306.pixel.y = SSD1306.pixel.y + bytes*8; //SSD1306.font.height;
    }

    if ((SSD1306.pixel.y + SSD1306.font.height) > SSD1306.screen.height)
    {
        SSD1306_scrollUp(module);            
        //SSD1306.pixel.y = 0;
    }

    switch (c)
    {
        case '\n':
            SSD1306.pixel.y = SSD1306.pixel.y + bytes*8; //SSD1306.font.height;
            break;
            
        case '\r':
            SSD1306.pixel.x = 0;
            break;
            
        case '\t':
            tab = SSD1306_TABSIZE * SSD1306.font.width;
            SSD1306.pixel.x += (SSD1306.pixel.x + tab) % tab;
            break;
            
        default:
            if (c < SSD1306.font.firstChar || c >= (SSD1306.font.firstChar + SSD1306.font.charCount))
                c = ' ';
            c = c - SSD1306.font.firstChar;

            // fixed width font
            if (SSD1306.font.address[FONT_LENGTH]==0 && SSD1306.font.address[FONT_LENGTH+1]==0)
            {
                width = SSD1306.font.width; 
                index = FONT_OFFSET + c * bytes * width;
            }

            // variable width font
            else
            {
                width = SSD1306.font.address[FONT_WIDTH_TABLE + c];
                for (i=0; i<c; i++)
                    index += SSD1306.font.address[FONT_WIDTH_TABLE + i];
                index = FONT_WIDTH_TABLE + SSD1306.font.charCount + index * bytes;
            }

            // save the coordinates
            x = SSD1306.pixel.x;
            y = SSD1306.pixel.y;

            // draw the character
            for (i=0; i<bytes; i++)
            {
                page = i * width;
                for (j=0; j<width; j++)
                {
                    dat = SSD1306.font.address[index + page + j];
                    // if char. takes place on more than 1 line (8 bits)
                    if (SSD1306.font.height > 8)
                    {
                        k = ((i+1)<<3);
                        if (SSD1306.font.height < k)
                            dat >>= k - SSD1306.font.height;
                    }
                    // Write the byte
                    #ifdef __SDCC
                    *(SSD1306_buffer[y >> 3] + x + j) = dat;
                    #else
                    SSD1306_buffer[y >> 3][x + j] = dat;
                    #endif
                }

                // 1px gap between chars
                #ifdef __SDCC
                *(SSD1306_buffer[y >> 3] + x + width) = 0;
                #else
                SSD1306_buffer[y >> 3][x + width] = 0;
                #endif

                // Next part of the current char will be one line under
                y += 8;
            }
            // Next char location
            SSD1306.pixel.x = x + width + 1;
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

#if defined(SSD1306PRINT)       || defined(SSD1306PRINTLN)    || \
    defined(SSD1306PRINTNUMBER) || defined(SSD1306PRINTFLOAT) || \
    defined(SSD1306PRINTCENTER)
void SSD1306_print(u8 module, u8 *string)
{
    while (*string != 0)
        SSD1306_printChar(module, *string++);
}
#endif

#if defined(SSD1306PRINTLN)
void SSD1306_println(u8 module, u8 *string)
{
    SSD1306_print(module, string);
    SSD1306_print(module, "\n\r");
}
#endif

#if defined(SSD1306PRINTCENTER)
void SSD1306_printCenter(u8 module, u8 *string)
{
    SSD1306.pixel.x = (SSD1306.screen.width - SSD1306_stringWidth(module, string)) / 2;
    
    // write string
    while (*string != 0)
        SSD1306_printChar(module, *string++);
}

u8 SSD1306_charWidth(u8 module, u8 c)
{
    // fixed width font
    if (SSD1306.font.address[FONT_LENGTH]==0 && SSD1306.font.address[FONT_LENGTH+1]==0)
        return (SSD1306.font.width + 1); 

    // variable width font
    if (c < SSD1306.font.firstChar || c > (SSD1306.font.firstChar + SSD1306.font.charCount))
        c = ' ';
    c = c - SSD1306.font.firstChar;
    return (SSD1306.font.address[FONT_WIDTH_TABLE + c] + 1);
}

u16 SSD1306_stringWidth(u8 module, u8* str)
{
    u16 width = 0;

    while(*str != 0)
        width += SSD1306_charWidth(module, *str++);

    return width;
}

#endif

#if defined(SSD1306PRINTNUMBER) || defined(SSD1306PRINTFLOAT)
void SSD1306_printNumber(u8 module, long value, u8 base)
{  
    SSD1306_INTF = module;
    printNumber(SSD1306_printChar2, value, base);
}
#endif

#if defined(SSD1306PRINTFLOAT)
void SSD1306_printFloat(u8 module, float number, u8 digits)
{ 
    SSD1306_INTF = module;
    printFloat(SSD1306_printChar2, number, digits);
}
#endif

#ifdef SSD1306PRINTF
void SSD1306_printf(u8 module, const u8 *fmt, ...)
{
    static u8 buffer[25];
    u8 *c=(char*)&buffer;
    va_list	args;

    va_start(args, fmt);
    psprintf2(buffer, fmt, args);
    va_end(args);

    while (*c != 0)
        SSD1306_printChar(module, *c++);
}
#endif

///	--------------------------------------------------------------------
/// Graphic functions
///	--------------------------------------------------------------------

#ifdef SSD1306GRAPHICS
/*  --------------------------------------------------------------------
    DESCRIPTION:
        Draws a pixel with current color.
    PARAMETERS:
        x,y coord.
    RETURNS:
    REMARKS:
    ------------------------------------------------------------------*/

void SSD1306_drawPixel(u8 module, u8 x, u8 y)
{
    if (x < SSD1306_DISPLAY_WIDTH && y < SSD1306_DISPLAY_HEIGHT)
        #ifdef __SDCC
        *(SSD1306_buffer[y >> 3] + x) |= 1 << (y % SSD1306_DISPLAY_ROWS);
        #else
        SSD1306_buffer[y >> 3][x] |= 1 << (y % SSD1306_DISPLAY_ROWS);
        #endif
}

void SSD1306_clearPixel(u8 module, u8 x, u8 y)
{
    if (x < SSD1306_DISPLAY_WIDTH && y < SSD1306_DISPLAY_HEIGHT)
        #ifdef __SDCC
        *(SSD1306_buffer[y >> 3] + x) &= ~(1 << (y % SSD1306_DISPLAY_ROWS));
        #else
        SSD1306_buffer[y >> 3][x] &= ~(1 << (y % SSD1306_DISPLAY_ROWS));
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
u8 SSD1306_getColor(u8 module, u8 x, u8 y)
{
    u8 column,temp,bit_mask;//page;
    unsigned int page;
    u8* GDptr = SSD1306_buffer;
   
    // Convert coordinates
    column = x & SSD1306_DISPLAY_WIDTH_MASK; // 0-128
    //page = (y>>SSD1306_DISPLAY_ROW_BITS)&SSD1306_DISPLAY_ROW_MASK; // 0-8
    temp = y & SSD1306_DISPLAY_ROW_MASK;
    bit_mask = 1<<temp;

    //GDptr+= column+(SSD1306_DISPLAY_WIDTH*page);  

    // Optimise
    page = (y << (SSD1306_DISPLAY_WIDTH_BITS - SSD1306_DISPLAY_ROW_BITS)); // 0-8 * 128
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
    //SSD1306_drawPixel(SSD1306_INTF, (u8)x, (u8)y);
    if (x < SSD1306_DISPLAY_WIDTH && y < SSD1306_DISPLAY_HEIGHT)
        #ifdef __SDCC
        *(SSD1306_buffer[y >> 3] + x) |= 1 << (y % SSD1306_DISPLAY_ROWS);
        #else
        SSD1306_buffer[y >> 3][x] |= 1 << (y % SSD1306_DISPLAY_ROWS);
        #endif
}

void SSD1306_drawLine(u8 module, u16 x0, u16 y0, u16 x1, u16 y1)
{
    SSD1306_INTF = module;
    drawLine(x0, y0, x1, y1);
}

void SSD1306_drawRect(u8 module, u16 x1, u16 y1, u16 x2, u16 y2)
{
    SSD1306_INTF = module;
    drawRect(x1, y1, x2, y2);
}

void SSD1306_drawRoundRect(u8 module, u16 x1, u16 y1, u16 x2, u16 y2)
{
    SSD1306_INTF = module;
    drawRoundRect(x1, y1, x2, y2);
}

void SSD1306_fillRect(u8 module, u16 x1, u16 y1, u16 x2, u16 y2)
{
    SSD1306_INTF = module;
    fillRect(x1, y1, x2, y2);
}

void SSD1306_fillRoundRect(u8 module, u16 x1, u16 y1, u16 x2, u16 y2)
{
    SSD1306_INTF = module;
    fillRoundRect(x1, y1, x2, y2);
}

void SSD1306_drawCircle(u8 module, u16 x, u16 y, u16 radius)
{
    SSD1306_INTF = module;
    drawCircle(x, y, radius);
}

void SSD1306_fillCircle(u8 module, u16 x, u16 y, u16 radius)
{
    SSD1306_INTF = module;
    fillCircle(x, y, radius);
}

#ifdef SSD1306DRAWBITMAP
void SSD1306_drawBitmap(u8 module1, u16 module2, const u8* filename, u16 x, u16 y)
{
    SSD1306_INTF = module1;
    drawBitmap(module2, filename, x, y);
}
#endif

#endif // SSD1306GRAPHICS

#endif /* __SSD1306_C */
