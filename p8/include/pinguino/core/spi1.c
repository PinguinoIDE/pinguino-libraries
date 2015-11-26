/*	----------------------------------------------------------------------------
    FILE:			spi1.c
    PROJECT:		pinguino
    PURPOSE:		Include all functions to handle SPI communication
                    Master and Slave
    PROGRAMER:		Régis Blanchot
    FIRST RELEASE:	03 Apr. 2010
    LAST RELEASE:	01 Oct. 2015
    ----------------------------------------------------------------------------
    CHANGELOG
    01 Oct. 2015    RB  added SPI2 support
    ----------------------------------------------------------------------------
    TODO
    * added 16F1459 support
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

#ifndef __SPI1_C__
#define __SPI1_C__

#ifndef __SPI1__
#define __SPI1__
#endif

#include <spi.h>
#include <spi.c>
#include <delayms.c>
#include <digitalp.c>
#include <digitalw.c>

#define SPI1_setPin(sda, sck)       SPI_setPin(SPI1, sda, sck)
#define SPI1_select()               SPI_select(SPI1)
#define SPI1_deselect()             SPI_deselect(SPI1)
#define SPI1_begin()                SPI_begin(SPI1)
#define SPI1_setBitOrder(bitorder)  SPI_setBitOrder(SPI1, bitorder)
#define SPI1_setDataMode(mode)      SPI_setDataMode(SPI1, mode)
#define SPI1_setMode(mode)          SPI_setMode(SPI1, mode)
#define SPI1_setClockDivider(div)   SPI_setClockDivider(SPI1, div)
#define SPI1_write(datax)           SPI_write(SPI1, datax)
#define SPI1_read()                 SPI_write(SPI1, 0xFF)

#endif // __SPI1_C__
