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

//#define BMP280DEBUGSERIAL
//#define BMP280DEBUGTESTDATA

#define BMP280SPISW     0
#define BMP280SPI1      1
#define BMP280SPI2      2
#define BMP280I2C1      3
#define BMP280I2C2      4

u8 BMP280_begin(int, ...);
//u8 BMP280_begin(int sdaPin, int sclPin);
    // call pressure.begin() to initialize BMP280 before use
    // returns 1 if success, 0 if failure (i2C connection problem.)
        
// command BMP280 to start a pressure measurement at a given precision
u8 BMP280_startMeasurment(u8, u8);

u8  BMP280_getTemperatureAndPressure(u8);
float BMP280_getTemperatureCelsius(u8);
float BMP280_getTemperatureFahrenheit(u8);
float BMP280_getPressure(u8);
// convert absolute pressure to sea-level pressure 
// P: absolute pressure (mbar)
// A: current altitude (meters)
// returns sealevel pressure in mbar
float BMP280_getPressureAtSealevel(u8, float, float);

// convert absolute pressure to altitude (given baseline pressure; sea-level, runway, etc.)
// P: absolute pressure (mbar)
// P0: fixed baseline pressure (mbar)
// returns signed altitude in meters
float BMP280_getAltitude(u8, float, float);


// calculation the true temperature from the given uncalibrated Temperature 
u8 BMP280_calcTemperature();

//calculation for measuring pressure.
u8 BMP280_calcPressure();

u8 BMP280_readCalibration(u8);
// Retrieve calibration data from device:
// The BMP280 includes factory calibration data stored on the device.
// Each device has different numbers, these must be retrieved and
// used in the calculations when taking measurements.

// read an signed int (16 bits) from a BMP280 register
// address: BMP280 register address
// value: external signed int for returned value (16 bits)
// returns 1 for success, 0 for fail, with result in value
u8 BMP280_readInt(u8, u8, float*);

// read an unsigned int (16 bits) from a BMP280 register
// address: BMP280 register address
// value: external unsigned int for returned value (16 bits)
// returns 1 for success, 0 for fail, with result in value
u8 BMP280_readUInt(u8, u8, float*);

// read a number of bytes from a BMP280 register
// values: array of u8 with register address in first location [0]
// length: number of bytes to read back
// returns 1 for success, 0 for fail, with read bytes in values[] array
u8 BMP280_readBytes(u8, u8 *, u8);
    
// write a number of bytes to a BMP280 register (and consecutive subsequent registers)
// values: array of u8 with register address in first location [0]
// length: number of bytes to write
// returns 1 for success, 0 for fail
u8 BMP280_writeBytes(u8, u8 *, u8);

//get uncalibrated UP and UT value.
u8 BMP280_getUnPT(u8);

// If any library command fails, you can retrieve an extended
// error code using this command. Errors are from the wire library: 
// 0 = Success
// 1 = Data too long to fit in transmit buffer
// 2 = Received NACK on transmit of address
// 3 = Received NACK on transmit of data
// 4 = Other error
u8 BMP280_getError();
    
#define BMP280_I2CADDR 0x76 // 7-bit address

#define	BMP280_REG_CONTROL 0xF4
#define	BMP280_REG_RESULT_PRESSURE 0xF7			// 0xF7(msb) , 0xF8(lsb) , 0xF9(xlsb) : stores the pressure data.
#define BMP280_REG_RESULT_TEMPRERATURE 0xFA		// 0xFA(msb) , 0xFB(lsb) , 0xFC(xlsb) : stores the temperature data.

#define	BMP280_COMMAND_TEMPERATURE 0x2E
#define	BMP280_COMMAND_PRESSURE0 0x25  			 
#define	BMP280_COMMAND_PRESSURE1 0x29  			
#define	BMP280_COMMAND_PRESSURE2 0x2D    
#define	BMP280_COMMAND_PRESSURE3 0x31    
#define	BMP280_COMMAND_PRESSURE4 0x5D    

#endif // BMP280_H
