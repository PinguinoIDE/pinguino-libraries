#ifndef __SPI2_C__
#define __SPI2_C__

#ifndef __SPI2__
#define __SPI2__
#endif

#include <spi.h>
#include <spi.c>
#include <delayms.c>
#include <digitalp.c>
#include <digitalw.c>


#define SPI2_setPin(sda, sck)       SPI_setPin(2, sda, sck)
#define SPI2_select()               SPI_select(2)
#define SPI2_deselect()             SPI_deselect(2)
#define SPI2_begin()                SPI_begin(2)
#define SPI2_setBitOrder(bitorder)  SPI_setBitOrder(2, bitorder)
#define SPI2_setDataMode(mode)      SPI_setDataMode(2, mode)
#define SPI2_setMode(mode)          SPI_setMode(2, mode)
#define SPI2_setClockDivider(div)   SPI_setClockDivider(2, div)
#define SPI2_write(datax)           SPI_write(2, datax)
#define SPI2_read()                 SPI_write(2, 0xFF)

#endif // __SPI2_C__
