/*  --------------------------------------------------------------------
    FILE:       serial.h
    PROJECT:    Pinguino
    PURPOSE:    UART Library for 8-bit Pinguino
    PROGRAMER:  Régis Blanchot
    --------------------------------------------------------------------
    CHANGELOG:
    09-05-2017  Regis Blanchot - first release
    20-09-2017  Regis Blanchot - fixed Serial_printChar (Serial_write)
    --------------------------------------------------------------------
    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2, or (at your option) any
    later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

    In other words, you are welcome to use, share and improve this program.
    You are forbidden to forbid anyone else to use, share and improve
    what you give them.   Help stamp out software-hoarding!
    ------------------------------------------------------------------*/

#ifndef __SERIAL_H
#define __SERIAL_H

#include <stdarg.h>         // variadic functions

#define UARTSTARTBIT 0
#define UARTSTOPBIT  1
#define UARTIDLEBIT  1

extern u8 UART_Module;      // Current active UART module

//void Serial_begin(int module, int baudrate, ...);
void Serial_begin(int module, u32 baudrate, va_list args);
void SerialSW_begin(u32 baudrate, ...);
void Serial_writeBit(u8 b);
void Serial_printChar(u8 module, u8 c);
void Serial_printChar2(u8 c);
void Serial_print(u8 module, const char *s);
void Serial_println(u8 module, const char *string);
void Serial_printNumber(u8 module, s32 value, u8 base);
void Serial_printFloat(u8 module, float number, u8 digits);
void Serial_printX(u8 module, const char *s, s32 value, u8 base);
void Serial_printf(u8 module, char *fmt, ...);
u8 Serial_readBit();
u8 Serial_readChar(u8 module);
u8 Serial_getKey(u8 module);
u8 * Serial_getString(u8 module);
void serial_interrupt(void);

#define Serial1_begin(b)            Serial_begin(UART1, b, NULL)
#define Serial1_write(c)            Serial_printChar(UART1, c)
#define Serial1_printChar(c)        Serial_printChar(UART1, c)
#define Serial1_print(s)            Serial_print(UART1, s)
#define Serial1_println(s)          Serial_println(UART1, s)
#define Serial1_printNumber(v, b)   Serial_printNumber(UART1, v, b)
#define Serial1_printFloat(n, d)    Serial_printFloat(UART1, n, d)
#define Serial1_printX(s, v, b)     Serial_printX(UART1, s, v, b)
//#define Serial1_printf(f, ...)      Serial_printf(UART1, f, ...)
#define Serial1_readChar()          Serial_readChar(UART1)
#define Serial1_getKey()            Serial_getKey(UART1)
#define Serial1_getString()         Serial_getString(UART1)
#define Serial1_available()         Serial_available(UART1)
#define Serial1_flush()             Serial_flush(UART1)

#define Serial2_begin(b)            Serial_begin(UART2, b, NULL)
#define Serial2_write(c)            Serial_printChar(UART2, c)
#define Serial2_printChar(c)        Serial_printChar(UART2, c)
#define Serial2_print(s)            Serial_print(UART2, s)
#define Serial2_println(s)          Serial_println(UART2, s)
#define Serial2_printNumber(v, b)   Serial_printNumber(UART2, v, b)
#define Serial2_printFloat(n, d)    Serial_printFloat(UART2, n, d)
#define Serial2_printX(s, v, b)     Serial_printX(UART2, s, v, b)
//#define Serial2_printf(f, ...)      Serial_printf(UART2, f, ...)
#define Serial2_readChar()          Serial_readChar(UART2)
#define Serial2_getKey()            Serial_getKey(UART2)
#define Serial2_getString()         Serial_getString(UART2)
#define Serial2_available()         Serial_available(UART2)
#define Serial2_flush()             Serial_flush(UART2)

//#define SerialSW_begin(b)           Serial_begin(UARTSW, b, ...)
#define SerialSW_printChar(c)       Serial_printChar(c)
#define SerialSW_print(s)           Serial_print(UARTSW, s)
#define SerialSW_println(s)         Serial_println(UARTSW, s)
#define SerialSW_printNumber(v, b)  Serial_printNumber(UARTSW, v, b)
#define SerialSW_printFloat(n, d)   Serial_printFloat(UARTSW, n, d)
#define SerialSW_printX(n, d)       Serial_printX(UARTSW, n, d)
//#define SerialSW_printf(f, ...)     Serial_printf(UARTSW, f, ...)
#define SerialSW_readChar()         Serial_readChar(UARTSW)
#define SerialSW_getKey()           Serial_getKey(UARTSW)
#define SerialSW_getString()        Serial_getString(UARTSW)
#define SerialSW_available()        Serial_available(UARTSW)
#define SerialSW_flush()            Serial_flush(UARTSW)

#define Serial_baudRateDivisor(f,b) ((f/(4*b))-1)
#define Serial_putchar(c)           Serial_printChar(c)
#define Serial_read(c)              Serial_readChar(c)

#endif /* __SERIAL_H */
