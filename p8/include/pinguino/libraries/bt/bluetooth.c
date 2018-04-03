/*  --------------------------------------------------------------------
    FILE:           bluetooth.c
    PROJECT:        Pinguino
    PURPOSE:        BGB203 basic functions
    PROGRAMER:      Regis Blabnchot <rblanchot@gmail.com>
    --------------------------------------------------------------------
    CHANGELOG :
    28 Oct. 2011    Regis Blabnchot - first release
    12 Sep. 2017    Regis Blabnchot - adapted to 8-bit Pinguinos
    --------------------------------------------------------------------
    TODO :
    * +BTRNM    getRemoteDeviceName
    * +BTSDP    getRemoteDeviceServices
    * +BTTST    setTestMode
    * +BTTX     setTestTransmission
    * +BTCOD    getDeviceClass
    * +BTLSV    setTimeOut
    * +BTPWR    setPower
    * +BTPIN    setPinCode (default is 0000)
    * +BTRXT
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

#ifndef __BLUETOOTH_C
#define __BLUETOOTH_C

#include <stdlib.h>             // ultoa
#include <stdarg.h>             // variable args
#include <string.h>             // strlen, ...
#include <typedef.h>            // u8, u32, ... definitions
#include <pin.h>                // USERLED
#include <bt/bluetooth.h>       // functions prototypes
#define SERIALUSEPORT1
#define SERIALUSEPORT2
//#define SERIALUSEPORTSW
#include <serial.c>             // Serial functions
#ifndef __PIC32MX__
//#include <usbcdc.c>             // CDC functions
#include <delayms.c>            // DelayMs
//#include <digitalt.c>           // toggle
#else
#include <delay.c>              // DelayMs
//#include <digitalw.c>           // toggle
#include <itoa.c>               // ultoa()
#endif

//  --------------------------------------------------------------------
//  Send an AT command to the module over the UART Port
//  An entire command sequence will have the following format:
//  <Prefix><Command><CR>
//  --------------------------------------------------------------------

BT_STATUS BT_sendCommand(u8 uart_port, u8 *fmt, ...)
{
    BT_RESPONSE *response;
    va_list args;
    va_start(args, fmt);    

    #ifndef __PIC32MX__
    //while (!Serial_available(uart_port));
    UART_Module = uart_port;
    //Serial_printf(uart_port, fmt, args);
    pprintf(Serial_printChar, fmt, args);
    #else
    //while (!SerialAvailable(uart_port));
    SerialPrintf(uart_port, fmt, args);
    #endif

    va_end(args);

    Delayms(BT_DELAY);

    #if defined(BT_USE_HC05) || defined(BT_USE_HC06)
    response = BT_getExtendedResponse(uart_port);
    #else
    response = BT_getCommandResponse(uart_port);
    #endif
    
    return BT_getStatus(response);
    //return BT_OK;
}

//  --------------------------------------------------------------------
//  Send data to the module over the UART Port
//  --------------------------------------------------------------------

void BT_send(u8 uart_port, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    
    #ifndef __PIC32MX__
    //while (!Serial_available(uart_port));
    UART_Module = uart_port;
    //Serial_printf(uart_port, fmt, args);
    pprintf(Serial_printChar, fmt, args);
    #else
    //while (!SerialAvailable(uart_port));
    SerialPrintf(uart_port, fmt, args);
    #endif

    va_end(args);

    Delayms(BT_DELAY);
}

/**------------------------------------------------------------------**/
#if defined(BT_USE_BGB203)
/**------------------------------------------------------------------**/

//  --------------------------------------------------------------------
//  Initialize the BGB203 Bluetooth module
//  uart_port : UART1, UART2 or UARTx depending on board used
//  return : name of the device (Pinguino)
//  --------------------------------------------------------------------

BT_STATUS BT_init(u8 uart_port, u32 baud_rate)
{
    BT_STATUS status;
    
    // Initialize Serial communication on uart_port
    #ifndef __PIC32MX__
    Serial_begin(uart_port, 115200, NULL);
    #else
    SerialConfigure(uart_port, UART_ENABLE, UART_RX_TX_ENABLED, 115200);
    #endif
    
    // Get into command mode
    //status = BT_setCommandMode(uart_port);
    //if (status != BT_OK) return BT_ERROR;

    //status = BT_getFirmware(uart_port);
    //if (status != BT_OK) return BT_ERROR;

    //status += BT_echoOff(uart_port);
    //if (status != BT_OK) return BT_ERROR;

    // Restore all factory settings
    //status = BT_restore(uart_port);
    //if (status != BT_OK) return BT_ERROR;

    // Change device local name
    //status = BT_setDeviceName(uart_port, (u8*)"Pinguino");
    //if (status != BT_OK) return BT_ERROR;

    // Pass through, DCE, enable escape sequence, disable entering in command mode with DTR/DSR, enable LED
    //status = BT_sendCommand(uart_port, (u8*)"AT+BTCFG=33");
    //if (status != BT_OK) return BT_ERROR;

    // Set Auto connection mode
    //status = BT_setAutoConnection(uart_port);
    //if (status != BT_OK) return BT_ERROR;

    // Set up the UART
    //status = BT_setUARTSpeed(uart_port, baud_rate);
    //if (status != BT_OK) return BT_ERROR;

    // No security required
    //status = BT_setSecurity(uart_port, 0);
    //if (status != BT_OK) return BT_ERROR;

    // Save settings in flash memory
    //status = BT_setSettings(uart_port);
    //if (status != BT_OK) return BT_ERROR;

    // Start a SPP server
    //status = BT_start(uart_port);
    return status;
}

//  --------------------------------------------------------------------
//  AT+BTPAR=<X>, <BDADDR>,<LINKKEY>
//  with X=0x01 : Pair with remote device
//  AT+BTCLT=<BDADDR>, <Port>, <Attempts>, <Period>
//  0x00 Delete any stored link key
//  0x01 Pair with remote device (initiate)
//  0x02 Allow another device to pair (wait for pair)
//  0x03 Configure Bluetooth address/Link key pair
//  --------------------------------------------------------------------

BT_STATUS BT_pair(u8 uart_port, u8 * bdaddr)
{
    BT_STATUS status;
    // Pair with remote device
    // This will complete when the remote pairing operation is complete
    // or is cancelled locally.
    status = BT_sendCommand(uart_port, (u8*)"AT+BTPAR=1, %s\r", bdaddr);
    if (status != BT_OK) return BT_ERROR;
    // on Port 1
    status = BT_sendCommand(uart_port, (u8*)"AT+BTCLT=%s, 1\r", bdaddr);
    return status;
    // Note that the device enters in Data Mode now (after +BTCLT command)
}

//  --------------------------------------------------------------------
//  Get a complete response from the module
//  --------------------------------------------------------------------

void BT_read(u8 uart_port, u8 *buffer)
{
    u8 c, i = 0;
    
    // wait until something is received
    /*
    #ifndef __PIC32MX__
    while (!Serial_available(uart_port));
    #else
    while (!SerialAvailable(uart_port));
    #endif
    */
    
    do {
        #ifndef __PIC32MX__
        c = Serial_readChar(uart_port);
        #else
        c = SerialRead(uart_port);
        #endif
        //CDCprintChar(c);
        // Serial_readCahr returns 255 (-1) if there is no reception
        /*
        if (c != 255)
            buffer[i++] = c;
        else
            buffer[i++] = '\0';
        */
        buffer[i++] = (c != 255) ? c : '\0';
    } while (c != 255 && i < sizeof(buffer));
    // C-string must be null-terminated
}

//  --------------------------------------------------------------------
//  Get status response
//  --------------------------------------------------------------------

BT_STATUS BT_getStatus(BT_RESPONSE *response)
{
    BT_STATUS code;
    
    // check first char if status is OK, ERROR or COMPLETE
    switch (response->status[0])
    {
        case 'O': code = BT_OK;         break;
        case 'E': code = BT_ERROR;      break;
        case 'C': code = BT_COMPLETE;   break;
    }
    return code;
}

//  --------------------------------------------------------------------
//  Get command response
//  --------------------------------------------------------------------

u8* BT_getCommand(BT_RESPONSE *response)
{
    return response->command;
}

//  --------------------------------------------------------------------
//  Get data response
//  --------------------------------------------------------------------

u8* BT_getData(BT_RESPONSE *response)
{
    return response->data;
}

//  --------------------------------------------------------------------
//  Get a short response from the module
//  <CR><LF><status><CR><LF>
//  with status = OK or ERROR
//  --------------------------------------------------------------------

BT_RESPONSE* BT_getCommandResponse(u8 uart_port)
{
    u8 buffer[64];
    static BT_RESPONSE response;

    // get a complete response from the module
    BT_read(uart_port, &buffer);

    // No Command or Data are return with normal commands
    response.command = NULL;
    response.data    = NULL;

    response.status  = strtok(buffer, BT_CRLF); // remove the first CRLF 
    response.status  = strtok(NULL, BT_CRLF);   // get the status

    return &response;
}

//  --------------------------------------------------------------------
//  Get an extended response from the module
//  <CR><LF><command><CR><LF><data><CR><LF><status><CR><LF>
//  --------------------------------------------------------------------

BT_RESPONSE* BT_getExtendedResponse(u8 uart_port)
{
    u8 buffer[64];
    static BT_RESPONSE response;
    
    // get a complete response from the module
    BT_read(uart_port, &buffer);

    // remove the first CRLF 
    response.command = strtok(buffer, BT_CRLF);
    // get the command name after the first <CR><LF>
    response.command = strtok(NULL, BT_DELIMITER);
    // get data after the next ": "
    response.data    = strtok(NULL, BT_CRLF);
    // get status before the last <CR><LF>
    response.status  = strtok(NULL, BT_CRLF);

    return &response;
}

//  --------------------------------------------------------------------
//  Get into command mode
//  In Command Mode, the module firmware assumes that data coming from
//  the UART are commands.
//  "+++" is the default escape sequence
//  It could be changed from 1 to 4 characters in length :
//  AT+BTESC=<Length>, <Byte 1>, <Byte 2>, <Byte 3>, <Byte 4>
//  --------------------------------------------------------------------

BT_STATUS BT_setCommandMode(u8 uart_port)
{
    BT_STATUS status;
    
    //return BT_sendCommand(uart_port, (u8*)"+++\r");
    status  = BT_sendCommand(uart_port, (u8*)"+++\r");
    status += BT_sendCommand(uart_port, (u8*)"+++\r");
    status += BT_sendCommand(uart_port, (u8*)"+++\r");
    return status;
}

//  --------------------------------------------------------------------
//  Configures the device not to echo received characters in command mode
//  --------------------------------------------------------------------

BT_STATUS BT_echoOff(u8 uart_port)
{
    return BT_sendCommand(uart_port, (u8*)"ATE0\r");
}

//  --------------------------------------------------------------------
//  Configures the device to echo received characters in command mode
//  --------------------------------------------------------------------

BT_STATUS BT_echoOn(u8 uart_port)
{
    return BT_sendCommand(uart_port, (u8*)"ATE1\r");
}

//  --------------------------------------------------------------------
//  Restore the current configuration settings back to the settings
//  that were stored by the Factory Settings tool
//  or settings that were stored to Flash.
//  --------------------------------------------------------------------

BT_STATUS BT_restore(u8 uart_port)
{
    return BT_sendCommand(uart_port, (u8*)"AT&F\r");
}

//  --------------------------------------------------------------------
//  Restore the current configuration settings back to internal default values
//  --------------------------------------------------------------------

BT_STATUS BT_reset(u8 uart_port)
{
    return BT_sendCommand(uart_port, (u8*)"ATZ\r");
}

//  --------------------------------------------------------------------
//  Ask the module for its firmware version 
//  --------------------------------------------------------------------

u8* BT_getFirmware(u8 uart_port)
{
    BT_RESPONSE *response;
    BT_sendCommand(uart_port, (u8*)"ATI\r");
    response = BT_getExtendedResponse(uart_port);
    return BT_getData(response);
}

//  --------------------------------------------------------------------
//  Change module name
//  AT+BTLNM="<Local Name>"
//  --------------------------------------------------------------------

BT_STATUS BT_setDeviceName(u8 uart_port, u8 *name)
{
    BT_RESPONSE *response;
    BT_sendCommand(uart_port, (u8*)"AT+BTLNM=\"%s\"\r", name); 
    response = BT_getExtendedResponse(uart_port);
    return BT_getStatus(response);
}

//  --------------------------------------------------------------------
//  Get module name
//  --------------------------------------------------------------------

u8* BT_getDeviceName(u8 uart_port)
{
    BT_RESPONSE *response;
    BT_sendCommand(uart_port, (u8*)"AT+BTLNM?\r");
    response = BT_getExtendedResponse(uart_port);
    return BT_getData(response);
}

//  --------------------------------------------------------------------
//  Set bluetooth device address (0x01)
//  --------------------------------------------------------------------

BT_STATUS BT_setDeviceAddress(u8 uart_port, u8 *bdaddr)
{
    BT_RESPONSE *response;
    BT_sendCommand(uart_port, (u8*)"AT+BTSET=1, %s\r", bdaddr);
    response = BT_getExtendedResponse(uart_port);
    return BT_getStatus(response);
}

//  --------------------------------------------------------------------
//  Get bluetooth device address
//  --------------------------------------------------------------------

u8* BT_getDeviceAddress(u8 uart_port)
{
    BT_RESPONSE *response;
    BT_sendCommand(uart_port, (u8*)"AT+BTBDA?\r");
    response = BT_getExtendedResponse(uart_port);
    return BT_getData(response);
}

//  --------------------------------------------------------------------
//  Allows automatic Bluetooth connection
//  AT+BTAUT=<X>, <Y>
//  <X> 0 or 1 : disable or enable auto connect mode
//  <Y> 0 or 1 : do not store or store auto connect mode setting to Flash
//  --------------------------------------------------------------------

BT_STATUS BT_setAutoConnection(u8 uart_port)
{
    BT_RESPONSE *response;
    BT_sendCommand(uart_port, (u8*)"AT+BTAUT=1, 0\r");
    response = BT_getExtendedResponse(uart_port);
    return BT_getStatus(response);
}

//  --------------------------------------------------------------------
//  Change UART settings
//  AT+BTURT=<Baud>, <Data Bits>, <Parity>, <Stop Bits>, <FlowControl>
//  Default baud rate is 115200
// Note : char * ultoa(char * buf, unsigned long val, int base);
//  --------------------------------------------------------------------

BT_STATUS BT_setUARTSpeed(u8 uart_port, u32 baud_rate)
{
    char *string;
    BT_RESPONSE *response;

    #ifndef __PIC32MX__
    BT_sendCommand(uart_port, (u8*)"AT+BTURT=%s, 8, 0, 1, 3\r", ultoa(string, baud_rate, 10));
    Serial_begin(uart_port, baud_rate, NULL);
    #else
    BT_sendCommand(uart_port, (u8*)"AT+BTURT=%s, 8, 0, 1, 3\r", ultoa(baud_rate, string, 10));
    SerialConfigure(uart_port, UART_ENABLE, UART_RX_TX_ENABLED, baud_rate);
    #endif
    response = BT_getExtendedResponse(uart_port);
    return BT_getStatus(response);
}

//  --------------------------------------------------------------------
//  Set security level when pairing
//  --------------------------------------------------------------------

BT_STATUS BT_setSecurity(u8 uart_port, u8 level)
{
    BT_RESPONSE *response;
    BT_sendCommand(uart_port, (u8*)"AT+BTSEC=%d\r", level);
    response = BT_getExtendedResponse(uart_port);
    return BT_getStatus(response);
}

//  --------------------------------------------------------------------
//  Writes settings to the BGB203 flash memory
//  Result will be the result code (either BT_OK or BT_ERROR).
//  --------------------------------------------------------------------

BT_STATUS BT_setSettings(u8 uart_port)
{
    BT_RESPONSE *response;
    BT_sendCommand(uart_port, (u8*)"AT+BTFLS\r");
    response = BT_getExtendedResponse(uart_port);
    return BT_getStatus(response);
}

//  --------------------------------------------------------------------
//  Start Bluetooth Server
//  AT+BTSRV=<Port>, “<Service Name>”, <ACL BDADDR>, <Flags>
//  --------------------------------------------------------------------

BT_STATUS BT_start(u8 uart_port)
{
    BT_RESPONSE *response;
    BT_sendCommand(uart_port, (u8*)"AT+BTSRV=1\r");
    response = BT_getExtendedResponse(uart_port);
    return BT_getStatus(response);
}

//  --------------------------------------------------------------------
//  Enter deep sleep mode
//  --------------------------------------------------------------------

BT_STATUS BT_sleep(u8 uart_port)
{
    BT_RESPONSE *response;
    BT_sendCommand(uart_port, (u8*)"AT+BTSLP\r");
    response = BT_getExtendedResponse(uart_port);
    return BT_getStatus(response);
}

//  --------------------------------------------------------------------
//  Search for new devices for s seconds
//  Result will be one (or more lines) of one of the following formats:
//  +BTINQ: <BDADDR>, <CLASS>
//  When the Inquiry is complete, the result will have the following format:
//  +BTINQ: COMPLETE
//  --------------------------------------------------------------------

u8* BT_search(u8 uart_port, u8 s)
{
    BT_RESPONSE *response;
    // cancels any current commands
    BT_sendCommand(uart_port, (u8*)"AT+BTCAN\r");
    BT_sendCommand(uart_port, (u8*)"AT+BTINQ=%d\r", s);
    response = BT_getExtendedResponse(uart_port);
    return BT_getData(response);
}

BT_STATUS BT_ok(u8 uart_port)
{
    u8 i;
    BT_STATUS ret;

    #ifndef __PIC32MX__
    while ( !(ret = Serial_readChar(uart_port)) );
    #else
    while ( !(ret = SerialRead(uart_port)) );
    #endif
    
    switch(ret)
    {
        case 'O': ret = BT_OK;       break;
        case 'C': ret = BT_COMPLETE; break;
        case 'E': ret = BT_ERROR;    break;
        default : ret = BT_ERROR;    break;
    }

    return (ret == BT_OK);
}

/**------------------------------------------------------------------**/
#elif defined(BT_USE_HC05) || defined(BT_USE_HC06)
/**------------------------------------------------------------------**/

//  --------------------------------------------------------------------
//  Initialize the BGB203 Bluetooth module
//  uart_port : UART1, UART2 or UARTx depending on board used
//  return : name of the device (Pinguino)
//  --------------------------------------------------------------------

BT_STATUS BT_init(u8 uart_port, u32 baud_rate)
{
    BT_STATUS status;
    
    // Initialize Serial communication on uart_port
    #ifndef __PIC32MX__
    Serial_begin(uart_port, baud_rate, NULL);
    #else
    SerialConfigure(uart_port, UART_ENABLE, UART_RX_TX_ENABLED, baud_rate);
    #endif
    
    // Check the connection
    //BT_sendCommand(uart_port, (u8*)"AT");
    //response = BT_getExtendedResponse(uart_port);
    //return BT_getStatus(response);
    
    // Set up the UART
    status = BT_setUARTSpeed(uart_port, baud_rate);
    //if (status != BT_OK) return BT_ERROR;

    // Change device local name
    status = BT_setDeviceName(uart_port, (u8*)"Pinguino");
    //if (status != BT_OK) return BT_ERROR;

    return status;
}

//  --------------------------------------------------------------------
//  Change UART settings
//  AT+BTURT=<Baud>, <Data Bits>, <Parity>, <Stop Bits>, <FlowControl>
//  Default baud rate is 115200
//  Note : char * ultoa(char * buf, unsigned long val, int base);
//  --------------------------------------------------------------------

BT_STATUS BT_setUARTSpeed(u8 uart_port, u32 baud_rate)
{
    u32 speed[] = {1200,2400,4800,9600,19200,38400,57600,115200};
    u8 index=0;
    BT_RESPONSE *response;

    while (index++ < sizeof(speed))
        if (speed[index] == baud_rate)
            continue;

    if (index == 8)
        index = 4;   // default speed (9600 bauds)
    else
        index += 1;

    //BT_sendCommand(uart_port, (u8*)"AT+BAUD%d", index);
    BT_sendCommand(uart_port, (u8*)"AT+BAUD4");
    response = BT_getExtendedResponse(uart_port);
    return BT_getStatus(response);
    //return BT_OK;
}

//  --------------------------------------------------------------------
//  Change module name
//  AT+NAMEname
//  --------------------------------------------------------------------

BT_STATUS BT_setDeviceName(u8 uart_port, u8 *name)
{
    BT_RESPONSE *response;
    BT_sendCommand(uart_port, (u8*)"AT+NAME%s", name); 
    response = BT_getExtendedResponse(uart_port);
    return BT_getStatus(response);
    //return BT_OK;
}

//  --------------------------------------------------------------------
//  Change PIN code
//  AT+PINxxxx
//  --------------------------------------------------------------------

BT_STATUS BT_setPincode(u8 uart_port, u16 pin)
{
    BT_RESPONSE *response;
    BT_sendCommand(uart_port, (u8*)"AT+PIN%04d", pin); 
    response = BT_getExtendedResponse(uart_port);
    return BT_getStatus(response);
    //return BT_OK;
}

//  --------------------------------------------------------------------
//  Ask the module for its firmware version 
//  --------------------------------------------------------------------

u8* BT_getFirmware(u8 uart_port)
{
    BT_RESPONSE *response;
    #if defined(BT_USE_HC06)
    BT_sendCommand(uart_port, (u8*)"AT+VERSION");
    #else
    BT_sendCommand(uart_port, (u8*)"AT+VERSION?\r\n");
    #endif
    response = BT_getExtendedResponse(uart_port);
    return BT_getData(response);
}

//  --------------------------------------------------------------------
//  Get a complete response from the module
//  --------------------------------------------------------------------

void BT_read(u8 uart_port, u8 *buffer)
{
    u8 c, i = 0;
    
    // wait until something is received
    #ifndef __PIC32MX__
    while (Serial_available(uart_port))
        buffer[i++] = Serial_readChar(uart_port);
    #else
    while (SerialAvailable(uart_port))
        buffer[i++] = SerialRead(uart_port);
    #endif
    buffer[i++] = '\0';
}

//  --------------------------------------------------------------------
//  Get an extended response from the module
//  <CR><LF><command><CR><LF><data><CR><LF><status><CR><LF>
//  --------------------------------------------------------------------

BT_RESPONSE* BT_getExtendedResponse(u8 uart_port)
{
    u8 buffer[64];
    static BT_RESPONSE response;
    
    // get a complete response from the module
    BT_read(uart_port, &buffer);

    // the command is not echoed
    response.command = NULL;
    // get status before the last <CR><LF>
    if (strstr((const char *)"OK", buffer))
    {
        response.status  = "OK";
        response.data    = strtok(buffer, (const char *)"OK");
    }
    else
    {
        response.status  = "ERROR";
        response.data    = NULL;
    }
    
    return &response;
}

//  --------------------------------------------------------------------
//  Get status response
//  --------------------------------------------------------------------

BT_STATUS BT_getStatus(BT_RESPONSE *response)
{
    BT_STATUS code;
    
    // check first char if status is OK, ERROR or COMPLETE
    switch (response->status[0])
    {
        case 'O': code = BT_OK;         break;
        case 'E': code = BT_ERROR;      break;
        case 'C': code = BT_COMPLETE;   break;
    }
    return code;
}

//  --------------------------------------------------------------------
//  Get command response
//  --------------------------------------------------------------------

u8* BT_getCommand(BT_RESPONSE *response)
{
    return response->command;
}

//  --------------------------------------------------------------------
//  Get data response
//  --------------------------------------------------------------------

u8* BT_getData(BT_RESPONSE *response)
{
    return response->data;
}


#endif /* BT_USE_BGB203 */

#endif /* __BLUETOOTH_C */
