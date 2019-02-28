/*	----------------------------------------------------------------------------
    FILE:			fileio.c
    PROJECT:		pinguino32
    PURPOSE:		SD Card file system functions
    AUTHORS:		Alfred Broda <alfredbroda@gmail.com>
                    Mark Harper <markfh@f2s.com>
                    Regis Blanchot <rblanchot@gmail.com>
    FIRST RELEASE:	23 dec. 2011
    LAST RELEASE:	06 jan. 2012
    ----------------------------------------------------------------------------
    based on original code by Regis Blanchot and FatFS example for PIC24
    ----------------------------------------------------------------------------
    [30-03-12][hgmvanbeek@gmail.com][Some cards have no card detect and no write protect]
    07 May 2012	As part of providing support for PIC32 Pinguino Micro and
                    potentially other cards removed #if defined (PIC32_Pinguino) etc
                    and #endif in function mount() so that SDCS is set via mount 
                    for all cards.
*/

//#ifndef __FILEIO_C__
//#define __FILEIO_C__

#include <compiler.h>
#include <typedef.h>

// standard C libraries used
//#include <ctype.h>              // toupper...
//#include <string.h>             // memcpy...
#include <stdarg.h>             // vargs...

#include <spi.h>                // in order to use default SPI port
#include <spi.c>                // in order to use default SPI port

#include <sd/ffconf.h>          // SD lib. config. file

#if _FS_TINY
#include <sd/tff.h>             // Tiny Fat Filesystem
#else
#include <sd/pff.h>             // Petit Fat Filesystem
#endif

#include <sd/fileio.h>          // file I/O routines
#include <sd/diskio.h>          // Disk access functions
#include <sd/diskio.c>          // Disk access functions


/*	----------------------------------------------------------------------------
 Scans the current disk and compiles a list of files with a given extension
 list     array of file names max * 8
 max      number of entries
 ext      file extension we are searching for
 return   number of files found
 --------------------------------------------------------------------------*/
#if 0
//unsigned listTYPE(char *listname, long *listsize, int max, const char *ext )
unsigned listTYPE(DIRTABLE *list, int max, const char *ext)
//unsigned listTYPE(char *list, int max, const char *ext )
{
    //TODO: implement

    return 0;
} // listTYPE

/* Prints the directory contents */
unsigned listDir(u8 module, const char *path)
{
    //TODO: remove all CDC references
    long p1;
    u8 res; //, b;
    u16 s1, s2;
    DIR_t dir; /* Directory object */

    res = f_opendir(module, &dir, "/");
    #ifdef SD_DEBUG
    debugf("f_opendir?\r\n ");
    put_rc(res);
    #endif
    p1 = s1 = s2 = 0;
    #ifdef SD_DEBUG
    debugf("\nf_readdir('%s'): ", path);
    #endif

    for (;;)
    {
        res = f_readdir(module, &dir, &Finfo);
        #ifdef SD_DEBUG
        put_rc(res);
        #endif
        if ((res != FR_OK) || !Finfo.fname[0])
            break;

        if (Finfo.fattrib & AM_DIR)
        {
            s2++;
        }
        else
        {
            s1++;
            p1 += Finfo.fsize;
        }

/* what about other outputs ?
        debugf("%c%c%c%c%c ",
                (Finfo.fattrib & AM_DIR) ? 'D' : '-',
                (Finfo.fattrib & AM_RDO) ? 'R' : '-',
                (Finfo.fattrib & AM_HID) ? 'H' : '-',
                (Finfo.fattrib & AM_SYS) ? 'S' : '-',
                (Finfo.fattrib & AM_ARC) ? 'A' : '-');
        debugf("%u/%02u/%02u %02u:%02u ",
                (Finfo.fdate >> 9) + 1980,
                (Finfo.fdate >> 5) & 15, Finfo.fdate & 31, (Finfo.ftime >> 11),
                (Finfo.ftime >> 5) & 63);
        debugf(" %9u ", Finfo.fsize);
        CDCprintln(" %-12s %s", Finfo.fname,
#if _USE_LFN
                Lfname);
#else
                "");
#endif
*/
    }

    return s1;
}
#endif


//#endif /* __FILEIO_C__ */
