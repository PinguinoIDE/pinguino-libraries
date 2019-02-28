/*  --------------------------------------------------------------------
    FILE:           spi.c
    PROJECT:        pinguino
    PURPOSE:        Functions to handle SPI communication
                    Master and Slave
    --------------------------------------------------------------------
    CHANGELOG
    03 Apr. 2010 - Régis Blanchot - first release
    01 Oct. 2015 - Régis Blanchot - added SPI2 support
    30 Nov. 2015 - Régis Blanchot - added PIC16F1459 support
    22 Jan. 2016 - Régis Blanchot - removed setPin(), extended begin() with vargs
    25 Jan. 2017 - Régis Blanchot - prepared SPI Sofware Data-In support (SDI)
    01 Feb. 2018 - Régis Blanchot - added SPI_writeBytes() and SPI_readBytes()
                                  - added SPI_writeChar() and SPI_readChar()
    --------------------------------------------------------------------
    TODO
    * SPI Sofware read function
    --------------------------------------------------------------------
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
    ------------------------------------------------------------------*/

#ifndef __SPI_C__
#define __SPI_C__

#ifndef __SPI__
#define __SPI__
#endif

#include <stdarg.h>
#include <compiler.h>
#include <const.h>              // SPISW, SPI1, SPI2
#include <macro.h>              // BitSet, BitClear, ...
#include <spi.h>
//#include <delayms.c>
#include <delayus.c>
#include <digitalp.c>
#include <digitalw.c>
#include <digitalr.c>

/*
            while (1)
            {
                digitalwrite(USERLED, HIGH);
                Delayms(100);
                digitalwrite(USERLED, LOW);
                Delayms(900);
            }
*/

#define TEMPO   50

/**
 * This function initializes the SPI hardware configuring polarity and edge
 * of transition using Standard SPI Mode Terminology (datasheet Table 19-1 p206)
 *
 *    Mode0,Mode1 CKP   CKE
 *    0,0         0     1
 *    0,1         0     0
 *    1,0         1     1
 *    1,1         1     0
 *
 * It's possible to use LOW or HIGH for mode0 to indicate the idle state
 * clock level, and RISING or FALLING to indicate when the transmit should
 * take place.
 *
 * The sync_mode param could be any of SPI_MASTER, MASTER_FOSC_4, MASTER_FOSC_16,
 * MASTER_FOSC_64, SLAVE_SS or SLAVE, where MASTER_FOSC_X indicates the SPI
 * clock speed used. If you want to use the /SS pin in slave mode you should
 * initialize the SPI using SPI_SLAVE_SS.
 sync_mode   description
   0       SPI_MASTER mode, clock = FOSC_4
   1       SPI_MASTER, clock = FOSC_16
   2       SPI_MASTER, clock = FOSC_64
   3       SPI_MASTER, clock = TMR2 output/2
   4       SPI_SLAVE mode, /SS pin enabled
   5       SPI_SLAVE, /SS pin disabled
 * smp_phase = Sample bit
   0  in slave mode
   0  in master mode : Input data sampled at middle of data output time
   1  in master mode : Input data sampled at end of data output time
   Nota bene : enter 128 as decimal value or 0x80 as hexadecimal value for the below smp_phase parameter
 */


/**
 *  This function initializes the SPI module to default values
 *  Called from main.c
 */

void spi_init()
{
    u8 i;

    for (i=0; i<NUMOFSPI; i++)
    {
        _spi[i].mode     = SPI_MODE0;
        _spi[i].divider  = SPI_CLOCK_DIV4;
        _spi[i].role     = SPI_MASTER;
        _spi[i].bitorder = SPI_MSBFIRST;
        _spi[i].phase    = SPI_HIGH_SPEED_MODE;
    }
}

/**
 *  SPI Software Clock function
 */

void SPI_pulse()
{
    u8 mode = _spi[SPISW].mode;

    if (mode == SPI_MODE0 || mode == SPI_MODE2)
        mode = SPI_RISING_EDGE;
    else
        mode = SPI_FALLING_EDGE;

    digitalwrite(_spi[SPISW].sck, !mode);
    Delayus(TEMPO);
    digitalwrite(_spi[SPISW].sck, mode);
    Delayus(TEMPO);
}

/**
 *  This function selects a SPI module
 */

void SPI_select(u8 module)
{
    switch(module)
    {
        case SPISW:
            digitalwrite(_spi[SPISW].cs, LOW);
            break;

        case SPI1:
            CSPIN = LOW;
            break;

        #if defined(__18f26j50)|| defined(__18f46j50) || \
            defined(__18f27j53)|| defined(__18f47j53)

        case SPI2:
            CS2PIN = LOW;
            break;
            
        #endif
    }
}

/**
 *  This function deselects a SPI module
 */

void SPI_deselect(u8 module)
{
    switch(module)
    {
        case SPISW:
            digitalwrite(_spi[SPISW].cs, HIGH);
            break;

        case SPI1:
            CSPIN = HIGH;
            break;

        #if defined(__18f26j50)|| defined(__18f46j50) || \
            defined(__18f27j53)|| defined(__18f47j53)

        case SPI2:
            CS2PIN = HIGH;
            break;
            
        #endif
    }
}

//#ifdef SPIBEGIN
void SPI_begin(int module, ...)
{
    va_list args;
    
    va_start(args, module); // args points on the argument after module

    // Reset the module
    //SPI_close(module);
    //SPI_deselect(module);
    
    if (module == SPISW)
    {
            _spi[SPISW].sdo = va_arg(args, int);
            _spi[SPISW].sdi = va_arg(args, int);
            _spi[SPISW].sck = va_arg(args, int);
            _spi[SPISW].cs  = va_arg(args, int);
            pinmode(_spi[SPISW].sdo, OUTPUT);
            pinmode(_spi[SPISW].sdi, INPUT);
            pinmode(_spi[SPISW].sck, OUTPUT);
            pinmode(_spi[SPISW].cs,  OUTPUT);
    }

    else if (module == SPI1)
    {
        SSPCON1bits.SSPEN = 0;          // first disables serial port and configures SPI pins as I/O pins
        SSPSTAT = 0x00;                 // power on state (SMP=0, CKE=0) 
        SSPCON1 = _spi[SPI1].divider;   // select serial speed
        SSPSTATbits.SMP = _spi[SPI1].phase;

        switch(_spi[SPI1].mode)
        {
            case SPI_MODE0:             // SPI bus mode 0,0
              SSPCON1bits.CKP = 0;      // clock idle state low
              SSPSTATbits.CKE = 0;      // data transmitted on rising edge
              break;
              
            case SPI_MODE1:             // default SPI bus mode 0,1
              SSPCON1bits.CKP = 0;      // clock idle state low
              SSPSTATbits.CKE = 1;      // data transmitted on falling edge
              break;
              
            case SPI_MODE2:             // SPI bus mode 1,0
              SSPCON1bits.CKP = 1;      // clock idle state high
              SSPSTATbits.CKE = 0;      // data transmitted on rising edge
              break;
              
            case SPI_MODE3:             // SPI bus mode 1,1
              SSPCON1bits.CKP = 1;      // clock idle state high
              SSPSTATbits.CKE = 1;      // data transmitted on falling edge
              break;
        }

        SDIPIN = INPUT;                 // define SDI pin as input
        SDOPIN = OUTPUT;                // define SDO pin as output

        switch(_spi[SPI1].role)
        {
            case SPI_SLAVE_SS:          // slave mode w /SS enable
                SSPIN  = INPUT;         // define /SS pin as input
                SCKPIN = INPUT;         // define clock pin as input
                break;

            case SPI_SLAVE:             // slave mode w/o /SS enable
                SCKPIN = INPUT;         // define clock pin as input
                break;

            default:                    // master mode, define clock en SS pin as output
                SSPIN  = OUTPUT;        // define SS  pin as output
                SCKPIN = OUTPUT;        // define clock pin as output
                break;
        }

        SSPCON1bits.SSPEN = 1;          // finally, enables serial port and configures SPI pins
        //Delayms(30);

        #ifdef SPIINT
            //IPR1bits.SSPIP = 1;
            SSP1INTFLAG = 0;
            PIE1bits.SSPIE = 1;
            interrupts();
        #endif
    } // end if SPI1

    #if defined(__18f26j50)|| defined(__18f46j50) || \
        defined(__18f27j53)|| defined(__18f47j53)

    else if (module == SPI2)
    {
        SSP2CON1bits.SSPEN = 0;         // first disables serial port and configures SPI pins as I/O pins
        SSP2STAT = 0x00;                // power on state (SMP=0, CKE=0) 
        SSP2CON1 = _spi[SPI2].divider;  // select serial mode
        SSP2STATbits.SMP = _spi[SPI2].phase;

        switch(_spi[SPI2].mode)
        {
            case SPI_MODE0:             // SPI bus mode 0,0
                SSP2CON1bits.CKP = 0;   // clock idle state low
                SSP2STATbits.CKE = 0;   // data transmitted on falling edge
                break;
              
            case SPI_MODE1:             // default SPI bus mode 0,1
                SSP2CON1bits.CKP = 0;   // clock idle state low
                SSP2STATbits.CKE = 1;   // data transmitted on falling edge
                break;
              
            case SPI_MODE2:             // SPI bus mode 1,0
                SSP2CON1bits.CKP = 1;   // clock idle state high
                SSP2STATbits.CKE = 0;   // data transmitted on rising edge
                break;
              
            case SPI_MODE3:             // SPI bus mode 1,1
                SSP2CON1bits.CKP = 1;   // clock idle state high
                SSP2STATbits.CKE = 1;   // data transmitted on falling edge
                break;
        }

        SDI2PIN = INPUT;                // define SDI pin as input
        SDO2PIN = OUTPUT;               // define SDO pin as output

        switch(_spi[SPI2].role)
        {
            case SPI_SLAVE_SS:          // slave mode w /SS enable
                SS2PIN  = INPUT;        // define /SS pin as input
                SCK2PIN = INPUT;        // define clock pin as input
                break;

            case SPI_SLAVE:             // slave mode w/o /SS enable
                SCK2PIN = INPUT;        // define clock pin as input
                break;

            default:                    // master mode, define clock en SS pin as output
                SS2PIN  = OUTPUT;       // define SS  pin as output
                SCK2PIN = OUTPUT;       // define clock pin as output
                break;
        }

        SSP2CON1bits.SSPEN = 1;         // finally, enables serial port and configures SPI pins
        //Delayms(30);

        #ifdef SPIINT
            PIE3bits.SSP2IE = 1;
            INTCONbits.GIEH = 1;
            INTCONbits.GIEL = 1;
            INTCONbits.PEIE = 1;
            //IPR1bits.SSPIP = 1;
            PIR3bits.SSP1INTFLAG = 0;
        #endif
    } // end if SPI2
    #endif
    
    va_end(args);           // cleans up the list
}
//#endif

/**
 * Disable all SPIx interrupts
 * Stops and resets the SPIx
 * Clears the receive buffer
 **/

#ifdef SPICLOSE
void SPI_close(u8 module)
{
    u8 rData;
    
    switch(module)
    {
        case SPI1:
            #ifdef SPIINT
            // 1.  Disable the SPI interrupts in the respective IEC0/1 register.
            IntDisable(INT_SPI1_FAULT); 
            IntDisable(INT_SPI1_TRANSFER_DONE); 
            IntDisable(INT_SPI1_RECEIVE_DONE);
            #endif
            // 2.  Stop and reset the SPI module by clearing the ON bit.
            SSPCON1 = 0;
            // 3.  Clear the receive buffer.
            rData = SSP1BUF;
            break;
        
        #if defined(__18f26j50)|| defined(__18f46j50) || \
            defined(__18f27j53)|| defined(__18f47j53)
            
        case SPI2:
            #ifdef SPIINT
            // 1.  Disable the SPI interrupts in the respective IEC0/1 register.
            IntDisable(INT_SPI2_FAULT); 
            IntDisable(INT_SPI2_TRANSFER_DONE); 
            IntDisable(INT_SPI2_RECEIVE_DONE);
            #endif
            // 2.  Stop and reset the SPI module by clearing the ON bit.
            SSP2CON1 = 0;
            // 3.  Clear the receive buffer.
            rData = SSP2BUF;
            break;
            
        #endif
    }
}
#endif

/**
 * This function sets the order of the bits shifted out of and into the SPI bus,
 * either LSBFIRST (least-significant bit first) or MSBFIRST (most-significant bit first). 
 */

//#ifdef SPISETBITORDER
#define SPI_setBitOrder(m, bo) _spi[m].bitorder = bo
//#endif

/**
 * This function sets the SPI data mode (clock polarity and phase)
 * Modes available are SPI_MODE0, SPI_MODE1, SPI_MODE2, or SPI_MODE3
 * Mode    CKP    CKE
 * 0       0       0
 * 1       0       1
 * 2       1       0
 * 3       1       1
 */

//#ifdef SPISETDATAMODE
#define SPI_setDataMode(module, mo)   _spi[module].mode = mo

//#endif

/**
 * This function sets the SPI mode.
 * Possible values are SPI_MASTER or SPI_SLAVE.
 * The default setting is SPI_MASTER.
 */

//#ifdef SPISETMODE
void SPI_setMode(u8 module, u8 mode)
{
    _spi[module].role  = mode;
    _spi[module].divider = mode;

    if (mode > SPI_CLOCK_DIV4)
        _spi[module].phase = SPI_STANDARD_SPEED_MODE;
    else
        _spi[module].phase = SPI_HIGH_SPEED_MODE;
}
//#endif

/**
 * This function sets the SPI clock divider relative to the system clock.
 * The dividers available are 4, 8, 16, 64.
 * The default setting is SPI_CLOCK_DIV4, which sets the SPI clock to one-quarter
 * the frequency of the system clock. 
 */

#define SPI_setClockDivider(module, divider)   SPI_setMode(module, divider)

//#ifdef SPISETCLOCKDIVIDER
/*
void SPI_setClockDivider(u8 module, u8 divider)
{
    _spi[module].role  = divider;
    _spi[module].divider = divider;

    if (divider > SPI_CLOCK_DIV4)
        _spi[module].phase = SPI_STANDARD_SPEED_MODE;
    else
        _spi[module].phase = SPI_HIGH_SPEED_MODE;
}
*/
//#endif

//#ifdef SPIWRITE
u8 SPI_write(u8 module, u8 dataout)
{
    u8 clear, i, bitMask;
    u8 datain = 0x00;

    switch (module)
    {
        case SPISW:
            // send data
            for (i = 0; i <= 7; i++)
            {
                if (_spi[SPISW].bitorder == SPI_MSBFIRST)
                    bitMask = (0x80 >> i);
                else
                    bitMask = (0x01 << i);
                
                digitalwrite(_spi[SPISW].sdo, (dataout & bitMask) ? 1:0);
                SPI_pulse();
            }
            
            // return answer
            for (i = 0; i <= 7; i++)
            {
                if (_spi[SPISW].bitorder == SPI_MSBFIRST)
                    bitMask = (0x80 >> i);
                else
                    bitMask = (0x01 << i);
                
                SPI_pulse();
                if (digitalread(_spi[SPISW].sdi))
                    datain |= bitMask;
            }

            return datain;

        case SPI1:
            clear = SSP1BUF;                // clears buffer
            SSP1INTFLAG = 0;                // enables SPI1 interrupt
            do                              // RB: 18-10-2016
            { 
                SSPCON1bits.WCOL = 0;       // must be cleared in software
                SSP1BUF = dataout;          // send data
            }
            while (SSPCON1bits.WCOL);       // we're still transmitting the previous data
            while (!SSP1INTFLAG);           // wait for transfer is complete

            /*
            if (SSPCON1bits.WCOL)           // still transmitting the previous data 
                return -1;                  // abort transmission
            else
                while (!SSP1INTFLAG);       // wait for transfer is complete
            */
            return SSP1BUF;                 // Return the Data received

        #if defined(__18f26j50)|| defined(__18f46j50) || \
            defined(__18f27j53)|| defined(__18f47j53)
            
        case SPI2:
            clear = SSP2BUF;                // clears buffer
            SSP2INTFLAG = 0;                // enables SPI2 interrupt
            do                              // RB: 18-02-2016
            { 
                SSP2CON1bits.WCOL = 0;      // must be cleared in software
                SSP2BUF = dataout;          // send data again
            }
            while (SSP2CON1bits.WCOL);      // we're still transmitting the previous data
            while (!SSP2INTFLAG);           // wait for transfer is complete

            /*
            SSP2CON1bits.WCOL = 0;          // clears colision flag
            SSP2BUF = dataout;              // send data
            if (SSP2CON1bits.WCOL)          // still transmitting the previous data 
                return -1;                  // abort transmission
            else
                while (!SSP2INTFLAG);       // wait for transfer is complete
            */
            return SSP2BUF;

        #endif
    }
    
    // error, not a valid SPI module
    return -1;
}

u8 SPI_writeChar(u8 module, u8 address, u8 val)
{
    u8 r;
    SPI_select(module);
    r = SPI_write(module, address);
    if (r)
        SPI_write(module, val);
    SPI_deselect(module);
    return r;
}

u8 SPI_writeBytes(u8 module, u8 address, u8* buffer, u8 length)
{
    u8 i, r;

    SPI_select(module);
    for(i = 0; i < length; i++)
    {
        SPI_write(module, address++);
        r = SPI_write(module, *buffer++);
    }
    SPI_deselect(module);
    return r;
}
//#endif

#define SPI_read(module) SPI_write(module, 0xFF)

u8 SPI_readChar(u8 module, u8 address)
{
    u8 val;

    SPI_select(module);
    SPI_write(module, address);
    val = SPI_read(module);
    SPI_deselect(module);

    return val;
}

u8 SPI_readBytes(u8 module, u8 address, u8 *buffer, u8 length)
{
    u8 i, r;

    SPI_select(module);
    r = SPI_write(module, address);
    if (r)
    {
        for(i = 0; i < length; i++)
            buffer[i] = SPI_read(module);
    }
    SPI_deselect(module);
    return r;
}

/***********************************************************************
 *  Interrupt routines 
 **********************************************************************/

#ifdef SPIINT

void spi1_interrupt()
{
    u8 c;
    if (SSP1INTFLAG)
    {
        c = SSP1BUF;
        SSP1BUF = SPI1_onEvent_func(c);
        SSP1INTFLAG = 0;
    }
}

static void SPI1_onEvent(u8(*func)(u8))
{
    SPI1_onEvent_func = func;
}

#if defined(__18f26j50)|| defined(__18f46j50) || \
    defined(__18f27j53)|| defined(__18f47j53)

void spi2_interrupt()
{
    u8 c;
    if (SSP2INTFLAG)
    {
        c = SSP2BUF;
        SSP2BUF = SPI2_onEvent_func(c);
        SSP2INTFLAG = 0;
    }
}

static void SPI2_onEvent(u8(*func)(u8))
{
    SPI2_onEvent_func = func;
}

#endif // __18f47j53__ ...

#endif // SPIINT

/**********************************************************************/

#endif // __SPI_C__
