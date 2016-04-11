/*	--------------------------------------------------------------------
    FILE:         __cdc.c
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
                  printFloat still to do
                  init optimized
    ------------------------------------------------------------------*/

#ifndef __USBCDC__
#define __USBCDC__

#define USB_USE_CDC

#include <compiler.h>
#include <typedef.h>
#include <macro.h>
//#include <usb/usb_cdc.h>
#include <usb/usb_cdc.c>
//#include <usb/usb_config.h>
//#include <usb/usb_config.c>
//#include <usb/picUSB.h>
#include <usb/picUSB.c>

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

    // Delayms(2000);
    
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
 * USB CDC send routine (private)
 * added by Regis Blanchot 18/02/2016
 * send len bytes of the CDCTxBuffer on CDC port
 **********************************************************************/

static void CDCsend(u8 len)
{
    u8 t=0xFF;

    if (deviceState != CONFIGURED) return;

    if (!CONTROL_LINE) return;

    if (!EP_IN_BD(USB_CDC_DATA_EP_NUM).Stat.UOWN)
    {
        if (len > USB_CDC_IN_EP_SIZE)
            len = USB_CDC_IN_EP_SIZE;
        
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
}

/***********************************************************************
 * USB CDC read routine (private)
 * added by Regis Blanchot 18/02/2016
 * read CDC port and fill CDCRxBuffer 
 **********************************************************************/

#if defined(CDCGETKEY) || defined(CDCGETSTRING)
static void CDCread()
{
    if (deviceState != CONFIGURED) return;
    
    if (!CONTROL_LINE) return;
    
    if (!EP_OUT_BD(USB_CDC_DATA_EP_NUM).Stat.UOWN)
    {
        // Set counter to num bytes ready for read
        EP_OUT_BD(USB_CDC_DATA_EP_NUM).Cnt = sizeof(CDCRxBuffer);
        // clear BDT Stat bits beside DTS
        EP_OUT_BD(USB_CDC_DATA_EP_NUM).Stat.uc &= BDS_DTS;
        // toggle DTS
        EP_OUT_BD(USB_CDC_DATA_EP_NUM).Stat.DTS ^= 1;
        // reset Buffer to original state
        EP_OUT_BD(USB_CDC_DATA_EP_NUM).Stat.uc |= (BDS_UOWN | BDS_DTSEN);
    }
}
#endif

/***********************************************************************
 * USB CDCprintChar routine (CDC.printChar or CDC.write)
 * added by regis blanchot 14/06/2011
 * write 1 char on CDC port
 **********************************************************************/

void CDCprintChar(u8 c)
{
    CDCTxBuffer[0] = c;
    CDCsend(1);
}

//#define CDCprintChar(c)     printChar(c)

/***********************************************************************
 * USB CDC print routine (CDC.print)
 * added by regis blanchot 04/03/2014
 * write a string on CDC port
 **********************************************************************/

#if defined(CDCPRINT) || defined(CDCPRINTLN)
void CDCprint(const char *string)
{
    //CDCputs(string, strlen(string));
    u8 len=0;
    char *s = CDCTxBuffer;

    while (*string)
    {
        *s++ = *string++;
        len++;
    }
 
    CDCsend(len);
}
#endif

/***********************************************************************
 * USB CDC print routine (CDC.println)
 * added by regis blanchot 04/03/2014
 * write a string followed by a carriage return character (ASCII 13, or '\r')
 * and a newline character (ASCII 10, or '\n') on CDC port
 **********************************************************************/

#if defined(CDCPRINTLN)
void CDCprintln(const char *string)
{
    u8 len=0;
    char *s = CDCTxBuffer;

    while (*string)
    {
        *s++ = *string++;
        len++;
    }
    *s++ = '\r';
    *s++ = '\n';
    
    CDCsend(len+2);
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
    printNumber(CDCprintChar, value, base);
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
    printFloat(CDCprintChar, number, digits);
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
    u8 len;
    va_list	args;

    va_start(args, fmt);
    len = psprintf2(&CDCTxBuffer[0], fmt, args);
    CDCsend(len);

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
    //u8 buffer[_CDCBUFFERLENGTH_];		// always get a full packet
    //while (!CDCgets(_cdc_buffer));
    //return (_cdc_buffer[0]);	// return only the first character
    u8 c=0;
    CDCRxBuffer[0]=0;
    do {
        CDCread();
        c = CDCRxBuffer[0];
    } while (!c);
    return c;
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
    //static u8 buffer[_CDCBUFFERLENGTH_];	// Needs static buffer at least.
    
    do {
        c = CDCgetkey();
        buffer[i++] = c;
        //CDCputs(&buffer[i-1], 1);
        //CDCputc(_cdc_buffer[i-1]);
        CDCputc(CDCRxBuffer[i-1]);
    } while (c != '\r');
    _cdc_buffer[i] = '\0';
    return _cdc_buffer;
}
#endif

/***********************************************************************
 * USB CDC interrupt routine
 * added by regis blanchot 05/02/2013
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

