/*	--------------------------------------------------------------------
    FILE:			serial1.c
    PROJECT:		pinguino 32
    PURPOSE:		serial functions on UART1
    PROGRAMER:		jean-pierre mandon <jp.mandon@gmail.com>
    FIRST RELEASE:	01 Jan. 2011
    LAST RELEASE:	15 Jan. 2015
    --------------------------------------------------------------------
    18 Feb. 2012 jp mandon added support for PIC32-PINGUINO-220
    15 Jan. 2015 regis blanchot - renamed _PINGUINOSERIAL1_C_ in __SERIAL1__
    29 Jan. 2015 regis blanchot - fixed PIC32_PINGUINO_220 support
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

#ifndef __SERIAL1__
#define __SERIAL1__

#include <typedef.h>
#include <stdarg.h>
#include <serial.c>

void serial1init(u32 speed)
{
    #ifdef PIC32_PINGUINO_220
        SerialConfigure(UART2, UART_ENABLE,	UART_RX_TX_ENABLED,	speed);
    #else
        SerialConfigure(UART1, UART_ENABLE,	UART_RX_TX_ENABLED,	speed);
    #endif
}

void serial1printchar(u8 c)
{
    #ifdef PIC32_PINGUINO_220
        SerialPutChar(UART2, c);
    #else
        SerialPutChar(UART1, c);
    #endif
}

#if defined(SERIALPRINT) || defined(SERIALPRINTLN) || \
    defined(SERIALPRINTNUMBER) || defined(SERIALPRINTFLOAT)
void serial1print(char *string)
{
    #ifdef PIC32_PINGUINO_220
        SerialPrint(UART2, string);
    #else
        SerialPrint(UART1, string);
    #endif
}
#endif

#ifdef SERIALPRINTLN
void serial1println(u8 *string)
{
    #ifdef PIC32_PINGUINO_220
    SerialPrintln(UART2, string);
    #else
    SerialPrintln(UART1, string);
    #endif
}
#endif

#ifdef SERIALPRINTNUMBER
void serial1printNumber(long value, u8 base)
{
    #ifdef PIC32_PINGUINO_220
    SerialPrintNumber(UART2, value, base);
    #else
    SerialPrintNumber(UART1, value, base);
    #endif
}
#endif

#ifdef SERIALPRINTFLOAT
void serial1printFloat(float number, u8 digits)
{
    #ifdef PIC32_PINGUINO_220
    SerialPrintFloat(UART2, number, digits);
    #else
    SerialPrintFloat(UART1, number, digits);
    #endif
}
#endif

#ifdef SERIALPRINTF
void serial1printf(char *fmt, ...)		
{
    va_list args;

    va_start(args, fmt);
    #ifdef PIC32_PINGUINO_220
        pprintf(SerialUART2WriteChar, (const u8 *)fmt, args);
    #else
        pprintf(SerialUART1WriteChar, (const u8 *)fmt, args);
    #endif
    va_end(args);
}
#endif /* SERIALPRINTF */

void serial1write(char c)
{
    #ifdef PIC32_PINGUINO_220
        SerialUART2WriteChar(c);
    #else
        SerialUART1WriteChar(c);
    #endif
}

char serial1getkey(void)
{
    #ifdef PIC32_PINGUINO_220
        return SerialGetKey(UART2);
    #else
        return SerialGetKey(UART1);
    #endif
}

char * serial1getstring(void)
{
    #ifdef PIC32_PINGUINO_220
        return SerialGetString(UART2);
    #else
        return SerialGetString(UART1);
    #endif
}

char serial1available(void)
{
    #ifdef PIC32_PINGUINO_220
        return SerialAvailable(UART2);
    #else
        return SerialAvailable(UART1);
    #endif
}

char serial1read(void)
{
    #ifdef PIC32_PINGUINO_220
        return SerialRead(UART2);
    #else
        return SerialRead(UART1);
    #endif
}

void serial1flush(void)
{
    #ifdef PIC32_PINGUINO_220
        SerialFlush(UART2);
    #else
        SerialFlush(UART1);
    #endif
}

BOOL serial1clearrxerror(void)
{
    #ifdef PIC32_PINGUINO_220
        return(SerialClearRxError(UART2));
    #else
        return(SerialClearRxError(UART1));
#endif
}

#endif /* __SERIAL1__ */
