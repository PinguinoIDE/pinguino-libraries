/*
 ** fileio.h
 **
 ** FAT16 support
 **
 ** 01/16/03	v1.0 LDJ PIC18
 ** 08/17/06 v2.0 LDJ PIC24 and SDMMC cards porting
[30-03-12][hgmvanbeek@gmail.com][Some cards have no card detect and no write protect]
07 May 2012	As part of providing support for PIC32 Pinguino Micro and
				potentially other cards removed #if !defined (PIC32_Pinguino) etc
				and #endif under "globals" so that SDCS is declared in the same
				manner for all cards and then set via the SD.mount() function.
 */

#ifndef __FILEIO_H__
#define __FILEIO_H__

#include <compiler.h>
#include <typedef.h>
#include <spi.h>
#include <sd/ffconf.h>
#if _FS_TINY
#include <sd/tff.h>             // Tiny Fat Filesystem
#else
#include <sd/pff.h>             // Petit Fat Filesystem
//#include <sd/ff.h>            // Fat Filesystem
#endif

#ifdef SD_DEBUG
    //#define CDCPRINTF
    //#define debugf          CDCprintf
    //#define SERIALPRINTF
    //#define debugf          Serial_printf
    void debugf(const u8 *fmt, ...);
    //void debugf(const u8 *fmt, va_list args);
#endif


//FATFS *Fat; // mounting info for storage device

//u32 AccSize; /* Work register for fs command */
//u16 AccFiles, AccDirs;
//FILINFO Finfo;

#if 0
typedef struct {
	char filename[9];
	char ext[4];
	char attrib;
	char reserved;
	char time;
	int ctime;
	int cdate;
	int latime;
	int eaindex;
	int ltime;
	int ldate;
	int cluster;
	long size;
} DIRTABLE;

//unsigned listTYPE(char *listname, long *listsize, int max, const char *ext);
unsigned listTYPE(DIRTABLE *list, int max, const char *ext);
//unsigned listTYPE(char *list, int max, const char *ext);
unsigned listDir(u8, const char *path);
#endif



#endif /* __FILEIO_H__ */
