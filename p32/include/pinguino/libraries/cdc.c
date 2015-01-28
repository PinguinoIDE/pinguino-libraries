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

    16 Nov. 2010 - Jean-Pierre Mandon - first release
            2011 - Régis Blanchot     - added printf, println, print, write, getKey, getString
    25 Feb. 2012 - Jean-Pierre Mandon - added support for 32MX220F032
    03 Mar. 2012 - Jean-Pierre Mandon - fixed a bug in WINDOWS CDC
    18 Jun. 2013 - Moreno manzini     - added CDC.USBIsConnected to check if USB cable is connected
    13 Mar. 2014 - Régis Blanchot     - added printNumber, printFloat
    13 Mar. 2014 - Régis Blanchot     - updated print, println, getKey and getString to spare memory on small memory PIC
    28 Aug. 2014 - Régis Blanchot     - added #include <string.h> to use strlen
    23 Jan. 2015 - Régis Blanchot     - added interrupt management
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

// comment to not use interrupt
//#define __USBCDCINTERRUPT__

#include <stdarg.h>
#include <typedef.h>
#include <pin.h>
#include <system.c>
#include <delay.c>
#include <interrupt.c>

#ifdef USERLED
#include <digitalw.c>
#endif

#if defined(CDCPRINT) || defined(CDCPRINTLN)
#include <string.h>         // strlen
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
extern unsigned usb_device_state;

/***********************************************************************
 * USB CDC init routine
 * called from main32.c
 **********************************************************************/

void CDC_init()
{
    #ifdef USERLED
    int led_count = 0;
    output(USERLED);
    #endif

    #ifdef __USBCDCINTERRUPT__
    // Configure USB interrupt
    // Note : USBIE and USBIF are not available on PIC32MX1XX devices.
    IntConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
    IntSetVectorPriority(_USB_1_VECTOR, 7, 3);
    IntClearFlag(_USB_IRQ);
    IntEnable(_USB_IRQ);
    #endif
    
    usb_device_init();

    // Blink the led until device is configured and no more suspended
    while ( U1PWRCbits.USUSPEND || usb_device_state < CONFIGURED_STATE )
    {
        usb_device_tasks();

        #ifdef USERLED
        if (led_count == 0)
        {
            led_count = 10000;
            toggle(USERLED);
        }
        
        led_count--;
        #endif
    }

    #ifdef USERLED
    low(USERLED);
    #endif

}

/***********************************************************************
 * USB CDC interrupt routine
 * added by regis blanchot 23/01/2015
 * cf. ISRwrapper.S
 **********************************************************************/

#ifdef __USBCDCINTERRUPT__
void USBInterrupt(void)
{
    if (IntGetFlag(_USB_IRQ))
    {
        IntClearFlag(_USB_IRQ);

        // handles device-to-host transactions
        cdc_tx_service();
        
        // Write a 1 to thess bits clear the interrupts
        U1IR = _U1IR_SOFIF_MASK | _U1IR_URSTIF_MASK;
        U1EIR = 0;
    }
}
#endif /* __USBCDCINTERRUPT__ */

/***********************************************************************
 * USB Callback Functions
 **********************************************************************/

// Process device-specific SETUP requests.
void usbcb_check_other_req()
{
    // Check the setup data packet (cf. usb_function_cdc.c)
    usb_check_cdc_request();
}

// Initialize the endpoints
void usbcb_init_ep()
{
    // Enable the CDC endpoint (cf. usb_function_cdc.c)
    cdc_init_ep();
}

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

#if defined(CDCPRINT) || defined(CDCPRINTLN)
void CDCprint(char *string)
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
void CDCprintln(char *string)
{
    CDCprint(string);
    CDCprint("\n\r");
}
#endif

/***********************************************************************
 * USB CDC printNumber routine (CDC.printNumber)
 * added by regis blanchot 14/06/2011
 * write a number on CDC port
 * base : see const.h (DEC, BIN, HEXA, OCTO, ...)
 **********************************************************************/

#if defined(CDCPRINTNUMBER) || defined(CDCPRINTFLOAT)
void CDCprintNumber(long value, u8 base)
{  
    u8 sign;
    u8 length;

    long i;
    unsigned long v;    // absolute value

    u8 tmp[12];
    u8 *tp = tmp;       // pointer on tmp

    u8 string[12];
    u8 *sp = string;    // pointer on string

    if (value==0)
    {
        cdc_puts("0", 1);
        return;
    }
    
    sign = ( (base == 10) && (value < 0) );

    if (sign)
        v = -value;
    else
        v = (unsigned long)value;

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

    cdc_puts(string, length);
}
#endif

/***********************************************************************
 * USB CDC printFloat routine (CDC.printFloat)
 * added by regis blanchot 14/06/2011
 * write a float number on CDC port
 * base : see const.h (DEC, BIN, HEXA, OCTO, ...)
 **********************************************************************/

#if defined(CDCPRINTFLOAT)
void CDCprintFloat(float number, u8 digits)
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
    CDCprintNumber(int_part, 10);

    // Print the decimal point, but only if there are digits beyond
    if (digits > 0)
        cdc_puts('.', 1); 

    // Extract digits from the remainder one at a time
    while (digits-- > 0)
    {
        remainder *= 10.0;
        toPrint = (unsigned int)remainder; //Integer part without use of math.h lib, I think better! (Fazzi)
        CDCprintNumber(toPrint, 10);
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
void CDCprintf(const u8 *fmt, ...)
{
    u8 buffer[_CDCBUFFERLENGTH_];
    u8 length;
    va_list	args;

    va_start(args, fmt);
    length = psprintf2(buffer, fmt, args);
    cdc_puts(buffer,length);
    va_end(args);
}
#endif

/***********************************************************************
 * USB CDC getKey routine (CDC.getKey)
 * added by regis blanchot 14/06/2011
 * wait and return a char from CDC port
 **********************************************************************/

#if defined(CDCGETKEY) || defined(CDCGETSTRING)
u8 CDCgetkey(void)
{
    u8 buffer[_CDCBUFFERLENGTH_];		// always get a full packet

    while (!CDCgets(buffer));
    return (buffer[0]);	// return only the first character
}
#endif

/***********************************************************************
 * USB CDC getString routine (CDC.getString)
 * added by regis blanchot 14/06/2011
 * wait and return a string from CDC port
 **********************************************************************/

#if defined(CDCGETSTRING)
u8 * CDCgetstring(void)
{
    u8 c, i = 0;
    static u8 buffer[_CDCBUFFERLENGTH_];	// Needs static buffer at least.
    
    do {
        c = CDCgetkey();
        buffer[i++] = c;
        cdc_puts(&buffer[i-1], 1);
    } while (c != '\r');
    buffer[i] = '\0';
    return buffer;
}
#endif

/***********************************************************************
 * USB CDC IsConnected (CDC.USBIsConnected)
 * added by Moreno Manzini 18 Jun. 2013 
 * check if USB cable is connected
 **********************************************************************/

#if 0
BOOL CDCUSBNotConnected = false;

BOOL CDCUSBIsConnected(void)
{
    if ( (U1OTGSTATbits.VBUSVD != 0) && (U1OTGSTATbits.SESVD != 0) )
    {
        if (CDCUSBNotConnected == true)
        {
            CDC_init();
            CDCUSBNotConnected = false;
        }
        return(true);
    }
    else
    {
        CDCUSBNotConnected = true;
        return (false);
    }
}

// from MLA
/*
typedef union _CONTROL_SIGNAL_BITMAP
{
    BYTE _byte;
    struct
    {
        unsigned DTE_PRESENT:1;
        unsigned CARRIER_CONTROL:1;
    };
} CONTROL_SIGNAL_BITMAP;
CONTROL_SIGNAL_BITMAP control_signal_bitmap;
*/

//extern u8 control_signal_bitmap;    // in libusb.a

//#define CDCClearDTR() (control_signal_bitmap &= 0xfe)

BOOL CDCDTRIsReady(void)
{
    if (CDCUSBIsConnected() == true)
    {
        if(control_signal_bitmap.DTE_PRESENT == 1)
        //if((control_signal_bitmap & 0x01) == 0x01)
            return (true);
        else
            return (false);
    }
    else
    {
        control_signal_bitmap.DTE_PRESENT = 0;
        //control_signal_bitmap &= 0xfe;       // !0x01
        //CDCClearDTR();
        return (false);
    }
}
#endif

#endif	/* __USBCDC */
