/*  --------------------------------------------------------------------
    MPU9250.c
    
    MPU-9250 library
    This library uses I2C or SPI connection.

    --------------------------------------------------------------------
    CHANGELOG:
    * Copyright (C) 2015 Brian Chen - Open source under the MIT License.
    * 2018-01-17 - Régis Blanchot - Adapted to Pinguino
    * 2018-01-17 - Régis Blanchot - Added I2C communication
    --------------------------------------------------------------------
    TODO:
    * accuracy improvment
    --------------------------------------------------------------------
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
    ------------------------------------------------------------------*/

#ifndef __PIC32MX__
#include <compiler.h>
#endif

#include <typedef.h>
#include <macro.h>
#include <const.h>
#include <stdarg.h>
#include "MPU9250.h"

#include <quaternions.c>
#include <fastmath.c>

#if defined(__PIC32MX__)
#include <digitalw.c>
#include <delay.c>
#else
#include <digitalw.c>
#include <digitalp.c>
#include <delayms.c>
#include <delayus.c>
#endif

#if defined(MPU9250I2C1ENABLE) || defined(MPU9250I2C2ENABLE)
#include <i2c.h>
#include <i2c.c>
#endif

#if defined(MPU9250SPISWENABLE) || \
    defined(MPU9250SPI1ENABLE)  || defined(MPU9250SPI2ENABLE)
#include <spi.h>
#include <spi.c>
#endif

#if defined(MPU9250DEBUGSERIAL)
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

#if defined(MPU9250I2C1ENABLE) || defined(MPU9250I2C2ENABLE)
int gMPU9250I2CADDR;
#endif

// Default low pass filter of 188Hz
float gAccDivider;
float gGyroDivider;

int gCalibData[3];

float gMagDataASA[3];

float gAccData[3];
float gGyroData[3];
float gMagData[3];
float gTempData;

// Bias corrections for gyro and accelerometer
float gGyroBias[3];
float gAccBias[3];

s16 gMagDataRaw[3];    

// vector to hold quaternion
float q[4] = {1.0f, 0.0f, 0.0f, 0.0f};
// integration interval for both filter schemes
float deltat = 0.01f; // 10ms
/*
float sum = 0.0f;
u32 Now, lastUpdate;
Now = micros();
deltat = ((Now - lastUpdate)/1000000.0f); // set integration time by time elapsed since last filter update
lastUpdate = Now;
sum += deltat; // sum for averaging filter update rate
*/

/*  --------------------------------------------------------------------
    CORE FUNCTIONS
    ------------------------------------------------------------------*/

#if defined(MPU9250SPISWENABLE) || \
    defined(MPU9250SPI1ENABLE)  || defined(MPU9250SPI2ENABLE)

#define MPU9250_writeChar(module, reg, val)             SPI_writeChar(module, reg, val)
#define MPU9250_writeBytes(module, reg, buffer, length) SPI_writeChar(module, reg, buffer, length)
#define MPU9250_readChar(module, reg)                   SPI_readChar(module, reg | READ_FLAG)
#define MPU9250_readBytes(module, reg, buffer, length)  SPI_readBytes(module, reg | READ_FLAG, buffer, length)

#elif defined(MPU9250I2C1ENABLE) || defined(MPU9250I2C2ENABLE)

#define MPU9250_writeChar(module, reg, val)             I2C_writeChar(module, reg, val)
#define MPU9250_writeBytes(module, reg, buffer, length) I2C_writeChar(module, reg, buffer, length)
#define MPU9250_readChar(module, reg)                   I2C_readChar(module, reg)
#define MPU9250_readBytes(module, reg, buffer, length)  I2C_readBytes(module, reg, buffer, length)

#endif

u8 AK8963_writeChar(int module, u8 reg, u8 val)
{
    // Set the I2C slave addres of AK8963 and set for write.
    MPU9250_writeChar(module, MPU9250_I2C_SLV0_ADDR, AK8963_I2C_ADDR);
    // I2C slave 0 register address from where to begin data transfer
    MPU9250_writeChar(module, MPU9250_I2C_SLV0_REG, reg);
    // Enable I2C and request the bytes from the magnetometer
    MPU9250_writeChar(module, MPU9250_I2C_SLV0_DO, val);
    // Enable I2C and send 1 byte
    MPU9250_writeChar(module, MPU9250_I2C_SLV0_CTRL, MPU9250_I2C_SLV0_EN | 1);
    // Gives some time
    Delayus(1000);
    // Read the bytes off the MPU9250 EXT_SENS_DATA registers
    return AK8963_readChar(module, reg);
}

u8 AK8963_readChar(int module, u8 reg)
{
    // Set the I2C slave addres of AK8963 and set for read.
    MPU9250_writeChar(module, MPU9250_I2C_SLV0_ADDR, AK8963_I2C_ADDR | READ_FLAG);
    // I2C slave 0 register address from where to begin data transfer
    MPU9250_writeChar(module, MPU9250_I2C_SLV0_REG, reg);
    // Enable I2C and request the bytes from the magnetometer
    MPU9250_writeChar(module, MPU9250_I2C_SLV0_CTRL, MPU9250_I2C_SLV0_EN | 1);
    // Gives some time
    Delayus(1000);
    // Read the bytes off the MPU9250 EXT_SENS_DATA registers
    return MPU9250_readChar(module, MPU9250_EXT_SENS_DATA_00);
}

void AK8963_readBytes(int module, u8 reg, u8 *buffer, u8 length)
{
    // Set the I2C slave addres of AK8963 and set for read.
    MPU9250_writeChar(module, MPU9250_I2C_SLV0_ADDR, AK8963_I2C_ADDR | READ_FLAG);
    // I2C slave 0 register address from where to begin data transfer
    MPU9250_writeChar(module, MPU9250_I2C_SLV0_REG, reg);
    // Enable I2C and request the bytes from the magnetometer
    MPU9250_writeChar(module, MPU9250_I2C_SLV0_CTRL, MPU9250_I2C_SLV0_EN | length);
    // Gives some time
    Delayus(1000);
    // Read the bytes off the MPU9250 EXT_SENS_DATA registers
    MPU9250_readBytes(module, MPU9250_EXT_SENS_DATA_00, buffer, length);
}

/*  --------------------------------------------------------------------
    INITIALIZATION
    * usage: call this function at startup,
    * giving the sample rate divider (raging from 0 to 255)
    * and low pass filter value; suitable values are:
    * BITS_DLPF_CFG_256HZ_NOLPF2
    * BITS_DLPF_CFG_188HZ
    * BITS_DLPF_CFG_98HZ
    * BITS_DLPF_CFG_42HZ
    * BITS_DLPF_CFG_20HZ
    * BITS_DLPF_CFG_10HZ 
    * BITS_DLPF_CFG_5HZ 
    * BITS_DLPF_CFG_2100HZ_NOLPF
    * returns 1 if an error occurred
    ------------------------------------------------------------------*/

#define MPU_InitRegNum 17

u8 MPU9250_init(int module, ...)
{
    #if defined(MPU9250SPISWENABLE)
    int sdo, sdi, sck, cs;
    #endif
    u8 line, r;
    u8 MPU_Init_Data[MPU_InitRegNum][2] = {
        {MPU9250_PWR_MGMT_1,     BIT_H_RESET},           // Reset Device
        {MPU9250_PWR_MGMT_1,     0x01},                  // Clock Source
        {MPU9250_PWR_MGMT_2,     0x00},                  // Enable Acc & Gyro
        {MPU9250_CONFIG,         BITS_DLPF_CFG_188HZ},   // Use DLPF set Gyroscope bandwidth 184Hz, Temperature bandwidth 188Hz
        {MPU9250_GYRO_CONFIG,    BITS_FS_250DPS},        // +-250dps
        {MPU9250_ACCEL_CONFIG,   BITS_FS_2G},            // +-2G
        {MPU9250_ACCEL_CONFIG_2, BITS_DLPF_CFG_188HZ},   // Set Acc Data Rates, Enable Acc LPF , Bandwidth 184Hz
        {MPU9250_INT_PIN_CFG,    0x12},                  // Any read will clear the interrupt 
        //{MPU9250_INT_PIN_CFG,    0x22},                // Latch the interrupt

        {MPU9250_USER_CTRL,      0x30},                  // I2C Master mode and set I2C_IF_DIS to disable slave mode I2C bus
        {MPU9250_I2C_MST_CTRL,   0x0D},                  // I2C configuration multi-master  IIC 400KHz
        
        {MPU9250_I2C_SLV0_ADDR,  AK8963_I2C_ADDR},       // Set the I2C slave addres of AK8963 and set for write.
        //{MPU9250_I2C_SLV4_CTRL,0x09},
        //{MPU9250_I2C_MST_DELAY_CTRL,0x81},             // Enable I2C Delayms

        {MPU9250_I2C_SLV0_REG,   AK8963_CNTL2},          // I2C slave 0 register address from where to begin data transfer
        {MPU9250_I2C_SLV0_DO,    0x01},                  // Reset AK8963
        {MPU9250_I2C_SLV0_CTRL,  0x81},                  // Enable I2C and set 1 byte

        {MPU9250_I2C_SLV0_REG,   AK8963_CNTL1},          // I2C slave 0 register address from where to begin data transfer
        #ifdef AK8963FASTMODE
        {MPU9250_I2C_SLV0_DO,    0x16},                  // Register value to 100Hz continuous measurement in 16bit
        #else
        {MPU9250_I2C_SLV0_DO,    0x12},                  // Register value to 8Hz continuous measurement in 16bit
        #endif
        {MPU9250_I2C_SLV0_CTRL,  0x81}                   // Enable I2C and set 1 byte
    };

    va_list args;
    
    // Debug
    
    #if defined(MPU9250DEBUGSERIAL)
    #if defined(__PIC32MX__)
    SerialConfigure(SERIALPORT, UART_ENABLE, UART_RX_TX_ENABLED, 9600);
    #else
    Serial_begin(SERIALPORT, 9600, NULL);
    #endif
    #endif

    va_start(args, module);             // args points on the argument after module

    #if defined(MPU9250SPISWENABLE)

    sdo = va_arg(args, int);            // get the next arg
    sdi = va_arg(args, int);            // get the next arg
    sck = va_arg(args, int);            // get the next arg
    cs  = va_arg(args, int);            // get the last arg

    SPI_setDataMode(module, SPI_MODE1);
    SPI_setBitOrder(module, SPI_MSBFIRST);
    SPI_begin(module, sdo, sdi, sck, cs);

    #elif defined(MPU9250SPI1ENABLE) || defined(MPU9250SPI2ENABLE)

    SPI_setMode(module, SPI_MASTER);
    // Clock idle state low (CKP = 0)
    // Data transmitted on falling edge (CKE = 1)
    // => (0, 1) = SPI_MODE1
    SPI_setDataMode(module, SPI_MODE1);
    // The internal registers and memory of the MPU-9250
    // can be accessed using SPI at max. 1MHz
    #if defined(__PIC32MX__)
    //SPI_setClockDivider(module, SPI_PBCLOCK_DIV64);
    SPI_setClock(module, 1000000);
    #else
    SPI_setClockDivider(module, SPI_CLOCK_DIV64);
    #endif
    SPI_begin(module, NULL);

    #elif defined(MPU9250I2C1ENABLE) || defined(MPU9250I2C2ENABLE)
    
    gMPU9250I2CADDR = va_arg(args, int);// get the next arg
    //if (gMPU9250I2CADDR == 0)
    //    gMPU9250I2CADDR = 0x71;
     
    // The internal registers and memory of the MPU-9250
    // can be accessed using I2C at 400 kHz
    I2C_master(module, I2C_400KHZ);

    #endif

    va_end(args);                       // cleans up the list

    // Init. sequence
    for (line = 0; line < MPU_InitRegNum; line++)
    {
        MPU9250_writeChar(module, MPU_Init_Data[line][0], MPU_Init_Data[line][1]);
        // Slow down the write speed, otherwise it won't work
        Delayus(1000);
    }

    // Check if MPU9250/MPU9255 is responding
    r = MPU9250_whoami(module);
    if (r != MPU9250_DEVID && r != MPU9255_DEVID)
        return 0;

    // Check if AK8963 is responding
    r = AK8963_whoami(module);
    if (r != AK8963_DEVID)
        return 0;

    MPU9250_getBiases(module, gGyroBias, gAccBias);
    
    // Suitable ranges are: BITS_FS_2G, BITS_FS_4G, BITS_FS_8G or BITS_FS_16G
    MPU9250_setAccelerometerScale(module, BITS_FS_2G);
    // Suitable ranges are: BITS_FS_250DPS, BITS_FS_500DPS, BITS_FS_1000DPS or BITS_FS_2000DPS
    MPU9250_setGyroscopeScale(module, BITS_FS_250DPS);
    
    // If experiencing problems here, just comment it out.
    // Should still be somewhat functional.
    MPU9250_getAccelerometerCalibration(module);
    MPU9250_getMagnetometerCalibration(module);
    
    return 1;
}

/* ACCELEROMETER SCALE
 * Called from initialization sequence to set the right range for the
 * accelerometers. Suitable ranges are:
 * BITS_FS_2G
 * BITS_FS_4G
 * BITS_FS_8G
 * BITS_FS_16G
 * returns the range set (2,4,8 or 16)
 */

u8 MPU9250_setAccelerometerScale(int module, u8 scale)
{
    u8 temp_scale;
    
    MPU9250_writeChar(module, MPU9250_ACCEL_CONFIG, scale);
    
    switch (scale)
    {
        case BITS_FS_2G:
            gAccDivider = 16384;
            break;
        case BITS_FS_4G:
            gAccDivider = 8192;
            break;
        case BITS_FS_8G:
            gAccDivider = 4096;
            break;
        case BITS_FS_16G:
            gAccDivider = 2048;
            break;   
    }
    
    temp_scale = MPU9250_readChar(module, MPU9250_ACCEL_CONFIG);
    
    switch (temp_scale)
    {
        case BITS_FS_2G:
            temp_scale = 2;
            break;
        case BITS_FS_4G:
            temp_scale = 4;
            break;
        case BITS_FS_8G:
            temp_scale = 8;
            break;
        case BITS_FS_16G:
            temp_scale = 16;
            break;   
    }
    
    return temp_scale;
}

/* GYROSCOPE SCALE
 * Called from the initialization sequence to set the right range for the
 * gyroscopes. Suitable ranges are:
 * BITS_FS_250DPS
 * BITS_FS_500DPS
 * BITS_FS_1000DPS
 * BITS_FS_2000DPS
 * returns the range set (250,500,1000 or 2000)
 */

u16 MPU9250_setGyroscopeScale(int module, u8 scale)
{
    u16 temp_scale;
    
    MPU9250_writeChar(module, MPU9250_GYRO_CONFIG, scale);

    switch (scale)
    {
        case BITS_FS_250DPS:   gGyroDivider = 131;  break;
        case BITS_FS_500DPS:   gGyroDivider = 65.5; break;
        case BITS_FS_1000DPS:  gGyroDivider = 32.8; break;
        case BITS_FS_2000DPS:  gGyroDivider = 16.4; break;   
    }

    temp_scale = MPU9250_readChar(module, MPU9250_GYRO_CONFIG);

    switch (temp_scale)
    {
        case BITS_FS_250DPS:   temp_scale = 250;    break;
        case BITS_FS_500DPS:   temp_scale = 500;    break;
        case BITS_FS_1000DPS:  temp_scale = 1000;   break;
        case BITS_FS_2000DPS:  temp_scale = 2000;   break;   
    }
    
    return temp_scale;
}

/* READ ACCELEROMETER
 * usage: call this function to read accelerometer data. Axis represents selected axis:
 * 0 -> X axis
 * 1 -> Y axis
 * 2 -> Z axis
 */

float * MPU9250_getAcceleration(int module)
{
    u8 response[6];
    s16 bit_data;
    float data;
    u8 i;
    
    MPU9250_readBytes(module, MPU9250_ACCEL_XOUT_H, response, 6);
    for(i = 0; i < 3; i++)
    {
        bit_data = ((s16)response[i*2]<<8) | response[i*2+1];
        data = (float)bit_data;
        gAccData[i] = data / gAccDivider - gAccBias[i];
    }
    return gAccData;
}

/* READ GYROSCOPE
 * usage: call this function to read gyroscope data. Axis represents selected axis:
 * 0 -> X axis
 * 1 -> Y axis
 * 2 -> Z axis
 */

float * MPU9250_getGyration(int module)
{
    u8 response[6];
    s16 bit_data;
    float data;
    u8 i;
    
    MPU9250_readBytes(module, MPU9250_GYRO_XOUT_H, response, 6);
    for(i = 0; i < 3; i++)
    {
        bit_data = ((s16)response[i*2]<<8) | response[i*2+1];
        data = (float)bit_data;
        gGyroData[i] = data / gGyroDivider - gGyroBias[i];
    }
    return gGyroData;
}

/* READ gTempData
 * usage: call this function to read gTempData data. 
 * returns the value in °C
 */

float MPU9250_getTemperatureCelsius(int module)
{
    u8 response[2];
    s16 bit_data;
    float data;

    MPU9250_readBytes(module, MPU9250_TEMP_OUT_H, response, 2);

    bit_data = ((s16)response[0]<<8)|response[1];
    data = (float)bit_data;
    gTempData = (data / 340) + 36.53;
    return gTempData;
}

float MPU9250_getTemperatureFahrenheit(int module)
{
    return MPU9250_getTemperatureCelsius(module) * 9.0 / 5.0 + 32.0;
}

float * MPU9250_getMagnetometer(int module)
{
    u8 response[7];
    float data;
    int i;

    AK8963_readBytes(module, AK8963_HXL, response, 7);
    for(i = 0; i < 3; i++)
    {
        gMagDataRaw[i] = ((s16)response[i*2+1]<<8)|response[i*2];
        data = (float)gMagDataRaw[i];
        gMagData[i] = data*gMagDataASA[i];
    }
    return gMagData;
}

float MPU9250_getYaw(int module, float declination)
{
    float yaw;
    float x = 2.0f * (q[1] * q[2] + q[0] * q[3]);
    float y = q[0] * q[0] + q[1] * q[1] - q[2] * q[2] - q[3] * q[3];
    yaw = fastatan2(x, y);
    yaw *= 180.0f / PI;
    yaw += declination; // according IRGF12 model
    return yaw;
}

float MPU9250_getPitch(int module)
{
    float pitch;
    float x = 2.0f * (q[1] * q[3] - q[0] * q[2]);
    pitch = -fastasin(x);
    pitch *= 180.0f / PI;
    return pitch;
}

float MPU9250_getRoll(int module)
{
    float roll;
    float x = 2.0f * (q[0] * q[1] + q[2] * q[3]);
    float y = q[0] * q[0] - q[1] * q[1] - q[2] * q[2] + q[3] * q[3];
    roll = fastatan2(x, y);
    roll *= 180.0f / PI;
    return roll;
}

/* READ ACCELEROMETER CALIBRATION
 * usage: call this function to read accelerometer data.
 * Axis represents selected axis:
 * 0 -> X axis
 * 1 -> Y axis
 * 2 -> Z axis
 * returns Factory Trim value
 */

void MPU9250_getAccelerometerCalibration(int module)
{
    u8 response[4];
    u8 temp_scale;
    //READ CURRENT ACC SCALE
    temp_scale = MPU9250_readChar(module, MPU9250_ACCEL_CONFIG);
    MPU9250_setAccelerometerScale(module, BITS_FS_8G);
    //ENABLE SELF TEST need modify
    //temp_scale=MPU9250_writeChar(module, MPU9250_ACCEL_CONFIG, 0x80>>axis);

    MPU9250_readBytes(module, MPU9250_SELF_TEST_X, response, 4);
    gCalibData[0] = ((response[0]&11100000)>>3) | ((response[3]&00110000)>>4);
    gCalibData[1] = ((response[1]&11100000)>>3) | ((response[3]&00001100)>>2);
    gCalibData[2] = ((response[2]&11100000)>>3) | ((response[3]&00000011));

    MPU9250_setAccelerometerScale(module, temp_scale);
}

void MPU9250_getMagnetometerCalibration(int module)
{
    u8 response[3];
    float data;
    int i;
    // Choose either 14-bit or 16-bit magnetometer resolution
    //u8 MFS_14BITS = 0; // 0.6 mG per LSB
    u8 MFS_16BITS =1; // 0.15 mG per LSB
    // 2 for 8 Hz, 6 for 100 Hz continuous magnetometer data read
    u8 M_8HZ = 0x02; // 8 Hz update
    //u8 M_100HZ = 0x06; // 100 Hz continuous magnetometer

    /* get the magnetometer calibration */

    MPU9250_writeChar(module, MPU9250_I2C_SLV0_ADDR,AK8963_I2C_ADDR|READ_FLAG);   // Set the I2C slave    addres of AK8963 and set for read.
    MPU9250_writeChar(module, MPU9250_I2C_SLV0_REG, AK8963_ASAX);                 // I2C slave 0 register address from where to begin data transfer
    MPU9250_writeChar(module, MPU9250_I2C_SLV0_CTRL, 0x83);                       // Read 3 bytes from the magnetometer

    //MPU9250_writeChar(module, MPU9250_I2C_SLV0_CTRL, 0x81);                     // Enable I2C and set bytes
    Delayms(100);  
    //response[0]=MPU9250_writeChar(module, MPU9250_EXT_SENS_DATA_01|READ_FLAG, 0x00); //Read I2C 

    MPU9250_writeChar(module, AK8963_CNTL1, 0x00);                               // set AK8963 to Power Down
    Delayms(50);                                                  // long wait between AK8963 mode changes
    MPU9250_writeChar(module, AK8963_CNTL1, 0x0F);                               // set AK8963 to FUSE ROM access
    Delayms(50);                                                  // long wait between AK8963 mode changes

    MPU9250_readBytes(module, MPU9250_EXT_SENS_DATA_00,response,3);
    //response=MPU9250_writeChar(module, MPU9250_I2C_SLV0_DO, 0x00);              // Read I2C 
    for(i = 0; i < 3; i++)
    {
        data=response[i];
        gMagDataASA[i] = ((data-128)/256+1)*Magnetometer_Sensitivity_Scale_Factor;
    }
    MPU9250_writeChar(module, AK8963_CNTL1, 0x00); // set AK8963 to Power Down
    Delayms(50);
    // Configure the magnetometer for continuous read and highest resolution.
    // Set bit 4 to 1 (0) to enable 16 (14) bit resolution in CNTL
    // register, and enable continuous mode data acquisition (bits [3:0]),
    // 0010 for 8 Hz and 0110 for 100 Hz sample rates.   
    MPU9250_writeChar(module, AK8963_CNTL1, MFS_16BITS << 4 | M_8HZ);            // Set magnetometer data resolution and sample ODR
    Delayms(50);
}

void MPU9250_read_all(int module)
{
    u8 response[21];
    s16 bit_data;
    float data;
    u8 i;

    // Send I2C command at first
    // Set the I2C slave addres of AK8963 and set for read.
    MPU9250_writeChar(module, MPU9250_I2C_SLV0_ADDR, AK8963_I2C_ADDR|READ_FLAG);
    // I2C slave 0 register address from where to begin data transfer
    MPU9250_writeChar(module, MPU9250_I2C_SLV0_REG, AK8963_HXL);
    // Read 7 bytes from the magnetometer
    MPU9250_writeChar(module, MPU9250_I2C_SLV0_CTRL, 0x87);
    // Must start your read from AK8963A register 0x03
    // and read seven bytes so that upon read of ST2 register 0x09
    // the AK8963A will unlatch the data registers for the next measurement.

    MPU9250_readBytes(module, MPU9250_ACCEL_XOUT_H, response, 21);

    // Get accelerometer value
    for (i = 0; i < 3; i++)
    {
        bit_data = ((s16)response[i*2]<<8) | response[i*2+1];
        data = (float)bit_data;
        gAccData[i] = data / gAccDivider - gAccBias[i];
    }

    // Get temperature value
    bit_data = ((s16)response[i*2]<<8) | response[i*2+1];
    data = (float)bit_data;
    gTempData = ((data-21)/333.87)+21;

    // Get gyroscope value
    for (i=4; i < 7; i++)
    {
        bit_data = ((s16)response[i*2]<<8) | response[i*2+1];
        data = (float)bit_data;
        gGyroData[i-4] = data/gGyroDivider - gGyroBias[i-4];
    }

    // Get Magnetometer value
    for (i=7; i < 10; i++)
    {
        gMagDataRaw[i-7] = ((s16)response[i*2+1]<<8) | response[i*2];
        data = (float)gMagDataRaw[i-7];
        gMagData[i-7] = data * gMagDataASA[i-7];
    }
}

void MPU9250_getBiases(int module, float *dest1, float *dest2)
{  
    u8 data[12];                       // hold accelerometer and gyro x, y, z, data
    s16 ii, packet_count, fifo_count;
    u8 mask_bit[3]        = {0, 0, 0}; // hold mask bit for each accelerometer bias axis
    s32 gyro_bias[3]      = {0, 0, 0};
    s32 accel_bias[3]     = {0, 0, 0};
    s32 accel_bias_reg[3] = {0, 0, 0}; // hold the factory accelerometer trim biases
    s16 accel_temp[3]     = {0, 0, 0};
    s16 gyro_temp[3]      = {0, 0, 0};
    s16 gyrosensitivity   = 131;       // = 131 LSB/degrees/sec
    s16 accelsensitivity  = 16384;     // = 16384 LSB/g
    s32 mask = 1uL;                    // Define mask for temperature compensation bit 0 of lower byte of accelerometer bias registers
    
    // reset device
    MPU9250_writeChar(module, MPU9250_PWR_MGMT_1,   0x80); // Write a one to bit 7 reset bit; toggle reset device
    Delayms(100);
   
    // get stable time source; Auto select clock source to be PLL gyroscope reference if ready 
    // else use the internal oscillator, bits 2:0 = 001
    MPU9250_writeChar(module, MPU9250_PWR_MGMT_1,   0x01);  
    MPU9250_writeChar(module, MPU9250_PWR_MGMT_2,   0x00);
    Delayms(200);                                    

    // Configure device for bias calculation
    MPU9250_writeChar(module, MPU9250_INT_ENABLE,   0x00); // Disable all interrupts
    MPU9250_writeChar(module, MPU9250_FIFO_EN,      0x00); // Disable FIFO
    MPU9250_writeChar(module, MPU9250_PWR_MGMT_1,   0x00); // Turn on internal clock source
    MPU9250_writeChar(module, MPU9250_I2C_MST_CTRL, 0x00); // Disable I2C master
    MPU9250_writeChar(module, MPU9250_USER_CTRL,    0x00); // Disable FIFO and I2C master modes
    MPU9250_writeChar(module, MPU9250_USER_CTRL,    0x0C); // Reset FIFO and DMP
    Delayms(15);
  
    // Configure MPU6050 gyro and accelerometer for bias calculation
    MPU9250_writeChar(module, MPU9250_CONFIG,       0x01); // Set low-pass filter to 188 Hz
    MPU9250_writeChar(module, MPU9250_SMPLRT_DIV,   0x00); // Set sample rate to 1 kHz
    MPU9250_writeChar(module, MPU9250_GYRO_CONFIG,  0x00); // Set gyro full-scale to 250 degrees per second, maximum sensitivity
    MPU9250_writeChar(module, MPU9250_ACCEL_CONFIG, 0x00); // Set accelerometer full-scale to 2 g, maximum sensitivity
    
    // Configure FIFO to capture accelerometer and gyro data for bias calculation
    MPU9250_writeChar(module, MPU9250_USER_CTRL,    0x40); // Enable FIFO  
    MPU9250_writeChar(module, MPU9250_FIFO_EN,      0x78); // Enable gyro and accelerometer sensors for FIFO  (max size 512 bytes in MPU-9150)
    Delayms(40); // accumulate 40 samples in 40 milliseconds = 480 bytes

    // At end of sample accumulation, turn off FIFO sensor read
    MPU9250_writeChar(module, MPU9250_FIFO_EN,      0x00); // Disable gyro and accelerometer sensors for FIFO
    MPU9250_readBytes(module, MPU9250_FIFO_COUNTH, data, 2); // read FIFO sample count
    fifo_count = ((s16)data[0] << 8) | data[1];
    packet_count = fifo_count/12;// How many sets of full gyro and accelerometer data for averaging
    
    for (ii = 0; ii < packet_count; ii++)
    {
        //accel_temp[3] = {0, 0, 0};
        //gyro_temp[3] = {0, 0, 0};

        MPU9250_readBytes(module, MPU9250_FIFO_R_W, data, 12); // read data for averaging

        accel_temp[0] = (s16) (((s16)data[0] << 8)  | data[1]  ); // Form signed 16-bit integer for each sample in FIFO
        accel_temp[1] = (s16) (((s16)data[2] << 8)  | data[3]  );
        accel_temp[2] = (s16) (((s16)data[4] << 8)  | data[5]  );    
        gyro_temp[0]  = (s16) (((s16)data[6] << 8)  | data[7]  );
        gyro_temp[1]  = (s16) (((s16)data[8] << 8)  | data[9]  );
        gyro_temp[2]  = (s16) (((s16)data[10] << 8) | data[11] );
        
        accel_bias[0] += (s32)accel_temp[0]; // Sum individual signed 16-bit biases to get accumulated signed 32-bit biases
        accel_bias[1] += (s32)accel_temp[1];
        accel_bias[2] += (s32)accel_temp[2];
        gyro_bias[0]  += (s32)gyro_temp[0];
        gyro_bias[1]  += (s32)gyro_temp[1];
        gyro_bias[2]  += (s32)gyro_temp[2];
    }

    accel_bias[0] /= (s32)packet_count; // Normalize sums to get average count biases
    accel_bias[1] /= (s32)packet_count;
    accel_bias[2] /= (s32)packet_count;
    gyro_bias[0]  /= (s32)packet_count;
    gyro_bias[1]  /= (s32)packet_count;
    gyro_bias[2]  /= (s32)packet_count;
    
    // Remove gravity from the z-axis accelerometer bias calculation
    if(accel_bias[2] > 0L)
        accel_bias[2] -= (s32)accelsensitivity;
    else
        accel_bias[2] += (s32)accelsensitivity;
   
    // Construct the gyro biases for push to the hardware gyro bias registers,
    // which are reset to zero upon device startup
    data[0] = (-gyro_bias[0]/4  >> 8) & 0xFF; // Divide by 4 to get 32.9 LSB per deg/s to conform to expected bias input format
    data[1] = (-gyro_bias[0]/4)       & 0xFF; // Biases are additive, so change sign on calculated average gyro biases
    data[2] = (-gyro_bias[1]/4  >> 8) & 0xFF;
    data[3] = (-gyro_bias[1]/4)       & 0xFF;
    data[4] = (-gyro_bias[2]/4  >> 8) & 0xFF;
    data[5] = (-gyro_bias[2]/4)       & 0xFF;
  
    // Push gyro biases to hardware registers
    MPU9250_writeChar(module, MPU9250_XG_OFFS_USRH, data[0]);
    MPU9250_writeChar(module, MPU9250_XG_OFFS_USRL, data[1]);
    MPU9250_writeChar(module, MPU9250_YG_OFFS_USRH, data[2]);
    MPU9250_writeChar(module, MPU9250_YG_OFFS_USRL, data[3]);
    MPU9250_writeChar(module, MPU9250_ZG_OFFS_USRH, data[4]);
    MPU9250_writeChar(module, MPU9250_ZG_OFFS_USRL, data[5]);
  
    // Output scaled gyro biases for display in the main program
    dest1[0] = (float)gyro_bias[0] / (float)gyrosensitivity;  
    dest1[1] = (float)gyro_bias[1] / (float)gyrosensitivity;
    dest1[2] = (float)gyro_bias[2] / (float)gyrosensitivity;

    // Construct the accelerometer biases for push to the hardware accelerometer bias registers. These registers contain
    // factory trim values which must be added to the calculated accelerometer biases; on boot up these registers will hold
    // non-zero values. In addition, bit 0 of the lower byte must be preserved since it is used for gTempData
    // compensation calculations. Accelerometer bias registers expect bias input as 2048 LSB per g, so that
    // the accelerometer biases calculated above must be divided by 8.

    MPU9250_readBytes(module, MPU9250_XA_OFFSET_H, data, 2); // Read factory accelerometer trim values
    accel_bias_reg[0] = (s32)(((s16)data[0] << 8) | data[1]);
    MPU9250_readBytes(module, MPU9250_YA_OFFSET_H, data, 2);
    accel_bias_reg[1] = (s32)(((s16)data[0] << 8) | data[1]);
    MPU9250_readBytes(module, MPU9250_ZA_OFFSET_H, data, 2);
    accel_bias_reg[2] = (s32)(((s16)data[0] << 8) | data[1]);
    
    for (ii = 0; ii < 3; ii++)
        if ((accel_bias_reg[ii] & mask))
            mask_bit[ii] = 0x01; // If gTempData compensation bit is set, record that fact in mask_bit
    
    // Construct total accelerometer bias, including calculated average accelerometer bias from above
    accel_bias_reg[0] -= (accel_bias[0]/8); // Subtract calculated averaged accelerometer bias scaled to 2048 LSB/g (16 g full scale)
    accel_bias_reg[1] -= (accel_bias[1]/8);
    accel_bias_reg[2] -= (accel_bias[2]/8);
  
    data[0] = (accel_bias_reg[0] >> 8) & 0xFF;
    data[1] = (accel_bias_reg[0])      & 0xFF;
    data[1] = data[1] | mask_bit[0]; // preserve gTempData compensation bit when writing back to accelerometer bias registers
    data[2] = (accel_bias_reg[1] >> 8) & 0xFF;
    data[3] = (accel_bias_reg[1])      & 0xFF;
    data[3] = data[3] | mask_bit[1]; // preserve gTempData compensation bit when writing back to accelerometer bias registers
    data[4] = (accel_bias_reg[2] >> 8) & 0xFF;
    data[5] = (accel_bias_reg[2])      & 0xFF;
    data[5] = data[5] | mask_bit[2]; // preserve gTempData compensation bit when writing back to accelerometer bias registers
 
    // Apparently this is not working for the acceleration biases in the MPU-9250
    // Are we handling the gTempData correction bit properly?
    // Push accelerometer biases to hardware registers
    MPU9250_writeChar(module, MPU9250_XA_OFFSET_H, data[0]);
    MPU9250_writeChar(module, MPU9250_XA_OFFSET_L, data[1]);
    MPU9250_writeChar(module, MPU9250_YA_OFFSET_H, data[2]);
    MPU9250_writeChar(module, MPU9250_YA_OFFSET_L, data[3]);
    MPU9250_writeChar(module, MPU9250_ZA_OFFSET_H, data[4]);
    MPU9250_writeChar(module, MPU9250_ZA_OFFSET_L, data[5]);

    // Output scaled accelerometer biases for display in the main program
    dest2[0] = (float)accel_bias[0] / (float)accelsensitivity; 
    dest2[1] = (float)accel_bias[1] / (float)accelsensitivity;
    dest2[2] = (float)accel_bias[2] / (float)accelsensitivity;
}
