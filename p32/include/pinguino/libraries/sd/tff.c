/*---------------------------------------------------------------------/
/  tff.c - Tiny FAT File system module R0.06 (C)ChaN, 2008
/----------------------------------------------------------------------/
/ The FatFs module is an experimenal project to implement FAT file system to
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
/----------------------------------------------------------------------/
/ Feb 26,'06 R0.00  Prototype.
/
/ Apr 29,'06 R0.01  First stable version.
/
/ Jun 01,'06 R0.02  Added FAT12 support.
/                   Removed unbuffered mode.
/                   Fixed a problem on small (<32M) patition.
/ Jun 10,'06 R0.02a Added a configuration option (_FS_MINIMUM).
/
/ Sep 22,'06 R0.03  Added f_rename().
/                   Changed option _FS_MINIMUM to _FS_MINIMIZE.
/ Dec 09,'06 R0.03a Improved cluster scan algolithm to write files fast.
/
/ Feb 04,'07 R0.04  Added FAT32 supprt.
/                   Changed some interfaces incidental to FatFs.
/                   Changed f_mountdrv() to f_mount().
/ Apr 01,'07 R0.04a Added a capability of extending file size to f_lseek().
/                   Added minimization level 3.
/                   Fixed a problem in FAT32 support.
/ May 05,'07 R0.04b Added a configuration option _USE_NTFLAG.
/                   Added FSInfo support.
/                   Fixed some problems corresponds to FAT32 support.
/                   Fixed DBCS name can result FR_INVALID_NAME.
/                   Fixed short seek (<= csize) collapses the file object.
/
/ Aug 25,'07 R0.05  Changed arguments of f_read() and f_write().
/ Feb 03,'08 R0.05a Added f_truncate() and f_utime().
/                   Fixed off by one error at FAT sub-type determination.
/                   Fixed btr in f_read() can be mistruncated.
/                   Fixed cached sector is not flushed when create and close
/                   without write.
/
/ Apr 01,'08 R0.06  Added f_forward(), f_putc(), f_puts(), f_printf() and f_gets().
/                   Improved performance of f_lseek() on moving to the same
/                   or following cluster.
/---------------------------------------------------------------------*/

#ifndef _TFF_C
#define _TFF_C

#include <string.h>         // memcmp(), memcpy()
#include <typedef.h>        // u8, u16, u32
#include <sd/ffconf.h>      // SD lib. config. file
#include <sd/tff.h>         // Tiny-FatFs declarations
#include <sd/diskio.h>      // Include file for user provided disk functions

// Printf
#if !_FS_READONLY
    #ifdef SDPRINTF
        #include <stdarg.h>
        #include <printFormated.c>     // #include <stdio.c>
    #endif
#endif

FATFS FAT;                  // File system object
FATFS *pFS = &FAT;         // Pointer on File system object
static word fsid;           // File system mount ID
extern volatile u8 Stat;    // Disk status

//FRESULT res;
//DIR_t dj;
//char fn[12];
//u8 *dir;

/*  --------------------------------------------------------------------

    Module Private Functions

    ------------------------------------------------------------------*/


/*  --------------------------------------------------------------------
    Change window offset
    spi: spi module
    sector: Sector number to make apperance in the pFAT->win
    returns: TRUE: successful, FALSE: failed
    NB : Move to zero only writes back dirty window
    ------------------------------------------------------------------*/

u8 move_window(u8 spi, dword sector)
{
    dword wsect;
    //FATFS *pFS = pFAT;

    wsect = pFS->winsect;
    /* Changed current window */
    if (wsect != sector)
    {
        #if !_FS_READONLY
        u8 n;
        /* Write back dirty window if needed */
        if (pFS->winflag)
        {
            if (disk_writesector(spi, 0, pFS->win, wsect, 1) != RES_OK)
                return FALSE;
            pFS->winflag = 0;
            /* In FAT area */
            if (wsect < (pFS->fatbase + pFS->sects_fat))
            {
                /* Refrect the change to all FAT copies */
                for (n = pFS->n_fats; n >= 2; n--)
                {
                    wsect += pFS->sects_fat;
                    disk_writesector(spi, 0, pFS->win, wsect, 1);
                }
            }
        }
        #endif
        if (sector)
        {
            if (disk_readsector(spi, 0, pFS->win, sector, 1) != RES_OK)
                return FALSE;
            pFS->winsect = sector;
        }
    }
    return TRUE;
}

/*-----------------------------------------------------------------------*/
/* Clean-up cached data                                                  */
/* FR_OK: successful, FR_RW_ERROR: failed                                */
/*-----------------------------------------------------------------------*/

#if !_FS_READONLY

#if defined(SDSYNC)  || defined(SDCLOSE) || defined(SDUNLINK) || \
    defined(SDMKDIR) || defined(SDCHMOD) || defined(SDUTIME)  || \
    defined(SDRENAME)

static FRESULT sync(u8 spi)
{
    //FATFS *pFS = pFAT;

    pFS->winflag = 1;
    if (!move_window(spi, 0))
        return FR_RW_ERROR;
        
    #if _USE_FSINFO
    /* Update FSInfo sector if needed */
    if (pFS->fs_type == FS_FAT32 && pFS->fsi_flag)
    {
        pFS->winsect = 0;
        memset(pFS->win, 0, 512U);
        ST_WORD(&pFS->win[BS_55AA], 0xAA55);
        ST_DWORD(&pFS->win[FSI_LeadSig], 0x41615252);
        ST_DWORD(&pFS->win[FSI_StrucSig], 0x61417272);
        ST_DWORD(&pFS->win[FSI_Free_Count], pFS->free_clust);
        ST_DWORD(&pFS->win[FSI_Nxt_Free], pFS->last_clust);
        disk_writesector(spi, 0, pFS->win, pFS->fsi_sector, 1);
        pFS->fsi_flag = 0;
    }
    #endif
    
    /* Make sure that no pending write process in the physical drive */
    if (disk_ioctl(spi, 0, CTRL_SYNC, NULL) != RES_OK)
        return FR_RW_ERROR;

    return FR_OK;
}
#endif

#endif // !_FS_READONLY

/*-----------------------------------------------------------------------*/
/* Get a cluster status                                                  */
/*-----------------------------------------------------------------------*/

static
CLUST get_cluster (	/* 0,>=2: successful, 1: failed */
    u8 spi,     /* spi module */
    CLUST clust		/* Cluster# to get the link information */
)
{
    word wc, bc;
    dword fatsect;
    //FATFS *pFS = pFAT;


    /* Valid cluster# */
    if (clust >= 2 && clust < pFS->max_clust)
    {
        fatsect = pFS->fatbase;
        switch (pFS->fs_type)
        {
            case FS_FAT12 :
                bc = (word)clust * 3 / 2;
                if (!move_window(spi, fatsect + bc / 512U)) break;
                wc = pFS->win[bc % 512U]; bc++;
                if (!move_window(spi, fatsect + bc / 512U)) break;
                wc |= (word)pFS->win[bc % 512U] << 8;
                return (clust & 1) ? (wc >> 4) : (wc & 0xFFF);

            case FS_FAT16 :
                if (!move_window(spi, fatsect + clust / 256)) break;
                return LD_WORD(&pFS->win[((word)clust * 2) % 512U]);

            #if _FAT32
            case FS_FAT32 :
                if (!move_window(spi, fatsect + clust / 128)) break;
                return LD_DWORD(&pFS->win[((word)clust * 4) % 512U]) & 0x0FFFFFFF;
            #endif
        }
    }

    return 1;	/* Out of cluster range, or an error occured */
}

/*-----------------------------------------------------------------------*/
/* Change a cluster status                                               */
/*-----------------------------------------------------------------------*/

#if !_FS_READONLY
static
BOOL put_cluster (	/* TRUE: successful, FALSE: failed */
    u8 spi,     /* spi module */
    CLUST clust,	/* Cluster# to change (must be 2 to pFS->max_clust-1) */
    CLUST val		/* New value to mark the cluster */
)
{
    word bc;
    u8 *p;
    dword fatsect;
    //FATFS *pFS = pFAT;


    fatsect = pFS->fatbase;
    switch (pFS->fs_type) {
    case FS_FAT12 :
        bc = (word)clust * 3 / 2;
        if (!move_window(spi, fatsect + bc / 512U)) return FALSE;
        p = &pFS->win[bc % 512U];
        *p = (clust & 1) ? ((*p & 0x0F) | ((u8)val << 4)) : (u8)val;
        bc++;
        pFS->winflag = 1;
        if (!move_window(spi, fatsect + bc / 512U)) return FALSE;
        p = &pFS->win[bc % 512U];
        *p = (clust & 1) ? (u8)(val >> 4) : ((*p & 0xF0) | ((u8)(val >> 8) & 0x0F));
        break;

    case FS_FAT16 :
        if (!move_window(spi, fatsect + clust / 256)) return FALSE;
        ST_WORD(&pFS->win[((word)clust * 2) % 512U], (word)val);
        break;
#if _FAT32
    case FS_FAT32 :
        if (!move_window(spi, fatsect + clust / 128)) return FALSE;
        ST_DWORD(&pFS->win[((word)clust * 4) % 512U], val);
        break;
#endif
    default :
        return FALSE;
    }
    pFS->winflag = 1;
    return TRUE;
}
#endif /* !_FS_READONLY */

/*-----------------------------------------------------------------------*/
/* Remove a cluster chain                                                */
/*-----------------------------------------------------------------------*/

#if !_FS_READONLY
static
BOOL remove_chain (	/* TRUE: successful, FALSE: failed */
    u8 spi,     /* spi module */
    CLUST clust		/* Cluster# to remove chain from */
)
{
    CLUST nxt;
    //FATFS *pFS = pFAT;


    while (clust >= 2 && clust < pFS->max_clust)
    {
        nxt = get_cluster(spi, clust);
        if (nxt == 1) return FALSE;
        if (!put_cluster(spi, clust, 0)) return FALSE;
        if (pFS->free_clust != (CLUST)0xFFFFFFFF)
        {
            //pFS->free_clust++;
            // XC8 : registers unavailable for code generation of this expression
            pFS->free_clust = pFS->free_clust + 1;
            #if _USE_FSINFO
            pFS->fsi_flag = 1;
            #endif
        }
        clust = nxt;
    }
    return TRUE;
}
#endif

/*-----------------------------------------------------------------------*/
/* Stretch or create a cluster chain                                     */
/*-----------------------------------------------------------------------*/

#if !_FS_READONLY
static
CLUST create_chain (	/* 0: No free cluster, 1: Error, >=2: New cluster number */
    u8 spi,     /* spi module */
    CLUST clust			/* Cluster# to stretch, 0 means create new */
)
{
    CLUST cstat, ncl, scl, mcl;
    //FATFS *pFS = pFAT;

    mcl = pFS->max_clust;
    /* Create new chain */
    if (clust == 0)
    {
        /* Get last allocated cluster */
        scl = pFS->last_clust;
        if (scl < 2 || scl >= mcl)
            scl = 1;
    }
    /* Stretch existing chain */
    else
    {
        cstat = get_cluster(spi, clust);/* Check the cluster status */
        if (cstat < 2) return 1;		/* It is an invalid cluster */
        if (cstat < mcl) return cstat;	/* It is already followed by next cluster */
        scl = clust;
    }

    ncl = scl;				/* Start cluster */
    for (;;)
    {
        ncl++;							/* Next cluster */
        if (ncl >= mcl)
        {				/* Wrap around */
            ncl = 2;
            if (ncl > scl) return 0;	/* No free custer */
        }
        cstat = get_cluster(spi, ncl);	/* Get the cluster status */
        if (cstat == 0) break;			/* Found a free cluster */
        if (cstat == 1) return 1;		/* Any error occured */
        if (ncl == scl) return 0;		/* No free custer */
    }

    if (!put_cluster(spi, ncl, (CLUST)0x0FFFFFFF)) return 1;/* Mark the new cluster "in use" */
    if (clust != 0 && !put_cluster(spi, clust, ncl)) return 1;	/* Link it to previous one if needed */

    /* Update fsinfo */
    pFS->last_clust = ncl;
    if (pFS->free_clust != (CLUST)0xFFFFFFFF)
    {
        //pFS->free_clust--;
        // XC8 : registers unavailable for code generation of this expression
        pFS->free_clust = pFS->free_clust - 1;
        #if _USE_FSINFO
        pFS->fsi_flag = 1;
        #endif
    }

    return ncl;		/* Return new cluster number */
}
#endif /* !_FS_READONLY */

/*-----------------------------------------------------------------------*/
/* Get sector# from cluster#                                             */
/*-----------------------------------------------------------------------*/

static
dword clust2sect (	/* !=0: sector number, 0: failed - invalid cluster# */
    CLUST clust		/* Cluster# to be converted */
)
{
    //FATFS *pFS = pFAT;

    clust -= 2;
    if (clust >= (pFS->max_clust - 2))
        return 0;		/* Invalid cluster# */
    return (dword)clust * pFS->csize + pFS->database;
}

/*-----------------------------------------------------------------------*/
/* Move directory pointer to next                                        */
/*-----------------------------------------------------------------------*/

static
BOOL next_dir_entry (	/* TRUE: successful, FALSE: could not move next */
    u8 spi,     /* spi module */
    DIR_t *dj				/* Pointer to directory object */
)
{
    CLUST clust;
    word idx;


    idx = dj->index + 1;
    if ((idx & 15) == 0) {		/* Table sector changed? */
        dj->sect++;				/* Next sector */
        if (dj->clust == 0) {	/* In static table */
            if (idx >= dj->fs->n_rootdir) return FALSE;	/* Reached to end of table */
        } else {				/* In dynamic table */
            if (((idx / 16) & (dj->fs->csize - 1)) == 0) {	/* Cluster changed? */
                clust = get_cluster(spi, dj->clust);		/* Get next cluster */
                if (clust < 2 || clust >= dj->fs->max_clust)	/* Reached to end of table */
                    return FALSE;
                dj->clust = clust;				/* Initialize for new cluster */
                dj->sect = clust2sect(clust);
            }
        }
    }
    dj->index = idx;	/* Lower 4 bit of dj->index indicates offset in dj->sect */
    return TRUE;
}

/*-----------------------------------------------------------------------*/
/* Get file status from directory entry                                  */
/*-----------------------------------------------------------------------*/

#if _FS_MINIMIZE <= 1

#if defined(SDREADDIR) || defined(SDSTAT)
static
void get_fileinfo (	/* No return code */
    FILINFO *finfo, /* Ptr to store the File Information */
    const u8 *dir	/* Ptr to the directory entry */
)
{
    u8 n, c, a;
    char *p;


    p = &finfo->fname[0];
    a = _USE_NTFLAG ? dir[DIR_NTres] : 0;	/* NT flag */
    for (n = 0; n < 8; n++) {	/* Convert file name (body) */
        c = dir[n];
        if (c == ' ') break;
        if (c == 0x05) c = 0xE5;
        if (a & 0x08 && c >= 'A' && c <= 'Z') c += 0x20;
        *p++ = c;
    }
    if (dir[8] != ' ') {		/* Convert file name (extension) */
        *p++ = '.';
        for (n = 8; n < 11; n++) {
            c = dir[n];
            if (c == ' ') break;
            if (a & 0x10 && c >= 'A' && c <= 'Z') c += 0x20;
            *p++ = c;
        }
    }
    *p = '\0';

    finfo->fattrib = dir[DIR_Attr];			/* Attribute */
    finfo->fsize = LD_DWORD(&dir[DIR_FileSize]);	/* Size */
    finfo->fdate = LD_WORD(&dir[DIR_WrtDate]);	/* Date */
    finfo->ftime = LD_WORD(&dir[DIR_WrtTime]);	/* Time */
}
#endif
#endif /* _FS_MINIMIZE <= 1 */

/*-----------------------------------------------------------------------*/
/* Pick a paragraph and create the name in format of directory entry     */
/*-----------------------------------------------------------------------*/

static
char make_dirfile (		/* 1: error - detected an invalid format, '\0'or'/': next character */
    const char **path,	/* Pointer to the file path pointer */
    char *dirname		/* Pointer to directory name buffer {Name(8), Ext(3), NT flag(1)} */
)
{
    u8 n, t, c, a, b;


    memset((void *)dirname, ' ', 8+3);	/* Fill buffer with spaces */
    a = 0; b = 0x18;	/* NT flag */
    n = 0; t = 8;
    for (;;) {
        c = *(*path)++;
        if (c == '\0' || c == '/') {		/* Reached to end of str or directory separator */
            if (n == 0) break;
            dirname[11] = _USE_NTFLAG ? (a & b) : 0;
            return c;
        }
        if (c <= ' ' || c == 0x7F) break;		/* Reject invisible chars */
        if (c == '.') {
            if (!(a & 1) && n >= 1 && n <= 8) {	/* Enter extension part */
                n = 8; t = 11; continue;
            }
            break;
        }
        if (_USE_SJIS &&
            ((c >= 0x81 && c <= 0x9F) ||		/* Accept S-JIS code */
            (c >= 0xE0 && c <= 0xFC))) {
            if (n == 0 && c == 0xE5)		/* Change heading \xE5 to \x05 */
                c = 0x05;
            a ^= 1; goto md_l2;
        }
        if (c == '"') break;				/* Reject " */
        if (c <= ')') goto md_l1;			/* Accept ! # $ % & ' ( ) */
        if (c <= ',') break;				/* Reject * + , */
        if (c <= '9') goto md_l1;			/* Accept - 0-9 */
        if (c <= '?') break;				/* Reject : ; < = > ? */
        if (!(a & 1)) {	/* These checks are not applied to S-JIS 2nd byte */
            if (c == '|') break;			/* Reject | */
            if (c >= '[' && c <= ']') break;/* Reject [ \ ] */
            if (_USE_NTFLAG && c >= 'A' && c <= 'Z')
                (t == 8) ? (b &= 0xF7) : (b &= 0xEF);
            if (c >= 'a' && c <= 'z') {		/* Convert to upper case */
                c -= 0x20;
                if (_USE_NTFLAG) (t == 8) ? (a |= 0x08) : (a |= 0x10);
            }
        }
    md_l1:
        a &= 0xFE;
    md_l2:
        if (n >= t) break;
        dirname[n++] = c;
    }
    return 1;
}

/*-----------------------------------------------------------------------*/
/* Trace a file path                                                     */
/*-----------------------------------------------------------------------*/

static
FRESULT trace_path (	/* FR_OK(0): successful, !=0: error code */
    u8 spi,     /* spi module */
    DIR_t *dj,			/* Pointer to directory object to return last directory */
    char *fn,			/* Pointer to last segment name to return */
    const char *path,	/* Full-path string to trace a file or directory */
    u8 **dir			/* Pointer to pointer to found entry to retutn */
)
{
    CLUST clust;
    char ds;
    u8 *dptr = NULL;
    //FATFS *pFS = pFAT;

    /* Initialize directory object */
    dj->fs = pFS;
    clust = pFS->dirbase;
    #if _FAT32
    if (pFS->fs_type == FS_FAT32)
    {
        dj->clust = dj->sclust = clust;
        dj->sect = clust2sect(clust);
    }
    else
    #endif
    {
        dj->clust = dj->sclust = 0;
        dj->sect = clust;
    }
    dj->index = 0;

    if (*path == '\0') {							/* Null path means the root directory */
        *dir = NULL; return FR_OK;
    }

    for (;;) {
        ds = make_dirfile(&path, fn);					/* Get a paragraph into fn[] */
        if (ds == 1) return FR_INVALID_NAME;
        for (;;) {
            if (!move_window(spi, dj->sect)) return FR_RW_ERROR;
            dptr = &pFS->win[(dj->index & 15) * 32];		/* Pointer to the directory entry */
            if (dptr[DIR_Name] == 0)					/* Has it reached to end of dir? */
                return !ds ? FR_NO_FILE : FR_NO_PATH;
            if (dptr[DIR_Name] != 0xE5					/* Matched? */
                && !(dptr[DIR_Attr] & AM_VOL)
                && !memcmp(&dptr[DIR_Name], fn, 8+3) ) break;
            if (!next_dir_entry(spi, dj))					/* Next directory pointer */
                return !ds ? FR_NO_FILE : FR_NO_PATH;
        }
        if (!ds) { *dir = dptr; return FR_OK; }			/* Matched with end of path */
        if (!(dptr[DIR_Attr] & AM_DIR)) return FR_NO_PATH;	/* Cannot trace because it is a file */
        clust =											/* Get cluster# of the directory */
        #if _FAT32
            ((dword)LD_WORD(&dptr[DIR_FstClusHI]) << 16) |
        #endif
            LD_WORD(&dptr[DIR_FstClusLO]);
        dj->clust = dj->sclust = clust;				/* Restart scannig with the new directory */
        dj->sect = clust2sect(clust);
        dj->index = 2;
    }
}

/*-----------------------------------------------------------------------*/
/* Reserve a directory entry                                             */
/*-----------------------------------------------------------------------*/

#if !_FS_READONLY
static
FRESULT reserve_direntry (	/* FR_OK: successful, FR_DENIED: no free entry, FR_RW_ERROR: a disk error occured */
    u8 spi,     /* spi module */
    DIR_t *dj,				/* Target directory to create new entry */
    u8 **dir				/* Pointer to pointer to created entry to return */
)
{
    CLUST clust;
    dword sector;
    u8 c, n, *dptr;
    FATFS *pFS = dj->fs;


    /* Re-initialize directory object */
    clust = dj->sclust;
    if (clust != 0)
    {	/* Dyanmic directory table */
        dj->clust = clust;
        dj->sect = clust2sect(clust);
    }
    else
    {			/* Static directory table */
        dj->sect = pFS->dirbase;
    }
    dj->index = 0;

    do {
        if (!move_window(spi, dj->sect)) return FR_RW_ERROR;
        dptr = &pFS->win[(dj->index & 15) * 32];	/* Pointer to the directory entry */
        c = dptr[DIR_Name];
        if (c == 0 || c == 0xE5) {			/* Found an empty entry! */
            *dir = dptr; return FR_OK;
        }
    } while (next_dir_entry(spi, dj));			/* Next directory pointer */
    /* Reached to end of the directory table */

    /* Abort when static table or could not stretch dynamic table */
    if (clust == 0 || !(clust = create_chain(spi, dj->clust))) return FR_DENIED;
    if (clust == 1 || !move_window(spi, 0)) return FR_RW_ERROR;

    pFS->winsect = sector = clust2sect(clust);	/* Cleanup the expanded table */
    memset((void *)pFS->win, 0, 512U);
    for (n = pFS->csize; n; n--) {
        if (disk_writesector(spi, 0, pFS->win, sector, 1) != RES_OK)
            return FR_RW_ERROR;
        sector++;
    }
    pFS->winflag = 1;
    *dir = pFS->win;
    return FR_OK;
}
#endif /* !_FS_READONLY */

/*  --------------------------------------------------------------------
    Load boot record and check if it's an FAT boot record
    spi     SPI# module
    sect    Sector# to check if it is an FAT boot record or not
    Returns :
    * 0:The FAT boot record
    * 1:Valid boot record but not an FAT
    * 2:Not a boot record or error
    ------------------------------------------------------------------*/

//#include <ctype.c>

static u8 check_fs(u8 spi, dword sect)
{
    //FATFS *pFS = pFAT;
    #ifdef __DEBUG__
    debug("Checking sector %d ...", sect);
    #endif

    // Load boot record
    if (disk_readsector(spi, 0, pFS->win, sect, 1) != RES_OK)
    {
        #ifdef __DEBUG__
        debug("ERROR SECTOR %d", sect);
        #endif
        return 2;
    }

    // Check record signature
    #ifdef __DEBUG__
    debug("Record signature=0x%X", LD_WORD(&pFS->win[BS_55AA]));
    #endif
    if (LD_WORD(&pFS->win[BS_55AA]) != 0xAA55)
        return 2;

    // Check FAT signature
    if (!memcmp(&pFS->win[BS_FilSysType], "FAT", 3))
        return 0;
    #ifdef __DEBUG__
    debug("FAT signature OK");
    #endif

    #if _FAT32
    // Check FAT32 signature
    if (!memcmp(&pFS->win[BS_FilSysType32], "FAT32", 5) && !(pFS->win[BPB_ExtFlags] & 0x80))
        return 0;
    #ifdef __DEBUG__
    debug("FAT32 signature OK");
    #endif
    #endif
    
    return 1;
}

/*  --------------------------------------------------------------------
    Make sure that the file system is valid
    u8 spi,             spi module
    const char **path,  Pointer to pointer to the path name (drive number)
    u8 chk_wp           !=0: Check media write protection for write access
    return              FR_OK(0): successful, !=0: any error occured
    ------------------------------------------------------------------*/

FRESULT auto_mount(u8 spi, const char **path, u8 chk_wp)
{
    u8 fmt, stat = 0;
    dword bootsect, fatsize, totalsect, maxclust;
    const char *p = *path;
    //FATFS* pFS = pFAT;
    
    #ifdef __DEBUG__
    debug("Disk mounting");
    #endif
    
    // Strip leading spaces
    //while (*p == ' ') p++;
    // Strip heading slash
    //if (*p == '/') p++;
    // Return pointer to the path name
    //*path = p;

    // Is the file system object registered?
    if (!pFS) return FR_NOT_ENABLED;

    // Check if the logical drive has been mounted
    // -----------------------------------------------------------------

    if (pFS->fs_type)
    {
        // and physical drive is kept initialized (has not been changed),
        if (!(Stat & STA_NOINIT))
        {
            #if !_FS_READONLY
            // Check write protection if needed
            if (chk_wp && (stat & STA_PROTECT))
                return FR_WRITE_PROTECTED;
            #endif
            // The file system object is valid
            return FR_OK;
        }
    }

    // Mount the logical drive
    // -----------------------------------------------------------------

    // Clean-up the file system object
    memset((void *)pFS, 0, sizeof(FATFS));

    // Initialize low level disk I/O layer
    stat = disk_initialize(spi, 0);

    // Check if the drive is ready
    if (stat & STA_NOINIT)
        return FR_NOT_READY;

    // Check write protection if needed
    #if !_FS_READONLY
    if (chk_wp && (stat & STA_PROTECT))
        return FR_WRITE_PROTECTED;
    #endif

    // Search FAT partition on the drive
    // -----------------------------------------------------------------

    // Check sector 0 as an SFD format
    bootsect = 0;
    fmt = check_fs(spi, bootsect);
    #ifdef __DEBUG__
    debug("fmt=%d", fmt);
    #endif
    
    // Not a FAT boot record, it may be patitioned
    if (fmt == 1)
    {
        // Check if the partition listed in top of the partition table exists
        if (pFS->win[MBR_Table+4])
        {
            // Partition offset in LBA
            bootsect = LD_DWORD(&pFS->win[MBR_Table+8]);
            #ifdef __DEBUG__
            debug("bootsect=%d", bootsect);
            #endif
            // Check the partition
            fmt = check_fs(spi, bootsect);
        }
    }

    // Not a valid FAT patition
    if (fmt || LD_WORD(&pFS->win[BPB_BytsPerSec]) != 512U)
    {
        #ifdef __DEBUG__
        debug("Invalid FAT patition");
        #endif
        return FR_NO_FILESYSTEM;
    }
    
    //
    // Initialize the file system object
    //
    
    // Number of sectors per FAT
    fatsize = LD_WORD(&pFS->win[BPB_FATSz16]);
    if (!fatsize)
        fatsize = LD_DWORD(&pFS->win[BPB_FATSz32]);
    pFS->sects_fat = (CLUST)fatsize;
    // Number of FAT copies
    pFS->n_fats = pFS->win[BPB_NumFATs];
    // Number of sectors in FAT area
    fatsize *= pFS->n_fats;
    // FAT start sector (lba)
    pFS->fatbase = bootsect + LD_WORD(&pFS->win[BPB_RsvdSecCnt]);
    // Number of sectors per cluster
    pFS->csize = pFS->win[BPB_SecPerClus];
    //Nmuber of root directory entries
    pFS->n_rootdir = LD_WORD(&pFS->win[BPB_RootEntCnt]);
    // Number of sectors on the file system
    totalsect = LD_WORD(&pFS->win[BPB_TotSec16]);
    if (!totalsect)
        totalsect = LD_DWORD(&pFS->win[BPB_TotSec32]);

    // max_clust = Last cluster# + 1
    maxclust = (totalsect - LD_WORD(&pFS->win[BPB_RsvdSecCnt]) - fatsize - pFS->n_rootdir / 16) / pFS->csize + 2;
    pFS->max_clust = maxclust;

    // Determine the FAT sub type
    fmt = FS_FAT12;
    if (maxclust >= 0xFF7)
        fmt = FS_FAT16;
    if (maxclust >= 0xFFF7)
    #if !_FAT32
        return FR_NO_FILESYSTEM;
    #else
        fmt = FS_FAT32;

    // Root directory start cluster
    if (fmt == FS_FAT32)
        pFS->dirbase = LD_DWORD(&pFS->win[BPB_RootClus]);
    // Root directory start sector (lba)
    else
    #endif
        pFS->dirbase = pFS->fatbase + fatsize;
    // Data start sector (lba)
    pFS->database = pFS->fatbase + fatsize + pFS->n_rootdir / 16;

    #if !_FS_READONLY
    // Initialize allocation information
    pFS->free_clust = (CLUST)0xFFFFFFFF;
    #if _USE_FSINFO
    // Get fsinfo if needed
    if (fmt == FS_FAT32)
    {
        pFS->fsi_sector = bootsect + LD_WORD(&pFS->win[BPB_FSInfo]);
        if (disk_readsector(spi, 0, pFS->win, pFS->fsi_sector, 1) == RES_OK &&
            LD_WORD(&pFS->win[BS_55AA]) == 0xAA55 &&
            LD_DWORD(&pFS->win[FSI_LeadSig]) == 0x41615252 &&
            LD_DWORD(&pFS->win[FSI_StrucSig]) == 0x61417272)
        {
            pFS->last_clust = LD_DWORD(&pFS->win[FSI_Nxt_Free]);
            pFS->free_clust = LD_DWORD(&pFS->win[FSI_Free_Count]);
        }
    }
    #endif // _USE_FSINFO
    #endif // !_FS_READONLY

    // FAT syb-type
    pFS->fs_type = fmt;
    // File system mount ID
    pFS->id = ++fsid;
    return FR_OK;
}

/*-----------------------------------------------------------------------*/
/* Check if the file/dir object is valid or not                          */
/*-----------------------------------------------------------------------*/

static
FRESULT validate (		/* FR_OK(0): The object is valid, !=0: Invalid */
    const FATFS *pFS,	/* Pointer to the file system object */
    word id				/* Member id of the target object to be checked */
)
{
    if (!pFS || !pFS->fs_type || pFS->id != id)
        return FR_INVALID_OBJECT;
    if (Stat & STA_NOINIT)
        return FR_NOT_READY;

    return FR_OK;
}

/*--------------------------------------------------------------------------

   Public Functions

--------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Mount/Unmount a Locical Drive                                         */
/* u8 drv     Logical drive number to be mounted/unmounted             */
/* FATFS *pFS    Pointer to new file system object (NULL for unmount)     */
/*-----------------------------------------------------------------------*/

//#if defined(SDMOUNT) || defined(SDUNMOUNT)
FRESULT f_mount(u8 drv)
{
    if (drv) return FR_INVALID_DRIVE;

    // Clean-up the file system object
    memset((void *)pFS, 0, sizeof(FATFS));

    return FR_OK;
}
//#endif

/*-----------------------------------------------------------------------*/
/* Open or Create a File                                                 */
/*-----------------------------------------------------------------------*/

#ifdef SDOPEN
FRESULT f_open (
    u8 spi,     /* spi module */
    FIL *pFILE,			/* Pointer to the blank file object */
    const char *path,	/* Pointer to the file name */
    u8 mode			/* Access mode and file open mode flags */
)
{
    FRESULT res;
    DIR_t dj;
    u8 *dir;
    char fn[8+3+1];


    pFILE->fs = NULL;		/* Clear file object */
    #if !_FS_READONLY
    mode &= (FA_READ|FA_WRITE|FA_CREATE_ALWAYS|FA_OPEN_ALWAYS|FA_CREATE_NEW);
    res = auto_mount(spi, &path, (u8)(mode & (FA_WRITE|FA_CREATE_ALWAYS|FA_OPEN_ALWAYS|FA_CREATE_NEW)));
    #else
    mode &= FA_READ;
    res = auto_mount(spi, &path, FA_OPEN_EXISTING); // 0
    #endif
    if (res != FR_OK) return res;
    res = trace_path(spi, &dj, fn, path, &dir);	/* Trace the file path */

    #if !_FS_READONLY
    /* Create or Open a File */
    if (mode & (FA_CREATE_ALWAYS|FA_OPEN_ALWAYS|FA_CREATE_NEW))
    {
        CLUST rs;
        dword dw;
        /* No file, create new */
        if (res != FR_OK)
        {
            if (res != FR_NO_FILE) return res;
            res = reserve_direntry(spi, &dj, &dir);
            if (res != FR_OK) return res;
            memset((void *)dir, 0, 32);		/* Initialize the new entry with open name */
            memcpy(&dir[DIR_Name], fn, 8+3);
            dir[DIR_NTres] = fn[11];
            mode |= FA_CREATE_ALWAYS;
        }
        /* Any object is already existing */
        else
        {
            if (mode & FA_CREATE_NEW)			/* Cannot create new */
                return FR_EXIST;
            if (!dir || (dir[DIR_Attr] & (AM_RDO|AM_DIR)))	/* Cannot overwrite (R/O or DIR_t) */
                return FR_DENIED;
            if (mode & FA_CREATE_ALWAYS)
            {
                /* Resize it to zero */
                #if _FAT32
                rs = ((dword)LD_WORD(&dir[DIR_FstClusHI]) << 16) | LD_WORD(&dir[DIR_FstClusLO]);
                ST_WORD(&dir[DIR_FstClusHI], 0);
                #else
                rs = LD_WORD(&dir[DIR_FstClusLO]);
                #endif
                ST_WORD(&dir[DIR_FstClusLO], 0);	/* cluster = 0 */
                ST_DWORD(&dir[DIR_FileSize], 0);	/* size = 0 */
                dj.fs->winflag = 1;
                dw = dj.fs->winsect;			/* Remove the cluster chain */
                if (!remove_chain(spi, rs) || !move_window(spi, dw))
                    return FR_RW_ERROR;
                dj.fs->last_clust = rs - 1;		/* Reuse the cluster hole */
            }
        }
        if (mode & FA_CREATE_ALWAYS)
        {
            dir[DIR_Attr] = 0;					/* Reset attribute */
            dw = get_fattime();
            ST_DWORD(&dir[DIR_CrtTime], dw);	/* Created time */
            dj.fs->winflag = 1;
            mode |= FA__WRITTEN;				/* Set file changed flag */
        }
    }
    /* Open an existing file */
    else
    {
    #endif /* !_FS_READONLY */
        if (res != FR_OK) return res;			/* Trace failed */
        if (!dir || (dir[DIR_Attr] & AM_DIR))	/* It is a directory */
            return FR_NO_FILE;
        #if !_FS_READONLY
        if ((mode & FA_WRITE) && (dir[DIR_Attr] & AM_RDO)) /* R/O violation */
            return FR_DENIED;
    }
    pFILE->dir_sect = dj.fs->winsect;		/* Pointer to the directory entry */
    pFILE->dir_ptr = dir;
        #endif
    pFILE->flag = mode;					/* File access mode */
    pFILE->org_clust =						/* File start cluster */
        #if _FAT32
        ((dword)LD_WORD(&dir[DIR_FstClusHI]) << 16) |
        #endif
        LD_WORD(&dir[DIR_FstClusLO]);
    pFILE->fsize = LD_DWORD(&dir[DIR_FileSize]);	/* File size */
    pFILE->fptr = 0; pFILE->csect = 255;		/* File pointer */
    pFILE->fs = dj.fs; pFILE->id = dj.fs->id;	/* Owner file system object of the file */
    return FR_OK;
}
#endif

/*-----------------------------------------------------------------------*/
/* Read File                                                             */
/*-----------------------------------------------------------------------*/

#if defined(SDREAD) || defined(SDREADLINE)
FRESULT f_read (
    u8 spi,     /* spi module */
    FIL *pFILE, 		/* Pointer to the file object */
    void *buff,		/* Pointer to data buffer */
    word btr,		/* Number of bytes to read */
    word *br		/* Pointer to number of bytes read */
)
{
    FRESULT res;
    dword sect, remain;
    word rcnt, cc;
    CLUST clust;
    u8 *rbuff = buff;


    *br = 0;
    res = validate(pFILE->fs, pFILE->id);					/* Check validity of the object */
    if (res != FR_OK) return res;
    if (pFILE->flag & FA__ERROR) return FR_RW_ERROR;	/* Check error flag */
    if (!(pFILE->flag & FA_READ)) return FR_DENIED;	/* Check access mode */
    remain = pFILE->fsize - pFILE->fptr;
    if (btr > remain) btr = (word)remain;			/* Truncate btr by remaining bytes */

    /* Repeat until all data transferred */
    for ( ;  btr; rbuff += rcnt, pFILE->fptr += rcnt, *br += rcnt, btr -= rcnt)
    {
        /* On the sector boundary? */
        if ((pFILE->fptr % 512U) == 0)
        {
            /* On the cluster boundary? */
            if (pFILE->csect >= pFILE->fs->csize)
            {
                /* On the top of the file? */
                clust = (pFILE->fptr == 0) ? pFILE->org_clust : get_cluster(spi, pFILE->curr_clust);
                if (clust < 2 || clust >= pFILE->fs->max_clust)
                    goto fr_error;
                pFILE->curr_clust = clust;				/* Update current cluster */
                pFILE->csect = 0;						/* Reset sector address in the cluster */
            }
            sect = clust2sect(pFILE->curr_clust) + pFILE->csect;	/* Get current sector */
            cc = btr / 512U;						/* When remaining bytes >= sector size, */
            if (cc)
            {								/* Read maximum contiguous sectors directly */
                if (pFILE->csect + cc > pFILE->fs->csize)	/* Clip at cluster boundary */
                    cc = pFILE->fs->csize - pFILE->csect;
                if (disk_readsector(spi, 0, rbuff, sect, (u8)cc) != RES_OK)
                    goto fr_error;
                pFILE->csect += (u8)cc;				/* Next sector address in the cluster */
                rcnt = 512U * cc;					/* Number of bytes transferred */
                continue;
            }
            pFILE->csect++;							/* Next sector address in the cluster */
        }
        sect = clust2sect(pFILE->curr_clust) + pFILE->csect - 1;	/* Get current sector */
        if (!move_window(spi, sect))
            goto fr_error;		/* Move sector window */
        rcnt = 512U - (pFILE->fptr % 512U);			/* Get partial sector from sector window */
        if (rcnt > btr) rcnt = btr;
        memcpy(rbuff, &pFILE->fs->win[pFILE->fptr % 512U], rcnt);
    }

    return FR_OK;

    fr_error:	/* Abort this file due to an unrecoverable error */
    pFILE->flag |= FA__ERROR;
    return FR_RW_ERROR;
}
#endif

/*  --------------------------------------------------------------------
    Get 2 bytes from the file
    u8 spi,     spi module used
    char* buff  pointer to the string buffer to read
    int len     size of string buffer
    FIL* fil    pointer to the file object
    ------------------------------------------------------------------*/

#ifdef SDREAD16
u16 f_read16(u8 spi, FIL* fil)
{
    u16 p;
    u16 rc;

    f_read(spi, fil, &p, 2, &rc);
    return p;
}
#endif

#ifdef SDREAD32
u32 f_read32(u8 spi, FIL* fil)
{
    u32 p;
    u16 rc;

    f_read(spi, fil, &p, 4, &rc);
    return p;
}
#endif

#if !_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Write File                                                            */
/*-----------------------------------------------------------------------*/

//#ifdef SDWRITE
FRESULT f_write (
    u8 spi,     /* spi module */
    FIL *pFILE,			/* Pointer to the file object */
    const void *buff,	/* Pointer to the data to be written */
    word btw,			/* Number of bytes to write */
    word *bw			/* Pointer to number of bytes written */
)
{
    FRESULT res;
    dword sect;
    word wcnt, cc;
    CLUST clust;
    const u8 *wbuff = buff;


    *bw = 0;
    res = validate(pFILE->fs, pFILE->id);					/* Check validity of the object */
    if (res != FR_OK) return res;
    if (pFILE->flag & FA__ERROR) return FR_RW_ERROR;	/* Check error flag */
    if (!(pFILE->flag & FA_WRITE)) return FR_DENIED;	/* Check access mode */
    if (pFILE->fsize + btw < pFILE->fsize) return FR_OK;	/* File size cannot reach 4GB */

    for ( ;  btw;									/* Repeat until all data transferred */
        wbuff += wcnt, pFILE->fptr += wcnt, *bw += wcnt, btw -= wcnt) {
        if ((pFILE->fptr % 512U) == 0) {				/* On the sector boundary? */
            if (pFILE->csect >= pFILE->fs->csize) {		/* On the cluster boundary? */
                if (pFILE->fptr == 0) {				/* On the top of the file? */
                    clust = pFILE->org_clust;			/* Follow from the origin */
                    if (clust == 0)					/* When there is no cluster chain, */
                        pFILE->org_clust = clust = create_chain(spi, 0);	/* Create a new cluster chain */
                } else {							/* Middle or end of the file */
                    clust = create_chain(spi, pFILE->curr_clust);			/* Trace or streach cluster chain */
                }
                if (clust == 0) break;				/* Could not allocate a new cluster (disk full) */
                if (clust == 1 || clust >= pFILE->fs->max_clust) goto fw_error;
                pFILE->curr_clust = clust;				/* Update current cluster */
                pFILE->csect = 0;						/* Reset sector address in the cluster */
            }
            sect = clust2sect(pFILE->curr_clust) + pFILE->csect;	/* Get current sector */
            cc = btw / 512U;						/* When remaining bytes >= sector size, */
            if (cc) {								/* Write maximum contiguous sectors directly */
                if (pFILE->csect + cc > pFILE->fs->csize)	/* Clip at cluster boundary */
                    cc = pFILE->fs->csize - pFILE->csect;
                if (disk_writesector(spi, 0, wbuff, sect, (u8)cc) != RES_OK)
                    goto fw_error;
                pFILE->csect += (u8)cc;				/* Next sector address in the cluster */
                wcnt = 512U * cc;					/* Number of bytes transferred */
                continue;
            }
            if (pFILE->fptr >= pFILE->fsize) {			/* Flush R/W window without reading if needed */
                if (!move_window(spi, 0)) goto fw_error;
                pFILE->fs->winsect = sect;
            }
            pFILE->csect++;							/* Next sector address in the cluster */
        }
        sect = clust2sect(pFILE->curr_clust) + pFILE->csect - 1;	/* Get current sector */
        if (!move_window(spi, sect)) goto fw_error;		/* Move sector window */
        wcnt = 512U - (pFILE->fptr % 512U);			/* Put partial sector into sector window */
        if (wcnt > btw) wcnt = btw;
        memcpy(&pFILE->fs->win[pFILE->fptr % 512U], wbuff, wcnt);
        pFILE->fs->winflag = 1;
    }

    if (pFILE->fptr > pFILE->fsize) {pFILE->fsize = pFILE->fptr;}	/* Update file size if needed */
    pFILE->flag |= FA__WRITTEN;						/* Set file changed flag */
    return res;

fw_error:	/* Abort this file due to an unrecoverable error */
    pFILE->flag |= FA__ERROR;
    return FR_RW_ERROR;
}
//#endif

/*-----------------------------------------------------------------------*/
/* Synchronize the file object                                           */
/*-----------------------------------------------------------------------*/

#if defined(SDSYNC) || defined(SDCLOSE)
FRESULT f_sync (
    u8 spi,     /* spi module */
    FIL *pFILE		/* Pointer to the file object */
)
{
    FRESULT res;
    dword tim;
    u8 *dir;

    /* Check validity of the object */
    res = validate(pFILE->fs, pFILE->id);
    if (res == FR_OK)
    {
        /* Has the file been written? */
        if (pFILE->flag & FA__WRITTEN)
        {
            /* Update the directory entry */
            if (!move_window(spi, pFILE->dir_sect)) return FR_RW_ERROR;
            dir = pFILE->dir_ptr;
            dir[DIR_Attr] |= AM_ARC;						/* Set archive bit */
            ST_DWORD(&dir[DIR_FileSize], pFILE->fsize);		/* Update file size */
            ST_WORD(&dir[DIR_FstClusLO], pFILE->org_clust);	/* Update start cluster */
            #if _FAT32
            ST_WORD(&dir[DIR_FstClusHI], pFILE->org_clust >> 16);
            #endif
            tim = get_fattime();                            /* Updated time */
            ST_DWORD(&dir[DIR_WrtTime], tim);
            pFILE->flag &= (u8)~FA__WRITTEN;
            res = sync(spi);
        }
    }
    return res;
}
#endif
#endif /* !_FS_READONLY */



/*  --------------------------------------------------------------------
    Close File
    spi     spi module
    *pFILE     Pointer to the file object to be closed
    ------------------------------------------------------------------*/

#ifdef SDCLOSE
FRESULT f_close (u8 spi, FIL *pFILE)
{
    FRESULT res;

    #if !_FS_READONLY
    res = f_sync(spi, pFILE);
    #else
    res = validate(pFILE->fs, pFILE->id);
    #endif
    if (res == FR_OK) pFILE->fs = NULL;
    return res;
}
#endif


#if _FS_MINIMIZE <= 2
/*-----------------------------------------------------------------------*/
/* Seek File R/W Pointer                                                 */
/*-----------------------------------------------------------------------*/

#ifdef SDLSEEK
FRESULT f_lseek (
    u8 spi,
    FIL *pFILE,		/* Pointer to the file object */
    dword ofs		/* File pointer from top of file */
)
{
    FRESULT res;
    CLUST clust;
    dword csize, ifptr;


    res = validate(pFILE->fs, pFILE->id);			/* Check validity of the object */
    if (res != FR_OK) return res;
    if (pFILE->flag & FA__ERROR) return FR_RW_ERROR;
    if (ofs > pFILE->fsize					/* In read-only mode, clip offset with the file size */
    #if !_FS_READONLY
         && !(pFILE->flag & FA_WRITE)
    #endif
        ) ofs = pFILE->fsize;

    ifptr = pFILE->fptr;
    pFILE->fptr = 0; pFILE->csect = 255;
    if (ofs > 0)
    {
        csize = (dword)pFILE->fs->csize * 512U;		/* Cluster size (byte) */
        if (ifptr > 0 &&
            (ofs - 1) / csize >= (ifptr - 1) / csize)
        {/* When seek to same or following cluster, */
            pFILE->fptr = (ifptr - 1) & ~(csize - 1);	/* start from the current cluster */
            ofs -= pFILE->fptr;
            clust = pFILE->curr_clust;
        }
        else
        {									/* When seek to back cluster, */
            clust = pFILE->org_clust;					/* start from the first cluster */
            #if !_FS_READONLY
            if (clust == 0)
            {						/* If no cluster chain, create a new chain */
                clust = create_chain(spi, 0);
                if (clust == 1) goto fk_error;
                pFILE->org_clust = clust;
            }
            #endif
            pFILE->curr_clust = clust;
        }
        if (clust != 0)
        {
            while (ofs > csize)
            {					/* Cluster following loop */
                #if !_FS_READONLY
                if (pFILE->flag & FA_WRITE)
                {			/* Check if in write mode or not */
                    clust = create_chain(spi, clust);	/* Force streached if in write mode */
                    if (clust == 0)
                    {				/* When disk gets full, clip file size */
                        ofs = csize; break;
                    }
                }
                else
                #endif
                    clust = get_cluster(spi, clust);		/* Follow cluster chain if not in write mode */
                if (clust < 2 || clust >= pFILE->fs->max_clust)
                    goto fk_error;
                pFILE->curr_clust = clust;
                pFILE->fptr += csize;
                ofs -= csize;
            }
            pFILE->fptr += ofs;
            pFILE->csect = (u8)(ofs / 512U);	/* Sector offset in the cluster */
            if (ofs % 512U)
                pFILE->csect++;
        }
    }

    #if !_FS_READONLY
    if (pFILE->fptr > pFILE->fsize)
    {			/* Set changed flag if the file was extended */
        pFILE->fsize = pFILE->fptr;
        pFILE->flag |= FA__WRITTEN;
    }
    #endif

    return FR_OK;

fk_error:	/* Abort this file due to an unrecoverable error */
    pFILE->flag |= FA__ERROR;
    return FR_RW_ERROR;
}
#endif



#if _FS_MINIMIZE <= 1
/*-----------------------------------------------------------------------*/
/* Create a directroy object                                             */
/*-----------------------------------------------------------------------*/

#ifdef SDOPENDIR
FRESULT f_opendir (
    u8 spi,
    DIR_t *dj,			/* Pointer to directory object to create */
    const char *path	/* Pointer to the directory path */
)
{
    FRESULT res;
    u8 *dir;
    char fn[8+3+1];

    res = auto_mount(spi, &path, FA_OPEN_EXISTING); // 0
    if (res == FR_OK)
    {
        /* Trace the directory path */
        res = trace_path(spi, dj, fn, path, &dir);
        /* Trace completed */
        if (res == FR_OK)
        {
            /* It is not the root dir */
            if (dir)
            {
                /* The entry is a directory */
                if (dir[DIR_Attr] & AM_DIR)
                {
                    dj->clust =
                        #if _FAT32
                        ((dword)LD_WORD(&dir[DIR_FstClusHI]) << 16) |
                        #endif
                        LD_WORD(&dir[DIR_FstClusLO]);
                    dj->sect = clust2sect(dj->clust);
                    dj->index = 2;
                }
                /* The entry is not a directory */
                else
                {
                    res = FR_NO_FILE;
                }
            }
            dj->id = dj->fs->id;
        }
    }
    return res;
}
#endif



/*-----------------------------------------------------------------------*/
/* Read Directory Entry in Sequense                                      */
/*-----------------------------------------------------------------------*/

#ifdef SDREADDIR
FRESULT f_readdir (
    u8 spi,
    DIR_t *dj,			/* Pointer to the directory object */
    FILINFO *finfo		/* Pointer to file information to return */
)
{
    FRESULT res;
    u8 *dir;
    u8 c;


    res = validate(dj->fs, dj->id);			/* Check validity of the object */
    if (res != FR_OK) return res;

    finfo->fname[0] = 0;
    while (dj->sect) {
        if (!move_window(spi, dj->sect))
            return FR_RW_ERROR;
        dir = &dj->fs->win[(dj->index & 15) * 32];	/* pointer to the directory entry */
        c = dir[DIR_Name];
        if (c == 0) break;							/* Has it reached to end of dir? */
        if (c != 0xE5 && !(dir[DIR_Attr] & AM_VOL))	/* Is it a valid entry? */
            get_fileinfo(finfo, dir);
        if (!next_dir_entry(spi, dj)) dj->sect = 0;		/* Next entry */
        if (finfo->fname[0]) break;					/* Found valid entry */
    }

    return FR_OK;
}
#endif



#if _FS_MINIMIZE == 0
/*-----------------------------------------------------------------------*/
/* Get File Status                                                       */
/*-----------------------------------------------------------------------*/

#ifdef SDSTAT
FRESULT f_stat (
    u8 spi,
    const char *path,	/* Pointer to the file path */
    FILINFO *finfo		/* Pointer to file information to return */
)
{
    FRESULT res;
    DIR_t dj;
    u8 *dir;
    char fn[8+3+1];

    res = auto_mount(spi, &path, FA_OPEN_EXISTING); // 0
    if (res == FR_OK)
    {
        /* Trace the file path */
        res = trace_path(spi, &dj, fn, path, &dir);
        /* Trace completed */
        if (res == FR_OK)
        {
            if (dir)    /* Found an object */
                get_fileinfo(finfo, dir);
            else        /* It is root dir */
                res = FR_INVALID_NAME;
        }
    }

    return res;
}
#endif



#if !_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Truncate File                                                         */
/*-----------------------------------------------------------------------*/

#ifdef SDTRUNCATE
FRESULT f_truncate (
    u8 spi,
    FIL *pFILE		/* Pointer to the file object */
)
{
    FRESULT res;
    CLUST ncl;


    res = validate(pFILE->fs, pFILE->id);		/* Check validity of the object */
    if (res != FR_OK) return res;
    if (pFILE->flag & FA__ERROR) return FR_RW_ERROR;	/* Check error flag */
    if (!(pFILE->flag & FA_WRITE)) return FR_DENIED;	/* Check access mode */

    if (pFILE->fsize > pFILE->fptr) {
        pFILE->fsize = pFILE->fptr;	/* Set file size to current R/W point */
        pFILE->flag |= FA__WRITTEN;
        if (pFILE->fptr == 0) {	/* When set file size to zero, remove entire cluster chain */
            if (!remove_chain(spi, pFILE->org_clust)) goto ft_error;
            pFILE->org_clust = 0;
        } else {				/* When truncate a part of the file, remove remaining clusters */
            ncl = get_cluster(spi, pFILE->curr_clust);
            if (ncl < 2) goto ft_error;
            if (ncl < pFILE->fs->max_clust) {
                if (!put_cluster(spi, pFILE->curr_clust, (CLUST)0x0FFFFFFF)) goto ft_error;
                if (!remove_chain(spi, ncl)) goto ft_error;
            }
        }
    }

    return FR_OK;

ft_error:	/* Abort this file due to an unrecoverable error */
    pFILE->flag |= FA__ERROR;
    return FR_RW_ERROR;
}
#endif



/*-----------------------------------------------------------------------*/
/* Get Number of Free Clusters                                           */
/*-----------------------------------------------------------------------*/

#ifdef SDGETFREE
FRESULT f_getfree (
    u8 spi,
    const char *drv,	/* Pointer to the logical drive number (root dir) */
    dword *nclust,		/* Pointer to the variable to return number of free clusters */
    FATFS **fatfs		/* Pointer to pointer to corresponding file system object to return */
)
{
    FRESULT res;
    FATFS *pFS;
    dword n, sect;
    CLUST clust;
    u8 fat, f, *p;


    /* Get drive number */
    res = auto_mount(spi, &drv, FA_OPEN_EXISTING); // 0
    if (res != FR_OK) return res;
    *fatfs = fs = pFAT;

    /* If number of free cluster is valid, return it without cluster scan. */
    if (fs->free_clust <= fs->max_clust - 2) {
        *nclust = fs->free_clust;
        return FR_OK;
    }

    /* Get number of free clusters */
    fat = fs->fs_type;
    n = 0;
    if (fat == FS_FAT12) {
        clust = 2;
        do {
            if ((word)get_cluster(spi, clust) == 0) n++;
        } while (++clust < fs->max_clust);
    } else {
        clust = fs->max_clust;
        sect = fs->fatbase;
        f = 0; p = 0;
        do {
            if (!f) {
                if (!move_window(spi, sect++)) return FR_RW_ERROR;
                p = fs->win;
            }
            /* R. Blanchot 22-01-2016 :
            if (!_FAT32 || fat == FS_FAT16) {
                if (LD_WORD(p) == 0) n++;
                p += 2; f += 1;
            } else {
                if (LD_DWORD(p) == 0) n++;
                p += 4; f += 2;
            }
            */
            #if _FAT32
            if (LD_DWORD(p) == 0) n++;
            p += 4; f += 2;
            #else
            if (fat == FS_FAT16)
            {
                if (LD_WORD(p) == 0) n++;
                p += 2; f += 1;
            }
            #endif
        } while (--clust);
    }
    fs->free_clust = n;
#if _USE_FSINFO
    if (fat == FS_FAT32) fs->fsi_flag = 1;
#endif

    *nclust = n;
    return FR_OK;
}
#endif



/*-----------------------------------------------------------------------*/
/* Delete a File or Directory                                            */
/*-----------------------------------------------------------------------*/

#ifdef SDUNLINK
FRESULT f_unlink (
    u8 spi,
    const char *path		/* Pointer to the file or directory path */
)
{
    FRESULT res;
    DIR_t dj;
    u8 *dir;
    u8 *sdir;
    dword dsect;
    char fn[8+3+1];
    CLUST dclust;


    res = auto_mount(spi, &path, FA_READ); // 1
    if (res != FR_OK) return res;
    res = trace_path(spi, &dj, fn, path, &dir);	/* Trace the file path */
    if (res != FR_OK) return res;			/* Trace failed */
    if (!dir) return FR_INVALID_NAME;		/* It is the root directory */
    if (dir[DIR_Attr] & AM_RDO) return FR_DENIED;	/* It is a R/O object */
    dsect = dj.fs->winsect;
    dclust =
        #if _FAT32
        ((dword)LD_WORD(&dir[DIR_FstClusHI]) << 16) |
        #endif
        LD_WORD(&dir[DIR_FstClusLO]);
    if (dir[DIR_Attr] & AM_DIR)
    {			/* It is a sub-directory */
        dj.clust = dclust;					/* Check if the sub-dir is empty or not */
        dj.sect = clust2sect(dclust);
        dj.index = 2;
        do {
            if (!move_window(spi, dj.sect)) return FR_RW_ERROR;
            sdir = &dj.fs->win[(dj.index & 15) * 32];
            if (sdir[DIR_Name] == 0) break;
            if (sdir[DIR_Name] != 0xE5 && !(sdir[DIR_Attr] & AM_VOL))
                return FR_DENIED;	/* The directory is not empty */
        } while (next_dir_entry(spi, &dj));
    }

    if (!move_window(spi, dsect)) return FR_RW_ERROR;	/* Mark the directory entry 'deleted' */
    dir[DIR_Name] = 0xE5;
    dj.fs->winflag = 1;
    if (!remove_chain(spi, dclust)) return FR_RW_ERROR;	/* Remove the cluster chain */

    return sync(spi);
}
#endif



/*-----------------------------------------------------------------------*/
/* Create a Directory                                                    */
/*-----------------------------------------------------------------------*/

#ifdef SDMKDIR
FRESULT f_mkdir (
    u8 spi,
    const char *path		/* Pointer to the directory path */
)
{
    FRESULT res;
    DIR_t dj;
    u8 *dir;
    u8 *fw, n;
    char fn[8+3+1];
    dword sect, dsect, tim;
    CLUST dclust, pclust;


    res = auto_mount(spi, &path, FA_READ); // 1
    if (res != FR_OK) return res;
    res = trace_path(spi, &dj, fn, path, &dir);	/* Trace the file path */
    if (res == FR_OK) return FR_EXIST;		/* Any file or directory is already existing */
    if (res != FR_NO_FILE) return res;

    res = reserve_direntry(spi, &dj, &dir); 		/* Reserve a directory entry */
    if (res != FR_OK) return res;
    sect = dj.fs->winsect;
    dclust = create_chain(spi, 0);				/* Allocate a cluster for new directory table */
    if (dclust == 1) return FR_RW_ERROR;
    dsect = clust2sect(dclust);
    if (!dsect) return FR_DENIED;
    if (!move_window(spi, dsect)) return FR_RW_ERROR;

    fw = dj.fs->win;
    memset((void *)fw, 0, 512U);					/* Clear the directory table */
    for (n = 1; n < dj.fs->csize; n++) {
        if (disk_writesector(spi, 0, fw, ++dsect, 1) != RES_OK)
            return FR_RW_ERROR;
    }

    memset(&fw[DIR_Name], ' ', 8+3);		/* Create "." entry */
    fw[DIR_Name] = '.';
    fw[DIR_Attr] = AM_DIR;
    tim = get_fattime();
    ST_DWORD(&fw[DIR_WrtTime], tim);
    memcpy(&fw[32], &fw[0], 32); fw[33] = '.';	/* Create ".." entry */
    pclust = dj.sclust;
    #if _FAT32
    ST_WORD(&fw[   DIR_FstClusHI], dclust >> 16);
    if (dj.fs->fs_type == FS_FAT32 && pclust == dj.fs->dirbase) pclust = 0;
    ST_WORD(&fw[32+DIR_FstClusHI], pclust >> 16);
    #endif
    ST_WORD(&fw[   DIR_FstClusLO], dclust);
    ST_WORD(&fw[32+DIR_FstClusLO], pclust);
    dj.fs->winflag = 1;

    if (!move_window(spi, sect)) return FR_RW_ERROR;
    memset(&dir[0], 0, 32);						/* Clean-up the new entry */
    memcpy(&dir[DIR_Name], fn, 8+3);			/* Name */
    dir[DIR_NTres] = fn[11];
    dir[DIR_Attr] = AM_DIR;						/* Attribute */
    ST_DWORD(&dir[DIR_WrtTime], tim);			/* Crated time */
    ST_WORD(&dir[DIR_FstClusLO], dclust);		/* Table start cluster */
    #if _FAT32
    ST_WORD(&dir[DIR_FstClusHI], dclust >> 16);
    #endif
    return sync(spi);
}
#endif


/*-----------------------------------------------------------------------*/
/* Change File Attribute                                                 */
/*-----------------------------------------------------------------------*/

#ifdef SDCHMOD
FRESULT f_chmod (
    u8 spi,
    const char *path,	/* Pointer to the file path */
    u8 value,			/* Attribute bits */
    u8 mask			/* Attribute mask to change */
)
{
    FRESULT res;
    DIR_t dj;
    u8 *dir;
    char fn[8+3+1];


    res = auto_mount(spi, &path, FA_READ); // 1
    if (res == FR_OK)
    {
        res = trace_path(spi, &dj, fn, path, &dir);	/* Trace the file path */
        if (res == FR_OK)
        {				/* Trace completed */
            if (!dir)
            {
                res = FR_INVALID_NAME;	/* Root directory */
            }
            else
            {
                mask &= AM_RDO|AM_HID|AM_SYS|AM_ARC;	/* Valid attribute mask */
                dir[DIR_Attr] = (value & mask) | (dir[DIR_Attr] & (u8)~mask);	/* Apply attribute change */
                res = sync(spi);
            }
        }
    }
    return res;
}
#endif



/*-----------------------------------------------------------------------*/
/* Change Timestamp                                                      */
/*-----------------------------------------------------------------------*/

#ifdef SDUTIME
FRESULT f_utime (
    u8 spi,
    const char *path,		/* Pointer to the file/directory name */
    const FILINFO *finfo	/* Pointer to the timestamp to be set */
)
{
    FRESULT res;
    DIR_t dj;
    u8 *dir;
    char fn[8+3+1];


    res = auto_mount(spi, &path, FA_READ); // 1
    if (res == FR_OK) {
        res = trace_path(spi, &dj, fn, path, &dir);	/* Trace the file path */
        if (res == FR_OK) {				/* Trace completed */
            if (!dir) {
                res = FR_INVALID_NAME;	/* Root directory */
            } else {
                ST_WORD(&dir[DIR_WrtTime], finfo->ftime);
                ST_WORD(&dir[DIR_WrtDate], finfo->fdate);
                res = sync(spi);
            }
        }
    }
    return res;
}
#endif

/*-----------------------------------------------------------------------*/
/* Rename File/Directory                                                 */
/*-----------------------------------------------------------------------*/

#ifdef SDRENAME
FRESULT f_rename (
    u8 spi,
    const char *path_old,	/* Pointer to the old name */
    const char *path_new	/* Pointer to the new name */
)
{
    FRESULT res;
    dword sect_old;
    u8 *dir_old, *dir_new, direntry[32-11];
    DIR_t dj;
    char fn[8+3+1];


    res = auto_mount(spi, &path_old, FA_READ); // 1
    if (res != FR_OK) return res;

    res = trace_path(spi, &dj, fn, path_old, &dir_old);	/* Check old object */
    if (res != FR_OK) return res;				/* The old object is not found */
    if (!dir_old) return FR_NO_FILE;
    sect_old = dj.fs->winsect;					/* Save the object information */
    memcpy(direntry, &dir_old[DIR_Attr], 32-11);

    res = trace_path(spi, &dj, fn, path_new, &dir_new);	/* Check new object */
    if (res == FR_OK) return FR_EXIST;			/* The new object name is already existing */
    if (res != FR_NO_FILE) return res;			/* Is there no old name? */
    res = reserve_direntry(spi, &dj, &dir_new); 		/* Reserve a directory entry */
    if (res != FR_OK) return res;

    memcpy(&dir_new[DIR_Attr], direntry, 32-11);	/* Create new entry */
    memcpy(&dir_new[DIR_Name], fn, 8+3);
    dir_new[DIR_NTres] = fn[11];
    dj.fs->winflag = 1;

    if (!move_window(spi, sect_old)) return FR_RW_ERROR;	/* Delete old entry */
    dir_old[DIR_Name] = 0xE5;

    return sync(spi);
}
#endif

#endif /* !_FS_READONLY */
#endif /* _FS_MINIMIZE == 0 */
#endif /* _FS_MINIMIZE <= 1 */
#endif /* _FS_MINIMIZE <= 2 */

#if _USE_MKFS && !_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Create File System on the Drive                                       */
/*-----------------------------------------------------------------------*/
#define N_ROOTDIR_t	512			/* Multiple of 32 and <= 2048 */
#define N_FATS		1			/* 1 or 2 */
#define MAX_SECTOR	64000000UL	/* Maximum partition size */
#define MIN_SECTOR	2000UL		/* Minimum partition size */

#ifdef SDMKFS
FRESULT f_mkfs (
    u8 spi,
    u8 partition,		/* Partitioning rule 0:FDISK, 1:SFD */
    word allocsize		/* Allocation unit size [bytes] */
)
{
    u8 fmt, m, *tbl,rc;
    dword b_part, b_fat, b_dir, b_data;		/* Area offset (LBA) */
    dword n_part, n_rsv, n_fat, n_dir;		/* Area size */
    dword n_clust, n;
    FATFS *pFS;
    u8 stat;

    /* Check validity of the parameters */
    if (partition >= 2) return FR_MKFS_ABORTED;
    for (n = 512; n <= 32768U && n != allocsize; n <<= 1);
    if (n != allocsize) return FR_MKFS_ABORTED;

    /* Check mounted drive and clear work area */
    pFS = pFAT;
    if (!pFS) return FR_NOT_ENABLED;
    pFS->fs_type = 0;

    /* Get disk statics */
    stat = disk_initialize(spi, 0);
    if (stat & STA_NOINIT) return FR_NOT_READY;
    if (stat & STA_PROTECT) return FR_WRITE_PROTECTED;
    if (disk_ioctl(spi, 0, GET_SECTOR_COUNT, &n_part) != RES_OK || n_part < MIN_SECTOR)
        return FR_MKFS_ABORTED;
    if (n_part > MAX_SECTOR) n_part = MAX_SECTOR;
    b_part = (!partition) ? 63 : 0;		/* Boot sector */
    n_part -= b_part;
    #if S_MAX_SIZ > 512						/* Check disk sector size */
    if (disk_ioctl(spi, 0, GET_SECTOR_SIZE, &SSZ(pFS)) != RES_OK
        || SSZ(pFS) > S_MAX_SIZ
        || SSZ(pFS) > allocsize)
        return FR_MKFS_ABORTED;
    #endif
    allocsize /= SSZ(pFS);		/* Number of sectors per cluster */

    /* Pre-compute number of clusters and FAT type */
    n_clust = n_part / allocsize;
    fmt = FS_FAT12;
    if (n_clust >= 0xFF5) fmt = FS_FAT16;
    if (n_clust >= 0xFFF5) fmt = FS_FAT32;
    /* Determine offset and size of FAT structure */
    switch (fmt) {
    case FS_FAT12:
        n_fat = ((n_clust * 3 + 1) / 2 + 3 + SSZ(pFS) - 1) / SSZ(pFS);
        n_rsv = 1 + partition;
        n_dir = N_ROOTDIR_t * 32 / SSZ(pFS);
        break;
    case FS_FAT16:
        n_fat = ((n_clust * 2) + 4 + SSZ(pFS) - 1) / SSZ(pFS);
        n_rsv = 1 + partition;
        n_dir = N_ROOTDIR_t * 32 / SSZ(pFS);
        break;
    default:
        n_fat = ((n_clust * 4) + 8 + SSZ(pFS) - 1) / SSZ(pFS);
        n_rsv = 33 - partition;
        n_dir = 0;
    }
    b_fat = b_part + n_rsv;			/* FATs start sector */
    b_dir = b_fat + n_fat * N_FATS;	/* Directory start sector */
    b_data = b_dir + n_dir;			/* Data start sector */

    /* Align data start sector to erase block boundary (for flash memory media) */
    rc=disk_ioctl(spi, 0, GET_BLOCK_SIZE, &n);
    if (rc != RES_OK) return FR_MKFS_ABORTED;
    n = (b_data + n - 1) & ~(n - 1);
    n_fat += (n - b_data) / N_FATS;
    /* b_dir and b_data are no longer used below */

    /* Determine number of cluster and final check of validity of the FAT type */
    n_clust = (n_part - n_rsv - n_fat * N_FATS - n_dir) / allocsize;
    if (   (fmt == FS_FAT16 && n_clust < 0xFF5)
        || (fmt == FS_FAT32 && n_clust < 0xFFF5))
        return FR_MKFS_ABORTED;

    /* Create partition table if needed */
    if (!partition) {
        dword n_disk = b_part + n_part;

        tbl = &pFS->win[MBR_Table];
        ST_DWORD(&tbl[0], 0x00010180);	/* Partition start in CHS */
        if (n_disk < 63UL * 255 * 1024) {	/* Partition end in CHS */
            n_disk = n_disk / 63 / 255;
            tbl[7] = (u8)n_disk;
            tbl[6] = (u8)((n_disk >> 2) | 63);
        } else {
            ST_WORD(&tbl[6], 0xFFFF);
        }
        tbl[5] = 254;
        if (fmt != FS_FAT32)			/* System ID */
            tbl[4] = (n_part < 0x10000) ? 0x04 : 0x06;
        else
            tbl[4] = 0x0c;
        ST_DWORD(&tbl[8], 63);			/* Partition start in LBA */
        ST_DWORD(&tbl[12], n_part);		/* Partition size in LBA */
        ST_WORD(&tbl[64], 0xAA55);		/* Signature */
        if (disk_writesector(spi, 0, pFS->win, 0, 1) != RES_OK)
            return FR_RW_ERROR;
    }

    /* Create boot record */
    tbl = pFS->win;								/* Clear buffer */
    memset((void *)tbl, 0, SSZ(pFS));
    ST_DWORD(&tbl[BS_jmpBoot], 0x90FEEB);		/* Boot code (jmp $, nop) */
    ST_WORD(&tbl[BPB_BytsPerSec], SSZ(pFS));		/* Sector size */
    tbl[BPB_SecPerClus] = (u8)allocsize;		/* Sectors per cluster */
    ST_WORD(&tbl[BPB_RsvdSecCnt], n_rsv);		/* Reserved sectors */
    tbl[BPB_NumFATs] = N_FATS;					/* Number of FATs */
    ST_WORD(&tbl[BPB_RootEntCnt], SSZ(pFS) / 32 * n_dir); /* Number of rootdir entries */
    if (n_part < 0x10000) {						/* Number of total sectors */
        ST_WORD(&tbl[BPB_TotSec16], n_part);
    } else {
        ST_DWORD(&tbl[BPB_TotSec32], n_part);
    }
    tbl[BPB_Media] = 0xF8;						/* Media descripter */
    ST_WORD(&tbl[BPB_SecPerTrk], 63);			/* Number of sectors per track */
    ST_WORD(&tbl[BPB_NumHeads], 255);			/* Number of heads */
    ST_DWORD(&tbl[BPB_HiddSec], b_part);		/* Hidden sectors */
    n = get_fattime();							/* Use current time as a VSN */
    if (fmt != FS_FAT32) {
        ST_DWORD(&tbl[BS_VolID], n);			/* Volume serial number */
        ST_WORD(&tbl[BPB_FATSz16], n_fat);		/* Number of secters per FAT */
        tbl[BS_DrvNum] = 0x80;					/* Drive number */
        tbl[BS_BootSig] = 0x29;					/* Extended boot signature */
        memcpy(&tbl[BS_VolLab], "NO NAME    FAT     ", 19);	/* Volume lavel, FAT signature */
    } else {
        ST_DWORD(&tbl[BS_VolID32], n);			/* Volume serial number */
        ST_DWORD(&tbl[BPB_FATSz32], n_fat);		/* Number of secters per FAT */
        ST_DWORD(&tbl[BPB_RootClus], 2);		/* Root directory cluster (2) */
        ST_WORD(&tbl[BPB_FSInfo], 1);			/* FSInfo record (bs+1) */
        ST_WORD(&tbl[BPB_BkBootSec], 6);		/* Backup boot record (bs+6) */
        tbl[BS_DrvNum32] = 0x80;				/* Drive number */
        tbl[BS_BootSig32] = 0x29;				/* Extended boot signature */
        memcpy(&tbl[BS_VolLab32], "NO NAME    FAT32   ", 19);	/* Volume lavel, FAT signature */
    }
    ST_WORD(&tbl[BS_55AA], 0xAA55);			/* Signature */
    if (disk_writesector(spi, 0, tbl, b_part+0, 1) != RES_OK)
        return FR_RW_ERROR;
    if (fmt == FS_FAT32)
        disk_writesector(spi, 0, tbl, b_part+6, 1);

    /* Initialize FAT area */
    for (m = 0; m < N_FATS; m++) {
        memset((void *)tbl, 0, SSZ(pFS));		/* 1st sector of the FAT  */
        if (fmt != FS_FAT32) {
            n = (fmt == FS_FAT12) ? 0x00FFFFF8 : 0xFFFFFFF8;
            ST_DWORD(&tbl[0], n);			/* Reserve cluster #0-1 (FAT12/16) */
        } else {
            ST_DWORD(&tbl[0], 0xFFFFFFF8);	/* Reserve cluster #0-1 (FAT32) */
            ST_DWORD(&tbl[4], 0xFFFFFFFF);
            ST_DWORD(&tbl[8], 0x0FFFFFFF);	/* Reserve cluster #2 for root dir */
        }
        if (disk_writesector(spi, 0, tbl, b_fat++, 1) != RES_OK)
            return FR_RW_ERROR;
        memset((void *)tbl, 0, SSZ(pFS));		/* Following FAT entries are filled by zero */
        for (n = 1; n < n_fat; n++) {
            if (disk_writesector(spi, 0, tbl, b_fat++, 1) != RES_OK)
                return FR_RW_ERROR;
        }
    }

    /* Initialize Root directory */
    m = (u8)((fmt == FS_FAT32) ? allocsize : n_dir);
    do {
        if (disk_writesector(spi, 0, tbl, b_fat++, 1) != RES_OK)
            return FR_RW_ERROR;
    } while (--m);

    /* Create FSInfo record if needed */
    if (fmt == FS_FAT32) {
        ST_WORD(&tbl[BS_55AA], 0xAA55);
        ST_DWORD(&tbl[FSI_LeadSig], 0x41615252);
        ST_DWORD(&tbl[FSI_StrucSig], 0x61417272);
        ST_DWORD(&tbl[FSI_Free_Count], n_clust - 1);
        ST_DWORD(&tbl[FSI_Nxt_Free], 0xFFFFFFFF);
        disk_writesector(spi, 0, tbl, b_part+1, 1);
        disk_writesector(spi, 0, tbl, b_part+7, 1);
    }

    return (disk_ioctl(spi, 0, CTRL_SYNC, NULL) == RES_OK) ? FR_OK : FR_RW_ERROR;
}
#endif
#endif /* _USE_MKFS && !_FS_READONLY */


#if _USE_FORWARD
/*-----------------------------------------------------------------------*/
/* Forward data to the stream directly                                   */
/*-----------------------------------------------------------------------*/

#ifdef SDFORWARD
FRESULT f_forward (
    u8 spi,
    FIL *pFILE, 						/* Pointer to the file object */
    word (*func)(const u8*,word),	/* Pointer to the streaming function */
    word btr,						/* Number of bytes to forward */
    word *br						/* Pointer to number of bytes forwarded */
)
{
    FRESULT res;
    dword remain;
    word rcnt;
    CLUST clust;


    *br = 0;
    res = validate(pFILE->pFS, pFILE->id);					/* Check validity of the object */
    if (res != FR_OK) return res;
    if (pFILE->flag & FA__ERROR) return FR_RW_ERROR;	/* Check error flag */
    if (!(pFILE->flag & FA_READ)) return FR_DENIED;	/* Check access mode */
    remain = pFILE->fsize - pFILE->fptr;
    if (btr > remain) btr = (word)remain;			/* Truncate btr by remaining bytes */

    for ( ;  btr && (*func)(NULL, 0);				/* Repeat until all data transferred */
        pFILE->fptr += rcnt, *br += rcnt, btr -= rcnt) {
        if ((pFILE->fptr % 512U) == 0) {				/* On the sector boundary? */
            if (pFILE->csect >= pFILE->fs->csize) {		/* On the cluster boundary? */
                clust = (pFILE->fptr == 0) ?			/* On the top of the file? */
                    pFILE->org_clust : get_cluster(spi, pFILE->curr_clust);
                if (clust < 2 || clust >= pFILE->fs->max_clust) goto ff_error;
                pFILE->curr_clust = clust;				/* Update current cluster */
                pFILE->csect = 0;						/* Reset sector address in the cluster */
            }
            pFILE->csect++;							/* Next sector address in the cluster */
        }
        if (!move_window(spi, clust2sect(pFILE->curr_clust) + pFILE->csect - 1))	/* Move sector window */
            goto ff_error;
        rcnt = 512U - (word)(pFILE->fptr % 512U);		/* Forward data from sector window */
        if (rcnt > btr) rcnt = btr;
        rcnt = (*func)(&pFILE->fs->win[(word)pFILE->fptr % 512U], rcnt);
        if (rcnt == 0) goto ff_error;
    }

    return FR_OK;

ff_error:	/* Abort this function due to an unrecoverable error */
    pFILE->flag |= FA__ERROR;
    return FR_RW_ERROR;
}
#endif
#endif /* _USE_FORWARD */



#if _USE_STRFUNC >= 1
/*-----------------------------------------------------------------------*/
/* Get a string from the file                                            */
/*-----------------------------------------------------------------------*/

#ifdef SDREADLINE
char* f_gets (
    u8 spi,
    FIL* fil,       /* Pointer to the file object */
    char* buff,     /* Pointer to the string buffer to read */
    int len         /* Size of string buffer */
)
{
    int i = 0;
    char *p = buff;
    word rc;


    while (i < len - 1) {			/* Read bytes until buffer gets filled */
        f_read(spi, fil, p, 1, &rc);
        if (rc != 1) break;			/* Break when no data to read */
#if _USE_STRFUNC >= 2
        if (*p == '\r') continue;	/* Strip '\r' */
#endif
        i++;
        if (*p++ == '\n') break;	/* Break when reached end of line */
    }
    *p = 0;
    return i ? buff : 0;			/* When no data read (eof or error), return with error. */
}
#endif


#if !_FS_READONLY
/*-----------------------------------------------------------------------*/
/* Put a character to the file                                           */
/*-----------------------------------------------------------------------*/
int f_putc (
    u8 spi,
    int chr,	/* A character to be output */
    FIL* fil	/* Ponter to the file object */
)
{
    word bw;
    char c;


#if _USE_STRFUNC >= 2
    if (chr == '\n') f_putc(spi, '\r', fil);	/* LF -> CRLF conversion */
#endif
    /* Special value may be used to switch the destination to any other device */
    /*	put_console(chr);	*/
    if (!fil)
    {
        return chr;
    }
    c = (char)chr;
    f_write(spi, fil, &c, 1, &bw);	/* Write a byte to the file */
    return bw ? chr : EOF;		/* Return the resulut */
}

/*-----------------------------------------------------------------------*/
/* Put a string to the file                                              */
/*-----------------------------------------------------------------------*/
int f_puts (
    u8 spi,
    const char* str,	/* Pointer to the string to be output */
    FIL* fil			/* Pointer to the file object */
)
{
    int n;


    for (n = 0; *str; str++, n++) {
        if (f_putc(spi, *str, fil) == EOF) return EOF;
    }
    return n;
}

/*-----------------------------------------------------------------------*/
/* Put a formatted string to the file                                    */
/* TODO : to replace with the Pinguino printf                            */
/*-----------------------------------------------------------------------*/

#ifdef SDPRINTF
void f_printf(u8 module, FIL* fil, const u8 *fmt, ...)
{
    static u8 buffer[25];
    u8 *c=(char*)&buffer;
    va_list	args;

    va_start(args, fmt);
    psprintf2(buffer, fmt, args); // Pinguino printf
    va_end(args);

    while (*c)
        f_putc(spi, *c++, fil);
}

#if 0
int f_printf (
    u8 spi,
    FIL* fil,			/* Pointer to the file object */
    const char* str,	/* Pointer to the format string */
    ...					/* Optional arguments... */
)
{
    va_list arp;
    u8 c, f, r;
    dword val;
    char s[16];
    int i, w, res, cc;


    va_start(arp, str);

    for (cc = res = 0; cc != EOF; res += cc) {
        c = *str++;
        if (c == 0) break;			/* End of string */
        if (c != '%') {				/* Non escape cahracter */
            cc = f_putc(spi, c, fil);
            if (cc != EOF) cc = 1;
            continue;
        }
        w = f = 0;
        c = *str++;
        if (c == '0') {				/* Flag: '0' padding */
            f = 1; c = *str++;
        }
        while (c >= '0' && c <= '9') {	/* Precision */
            w = w * 10 + (c - '0');
            c = *str++;
        }
        if (c == 'l') {				/* Prefix: Size is long int */
            f |= 2; c = *str++;
        }
        if (c == 's') {				/* Type is string */
            cc = f_puts(spi, va_arg(arp, char*), fil);
            continue;
        }
        if (c == 'c') {				/* Type is character */
            cc = f_putc(spi, va_arg(arp, char), fil);
            if (cc != EOF) cc = 1;
            continue;
        }
        r = 0;
        if (c == 'd') r = 10;		/* Type is signed decimal */
        if (c == 'u') r = 10;		/* Type is unsigned decimal */
        if (c == 'X') r = 16;		/* Type is unsigned hexdecimal */
        if (r == 0) break;			/* Unknown type */
        if (f & 2) {				/* Get the value */
            val = (dword)va_arg(arp, long);
        } else {
            val = (c == 'd') ? (dword)(long)va_arg(arp, int) : (dword)va_arg(arp, unsigned int);
        }
        /* Put numeral string */
        if (c == 'd') {
            if (val >= 0x80000000) {
                val = 0 - val;
                f |= 4;
            }
        }
        i = sizeof(s) - 1; s[i] = 0;
        do {
            c = (u8)(val % r + '0');
            if (c > '9') c += 7;
            s[--i] = c;
            val /= r;
        } while (i && val);
        if (i && (f & 4)) s[--i] = '-';
        w = sizeof(s) - 1 - w;
        while (i && i > w) s[--i] = (f & 1) ? '0' : ' ';
        cc = f_puts(spi, &s[i], fil);
    }

    va_end(arp);
    return (cc == EOF) ? cc : res;
}
#endif
#endif /* SDPRINTF */
#endif /* !_FS_READONLY */
#endif /* _USE_STRFUNC >= 1*/

u8 isDirectory(FILINFO *info)
{
    return ((info->fattrib & AM_DIR) ? True:False);
}

u8 isFile(FILINFO *info)
{
    return ((info->fattrib & AM_DIR) ? False:True);
}

u8 isNotEmpty(FILINFO *info)
{
    //return (!info->fname[0]);
    return (info->fname[0]);
}

u8 isReadOnly(FILINFO *info)
{
    return ((info->fattrib & AM_RDO) ? True:False);
}

u8 isHidden(FILINFO *info)
{
    return ((info->fattrib & AM_HID) ? True:False);
}

u8 isSystem(FILINFO *info)
{
    return ((info->fattrib & AM_SYS) ? True:False);
}

u8 isArchive(FILINFO *info)
{
    return ((info->fattrib & AM_ARC) ? True:False);
}

u8 * getName(FILINFO *info)
{
    return (info->fname);
}

u32 getSize(FILINFO *info)
{
    return (info->fsize);
}

#endif // _TFF_C
