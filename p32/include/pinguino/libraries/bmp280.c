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
    --------------------------------------------------------------------
    TODO:
    * back from float to integer ?
    * Check that BMP280 SPI max is always < 10MHz
    --------------------------------------------------------------------
    Our example code uses the "pizza-eating" license. You can do anything
    you like with this code. No really, anything. If you find it useful,
    buy me (Mhafuzul Islam) italian pizza someday.
    ------------------------------------------------------------------*/

#ifndef __BMP280_C
#define __BMP280_C

#ifndef WIRE
#define WIRE
#endif

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

#if defined(BMP280SPISWENABLE) || defined(BMP280SPI1ENABLE) || defined(BMP280SPI2ENABLE)
#include <spi.h>
#include <spi.c>
#endif

#ifndef __PIC32MX__
#include <digitalw.c>
#include <digitalp.c>
#include <delayms.c>
#else
#include <digitalw.c>
#include <delay.c>
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

#if defined(BMP280DEBUGSERIAL) || defined(BMP280DEBUGTESTDATA)
    #ifndef SERIALPRINT
    #define SERIALPRINT
    #endif
    //#ifndef SERIALPRINTF
    //#define SERIALPRINTF
    //#endif
    //#ifndef SERIALPRINTLN
    //#define SERIALPRINTLN
    //#endif 
    //#ifndef SERIALPRINTNUMBER
    //#define SERIALPRINTNUMBER
    //#endif
    #ifndef SERIALPRINTFLOAT
    #define SERIALPRINTFLOAT
    #endif
    #include <serial.c> 
#endif

//int gDig_T2 , gDig_T3 , gDig_T4 , gDig_P2 , gDig_P3, gDig_P4, gDig_P5, gDig_P6, gDig_P7, gDig_P8, gDig_P9; 
//unsigned int gDig_P1 , gDig_T1 ;
float gDig_T1, gDig_T2 , gDig_T3, gDig_T4;
float gDig_P1, gDig_P2 , gDig_P3, gDig_P4, gDig_P5, gDig_P6, gDig_P7, gDig_P8, gDig_P9; 
float gT_fine, gT, gP, gA, gUT, gUP;

u8 error;

/*  --------------------------------------------------------------------
    Initialize library and coefficient for measurements
    ------------------------------------------------------------------*/

u8 BMP280_begin(int module, ...)
{
    int sda, sdi, sck, cs;
    va_list args;
    
    #if defined(BMP280DEBUGSERIAL) || defined(BMP280DEBUGTESTDATA)
    Serial_begin(9600);
    #endif

    va_start(args, module); // args points on the argument after module

    #if defined(BMP280SPISWENABLE)
    sda = va_arg(args, int);         // get the next arg
    //sdi = va_arg(args, int);         // get the next arg
    sck = va_arg(args, int);         // get the next arg
    cs  = va_arg(args, int);         // get the last arg
    SPI_setBitOrder(module, SPI_MSBFIRST);
    SPI_begin(module, sda, sck, cs);
    #endif

    // TODO : BMP280 SPI max is 10MHz
    #if defined(BMP280SPI1ENABLE) || defined(BMP280SPI2ENABLE)
    SPI_setMode(module, SPI_MASTER);
    SPI_setDataMode(module, SPI_MODE1);
    #ifndef __PIC32MX__
    //maximum baud rate possible = FPB = FOSC/4
    SPI_setClockDivider(module, SPI_CLOCK_DIV4);
    #else
    //maximum baud rate possible = FPB/2
    SPI_setClockDivider(module, SPI_PBCLOCK_DIV2);
    #endif
    SPI_begin(module);
    #endif

    #if defined(BMP280I2C1ENABLE) || defined(BMP280I2C2ENABLE)
    Wire_begin(module, BMP280_I2CADDR, I2C_100KHZ);
    #endif

    va_end(args);                       // cleans up the list
    
    return (BMP280_readCalibration(module));
}

// The BMP280 includes factory calibration data stored on the device.
// Each device has different numbers, these must be retrieved and
// used in the calculations when taking measurements.

// Retrieve calibration data from device:
u8 BMP280_readCalibration(u8 module)
{
    if (BMP280_readUInt(module, 0x88, &gDig_T1) &&
        BMP280_readInt(module, 0x8A, &gDig_T2)  &&
        BMP280_readInt(module, 0x8C, &gDig_T3)  &&
        BMP280_readUInt(module, 0x8E, &gDig_P1) &&
        BMP280_readInt(module, 0x90, &gDig_P2)  &&
        BMP280_readInt(module, 0x92, &gDig_P3)  &&
        BMP280_readInt(module, 0x94, &gDig_P4)  &&
        BMP280_readInt(module, 0x96, &gDig_P5)  &&
        BMP280_readInt(module, 0x98, &gDig_P6)  &&
        BMP280_readInt(module, 0x9A, &gDig_P7)  &&
        BMP280_readInt(module, 0x9C, &gDig_P8)  &&
        BMP280_readInt(module, 0x9E, &gDig_P9))
        {
        #ifdef BMP280DEBUGSERIAL
        Serial_print("gDig_T1="); Serial_printFloat(gDig_T1,2); Serial_print("\r\n");
        Serial_print("gDig_T2="); Serial_printFloat(gDig_T2,2); Serial_print("\r\n");
        Serial_print("gDig_T3="); Serial_printFloat(gDig_T3,2); Serial_print("\r\n");
        Serial_print("gDig_P1="); Serial_printFloat(gDig_P1,2); Serial_print("\r\n");
        Serial_print("gDig_P2="); Serial_printFloat(gDig_P2,2); Serial_print("\r\n");
        Serial_print("gDig_P3="); Serial_printFloat(gDig_P3,2); Serial_print("\r\n");
        Serial_print("gDig_P4="); Serial_printFloat(gDig_P4,2); Serial_print("\r\n");
        Serial_print("gDig_P5="); Serial_printFloat(gDig_P5,2); Serial_print("\r\n");
        Serial_print("gDig_P6="); Serial_printFloat(gDig_P6,2); Serial_print("\r\n");
        Serial_print("gDig_P7="); Serial_printFloat(gDig_P7,2); Serial_print("\r\n");
        Serial_print("gDig_P8="); Serial_printFloat(gDig_P8,2); Serial_print("\r\n");
        Serial_print("gDig_P9="); Serial_printFloat(gDig_P9,2); Serial_print("\r\n");
        #endif
        #ifdef BMP280DEBUGTESTDATA
        gDig_T1 = 27504.0;
        gDig_T2 = 26435.0;
        gDig_T3 = -1000.0;
        gDig_P1 = 36477.0;
        gDig_P2 = -10685.0;
        gDig_P3 = 3024.0;
        gDig_P4 = 2855.0;
        gDig_P5 = 140.0;
        gDig_P6 = -7.0;
        gDig_P7 = 15500.0;
        gDig_P8 = -14600.0;
        gDig_P9 = 6000.0;
        Serial_print("gDig_T1="); Serial_printFloat(gDig_T1,2); Serial_print("\r\n");
        Serial_print("gDig_T2="); Serial_printFloat(gDig_T2,2); Serial_print("\r\n");
        Serial_print("gDig_T3="); Serial_printFloat(gDig_T3,2); Serial_print("\r\n");
        Serial_print("gDig_P1="); Serial_printFloat(gDig_P1,2); Serial_print("\r\n");
        Serial_print("gDig_P2="); Serial_printFloat(gDig_P2,2); Serial_print("\r\n");
        Serial_print("gDig_P3="); Serial_printFloat(gDig_P3,2); Serial_print("\r\n");
        Serial_print("gDig_P4="); Serial_printFloat(gDig_P4,2); Serial_print("\r\n");
        Serial_print("gDig_P5="); Serial_printFloat(gDig_P5,2); Serial_print("\r\n");
        Serial_print("gDig_P6="); Serial_printFloat(gDig_P6,2); Serial_print("\r\n");
        Serial_print("gDig_P7="); Serial_printFloat(gDig_P7,2); Serial_print("\r\n");
        Serial_print("gDig_P8="); Serial_printFloat(gDig_P8,2); Serial_print("\r\n");
        Serial_print("gDig_P9="); Serial_printFloat(gDig_P9,2); Serial_print("\r\n");
        #endif
        return (1);
    }
    else 
        return (0);
}

/*
**	Read a signed integer (two bytes) from device
**	@param : address = register to start reading (plus subsequent register)
**	@param : value   = external variable to store data (function modifies value)
*/

u8 BMP280_readInt(u8 module, u8 address, float *value)
{
    u8 data[2];	//u8 is 4bit, 1byte
    
    data[0] = address;
    if (BMP280_readBytes(module, data, 2))
    {
        value = (float*)(signed int)(((unsigned int)data[1]<<8)|(unsigned int)data[0]);
        return(1);
    }
    value = 0;
    return(0);
}

/* 
**	Read an unsigned integer (two bytes) from device
**	@param : address = register to start reading (plus subsequent register)
**	@param : value 	 = external variable to store data (function modifies value)
*/

u8 BMP280_readUInt(u8 module, u8 address, float *value)
{
    u8 data[2];	//4bit
     
    data[0] = address;
    if (BMP280_readBytes(module, data,2))
    {
        value = (float*)(unsigned int)(((unsigned int)data[1]<<8)|(unsigned int)data[0]);
        return(1);
    }
    value = 0;
    return(0);
}

/*
** Read an array of bytes from device
** @param : value  = external array to hold data. Put starting register in values[0].
** @param : length = number of bytes to read
*/

u8 BMP280_readBytes(u8 module, u8 *values, u8 length)
{
    u8 x;

    #if defined(BMP280SPISWENABLE) || defined(BMP280SPI1ENABLE) || defined(BMP280SPI2ENABLE)
    error = SPI_write(module, values[0]);

    if (error == 0)
    {
        for(x=0; x<length; x++)
            values[x] = SPI_read(module);
        return (1);
    }
    #endif

    #if defined(BMP280I2C1ENABLE) || defined(BMP280I2C2ENABLE)
    Wire_beginTransmission(module, BMP280_I2CADDR);
    Wire_writeChar(module, values[0]);
    error = Wire_endTransmission(module, true);
    
    if (error == 0)
    {
        Wire_requestFrom(module, BMP280_I2CADDR, length);
        while(Wire_available(module) != length); // wait until bytes are ready
        for (x=0; x<length; x++)
            values[x] = Wire_readChar(module);
        return(1);
    }
    #endif

    return(0);
}

/*
** Write an array of bytes to device
** @param : values = external array of data to write. Put starting register in values[0].
** @param : length = number of bytes to write
*/

u8 BMP280_writeBytes(u8 module, u8 *values, u8 length)
{
    #if defined(BMP280SPISWENABLE) || defined(BMP280SPI1ENABLE) || defined(BMP280SPI2ENABLE)
    u8 x;
    for (x=0; x<length; x++)
        error = SPI_write(module, values[x]);
    #endif

    #if defined(BMP280I2C1ENABLE) || defined(BMP280I2C2ENABLE)
    Wire_beginTransmission(module, BMP280_I2CADDR);
    Wire_writeCharS(module, values, length);
    error = Wire_endTransmission(module, true);
    #endif

    return (!error);
}

#define BMP280_getOversampling()    (oversampling)
#define BMP280_setOversampling(oss)  oversampling = oss


/*
**	Begin a measurement cycle.
** Oversampling: 0 to 4, higher numbers are slower, higher-res outputs.
** @returns : delay in ms to wait, or 0 if I2C error.
*/

u8 BMP280_startMeasurment(u8 module, u8 oversampling)
{
    u8 data[2], d;
    
    data[0] = BMP280_REG_CONTROL;

    switch (oversampling)
    {
        case 0:
            data[1] = BMP280_COMMAND_PRESSURE0;
            d = 8;
            break;
        case 1:
            data[1] = BMP280_COMMAND_PRESSURE1;
            d = 10;
            break;
        case 2:
            data[1] = BMP280_COMMAND_PRESSURE2;
            d = 15;
            break;
        case 3:
            data[1] = BMP280_COMMAND_PRESSURE3;
            d = 24;
            break;
        case 4:
            data[1] = BMP280_COMMAND_PRESSURE4;
            d = 45;
            break;
        default:
            data[1] = BMP280_COMMAND_PRESSURE0;
            d = 9;
            break;
    }

    if (BMP280_writeBytes(module, data, 2)) // good write?
    {
        Delayms(d); // wait before retrieving data
        return (1);
    }
    else
    {
        return (0); // or return 0 if there was a problem communicating with the BMP
    }
}

/*
**	Get the uncalibrated pressure and temperature value.
**  @param : gUP = stores the uncalibrated pressure value.(20bit)
**  @param : gUT = stores the uncalibrated temperature value.(20bit)
*/
u8 BMP280_getUnPT(u8 module)
{
    u8 data[6];
    u8 result;
    
    data[0] = BMP280_REG_RESULT_PRESSURE;   // 0xF7 

    result = BMP280_readBytes(module, data, 6);     // from 0xF7 to 0xFC
    if (result) // good read
    {
        //float factor = pow(2, 4);
        gUP = (float)(data[0] * 4096 + data[1] * 16 + data[2] / 16) ;	//20bit UP
        gUT = (float)(data[3] * 4096 + data[4] * 16 + data[5] / 16) ;	//20bit UT
        #ifdef BMP280DEBUGSERIAL
        Serial_printFloat(gUT, 2);
        Serial_print(" ");
        Serial_printFloat(gUP, 2); 
        #endif
        #ifdef BMP280DEBUGTESTDATA
        gUT = 519888.0;
        gUP = 415148.0;
        Serial_printFloat(gUT, 2);
        Serial_print(" ");
        Serial_printFloat(gUP, 2); 
        #endif
    }
    return(result);
}

float BMP280_getTemperatureCelsius(u8 module)
{
    BMP280_getTemperatureAndPressure(module);
    return gT;
}

float BMP280_getTemperatureFahrenheit(u8 module)
{
    BMP280_getTemperatureAndPressure(module);
    return (gT*9/5+32);
}

float BMP280_getPressure(u8 module)
{
    BMP280_getTemperatureAndPressure(module);
    return gP;
}

// Given a pressure P (mb) taken at a specific altitude (meters),
// return the equivalent pressure (mb) at sea level.
// This produces pressure readings that can be used for weather measurements.
float BMP280_getPressureAtSealevel(u8 module, float P, float A)
{
    #ifdef __XC8__
    return(P/fastpow(1-(A/44330.0),5.255));
    #else
    return(P/powf(1-(A/44330.0),5.255));
    #endif
}

// Given a pressure measurement P (mb) and the pressure at a baseline P0 (mb),
// return altitude (meters) above baseline.
float BMP280_getAltitude(u8 module, float P, float baseline)
{
    #ifdef __XC8__
    return(44330.0*(1-fastpow(P/baseline,1/5.255)));
    #else
    return(44330.0*(1-powf(P/baseline,1/5.255)));
    #endif
}


/*
** Retrieve temperature and pressure.
** @param : T = stores the temperature value in degC.
** @param : P = stores the pressure value in mBar.
*/

u8 BMP280_getTemperatureAndPressure(u8 module)
{
    // calculate gUT and gUP
    if(BMP280_getUnPT(module))
    {
        // calculate the temperature gT and gT_fine
        if (BMP280_calcTemperature())
        {
            // calculate the pressure gP
            if (BMP280_calcPressure())
                return (1);
            else
                error = 3 ;	// pressure error ;
            return (9);
        }
        else 
            error = 2;	// temperature error ;
    }
    else 
        error = 1;

    return (9);
}

/*
** temperature calculation
** @param : T  = stores the temperature value after calculation.
** @param : gUT = the uncalibrated temperature value.
*/

u8 BMP280_calcTemperature()
{
    float var1 = (gUT/16384.0 - gDig_T1/1024.0)*gDig_T2;
    float var2 = ((gUT/131072.0 - gDig_T1/8192.0)*(gUT/131072.0 - gDig_T1/8192.0))*gDig_T3;

    gT_fine = var1 + var2;
    gT = gT_fine/(float)5120.0;
    #ifdef BMP280DEBUGSERIAL
    Serial_printFloat(var1, 2);
    Serial_print(" ");
    Serial_printFloat(var2, 2);
    Serial_print(" ");
    Serial_printFloat(gT_fine, 2);
    Serial_print(" ");
    Serial_printFloat(gT, 2);
    #endif
    
    if(gT>100 || gT <-100)
        return 0;
    
    return (1);
}

/*
**	Pressure calculation from uncalibrated pressure value.
**  @param : P  = stores the pressure value.
**  @param : gUP = uncalibrated pressure value. 
*/

u8 BMP280_calcPressure()
{
    //u8 result;
    float var1 , var2 ;
    
    var1 = (gT_fine/2.0) - 64000.0;
    #ifdef BMP280DEBUGSERIAL
    Serial_print("var1 = ");Serial_printFloat(var1,2); Serial_print("\r\n");
    #endif

    var2 = var1 * (var1 * gDig_P6/32768.0);	//not overflow
    #ifdef BMP280DEBUGSERIAL
    Serial_print("var2 = ");Serial_printFloat(var2,2); Serial_print("\r\n");
    #endif

    var2 = var2 + (var1 * gDig_P5 * 2.0);	//overflow
    #ifdef BMP280DEBUGSERIAL
    Serial_print("var2 = ");Serial_printFloat(var2,2); Serial_print("\r\n");
    #endif
        
    var2 = (var2/4.0)+((gDig_P4)*65536.0);
    #ifdef BMP280DEBUGSERIAL
    Serial_print("var2 = ");Serial_printFloat(var2,2); Serial_print("\r\n");
    #endif
        
    var1 = (gDig_P3 * var1 * var1/524288.0 + gDig_P2 * var1) / 524288.0;
    #ifdef BMP280DEBUGSERIAL
    Serial_print("var1 = ");Serial_printFloat(var1,2); Serial_print("\r\n");
    #endif

    var1 = (1.0 + var1/32768.0) * gDig_P1;
    #ifdef BMP280DEBUGSERIAL
    Serial_print("var1 = ");Serial_printFloat(var1,2); Serial_print("\r\n");
    #endif
        
    gP = 1048576.0- gUP;
    #ifdef BMP280DEBUGSERIAL
    Serial_print("gP = ");Serial_printFloat(gP,2); Serial_print("\r\n");
    #endif
        
    gP = (gP-(var2/4096.0))*6250.0/var1 ;	//overflow
    #ifdef BMP280DEBUGSERIAL
    Serial_print("gP = ");Serial_printFloat(gP,2); Serial_print("\r\n");
    #endif
        
    var1 = gDig_P9*gP*gP/2147483648.0;	//overflow
    #ifdef BMP280DEBUGSERIAL
    Serial_print("var1 = ");Serial_printFloat(var1,2); Serial_print("\r\n");
    #endif

    var2 = gP*gDig_P8/32768.0;
    #ifdef BMP280DEBUGSERIAL
    Serial_print("var2 = ");Serial_printFloat(var2,2); Serial_print("\r\n");
    #endif

    gP = gP + (var1+var2+gDig_P7)/16.0;
    #ifdef BMP280DEBUGSERIAL
    Serial_print("gP = ");Serial_printFloat(gP,2); Serial_print("\r\n");
    #endif
        
    gP = gP/100.0 ;
    
    if(gP>1200.0 || gP < 800.0)
        return (0);

    return (1);
}

// If any library command fails, you can retrieve an extended
// error code using this command. Errors are from the wire library: 
// 0 = Success
// 1 = Data too long to fit in transmit buffer
// 2 = Received NACK on transmit of address
// 3 = Received NACK on transmit of data
// 4 = Other error
#define BMP280_getError()   (error)

#endif // __BMP280_C
