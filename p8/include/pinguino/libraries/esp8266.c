/***********************************************************************
 * C library for the ESP8266 WiFi module with a PIC microcontroller
 * Copyright (C) 2015 Camil Staps <info@camilstaps.nl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 ***********************************************************************
 *
 * File:    esp8266.c
 * Author:  Camil Staps <info@camilstaps.nl>
 * Website: http://github.com/camilstaps/ESP8266_PIC
 * Version: 0.1
 *
 * See:     esp8266.h
 *
 * C library for interfacing the ESP8266 Wi-Fi transceiver module with a PIC18
 * microcontroller. Should be used with the XC8 compiler.
 * 
 * Modified and adapted by Circuitdigest.com
 * Author: Aswinth Raj  <mailtoaswinth@gmail.com>
 * Website: circuitdigest.com
 ***********************************************************************
 * CHANGELOG :
 * 2018-02-13 - Regis Blanchot - added TCP/IP functions
 ***********************************************************************
 * TODO :
 * - I2C support
 * - SPI support
 **********************************************************************/

// Pinguino standards
#include <typedef.h>
#include <const.h>
#include <macro.h>
#include <stdarg.h>
#include <string.h>                     // strlen
//#include <stdio.h>
//#if !defined(SERIALUSEPORTSW)
//#define SERIALUSEPORTSW
//#endif
#if !defined(SERIALUSEPORT1)
#define SERIALUSEPORT1
#endif
#if !defined(SERIALUSEPORT2)
#define SERIALUSEPORT2
#endif
#if !defined(SERIALREAD)
#define SERIALREAD
#endif
//#if defined(WIFIPRINTCHAR) && !defined(SERIALPRINTCHAR)
#if !defined(SERIALPRINTCHAR)
#define SERIALPRINTCHAR
#endif
//#if defined(WIFIPRINT) && !defined(SERIALPRINTSTRING)
#if !defined(SERIALPRINTSTRING)
#define SERIALPRINTSTRING
#endif
#if defined(WIFIPRINTLN) && !defined(SERIALPRINTLN)
#define SERIALPRINTLN
#endif
//#if defined(WIFIPRINTNUMBER) && !defined(SERIALPRINTNUMBER)
#if !defined(SERIALPRINTNUMBER)
#define SERIALPRINTNUMBER
#endif
#if defined(WIFIPRINTFLOAT) && !defined(SERIALPRINTFLOAT)
#define SERIALPRINTFLOAT
#endif
#if defined(WIFIPRINTX) && !defined(SERIALPRINTX)
#define SERIALPRINTX
#endif
#if defined(WIFIPRINTF) && !defined(SERIALPRINTF)
#define SERIALPRINTF
#endif
#include <serial.c>
#include <esp8266.h>

#if !defined(__PIC32MX__)
#include <delayms.c>
#include <delayus.c>
#else
#include <delay.c>
#endif

u8 gESP8266MODE = ESP8266_STATION|ESP8266_SOFTAP;
u8 gESP8266MUX  = ESP8266_ENABLED;

/***********************************************************************
 *                                                                     *
 *                      UART-Related functions                         *
 *                                                                     *
 **********************************************************************/

// Send one byte of date to UART
#define esp8266_putch(u, c)          Serial_printChar(u, c)
#define esp8266_printChar(u, c)      Serial_printChar(u, c)
// Get one byte of date from UART
#define esp8266_getch(u)             (RCREG2) //Serial_readChar(u)
/*
u8 esp8266_getch(uart)
{
    u8 c;
    
    while (!Serial_available(uart));
    c = Serial_readChar(uart);
    Serial_flush(uart);
    return c;
}
*/
// Convert string to bytes
#define esp8266_print(u, s)          Serial_print(u, s)
// Convert integer to bytes
#define esp8266_printNumber(u, v, b) Serial_printNumber(u, v, b)
// Convert float to bytes
#if defined(WIFIPRINTFLOAT)
#define esp8266_printFloat(u, f, d)  Serial_printFloat(u, f, d)
#endif
// Convert format to bytes
#if defined(WIFIPRINTX)
#define esp8266_printx(u, f, b)      Serial_printX(u, f, b)
#endif
// Convert format to bytes
#if defined(WIFIPRINTF)
void esp8266_printf(u8 uart, u8* f, ...)
{
    Serial_printf(uart, f, ...);
}
#endif
/**
 * Initialize the USART module to communicate with the ESP8266 module.
 * The default baud rate is 115200.
 * Set the Wifi mode :
 * @param Wifi mode, either :
 * ESP8266_STATION : Station mode (client)
 * ESP8266_SOFTAP  : Access Point mode (host)
 * ESP8266_STATION|ESP8266_SOFTAP (dual)
*/
u8 esp8266_init(u8 uart)
{
    // Initialize Serial communication on uart_port
    #if defined(__PIC32MX__)
    SerialConfigure(uart, UART_ENABLE, UART_RX_TX_ENABLED, 115200);
    #else
    Serial_begin(uart, 115200, NULL);
    #endif
    
    return esp8266_isStarted(uart);
}

/***********************************************************************
 *                                                                     *
 *                          Basic functions                            *
 *                                                                     *
 **********************************************************************/
/**
 * Updates the Software through Wi-Fi
 * It is suggested to call AT+RESTORE to restore the factory default
 * settings after upgrading the AT firmware.
 */
u8 esp8266_update(u8 uart)
{
    u8 r;
    esp8266_print(uart, (const char *)"AT+CIUPDATE=4\r\n");
    r = esp8266_waitResponse(uart);
    if (r) 
        esp8266_print(uart, (const char *)"AT+RESTORE\r\n");
    return r;
}

/**
 * Wait until we found a string on the input.
 *
 * Careful: this will read everything until that string (even if it's never
 * found). You may lose important data.
 *
 * @param string
 *
 * @return the number of u8acters read
 */
u16 esp8266_waitFor(u8 uart, u8 *string)
{
    u8 so_far = 0;
    u8 received;
    u16 counter = 0;

    do
    {
        received = esp8266_getch(uart);
        counter++;
        (received == string[so_far]) ? so_far++ : so_far = 0;
    }
    while (string[so_far] != 0);
    return counter;
}

/**
 * Wait until we received the ESP is done and sends its response.
 *
 * This is a function for internal use only.
 *
 * Currently the following responses are implemented:
 *  * OK
 *  * ready
 *  * FAIL
 *  * no change
 *  * Linked
 *  * Unlink
 *
 * Not implemented yet:
 *  * DNS fail (or something like that)
 *
 * @return a constant from esp8266.h describing the status response.
 */
u8 esp8266_waitResponse(u8 uart)
{
    u8 i;
    u8 so_far[6] = {0,0,0,0,0,0};
    const u8 lengths[6] = {2,5,4,9,6,6};
    const u8* strings[6] = {"OK", "ready", "FAIL", "no change", "Linked", "Unlink"};
    const u8 responses[6] = {ESP8266_OK, ESP8266_READY, ESP8266_FAIL, ESP8266_NOCHANGE, ESP8266_LINKED, ESP8266_UNLINK};
    u8 received;
    u8 response;
    u8 continue_loop = true;
    
    while (continue_loop)
    {
        Serial_printChar(UART1, '[');
        received = esp8266_getch(uart);
        Serial_printChar(UART1, received);
        Serial_printChar(UART1, ']');
        for (i = 0; i < 6; i++)
        {
            if (strings[i][so_far[i]] == received)
            {
                so_far[i]++;
                if (so_far[i] == lengths[i])
                {
                    response = responses[i];
                    continue_loop = false;
                }
            }
            else
            {
                so_far[i] = 0;
            }
        }
    }
    return response;
}

/**
 * Check if the module is started
 *
 * This sends the `AT` command to the ESP and waits until it gets a response.
 *
 * @return true if the module is started, false if something went wrong
 */
u8 esp8266_isStarted(u8 uart)
{
    esp8266_print(uart, (const char *)"AT\r\n");
    return (esp8266_waitResponse(uart) == ESP8266_OK);
}

/**
 * Restart the module
 *
 * This sends the `AT+RST` command to the ESP
 * and waits until there is a response.
 *
 * @return true if the module restarted properly
 */
u8 esp8266_restart(u8 uart)
{
    esp8266_print(uart, (const char *)"AT+RST\r\n");
    if (esp8266_waitResponse(uart) != ESP8266_OK)
        return false;
    return (esp8266_waitResponse(uart) == ESP8266_READY);
}

/**
 * Configures the sleep modes.
 *
 * This command can only be used in Station mode.
 * Modem-sleep is the default sleep mode.
 *
 * This sends the AT+SLEEP=<sleep mode> command to the ESP module.
 *
 * @param mode :
 * ESP8266_NOSLEEP :    disables sleep
 * ESP8266_LIGHTSLEEP : light sleep
 * ESP8266_MODEMSLEEP : modem sleep
 */
u8 esp8266_sleep(u8 uart, u8 mode)
{
    esp8266_print(uart, (const char *)"AT+SLEEP=");
    esp8266_putch(uart, mode + '0');
    esp8266_print(uart, (const char *)"\r\n");
    return esp8266_waitResponse(uart);
}

/**
 * Enable / disable command echoing.
 *
 * Enabling this is useful for debugging: one could sniff the TX line from the
 * ESP8266 with his computer and thus receive both commands and responses.
 *
 * This sends the ATE command to the ESP module.
 *
 * @param echo whether to enable command echoing or not
 */
void esp8266_echo(u8 uart, u8 echo)
{
    esp8266_print(uart, (const char *)"ATE");
    if (echo)
        esp8266_putch(uart, '1');
    else
        esp8266_putch(uart, '0');
    esp8266_print(uart, (const char *)"\r\n");
    esp8266_waitFor(uart, (u8*)"OK");
}

/***********************************************************************
 *                                                                     *
 *                          Wi-Fi functions                            *
 *                                                                     *
 **********************************************************************/
/* AT+CWMODE : Sets the Wi-Fi mode (STA/AP/STA+AP).
 * AT+CWJAP : Connects to an AP.
   AT+CWLAPOPT : Sets the configuration of command AT+CWLAP.
   AT+CWLAP : Lists available APs.
 * AT+CWQAP : Disconnects from the AP.
 * AT+CWSAP : Sets the configuration of the ESP SoftAP.
 * AT+CWLIF : Gets the Station IP to which the ESP SoftAP is connected.
 * AT+CWDHCP : Enables/disables DHCP.
   AT+CWDHCPS : Sets the IP range of the ESP SoftAP DHCP server. Saves the setting in flash.
 * AT+CWAUTOCONN : Connects to the AP automatically on power-up.
 * AT+CIPSTAMAC : Sets the MAC address of ESP Station.
 * AT+CIPAPMAC : Sets the MAC address of ESP SoftAP.
 * AT+CIPSTA : Sets the IP address of ESP Station.
 * AT+CIPAP : Sets the IP address of ESP SoftAP.
   AT+CWSTARTSMART : Starts SmartConfig.
   AT+CWSTOPSMART : Stops SmartConfig.
   AT+WPS : Enables the WPS function.
 **********************************************************************/

/**
 * Set the WiFi mode.
 *
 * This sends the AT+CWMODE command to the ESP module.
 *
 * @param Wifi mode, either :
 *
 * ESP8266_STATION : Station mode (client)
 * In STA mode, the ESP can connect to an AP (access point) such as
 * the Wi-Fi network from your house.
 * This allows any device connected to that network to communicate with the module.
 * 
 * ESP8266_SOFTAP  : Access Point mode (host)
 * In AP, the Wi-Fi module acts as a Wi-Fi network, or access point (hence the name).
 * It allows other devices to connect to it.
 * It establishes a two-way communication between the ESP8266 and the
 * device that is connected to it via Wi-Fi.
 * 
 * ESP8266_STATION|ESP8266_SOFTAP (dual)
 * In this mode ESP act as both an AP as well as in STA mode.
 */
u8 esp8266_mode(u8 uart, u8 mode)
{
    gESP8266MODE = mode;
    // Deprecated
    //esp8266_print(uart, (const char *)"AT+CWMODE=");
    esp8266_print(uart, (const char *)"AT+CWMODE_CUR=");
    esp8266_putch(uart, mode + '0');
    esp8266_print(uart, (const char *)"\r\n");
    return esp8266_waitResponse(uart);
}

//#define esp8266_clientInit(uart) esp8266_mode(uart, ESP8266_STATION)
//#define esp8266_serverInit(uart) esp8266_mode(uart, ESP8266_SOFTAP)

/**
 * Connect to an access point (router).
 *
 * This sends the AT+CWJAP command to the ESP module.
 *
 * @param ssid The SSID to connect to
 * @param pass The password of the network
 * @return an ESP status code :
 * ESP8266_OK
 * ESP8266_TIMEOUT
 * ESP8266_WRONGPASS
 * ESP8266_NOTARGET
 * ESP8266_FAILED
 */
u8 esp8266_connect(u8 uart, u8* ssid, u8* pass)
{
    // Decprecated
    //esp8266_print(uart, (const char *)"AT+CWJAP=\"");
    esp8266_print(uart, (const char *)"AT+CWJAP_CUR=\"");
    esp8266_print(uart, ssid);
    esp8266_print(uart, (const char *)"\",\"");
    esp8266_print(uart, pass);
    esp8266_print(uart, (const char *)"\"\r\n");
    return esp8266_waitResponse(uart);
}

/**
 * Lists available APs.
 * 
 * @param list pointer on SSID string tab : list[][]
 * @param max SSID to list, 0 = all
 */
/*
void esp8266_list(u8 uart, u8* list, u8 max)
{
    u8 i, ssid_num=0;
    
    if (max == 0)
        max = 255;
        
    esp8266_print(uart, (const char *)"AT+CWLAP\r\n");
    while (esp8266_getch(uart) != '+' & ssid < max)
    {
        i = 0;
        ssid_num++;
        // wait for the first " to be reached
        while (esp8266_getch(uart) != '\"');
        // get the ssid name while the last " has not been reached
        while (esp8266_getch(uart) != '\"')
            list[ssid_num][i++] = esp8266_getch(uart);
        // 0-terminated string
        list[ssid][i] = '\0';
    }
}
*/

/**
 * Disconnect from the access point.
 *
 * This sends the AT+CWQAP command to the ESP module.
 */
void esp8266_disconnect(u8 uart)
{
    esp8266_print(uart, (const char *)"AT+CWQAP\r\n");
    esp8266_waitFor(uart, (u8*)"OK");
}

/**
 * Function to configure a Software Enabled Access Point
 * SoftAP is an abbreviated term for "software enabled access point".
 * This is software enabling a computer which hasn't been specifically
 * made to be a router into a wireless access point.
 * It is often used interchangeably with the term "virtual router".
 * 
 * @param softssid   the SSID name, ex PINGUINO
 * @param softpass   the password to be connected to the SSID
 * @param encryption WEP is not supported
 * @param hidden     SSID braodcasted (0) or not (1)
 */
u8 esp8266_create(u8 uart, u8* softssid, u8* softpass, u8 encryption, u8 hidden)
{
    // Deprecated
    //esp8266_print(uart, (const char *)"AT+CWSAP=\"");
    esp8266_print(uart, (const char *)"AT+CWSAP_CUR=\"");
    esp8266_print(uart, softssid);
    esp8266_print(uart, (const char *)"\",\"");
    esp8266_print(uart, softpass);
    esp8266_print(uart, (const char *)"\",5,");       // channel = 5
    esp8266_putch(uart, encryption + '0');
    esp8266_putch(uart, ',');
    esp8266_putch(uart, hidden + '0');
    esp8266_print(uart, (const char *)"\r\n");
    return esp8266_waitResponse(uart);
}

/**
 * Enables/Disables DHCP.
 * 
 * @param dhcp, either :
 * ESP8266_DISABLED
 * ESP8266_ENABLED
 */
u8 esp8266_dhcp(u8 uart, u8 dhcp)
{
    esp8266_print(uart, (const char *)"AT+DHCP_CUR=\"");
    if (dhcp == ESP8266_ENABLED)
        esp8266_print(uart, (const char *)"1,1");
    else
        esp8266_print(uart, (const char *)"0,0");
    return esp8266_waitResponse(uart);
}

/**
 * Connects to the AP automatically on power-up.
 * 
 * @param enable, either :
 * ESP8266_DISABLED
 * ESP8266_ENABLED
 */
u8 esp8266_autoconnect(u8 uart, u8 enable)
{
    esp8266_print(uart, (const char *)"AT+CWAUTOCONN=\"");
    esp8266_putch(uart, enable + '0');
    esp8266_print(uart, (const char *)"\r\n");
    return esp8266_waitResponse(uart);
}

/**
 * Sets the MAC address of ESP Station (AT+CIPSTAMAC).
 * Sets the MAC address of ESP SoftAP (AT+CIPAPMAC).
 * Must be called after esp8266_mode(mode) where mode is
 * ESP8266_STATION : Station mode (client)
 * ESP8266_SOFTAP  : Access Point mode (host)
 * ESP8266_STATION|ESP8266_SOFTAP (dual)
 */
u8 esp8266_setMAC(u8 uart, u8* mac)
{
    if (gESP8266MODE & ESP8266_STATION)
        esp8266_print(uart, (const char *)"AT+CIPSTAMAC=\"");
    else
        esp8266_print(uart, (const char *)"AT+CIPAPMAC=\"");
    esp8266_print(uart, mac);
    esp8266_print(uart, (const char *)"\"\r\n");
    return esp8266_waitResponse(uart);
}

/**
 * Sets the IP address of ESP Station (AT+CIPSTA).
 * Sets the IP address of ESP SoftAP (AT+CIPAP).
 * Must be called after esp8266_mode(mode) where mode is
 * ESP8266_STATION : Station mode (client)
 * ESP8266_SOFTAP  : Access Point mode (host)
 * ESP8266_STATION|ESP8266_SOFTAP (dual)
 */
u8 esp8266_setIP(u8 uart, u8* ip)
{
    if (gESP8266MODE & ESP8266_STATION)
        esp8266_print(uart, (const char *)"AT+CIPSTA=\"");
    else
        esp8266_print(uart, (const char *)"AT+CIPAP=\"");
    esp8266_print(uart, ip);
    esp8266_print(uart, (const char *)"\"\r\n");
    return esp8266_waitResponse(uart);
}

/***********************************************************************
 *                                                                     *
 *                    TCP/IP-Related functions                         *
 *                                                                     *
 **********************************************************************/
/* AT+CIPSTATUS        : Gets the connection status.
 * AT+CIPDOMAIN        : DNS function.
 **AT+CIPSTART         : Establishes TCP connection, UDP transmission or SSL connection.
 **AT+CIPSEND          : Sends data.
 * AT+CIPSENDEX        : Sends data when length of data is <length>, or when \0 appears in the data.
 **AT+CIPCLOSE         : Closes TCP/UDP/SSL connection.
 **AT+CIFSR            : Gets the local IP address.
 **AT+CIPMUX           : Configures the multiple connections mode.
 **AT+CIPSERVER        : Deletes/Creates TCP or SSL server.
 **AT+CIPSERVERMAXCONN : Set the Maximum Connections Allowed by Server.
 * AT+CIPMODE          : Configures the transmission mode (normal or UART-WIFI).
 * AT+SAVETRANSLINK    : Saves the transparent transmission link in flash.
 **AT+CIPSTO           : Sets timeout when ESP runs as a TCP server.
 * AT+CIPSNTPCFG       : Configures the time domain and SNTP server.
 * AT+CIPSNTPTIME      : Queries the SNTP time.
 * AT+CIUPDATE         : Updates the software through Wi-Fi.
 * AT+CIPDINFO         : Shows remote IP and remote port with +IPD.
 **********************************************************************/

/**
 * Open a TCP or UDP connection.
 *
 * TCP/IP is a suite of protocols used by devices to communicate over
 * the Internet and most local networks.
 * It is named after two of it’s original protocols the Transmission
 * Control Protocol (TCP) and the Internet Protocol (IP).
 * TCP provides apps a way to deliver (and receive) an ordered and
 * error-checked stream of information packets over the network.
 * The User Datagram Protocol (UDP) is used by apps to deliver a faster
 * stream of information by doing away with error-checking.
 * 
 * This function sends the AT+CIPSTART command to the ESP module.
 *
 * @param protocol Either ESP8266_TCP or ESP8266_UDP
 * @param ip The IP or hostname to connect to; as a string
 * @param port The port to connect to
 *
 * @return true if the connection is opened after this.
 */
u8 esp8266_serverStart(u8 uart, u8 protocol, u8* ip, u8 port)
{
    esp8266_print(uart, (const char *)"AT+CIPSTART=\"");
    if (protocol == ESP8266_TCP)
        esp8266_print(uart, (const char *)"TCP");
    else
        esp8266_print(uart, (const char *)"UDP");
    esp8266_print(uart, (const char *)"\",\"");
    esp8266_print(uart, ip);
    esp8266_print(uart, (const char *)"\",");
    esp8266_printNumber(uart, port, DEC);
    esp8266_print(uart, (const char *)"\r\n");
    if (esp8266_waitResponse(uart) != ESP8266_OK)
        return 0;
    if (esp8266_waitResponse(uart) != ESP8266_LINKED)
        return 0;
    return 1;
}

/**
 * Send data over a connection.
 *
 * This sends the AT+CIPSEND command to the ESP module.
 *
 * @param data The data to send
 *
 * @return true if the data was sent correctly.
 */
u8 esp8266_serverSend(u8 uart, u8* data)
{
    esp8266_print(uart, (const char *)"AT+CIPSEND=");
    esp8266_printNumber(uart, strlen(data), DEC);
    esp8266_print(uart, (const char *)"\r\n");
    while (esp8266_getch(uart) != '>');
        esp8266_print(uart, data);
    if (esp8266_waitResponse(uart) == ESP8266_OK)
        return 1;
    return 0;
}

/**
 * Close TCP/UDP connection(s).
 * Close all connections in multi-connections environment.
 *
 * @return true if the data was sent correctly.
 */
u8 esp8266_serverClose(u8 uart)
{
    esp8266_print(uart, (const char *)"AT+CIPCLOSE=5\r\n");
    return esp8266_waitResponse(uart);
}

/**
 * Store the current local IPv4 address.
 *
 * This sends the AT+CIFSR command to the ESP module.
 *
 * The result will not be stored as a string but byte by byte. For example, for
 * the IP 192.168.0.1, the value of ip will be: {0xc0, 0xa8, 0x00, 0x01}.
 *
 * @param ip a pointer to an array of the type u8[4]; this
 * array will be filled with the local IP.
 */
void esp8266_serverGetLocalIP(u8 uart, u8* ip)
{
    u8 i, received;

    esp8266_print(uart, (const char *)"AT+CIFSR\r\n");
    do
    {
        received = esp8266_getch(uart);
    }
    while (received < '0' || received > '9');
    for (i = 0; i < 4; i++)
    {
        ip[i] = 0;
        do
        {
            ip[i] = 10 * ip[i] + received - '0';
            received = esp8266_getch(uart);
        }
        while (received >= '0' && received <= '9');
        received = esp8266_getch(uart);
    }
    esp8266_waitFor(uart, (u8*)"OK");
}

// Function to enable/disable multiple connections
u8 esp8266_serverMux(u8 uart, u8 bool)
{
    esp8266_print(uart, (const char *)"AT+CIPMUX=");
    esp8266_putch(uart, bool + '0');
    esp8266_print(uart, (const char *)"\r\n");
    return esp8266_waitResponse(uart);
}

/**
 * Function to create a server
 * A TCP server can only be created when
 * multiple connections are activated (AT+CIPMUX=1).
 * 
 * @param port number
 */
u8 esp8266_serverCreate(u8 uart, u16 port)
{
    esp8266_print(uart, (const char *)"AT+CIPMUX=1");
    esp8266_print(uart, (const char *)"AT+CIPSERVER=1,");
    esp8266_printNumber(uart, port, DEC);
    esp8266_print(uart, (const char *)"\r\n");
    return esp8266_waitResponse(uart); 
}

// Function to create server on Port 80
u8 esp8266_serverDelete(u8 uart, u16 port)
{
    esp8266_print(uart, (const char *)"AT+CIPSERVER=0,");
    esp8266_printNumber(uart, port, DEC);
    esp8266_print(uart, (const char *)"\r\n");
    return esp8266_waitResponse(uart); 
}

// Set the maximum number of clients allowed to connect to the TCP server.
u8 esp8266_serverMaxConnections(u8 uart, u8 num)
{
    esp8266_print(uart, (const char *)"AT+CIPSERVERMAXCONN=");
    esp8266_printNumber(uart, num, DEC);
    esp8266_print(uart, (const char *)"\r\n");
    return esp8266_waitResponse(uart); 
}

/**
 * Set TCP server timeout
 * 
 * @param timeout [0, 7200] sec.
 */
u8 esp8266_serverTimeout(u8 uart, u16 timeout)
{
    esp8266_print(uart, (const char *)"AT+CIPSTO=");
    esp8266_printNumber(uart, timeout, DEC);
    esp8266_print(uart, (const char *)"\r\n");
    return esp8266_waitResponse(uart); 
}

/**
 * Read a string of data that is sent to the ESP8266.
 *
 * This waits for a +IPD line from the module. If more bytes than the maximum
 * are received, the remaining bytes will be discarded.
 *
 * @param buffer a pointer to a u8acter array to store the data in
 * @param max_length maximum amount of bytes to read in
 * @param discard_headers if set to true, we will skip until the first \r\n\r\n,
 * for HTTP this means skipping the headers.
 */
void esp8266_serverReceive(u8 uart, u8* buffer, u16 max_length, u8 discard_headers)
{
    u16 i, length = 0;
    u8 received;
    
    esp8266_waitFor(uart, (u8*)"+IPD,");

    received = esp8266_getch(uart);
    do
    {
        length = length * 10 + received - '0';
        received = esp8266_getch(uart);
    }
    while (received >= '0' && received <= '9');

    if (discard_headers)
        length -= esp8266_waitFor(uart, (u8*)"\r\n\r\n");

    if (length < max_length)
        max_length = length;

    /*sprintf(buffer, "%u,%u:%c%c", length, max_length, esp8266_getch(), esp8266_getch());
    return;*/

    for (i = 0; i < max_length; i++)
        buffer[i] = esp8266_getch(uart);
    buffer[i] = 0;
    for (; i < length; i++)
        esp8266_getch(uart);
    esp8266_waitFor(uart, (u8*)"OK");
}

// Function to get the Station IP and MAC
// to which the ESP SoftAP is connected.
void esp8266_serverGetIPandMAC(u8 uart, u8 *ip, u8 *mac)
{
    u8 rex;
    esp8266_print(uart, (const char *)"AT+CWLIF\r\n");

    // get IP
    do
    {
        rex = esp8266_getch(uart) ;
        *ip++ = rex;
    }
    while(rex != ',');

    // get MAC
    do
    {
        rex = esp8266_getch(uart);
        *mac++ = rex;
    }
    while(rex!='O');
}
 
// Function to get the IP
void esp8266_serverGetIP(u8 uart, u8 *ip)
{
    u8 tmp[];
    esp8266_serverGetIPandMAC(uart, ip, tmp);
}

// Function to get the MAC
void esp8266_serverGetMAC(u8 uart, u8 *mac)
{
    u8 tmp[];
    esp8266_serverGetIPandMAC(uart, tmp, mac);
}

/***********************************************************************
 *                                                                     *
 *                      Mail-Related functions                         *
 *                                                                     *
 **********************************************************************/

// Connect to SMPT2GO server
u8 esp8266_mailConnectSMPT2GO(u8 uart)
{
    esp8266_print(uart, (const char *)"AT+CIPSTART=4,\"TCP\",\"mail.smtp2go.com\",2525\r\n");
    esp8266_waitResponse(uart);
    esp8266_print(uart, (const char *)"AT+CIPSEND=4,20\r\n");
    esp8266_waitResponse(uart);
    esp8266_print(uart, (const char *)"EHLO 192.168.1.123\r\n");
    esp8266_waitResponse(uart);
    esp8266_print(uart, (const char *)"AT+CIPSEND=4,12\r\n");
    esp8266_waitResponse(uart);
    esp8266_print(uart, (const char *)"AUTH LOGIN\r\n");
    return esp8266_waitResponse(uart);
}

// Quit Connection from SMPT server
u8 esp8266_mailDisconnectSMPT2GO(u8 uart)
{
    esp8266_print(uart, (const char *)"AT+CIPSEND=4,6\r\n");
    esp8266_waitResponse(uart);
    esp8266_print(uart, (const char *)"QUIT\r\n");
    return esp8266_waitResponse(uart);
}

// Enter into Start typing the mail
u8 esp8266_mailStart(u8 uart)
{
    esp8266_print(uart, (const char *)"AT+CIPSEND=4,6\r\n");
    esp8266_waitResponse(uart);
    esp8266_print(uart, (const char *)"DATA\r\n");
    return esp8266_waitResponse(uart);
}

// End the Mail using a "."
u8 esp8266_mailEnd(u8 uart)
{
    esp8266_print(uart, (const char *)"AT+CIPSEND=4,3\r\n");
    esp8266_waitResponse(uart);
    esp8266_print(uart, (const char *)".\r\n");
    return esp8266_waitResponse(uart);
}
 
/**
 * LOG IN with your SMPT2GO approved mail ID
 * Visit the page https://www.smtp2go.com/ and sign up using any Gmail ID
 * Once you gmail ID is SMPT2GO approved convert your mail ID and password in 64 base format
 * visit https://www.base64encode.org/ for converting 64 base format online
 * FORMAT -> esp8266_login_mail("mailID in base 64","Password in base 64");
 * This program uses the ID-> aswinthcd@gmail.com and password -> circuitdigest as an example
 */
u8 esp8266_mailLogin(u8 uart, u8* mail_ID, u8* mail_Pas)
{
    u8 len = strlen(mail_ID) + 2;
    u8 l2 = len%10;
    u8 l1 = (len/10)%10;

    esp8266_print(uart, (const char *)"AT+CIPSEND=4,");
    if ((l1+'0')>'0')
        esp8266_putch(uart, l1+'0');
    esp8266_putch(uart, l2+'0');
    esp8266_print(uart, (const char *)"\r\n");
    esp8266_waitResponse(uart);

    esp8266_print(uart, mail_ID);
    esp8266_print(uart, (const char *)"\r\n");
    esp8266_waitResponse(uart);

    len = strlen(mail_Pas) + 2;
    l2 = len%10;
    l1 = (len/10)%10;

    esp8266_print(uart, (const char *)"AT+CIPSEND=4,");
    if ((l1+'0')>'0')
        esp8266_putch(uart, l1+'0');
    esp8266_putch(uart, l2+'0');
    esp8266_print(uart, (const char *)"\r\n");
    esp8266_waitResponse(uart);

    esp8266_print(uart, mail_Pas);
    esp8266_print(uart, (const char *)"\r\n");
    return esp8266_waitResponse(uart);
}
 
u8 esp8266_mailSendID(u8 uart, u8* send_ID)
{
    u8 len = strlen(send_ID) + 14;
    u8 l2 = len%10;
    u8 l1 = (len/10)%10;

    esp8266_print(uart, (const char *)"AT+CIPSEND=4,");
    if ((l1+'0')>'0')
        esp8266_putch(uart, l1+'0');
    esp8266_putch(uart, l2+'0');
    esp8266_print(uart, (const char *)"\r\n");
    esp8266_waitResponse(uart);

    esp8266_print(uart, (const char *)"MAIL FROM:<");
    esp8266_print(uart, send_ID);
    esp8266_print(uart, (const char *)">\r\n");
    return esp8266_waitResponse(uart);
} 
 
u8 esp8266_mailRecID(u8 uart, u8* rec_ID)
{
    u8 len = strlen(rec_ID) + 12;
    u8 l2 = len%10;
    u8 l1 = (len/10)%10;

    esp8266_print(uart, (const char *)"AT+CIPSEND=4,");
    if ((l1+'0')>'0')
        esp8266_putch(uart, l1+'0');
    esp8266_putch(uart, l2+'0');
    esp8266_print(uart, (const char *)"\r\n");
    esp8266_waitResponse(uart);

    esp8266_print(uart, (const char *)"RCPT To:<");
    esp8266_print(uart, rec_ID);
    esp8266_print(uart, (const char *)">\r\n");
    return esp8266_waitResponse(uart);
} 
  
u8 esp8266_mailSubject(u8 uart, u8* subject)
{
    u8 len = strlen(subject) + 10;
    u8 l2 = len%10;
    u8 l1 = (len/10)%10;

    esp8266_print(uart, (const char *)"AT+CIPSEND=4,");
    if ((l1+'0')>'0')
        esp8266_putch(uart, l1+'0');
    esp8266_putch(uart, l2+'0');
    esp8266_print(uart, (const char *)"\r\n");
    esp8266_waitResponse(uart);

    esp8266_print(uart, (const char *)"Subject:");
    esp8266_print(uart, subject);
    esp8266_print(uart, (const char *)"\r\n");
    return esp8266_waitResponse(uart);
} 
    
u8 esp8266_mailBody(u8 uart, u8* body)
{
    u8 len = strlen(body) + 2;
    u8 l2 = len%10;
    u8 l1 = (len/10)%10;

    esp8266_print(uart, (const char *)"AT+CIPSEND=4,");
    if ((l1+'0')>'0')
        esp8266_putch(uart, l1+'0');
    esp8266_putch(uart, l2+'0');
    esp8266_print(uart, (const char *)"\r\n");
    esp8266_waitResponse(uart);

    esp8266_print(uart, body);
    esp8266_print(uart, (const char *)"\r\n");
    return esp8266_waitResponse(uart);   
} 
