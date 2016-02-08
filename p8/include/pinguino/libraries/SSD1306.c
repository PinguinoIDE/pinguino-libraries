/*  --------------------------------------------------------------------
    FILE:			SSD1306[module].c
    PROJECT:		pinguino
    PURPOSE:		Drive 0.96" 128x64 Oled display (SSD1306 controller)
    PROGRAMER:		regis blanchot <rblanchot@gmail.com>
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
    ------------------------------------------------------------------------
    TODO:
    * Manage the 2 size available : SSD1306_128X64 or SSD1306_128X32
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

1   VCC     |   2       GND
3   CS      |   4       RES
5   D/C     |   6       R/W
7   E/RD    |   8       D0
9   D1      |   10      D2
11  D3      |   12      D4
13  D5      |   14      D6
15  D7      |   16      NC

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

#include <SSD1306.h>


// Screen buffer
#include <logo/pinguino128x64.h>
//u8 logo[128*4];

// Pinguino standards
//#include <string.h>
#include <digitalw.c>
#include <typedef.h>
#include <const.h>
#include <macro.h>
#include <stdarg.h>
#include <delayms.c>
#include <delayus.c>
#include <spi.c>
#include <i2c.c>
#include <pmp.c>

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

u8 *SSD1306_buffer = logo;
//*SSD1306_bufferptr = SSD1306_buffer;
u8 SSD1306_MOD;
u8 SSD1306_I2C_ADDR;

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

void SSD1306_sendCommand(module, u8 module, u8 val)
{
    switch (module)
    {
        case PMP6800:
            PMP_wait();         // wait for PMP to be available
            PMADDRH = 0;
            PMADDRL = 0;
            PMDIN1L = val;

            Low(CS);            // Chip select
            Low(WR);            // WRITE
            Low(DC);            // COMMAND

            High(E);
            DATA = val;         // WR should stay low at least 60ns
            Low(E);             // A rising edge of WR enables Write mode

            High(CS);           // Chip deselected
            break;

        case PMP8080:
            PMP_wait();         // wait for PMP to be available
            PMADDRH = 0;
            PMADDRL = 0;
            PMDIN1L = val;

            Low(CS);            // Chip select
            Low(DC);            // COMMAND read/write

            Low(WR);
            DATA = val;         // WR should stay low at least 60ns
            High(WR);           // A rising edge of WR enables Write mode

            High(CS);           // Chip deselected
            break;

        case I2C:
            I2C_start();
            if (I2C_write(SSD1306_I2C_ADDR | I2C_WRITE))
            {
                I2C_write(0x00); // Co = 0, D/C = 0
                I2C_write(val);
            }
            I2C_stop();
            break

        case SPISW:
        case SPI1:
        case SPI2:
            SPI_select(module);
            Low(DC);             // COMMAND read/write
            SPI_write(module, val);
            SPI_deselect(module);
            break;
    }
}

void SSD1306_sendData(u8 val)
{
    switch (module)
    {
        case PMP6800:
            PMP_wait();         // wait for PMP to be available
            PMADDRH = 0;        // High8(DC);
            PMADDRL = DC;       // low8(DC);
            PMDIN1L = val;

            Low(CS);            // Chip select
            High(DC);           // DATA read/write

            Low(WR);
            DATA = val;         // WR should stay low at least 60ns
            High(WR);           // A rising edge of WR enables Write mode

            High(CS);           // Chip deselected
            break;
            
        case PMP8080:
            PMP_wait();         // wait for PMP to be available
            PMADDRH = 0;        // High8(DC);
            PMADDRL = DC;       // low8(DC);
            PMDIN1L = val;

            Low(CS);            // Chip select
            Low(WR);            // WRITE
            High(DC);           // DATA

            High(E);
            DATA = val;         // WR should stay low at least 60ns
            Low(E);             // A rising edge of WR enables Write mode

            High(CS);           // Chip deselected
            break;
            
        case I2C:
            I2C_start();
            if (I2C_write(SSD1306_I2C_ADDR | I2C_WRITE))
            {
                I2C_write(0x40); // Co = 0, D/C = 1
                I2C_write(val);
            }
            I2C_stop();
            break;

        case SPISW:
        case SPI1:
        case SPI2:
            SPI_select(module);
            High(DC);           // DATA read/write
            SPI_write(module, val);
            SPI_deselect(module);
            break;
    }
}

///	--------------------------------------------------------------------
/// Basic functions (TODO : replace functions with #define) 
///	--------------------------------------------------------------------

void SSD1306_displayOff(u8 module)
{
    SSD1306_sendCommand(module, 0xAE);
}

void SSD1306_displayOn(u8 module)
{
    SSD1306_sendCommand(module, 0xAF);
}

void SSD1306_sleep(u8 module)
{
    SSD1306_sendCommand(module, 0xAE);
}

void SSD1306_wake(u8 module)
{
    SSD1306_sendCommand(module, 0xAF);
}

void SSD1306_setInverse(u8 module, u8 value)
{
    SSD1306_sendCommand(module, value ? 0xA7 : 0xA6);
}

void SSD1306_setDisplayOffset(u8 module, u8 value)
{
    SSD1306_sendCommand(module, 0xD3);
    SSD1306_sendCommand(module, value & 0x3F); 
}

void SSD1306_setContrast(u8 module, u8 value) 
{
    SSD1306_sendCommand(module, 0x81);
    SSD1306_sendCommand(module, value);
}

void SSD1306_setDisplayStartLine(u8 module, u8 value)
{
    SSD1306_sendCommand(module, 0x40 | value);
}

void SSD1306_setSegmentRemap(u8 module, u8 value)
{
    SSD1306_sendCommand(module, value ? 0xA1 : 0xA0);
}

void SSD1306_setMultiplexRatio(u8 module, u8 value)
{
    SSD1306_sendCommand(module, 0xA8);
    SSD1306_sendCommand(module, value & 0x3F);
}

void SSD1306_setComOutputScanDirection(u8 module, u8 value)
{
    SSD1306_sendCommand(module, value ? 0xC8 : 0xC0);
}

void SSD1306_setComPinsHardwareConfiguration(u8 module, u8 sequential, u8 lr_remap)
{
    SSD1306_sendCommand(module, 0xDA);
    SSD1306_sendCommand(module, 0x02 | ((sequential & 1) << 4) | ((lr_remap & 1) << 5));
}

void SSD1306_pamSetStartAddress(u8 module, u8 address)
{
    // "Set Lower Column Start Address for Page Addressing Mode"
    SSD1306_sendCommand(module, address & 0x0F);

    // "Set Higher Column Start Address for Page Addressing Mode"
    SSD1306_sendCommand(module, (address << 4) & 0x0F);
}

void SSD1306_setMemoryAddressingMode(u8 module, u8 mode)
{
    SSD1306_sendCommand(module, 0x20);
    SSD1306_sendCommand(module, mode & 0x3);
}

void SSD1306_hvSetColumnAddress(u8 module, u8 start, u8 end)
{
    SSD1306_sendCommand(module, 0x21);
    SSD1306_sendCommand(module, start & DISPLAY_WIDTH_MASK);
    SSD1306_sendCommand(module, end & DISPLAY_WIDTH_MASK);
}

void SSD1306_hvSetPageAddress(u8 module, u8 start, u8 end)
{
    SSD1306_sendCommand(module, 0x22);
    SSD1306_sendCommand(module, start & DISPLAY_ROW_MASK);
    SSD1306_sendCommand(module, end & DISPLAY_ROW_MASK);
}

void SSD1306_pamSetPageStart(u8 module, u8 address)
{
    SSD1306_sendCommand(module, 0xB0 | (address & DISPLAY_ROW_MASK));
}

void SSD1306_setDisplayClockRatioAndFrequency(u8 module, u8 ratio, u8 frequency)
{
    SSD1306_sendCommand(module, 0xD5);
    SSD1306_sendCommand(module, (ratio & 0x0F) | ((frequency & 0x0F) << 4));
}

void SSD1306_setPrechargePeriod(u8 module, u8 phase1, u8 phase2)
{
    SSD1306_sendCommand(module, 0xD9);
    SSD1306_sendCommand(module, (phase1 & 0x0F) | ((phase2 & 0x0F ) << 4));
}

void SSD1306_setVcomhDeselectLevel(u8 module, u8 level)
{
    SSD1306_sendCommand(module, 0xDB);
    SSD1306_sendCommand(module, (level & 0x03) << 4);
}

void SSD1306_nop(u8 module)
{
    SSD1306_sendCommand(module, 0xE3);
}

void SSD1306_setChargePumpEnable(u8 module, u8 enable)
{
    SSD1306_sendCommand(module, 0x8D);
    SSD1306_sendCommand(module, enable ? 0x14 : 0x10);
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
            OLED D/C#   connected to Pinguino PMA[0:15] (dc)
            OLED W/R#   connected to Pinguino PMRD/PMWR
            OLED E/RD#  connected to GND
            OLED D[7:0] connected to Pinguino PMD[7:0]
            SSD1306.init(PMP6800, rst, dc);
            
    if MODULE = PMP8080
            OLED CS#    connected to GND
            OLED RES#   connected to any GPIO (res)
            OLED D/C#   connected to Pinguino PMA1
            OLED W/R#   connected to Pinguino PMWR
            OLED E/RD#  connected to GND
            OLED D[7:0] connected to Pinguino PMD[7:0]
            SSD1306.init(PMP6800, rst);
            
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
            SSD1306.init(SPISW, rst, dc, sda, sck, cs);
    
    if MODULE = SPIx (SPI1, SPI2, ...)
            SSD1306.init(SPI1, rst, dc);
    ------------------------------------------------------------------*/

void SSD1306_init(u8 module, ...)
{
    u8 RES, DC;
    u8 sda, sck, cs;
    u8 RW, E, RD;
    //u8 d0, d1, d2, d3, d4, d5, d6, d7;

    va_list args;
    
    SSD1306_MOD = module;
    
    va_start(args, module); // args points on the argument after module

    // init SPI communication

    switch (module)
    {
        case SPISW:
            sda = va_arg(args, u8);         // get the next arg
            sck = va_arg(args, u8);         // get the next arg
            cs  = va_arg(args, u8);         // get the last arg
            SPI_setBitOrder(module, SPI_MSBFIRST);
            SPI_begin(module, sda, sck, cs);
            break;
            
        case SPI1:
        case SPI2:
            SPI_setMode(module, SPI_MASTER);
            SPI_setDataMode(module, SPI_MODE1);
            //maximum baud rate possible = FPB = FOSC/4
            SPI_setClockDivider(module, SPI_CLOCK_DIV4);
            SPI_begin(module);
            break;

        case I2C:
            SSD1306_I2C_ADDR = va_arg(args, u8);
            I2C_init(I2C_MASTER_MODE, I2C_100KHZ);
            break;

        case PMP6800:
        case PMP8080:
            RES = va_arg(args, u8);
            DC = va_arg(args, u8);
            digitalwrite(RES, HIGH);
            pinmode(RES, OUTPUT);               // output

            if   (module == PMP6800)
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
            PMP_setAddress(DC);                 // DC
            PMP_autoIncrement(0);
            PMP_setWaitStates(4, 16, 4);        // WR strobe must be at least 60ns
            PMP_init();
            break;

        case PORTB6800 :
        case PORTD6800 :
        default:
            //void SSD1306_init(u8 res, u16 dc, u8 d0, u8 d1, u8 d2, u8 d3, u8 d4, u8 d5, u8 d6, u8 d7)
            #define DATA		LATD    // RB0 to RB7
            #define dDATA		TRISD

            dDATA = 0x00;           // output
            dCMD  = 0x00;           // output
            DATA  = 0x00;
            CMD   = 0xFF;           // every pin HIGH
            break;
    }
    va_end(args);           // cleans up the list
    
    // default Screen Values

    //SSD1306[module].orientation   = PORTRAIT;
    SSD1306[module].cursor.x      = 0;
    SSD1306[module].cursor.y      = 0;
    SSD1306[module].cursor.page   = 0;
    SSD1306[module].screen.startx = 0;
    SSD1306[module].screen.starty = 0;
    SSD1306[module].screen.endx   = DISPLAY_WIDTH  - 1;
    SSD1306[module].screen.endy   = DISPLAY_HEIGHT - 1;
    SSD1306[module].screen.width  = DISPLAY_WIDTH;
    SSD1306[module].screen.height = DISPLAY_HEIGHT;

    /** reset device
    When RES input is low, the chip is initialized with the following status:
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
    if (SSD1306_MOD != I2C)
    {
        digitalwrite(RES, LOW);    // initialized the chip
        Delayus(50);               // for at least 3us
        digitalwrite(RES, HIGH);
    }

    SSD1306_displayOn(module);
    Delayms(100);               // wait for SEG/COM to be ON
    SSD1306_displayOff(module);

    SSD1306_setDisplayClockRatioAndFrequency(module, 0, 8);
    SSD1306_setMultiplexRatio(module, 0x3F); // 1/64 duty
    SSD1306_setPrechargePeriod(module, 0xF, 0x01);
    SSD1306_setDisplayOffset(module, 0);    
    SSD1306_setDisplayStartLine(module, 0);  
    SSD1306_setChargePumpEnable(module, 1);    
    SSD1306_setMemoryAddressingMode(module, 0); // horizontal addressing mode; across then down
    SSD1306_setSegmentRemap(module, 1);
    SSD1306_setComOutputScanDirection(module, 1);
    SSD1306_setComPinsHardwareConfiguration(module, 1, 0);
    SSD1306_setContrast(module, 0xFF);
    SSD1306_setVcomhDeselectLevel(module, 1);

    SSD1306_wake(module);
    SSD1306_setInverse(module, 0);

    SSD1306_hvSetColumnAddress(module, 0, 127);
    SSD1306_hvSetPageAddress(module, 0, 7);

    SSD1306_pamSetStartAddress(module, 0);
    SSD1306_pamSetPageStart(module, 0);

    // SSD1306_setPrechargePeriod(module, 2, 2);
}

///	--------------------------------------------------------------------
/// Update the display
///	--------------------------------------------------------------------

void SSD1306_refresh(u8 module)
{
    u16 i;

    SSD1306_hvSetColumnAddress(module, 0, DISPLAY_WIDTH-1);
    SSD1306_hvSetPageAddress(module, 0, DISPLAY_ROWS-1);

    for (i = 0; i < DISPLAY_SIZE; i++)
        SSD1306_sendData(module, SSD1306_buffer[i]);
}

///	--------------------------------------------------------------------
/// Clear the display
///	--------------------------------------------------------------------

void SSD1306_clearScreen(u8 module)
{
    u16 i;

    for (i = 0; i < DISPLAY_SIZE; i++)
        SSD1306_buffer[i] = 0;
        
    SSD1306[module].cursor.x    = 0;
    SSD1306[module].cursor.y    = 0;
    SSD1306[module].cursor.page = 0;
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
    SSD1306_sendCommand(module, start & DISPLAY_ROW_MASK);

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
    SSD1306_sendCommand(module, end & DISPLAY_ROW_MASK);

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
    SSD1306_sendCommand(module, start & DISPLAY_ROW_MASK);

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
    SSD1306_sendCommand(module, end & DISPLAY_ROW_MASK);

    SSD1306_sendCommand(module, vertical_offset);    

    // activate scroll
    SSD1306_sendCommand(module, SSD1306_ACTIVATE_SCROLL);
}

void SSD1306_stopScroll(u8 module)
{
    // all scroll configurations are removed from the display when executing this command.
    SSD1306_sendCommand(module, SSD1306_DEACTIVATE_SCROLL);
}

// Right handed 1-row scroll
// The display is 16 rows tall.
void SSD1306_scrollRight(u8 module)
{
    SSD1306_startHorizontalScroll(module, SSD1306_RIGHT_HORIZONTAL_SCROLL, 0, 7, 2);
}

// Left handed 1-row scroll
// The display is 16 rows tall
void SSD1306_scrollLeft(u8 module)
{
    SSD1306_startHorizontalScroll(module, SSD1306_LEFT_HORIZONTAL_SCROLL, 0, 7, 2);
}

// Up handed 1-row scroll
// The display is 16 rows tall.
void SSD1306_scrollUp(u8 module)
{
    u8 x, y;

    for (y = 1; y <= 7; y++)
    {
        for (x = 0; x < 128; x++)
        {
            SSD1306_buffer[x + 128 * (y - 1)] = SSD1306_buffer[x + 128 * y];
        }
    }

    for (x = 0; x < 128; x++)
    {
        SSD1306_buffer[x + 128 * 7] = 0;
    }    

    SSD1306[module].cursor.y--;
}

// Down handed 1-row scroll
// The display is 16 rows tall
void SSD1306_scrollDown(u8 module)
{
    SSD1306_startVerticalAndHorizontalScroll(module, SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL, 0, 64, 0, 1);
}

///	--------------------------------------------------------------------
/// Print functions
///	--------------------------------------------------------------------

void SSD1306_setFont(u8 module, const u8 *font)
{
    SSD1306[module].font.address = font;
    //SSD1306[module].font.width   = SSD1306_getFontWidth()+1;
    //SSD1306[module].font.height  = SSD1306_getFontHeight();
    SSD1306[module].font.width   = font[0];
    SSD1306[module].font.height  = font[1];
}

/*	--------------------------------------------------------------------
    DESCRIPTION:
        write a char at (SSD1306[module].cursor.x, SSD1306[module].cursor.y)
    PARAMETERS:
        * c ascii code of the character to print
    RETURNS:
    REMARKS:
------------------------------------------------------------------*/

void printChar(u8 c)
{
    SSD1306_printChar(SSD1306_SPI, c);
}

void SSD1306_printChar(u8 module, u8 c)
{
    u8  b;

    while (SSD1306[module].cursor.x >= (DISPLAY_WIDTH / SSD1306[module].font.width))
    {
        SSD1306[module].cursor.x -= (DISPLAY_WIDTH / SSD1306[module].font.width);
        SSD1306[module].cursor.y++;            
    }

    while (SSD1306[module].cursor.y > 7) // replace by SSD1306[module].font.height ???
    {
        SSD1306_scrollUp();            
    }

    switch (c)
    {
        case '\n':
            SSD1306[module].cursor.y++;
            break;
            
        case '\r':
            SSD1306[module].cursor.x = 0;
            break;
            
        case '\t':
            SSD1306[module].cursor.x = (SSD1306[module].cursor.x + 4) % 4;
            break;
            
        default:
            for (b = 0; b < SSD1306[module].font.width; b++)
            {
                SSD1306_buffer[SSD1306[module].cursor.x * SSD1306[module].font.width + SSD1306[module].cursor.y * 128 + b] = 
                    SSD1306[module].font.address[2 + (c - 32) * SSD1306[module].font.width + b];
            }
        
        SSD1306[module].cursor.x++;
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

#if defined(SSD1306PRINT)       || defined(SSD1306PRINTLN)    || \
    defined(SSD1306PRINTNUMBER) || defined(SSD1306PRINTFLOAT)
void SSD1306_print(u8 module, u8 *string)
{
    while (*string != 0)
        SSD1306_printChar(u8 module, *string++);
}
#endif

#if defined(SSD1306PRINTLN)
void SSD1306_println(u8 module, u8 *string)
{
    SSD1306_print(u8 module, string);
    SSD1306_print(u8 module, "\n\r");
}
#endif

#if defined(ST7735PRINTCENTER)
void SSD1306_printCenter(u8 module, const u8 *string)
{
    u8 strlen, nbspace;
    const u8 *p;

    for (p = string; *p; ++p);
    strlen = p - string;

    nbspace = (SSD1306[module].screen.width / SSD1306[module].font.width - strlen) / 2;
    
    // write spaces before
    while(nbspace--)
        SSD1306_printChar(module, 32);

    // write string
    SSD1306_print(module, string);
    SSD1306_print(module, (u8*)"\n\r");
}
#endif

#if defined(SSD1306PRINTNUMBER) || defined(SSD1306PRINTFLOAT)
void SSD1306_printNumber(u8 module, long value, u8 base)
{  
    SSD1306_MOD = module;
    printNumber(value, base);
}
#endif

#if defined(SSD1306PRINTFLOAT)
void SSD1306_printFloat(u8 module, float number, u8 digits)
{ 
    SSD1306_MOD = module;
    printFloat(number, digits);
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

/*	--------------------------------------------------------------------
    DESCRIPTION:
        Sets the cursor to the specified x,y position
    PARAMETERS:
        0 < x < 319 if 0 < y < 239
    or	0 < x < 239 if 0 < y < 319
    RETURNS:
    REMARKS:
    TODO:
        check x and y
------------------------------------------------------------------*/

void SSD1306_setCursor(u8 module, u8 x, u8 y)
{
    if ( x >= DISPLAY_WIDTH || y >= (DISPLAY_HEIGHT) ) return;

    SSD1306[module].cursor.x = x;
    SSD1306[module].cursor.y = y;
    SSD1306[module].cursor.page = x & DISPLAY_ROW_MASK;  // current page
}

#ifdef SSD1306GRAPHICS
/*	--------------------------------------------------------------------
    DESCRIPTION:
        Draws a pixel with current color.
    PARAMETERS:
        x,y coord.
    RETURNS:
    REMARKS:
------------------------------------------------------------------*/

void SSD1306_drawPixel(u8 module, u8 x, u8 y)
{
    if ( x >= DISPLAY_WIDTH || y >= (DISPLAY_HEIGHT) ) return;

    SSD1306_buffer[x + (y / DISPLAY_ROWS) * DISPLAY_WIDTH] |= 1 << (y % DISPLAY_ROWS);
}

void SSD1306_clearPixel(u8 module, u8 x, u8 y)
{
    if (x >= DISPLAY_WIDTH || y >= (DISPLAY_HEIGHT) ) return;
    
    SSD1306_buffer[x + (y / DISPLAY_ROWS) * DISPLAY_WIDTH] &= ~(1 << (y % DISPLAY_ROWS));
}

/*	--------------------------------------------------------------------
    DESCRIPTION:
        Returns pixel color at x,y position.
    PARAMETERS:
        x,y coord.
    RETURNS:
        color
    REMARKS:
------------------------------------------------------------------*/

u8 SSD1306_getColor(u8 module, u8 x, u8 y)
{
    u8 column,temp,bit_mask;//page;
    unsigned int page;
    u8* GDptr = SSD1306_bufferptr;
   
    // Convert coordinates
    column = x & DISPLAY_WIDTH_MASK; // 0-128
    //page = (y>>DISPLAY_ROW_BITS)&DISPLAY_ROW_MASK; // 0-8
    temp = y & DISPLAY_ROW_MASK;
    bit_mask = 1<<temp;

    //GDptr+= column+(DISPLAY_WIDTH*page);  

    // Optimise
    page = (y << (DISPLAY_WIDTH_BITS - DISPLAY_ROW_BITS)); // 0-8 * 128
    GDptr += column+page;

    // Set the bit
    if (*GDptr & bit_mask) // Pixel on
    {
            return 1;
    }
    else // Pixel off
    {
            return 0;
    }
}

/*
void SSD1306_drawBitmap(u16 x, u16 y, u16 w, u16 h, u16* bitmap)
{
}
*/

/*	--------------------------------------------------------------------
    DESCRIPTION:
        Graphic routines based on drawPixel in graphics.c
    PARAMETERS:
    RETURNS:
    REMARKS:
------------------------------------------------------------------*/

void drawPixel(u16 x, u16 y)
{
    SSD1306_drawPixel(SSD1306_SPI, x, y);
}

void drawHLine(u16 x, u16 y, u16 w)
{
    SSD1306_drawHLine(SSD1306_SPI, x, y, w);
}

void drawVLine(u16 x, u16 y, u16 h)
{
    SSD1306_drawVLine(SSD1306_SPI, x, y, h);
}

void SSD1306_drawLine(u8 module, u16 x0, u16 y0, u16 x1, u16 y1)
{
    SSD1306_SPI = module;
    drawLine(x0, y0, x1, y1);
}

void SSD1306_drawRect(u8 module, u16 x1, u16 y1, u16 x2, u16 y2)
{
    SSD1306_SPI = module;
    drawRect(x1, y1, x2, y2);
}

void SSD1306_drawRoundRect(u8 module, u16 x1, u16 y1, u16 x2, u16 y2)
{
    SSD1306_SPI = module;
    drawRoundRect(x1, y1, x2, y2);
}

void SSD1306_drawCircle(u8 module, u16 x, u16 y, u16 radius)
{
    SSD1306_SPI = module;
    drawCircle(x, y, radius);
}

void SSD1306_fillCircle(u8 module, u16 x, u16 y, u16 radius)
{
    SSD1306_SPI = module;
    fillCircle(x, y, radius);
}

void SSD1306_fillRect(u8 module, u16 x1, u16 y1, u16 x2, u16 y2)
{
    SSD1306_SPI = module;
    fillRect(x1, y1, x2, y2);
}

void SSD1306_fillRoundRect(u8 module, u16 x1, u16 y1, u16 x2, u16 y2)
{
    SSD1306_SPI = module;
    fillRoundRect(x1, y1, x2, y2);
}

#ifdef ST7735DRAWBITMAP
void SSD1306_drawBitmap(u8 module1, u8 module2, const u8* filename, u16 x, u16 y)
{
    SSD1306_SPI = module1;
    drawBitmap(module2, filename, x, y);
}
#endif

#endif // SSD1306GRAPHICS

#endif /* __SSD1306_C */
