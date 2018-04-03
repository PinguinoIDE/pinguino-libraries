/*  --------------------------------------------------------------------
    FILE:       i2c.h
    PROJECT:    Pinguino
    PURPOSE:    I2C communication for Master and Slave
    PROGRAMER:  Régis Blanchot
    --------------------------------------------------------------------
    25 Nov. 2016 - Régis Blanchot - created from i2c.c
    01 Feb. 2018 - Régis Blanchot - renamed writeChar() to write()
                                  - renamed readChar() to read()
                                  - added new writeChar() and readChar() functions
                                  - added I2C_writeBytes() and I2C_readBytes()
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

#ifndef __I2C_H
#define __I2C_H

#define __I2C__

#include <typedef.h>

#if defined(__18f46j50) || defined(__18f46j53) || defined(__18f47j53)
#define NUMOFI2C        2
#else
#define NUMOFI2C        1
#endif

// Modules (02-12-2016 - RB - moved to const.h)
//#define I2C1            1
//#define I2C2            2

#define I2C_WRITE       0
//TODO : replace with the following to avoid conflict with XC8 headers files
//#define I2C_WRITE_BIT   0
//#define I2C_READ_BIT  1

// Mode I2C
#define I2C_MASTER_MODE 0
#define I2C_SLAVE_MODE  1
#define I2C_SLEW_OFF    0
#define I2C_SLEW_ON     1

// Speed definitions
#define I2C_100KHZ      100
#define I2C_400KHZ      400
#define I2C_1MHZ        1000

// Status
#define I2C_READY 0
#define I2C_MRX   1
#define I2C_MTX   2
#define I2C_SRX   3
#define I2C_STX   4

/*
#if defined(__18f2550) || defined(__18f4550) || \
    defined(__18f26j50) || defined(__18f46j50)
#define SSPCONF1 SSPCON1
#define SSPCONF2 SSPCON2
#define SSPAD    SSPADD
#define SSPBF    SSPBUF
#define SSPSTSMP SSPSTATbits.SMP
#define SSPSTR_W SSPSTATbits.R_W
#define SSPACKEN SSPCON2bits.ACKEN
#define SSPACKST SSPCON2bits.ACKSTAT
#define SSPACKDT SSPCON2bits.ACKDT
#define SSPPEN   SSPCON2bits.PEN
#define SSPRCEN  SSPCON2bits.RCEN
#define SSPRSEN  SSPCON2bits.RSEN
#define SSPSEN   SSPCON2bits.SEN

#elif defined(__18f25k50) || defined(__18f45k50)
#define SSPCONF1 SSP1CON1
#define SSPCONF2 SSP1CON2
#define SSPAD    SSP1ADD
#define SSPBF    SSP1BUF
#define SSPSTSMP SSP1STATbits.SMP
#define SSPSTR_W SSP1STATbits.R_NOT_W
#define SSPACKEN SSP1CON2bits.ACKEN
#define SSPACKST SSP1CON2bits.ACKSTAT
#define SSPACKDT SSP1CON2bits.ACKDT
#define SSPPEN   SSP1CON2bits.PEN
#define SSPRCEN  SSP1CON2bits.RCEN
#define SSPRSEN  SSP1CON2bits.RSEN
#define SSPSEN   SSP1CON2bits.SEN

#else
    #error "Processor Not Yet Supported. Please, Take Contact with Developpers."
#endif
*/

/// PROTOTYPES

void I2C_master(u8, u16);
void I2C_slave(u8, u16);
void I2C_init(u8, u8, u16);

u8 I2C_write(u8, u8);
u8 I2C_writeChar(u8, u8, u8, u8);
u8 I2C_writeBytes(u8, u8, u8, u8*, u8);

u8 I2C_read(u8);
u8 I2C_readChar(u8, u8, u8);
u8 I2C_readBytes(u8, u8, u8, u8*, u8);

void I2C_wait(u8);
void I2C_idle(u8);
//u8 I2C_waitAck(u8);
void I2C_start(u8);
void I2C_stop(u8);
void I2C_restart(u8);
void I2C_sendNack(u8);
void I2C_sendAck(u8);

#define I2C1_master(x)      I2C_master(I2C1, x)
#define I2C1_slave(x)       I2C_slave(I2C1, x)
#define I2C1_init(x, y)     I2C_init(I2C1, x, y)
#define I2C1_write(x)       I2C_write(I2C1, x)
#define I2C1_writeChar(x)   I2C_writeChar(I2C1, x)
#define I2C1_read()         I2C_read(I2C1)
#define I2C1_readChar()     I2C_readChar(I2C1)
#define I2C1_wait()         I2C_wait(I2C1)
#define I2C1_idle()         I2C_idle(I2C1)
//#define I2C1_waitAck()    I2C_waitAck(I2C1)
#define I2C1_start()        I2C_start(I2C1)
#define I2C1_stop()         I2C_stop(I2C1)
#define I2C1_restart()      I2C_restart(I2C1)
#define I2C1_sendNack()     I2C_sendNack(I2C1)
#define I2C1_sendAck()      I2C_sendAck(I2C1)

// PIC18F with 2 I2C modules
#if defined(__18f26j50) || defined(__18f46j50) || \
    defined(__18f26j53) || defined(__18f46j53) || \
    defined(__18f27j53) || defined(__18f47j53)

#define I2C2_master(x)      I2C_master(I2C2, x)
#define I2C2_slave(x)       I2C_slave(I2C2, x)
#define I2C2_init(x, y)     I2C_init(I2C2, x, y)
#define I2C2_write(x)       I2C_write(I2C2, x)
#define I2C2_writeChar(x)   I2C_writeChar(I2C2, x)
#define I2C2_read()         I2C_read(I2C2)
#define I2C2_readChar()     I2C_readChar(I2C2)
#define I2C2_wait()         I2C_wait(I2C2)
#define I2C2_idle()         I2C_idle(I2C2)
//#define I2C2_waitAck()    I2C_waitAck(I2C2)
#define I2C2_start()        I2C_start(I2C2)
#define I2C2_stop()         I2C_stop(I2C2)
#define I2C2_restart()      I2C_restart(I2C2)
#define I2C2_sendNack()     I2C_sendNack(I2C2)
#define I2C2_sendAck()      I2C_sendAck(I2C2)

#endif

#endif // __I2C_H
