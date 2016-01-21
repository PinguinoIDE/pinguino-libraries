/*  --------------------------------------------------------------------
    FILE:           diskio.c
    PROJECT:        Pinguino
    PURPOSE:        SD Card file system functions
    AUTHORS:        Regis Blanchot <rblanchot@gmail.com>
                    André Gentric <>
                    Alfred Broda <alfredbroda@gmail.com>
                    Mark Harper <markfh@f2s.com>
    FIRST RELEASE:  23 Dec. 2011
    LAST RELEASE:   20 Jan. 2016
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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
    MA  02111-1307  USA
    ------------------------------------------------------------------*/

#ifndef _DISKIO_C
#define _DISKIO_C

#include <compiler.h>
#include <typedef.h>
#include <spi.h>                // Pinguino SPI lib.
#include <spi.c>
#include <sd/ffconf.h>          // SD lib. config. file

#if _FS_TINY
#include <sd/tff.c>             // Tiny Fat Filesystem
#else
#include <sd/pff.c>             // Petit Fat Filesystem
#endif

#include <sd/diskio.h>
#include <oscillator.c>

#ifdef SD_DEBUG
    #define ST7735PRINTF
    #include <ST7735.c>
    /*
    void debugf(const u8 *fmt, ...)
    {
        va_list args;
        va_start(args, format);
        ST7735_printf(SPI2, fmt, args);
        va_end(args);
    }
    */
    //#include <__cdc.c>
    //#include <serial.c>
#endif

//#include <delayms.c>
//#include <delayus.c>
//#include <digitalw.c>
//#include <millis.c>

// For boards known to support the RTCC library
#if defined(__18f26j50) || defined(__18f46j50) || \
    defined(__18f27j53) || defined(__18f47j53)
    #define RTCCGETTIMEDATE
    #include <rtcc.c>
    #include <rtcc1.c>
#endif

//static volatile u16 Timer1, Timer2; /* 1000Hz decrement timer */
static volatile DSTATUS Stat = STA_NOINIT; /* Disk status */
u8 type=0;

/*  --------------------------------------------------------------------
    Repeatedly reads the SD card until we get a valid response
    * u8 spi: SPI module (SPISW, SPI1, SPI2, ...)
    ------------------------------------------------------------------*/
    
u8 disk_getresponse(u8 spi)
{
    u8 res;
    u8 timeout = NCR_TIMEOUT;
    
    do
        res = SPI_read(spi);
    while ((res & 0x80) && --timeout);

    return res;
}

/*  --------------------------------------------------------------------
    Wait for card ready
    returns : 0xFF=OK, Other=Timeout
    ------------------------------------------------------------------*/

static u8 disk_ready(u8 spi)
{
    u8  res;
    u16 timeout = 500; //NCR_TIMEOUT;

    do
        res = SPI_read(spi);
    while (res != 0xFF && --timeout);

    return (res == 0xFF) ? 1 : 0;
}

/*  --------------------------------------------------------------------
    Select the card and wait until it's ready
    Returns 1 if OK, 0 if Timeout
    ------------------------------------------------------------------*/

static u8 disk_select(u8 spi)
{
    //SPI_deselect(spi);
    SPI_select(spi);
    SPI_write(spi, 0xFF);        // Dummy clock (force DO enabled)

    // OK
    if (disk_ready(spi))
        return 1;

    // Timeout
    SPI_deselect(spi);
    return 0;
}

static void disk_deselect(u8 spi)
{
    SPI_deselect(spi);
    SPI_write(spi, 0xFF);        // Dummy clock (force DO enabled)
}

/*  --------------------------------------------------------------------
    Send a command packet
    * u8 spi:   SPI module (SPISW, SPI1, SPI2, ...)
    * u8 cmd:   Command byte
    * u32 arg:  Argument
    * Return :  R1 byte
    bit 0 = Idle state
    bit 1 = Erase Reset
    bit 2 = Illegal command
    bit 3 = Communication CRC error
    bit 4 = Erase sequence error
    bit 5 = Address error
    bit 6 = Parameter error
    bit 7 = Always 0
    ------------------------------------------------------------------*/

u8 disk_sendcmd(u8 spi, u8 cmd, u32 arg)
{
    u8  crc, R1;
    //u32 timeout;

    // Select card
    disk_deselect(spi);
    if (!disk_select(spi))
        return 0xFF;
    
    // Send command (bit 6 set)
    SPI_write(spi, cmd | 0x40);

    // Send argument
    SPI_write(spi, (u8)(arg >> 24));    // Argument[31..24]
    SPI_write(spi, (u8)(arg >> 16));    // Argument[23..16]
    SPI_write(spi, (u8)(arg >>  8));    // Argument[15..8]
    SPI_write(spi, (u8)(arg      ));    // Argument[7..0]

    // Send CRC
    // The only commands that must have a valid CRC are GO_IDLE_STATE and SEND_IF_COND.
    // For all other commands the CRC is optional.
    // The SD card can be programmed to skip CRC checking allowing
    // simple microcontrollers to skip the CRC generation.
    // This library does not generate or check the CRC values.
    
    crc = 0x01;                         // Dummy CRC + Stop
    if (cmd == GO_IDLE_STATE)
        crc = 0x95;                     // Valid CRC for CMD0(0)
    if (cmd == SEND_IF_COND)
        crc = 0x87;                     // Valid CRC for CMD8(0x1AA)
    /*
    if (cmd == STOP_TRANSMISSION)
        crc = 0xC3;
    */
    SPI_write(spi, crc);

    // Receive command response (Skip a stuff byte when stop reading)
    if (cmd == STOP_TRANSMISSION)       // CMD12
        SPI_read(spi);

    // Wait for a valid response
    R1 = disk_getresponse(spi);

    /*
    if (cmd == STOP_TRANSMISSION)       // CMD12
    {
        timeout = WRITE_TIMEOUT;
        do
            R1 = SPI_read(spi);
        while ((R1 == 0x00) && (--timeout));
        R1 = 0x00;
    }
    
    SPI_write(spi, 0xFF);
    if ((cmd != CMD9)&&(cmd != CMD10)&&(cmd != CMD17)&&(cmd != CMD18)&&(cmd != CMD24)&&(cmd != CMD25))
        SPI_deselect(spi);
    */
    
    return R1;
}

/*  --------------------------------------------------------------------
    Send a command packet
    * u8 spi:   SPI module (SPISW, SPI1, SPI2, ...)
    * u8 cmd:   Command byte
    * u32 arg:  Argument
    The CS signal must be driven high to low prior to send a command
    frame and held it low during the transaction (command, response and
    data transfer if exist)
    returns R1 byte :
    00 - command accepted
    01 - command received
    FF - timeout
    other codes:
    bit 0 = Idle state
    bit 1 = Erase Reset
    bit 2 = Illegal command
    bit 3 = Communication CRC error
    bit 4 = Erase sequence error
    bit 5 = Address error
    bit 6 = Parameter error
    bit 7 = Always 0
    ------------------------------------------------------------------*/
    
u8 disk_sendcommand(u8 spi, u8 cmd, u32 arg)
{
    u8  R1, c;
    u32 timeout = IDLE_TIMEOUT;
    
    do
    {
        c = cmd;
        
        // if ACMD<n> send CMD55 first
        if (c & 0x80)
        {
            R1 = disk_sendcmd(spi, APP_CMD, 0);
            // return if CMD55 is not accepted
            if (R1 > 0x01)
            {
                #ifdef SD_DEBUG
                //ST7735_printf(SPI2, "ERROR CMD%d=%d\r\n", c, R1);
                #endif
                return R1;
            }
            // clearing bit 7 turns ACMD<n> to CMD<n>
            c &= 0x7F;
        }

        // send CMD<n>
        R1 = disk_sendcmd(spi, c, arg);
    }
    // When CMD1 or ACMD41 are accepted, the SD card will leave the IDLE_STATE.
    // We repeat sending the ACMD41 until the SD card reports it has left the IDLE_STATE
    // i.e. 0x00 is returned
    while ((cmd == CMD1 || cmd == ACMD41) && R1 != 0x00 && --timeout);

    #ifdef SD_DEBUG
    //ST7735_printf(SPI2, "CMD%d=%d\r\n", c, R1);
    #endif

    return R1;
}

/*  --------------------------------------------------------------------
    Initialize Disk Drive
    * u8 spi:   SPI module (SPISW, SPI1, SPI2, ...)
    * u8 drv : Physical drive number (0)
    returns disk status : STA_NODISK, STA_NOINIT or OK (0) 
    ------------------------------------------------------------------*/

DSTATUS disk_initialize(u8 spi, u8 drv)
{
    u8 timeout = CMD_TIMEOUT;
    u8 n, ocr[4];
    u8 fpb, fsd, div;

    if (drv) return STA_NOINIT;         // Supports only single drive
    if (Stat & STA_NODISK) return Stat; // No card in the socket

    // Send 74 or more clock cycles to start up
    // -----------------------------------------------------------------

    SPI_deselect(spi);
    for (n = 80; n; n--)
        SPI_write(spi, 0xFF);

    // Send in the "software reset" command (CMD0)
    // -----------------------------------------------------------------
    // The card enters SPI mode and responds R1 with IN_IDLE_STATE bit (0x01).
    // In idle state, the card accepts only CMD0, CMD1, ACMD41,CMD58 and CMD59

    if (disk_sendcommand(spi, GO_IDLE_STATE, 0) != 0x01)
    {
        #ifdef SD_DEBUG
        //ST7735_printf(SPI2, "Card not ready\r\n");
        #endif
        return STA_NODISK;
    }

    // Send a CMD8 with voltage pattern (only valid with SDv2)
    // -----------------------------------------------------------------
    // CMD8 has been rejected, try ACMD41 or CMD1
    // ACMD41 instead of CMD1 is recommended for SDC,
    // so we try ACMD41 first and retry with CMD1 if rejected
    // -----------------------------------------------------------------

    if (disk_sendcommand(spi, SEND_IF_COND, 0x1AA) != 0x01) // CMD8
    {
        if (disk_sendcommand(spi, SD_SEND_OP_COND, 0) == CMD_OK) // ACMD41
        {
            type = CT_SD1;
            #ifdef SD_DEBUG
            //debugf("Found SD type 1\r\n");
            #endif
        }
        else if (disk_sendcommand(spi, SEND_OP_COND, 0) == CMD_OK) // CMD1
        {
            type = CT_MMC;
            #ifdef SD_DEBUG
            //debugf("Found MMC\r\n");
            #endif
        }
        else
        {
            #ifdef SD_DEBUG
            //debugf("Unknown Interface\r\n");
            #endif
            return STA_NOINIT;
        }
    }
    
    // CMD8 has been accepted
    // -----------------------------------------------------------------

    else
    {
   
        // Get trailing return value of R7 response
        for (n = 0; n < 4; n++)
            ocr[n] = SPI_read(spi);

        #ifdef SD_DEBUG
        //ST7735_printf(SPI2, "CMD8=0x%X%X\r\n", ocr[2], ocr[3]);
        //ST7735_printf(SPI2, "R7=0x%02X%02X%02X%02X\r\n", ocr[3], ocr[2], ocr[1], ocr[0]);
        #endif
        
        // Check if the voltage pattern has been return
        if (ocr[2] == 0x01 && ocr[3] == 0xAA)
        {
            // Repeat sending ACMD41 + HCS bit until the card responds
            // with an ok value of 0x00
            if (disk_sendcommand(spi, SD_SEND_OP_COND, (u32)1<<30) != 0x00)
                return STA_NOINIT;
            
            // Send CMD58 and Check CCS bit in the OCR
            if (disk_sendcommand(spi, READ_OCR, 0) == CMD_OK) // CMD58
            {
                for (n = 0; n < 4; n++)
                    ocr[n] = SPI_read(spi);

                type = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2; // SDHC or SDv2

                #ifdef SD_DEBUG
                //ST7735_printf(SPI2, "OCR[0]=0x%X\n\r",ocr[0]);
                //ST7735_printf(SPI2, "R7=0x%02X%02X%02X%02X\r\n", ocr[3], ocr[2], ocr[1], ocr[0]);
                /*
                if (type == CT_SD2)
                    ST7735_printf(SPI2, "Found SD type 2\r\n");
                else
                    ST7735_printf(SPI2, "Found SDHC\r\n");
                */
                #endif
            }
        }
    }

    // Turn off the CRC requirement // CMD59
    // -----------------------------------------------------------------

    if (disk_sendcommand(spi, CRC_ON_OFF, 0) != CMD_OK)
        return STA_NOINIT;

    #ifdef SD_DEBUG
    //ST7735_printf(SPI2, "Turn off the CRC\r\n");
    #endif

    // CMD16 sets R/W block length
    // For SDHC the block length is always fixed to 512 bytes
    // but for other types (MMC, SD v1 and SD v2 standard capacity),
    // we need to tell the card what block length to use.
    // Block length is set to 512 bytes to allow the software to be used
    // with all types of SD/MMC cards.
    // -----------------------------------------------------------------

    if (type != CT_SD2 | CT_BLOCK)
        if (disk_sendcommand(spi, SET_BLOCK_LEN, 512) != CMD_OK)
            return STA_NOINIT;

    #ifdef SD_DEBUG
    //ST7735_printf(SPI2, "Set block length to 512\r\n");
    #endif

    // Set SD status
    // -----------------------------------------------------------------

    if (type)
        // Clear STA_NOINIT flag
        Stat &= ~STA_NOINIT;
    else
        // Set STA_NOINIT flag
        Stat |= STA_NOINIT;

    // Ramp the SPI clock up to full speed
    // -----------------------------------------------------------------

    if (type && (spi != SPISW))
    {
        // 6. increase speed to the max. baud rate possible
        SPI_close(spi);
        SPI_setDataMode(spi, SPI_MODE1); // SPI_MODE3

        #ifdef SD_DEBUG
        //ST7735_printf(SPI2, "SpeedClass=%dMB/s\r\n", type);
        #endif

        // fpb = FOSC/4 = Max. SPI speed
        fpb = System_getPeripheralFrequency() / 1000000;
        // fsd = SD Class Speed
        // ex. Class 4 : 4 MB/s = 32 Mb/s
        // SPI send 1 bit per clock tick
        // so Class 4 should support up to 32MHz SPI
        fsd = type * 8;

        // if fsd < fpb then fspi = fsd
        if (fsd < fpb)
        {
            for (n=1; n<=10; n++)
            {
                div = 1 << n;
                if ( (fpb/div) < fsd )
                    break;
            }
            SPI_setClockDivider(spi, div);
            #ifdef SD_DEBUG
            //ST7735_printf(SPI2, "SPI @ %dMHz\r\n", fpb/div);
            #endif
        }
        
        // otherwise fsd >= fpb so wwe set Fspi = max
        else
        {
            SPI_setMode(spi, SPI_MASTER_FOSC_4);
            #ifdef SD_DEBUG
            //ST7735_printf(SPI2, "SPI @ %dMHz\r\n", fpb);
            #endif
        }
        
        SPI_begin(spi);
    }

    return Stat;
}

/*  --------------------------------------------------------------------
    Get Disk Status
    u8 drv : Physical drive number (0)
    ------------------------------------------------------------------*/

DSTATUS disk_status(u8 drv)
{
    if (drv)
        return STA_NOINIT; /* Supports only single drive */
    return Stat;
}

/*--------------------------------------------------------------------*/
/* Receive a data packet from MMC                                     */
/* Returns : 1:OK, 0:Failed                                           */
/* u8 *buff : Data buffer to store received data                      */
/* u16 btr : Byte count (must be multiple of 4)                       */
/*--------------------------------------------------------------------*/

static u8 disk_readblock(u8 spi, u8 *buff, u16 btr)
{
    u8 token;
    u8 timeout = 200;
    
    do
        token = SPI_read(spi);
    while ((token == 0xFF) && --timeout);

    // Wait for disk ready
    // If not valid data token, return with error */

    //if (disk_getresponse(spi) != 0xFE)
    if (token != 0xFE)
    {
        #ifdef SD_DEBUG
        //ST7735_printf(SPI2, "NO VALID DATA TOKEN RETURNED\r\n");
        #endif
        return 0;
    }
    
    /* Receive the data block into buffer */
    do
    {
        *(buff++) = SPI_read(spi);
        *(buff++) = SPI_read(spi);
        *(buff++) = SPI_read(spi);
        *(buff++) = SPI_read(spi);
    } while (btr -= 4);

    /* Send Dummy CRC */
    SPI_write(spi, 0xFF);
    SPI_write(spi, 0xFF);

    /* Return with success */
    return 1;
}

/*  --------------------------------------------------------------------
    Send a data packet to MMC
    * u8 spi:   SPI module (SPISW, SPI1, SPI2, ...)
    * u8 *buff : 512 byte data block to be transmitted
    * u8 token : Data token
    Returns : 1:OK, 0:Failed
    ------------------------------------------------------------------*/

#if _FS_READONLY == 0
static int disk_writeblock(u8 spi, const u8 *buff, u8 token)
{
    u8 res;
    u16 bc = 512;

    if (!disk_ready(spi))
        return 0;

    /* Xmit a token */
    SPI_write(spi, token);

    /* Not StopTran token */
    if (token != 0xFD)
    {

        /* Xmit the 512 byte data block to the MMC */
        do {
            SPI_write(spi, *buff++);
            SPI_write(spi, *buff++);
        } while (bc -= 2);
        
        /* Send dummy CRC */
        SPI_write(spi, 0xFF);
        SPI_write(spi, 0xFF);

        /* Receive a data response */
        res = SPI_read(spi);

        /* If not accepted, return with error */
        if ((res & 0x1F) != 0x05)
            return 0;
    }

    return 1;
}
#endif	/* _FS_READONLY */

/*  --------------------------------------------------------------------
    Read Sector(s)
    u8 drv : Physical drive nmuber (0) 
    u8 *buff : Pointer to the data buffer to store read data 
    u32 sector : Start sector number (LBA)
    u8 count : Sector count (1..255)
    ------------------------------------------------------------------*/

DRESULT disk_readsector(u8 spi, u8 drv, u8 *buff, u32 sector, u8 count)
{
    if (drv || !count)
        return RES_PARERR;

    if (Stat & STA_NOINIT)
        return RES_NOTRDY;

    // Convert to byte address if  needed
    if (!(type & CT_BLOCK))
        sector *= 512;
        //sector <= 9;

    /* Single block read */
    if (count == 1)
    {
        #ifdef SD_DEBUG
        //ST7735_printf(SPI2, "Single block read\r\n");
        #endif
        // check if command was accepted
        if (disk_sendcommand(spi, READ_SINGLE_BLOCK, sector) == CMD_OK)
        {
            #ifdef SD_DEBUG
            ST7735_printf(SPI2, "Readind sector %d\r\n", sector);
            #endif
            if (disk_readblock(spi, buff, 512))
                count = 0;
        }
    }
    
    /* Multiple block read */
    else
    {
        #ifdef SD_DEBUG
        //ST7735_printf(SPI2, "Multiple block read\r\n");
        #endif

        // check if command was accepted
        if (disk_sendcommand(spi, READ_MULTIPLE_BLOCKS, sector) == CMD_OK)
        {
            #ifdef SD_DEBUG
            //ST7735_printf(SPI2, "Readind sector %d\r\n", sector);
            #endif
            do {
                if (!disk_readblock(spi, buff, 512))
                    break;
                buff += 512;
            } while (--count);
            disk_sendcommand(spi, STOP_TRANSMISSION, 0);
        }
    }
    
    //SPI_deselect(spi);

    #ifdef SD_DEBUG
    //ST7735_printf(SPI2, "count=%d\r\n", count);
    #endif

    return (count ? RES_ERROR : RES_OK);
}

/*  --------------------------------------------------------------------
    Write Sector(s)
    u8 drv : Physical drive nmuber (0)
    u8 *buff : Pointer to the data buffer to store read data
    u32 sector : Start sector number (LBA)
    u8 count : Sector count (1..255)
    ------------------------------------------------------------------*/

#if _FS_READONLY == 0
DRESULT disk_writesector(u8 spi, u8 drv, const u8 *buff, u32 sector, u8 count)
{
    if (drv || !count)
        return RES_PARERR;

    if (Stat & STA_NOINIT)
        return RES_NOTRDY;

    if (Stat & STA_PROTECT)
        return RES_WRPRT;

    // Convert to byte address if needed
    if (!(type & CT_BLOCK))
        sector *= 512;

    // Single block write
    if (count == 1)
    {
        if (disk_sendcommand(spi, WRITE_SINGLE_BLOCK, sector) == CMD_OK)
           if (disk_writeblock(spi, buff, 0xFE))
                count = 0;
    }
    
    /* Multiple block write */
    else
    {
        if (type & CT_SDC)
            disk_sendcommand(spi, SET_WR_BLK_ERASE_COUNT, count);

        /* WRITE_MULTIPLE_BLOCK */
        if (disk_sendcommand(spi, WRITE_MULTIPLE_BLOCKS, sector) == CMD_OK)
        {
            do {
                if (disk_writeblock(spi, buff, 0xFC) != CMD_OK)
                    break;
                buff += 512;
            } while (--count);

            if (disk_writeblock(spi, 0, 0xFD) != CMD_OK) /* STOP_TRAN token */
                count = 1;
        }
    }
    
    SPI_deselect(spi);

    return count ? RES_ERROR : RES_OK;
}
#endif /* _READONLY */

/*  --------------------------------------------------------------------
    Display Sector(s)
    u8 drv : Physical drive nmuber (0)
    u8 *buff : Pointer to the data buffer to store read data
    u32 sector : Start sector number (LBA)
    u8 count : Sector count (1..255)
    Input:           Pointer to a 512 byte buffer
    Output:          Humen readable data
    Overview:        Data is outputed in groups of 16 bytes per row
    ------------------------------------------------------------------*/

#if _USE_STRFUNC
/*
const char * disk_displaysector(const char * datx)
{
    u16 k, px;
    const char *str;

    for(k = 0; k < 512; k++)
    {
        Serial_printf("%2X ",datx[k]);

        if( ((k + 1) % 16) == 0)
        {
            Serial_printf("  ");

            for(px = (k - 15); px <= k; px++)
            {
                if( ((datx[px] > 33) && (datx[px] < 126)) || (datx[px] == 0x20) )
                {
                    Serial_printf("%c ",datx[px]);
                }
                else
                {
                    Serial_printf(".");
                }
            }

            Serial_printf("\n\r");
        }
    }

    return str;
}
*/
#endif

/*  --------------------------------------------------------------------
    Miscellaneous Functions
    u8 drv : Physical drive number (0)
    u8 ctrl : Control code
    void *buff : Buffer to send/receive data block
    ------------------------------------------------------------------*/

DRESULT disk_ioctl(u8 spi, u8 drv, u8 ctrl, void *buff)
{
    DRESULT res;
    u8 n, csd[16], *ptr = buff;
    u32 csize;

    if (drv)
        return RES_PARERR;
        
    if (Stat & STA_NOINIT)
        return RES_NOTRDY;

    res = RES_ERROR;

    switch (ctrl)
    {

        /* Flush dirty buffer if present */
        case CTRL_SYNC:
            if (disk_select(spi))
            {
                SPI_deselect(spi);
                res = RES_OK;
            }
            break;

        /* Get number of sectors on the disk (u16) */
        case GET_SECTOR_COUNT:
            if ((disk_sendcommand(spi,SEND_CSD, 0) == CMD_OK)
                && disk_readblock(spi, csd, 16))
            {
                /* SDv2? */
                if ((csd[0] >> 6) == 1)
                {
                    csize = csd[9] + ((u16) csd[8] << 8) + 1;
                    *(u32*) buff = (u32) csize << 10;
                }
                
                /* SDv1 or MMCv2 */
                else
                {
                    n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
                    csize = (csd[8] >> 6) + ((u16) csd[7] << 2) + ((u16) (csd[6] & 3) << 10) + 1;
                    *(u32*) buff = (u32) csize << (n - 9);
                }
                res = RES_OK;
            }
            break;

        /* Get sectors on the disk (u16) */
        case GET_SECTOR_SIZE:
            *(u16*) buff = 512;
            res = RES_OK;
            break;

        /* Get erase block size in unit of sectors (u32) */
        case GET_BLOCK_SIZE:
            /* SDv2? */
            if (type & CT_SD2)
            {
                /* Read SD status */
                if (disk_sendcommand(spi, SD_STATUS, 0) == CMD_OK)
                {
                    SPI_write(spi, 0xFF);
                    /* Read partial block */
                    if (disk_readblock(spi, csd, 16))
                    {
                        for (n = 64 - 16; n; n--)
                            SPI_write(spi, 0xFF); /* Purge trailing data */

                        *(u32*) buff = 16UL << (csd[10] >> 4);
                        res = RES_OK;
                    }
                }
            }
            
            /* SDv1 or MMCv3 */
            else
            {
                /* Read CSD */
                if ((disk_sendcommand(spi, SEND_CSD, 0) == CMD_OK)
                    && disk_readblock(spi, csd, 16))
                {
                    /* SDv1 */
                    if (type & CT_SD1)
                        *(u32*) buff = (((csd[10] & 63) << 1) + ((u16) (csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
                    /* MMCv3 */
                    else
                        *(u32*) buff = ((u16) ((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
                    res = RES_OK;
                }
            }
            break;

        /* Get card type flags (1 byte) */
        case MMC_GET_TYPE:
            *ptr = type;
            res = RES_OK;
            break;

        /* Receive CSD as a data block (16 bytes) */
        case MMC_GET_CSD:
            /* READ_CSD */
            if ((disk_sendcommand(spi, SEND_CSD, 0) == CMD_OK)
                && disk_readblock(spi, buff, 16))
                res = RES_OK;
            break;

        /* Receive CID as a data block (16 bytes) */
        case MMC_GET_CID:
            /* READ_CID */
            if ((disk_sendcommand(spi, SEND_CID, 0) == CMD_OK)
                && disk_readblock(spi, buff, 16))
                res = RES_OK;
            break;

        /* Receive OCR as an R3 resp (4 bytes) */
        case MMC_GET_OCR:
            if (disk_sendcommand(spi, READ_OCR, 0) == CMD_OK)
            {
                for (n = 0; n < 4; n++)
                    *((u8*) buff + n) = SPI_write(spi, 0xFF);
                res = RES_OK;
            }
            break;

        /* Receive SD status as a data block (64 bytes) */
        case MMC_GET_SDSTAT:
            if (disk_sendcommand(spi, SD_STATUS, 0) == CMD_OK)
            {
                SPI_write(spi, 0xFF);
                if (disk_readblock(spi, buff, 64))
                    res = RES_OK;
            }
            break;

        default:
            res = RES_PARERR;
    }

    SPI_deselect(spi);

    return res;
}

/*  --------------------------------------------------------------------
    initializes a MEDIA structure for file access
    will mount only the first partition on the disk/card
    ------------------------------------------------------------------*/

FRESULT disk_mount(u8 module, FATFS *fs, ...)
{
    u8 sda, sck, cs;
    va_list args;

    va_start(args, module); // args points on the argument after module

    #if defined(SD_DEBUG) && defined(__SERIAL__)
    Serial_begin(9600);
    #endif
    
    // Init the SPI module
    // -----------------------------------------------------------------
    
    if (module == SPISW)
    {
        sda = va_arg(args, u8);             // get the next arg
        sck = va_arg(args, u8);             // get the next arg
        cs  = va_arg(args, u8);             // get the last arg
        SPI_setPin(module, sda, sck, cs);
        SPI_setBitOrder(module, SPI_MSBFIRST);
    }
    else
    { 
        //minimum baud rate possible = FPB/64
        SPI_setMode(module, SPI_MASTER_FOSC_64);
        SPI_setDataMode(module, SPI_MODE1);
        SPI_begin(module);
    }

    // Memory allocation
    // -----------------------------------------------------------------

    return f_mount(0, fs);
}

/*  --------------------------------------------------------------------
    free MEDIA structure and SPI module
    ------------------------------------------------------------------*/

void disk_unmount(u8 module)
{
    f_mount(0, NULL);
    SPI_deselect(module);
    SPI_close(module);
}

/*  --------------------------------------------------------------------
    display error
    ------------------------------------------------------------------*/

const char * put_rc(FRESULT rc)
{
    FRESULT i;
    const char *str =
        "OK\0" "DISK_ERR\0" "INT_ERR\0" "NOT_READY\0" "NO_FILE\0" "NO_PATH\0"
        "INVALID_NAME\0" "DENIED\0" "EXIST\0" "INVALID_OBJECT\0" "WRITE_PROTECTED\0"
        "INVALID_DRIVE\0" "NOT_ENABLED\0" "NO_FILE_SYSTEM\0" "MKFS_ABORTED\0" "TIMEOUT\0"
        "LOCKED\0" "NOT_ENOUGH_CORE\0" "TOO_MANY_OPEN_FILES\0";

    for (i = 0; i != rc && *str; i++)
        while (*str++);

    return str;
}

/*  --------------------------------------------------------------------
    Device Timer Interrupt Procedure  (Platform dependent)
    This function must be called in period of 1ms
    ------------------------------------------------------------------*/

#if 0
void disk_timerproc(u8 spi)
{
    static u16 pv;
    u16 p;
    u8 s;
    u16 n;

    n = Timer1; /* 1000Hz decrement timer */
    if (n)
        Timer1 = --n;

    n = Timer2;
    if (n)
        Timer2 = --n;

    p = pv;
    pv = getCD(spi) & getWP(spi); /* Sample socket switch */

    /* Have contacts stabled? */
    if (p == pv)
    {
        s = Stat;

        if (p & getWP(spi)) /* WP is H (write protected) */
            s |= STA_PROTECT;
        else
            /* WP is L (write enabled) */
            s &= ~STA_PROTECT;

        if (p & getCD(spi)) /* INS = H (Socket empty) */
            s |= (STA_NODISK | STA_NOINIT);
        else
            /* INS = L (Card inserted) */
            s &= ~STA_NODISK;

        Stat = s;
    }
}
#endif

/*  --------------------------------------------------------------------
    User Provided RTC Function for FatFs spi
    This is a real time clock service to be called from FatFs spi.
    Any valid time must be returned even if the system does not support
    an RTC.
    This function is not required in read-only cfg.
    The current time is returned packed into a u32
    (32 bit) value. The bit fields are as follows:
        bits 31:25	Year from 1980 (0..127)
        bits 24:21	Month (1..12)
        bits 20:16	Day in month (1..31)
        bits 15:11	Hour (0..23)
        bits 10:05	Minute (0..59)
        bits 04:00	Second / 2 (0..29)
    ------------------------------------------------------------------*/

u32 get_fattime(void)
{
    u32 tmr = 0;

    // For boards known to support the RTCC library
    #if defined(__18f26j50) || defined(__18f46j50) || \
        defined(__18f27j53) || defined(__18f47j53)
    
    rtccTime pTm, cTm;
    rtccDate pDt, cDt;

    RTCC_GetTimeDate(&pTm, &pDt);       // get time and date from RTC
                                        // assumes RTC has been set and is running
                                        // OK - could be expanded to check that RTC
                                        // is running and that a valid value is
                                        // being returned by the RTC
    cTm = RTCC_ConvertTime(&pTm);       // convert time from bcd to decimal format
    cDt = RTCC_ConvertDate(&pDt);       // convert date from bcd to decimal format

    // Pack date and time into a u32 variable
    tmr = cDt.year + 20;
    tmr = (tmr << 4) | cDt.month;       // shifts left 4 bits and adds monthth
    tmr = (tmr << 5) | cDt.dayofmonth;  // shifts left 5 bits and adds m.day
    tmr = (tmr << 5) | cTm.hours;       // shifts left 5 bits and adds hour
    tmr = (tmr << 6) | cTm.minutes;     // shift left 6 bits and adds minutes
    tmr = (tmr << 5) | (cTm.seconds/2); // shifts left 5 bits and adds seconds/2

    // For other boards use a fixed date and time of 01 Jan 2012 12:00:00
    #else

    tmr = 12 + 20;
    tmr = (tmr << 4) | 1;               // shifts left 4 bits and adds month
    tmr = (tmr << 5) | 1;               // shifts left 5 bits and adds m.day
    tmr = (tmr << 5) | 12;              // shifts left 5 bits and adds hour
    tmr = (tmr << 6) | 0;               // shift left 6 bits and adds minutes
    tmr = (tmr << 5) | (0/2);           // shifts left 5 bits and adds seconds/2

    #endif

    return tmr;
}

#endif // _DISKIO_C
