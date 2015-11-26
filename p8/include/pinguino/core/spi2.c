/*	----------------------------------------------------------------------------
    FILE:			spi2.c
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

#ifndef __SPI2_C__
#define __SPI2_C__

#ifndef __SPI2__
#define __SPI2__
#endif

#if !defined(__18f26j50) && !defined(__18f46j50) && \
    !defined(__18f27j53) && !defined(__18f47j53)
    #error "*** No SPI2 for this microcontroller ***"
#endif

#include <spi.h>
#include <spi.c>
#include <delayms.c>
#include <digitalp.c>
#include <digitalw.c>

#define SPI2_setPin(sda, sck)       SPI_setPin(SPI2, sda, sck)
#define SPI2_select()               SPI_select(SPI2)
#define SPI2_deselect()             SPI_deselect(SPI2)
#define SPI2_begin()                SPI_begin(SPI2)
#define SPI2_setBitOrder(bitorder)  SPI_setBitOrder(SPI2, bitorder)
#define SPI2_setDataMode(mode)      SPI_setDataMode(SPI2, mode)
#define SPI2_setMode(mode)          SPI_setMode(SPI2, mode)
#define SPI2_setClockDivider(div)   SPI_setClockDivider(SPI2, div)
#define SPI2_write(datax)           SPI_write(SPI2, datax)
#define SPI2_read()                 SPI_write(SPI2, 0xFF)

#endif // __SPI2_C__
