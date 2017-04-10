// Bulk module for Pinguino
// André Gentric 2013
// Régis Blanchot 2016

#ifndef __USBBULK__
#define __USBBULK__

// Version
#define BULK_MAJOR_VER 0
#define BULK_MINOR_VER 1

/***********************************************************************
 ** Config. ************************************************************
 **********************************************************************/
//#define __USBPOLLING__
//#define __ALLOW_RESUME__
//#define __ALLOW_SUSPEND__
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
#ifndef __USBPOLLING__
#define __USBINTERRUPT__
#endif

#include <p32xxxx.h>
#include <stdarg.h>
#include <string.h>         // strlen
#include <typedef.h>
#include <pin.h>
#include <system.c>
//#include <delay.c>

// Printf
#if defined(BULKPRINTF)
#include <printFormated.c>
#endif

// PrintFloat
#if defined(BULKPRINTFLOAT)
    #include <printFloat.c>
#endif

// PrintNumber
#if defined(BULKPRINTNUMBER) || defined(BULKPRINTFLOAT)
    #include <printNumber.c>
#endif

// Debug
#ifdef BULKDEBUG
#include <serial.c>
#endif

#include <usb/usb_device.c>
#include <usb/usb_function_bulk.c>
#include <usb/usb_descriptor.c>

// BULK buffer length
#ifndef _BULKBUFFERLENGTH_
#define _BULKBUFFERLENGTH_      64
#endif

#define EP1_BUFFER_SIZE         64//added 24-8-13
#define BULK_available()        (BULKavailable())

void BULK_begin()
{
    usb_device_init();
    usb_switch_on();

    #ifdef __USBINTERRUPT__
    // Configure and start USB interrupt
    IntConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
    IntSetVectorPriority(_USB_1_VECTOR, 7, 3);
    IntClearFlag(_USB_IRQ);
    IntEnable(_USB_IRQ);
    #endif
}

/***********************************************************************
 * Send a char to the USB.
 * Return a number of free bytes in transmit buffer.
 **********************************************************************/
 
u8 BULK_write(u8 *txpointer, u8 length)
{
    return(BULKputs(txpointer,length));
}

void BULK_printChar(char c)
{
    /*
    if (bulk_trf_state == BULK_TX_READY)
    {
        bulk_tx_len = 1;
        bulk_trf_state = BULK_TX_BUSY;
        bulk_data_tx[0] = c;
    }
    */
    while (bulk_trf_state != BULK_TX_READY)
        bulk_tx_service();

    bulk_tx_len = 1;
    bulk_trf_state = BULK_TX_BUSY;
    bulk_data_tx[0] = c;

    //return sizeof(bulk_data_tx) - bulk_tx_len;
}

/***********************************************************************
 * USB BULK print routine (BULK.print)
 * write a string on the BULK port
 * 2014-03-04   Régis Blanchot    added  
 * 2015-01-23   Régis Blanchot    updated 
 * 2016-07-01   Régis Blanchot    optimized 
 **********************************************************************/

#if defined(BULKWRITE) || defined(BULKPRINT) || defined(BULKPRINTLN) || defined(BULKPRINTF)
void BULK_print(const char *string)
{
    /*
    if (bulk_trf_state == BULK_TX_READY)
    {
        bulk_tx_len = strlen(string);
        bulk_trf_state = BULK_TX_BUSY;
        *bulk_data_tx = *string;
    }
    */
    while (bulk_trf_state != BULK_TX_READY)
        bulk_tx_service();

    bulk_tx_len = strlen(string);
    bulk_trf_state = BULK_TX_BUSY;
    *bulk_data_tx = *string;
}
#endif

/***********************************************************************
 * USB BULK print routine (BULK.println)
 * added by Régis Blanchot 04/03/2014
 * write a string followed by a carriage return character (ASCII 13, or '\r')
 * and a newline character (ASCII 10, or '\n') on BULK port
 **********************************************************************/

#if defined(BULKPRINTLN)
void BULK_println(const char *string)
{
    BULK_print(string);
    BULK_print("\n\r");
}
#endif

/***********************************************************************
 * USB BULK printNumber routine (BULK.printNumber)
 * added by Régis Blanchot 15/12/2016
 * write a number on BULK port
 * base : see const.h (DEC, BIN, HEXA, OCTO, ...)
 **********************************************************************/

#if defined(BULKPRINTNUMBER) || defined(BULKPRINTFLOAT)
void BULK_printNumber(long value, u8 base)
{
    printNumber(BULK_printChar, value, base);
}
#endif

/***********************************************************************
 * USB BULK printFloat routine (BULK.printFloat)
 * added by Régis Blanchot 15/12/2016
 * write a float number on BULK port
 * base : see const.h (DEC, BIN, HEXA, OCTO, ...)
 **********************************************************************/

#if defined(BULKPRINTFLOAT)
void BULK_printFloat(float number, u8 digits)
{ 
    printFloat(BULK_printChar, number, digits);
}
#endif

/***********************************************************************
 * USB BULK printf routine (BULK.printf)
 * added by Régis Blanchot 15/12/2016
 * write a formated string on BULK port
 **********************************************************************/

#if defined(BULKPRINTF)
void BULK_printf(const char *fmt, ...)
{
    u8 length;
    va_list	args;

    va_start(args, fmt);
    length = psprintf2(bulk_data_tx, fmt, args);
    BULKputs(bulk_data_tx,length);
    //BULK_print(bulk_data_tx);//, length);
    
    va_end(args);
}
#endif

/***********************************************************************
 * USB BULK getKey routine (BULK.getKey)
 * added by Régis Blanchot 15/12/2016
 * wait and return a char from BULK port
 **********************************************************************/

u8 BULK_read(u8 *rxpointer)
{
    return(BULKgets(rxpointer)); // now rxpointer buffer is filled and the buffer length is returned
}

#if defined(BULKGETKEY) || defined(BULKGETSTRING)
u8 BULK_getkey(void)
{
    u8 buffer[_BULKBUFFERLENGTH_];   // always get a full packet

    while (!BULKgets(buffer));
    return (buffer[0]);             // return only the first character
}
#endif

/***********************************************************************
 * USB BULK getString routine (BULK.getString)
 * added by Régis Blanchot 15/12/2016
 * wait and return a string from BULK port
 **********************************************************************/

#if defined(BULKGETSTRING)
u8 * BULK_getstring(void)
{
    u8 c, i = 0;
    static u8 buffer[_BULKBUFFERLENGTH_];// Needs static buffer at least.
    
    do {
        c = BULK_getkey();
        buffer[i++] = c;
        //bulk_puts(&buffer[i-1], 1);
        BULK_printChar(buffer[i-1]);
    } while (c != '\r');
    buffer[i] = '\0';
    return buffer;
}
#endif

#endif /* __USBBULK */
