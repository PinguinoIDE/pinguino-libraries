/*	--------------------------------------------------------------------
    FILE:           debug.c
    PROJECT:        Pinguino
    PURPOSE:        Debug functions
    PROGRAMER:      Regis Blanchot <rblanchot@gmail.com>
    FIRST RELEASE:  29 Jan. 2015
    --------------------------------------------------------------------
    21 Jun. 2016    Regis Blanchot - added ST7735 output and debug_init()
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

#ifndef __DEBUG_C
#define __DEBUG_C

#if defined(USBCDCDEBUG)  || defined(ST7735DEBUG)  || \
    defined(SERIAL1DEBUG) || defined(SERIAL2DEBUG)
    #define __DEBUG__
#endif

#include <stdarg.h>

#if defined(USBCDCDEBUG)
    #define CDCPRINTF
    #include <__cdc.c>

#elif defined(ST7735DEBUG)
    #define ST7735PRINTF
    #include <st7735.c>

#elif defined(SERIAL1DEBUG)
    #define SERIALPRINTF
    #include <serial1.c>

#elif defined(SERIAL2DEBUG)
    #define SERIALPRINTF
    #include <serial2.c>
    
#endif

void debug_init()
{
    #if defined(SERIAL1DEBUG)
        serial1init(9600);
        serial1printf("\f");        // clear screen

    #elif defined(SERIAL2DEBUG)
        serial2init(9600);
        serial2printf("\f");        // clear screen

    #endif
}

void debug(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    
    #if defined(USBCDCDEBUG)
        CDCprintf("debug: ");
        CDCprintf(format, args);
        CDCprintf("\r\n");

    #elif defined(ST7735DEBUG)
        ST7735printf(SPI2, "debug: ");
        ST7735printf(SPI2, format, args);
        ST7735printf(SPI2, "\r\n");

    #elif defined(SERIAL1DEBUG)
        serial1printf("debug: ");
        //serial1printf(format, args);
        pprintf(SerialUART1WriteChar, format, args);
        serial1printf("\r\n");

    #elif defined(SERIAL2DEBUG)
        serial2printf("debug: ");
        //serial2printf(format, args);
        pprintf(SerialUART2WriteChar, format, args);
        serial2printf("\r\n");

    #endif
    
    va_end(args);
}

#endif	/* __DEBUG_C */

