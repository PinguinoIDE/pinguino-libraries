/**
 * Invensense MPU-9250 library using the SPI interface
 *
 * Copyright (C) 2015 Brian Chen
 * 
 * Open source under the MIT License. See LICENSE.txt.
 */

#ifndef MPU9250_H
#define MPU9250_H

#include <typedef.h>

// #define AK8963FASTMODE

// mpu9250 registers
#define MPU9250_XG_OFFS_TC          0x00
#define MPU9250_YG_OFFS_TC          0x01
#define MPU9250_ZG_OFFS_TC          0x02
#define MPU9250_X_FINE_GAIN         0x03
#define MPU9250_Y_FINE_GAIN         0x04
#define MPU9250_Z_FINE_GAIN         0x05
#define MPU9250_XA_OFFS_H           0x06
#define MPU9250_XA_OFFS_L           0x07
#define MPU9250_YA_OFFS_H           0x08
#define MPU9250_YA_OFFS_L           0x09
#define MPU9250_ZA_OFFS_H           0x0A
#define MPU9250_ZA_OFFS_L           0x0B
#define MPU9250_PRODUCT_ID          0x0C
#define MPU9250_SELF_TEST_X         0x0D
#define MPU9250_SELF_TEST_Y         0x0E
#define MPU9250_SELF_TEST_Z         0x0F
#define MPU9250_SELF_TEST_A         0x10
#define MPU9250_XG_OFFS_USRH        0x13
#define MPU9250_XG_OFFS_USRL        0x14
#define MPU9250_YG_OFFS_USRH        0x15
#define MPU9250_YG_OFFS_USRL        0x16
#define MPU9250_ZG_OFFS_USRH        0x17
#define MPU9250_ZG_OFFS_USRL        0x18
#define MPU9250_SMPLRT_DIV          0x19
#define MPU9250_CONFIG              0x1A
#define MPU9250_GYRO_CONFIG         0x1B
#define MPU9250_ACCEL_CONFIG        0x1C
#define MPU9250_ACCEL_CONFIG_2      0x1D
#define MPU9250_LP_ACCEL_ODR        0x1E
#define MPU9250_MOT_THR             0x1F
#define MPU9250_FIFO_EN             0x23
#define MPU9250_I2C_MST_CTRL        0x24
#define MPU9250_I2C_SLV0_ADDR       0x25
#define MPU9250_I2C_SLV0_REG        0x26
#define MPU9250_I2C_SLV0_CTRL       0x27
#define MPU9250_I2C_SLV1_ADDR       0x28
#define MPU9250_I2C_SLV1_REG        0x29
#define MPU9250_I2C_SLV1_CTRL       0x2A
#define MPU9250_I2C_SLV2_ADDR       0x2B
#define MPU9250_I2C_SLV2_REG        0x2C
#define MPU9250_I2C_SLV2_CTRL       0x2D
#define MPU9250_I2C_SLV3_ADDR       0x2E
#define MPU9250_I2C_SLV3_REG        0x2F
#define MPU9250_I2C_SLV3_CTRL       0x30
#define MPU9250_I2C_SLV4_ADDR       0x31
#define MPU9250_I2C_SLV4_REG        0x32
#define MPU9250_I2C_SLV4_DO         0x33
#define MPU9250_I2C_SLV4_CTRL       0x34
#define MPU9250_I2C_SLV4_DI         0x35
#define MPU9250_I2C_MST_STATUS      0x36
#define MPU9250_INT_PIN_CFG         0x37
#define MPU9250_INT_ENABLE          0x38
#define MPU9250_ACCEL_XOUT_H        0x3B
#define MPU9250_ACCEL_XOUT_L        0x3C
#define MPU9250_ACCEL_YOUT_H        0x3D
#define MPU9250_ACCEL_YOUT_L        0x3E
#define MPU9250_ACCEL_ZOUT_H        0x3F
#define MPU9250_ACCEL_ZOUT_L        0x40
#define MPU9250_TEMP_OUT_H          0x41
#define MPU9250_TEMP_OUT_L          0x42
#define MPU9250_GYRO_XOUT_H         0x43
#define MPU9250_GYRO_XOUT_L         0x44
#define MPU9250_GYRO_YOUT_H         0x45
#define MPU9250_GYRO_YOUT_L         0x46
#define MPU9250_GYRO_ZOUT_H         0x47
#define MPU9250_GYRO_ZOUT_L         0x48
#define MPU9250_EXT_SENS_DATA_00    0x49
#define MPU9250_EXT_SENS_DATA_01    0x4A
#define MPU9250_EXT_SENS_DATA_02    0x4B
#define MPU9250_EXT_SENS_DATA_03    0x4C
#define MPU9250_EXT_SENS_DATA_04    0x4D
#define MPU9250_EXT_SENS_DATA_05    0x4E
#define MPU9250_EXT_SENS_DATA_06    0x4F
#define MPU9250_EXT_SENS_DATA_07    0x50
#define MPU9250_EXT_SENS_DATA_08    0x51
#define MPU9250_EXT_SENS_DATA_09    0x52
#define MPU9250_EXT_SENS_DATA_10    0x53
#define MPU9250_EXT_SENS_DATA_11    0x54
#define MPU9250_EXT_SENS_DATA_12    0x55
#define MPU9250_EXT_SENS_DATA_13    0x56
#define MPU9250_EXT_SENS_DATA_14    0x57
#define MPU9250_EXT_SENS_DATA_15    0x58
#define MPU9250_EXT_SENS_DATA_16    0x59
#define MPU9250_EXT_SENS_DATA_17    0x5A
#define MPU9250_EXT_SENS_DATA_18    0x5B
#define MPU9250_EXT_SENS_DATA_19    0x5C
#define MPU9250_EXT_SENS_DATA_20    0x5D
#define MPU9250_EXT_SENS_DATA_21    0x5E
#define MPU9250_EXT_SENS_DATA_22    0x5F
#define MPU9250_EXT_SENS_DATA_23    0x60
#define MPU9250_I2C_SLV0_DO         0x63
#define MPU9250_I2C_SLV1_DO         0x64
#define MPU9250_I2C_SLV2_DO         0x65
#define MPU9250_I2C_SLV3_DO         0x66
#define MPU9250_I2C_MST_DELAY_CTRL  0x67
#define MPU9250_SIGNAL_PATH_RESET   0x68
#define MPU9250_MOT_DETECT_CTRL     0x69
#define MPU9250_USER_CTRL           0x6A
#define MPU9250_PWR_MGMT_1          0x6B
#define MPU9250_PWR_MGMT_2          0x6C
#define MPU9250_BANK_SEL            0x6D
#define MPU9250_MEM_START_ADDR      0x6E
#define MPU9250_MEM_R_W             0x6F
#define MPU9250_DMP_CFG_1           0x70
#define MPU9250_DMP_CFG_2           0x71
#define MPU9250_FIFO_COUNTH         0x72
#define MPU9250_FIFO_COUNTL         0x73
#define MPU9250_FIFO_R_W            0x74
#define MPU9250_WHOAMI              0x75
#define MPU9250_XA_OFFSET_H         0x77
#define MPU9250_XA_OFFSET_L         0x78
#define MPU9250_YA_OFFSET_H         0x7A
#define MPU9250_YA_OFFSET_L         0x7B
#define MPU9250_ZA_OFFSET_H         0x7D
#define MPU9250_ZA_OFFSET_L         0x7E
#define MPU9250_I2C_SLV0_EN         0x80

/* ---- AK8963 Reg In MPU9250 ----------------------------------------------- */
 
#define AK8963_I2C_ADDR            0x0C //0x18
#define AK8963_DEVID               0x48
#define MPU9250_DEVID              0x71
#define MPU9255_DEVID              0x73
 
// Read-only Reg
#define AK8963_WIA                 0x00
#define AK8963_INFO                0x01
#define AK8963_ST1                 0x02
#define AK8963_HXL                 0x03
#define AK8963_HXH                 0x04
#define AK8963_HYL                 0x05
#define AK8963_HYH                 0x06
#define AK8963_HZL                 0x07
#define AK8963_HZH                 0x08
#define AK8963_ST2                 0x09
// Write/Read Reg
#define AK8963_CNTL1               0x0A
#define AK8963_CNTL2               0x0B
#define AK8963_ASTC                0x0C
#define AK8963_TS1                 0x0D
#define AK8963_TS2                 0x0E
#define AK8963_I2CDIS              0x0F
// Read-only Reg ( ROM )
#define AK8963_ASAX                0x10
#define AK8963_ASAY                0x11
#define AK8963_ASAZ                0x12
 
// Configuration bits mpu9250
#define BIT_SLEEP                  0x40
#define BIT_H_RESET                0x80
#define BITS_CLKSEL                0x07
#define MPU_CLK_SEL_PLLGYROX       0x01
#define MPU_CLK_SEL_PLLGYROZ       0x03
#define MPU_EXT_SYNC_GYROX         0x02
#define BITS_FS_250DPS             0x00
#define BITS_FS_500DPS             0x08
#define BITS_FS_1000DPS            0x10
#define BITS_FS_2000DPS            0x18
#define BITS_FS_2G                 0x00
#define BITS_FS_4G                 0x08
#define BITS_FS_8G                 0x10
#define BITS_FS_16G                0x18
#define BITS_FS_MASK               0x18
#define BITS_DLPF_CFG_256HZ_NOLPF2 0x00
#define BITS_DLPF_CFG_188HZ        0x01
#define BITS_DLPF_CFG_98HZ         0x02
#define BITS_DLPF_CFG_42HZ         0x03
#define BITS_DLPF_CFG_20HZ         0x04
#define BITS_DLPF_CFG_10HZ         0x05
#define BITS_DLPF_CFG_5HZ          0x06
#define BITS_DLPF_CFG_2100HZ_NOLPF 0x07
#define BITS_DLPF_CFG_MASK         0x07
#define BIT_INT_ANYRD_2CLEAR       0x10
#define BIT_RAW_RDY_EN             0x01
#define BIT_I2C_IF_DIS             0x10
 
#define READ_FLAG                  0x80
 
/* ---- Sensitivity --------------------------------------------------------- */
 
#define MPU9250A_2g                ((float)0.000061035156f) // 0.000061035156 g/LSB
#define MPU9250A_4g                ((float)0.000122070312f) // 0.000122070312 g/LSB
#define MPU9250A_8g                ((float)0.000244140625f) // 0.000244140625 g/LSB
#define MPU9250A_16g               ((float)0.000488281250f) // 0.000488281250 g/LSB
 
#define MPU9250G_250dps            ((float)0.007633587786f) // 0.007633587786 dps/LSB
#define MPU9250G_500dps            ((float)0.015267175572f) // 0.015267175572 dps/LSB
#define MPU9250G_1000dps           ((float)0.030487804878f) // 0.030487804878 dps/LSB
#define MPU9250G_2000dps           ((float)0.060975609756f) // 0.060975609756 dps/LSB
 
#define MPU9250M_4800uT            ((float)0.6f)            // 0.6 uT/LSB
 
#define MPU9250T_85degC            ((float)0.002995177763f) // 0.002995177763 degC/LSB
 
#define Magnetometer_Sensitivity_Scale_Factor ((float)0.15f)    

// PROTOTYPES 

u8   MPU9250_write8(int module, u8 reg, u8 val);
u8   MPU9250_read8(int module, u8 reg);
void MPU9250_readBytes(int module, u8 reg, u8 *buffer, u8 length);
//u8 MPU9250_whoami(int module);
#define MPU9250_whoami(module)  MPU9250_read8(module, MPU9250_WHOAMI)

u8 AK8963_write8(int module, u8 reg, u8 val);
u8 AK8963_read8(int module, u8 reg);
void AK8963_readBytes(int module, u8 reg, u8 *buffer, u8 length);
//u8 AK8963_whoami(int module);
#define AK8963_whoami(module)   AK8963_read8(module, AK8963_WIA)

u8 MPU9250_init(int module, ...);

float* MPU9250_getMagnetometer(int module);
float* MPU9250_getAcceleration(int module);
float* MPU9250_getGyration(int module);
float  MPU9250_getTemperatureCelsius(int module);
float  MPU9250_getTemperatureFahrenheit(int module);
float  MPU9250_getYaw(int module, float declination);
float  MPU9250_getPitch(int module);
float  MPU9250_getRoll(int module);

void MPU9250_read_all(int module);
void MPU9250_getBiases(int module, float *dest1, float *dest2);
u16   MPU9250_setGyroscopeScale(int module, u8 scale);
u8   MPU9250_setAccelerometerScale(int module, u8 scale);
void MPU9250_getAccelerometerCalibration(int module);
void MPU9250_getMagnetometerCalibration(int module);
//u8   MPU9250_getCNTL1(int module);
#define MPU9250_getCNTL1(module) AK8963_read8(module, AK8963_CNTL1)

/*
#define MPU9250_getGyroData()[0];
#define MPU9250.gyro_data[1];
#define MPU9250.gyro_data[2];
#define MPU9250.accel_data[0];
#define MPU9250.accel_data[1];
#define MPU9250.accel_data[2];
#define MPU9250.mag_data[0];
#define MPU9250.mag_data[1];
#define MPU9250.mag_data[2];
#define MPU9250.temperature;
*/

#endif // MPU9250_H
