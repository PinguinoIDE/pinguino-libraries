/*	----------------------------------------------------------------------------
    FILE:				i2c1.c
    PROJECT:			pinguino
    PURPOSE:			I2C1 functions
    PROGRAMER:			regis blanchot <rblanchot@gmail.com>
    FIRST RELEASE:		11 apr. 2011
    LAST RELEASE:		25 feb. 2012
    ----------------------------------------------------------------------------
    TODO :
    ----------------------------------------------------------------------------
    CHANGELOG :
        13 may. 2011 I2C_init default mode is master 400 khz jp.mandon@gmail.com
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

#ifndef __I2C1__
#define __I2C1__

#include <typedef.h>
#include <const.h>
#include <macro.h>
#include <system.c>
#include <i2c.c>

/// PROTOTYPES

u8   I2C1_write(u16, u8*, u8);

#define I2C1_master(speed)          I2C_init(I2C1, I2C_MASTER_MODE, speed)
#define I2C1_slave(DeviceID)        I2C_init(I2C1, I2C_SLAVE_MODE, DeviceID)
#define I2C1_init()                 I2C_init(I2C1, I2C_MASTER_MODE, I2C_100KHZ)
#define I2C1_send(address, byte)    I2C_send(I2C1, address, byte)
#define I2C1_get(address)           I2C_get(I2C1, address)
#define I2C1_sendID(DeviceID, rw)   I2C_sendID(I2C1, DeviceID, rw)
#define I2C1_writechar(byte)        I2C_writechar(I2C1, byte)
#define I2C1_readchar()             I2C_readchar(I2C1)
#define I2C1_wait()                 I2C_wait(I2C1)
#define I2C1_start()                I2C_start(I2C1)
#define I2C1_stop()                 I2C_stop(I2C1)
#define I2C1_restart()              I2C_restart(I2C1)
#define I2C1_sendNack()             I2C_sendNack(I2C1)
#define I2C1_sendAck()              I2C_sendAck(I2C1)

/*	----------------------------------------------------------------------------
    ---------- Writes bytes to the slave
    --------------------------------------------------------------------------*/

u8 I2C1_write(u16 address, u8* bytes, u8 length)
{
    u8 i, r;
    for (i=0; i<length; i++)
        r = I2C_write(I2C1, address, bytes++);
    return r;
}

#endif  /* __I2C1__ */

