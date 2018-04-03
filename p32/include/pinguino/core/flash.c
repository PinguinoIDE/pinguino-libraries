/***********************************************************************
    Title:	Pinguino Flash Memory Operations
    File:	flash.c
    Descr.: flash operations for supported PIC32MX
    Author:	Régis Blanchot <rblanchot@gmail.com>

    To do/change :
    * u32 FlashRead(u32 address)
    * User has to init serial bus

    This file is part of Pinguino (http://www.pinguino.cc)
    Released under the LGPL license (http://www.gnu.org/licenses/lgpl.html)
***********************************************************************/

#ifndef __FLASH_C
#define __FLASH_C

#include <p32xxxx.h>
#include <typedefs.h>
#include <flash.h>              // ConvertToPhysicalAddress
#include <delay.c>              // Delayus
#include <mips.h>               // EnableInterrupt(), ...

#if (_DEBUG_ENABLE_)
#define SERIALPRINTX            // Enable SerialPrintX function
#define SERIALPORT UART1        // Output is on UART1
#include <serial.c>             // Serial functions
#endif

/***********************************************************************
 * Performs flash Write/Erase operation
 * This function must generate MIPS32 code only
 **********************************************************************/

u8 FlashOperation(u8 op)
{
    u8 res;
    u32 status;

    // Suspend or Disable all Interrupts
    status = DisableInterrupt();

    // 1-Select Flash operation to perform
    // Enable writes to WR bit and LVD circuit
    NVMCON = _NVMCON_WREN_MASK | op;

    // 2-Wait for LVD to become stable (at least 6us).
    Delayus(7);
    
    // 3-Write unlock sequence before the WR bit is set
    NVMKEY = 0xAA996655;
    NVMKEY = 0x556699AA;

    // 4-Start the operation (WR=1)
    // Must be an atomic instruction
    NVMCONSET = _NVMCON_WR_MASK;
    //NVMCON |= _NVMCON_WR_MASK;
    //NVMCONbits.WR = 1;

    // 5-Wait for operation to complete (WR=0)
    while (NVMCON & _NVMCON_WR_MASK);
    //while (NVMCONbits.WR);

    // 6-Disable Flash Write/Erase operations
    NVMCONCLR = _NVMCON_WREN_MASK;
    //NVMCONbits.WREN = 0;

    // Restore Interrupts if necessary
    if (status & 1)
        EnableInterrupt();
    else
        DisableInterrupt();

    res = FlashError();
    
    #if (_DEBUG_ENABLE_)
    if (res)
        SerialPrint(SERIALPORT, "Error\r\n");
    #endif
    
    return res;
}

/***********************************************************************
 * Erase a single page of program flash,
 * The page to be erased is selected using NVMADDR.
 * Returns '0' if operation completed successfully.
 **********************************************************************/

u8 FlashErasePage(void* address)
{
    u8 res;

    // Convert Address to Physical Address
    NVMADDR = ConvertToPhysicalAddress(address);

    #if (_DEBUG_ENABLE_)
        SerialPrintX(SERIALPORT, "Erase page at 0x", NVMADDR, 16);
    #endif

    // Unlock and Erase Page
    res = FlashOperation(FLASH_PAGE_ERASE);

    // Clears the NVMCON error flag if necessary
    if (res)
        FlashClearError()

    // Return WRERR state.
    return res;
}

/***********************************************************************
 * Writes a word (4 Bytes) pointed to by NVMADDR.
 * Returns '0' if operation completed successfully.
 **********************************************************************/

u8 FlashWriteWord(void* address, u32 data)
{
    u8 res;

    NVMADDR = ConvertToPhysicalAddress(address);

    // Load data into NVMDATA register
    NVMDATA = data;

    #if (_DEBUG_ENABLE_)
        SerialPrintX(SERIALPORT, "Address 0x", NVMADDR, 16);
        SerialPrintX(SERIALPORT, "Data    0x", NVMDATA, 16);
    #endif

    // Unlock and Write Word
    res = FlashOperation(FLASH_WORD_WRITE);

    // Clears the NVMCON error flag if necessary
    if (res)
        FlashClearError()

    return res;
}

/***********************************************************************
 * Writes a block of data (1 row is 128 instructions = 512 Bytes).
 * The row at the location pointed to by NVMADDR is programmed with
 * the data buffer pointed to by NVMSRCADDR.
 * Returns '0' if operation completed successfully.
 **********************************************************************/

u8 FlashWriteRow(void* address, void* data)
{
    u32 res;

    // Set NVMADDR to Address of row t program
    NVMADDR = ConvertToPhysicalAddress(address);

    // Set NVMSRCADDR to the SRAM data buffer Address
    NVMSRCADDR = ConvertToPhysicalAddress(data);

    // Unlock and Write Row
    res = FlashOperation(FLASH_ROW_WRITE);

    // Clears the NVMCON error flag if necessary
    if (res)
        FlashClearError()

    return res;
}

/***********************************************************************
 * Reads a word of data at the location pointed.
 * Flash memory is readable as any other memory.
 * We simply set a pointer to the desired address and read away.
 * No special code required.
 **********************************************************************/

u32 FlashRead(void* address)
{
    u32 addr = ConvertToPhysicalAddress(address);
    u32 data = *(u32*)(addr);

    return data;
}

#endif // __FLASH_C
