/*	----------------------------------------------------------------------------
    FILE:			bluetooth.h
    PROJECT:		pinguino
    PURPOSE:		BGB203 basic functions
    PROGRAMER:		regis blabnchot <rblanchot@gmail.com>
    FIRST RELEASE:	12 Sept. 2017
    LAST RELEASE:	12 Sept. 2017
    ----------------------------------------------------------------------------
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
    --------------------------------------------------------------------------*/

#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

#define CDCPRINTCHAR
#define SERIALREAD
#define SERIALPRINT
#define SERIALPRINTLN
#define SERIALPRINTNUMBER
#define SERIALPRINTFLOAT
#define SERIALPRINTF

#include <stdarg.h>             // variable args
#include <typedef.h>            // u8, u32, ... definitions

// Delay (ms) before sending a new command to the BT module
#if defined (BT_USE_BGB203)
#define BT_DELAY        10
#else
#define BT_DELAY        1000
#endif

#define BT_CRLF         "/r/n"  // <CR><LF>
#define BT_CR           "/r"    // <CR>
#define BT_PREFIX       "AT"    
#define BT_DELIMITER    ": "    

// BT return codes
typedef enum
{
    BT_OK = 111,
    BT_ERROR,
    BT_COMPLETE
} BT_STATUS;

// BT response
typedef struct
{
    u8 *command;
    u8 *data;
    u8 *status;     // OK or ERROR
} BT_RESPONSE;

BT_STATUS BT_init(u8 uart_port, u32 baud_rate);
BT_STATUS BT_pair(u8 uart_port, u8 * bdaddr);
BT_STATUS BT_start(u8 uart_port);
BT_STATUS BT_sleep(u8 uart_port);
BT_STATUS BT_ok(u8 uart_port);

void BT_read(u8 uart_port, u8 *buffer);
void BT_send(u8 uart_port, char *fmt, ...);

u8* BT_getDeviceName(u8 uart_port);
u8* BT_getDeviceAddress(u8 uart_port);
u8* BT_getFirmware(u8 uart_port);
u8* BT_search(u8 uart_port, u8 s);

BT_RESPONSE* BT_getExtendedResponse(u8 uart_port);
BT_RESPONSE* BT_getCommandResponse(u8 uart_port);

BT_STATUS BT_getStatus(BT_RESPONSE *response);
u8* BT_getData(BT_RESPONSE *response);
u8* BT_getCommand(BT_RESPONSE *response);

BT_STATUS BT_sendCommand(u8 uart_port, u8 *fmt, ...);
BT_STATUS BT_setCommandMode(u8 uart_port);

BT_STATUS BT_echoOff(u8 uart_port);
BT_STATUS BT_echoOn(u8 uart_port);
BT_STATUS BT_restore(u8 uart_port);
BT_STATUS BT_reset(u8 uart_port);

BT_STATUS BT_setPincode(u8 uart_port, u16 pin);
BT_STATUS BT_setDeviceName(u8 uart_port, u8 * name);
BT_STATUS BT_setDeviceAddress(u8 uart_port, u8 * bdaddr);
BT_STATUS BT_setAutoConnection(u8 uart_port);
BT_STATUS BT_setUARTSpeed(u8 uart_port, u32 baud_rate);
BT_STATUS BT_setSecurity(u8 uart_port, u8 level);
BT_STATUS BT_setSettings(u8 uart_port);

#define BT_begin(m, p, b) BT_init(p, b)

#endif /* __BLUETOOTH_H */
