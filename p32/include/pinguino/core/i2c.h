/*  --------------------------------------------------------------------
    FILE:           i2c.h
    PROJECT:        pinguino32
    PURPOSE:        I2C functions
    PROGRAMER:      Regis Blanchot <rblanchot@gmail.com>
    FIRST RELEASE:  04 Mar. 2011
    --------------------------------------------------------------------
    CHANGELOG :
    --------------------------------------------------------------------
    04/04/2012  alterations by RLmonitor
                tested on PIC32MX440 pinguino-OTG
                added speed modes 100kHz, 400kHz, 1 MHz
                changed completion flag polling to the various bits e.g. PEN, SEN
                I2C_readchar 2nd argument - NACK or ACK
                implemented init, start, restart, stop, writechar, readchar - don't need others
                suggest 2 new functions - writebytes(module, address, *bytes, count),
                readbytes(module, address, *bytes, count)
                NO interrupt facility - not sure that it is needed?
    29/04/2014  Alterations by Moreno Manzini as suggested by Djpark
                Create 2 versions of I2C_wait, normally is used the standard one.
                If definited I2CWAIT_WORKAROUND is used the second version which avoid potentially infinite loop.
                I2CWAIT_WORKAROUND is defined as default.
    23/03/2015  Regis Blanchot added I2C_master and I2C_slave functions
                Added support to PIC32MX270F256B
    11/08/2015  Robert Teschner added slave functions after Regis added
                interrupt methods. Fixed PIC32MX220 freezing after interrupt enable.
    --------------------------------------------------------------------
    TODO : further slave modes improvement in case of slave writing
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

#include <typedef.h>

// Mode I2C
#define I2C_WRITE               0
#define I2C_READ                1
#define I2C_MASTER_MODE         0
#define I2C_SLAVE_MODE          1
#define I2C_MULTIMASTER_MODE    2
#define I2C_SLEW_OFF            0
#define I2C_SLEW_ON             1

//RL added speed definitions
#define I2C_100KHZ              100000
#define I2C_400KHZ              400000
#define I2C_1MHZ                1000000
#define I2C_TIMEOUT             4000

#define I2C_7BIT_ADDRESS        7
#define I2C_10BIT_ADDRESS       10

// Module I2C
#define I2C1                    1
#define I2C2                    2

#define I2C_BUFFER_LENGTH       16        // @regis: I would guess 64 bits are far to much

/// PROTOTYPES

void I2C_master(u8, u32);
void I2C_slave(u8, u16, u32);
void I2C_init(u8, u8, u32);
//u8   I2C_send(u8, u8, u8);
//u8   I2C_get(u8, u16);
u8   I2C_writeChar(u8, u8);
u8   I2C_readChar(u8);//, u8);
//u8   I2C_read(u8);
void I2C_wait(u8);
void I2C_start(u8);
void I2C_stop(u8);
void I2C_restart(u8);
void I2C_sendNack(u8);
void I2C_sendAck(u8);

u8   I2C1Interrupt();
u8   I2C2Interrupt();

#define I2C1_master(speed)          I2C_init(I2C1, I2C_MASTER_MODE, speed)
#define I2C1_slave(DeviceID)        I2C_init(I2C1, I2C_SLAVE_MODE, DeviceID)
#define I2C1_init()                 I2C_init(I2C1, I2C_MASTER_MODE, I2C_100KHZ)
//#define I2C1_send(address, byte)    I2C_send(I2C1, address, byte)
//#define I2C1_get(address)           I2C_get(I2C1, address)
//#define I2C1_sendID(DeviceID, rw)   I2C_sendID(I2C1, DeviceID, rw)
#define I2C1_writeChar(byte)        I2C_writeChar(I2C1, byte)
#define I2C1_readChar()             I2C_readChar(I2C1)
#define I2C1_wait()                 I2C_wait(I2C1)
#define I2C1_start()                I2C_start(I2C1)
#define I2C1_stop()                 I2C_stop(I2C1)
#define I2C1_restart()              I2C_restart(I2C1)
#define I2C1_sendNack()             I2C_sendNack(I2C1)
#define I2C1_sendAck()              I2C_sendAck(I2C1)

#define I2C2_master(speed)          I2C_init(I2C2, I2C_MASTER_MODE, speed)
#define I2C2_slave(DeviceID)        I2C_init(I2C2, I2C_SLAVE_MODE, DeviceID)
#define I2C2_init()                 I2C_init(I2C2, I2C_MASTER_MODE, I2C_100KHZ)
//#define I2C2_send(address, byte)    I2C_send(I2C2, address, byte)
//#define I2C2_get(address)           I2C_get(I2C2, address)
//#define I2C2_sendID(DeviceID, rw)   I2C_sendID(I2C2, DeviceID, rw)
#define I2C2_writeChar(byte)        I2C_writeChar(I2C2, byte)
#define I2C2_readChar()             I2C_readChar(I2C2)
#define I2C2_wait()                 I2C_wait(I2C2)
#define I2C2_start()                I2C_start(I2C2)
#define I2C2_stop()                 I2C_stop(I2C2)
#define I2C2_restart()              I2C_restart(I2C2)
#define I2C2_sendNack()             I2C_sendNack(I2C2)
#define I2C2_sendAck()              I2C_sendAck(I2C2)

#endif	/* __I2C_H */
