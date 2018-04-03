/*  --------------------------------------------------------------------
    BMP280.c
    
    Bosch BMP280 pressure sensor library
    This library uses I2C or SPI connection.

    Uses floating-point equations from BMP280 datasheet.
    --------------------------------------------------------------------
    CHANGELOG:
    16-09-2014 - Mhafuzul Islam -   initial version
    20-01-2017 - Dave Maners    -   adpated for Pinguino
    23-01-2017 - Régis Blanchot -   fixed
    25-01-2017 - Régis Blanchot -   added SPI communication
    25-01-2017 - Régis Blanchot -   added PIC32MX support
    13-12-2017 - Régis Blanchot -   replaced floats with integers
    14-12-2017 - Régis Blanchot -   fixed different bugs
    --------------------------------------------------------------------
    TODO:
    * Check that BMP280 SPI max is always < 10MHz
    --------------------------------------------------------------------
    Our example code uses the "pizza-eating" license. You can do anything
    you like with this code. No really, anything. If you find it useful,
    buy me (Mhafuzul Islam) italian pizza someday.
    ------------------------------------------------------------------*/

#ifndef __BMP280_C
#define __BMP280_C

//#ifndef WIRE
//#define WIRE
//#endif

#ifndef __PIC32MX__
#include <compiler.h>
#endif
#include <typedef.h>
#include <macro.h>
#include <const.h>
#include <stdarg.h>

#include <bmp280.h>

#if defined(BMP280I2C1ENABLE) || defined(BMP280I2C2ENABLE)
#include <i2c.h>
#include <i2c.c>
#endif

#if defined(BMP280SPISWENABLE) || \
    defined(BMP280SPI1ENABLE)  || defined(BMP280SPI2ENABLE)
#include <spi.h>
#include <spi.c>
#endif

#if defined(__PIC32MX__)
#include <digitalw.c>
#include <delay.c>
#else
#include <digitalw.c>
#include <digitalp.c>
#include <delayms.c>
#endif

#ifdef __XC8__
    #ifndef FASTLOG
    #define FASTLOG
    #endif
    #ifndef FASTEXP
    #define FASTEXP
    #endif
    #ifndef FASTPOW
    #define FASTPOW
    #endif
    #include <fastmath.c>
#else
    #include <math.h>
#endif

#if defined(BMP280DEBUGSERIAL)
    #ifndef SERIALPRINTX
    #define SERIALPRINTX
    #endif
    #ifndef SERIALPRINT
    #define SERIALPRINT
    #endif
    #include <serial.c>
    #if defined(__PIC32MX__)
    #define Serial_printX SerialPrintX
    #define Serial_print SerialPrint
    #endif
#endif

u16 gDig_T1;
s16 gDig_T2, gDig_T3;

u16 gDig_P1;
s16 gDig_P2, gDig_P3, gDig_P4, gDig_P5;
s16 gDig_P6, gDig_P7, gDig_P8, gDig_P9; 

// gFT carries fine temperature as global value
u32 gUT, gUP;
s32 gFT, gP, gT, gA;

u8 gOK, gError, gOversampling = 255;

#if defined(BMP280I2C1ENABLE) || defined(BMP280I2C2ENABLE)
int gBMP280I2CADDR;
#endif

/*  --------------------------------------------------------------------
    Initialize library and coefficient for measurements
    SPI up to 10 MHz
    I2C up to 3.4 MHz
    ------------------------------------------------------------------*/

u8 BMP280_begin(int module, ...)
{
    u8 id;

    #if defined(BMP280SPISWENABLE)
    int sdo, sdi, sck, cs;
    #endif
    
    va_list args;
    
    #if defined(BMP280DEBUGSERIAL)
    #if defined(__PIC32MX__)
    SerialConfigure(SERIALPORT, UART_ENABLE, UART_RX_TX_ENABLED, 9600);
    #else
    Serial_begin(SERIALPORT, 9600, NULL);
    #endif
    #endif

    va_start(args, module); // args points on the argument after module

    #if defined(BMP280SPISWENABLE)
    
    sdo = va_arg(args, int);            // get the next arg
    //sdi = va_arg(args, int);          // get the next arg
    sck = va_arg(args, int);            // get the next arg
    cs  = va_arg(args, int);            // get the last arg
    SPI_setBitOrder(module, SPI_MSBFIRST);
    SPI_begin(module, sdo, sck, cs);

    #elif defined(BMP280SPI1ENABLE) || defined(BMP280SPI2ENABLE)

    // Note : the 4-wire mode is used by default
    // The 3-wire mode can be selected by setting the spi3w_en bit to 1
    // (Bit 0 of register 0xF5 = BMP280_REG_CONFIG)

    SPI_setMode(module, SPI_MASTER);
    SPI_setDataMode(module, SPI_MODE1);
    //BMP280 maximum baud rate possible = 10MHz
    #if defined(__PIC32MX__)
    //FPB/4 = 10MHz
    SPI_setClockDivider(module, SPI_PBCLOCK_DIV4);
    #else
    //FOSC/8 = 8 MHz < 10 MHz
    SPI_setClockDivider(module, SPI_CLOCK_DIV8);
    #endif
    SPI_begin(module, NULL);

    #elif defined(BMP280I2C1ENABLE) || defined(BMP280I2C2ENABLE)
    
    gBMP280I2CADDR = va_arg(args, int);       // get the next arg
    //if (gBMP280I2CADDR == 0)
    //    gBMP280I2CADDR = 0x76;
     
    I2C_master(module, I2C_100KHZ);
    //I2C_master(module, I2C_400KHZ);
    //I2C_master(module, I2C_1MHZ);

    #endif

    va_end(args);                       // cleans up the list
    
    //Power-on-reset
    //BMP280_write8(module, BMP280_REG_RESET, 0xB6);

    id = BMP280_read8(module, BMP280_REG_ID);
    if (id == 0x58 || id == 0x60)       // BMP280 or BME280
    {
        BMP280_readCalibration(module);
        return 1;
    }

    #if BMP280DEBUGSERIAL
    Serial_print(SERIALPORT, "Error : init\r\n");
    #endif
    return 0;
}

/*
** Write a byte to device
** @param : reg = register to address
** @param : val = data to write
** Note : address is not auto-incremented
** The control bytes consist of the SPI register address
** (= full register address without bit 7) and the write
** command (bit7 = RW = ‘0’).
** SPI returns 0x00 when no error
** I2C returns 0x00 when an error occurs
*/

u8 BMP280_write8(u8 module, u8 reg, u8 val)
{
    #if defined(BMP280SPISWENABLE) || \
        defined(BMP280SPI1ENABLE)  || defined(BMP280SPI2ENABLE)

    SPI_select(module);
    gOK = !SPI_write(module, BitClear(reg, 7));// write, bit 7 = 0
    if (gOK) SPI_write(module, val);
    SPI_deselect(module);
    
    #elif defined(BMP280I2C1ENABLE) || defined(BMP280I2C2ENABLE)

    I2C_start(module);          // send start condition
    gOK = I2C_writeChar(module, (gBMP280I2CADDR << 1) & 0xFE);
    if (gOK) gOK = I2C_writeChar(module, reg);
    if (gOK) gOK = I2C_writeChar(module, val);
    I2C_stop(module);           // send stop condition

    #endif

    #if BMP280DEBUGSERIAL
    if (!gOK) Serial_print(SERIALPORT, "Error : write8\r\n");
    #endif

    return gOK;
}

/* 
** Read a byte from device
** @param  : reg = register to read
** Note : the control bytes consist of the SPI register address
** (= full register address without bit 7) and the read command
** (bit 7 = RW = ‘1’).
*/

u8 BMP280_read8(u8 module, u8 reg)
{
    u8 val;
    
    #if defined(BMP280SPISWENABLE) || \
        defined(BMP280SPI1ENABLE)  || defined(BMP280SPI2ENABLE)

    SPI_select(module);
    gOK = !SPI_write(module, BitSet(reg, 7));// read, bit 7 = 1
    if (gOK) val = SPI_read(module);
    SPI_deselect(module);

    #elif defined(BMP280I2C1ENABLE) || defined(BMP280I2C2ENABLE)

    I2C_start(module);
    I2C_writeChar(module, (gBMP280I2CADDR << 1) & 0xFE);
    I2C_writeChar(module, reg);
    //I2C_stop(module);
    I2C_start(module);
    I2C_writeChar(module, (gBMP280I2CADDR << 1) | 0x01);
    val = I2C_readChar(module);
    I2C_sendNack(module);       // last byte is sent
    I2C_stop(module);

    #endif

    #if BMP280DEBUGSERIAL
    if (gOK)
        Serial_printX(SERIALPORT, "read8=", val, HEX);
    else
        Serial_print(SERIALPORT, "Error : read8\r\n");
    #endif

    return val;
}

/* 
** Read an unsigned integer (two bytes) from device
** @param : reg = register to start reading (plus subsequent register)
** The control bytes consist of the SPI register address
** (= full register address without bit 7) and the read command
** (bit 7 = RW = ‘1’).
** Note that LSB is read first (Little Endian)
*/

u16 BMP280_read16(u8 module, u8 reg)
{
    u8 lsb, msb;
    
    #if defined(BMP280SPISWENABLE) || \
        defined(BMP280SPI1ENABLE)  || defined(BMP280SPI2ENABLE)

    SPI_select(module);
    gOK = !SPI_write(module, BitSet(reg, 7));// read, bit 7 = 1
    if (gOK)
    {
        lsb = SPI_read(module);
        msb = SPI_read(module);
    }
    SPI_deselect(module);

    #elif defined(BMP280I2C1ENABLE) || defined(BMP280I2C2ENABLE)

    I2C_start(module);
    I2C_writeChar(module, (gBMP280I2CADDR << 1) & 0xFE);
    I2C_writeChar(module, reg);
    //I2C_stop(module);

    I2C_start(module);
    I2C_writeChar(module, (gBMP280I2CADDR << 1) | 0x01);
    lsb = I2C_readChar(module);
    I2C_sendAck(module);        // not the last byte
    msb = I2C_readChar(module);
    I2C_sendNack(module);       // last byte to read
    I2C_stop(module);

    #endif

    #if BMP280DEBUGSERIAL
    if (gOK)
    {
        //Serial_printX(SERIALPORT, "read16 MSB = ", msb, HEX);
        //Serial_printX(SERIALPORT, "read16 LSB = ", lsb, HEX);
        Serial_printX(SERIALPORT, "read16 = ", (s16)((msb<<8)|lsb), DEC);
    }
    else
        Serial_print(SERIALPORT, "Error : read16\r\n");
    #endif

    return ((msb << 8) | lsb);
}

/*
** Read an array of bytes from device
** @param : value  = external array to hold data.
** Note : start register is in values[0].
** @param : length = number of bytes to read
** The control bytes consist of the SPI register address
** (= full register address without bit 7) and the read command
** (bit 7 = RW = ‘1’).
*/

u8 BMP280_readBytes(u8 module, u8 *values, u8 reg, u8 length)
{
    u8 x;

    #if defined(BMP280SPISWENABLE) || \
        defined(BMP280SPI1ENABLE)  || defined(BMP280SPI2ENABLE)

    SPI_select(module);
    gOK = !SPI_write(module, BitSet(reg, 7));
    if (gOK)
    {
        for (x=0; x<length; x++)
            values[x] = SPI_read(module);
    }
    SPI_deselect(module);

    #elif defined(BMP280I2C1ENABLE) || defined(BMP280I2C2ENABLE)

    I2C_start(module);          // send start condition
    I2C_writeChar(module, (gBMP280I2CADDR << 1) & 0xFE);
    gOK = I2C_writeChar(module, reg);
    //I2C_stop(module);           // send stop condition

    if (gOK)
    {
        I2C_start(module);      // send start condition
        I2C_writeChar(module, (gBMP280I2CADDR << 1) | 0x01);
        for (x=0; x<length; x++)
        {
            values[x] = I2C_readChar(module);
            #if BMP280DEBUGSERIAL
            Serial_printX(SERIALPORT, "data = ", values[x], DEC);
            #endif
            // Send a Ack or a Nack if it's the last byte
            (x == length - 1) ? I2C_sendNack(module) : I2C_sendAck(module);
        }
    }
    I2C_stop(module);       // send stop condition

    #endif

    #if BMP280DEBUGSERIAL
    if (!gOK)
        Serial_print(SERIALPORT, "Error : readBytes\r\n");
    #endif
    return gOK;
}

// The BMP280 includes factory calibration data stored on the device.
// Each device has different numbers, these must be retrieved and
// used in the calculations when taking measurements.

// Retrieve calibration data from device:
// Replace with a burst read from 0x88 to 0x9F ?
// BMP280_readBytes(module, dig, 0x88, 24);
void BMP280_readCalibration(u8 module)
{
    gDig_T1 = BMP280_read16(module, 0x88);
    gDig_T2 = (s16)BMP280_read16(module, 0x8A);
    gDig_T3 = (s16)BMP280_read16(module, 0x8C);
    
    gDig_P1 = BMP280_read16(module, 0x8E);
    gDig_P2 = (s16)BMP280_read16(module, 0x90);
    gDig_P3 = (s16)BMP280_read16(module, 0x92);
    gDig_P4 = (s16)BMP280_read16(module, 0x94);
    gDig_P5 = (s16)BMP280_read16(module, 0x96);
    gDig_P6 = (s16)BMP280_read16(module, 0x98);
    gDig_P7 = (s16)BMP280_read16(module, 0x9A);
    gDig_P8 = (s16)BMP280_read16(module, 0x9C);
    gDig_P9 = (s16)BMP280_read16(module, 0x9E);
}

/*
**  Get the uncalibrated pressure and temperature value.
**  gUP stores the uncalibrated pressure value (20bit)
**  gUT stores the uncalibrated temperature value (20bit)
*/
u8 BMP280_getUncalibrated(u8 module)
{
    u8 data[6];
    
    // burst read from 0xF7 to 0xFC
    gOK = BMP280_readBytes(module, data, BMP280_REG_RESULT, 6);
    if (gOK)
    {
        // gUT and gUP are 20-bit numbers stored in 3 bytes
        // from bit 23 to 4 so we shift the bits to the right
        //gUP = ((u32)data[0] << 16) | ((u32)data[1] << 8) | ((u32)data[2]);
        gUP  = data[0];
        gUP <<= 8;
        gUP |= data[1];
        gUP <<= 8;
        gUP |= data[2];
        gUP >>= 4;
        //gUT = ((u32)data[3] << 16) | ((u32)data[4] << 8) | ((u32)data[5]);
        gUT  = data[3];
        gUT <<= 8;
        gUT |= data[4];
        gUT <<= 8;
        gUT |= data[5];
        gUT >>= 4;

        #if BMP280DEBUGSERIAL
        Serial_printX(SERIALPORT, "gUT = ", gUT, DEC);
        Serial_printX(SERIALPORT, "gUP = ", gUP, DEC);
        #endif
    }

    #if BMP280DEBUGSERIAL
    if (!gOK) Serial_print(SERIALPORT, "Error : getUncalibrated\r\n");
    #endif
    return gOK;
}

/*
** Begin a measurement cycle
** Oversampling: 0 to 4
** The higher the number, the higher the resolution
** the higher the resolution, the slower the calculation
** @returns : delay in ms to wait, or 0 if I2C error.
*/

u8 BMP280_startMeasurment(u8 module, u8 oversampling)
{
    // Bit 7, 6, 5 = osrs_t[2:0] Controls oversampling of temperature data
    // 001                   ×1  16 bit / 0.0050 °C
    // 010                   ×2  17 bit / 0.0025 °C
    // 011                   ×4  18 bit / 0.0012 °C
    // 100                   ×8  19 bit / 0.0006 °C
    // 101, 110, 111         ×16 20 bit / 0.0003 °C

    // Bit 4, 3, 2 = osrs_p[2:0] Controls oversampling of pressure data
    // Ultra low power       ×1  16 bit / 2.62 Pa (recommended T. os ×1)
    // Low power             ×2  17 bit / 1.31 Pa (                  ×1)
    // Standard resolution   ×4  18 bit / 0.66 Pa (                  ×1)
    // High resolution       ×8  19 bit / 0.33 Pa (                  ×1)
    // Ultra high resolution ×16 20 bit / 0.16 Pa (                  ×2)

    // Bit 1, 0    = mode[1:0]   Controls the power mode of the device
    // 00 Sleep mode
    // 01 and 10 Forced mode
    // 11 Normal mode
    
    // 0x25 = 0b 001 001 01
    // 0x29 = 0b 001 010 01
    // 0x2D = 0b 001 011 01
    // 0x31 = 0b 001 100 01 
    // 0x5D = 0b 010 111 01
    
    // BMP280_COMMAND_PRESSURE0 to 4
    u8 val[5] = {0x25, 0x29, 0x2D, 0x31, 0x5D};

    if (oversampling > 4)
        oversampling = 0;
    gOversampling = oversampling;
    
    if (BMP280_write8(module, BMP280_REG_CONTROL, val[oversampling]))
    {
        // wait before retrieving data
        Delayms(10 * oversampling + 10);
        // get uncalibrated Temperature and Pressure
        BMP280_getUncalibrated(module);
        return (1);
    }
    else
    {
        #if BMP280DEBUGSERIAL
        Serial_print(SERIALPORT, "Error : startMeasurment\r\n");
        #endif
        return (0); // or return 0 if there was a problem communicating with the BMP
    }
}

/*
** Retrieve temperature and pressure.
** @param : T = stores the temperature value in degC.
** @param : P = stores the pressure value in mBar.
*/

void BMP280_getTemperatureAndPressure(u8 module)
{
    // In case users forget to use the startMeasurment function ...
    if (gOversampling > 4)
        BMP280_startMeasurment(module, 0);
    BMP280_getCalibratedTemperature();
    BMP280_getCalibratedPressure();
}

/*
** Temperature calculation
** @param  : gUT, the uncalibrated temperature value.
** @return : gT, the temperature value after calculation.
** Temperature in DegC, resolution is 0.01 DegC.
** Output value of “2123” equals 21.23 DegC.
*/

void BMP280_getCalibratedTemperature()
{
    s32 var1, var2;
    
    var1 = ((s32)gUT >> 3);
    var1 -= ((s32)gDig_T1 << 1);
    var1 *= ((s32)gDig_T2);
    var1 >>= 11;
    
    var2 = ((s32)gUT >> 4);
    var2 -= ((s32)gDig_T1);
    var2 = var2 * var2;
    var2 >>= 12;
    var2 *= ((s32)gDig_T3);
    var2 >>= 14;

    gFT = (s32)(var1 + var2);
    gT = (gFT * 5 + 128) >> 8;
}

/*
** Pressure calculation from uncalibrated pressure value.
** @param  : gUP, uncalibrated pressure value. 
** @return : gP, the pressure value.
** Pressure in Pa as unsigned 32 bit integer.
** Output value of “96386” equals 96386 Pa = 963.86 hPa
*/

void BMP280_getCalibratedPressure()
{
    s32 var1, var2;

    var1 = (gFT >> 1) - (s32)64000;
    var2 = (((var1 >> 2) * (var1 >> 2)) >> 11 ) * (s32)gDig_P6;
    var2 = var2 + var1 * ((s32)gDig_P5) << 1;
    var2 = (var2 >> 2) + ((s32)gDig_P4 << 16);
    var1 = (((s32)gDig_P3 * (((var1 >> 2) * (var1 >> 2)) >> 13 )) >> 3) + (((s32)gDig_P2 * var1) >> 1) >> 18;
    var1 =((((32768 + var1))*((s32)gDig_P1))>>15);

    // avoid exception caused by division by zero
    if (var1 == 0)
        return;

    gP = (((u32)(((s32)1048576) - (s32)gUP) - (var2 >> 12))) * 3125;

    if (gP < 0x80000000)
        gP = (gP << 1) / ((u32)var1);
    else
        gP = (gP / (u32)var1) << 1;

    var1 = ((s32)gDig_P9 * ((s32)(((gP >> 3) * (gP >> 3)) >> 13))) >> 12;
    var2 = ((s32)(gP >> 2)) * (s32)gDig_P8;
    var2 = var2 >> 13;
    gP = (u32)((s32)gP + ((var1 + var2 + (s32)gDig_P7) >> 4));
}

/*
** User's functions
*/
 
s32 BMP280_getTemperatureCelsius(u8 module)
{
    BMP280_getTemperatureAndPressure(module);
    return gT;
}

s32 BMP280_getTemperatureFahrenheit(u8 module)
{
    s32 ti;
    u32 tf;
    
    BMP280_getTemperatureAndPressure(module);

    ti = (gT/100) * 9 / 5 + 32;
    //if (gT < 0)
    //tf = (-gT%100) * 9 / 5;
    //else
    tf = (gT%100) * 9 / 5;
    
    return (ti * 100 + tf);
}

u32 BMP280_getPressure(u8 module)
{
    BMP280_getTemperatureAndPressure(module);
    return gP;
}

// Given a pressure P (mb) taken at a specific altitude (meters),
// return the equivalent pressure (mb) at sea level.
// This produces pressure readings that can be used for weather measurements.
float BMP280_getPressureAtSealevel(u8 module, float P, float A)
{
    float K = 1.0 - A / 44330.0;
    
    #ifdef __XC8__
    return (P / fastpow(K, 5.255));
    #else
    return (P / powf(K, 5.255));
    #endif
}

// Given a pressure measurement P (Pa)
// and the pressure at a baseline P0 (Pa),
// return altitude (meters) above baseline.
s32 BMP280_getAltitude(u8 module, u32 P, u32 P0)
{
    float K = (float)P / (float)P0;
    
    #ifdef __XC8__
    return (s32)(44330.0 * (1 - fastpow(K, 1/5.255)));
    #else
    return (s32)(44330.0 * (1 - powf(K, 1/5.255)));
    #endif
}

#endif // __BMP280_C
