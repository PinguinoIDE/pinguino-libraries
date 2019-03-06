/**
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
 *****************************************************************************
 *
 * File:    esp8266.h
 * Author:  Camil Staps <info@camilstaps.nl>
 * Website: http://github.com/camilstaps/ESP8266_PIC
 * Version: 0.1
 *
 * See:     esp8266.c
 *
 * This is the header file for the ESP8266 PIC18 library. It contains:
 *
 *  * Constants and u8masks
 *  * Prototypes for functions that should be written by the user for his
 *    implementation
 *  * Prototypes for functions that can be used by the user
 *  * Prototypes for functions that are intended for internal use only
 */

#ifndef ESP8266_H
#define	ESP8266_H

#include <typedef.h>
//#include <stdint.h>
//#include <stdio.h>
//#include <string.h>

/***********************************************************************
 * Some constants
 **********************************************************************/

#define ESP8266_STATION     0x01
#define ESP8266_SOFTAP      0x02

#define ESP8266_TCP         1
#define ESP8266_UDP         0

#define ESP8266_OK          1
#define ESP8266_READY       2
#define ESP8266_FAIL        3
#define ESP8266_NOCHANGE    4
#define ESP8266_LINKED      5
#define ESP8266_UNLINK      6

#define ESP8266_TIMEOUT     1 // connection timeout.
#define ESP8266_WRONGPASS   2 // wrong password.
#define ESP8266_NOTARGET    3 // cannot find the target AP.
#define ESP8266_FAILED      4 // connection failed.

#define ESP8266_OPEN        0
#define ESP8266_WPAPSK      1
#define ESP8266_WPA2PSK     2
#define ESP8266_WPAWPA2PSK  3

#define ESP8266_DISABLED    0
#define ESP8266_ENABLED     1

#define ESP8266_NOSLEEP     0
#define ESP8266_LIGHTSLEEP  1
#define ESP8266_MODEMSLEEP  2

/***********************************************************************
 *                        UART-Related functions                       *
 **********************************************************************/
//void esp8266_putch(u8);                   // Print a byte to the output
//u8 esp8266_getch(u8);                     // Get a byte from the output
//void esp8266_print(u8, const u8*);        // Print a string to the output
//void esp8266_printNumber(u8, s32, u8);    // Print a number to the output
u8 esp8266_init(u8 uart);                   // Starts the ESP and check if the module is started (AT)
/***********************************************************************
 *                          Basic functions                            *
 **********************************************************************/
u8 esp8266_update(u8);                      // Update ESP software
u16 esp8266_waitFor(u8, u8*);               // Wait for a certain string on the input
u8 esp8266_waitResponse(u8);                // Wait for any response on the input
u8 esp8266_isStarted(u8);                   // Check if the module is started (AT)
u8 esp8266_restart(u8);                     // Restart module (AT+RST)
u8 esp8266_sleep(u8, u8);                   // Sleep mode
void esp8266_echo(u8, u8);                  // Enabled/disable command echoing (ATE)
/***********************************************************************
 *                          Wi-Fi functions                            *
 **********************************************************************/
u8 esp8266_mode(u8, u8);                    // AT+CWMODE : WIFI Mode (station/softAP/station+softAP)
u8 esp8266_connect(u8, u8*, u8*);           // AT+CWJAP : Connect to AP
void esp8266_list(u8, u8*, u8);             // AT+CWLAP: List available APs
void esp8266_disconnect(u8);                // AT+CWQAP : Disconnect from AP
u8 esp8266_create(u8, u8*, u8*, u8, u8);    // AT+CWSAP : Configure a Software Enabled Access Point
u8 esp8266_dhcp(u8, u8);                    // AT+CWDHCP : Enables/Disables DHCP
u8 esp8266_autoconnect(u8, u8);             // AT+CWAUTOCONN : Connects to the AP automatically on power-up)
u8 esp8266_setMAC(u8, u8*);                 // AT+CIPSTAMAC : Sets the MAC address of ESP32 Station.
                                            // AT+CIPAPMAC : Sets the MAC address of ESP32 SoftAP.
u8 esp8266_setIP(u8, u8*);                  // AT+CIPSTA : Sets the IP address of ESP32 Station.
                                            // AT+CIPAP : Sets the IP address of ESP32 SoftAP.
/***********************************************************************
 *                    TCP/IP-Related functions                         *
 **********************************************************************/
u8 esp8266_serverStart(u8, u8, u8*, u8);    // Create connection (AT+CIPSTART)
u8 esp8266_serverSend(u8, u8*);             // Send data (AT+CIPSEND)
u8 esp8266_serverClose(u8);                 // Close TCP/UDP connection(s).
void esp8266_serverGetLocalIP(u8, u8*);     // Get local IP (AT+CIFSR)
u8 esp8266_serverMux(u8, u8);               // Enable/disable multiple connections
u8 esp8266_serverCreate(u8, u16);           // Create a TCP server
u8 esp8266_serverDelete(u8, u16);           // Delete a TCP server
u8 esp8266_serverMaxConnections(u8, u8);    // Set the maximum number of clients allowed to connect to the TCP server.
u8 esp8266_serverTimeout(u8, u16);          // Set TCP server timeout in sec.
void esp8266_serverReceive(u8,u8*,u16,u8);  // Receive data (+IPD)
void esp8266_serverGetIPandMAC(u8,u8*,u8*); // Get IP and MAC
void esp8266_serverGetIP(u8, u8*);          // Get the IP
void esp8266_serverGetMAC(u8, u8*);         // Get the MAC
/***********************************************************************
 *                      Mail-Related functions                         *
 **********************************************************************/
u8 esp8266_mailConnectSMPT2GO(u8);          // Connect to SMPT2GO server
u8 esp8266_mailDisconnectSMPT2GO(u8);       // Quit Connection from SMPT server
u8 esp8266_mailStart(u8);                   // Enter into Start typing the mail
u8 esp8266_mailLogin(u8, u8*, u8*);
u8 esp8266_mailSendID(u8, u8*);
u8 esp8266_mailRecID(u8, u8*);
u8 esp8266_mailSubject(u8, u8*);
u8 esp8266_mailBody(u8, u8*);
u8 esp8266_mailEnd(u8);                     // End the Mail using a "."

#endif  /* ESP8266_H */
