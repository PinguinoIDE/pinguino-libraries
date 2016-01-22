/*  --------------------------------------------------------------------
    Low level disk interface modlue include file  R0.05   (C)ChaN, 2007
    ------------------------------------------------------------------*/

#ifndef _DISKIO_H
#define _DISKIO_H

#include <compiler.h>
#include <typedef.h>
#include <sd/ffconf.h>

// Status of Disk Functions 
typedef u8	DSTATUS;

// Results of Disk Functions 
typedef enum
{
	RES_OK = 0,                             // 0: Successful 
	RES_ERROR,                              // 1: R/W Error 
	RES_WRPRT,                              // 2: Write Protected 
	RES_NOTRDY,                             // 3: Not Ready 
	RES_PARERR                              // 4: Invalid Parameter 
} DRESULT;

// Prototypes for disk control functions 
FRESULT disk_mount(u8, FATFS*, ...);
void disk_unmount(u8);
u8 disk_getresponse(u8);
DSTATUS disk_initialize(u8, u8);
DRESULT disk_ioctl(u8, u8, u8, void*);
DRESULT disk_readsector(u8, u8, u8*, u32, u8);
void    disk_timerproc (u8);
DWORD   get_fattime(void);
const char * put_rc (FRESULT);
u8 disk_sendcmd(u8, u8, u32);
u8 disk_sendcommand(u8, u8, u32);
DSTATUS disk_status(u8);
#if	_FS_READONLY == 0
DRESULT disk_writesector(u8, u8, const u8*, u32, u8);
#endif
const char * put_rc (FRESULT);

// Timeout
#define NCR_TIMEOUT             (u8)20      // There can be 0-8 no command return values
                                            // until the return is received
#define WRITE_TIMEOUT           (u32)0xA0000// should be at least 250ms
#define IDLE_TIMEOUT            (u32)0x7000 // should be at least 1000ms
#define CMD_TIMEOUT             (u8)0xFF

// Command return
#define CMD_OK                  0x00
#define CMD_RECEIVED            0x01

// Disk Status Bits (DSTATUS)
#define STA_NOINIT              0x01        // Drive not initialized 
#define STA_NODISK              0x02        // No medium in the drive 
#define STA_PROTECT             0x04        // Write protected 

// Command code for disk_ioctrl() 

// Generic ioctl command (defined for FatFs) 
#define CTRL_SYNC               0       	// Flush disk cache (for write functions) 
#define GET_SECTOR_COUNT        1       	// Get media size (for only f_mkfs()) 
#define GET_SECTOR_SIZE         2       	// Get sector size (for multiple sector size (_MAX_SS >= 1024)) 
#define GET_BLOCK_SIZE          3       	// Get erase block size (for only f_mkfs()) 

// Generic ioctl command 
#define CTRL_POWER              4       	// Get/Set power status 
#define CTRL_LOCK               5       	// Lock/Unlock media removal 
#define CTRL_EJECT              6       	// Eject media 

// MMC/SDC specific ioctl command 
#define MMC_GET_TYPE            10      	// Get card type 
#define MMC_GET_CSD             11      	// Get CSD 
#define MMC_GET_CID             12      	// Get CID 
#define MMC_GET_OCR             13      	// Get OCR 
#define MMC_GET_SDSTAT          14      	// Get SD status 

// ATA/CF specific ioctl command 
#define ATA_GET_REV             20      	// Get F/W revision 
#define ATA_GET_MODEL           21      	// Get model name 
#define ATA_GET_SN              22      	// Get serial number 

// NAND specific ioctl command 
#define NAND_FORMAT             30      	// Create physical format 
#define NAND_ERASE              31      	// Force erased a block 

// MMC/SDC card type definitions (CardType) 
#define CT_MMC                  0x01
#define CT_SD1                  0x02
#define CT_SD2                  0x04
#define CT_SDC                  (CT_SD1|CT_SD2)
#define CT_BLOCK                0x08

// Definitions for MMC/SDC command
#define CMD0                    (0)         // GO_IDLE_STATE
#define CMD1                    (1)         // SEND_OP_COND
#define CMD8                    (8)         // SEND_IF_COND
#define CMD9                    (9)         // SEND_CSD
#define CMD10                   (10)        // SEND_CID
#define CMD12                   (12)        // STOP_TRANSMISSION
#define ACMD13                  (13|0x80)   // SD_STATUS (SDC)
#define CMD16                   (16)        // SET_BLOCKLEN
#define CMD17                   (17)        // READ_SINGLE_BLOCK
#define CMD18                   (18)        // READ_MULTIPLE_BLOCK
#define CMD23                   (23)        // SET_BLOCK_COUNT
#define ACMD23                  (23|0x80)   // SET_WR_BLK_ERASE_COUNT (SDC)
#define CMD24                   (24)        // WRITE_BLOCK
#define CMD25                   (25)        // WRITE_MULTIPLE_BLOCK
#define CMD41                   (41)        // SEND_OP_COND (ACMD)
#define ACMD41                  (41|0x80)   // SEND_OP_COND (SDC)
#define CMD55                   (55)        // APP_CMD
#define CMD58                   (58)        // READ_OCR

#define GO_IDLE_STATE           CMD0
#define SEND_OP_COND            CMD1
#define SEND_IF_COND            CMD8
#define SEND_CSD                CMD9
#define SEND_CID                CMD10
#define STOP_TRANSMISSION       CMD12
#define SEND_STATUS             CMD13
#define SD_STATUS               ACMD13
#define SET_BLOCK_LEN           CMD16
#define READ_SINGLE_BLOCK       CMD17
#define READ_MULTIPLE_BLOCKS    CMD18
#define SET_BLOCK_COUNT         CMD23
#define SET_WR_BLK_ERASE_COUNT  ACMD23
#define WRITE_SINGLE_BLOCK      CMD24
#define WRITE_MULTIPLE_BLOCKS   CMD25
#define ERASE_BLOCK_START_ADDR  32
#define ERASE_BLOCK_END_ADDR    33
#define ERASE_SELECTED_BLOCKS   38
#define SD_SEND_OP_COND         ACMD41
#define APP_CMD                 CMD55
#define READ_OCR                CMD58
#define CRC_ON_OFF              59

// FILEIO ERROR CODES
#define FE_IDE_ERROR        1   // IDE command execution error
#define FE_NOT_PRESENT      2   // CARD not present
#define FE_PARTITION_TYPE   3   // WRONG partition type
#define FE_INVALID_MBR      4   // MBR sector invalid signtr
#define FE_INVALID_BR       5   // Boot Record invalid signtr
#define FE_MEDIA_NOT_MNTD   6   // Media not mounted
#define FE_FILE_NOT_FOUND   7   // File not found,open for read
#define FE_INVALID_FILE     8   // File not open
#define FE_FAT_EOF          9   // attempt to read beyond EOF
#define FE_EOF             10   // Reached the end of file
#define FE_INVALID_CLUSTER 11   // Invalid cluster > maxcls
#define FE_DIR_FULL        12   // All root dir entry are taken
#define FE_MEDIA_FULL      13   // All clusters taken
#define FE_FILE_OVERWRITE  14   // A file with same name exist
#define FE_CANNOT_INIT     15   // Cannot init the CARD
#define FE_CANNOT_READ_MBR 16   // Cannot read the MBR
#define FE_MALLOC_FAILED   17   // Could not allocate memory
#define FE_INVALID_MODE    18   // Mode was not r.w.
#define FE_FIND_ERROR      19   // Failure during FILE search

// file attributes
#define ATT_RO      1           // attribute read only
#define ATT_HIDE    2           // attribute hidden
#define ATT_SYS     4           //  "       system file
#define ATT_VOL     8           //  "       volume label
#define ATT_DIR     0x10        //  "       sub-directory
#define ATT_ARC     0x20        //  "       (to) archive
#define ATT_LFN     0x0f        // mask for Long File Name
#define FOUND       2           // directory entry match
#define NOT_FOUND   1           // directory entry not found

#define DW_CHAR		sizeof(char)
#define DW_SHORT	sizeof(short)
#define DW_LONG		sizeof(long)

// macros to extract words and longs from a byte array
// watch out, a processor trap will be generated if the address
// is not word aligned
#define ReadW( a, f) *(unsigned short*)(a+f)
#define ReadL( a, f) *(unsigned short*)(a+f) + (( *(unsigned short*)(a+f+2))<<16)

// this is a "safe" versions of ReadW
// to be used on odd address fields
#define ReadOddW( a, f) (*(a+f) + ( *(a+f+1) << 8))

// Global
char FError; // error mail box
//DSTATUS Stat; /* Disk status */

#endif // _DISKIO_H
