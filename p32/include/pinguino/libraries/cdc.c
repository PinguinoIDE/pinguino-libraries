/*  --------------------------------------------------------------------
    FILE:               cdc.c
    PROJECT:            pinguino 32
    PURPOSE:            usb cdc module based on the Microchip USB stack
    PROGRAMERS:         Regis Blanchot <rblanchot@gmail.com>
                        Jean-Pierre Mandon <jp.mandon@gmail.com>
    FIRST RELEASE:      16 Nov. 2010
    LAST RELEASE:       23 Jan. 2015
    --------------------------------------------------------------------
    CHANGELOG:

    16 Nov. 2010 - 1.0 - Jean-Pierre Mandon - first release
            2011 - 1.1 - Régis Blanchot     - added printf, println, print, write, getKey, getString
    25 Feb. 2012 - 1.2 - Jean-Pierre Mandon - added support for 32MX220F032
    03 Mar. 2012 - 1.3 - Jean-Pierre Mandon - fixed a bug in WINDOWS CDC
    18 Jun. 2013 - 1.4 - Moreno manzini     - added CDC.USBIsConnected to check if USB cable is connected
    13 Mar. 2014 - 1.5 - Régis Blanchot     - added printNumber, printFloat
    13 Mar. 2014 - 1.6 - Régis Blanchot     - updated print, println, getKey and getString to spare memory on small memory PIC
    28 Aug. 2014 - 1.7 - Régis Blanchot     - added #include <string.h> to use strlen
    23 Jan. 2015 - 2.0 - Régis Blanchot     - replaced use of libcdc.a with c files
    02 Feb. 2015 - 2.1 - Régis Blanchot     - added interrupt-driven mode
    04 Feb. 2015 - 2.2 - Régis Blanchot     - added CDC.begin and CDC.polling functions
    06 Feb. 2015 - 2.3 - Régis Blanchot     - renamed CDC.USBIsConnected in CDC.isConnected
    06 Feb. 2015 - 2.4 - Régis Blanchot     - renamed CDC.TXIsReady in CDC.available
    06 Feb. 2015 - 2.5 - Régis Blanchot     - added CDC.isReady and CDC.connect
    10 Feb. 2015 - 2.6 - Régis Blanchot     - splited usb_device_tasks() to create usb_enable_module()
    12 Feb. 2015 - 2.7 - Régis Blanchot     - added interrupt attach/detach USB cable routine
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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
----------------------------------------------------------------------*/

#ifndef __USBCDC__
#define __USBCDC__

#define DEBUG

#define CDC_MAJOR_VER 2
#define CDC_MINOR_VER 7

// Polling or interrupt mode
#if !defined(__USBCDCPOLLING__)
#define __USBCDCINTERRUPT__
#endif

#include <stdarg.h>
#include <typedef.h>
#include <pin.h>
#include <system.c>
#include <delay.c>
#include <interrupt.c>

#ifdef DEBUG
    #ifdef USERLED
    #include <digitalw.c>
    #endif

    #ifndef SERIALPRINT
    #define SERIALPRINT
    #endif

    #ifndef SERIALPRINTNUMBER
    #define SERIALPRINTNUMBER
    #endif

    #include <serial.c>
    #define UART UART1
#endif

#if defined(CDCWRITE) || defined(CDCPRINT) || defined(CDCPRINTLN)
#include <string.h>         // strlen, bzero, ...
#endif

#if defined(CDCPRINTF)
#include <printf.c>
#endif

#include <usb/usb_device.c>
#include <usb/usb_function_cdc.c>
#include <usb/usb_descriptor.c>

// CDC buffer length
#ifndef _CDCBUFFERLENGTH_
#define _CDCBUFFERLENGTH_ 64
#endif

//u8 _cdc_buffer[_CDCBUFFERLENGTH_];  // usb buffer
//extern unsigned usb_device_state;

USBVOLATILE  u8 cdc_USBNotConnected = true;
USBVOLATILE u32 cdc_bps;

void CDC_begin(u32);
void CDC_connect(void);

/***********************************************************************
 * USB CDC init routine
 * Call usb_device_init();
 * Call CDC_connect();
 **********************************************************************/

void CDC_begin(u32 baudrate)
{
    cdc_bps = baudrate;
    
    #ifdef DEBUG
    SerialConfigure(UART, UART_ENABLE, UART_RX_TX_ENABLED, 9600);
    #endif

    usb_device_init();
    CDC_connect();

    #ifdef __USBCDCINTERRUPT__
    // Configure and start USB interrupt
    IntConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
    IntSetVectorPriority(_USB_1_VECTOR, 7, 3);
    IntClearFlag(_USB_IRQ);
    IntEnable(_USB_IRQ);
    #endif
}

/***********************************************************************
 * This function runs until the device is connected.
 * Call usb_enable_module() if cable was not connected
 * Call usb_device tasks() to process all interrupts
 * TODO : get out if USB cable is unplugged
 **********************************************************************/

void CDC_connect(void)
{
    #if defined(DEBUG) && defined(USERLED)
    u32 led_count = 0;
    output(USERLED);
    #endif

    // Blink the led until device is configured and no more suspended
    while ( (U1PWRC & _U1PWRC_USUSPEND_MASK) || (usb_device_state < CONFIGURED_STATE) )
    {
        #if defined(DEBUG) && defined(USERLED)
        if (led_count == 0)
        {
            led_count = 10000;
            toggle(USERLED);
        }
        led_count--;
        #endif

        //usb_enable_module();
        usb_device_tasks();

        //#ifdef DEBUG
        //SerialPrint(UART,"usb_device_state = ");
        //SerialPrintNumber(UART, usb_device_state, DEC);
        //SerialPrint(UART,"\r\n");
        //#endif
    }

    #if defined(DEBUG) && defined(USERLED)
    low(USERLED);
    #endif

    cdc_USBNotConnected = false;
}

/***********************************************************************
 * CDC_isConnected (CDC.isConnected)
 * added by Moreno Manzini 18 Jun. 2013 
 * check if USB cable is connected
 * modified by Regis Blanchot 06 Feb. 2015
 * Works only with Olimex boards
 **********************************************************************/

u8 CDC_isConnected(void)
{
    // VBUS detection is required just for self-powered devices.
    if ( (U1OTGSTAT & _U1OTGSTAT_VBUSVD_MASK) && 
         (U1OTGSTAT & _U1OTGSTAT_SESVD_MASK ) )
    {
        if (cdc_USBNotConnected == true)
        {
            CDC_connect();
            cdc_USBNotConnected = false;
        }
        return(true);
    }
    else
    {
        cdc_USBNotConnected = true;
        return (false);
    }
}

/***********************************************************************
 * CDC_available (CDC.available)
 * added by Moreno Manzini 18 Jun. 2013 
 * check if CDC is available
 * modified by Regis Blanchot 06 Feb. 2015
 **********************************************************************/

u8 CDC_available(void)
{
    if (CDC_isConnected())
    {
        if(control_signal_bitmap.DTE_PRESENT == 1)
            return (true);
        else
            return (false);
    }
    else
    {
        control_signal_bitmap.DTE_PRESENT = 0;
        return (false);
    }
}

/***********************************************************************
 * USB CDC interrupt routine
 * added by regis blanchot 23/01/2015
 * cf. ISRwrapper.S
 * Alternatively, this routine may be called from the Interrupt Service
 * Routine (ISR) whenever a USB interrupt occurs. If it is used this
 * way, the entire USB firmware stack (the non-user API portion of the
 * CDC serial driver) operates in an interrupt context (including the
 * applicationís event-handler callback routine)
 **********************************************************************/

#ifdef __USBCDCINTERRUPT__

void USBInterrupt(void)
{
    // Clear general USB flag
    IntClearFlag(_USB_IRQ);
    // Process all interrupts
    //usb_enable_module();
    usb_device_tasks();
    // Clear SOF and RESET flags
    U1IR = _U1IR_SOFIF_MASK | _U1IR_URSTIF_MASK;
    // Clear all Error flags
    U1EIR = 0;
}

#else

/***********************************************************************
 * Dummy function needed by the IDE pre-processor
 **********************************************************************/

#define CDC_polling()

#endif /* __USBCDCINTERRUPT__ */

/***********************************************************************
 * USB CDC read routine (CDC.read)
 * modified by régis blanchot 18-05-2011 
 * get a string from CDC port
 * return num. bytes read
 **********************************************************************/

/*
u8 CDCgets(u8 *buffer)
{
    #if defined(__32MX220F032D__) || defined(__32MX220F032B__) || \
        defined(__32MX250F128B__) || defined(__32MX270F256B__)

        USB_Service();
        return USB_Service_CDC_GetString( buffer );

    #else

        CDCTxService();
        return getsUSBUSART(buffer, 64);

    #endif

    if (mUSBUSARTIsTxTrfReady())
    {
        CDCTxService();
        numBytesRead = getsUSBUSART(buffer, 64);
        CDCTxService();
        return numBytesRead;
    }
}
*/

/***********************************************************************
 * USB CDC print routine (CDC.print)
 * added by regis blanchot 04/03/2014
 * write a string on CDC port
 **********************************************************************/

#if defined(CDCWRITE) || defined(CDCPRINT) || defined(CDCPRINTLN)
void CDC_print(const char *string)
{
    cdc_puts(string, strlen(string));
}
#endif

/***********************************************************************
 * USB CDC print routine (CDC.println)
 * added by regis blanchot 04/03/2014
 * write a string followed by a carriage return character (ASCII 13, or '\r')
 * and a newline character (ASCII 10, or '\n') on CDC port
 **********************************************************************/

#if defined(CDCPRINTLN)
void CDC_println(const char *string)
{
    CDC_print(string);
    CDC_print("\n\r");
}
#endif

/***********************************************************************
 * USB CDC printNumber routine (CDC.printNumber)
 * added by regis blanchot 14/06/2011
 * write a number on CDC port
 * base : see const.h (DEC, BIN, HEXA, OCTO, ...)
 **********************************************************************/

#if defined(CDCPRINTNUMBER) || defined(CDCPRINTFLOAT)
void CDC_printNumber(long value, u8 base)
{  
    u8 sign;
    u8 length;

    long i;
    u32 v;              // absolute value

    u8 tmp[12];
    u8 *tp = tmp;       // pointer on tmp

    u8 string[12];
    u8 *sp = string;    // pointer on string

    if (value==0)
    {
        cdc_putc('0');
        return;
    }
    
    sign = ( (base == 10) && (value < 0) );

    if (sign)
        v = -value;
    else
        v = (u32)value;

    //while (v || tp == tmp)
    while (v)
    {
        i = v % base;
        v = v / base;
        
        if (i < 10)
            *tp++ = i + '0';
        else
            *tp++ = i + 'A' - 10;
    }

    // start of string
    if (sign)
        *sp++ = '-';

    length = sign + tp - tmp + 1;

    // backwards writing 
    while (tp > tmp)
        *sp++ = *--tp;

    // end of string
    *sp = 0;

    cdc_puts((const char *)string, length);
}
#endif

/***********************************************************************
 * USB CDC printFloat routine (CDC.printFloat)
 * added by regis blanchot 14/06/2011
 * write a float number on CDC port
 * base : see const.h (DEC, BIN, HEXA, OCTO, ...)
 **********************************************************************/

#if defined(CDCPRINTFLOAT)
void CDC_printFloat(float number, u8 digits)
{ 
    u8 i, toPrint;
    u16 int_part;
    float rounding, remainder;

    // Handle negative numbers
    if (number < 0.0)
    {
        cdc_puts('-', 1);
        number = -number;
    }

    // Round correctly so that print(1.999, 2) prints as "2.00"  
    rounding = 0.5;
    for (i=0; i<digits; ++i)
        rounding /= 10.0;

    number += rounding;

    // Extract the integer part of the number and print it  
    int_part = (u16)number;
    remainder = number - (float)int_part;
    CDC_printNumber(int_part, 10);

    // Print the decimal point, but only if there are digits beyond
    if (digits > 0)
        cdc_putc('.'); 

    // Extract digits from the remainder one at a time
    while (digits-- > 0)
    {
        remainder *= 10.0;
        toPrint = (u32)remainder; //Integer part without use of math.h lib, I think better! (Fazzi)
        CDC_printNumber(toPrint, 10);
        remainder -= toPrint; 
    }
}
#endif

/***********************************************************************
 * USB CDC printf routine (CDC.printf)
 * added by regis blanchot 14/06/2011
 * write a formated string on CDC port
 **********************************************************************/

#if defined(CDCPRINTF)
void CDC_printf(const char *fmt, ...)
{
    u8 buffer[_CDCBUFFERLENGTH_];
    u8 length;
    va_list	args;

    va_start(args, fmt);
    length = psprintf2(buffer, fmt, args);
    cdc_puts(&buffer,length);
    
    va_end(args);
}
#endif

/***********************************************************************
 * USB CDC getKey routine (CDC.getKey)
 * added by regis blanchot 14/06/2011
 * wait and return a char from CDC port
 **********************************************************************/

#if defined(CDCGETKEY) || defined(CDCGETSTRING)
u8 CDC_getkey(void)
{
    u8 buffer[_CDCBUFFERLENGTH_];   // always get a full packet

    while (!cdc_gets(buffer));
    return (buffer[0]);             // return only the first character
}
#endif

/***********************************************************************
 * USB CDC getString routine (CDC.getString)
 * added by regis blanchot 14/06/2011
 * wait and return a string from CDC port
 **********************************************************************/

#if defined(CDCGETSTRING)
u8 * CDC_getstring(void)
{
    u8 c, i = 0;
    static u8 buffer[_CDCBUFFERLENGTH_];	// Needs static buffer at least.
    
    do {
        c = CDC_getkey();
        buffer[i++] = c;
        //cdc_puts(&buffer[i-1], 1);
        cdc_putc(buffer[i-1]);
    } while (c != '\r');
    buffer[i] = '\0';
    return buffer;
}
#endif

#endif	/* __USBCDC */
