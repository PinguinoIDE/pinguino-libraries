#ifndef __SPI1_C__
#define __SPI1_C__

#ifndef __SPI1__
#define __SPI1__
#endif

#include <spi.h>
#include <spi.c>
#include <delayms.c>
#include <digitalp.c>
#include <digitalw.c>


#define SPI1_setPin(sda, sck)       SPI_setPin(1, sda, sck)
#define SPI1_select()               SPI_select(1)
#define SPI1_deselect()             SPI_deselect(1)
#define SPI1_begin()                SPI_begin(1)
#define SPI1_setBitOrder(bitorder)  SPI_setBitOrder(1, bitorder)
#define SPI1_setDataMode(mode)      SPI_setDataMode(1, mode)
#define SPI1_setMode(mode)          SPI_setMode(1, mode)
#define SPI1_setClockDivider(div)   SPI_setClockDivider(1, div)
#define SPI1_write(datax)           SPI_write(1, datax)
#define SPI1_read()                 SPI_write(1, 0xFF)

#endif // __SPI1_C__
