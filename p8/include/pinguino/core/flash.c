/*	--------------------------------------------------------------------
    FILE:			flash.c
    PROJECT:		pinguino
    PURPOSE:		flash memory functions
    PROGRAMER:		jean-pierre mandon <jpmandon@gmail.com>
                    rblanchot@gmail.com
    --------------------------------------------------------------------
    CHANGELOG:
      -  -2008 - jp mandon - first release
      -  -2010 - rblanchot - fixed for Pinguino IDE v9
    08-09-2015 - rblanchot - added PINGUINO1459 support
    --------------------------------------------------------------------
    TODO :
    * fix 16F and 18F syntax
    * fix 16F enable/disable interrupt
    * fix erase block size (can be 1024- or 64-byte long) ?
    * fix write block minimal size (can be 2- or 32-bytes long)
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
    --------------------------------------------------------------------------*/

#ifndef __FLASH_C__
#define __FLASH_C__

#include <compiler.h>
#include <typedef.h>
//#include <system.c>

#if defined(__16F1459)
    #define ERASE   PMCON1bits.FREE
#else
    #define ERASE   EECON1bits.FREE
#endif

/**------------------------------------------------------------------**/
#if defined(_PIC14E) //__16F1459 || __16F1708
/**------------------------------------------------------------------**/

u16 Flash_read(u16 address)
{
    // 1. Write the desired address to the PMADRH:PMADRL register pair.
    //PMADRH = address >> 8;
    //PMADRL = address;
    PMADR = address;
    // 2. Clear or set the CFGS bit of the PMCON1 register.
    PMCON1bits.CFGS = (address & 0x8000) ? 1:0;
    // 3. Then, set control bit RD of the PMCON1 register.
    PMCON1bits.RD = 1;
    // 4. The two instructions following a program memory read are required to be NOPs
    __asm__("NOP");
    __asm__("NOP");
    
    //return ((PMDATH << 8) + PMDATL);
    return PMDAT;
}

void Flash_erase(u16 address)
{
    PMADR = address;
    PMCON1bits.CFGS = (address & 0x8000) ? 1:0;
    PMCON1bits.FREE = 1;
    PMCON1bits.WREN = 1;
    PMCON2 = 0x55;
    PMCON2 = 0xAA;
    PMCON1bits.WR = 1;
    __asm__("NOP");
    __asm__("NOP");
    PMCON1bits.WREN = 0;
}

void Flash_write(u16 address, u16 *pdata, u8 counter)
{
    PMADR = address;
    PMCON1bits.CFGS = (address & 0x8000) ? 1:0;
    PMCON1bits.FREE = 0;
    PMCON1bits.LWLO = 1;
    PMCON1bits.WREN = 1;

    while (counter-- > 1)       // until we reach the last word
    {
        PMDAT = *pdata++;       // next word to load
        PMCON2 = 0x55;
        PMCON2 = 0xAA;
        PMCON1bits.WR = 1;
        __asm__("NOP");
        __asm__("NOP");
        PMADR++;                // next address
    }

    PMDAT = *pdata++;            // next word to load
    PMCON1bits.LWLO = 0;        // Write Latches to Flash
    PMCON2 = 0x55;
    PMCON2 = 0xAA;
    PMCON1bits.WR = 1;
    __asm__("NOP");
    __asm__("NOP");
    PMCON1bits.WREN = 0;
}

/**------------------------------------------------------------------**/
#else
/**------------------------------------------------------------------**/

u16 Flash_read(u32 address)
{
    u8 h8,l8;

    TBLPTRU = address >> 16;
    TBLPTRH = address >> 8;
    TBLPTRL = address;

    __asm__("tblrd*+");
    l8 = TABLAT;
    __asm__("tblrd*+");
    h8 = TABLAT;
    
    return ((h8 << 8) + l8);
}

void Flash_erase(u32 address)
{
    u8 status = INTCONbits.GIE;
    
    TBLPTRU = address >> 16;
    TBLPTRH = address >> 8;
    TBLPTRL = address;

    #if !defined(__18f26j50) && !defined(__18f46j50) && \
        !defined(__18f26j53) && !defined(__18f46j53) && \
        !defined(__18f27j53) && !defined(__18f47j53)
    EECON1bits.EEPGD = 1; // Program Memory
    EECON1bits.CFGS = 0;  // but not the config space
    #endif
    EECON1bits.WREN = 1;  // Write enabled
    EECON1bits.FREE = 1;  // Erase operation

    if (status)           // Disabled interrupts
        INTCONbits.GIE = 0;
    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1bits.WR = 1;
    if (status)           // Re-enabled interrupts
        INTCONbits.GIE = 1;
}

void Flash_write(u16 destination_add, u8 *destination)
{
    u8 i;
    u8 status = INTCONbits.GIE;

    TBLPTRU=0;
    TBLPTRH=destination_add>>8;
    TBLPTRL=destination_add;

     __asm__("tblrd*-");
        
    for(i = 0; i < 32; i++)
    {
        TABLAT = *destination;
        destination++;
        __asm__("tblwt+*");
    } 

    #if !defined(__18f26j50) && !defined(__18f46j50) && \
        !defined(__18f26j53) && !defined(__18f46j53) && \
        !defined(__18f27j53) && !defined(__18f47j53)
    EECON1bits.EEPGD = 1; // Program Memory
    EECON1bits.CFGS = 0;  // but not the config space
    #endif
    EECON1bits.WREN = 1;  // Write enabled
    EECON1bits.FREE = 0;  // Write operation

    if (status)           // Disabled interrupts
        INTCONbits.GIE = 0;
    EECON2 = 0x55;
    EECON2 = 0xAA;
    EECON1bits.WR = 1;
    if (status)           // Re-enabled interrupts
        INTCONbits.GIE = 1;
}

#endif // _PIC14E

#endif // __FLASH_C__
