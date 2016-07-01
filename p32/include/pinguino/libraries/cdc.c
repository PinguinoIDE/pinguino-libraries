/*  --------------------------------------------------------------------
    FILE:               cdc.c
    PROJECT:            pinguino 32
    PURPOSE:            usb cdc module based on the Microchip USB stack
    PROGRAMERS:         Regis Blanchot <rblanchot@gmail.com>
                        Jean-Pierre Mandon <jp.mandon@gmail.com>
    FIRST RELEASE:      16 Nov. 2010
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
    12 Feb. 2015 - 2.7 - Régis Blanchot     - added usb_check_cable() an interrupt attach/detach USB cable routine
    30 Mar. 2015 - 2.8 - Régis Blanchot     - fixed usb_device_init() and usb_device_task()
    23 Jun. 2016 - 2.9 - Régis Blanchot     - added Print functions support
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

// Version
#define CDC_MAJOR_VER 2
#define CDC_MINOR_VER 9

/***********************************************************************
 ** Config. ************************************************************
 **********************************************************************/
//#define __USBCDCPOLLING__
#define __ALLOW_RESUME__
#define __ALLOW_SUSPEND__
//#define __ALLOW_DEBUG__
/**********************************************************************/

// __DEBUG__ cf. core/__DEBUG__.c
#ifdef __ALLOW_DEBUG__
#if defined(PIC32_PINGUINO) || defined(PIC32_PINGUINO_OTG) || defined(PIC32_PINGUINO_MICRO)
#define SERIAL2DEBUG
#else
#define SERIAL1DEBUG
#endif
#endif

// Polling or interrupt mode
#ifndef __USBCDCPOLLING__
#define __USBCDCINTERRUPT__
#endif

#include <p32xxxx.h>
#include <stdarg.h>
#include <string.h>         // strlen
#include <typedef.h>
#include <pin.h>
#include <system.c>
//#include <delay.c>

#ifdef __USBCDCINTERRUPT__
#include <interrupt.c>
#endif

#if defined(USBCDCDEBUG)  || defined(ST7735DEBUG)  || \
    defined(SERIAL1DEBUG) || defined(SERIAL2DEBUG)
    #include <debug.c>
#endif

#ifdef USERLED
#include <digitalw.c>
#endif

// Printf
#ifdef CDCPRINTF
    #include <printFormated.c>
#endif

// PrintFloat
#if defined(CDCPRINTFLOAT)
    #include <printFloat.c>
#endif

// PrintNumber
#if defined(CDCPRINTNUMBER) || defined(CDCPRINTFLOAT)
    #include <printNumber.c>
#endif

#include <usb/usb_device.c>
#include <usb/usb_function_cdc.c>
#include <usb/usb_descriptor.c>

// CDC buffer length
#ifndef _CDCBUFFERLENGTH_
#define _CDCBUFFERLENGTH_ 64
#endif

extern USBVOLATILE u8  usb_device_state;
extern u32 cdc_bps;

void CDC_begin(u32);

/***********************************************************************
 * USB CDC init routine
 * Call usb_device_init();
 * Call CDC_connect();
 **********************************************************************/

void CDC_begin(u32 baudrate)
{
    cdc_bps = baudrate;
    
    #ifdef __DEBUG__
    debug("----------------------");
    debug("    www.PINGUINO.cc   ");
    debug("USB CDC Library v%d.%02d", CDC_MAJOR_VER, CDC_MINOR_VER);
    debug("----------------------");
    #endif

    usb_device_init();
    usb_switch_on();

    #ifdef __USBCDCINTERRUPT__
    // Configure and start USB interrupt
    IntConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
    IntSetVectorPriority(_USB_1_VECTOR, 7, 3);
    IntClearFlag(_USB_IRQ);
    IntEnable(_USB_IRQ);
    #endif
}

/***********************************************************************
 * Dummy function needed by the IDE pre-processor
 **********************************************************************/

#ifndef __USBCDCINTERRUPT__

#define CDC_polling()

#endif /* __USBCDCINTERRUPT__ */

/***********************************************************************
 * Send a char to the USB.
 * Return a number of free bytes in transmit buffer.
 **********************************************************************/
 
void cdc_printChar(char c)
{
    /*
    if (cdc_trf_state == CDC_TX_READY)
    {
        cdc_tx_len = 1;
        cdc_trf_state = CDC_TX_BUSY;
        cdc_data_tx[0] = c;
    }
    */
    while (cdc_trf_state != CDC_TX_READY)
        cdc_tx_service();

    cdc_tx_len = 1;
    cdc_trf_state = CDC_TX_BUSY;
    cdc_data_tx[0] = c;

    //return sizeof(cdc_data_tx) - cdc_tx_len;
}

/***********************************************************************
 * USB CDC print routine (CDC.print)
 * write a string on the CDC port
 * 2014-03-04   Régis Blanchot    added  
 * 2015-01-23   Régis Blanchot    updated 
 * 2016-07-01   Régis Blanchot    optimized 
 **********************************************************************/

#if defined(CDCWRITE) || defined(CDCPRINT) || defined(CDCPRINTLN) || defined(CDCPRINTF)
void CDC_print(const char *string)
{
    /*
    if (cdc_trf_state == CDC_TX_READY)
    {
        cdc_tx_len = strlen(string);
        cdc_trf_state = CDC_TX_BUSY;
        *cdc_data_tx = *string;
    }
    */
    while (cdc_trf_state != CDC_TX_READY)
        cdc_tx_service();

    cdc_tx_len = strlen(string);
    cdc_trf_state = CDC_TX_BUSY;
    *cdc_data_tx = *string;
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
    printNumber(CDC_printChar, value, base);
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
    printFloat(CDC_printChar, number, digits);
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
    u8 length;
    va_list	args;

    va_start(args, fmt);
    length = psprintf2(cdc_data_tx, fmt, args);
    CDC_print(cdc_data_tx);//, length);
    
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
        CDC_printChar(buffer[i-1]);
    } while (c != '\r');
    buffer[i] = '\0';
    return buffer;
}
#endif

#endif	/* __USBCDC */
