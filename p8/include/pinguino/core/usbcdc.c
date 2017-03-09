/*	--------------------------------------------------------------------
    FILE:         usbcdc.c
    PROJECT:      pinguino
    PURPOSE:      USB CDC routines for use with pinguino board, 
    PROGRAMER:    Jean-Pierre Mandon 2010
    CHANGELOG:
    --------------------------------------------------------------------
    14 Jun 2011 - Regis Blanchot (rblanchot@gmail.com) added :
                  printf, println, print, write, getKey, getString
    05 Feb 2013 - Regis Blanchot (rblanchot@gmail.com) moved :
                  interrupt routine
    04 Mar 2014 - Regis Blanchot (rblanchot@gmail.com) added :
                  print, printNumber and printFloat
    10 Mar 2014 - Regis Blanchot (rblanchot@gmail.com) fixed :
                  printNumber, getKey and getString
    18 Feb 2016 - Regis Blanchot (rblanchot@gmail.com) :
                  * use of printFormated.c, printFloat.c and printNumber.c
                  * optimized CDCsend, CDCRead, 
                  * added 16F1549 support (still not working, codesize too large)
                  * changed cdc_init() to CDC_begin(u32 baudrate)
    14 Sep 2016 -  Regis Blanchot (rblanchot@gmail.com) changed :
                  * void CDCwrite(u8 len) in u8 CDCwrite(u8 *buffer, u8 len)
                  * void CDC.read() in u8 CDC.read(u8* buffer, u8 len)
    ------------------------------------------------------------------*/

#ifndef __USBCDC__
#define __USBCDC__

#define USB_USE_CDC
//#define USB_USE_DEBUG

#include <compiler.h>
#include <typedef.h>
#include <macro.h>
#include <delayms.c>

// Debug
#ifdef USB_USE_DEBUG
#define SERIALPRINTF
#include <serial.c>
#endif

//#include <usb/usb_cdc.h>
#include <usb/usb_cdc.c>
//#include <usb/usb_config.h>
//#include <usb/usb_config.c>
//#include <usb/picUSB.h>
#include <usb/picUSB.c>

#include <string.h>

//#ifdef boot2
//    #include <delayms.c>
//#endif

// Printf
#if defined(CDCPRINTF)
    #include <printFormated.c>
    #include <stdarg.h>
#endif

// PrintFloat
#if defined(CDCPRINTFLOAT)
    #include <printFloat.c>
#endif

// PrintNumber
#if defined(CDCPRINTNUMBER) || defined(CDCPRINTFLOAT)
    #include <printNumber.c>
#endif

u32 cdc_baudrate = USB_CDC_BAUDRATE_DEFAULT;

/***********************************************************************
 * USB CDC init routine
 * called from main.c
 * 2016-02-18 - RB - renamed cdc_init to CDC_begin(u32 baudrate)
 * where baudrate can be 9600, 38400 or 115200
 **********************************************************************/

void CDCbegin(u32 baudrate)
{
    u32 counter=-1;
    u8 i;
    
    #ifdef USB_USE_DEBUG
    Serial_begin(9600);
    #endif

    // Disable global interrupts
    noInterrupts();

    // Disable the USB module
    UCON=0;
    UCFG=0;

    UEP1=0;UEP2=0;UEP3=0;
    UEP4=0;UEP5=0;UEP6=0;
    UEP7=0;UEP8=0;UEP9=0;
    UEP10=0;UEP11=0;UEP12=0;
    UEP13=0;UEP14=0;UEP15=0;

    Delayms(2000);
    
    // Enable pullup resistors; full speed mode (0x14)
    #ifdef __XC8__
    UCFG = _UCFG_UPUEN_MASK | _UCFG_FSEN_MASK;
    #else
    UCFG = _UPUEN | _FSEN;
    #endif
    
    deviceState = DETACHED;
    remoteWakeup = 0;
    currentConfiguration = 0;
    cdc_baudrate = baudrate;

    // Check USB activity
    #if 0
    while (deviceState != CONFIGURED)
    {
        EnableUSBModule();
        ProcessUSBTransactions();
    }
    #else
    while (counter-- && deviceState != CONFIGURED)
    {
        EnableUSBModule();
        ProcessUSBTransactions();
    }
    #endif

    // Enable Interrupt
    #if defined(__16F1459)
        PIR2bits.USBIF = 0;     // clear usb interrupt flag
        PIE2bits.USBIE = 1;     // enable usb interrupt
    #elif defined(__18f25k50) || defined(__18f45k50)
        PIR3bits.USBIF = 0;     // clear usb interrupt flag
        PIE3bits.USBIE = 1;     // enable usb interrupt
        IPR3bits.USBIP = 1;     // high priority interrupt
    #else
        PIR2bits.USBIF = 0;     // clear usb interrupt flag
        PIE2bits.USBIE = 1;     // enable usb interrupt
        IPR2bits.USBIP = 1;     // high priority interrupt
    #endif
    
    interrupts();
}

/***********************************************************************
 * USB CDC write routine (CDC.write)
 * send len bytes of the CDCTxBuffer on CDC port
 * returns number of bytes written
 * THIS FUNCTION IS CALLED BY ALMOST ALL THE OTHERS
 **********************************************************************/

u8 CDCwrite(u8 *buffer, u8 len)
{
    u8 i, t=0xFF;

    if (deviceState != CONFIGURED) return 0;

    if (!CONTROL_LINE) return 0;

    if (!EP_IN_BD(USB_CDC_DATA_EP_NUM).Stat.UOWN)
    {
        if (len > USB_CDC_IN_EP_SIZE)
            len = USB_CDC_IN_EP_SIZE;
        
        for (i=0; i<len; i++)
            CDCTxBuffer[i] = buffer[i];
        
        //EP_IN_BD(USB_CDC_DATA_EP_NUM).ADDR = PTR16(&CDCTxBuffer); 
        // Set counter to num bytes ready for send
        EP_IN_BD(USB_CDC_DATA_EP_NUM).Cnt = len;
        // clear BDT Stat bits beside DTS
        EP_IN_BD(USB_CDC_DATA_EP_NUM).Stat.uc &= BDS_DTS;
        // togle DTS
        EP_IN_BD(USB_CDC_DATA_EP_NUM).Stat.DTS ^= 1;
        // reset Buffer to original state
        EP_IN_BD(USB_CDC_DATA_EP_NUM).Stat.uc |= (BDS_UOWN | BDS_DTSEN);
    }
    
    while(t--);
    
    return i;
}

/***********************************************************************
 * USB CDC read routine (CDC.read)
 * read CDC port and fill CDCRxBuffer
 * returns number of bytes read
 **********************************************************************/

#if defined(CDCREAD) || defined(CDCGETKEY) || defined(CDCGETSTRING)
u8 CDCread(u8* buffer, u8 len)
{
    u8 i=0;
    
    if (deviceState != CONFIGURED) return 0;
    
    // Only Process if we own the buffer (aka not own by SIE)
    if (!CONTROL_LINE) return 0;
    
    // Only process if a serial device is connected
    if (!EP_OUT_BD(USB_CDC_DATA_EP_NUM).Stat.UOWN)
    {
        for (i=0; i < EP_OUT_BD(USB_CDC_DATA_EP_NUM).Cnt; i++)
            buffer[i] = CDCRxBuffer[i];

        // clear BDT Stat bits beside DTS and then togle DTS
        EP_OUT_BD(USB_CDC_DATA_EP_NUM).Stat.uc &= BDS_DTS;
        EP_OUT_BD(USB_CDC_DATA_EP_NUM).Stat.DTS ^= 1;
        // reset buffer count and handle controll of buffer to USB
        EP_OUT_BD(USB_CDC_DATA_EP_NUM).Cnt = sizeof(CDCRxBuffer);
        EP_OUT_BD(USB_CDC_DATA_EP_NUM).Stat.uc |= (BDS_UOWN | BDS_DTSEN);

        return i;
    }
        
    return 0;
}
#endif

/***********************************************************************
 * USB CDCprintChar routine (CDC.printChar)
 * write 1 char on CDC port
 * CAUTION : THIS IS A CALLBACK FUNCTION AND CAN NOT BE A MACRO
 **********************************************************************/

#if defined(CDCPRINTCHAR) || defined(CDCPRINTNUMBER) || \
    defined(CDCPRINTFLOAT) || defined(CDCGETSTRING)
void CDCprintChar(u8 c)
{
    CDCwrite(&c, 1);
}
#endif

/***********************************************************************
 * USB CDC print routine (CDC.print)
 * write a string on CDC port
 **********************************************************************/

#if defined(CDCPRINT) || defined(CDCPRINTLN)
#define CDCprint(string)    CDCwrite(string, strlen(string))
/*
void CDCprint(const char *string)
{
    CDCwrite(string, strlen(string));
}
*/
#endif

/***********************************************************************
 * USB CDC print routine (CDC.println)
 * write a string followed by a carriage return character (ASCII 13, or '\r')
 * and a newline character (ASCII 10, or '\n') on CDC port
 **********************************************************************/

#if defined(CDCPRINTLN)
void CDCprintln(const char *string)
{
    u8 len=0;
    char *s = &CDCTxBuffer[0];

    while (*string)
    {
        *s++ = *string++;
        len++;
    }
    *s++ = '\r';
    *s++ = '\n';
    
    CDCwrite(CDCTxBuffer, len+2);
}
#endif

/***********************************************************************
 * USB CDC printNumber routine (CDC.printNumber)
 * write a number on CDC port
 * base : see const.h (DEC, BIN, HEXA, OCTO, ...)
 **********************************************************************/

#if defined(CDCPRINTNUMBER) || defined(CDCPRINTFLOAT)
#define CDCprintNumber(value, base)    printNumber(CDCprintChar, value, base)
/*
void CDCprintNumber(long value, u8 base)
{  
    printNumber(CDCprintChar, value, base);
}
*/
#endif

/***********************************************************************
 * USB CDC printFloat routine (CDC.printFloat)
 * write a float number with n digits on CDC port
 **********************************************************************/

#if defined(CDCPRINTFLOAT)
#define CDCprintFloat(number, digits)    printFloat(CDCprintChar, number, digits)
/*
void CDCprintFloat(float number, u8 digits)
{ 
    printFloat(CDCprintChar, number, digits);
}
*/
#endif

/***********************************************************************
 * USB CDC printf routine (CDC.printf)
 * write a formated string on CDC port
 **********************************************************************/

#if defined(CDCPRINTF)
void CDCprintf(const u8 *fmt, ...)
{
    u8 len;
    va_list	args;

    va_start(args, fmt);
    len = psprintf2(CDCTxBuffer, fmt, args);
    CDCwrite(CDCTxBuffer, len);
    va_end(args);
}
#endif

/***********************************************************************
 * USB CDC getKey routine (CDC.getKey)
 * wait and return a char from CDC port
 **********************************************************************/

#if defined(CDCGETKEY) || defined(CDCGETSTRING)
u8 CDCgetkey(void)
{
    u8 c=0;
    while (!CDCread(&c, 1));
    return c;
}
#endif

/***********************************************************************
 * USB CDC getString routine (CDC.getString)
 * wait and return a string from CDC port
 **********************************************************************/

#if defined(CDCGETSTRING)
u8* CDCgetstring(void)
{
    u8 c, i=0;
    static u8 buffer[32];

    do {
        c = CDCgetkey();
        if (CDCwrite(&c, 1) == 1)
            buffer[i++] = c;
    } while (c != '\r');
    buffer[i-1] = 0;            // EOL
    
    return buffer;
}
#endif

/***********************************************************************
 * USB CDC interrupt routine
 * called from main.c
 **********************************************************************/
 
void CDC_interrupt(void)
{
    #if defined(__18f25k50) || defined(__18f45k50)
    if(PIR3bits.USBIF)
    {
        PIR3bits.USBIF = 0;
    #else
    if(PIR2bits.USBIF)
    {
        PIR2bits.USBIF = 0;
    #endif
        ProcessUSBTransactions();
        //UIRbits.SOFIF = 0;
        //UIRbits.URSTIF = 0;
        //UEIR = 0;
    }
}

#endif /* __USBCDC__ */
