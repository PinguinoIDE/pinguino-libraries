/*---------------------------------------------------------------------------/
/  FatFs - FAT file system module include file  R0.09     (C)ChaN, 2011
/----------------------------------------------------------------------------/
/ FatFs module is a generic FAT file system module for small embedded systems.
/ This is a free software that opened for education, research and commercial
/ developments under license policy of following trems.
/
/  Copyright (C) 2011, ChaN, all right reserved.
/
/ * The FatFs module is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial product UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/----------------------------------------------------------------------------*/

#ifndef _FATFS
#define _FATFS	6502	/* Revision ID */

#ifdef __cplusplus
extern "C" {
#endif

#include <compiler.h>
#include <typedef.h>
#include <sd/ffconf.h>  /* FatFs configuration options */

#if _FATFS != _FFCONF
#error Wrong configuration file (ffconf.h).
#endif

/* Definitions of volume management */

#if _MULTI_PARTITION		/* Multiple partition configuration */

typedef struct
{
    u8 pd;	/* Physical drive number */
    u8 pt;	/* Partition: 0:Auto detect, 1-4:Forced partition) */
} PARTITION;

extern PARTITION VolToPart[];	/* Volume - Partition resolution table */

#define LD2PD(vol) (VolToPart[vol].pd)	/* Get physical drive number */
#define LD2PT(vol) (VolToPart[vol].pt)	/* Get partition index */

#else						/* Single partition configuration */

#define LD2PD(vol) (vol)	/* Each logical drive is bound to the same physical drive number */
#define LD2PT(vol) 0		/* Always mounts the 1st partition or in SFD */

#endif



/* Type of path name strings on FatFs API */

#if _LFN_UNICODE            /* Unicode string */

    #if !_USE_LFN
        #error _LFN_UNICODE must be 0 in non-LFN cfg.
    #endif

    #ifndef _INC_TCHAR
        typedef WCHAR TCHAR;
        #define _T(x) L ## x
        #define _TEXT(x) L ## x
    #endif

#else                       /* ANSI/OEM string */

    #ifndef _INC_TCHAR
        typedef char TCHAR;
        #define _T(x) x
        #define _TEXT(x) x
    #endif

#endif

/* File system object structure (FATFS) */

typedef struct
{
    u8	fs_type;		/* FAT sub-type (0:Not mounted) */
    u8	drv;			/* Physical drive number */
    u8	csize;			/* Sectors per cluster (1,2,4...128) */
    u8	n_fats;			/* Number of FAT copies (1,2) */
    u8	wflag;			/* win[] dirty flag (1:must be written back) */
    u8	fsi_flag;		/* fsinfo dirty flag (1:must be written back) */
    u16	id;				/* File system mount ID */
    u16	n_rootdir;		/* Number of root directory entries (FAT12/16) */
    #if _MAX_SS != 512
    u16	ssize;			/* Bytes per sector (512, 1024, 2048 or 4096) */
    #endif
    #if _FS_READONLY == 0
    u32	last_clust;		/* Last allocated cluster */
    u32	free_clust;		/* Number of free clusters */
    u32	fsi_sector;		/* fsinfo sector (FAT32) */
    #endif
    #if _FS_RPATH == 1
    u32	cdir;			/* Current directory start cluster (0:root) */
    #endif
    u32	n_fatent;		/* Number of FAT entries (= number of clusters + 2) */
    u32	fsize;			/* Sectors per FAT */
    u32	fatbase;		/* FAT start sector */
    u32	dirbase;		/* Root directory start sector (FAT32:Cluster#) */
    u32	database;		/* Data start sector */
    u32	winsect;		/* Current sector appearing in the win[] */
    u8	win[_MAX_SS];	/* Disk access window for Directory, FAT (and Data on tiny cfg) */
} FATFS;



/* File object structure (FIL) */

typedef struct {
    FATFS*	fs;				/* Pointer to the owner file system object */
    u16	id;				/* Owner file system mount ID */
    u8	flag;			/* File status flags */
    u8	pad1;
    u32	fptr;			/* File read/write pointer (0 on file open) */
    u32	fsize;			/* File size */
    u32	sclust;			/* File start cluster (0 when fsize==0) */
    u32	clust;			/* Current cluster */
    u32	dsect;			/* Current data sector */
    #if _FS_READONLY == 0
    u32	dir_sect;		/* Sector containing the directory entry */
    u8*	dir_ptr;		/* Ponter to the directory entry in the window */
    #endif
    #if _USE_FASTSEEK == 1
    u32*	cltbl;			/* Pointer to the cluster link map table (null on file open) */
    #endif
    #if _FS_SHARE == 1
    u16	lockid;			/* File lock ID (index of file semaphore table) */
    #endif
    #if _FS_TINY == 0
    u8	buf[_MAX_SS];	/* File data read/write buffer */
    #endif
} FIL;



/* Directory object structure (DIR) */

typedef struct {
    FATFS*	fs;				/* Pointer to the owner file system object */
    u16	id;				/* Owner file system mount ID */
    u16	index;			/* Current read/write index number */
    u32	sclust;			/* Table start cluster (0:Root dir) */
    u32	clust;			/* Current cluster */
    u32	sect;			/* Current sector */
    u8*	dir;			/* Pointer to the current SFN entry in the win[] */
    u8*	fn;				/* Pointer to the SFN (in/out) {file[8],ext[3],status[1]} */
    #if _USE_LFN == 1
    WCHAR*	lfn;			/* Pointer to the LFN working buffer */
    u16	lfn_idx;		/* Last matched LFN index number (0xFFFF:No LFN) */
    #endif
} DIR_t;



/* File status structure (FILINFO) */

typedef struct {
    u32	fsize;			/* File size */
    u16	fdate;			/* Last modified date */
    u16	ftime;			/* Last modified time */
    u8	fattrib;		/* Attribute */
    TCHAR	fname[13];		/* Short file name (8.3 format) */
    #if _USE_LFN == 1
    TCHAR*	lfname;			/* Pointer to the LFN buffer */
    u16 	lfsize;			/* Size of LFN buffer in TCHAR */
    #endif
} FILINFO;



/* File function return code (FRESULT) */

typedef enum {
    FR_OK = 0,				/* (0) Succeeded */
    FR_DISK_ERR,			/* (1) A hard error occured in the low level disk I/O layer */
    FR_INT_ERR,				/* (2) Assertion failed */
    FR_NOT_READY,			/* (3) The physical drive cannot work */
    FR_NO_FILE,				/* (4) Could not find the file */
    FR_NO_PATH,				/* (5) Could not find the path */
    FR_INVALID_NAME,		/* (6) The path name format is invalid */
    FR_DENIED,				/* (7) Acces denied due to prohibited access or directory full */
    FR_EXIST,				/* (8) Acces denied due to prohibited access */
    FR_INVALID_OBJECT,		/* (9) The file/directory object is invalid */
    FR_WRITE_PROTECTED,		/* (10) The physical drive is write protected */
    FR_INVALID_DRIVE,		/* (11) The logical drive number is invalid */
    FR_NOT_ENABLED,			/* (12) The volume has no work area */
    FR_NO_FILESYSTEM,		/* (13) There is no valid FAT volume */
    FR_MKFS_ABORTED,		/* (14) The f_mkfs() aborted due to any parameter error */
    FR_TIMEOUT,				/* (15) Could not get a grant to access the volume within defined period */
    FR_LOCKED,				/* (16) The operation is rejected according to the file shareing policy */
    FR_NOT_ENOUGH_CORE,		/* (17) LFN working buffer could not be allocated */
    FR_TOO_MANY_OPEN_FILES,	/* (18) Number of open files > _FS_SHARE */
    FR_INVALID_PARAMETER	/* (19) Given parameter is invalid */
} FRESULT;



/*--------------------------------------------------------------*/
/* FatFs module application interface                           */

FRESULT f_mount (u8, FATFS*);					/* Mount/Unmount a logical drive */
FRESULT f_open (u8, FIL*, const TCHAR*, u8);		/* Open or create a file */
FRESULT f_read (u8, FIL*, void*, u16, u16*);			/* Read data from a file */
FRESULT f_lseek (u8, FIL*, u32);						/* Move file pointer of a file object */
FRESULT f_close (u8, FIL*);								/* Close an open file object */
FRESULT f_opendir (u8, DIR_t*, const TCHAR*);				/* Open an existing directory */
FRESULT f_readdir (u8, DIR_t*, FILINFO*);					/* Read a directory item */
FRESULT f_stat (u8, const TCHAR*, FILINFO*);			/* Get file status */
FRESULT f_write (u8, FIL*, const void*, u16, u16*);	/* Write data to a file */
FRESULT f_getfree (u8, const TCHAR*, u32*, FATFS**);	/* Get number of free clusters on the drive */
FRESULT f_truncate (u8, FIL*);							/* Truncate file */
FRESULT f_sync (u8, FIL*);								/* Flush cached data of a writing file */
FRESULT f_unlink (u8, const TCHAR*);					/* Delete an existing file or directory */
FRESULT	f_mkdir (u8, const TCHAR*);						/* Create a new directory */
FRESULT f_chmod (u8, const TCHAR*, u8, u8);	/* Change attriburte of the file/dir */
FRESULT f_utime (u8, const TCHAR*, const FILINFO*);		/* Change timestamp of the file/dir */
FRESULT f_rename (u8, const TCHAR*, const TCHAR*);		/* Rename/Move a file or directory */
FRESULT f_chdrive (u8, u8);						/* Change current drive */
FRESULT f_chdir (u8, const TCHAR*);						/* Change current directory */
FRESULT f_getcwd (u8, TCHAR*, u16);					/* Get current directory */
FRESULT f_forward (u8, FIL*, u16(*)(const u8*,u16), u16, u16*);	/* Forward data to the stream */
FRESULT f_mkfs (u8, u8, u8, u16);			/* Create a file system on the drive */
FRESULT	f_fdisk (u8, u8, const u32[], void*);	/* Divide a physical drive into some partitions */
int f_putc (u8, TCHAR, FIL*);							/* Put a character to the file */
int f_puts (u8, const TCHAR*, FIL*);					/* Put a string to the file */
int f_printf (u8, FIL*, const TCHAR*, ...);				/* Put a formatted string to the file */
TCHAR* f_gets (u8, TCHAR*, int, FIL*);					/* Get a string from the file */

#define f_eof(fp) (((fp)->fptr == (fp)->fsize) ? 1 : 0)
#define f_error(fp) (((fp)->flag & FA__ERROR) ? 1 : 0)
#define f_tell(fp) ((fp)->fptr)
#define f_size(fp) ((fp)->fsize)

#ifndef EOF
#define EOF (-1)
#endif

/*--------------------------------------------------------------*/
/* Additional user defined functions                            */

/* RTC function */
#if _FS_READONLY == 0
u32 get_fattime (void);
#endif

/* Unicode support functions */
#if _USE_LFN == 1					/* Unicode - OEM code conversion */
WCHAR ff_convert (WCHAR, u16);		/* OEM-Unicode bidirectional conversion */
WCHAR ff_wtoupper (WCHAR);			/* Unicode upper-case conversion */
#if _USE_LFN == 3					/* Memory functions */
void* ff_memalloc (u16);			/* Allocate memory block */
void ff_memfree (void*);			/* Free memory block */
#endif
#endif

/*--------------------------------------------------------------*/
/* Flags and offset address                                     */


/* File access control and file status flags (FIL.flag) */

#define	FA_READ				0x01
#define	FA_OPEN_EXISTING	0x00
#define FA__ERROR			0x80

#if _FS_READONLY == 0
#define	FA_WRITE			0x02
#define	FA_CREATE_NEW		0x04
#define	FA_CREATE_ALWAYS	0x08
#define	FA_OPEN_ALWAYS		0x10
#define FA__WRITTEN			0x20
#define FA__DIRTY			0x40
#endif


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
#define AM_MASK	0x3F	/* Mask of defined bits */


/* Fast seek feature */
#define CREATE_LINKMAP	0xFFFFFFFF



/*--------------------------------*/
/* Multi-byte word access macros  */

#if _WORD_ACCESS == 1	/* Enable word access to the FAT structure */

#define	LD_WORD(ptr)		(u16)(*(u16*)(u8*)(ptr))
#define	LD_DWORD(ptr)		(u32)(*(u32*)(u8*)(ptr))
#define	ST_WORD(ptr,val)	*(u16*)(u8*)(ptr)=(u16)(val)
#define	ST_DWORD(ptr,val)	*(u32*)(u8*)(ptr)=(u32)(val)

#else					/* Use byte-by-byte access to the FAT structure */

#define	LD_WORD(ptr)		(u16)(((u16)*((u8*)(ptr)+1)<<8)|(u16)*(u8*)(ptr))
#define	LD_DWORD(ptr)		(u32)(((u32)*((u8*)(ptr)+3)<<24)|((u32)*((u8*)(ptr)+2)<<16)|((u16)*((u8*)(ptr)+1)<<8)|*(u8*)(ptr))
#define	ST_WORD(ptr,val)	*(u8*)(ptr)=(u8)(val); *((u8*)(ptr)+1)=(u8)((u16)(val)>>8)
#define	ST_DWORD(ptr,val)	*(u8*)(ptr)=(u8)(val); *((u8*)(ptr)+1)=(u8)((u16)(val)>>8); *((u8*)(ptr)+2)=(u8)((u32)(val)>>16); *((u8*)(ptr)+3)=(u8)((u32)(val)>>24)

#endif

#ifdef __cplusplus
}
#endif

#endif /* _FATFS */
