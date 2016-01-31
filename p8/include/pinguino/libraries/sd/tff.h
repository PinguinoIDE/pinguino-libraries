/*--------------------------------------------------------------------------/
/  Tiny-FatFs - FAT file system module include file  R0.06    (C)ChaN, 2008
/---------------------------------------------------------------------------/
/ FatFs module is an experimenal project to implement FAT file system to
/ cheap microcontrollers. This is a free software and is opened for education,
/ research and development under license policy of following trems.
/
/  Copyright (C) 2008, ChaN, all right reserved.
/
/ * The FatFs module is a free software and there is no warranty.
/ * You can use, modify and/or redistribute it for personal, non-profit or
/   commercial use without any restriction under your responsibility.
/ * Redistributions of source code must retain the above copyright notice.
/
/---------------------------------------------------------------------------*/

#ifndef _FATFS
#define _FATFS

#include <typedef.h>
#include <sd/ffconf.h>

/* Type definition for cluster number */
#if _FAT32
typedef dword	CLUST;
#else
typedef word	CLUST;
#undef _USE_FSINFO
#define _USE_FSINFO	0
#endif

/* Definitions corresponds to multiple sector size (not tested) */
#define	S_MAX_SIZ	512U			/* Do not change */
#if S_MAX_SIZ > 512U
#define	SSZ(fs)	((fs)->s_size)
#else
#define	SSZ(fs)	512U
#endif

/* File system object structure */
typedef struct {
    word    id;				/* File system mount ID */
    word    n_rootdir;		/* Number of root directory entries */
    dword   winsect;		/* Current sector appearing in the win[] */
    dword   fatbase;		/* FAT start sector */
    dword   dirbase;		/* Root directory start sector */
    dword   database;		/* Data start sector */
    CLUST   sects_fat;		/* Sectors per fat */
    CLUST   max_clust;		/* Maximum cluster# + 1 */
    #if !_FS_READONLY
    CLUST   last_clust;		/* Last allocated cluster */
    CLUST   free_clust;		/* Number of free clusters */
    #if _USE_FSINFO
    dword   fsi_sector;		/* fsinfo sector */
    u8      fsi_flag;		/* fsinfo dirty flag (1:must be written back) */
    u8      pad1;
    #endif
    #endif
    u8      fs_type;		/* FAT sub type */
    u8      csize;			/* Number of sectors per cluster */
    u8      n_fats;			/* Number of FAT copies */
    u8      winflag;		/* win[] dirty flag (1:must be written back) */
    u8      win[512];		/* Disk access window for Directory/FAT/File */
} FATFS;

/* Directory object structure */
typedef struct {
    word	id;			/* Owner file system mount ID */
    word	index;		/* Current index */
    FATFS*	fs;			/* Pointer to the owner file system object */
    CLUST	sclust;		/* Start cluster */
    CLUST	clust;		/* Current cluster */
    dword	sect;		/* Current sector */
} DIR_t;

/* File object structure */
typedef struct {
    word	id;				/* Owner file system mount ID */
    u8	flag;			/* File status flags */
    u8	csect;			/* Sector address in the cluster */
    FATFS*	fs;				/* Pointer to owner file system */
    dword	fptr;			/* File R/W pointer */
    dword	fsize;			/* File size */
    CLUST	org_clust;		/* File start cluster */
    CLUST	curr_clust;		/* Current cluster */
    dword	curr_sect;		/* Current sector */
    #if !_FS_READONLY
    dword	dir_sect;		/* Sector containing the directory entry */
    u8*	dir_ptr;		/* Ponter to the directory entry in the window */
    #endif
} FIL;

/* File status structure */
typedef struct {
    dword fsize;			/* Size */
    word fdate;				/* Date */
    word ftime;				/* Time */
    u8 fattrib;			/* Attribute */
    char fname[8+1+3+1];	/* Name (8.3 format) */
} FILINFO;

/* File function return code (FRESULT) */
typedef enum {
    FR_OK = 0,			/* 0 */
    FR_NOT_READY,		/* 1 */
    FR_NO_FILE,			/* 2 */
    FR_NO_PATH,			/* 3 */
    FR_INVALID_NAME,	/* 4 */
    FR_INVALID_DRIVE,	/* 5 */
    FR_DENIED,			/* 6 */
    FR_EXIST,			/* 7 */
    FR_RW_ERROR,		/* 8 */
    FR_WRITE_PROTECTED,	/* 9 */
    FR_NOT_ENABLED,		/* 10 */
    FR_NO_FILESYSTEM,	/* 11 */
    FR_INVALID_OBJECT,	/* 12 */
    FR_MKFS_ABORTED		/* 13 (not used) */
} FRESULT;

u8 move_window(u8, dword);

/*-----------------------------------------------------*/
/* Tiny-FatFs module application interface             */

FRESULT f_mount (u8, FATFS*);						/* Mount/Unmount a logical drive */
//FRESULT f_mount (u8);						            /* Mount/Unmount a logical drive */
FRESULT f_open (u8, FIL*, const char*, u8);			/* Open or create a file */
FRESULT f_read (u8, FIL*, void*, word, word*);			/* Read data from a file */
u16 f_read16(u8, FIL*);
u32 f_read32(u8, FIL*);
FRESULT f_write (u8, FIL*, const void*, word, word*);	/* Write data to a file */
FRESULT f_lseek (u8, FIL*, dword);						/* Move file pointer of a file object */
FRESULT f_close (u8, FIL*);								/* Close an open file object */
FRESULT f_opendir (u8, DIR_t*, const char*);			/* Open an existing directory */
FRESULT f_readdir (u8, DIR_t*, FILINFO*);				/* Read a directory item */
FRESULT f_stat (u8, const char*, FILINFO*);				/* Get file status */
FRESULT f_getfree (u8, const char*, dword*, FATFS**);	/* Get number of free clusters on the drive */
FRESULT f_truncate (u8, FIL*);							/* Truncate file */
FRESULT f_sync (u8, FIL*);								/* Flush cached data of a writing file */
FRESULT f_unlink (u8, const char*);						/* Delete an existing file or directory */
FRESULT	f_mkdir (u8, const char*);						/* Create a new directory */
FRESULT f_chmod (u8, const char*, u8, u8);			/* Change file/dir attriburte */
FRESULT f_utime (u8, const char*, const FILINFO*);		/* Change file/dir timestamp */
FRESULT f_rename (u8, const char*, const char*);		/* Rename/Move a file or directory */
FRESULT f_forward (u8, FIL*, word(*)(const u8*,word), word, word*);	/* Forward data to the stream */

#if _USE_STRFUNC
#define feof(fp) ((fp)->fptr == (fp)->fsize)
#define EOF -1
int fputc (u8, int, FIL*);								/* Put a character to the file */
int fputs (u8, const char*, FIL*);						/* Put a string to the file */
int fprintf (u8, FIL*, const char*, ...);				/* Put a formatted string to the file */
char* fgets (u8, FIL*, char*, int);						/* Get a string from the file */
#endif

u8 isDirectory(FILINFO *);
u8 isFile(FILINFO *);
u8 isNotEmpty(FILINFO *);
u8 isArchive(FILINFO *);
u8 isHidden(FILINFO *);
u8 isReadOnly(FILINFO *);
u8 isSystem(FILINFO *);
u8 * getName(FILINFO *);
u32 getSize(FILINFO *);

/* User defined function to give a current time to fatfs module */

extern dword get_fattime (void);
/* 31-25: Year(0-127 +1980), 24-21: Month(1-12), 20-16: Day(1-31) */
/* 15-11: Hour(0-23), 10-5: Minute(0-59), 4-0: Second(0-29 *2) */

/* File access control and file status flags (FIL.flag) */

#define	FA_READ				0x01
#define	FA_OPEN_EXISTING	0x00
#if !_FS_READONLY
#define	FA_WRITE			0x02
#define	FA_CREATE_NEW		0x04
#define	FA_CREATE_ALWAYS	0x08
#define	FA_OPEN_ALWAYS		0x10
#define FA__WRITTEN			0x20
#endif
#define FA__ERROR			0x80

/* FAT sub type (FATFS.fs_type) */

#define FS_FAT12	1
#define FS_FAT16	2
#define FS_FAT32	3

/* File attribute bits for directory entry */

#define	AM_RDO	0x01	/* Read only */
#define	AM_HID	0x02	/* Hidden */
#define	AM_SYS	0x04	/* System */
#define	AM_VOL	0x08	/* Volume label */
#define AM_LFN	0x0F	/* LFN entry */
#define AM_DIR	0x10	/* Directory */
#define AM_ARC	0x20	/* Archive */

/* Offset of FAT structure members */

#define BS_jmpBoot			0
#define BS_OEMName			3
#define BS_DrvNum			36
#define BS_BootSig			38
#define BS_VolID			39
#define BS_VolLab			43
#define BS_FilSysType		54
#define BS_DrvNum32			64
#define BS_BootSig32		66
#define BS_VolID32			67
#define BS_VolLab32			71
#define BS_FilSysType32		82
#define BS_55AA				510

#define BPB_BytsPerSec		11
#define BPB_SecPerClus		13
#define BPB_RsvdSecCnt		14
#define BPB_NumFATs			16
#define BPB_RootEntCnt		17
#define BPB_TotSec16		19
#define BPB_Media			21
#define BPB_FATSz16			22
#define BPB_SecPerTrk		24
#define BPB_NumHeads		26
#define BPB_HiddSec			28
#define BPB_TotSec32		32
#define BPB_FATSz32			36
#define BPB_ExtFlags		40
#define BPB_FSVer			42
#define BPB_RootClus		44
#define BPB_FSInfo			48
#define BPB_BkBootSec		50

#define	FSI_LeadSig			0
#define	FSI_StrucSig		484
#define	FSI_Free_Count		488
#define	FSI_Nxt_Free		492

#define MBR_Table			446

#define	DIR_Name			0
#define	DIR_Attr			11
#define	DIR_NTres			12
#define	DIR_CrtTime			14
#define	DIR_CrtDate			16
#define	DIR_FstClusHI		20
#define	DIR_WrtTime			22
#define	DIR_WrtDate			24
#define	DIR_FstClusLO		26
#define	DIR_FileSize		28

/* Multi-byte word access macros  */

#if _MCU_ENDIAN == 1	/* Use word access */
#define	LD_WORD(ptr)		(word)(*(word*)(u8*)(ptr))
#define	LD_DWORD(ptr)		(dword)(*(dword*)(u8*)(ptr))
#define	ST_WORD(ptr,val)	*(word*)(u8*)(ptr)=(word)(val)
#define	ST_DWORD(ptr,val)	*(dword*)(u8*)(ptr)=(dword)(val)

#elif _MCU_ENDIAN == 2	/* Use byte-by-byte access */
#define	LD_WORD(ptr)		(word)(((word)*(volatile u8*)((ptr)+1)<<8)|(word)*(volatile u8*)(ptr))
#define	LD_DWORD(ptr)		(dword)(((dword)*(volatile u8*)((ptr)+3)<<24)|((dword)*(volatile u8*)((ptr)+2)<<16)|((word)*(volatile u8*)((ptr)+1)<<8)|*(volatile u8*)(ptr))
#define	ST_WORD(ptr,val)	*(volatile u8*)(ptr)=(u8)(val); *(volatile u8*)((ptr)+1)=(u8)((word)(val)>>8)
#define	ST_DWORD(ptr,val)	*(volatile u8*)(ptr)=(u8)(val); *(volatile u8*)((ptr)+1)=(u8)((word)(val)>>8); *(volatile u8*)((ptr)+2)=(u8)((dword)(val)>>16); *(volatile u8*)((ptr)+3)=(u8)((dword)(val)>>24)

#else
#error Do not forget to set _MCU_ENDIAN properly!
#endif

#endif /* _FATFS */
