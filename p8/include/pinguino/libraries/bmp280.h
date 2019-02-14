/*
    BMP280.h
    Bosch BMP280 pressure sensor library for the Arduino microcontroller.
    This library uses I2C connection.

    Uses floating-point equations from BMP280 datasheet.

    modified by mhafuzul islam

    version 1.01		 16/9/2014 initial version

    Our example code uses the "pizza-eating" license. You can do anything
    you like with this code. No really, anything. If you find it useful,
    buy me italian pizza someday.
*/

#ifndef BMP280_H
#define BMP280_H

//#define BMP280DEBUGSERIAL1
//#define BMP280DEBUGSERIAL2

#if defined(BMP280DEBUGSERIAL1)
    #define SERIALUSEPORT1
    #define SERIALPORT UART1
    #define BMP280DEBUGSERIAL 1
#endif

#if defined(BMP280DEBUGSERIAL2)
    #define SERIALUSEPORT2
    #define SERIALPORT UART2
    #define BMP280DEBUGSERIAL 1
#endif

/*
#define BMP280SPISW     0
#define BMP280SPI1      1
#define BMP280SPI2      2
#define BMP280I2C1      1
#define BMP280I2C2      2
*/

u8 BMP280_begin(int, ...);
        
// Retrieve calibration data from device:
// The BMP280 includes factory calibration data stored on the device.
// Each device has different numbers, these must be retrieved and
// used in the calculations when taking measurements.
void BMP280_readCalibration(u8);

// write a char to a BMP280 register
// reg: register address
// value: value to write
// returns 1 for success, 0 for fail
//u8 BMP280_write8(u8, u8, u8);

// read a char from a BMP280 register
// address: register address
// returns value read or 0 for fail
//u8 BMP280_read8(u8, u8);

// read an unsigned 32 bits from a BMP280 register
u16 BMP280_read16(u8, u8);

// read a number of bytes from a BMP280 register
// values: array of u8 with register address in first location [0]
// length: number of bytes to read back
// returns 1 for success, 0 for fail, with read bytes in values[] array
//u8 BMP280_readBytes(u8, u8 *, u8, u8);
    
// command BMP280 to start a pressure measurement at a given precision
u8 BMP280_startMeasurment(u8, u8);

// get uncalibrated Pressure and Temperature value.
u8 BMP280_getUncalibrated(u8);

// calculate the true temperature from uncalibrated Temperature 
void BMP280_getCalibratedTemperature();

// calculate the true pressure from uncalibrated Pressure
void BMP280_getCalibratedPressure();

void BMP280_getTemperatureAndPressure(u8);
s32 BMP280_getTemperatureCelsius(u8);
s32 BMP280_getTemperatureFahrenheit(u8);
u32 BMP280_getPressure(u8);

// convert absolute pressure to sea-level pressure 
// returns sealevel pressure in mbar
float BMP280_getPressureAtSealevel(u8, float, float);

// convert absolute pressure to altitude (given baseline pressure; sea-level, runway, etc.)
// P: absolute pressure (mbar)
// P0: fixed baseline pressure (mbar)
// returns signed altitude in meters
s32 BMP280_getAltitude(u8, u32, u32);

#define BMP280_READ_FLAG                0x80

// REGISTERS
#define	BMP280_REG_ID                   0xD0
#define	BMP280_REG_RESET                0xE0
#define	BMP280_REG_STATUS               0xF3
#define	BMP280_REG_CONTROL              0xF4
#define	BMP280_REG_CONFIG               0xF5
#define	BMP280_REG_RESULT               0xF7
// 0xF7(msb) , 0xF8(lsb) , 0xF9(xlsb) : stores the pressure data.
#define	BMP280_REG_RESULT_TEMPERATURE   0xF7
// 0xFA(msb) , 0xFB(lsb) , 0xFC(xlsb) : stores the temperature data.
#define BMP280_REG_RESULT_PRESSURE      0xFA

#define	BMP280_COMMAND_TEMPERATURE      0x2E // 0b 001  011  10
#define	BMP280_COMMAND_PRESSURE0        0x25 // 0b 001  001  01
#define	BMP280_COMMAND_PRESSURE1        0x29 // 0b 001  010  01
#define	BMP280_COMMAND_PRESSURE2        0x2D // 0b 001  011  01
#define	BMP280_COMMAND_PRESSURE3        0x31 // 0b 001  100  01
#define	BMP280_COMMAND_PRESSURE4        0x5D // 0b 010  111  01 

#endif // BMP280_H
