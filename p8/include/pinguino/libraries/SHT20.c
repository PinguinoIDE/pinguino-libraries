/*    --------------------------------------------------------------------
    FILE:           SHT20.c
    PROJECT:        Pinguino
    PURPOSE:        Read current humidity and temperature on SHT20 sensor.
    PROGRAMER:      regis blanchot <rblanchot@gmail.com>
    --------------------------------------------------------------------
    25 Jan 2019	Régis Blanchot      first version based on DFRobot file
    --------------------------------------------------------------------
    TODO :
    --------------------------------------------------------------------
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
    ------------------------------------------------------------------*/

#ifndef __SHT20_C
    #define __SHT20_C
    #define __SHT20__
    //#define DS18X20DEBUG

    #include <const.h>                  // false, true, ...
    #include <macro.h>
    #include <typedef.h>
	#include <SHT20.h>
	#include <i2c.c>

	u8 SHT20_I2CMODULE;
	
/*  --------------------------------------------------------------------
    ---------- GLOBAL VARIABLES
    ------------------------------------------------------------------*/

void SHT20_init(u8 i2c_module)
{
	SHT20_I2CMODULE = i2c_module;
	
    I2C_init(i2c_module, I2C_MASTER_MODE, I2C_100KHZ);
    //I2C_init(i2c_module, I2C_MASTER_MODE, I2C_400KHZ);
    //I2C_init(i2c_module, I2C_MASTER_MODE, I2C_1MHZ);
}

u16 SHT20_readValue(u8 cmd)
{
    u8 msb, lsb, checksum;
    u8 toRead;
	u8 b[MAX_COUNTER];
    u8 counter;
    u16 rawValue;
	
    // Send command

    I2C_start(SHT20_I2CMODULE);                              // send start condition
    I2C_write(SHT20_I2CMODULE, (SLAVE_ADDRESS << 1) & 0xFE); // write operation (bit 0 set to 0)
    I2C_write(SHT20_I2CMODULE, cmd);

    // Read 3 bytes

    I2C_start(SHT20_I2CMODULE);                            // send start condition again
    I2C_write(SHT20_I2CMODULE, (SLAVE_ADDRESS << 1) + 1);  // read operation (bit 0 set to 1)
	delay(DELAY_INTERVAL);
    msb = I2C_read(SHT20_I2CMODULE);
	delay(DELAY_INTERVAL);
    lsb = I2C_read(SHT20_I2CMODULE);
	delay(DELAY_INTERVAL);
    checksum = I2C_read(SHT20_I2CMODULE);
	delay(DELAY_INTERVAL);
	
	// End of the read sequence

    I2C_stop(SHT20_I2CMODULE);                             // send stop confition

    rawValue = ((u16) msb << 8) | (u16) lsb;

    if (SHT20_checkCRC(rawValue, checksum) != 0)
        return (ERROR_BAD_CRC);

    return (rawValue & 0xFFFC);
}

float SHT20_readHumidity(void)
{
    float tempRH;
    float rh;
    u16 rawHumidity = SHT20_readValue(TRIGGER_HUMD_MEASURE_NOHOLD);

    if (rawHumidity == ERROR_I2C_TIMEOUT || rawHumidity == ERROR_BAD_CRC)
        return (rawHumidity);

    tempRH = rawHumidity * (125.0 / 65536.0);
    rh = tempRH - 6.0;
    return (rh);
}

float SHT20_readTemperature(void)
{
    float tempTemperature;
    float realTemperature;
    u16 rawTemperature = SHT20_readValue(TRIGGER_TEMP_MEASURE_NOHOLD);

    if(rawTemperature == ERROR_I2C_TIMEOUT || rawTemperature == ERROR_BAD_CRC)
        return (rawTemperature);

    tempTemperature = rawTemperature * (175.72 / 65536.0);
    realTemperature = tempTemperature - 46.85;
    return (realTemperature);
}

void SHT20_setResolution(u8 resolution)
{
    u8 userRegister = SHT20_readUserRegister();
    userRegister &= B01111110;
    resolution &= B10000001;
    userRegister |= resolution;
    SHT20_writeUserRegister(userRegister);
}

u8 SHT20_readUserRegister(void)
{
    u8 userRegister;
    i2cPort->beginTransmission(SLAVE_ADDRESS);
    i2cPort->write(READ_USER_REG);
    i2cPort->endTransmission();
    i2cPort->requestFrom(SLAVE_ADDRESS, 1);
    userRegister = I2C_read(SHT20_I2CMODULE);
    return (userRegister);
}

void SHT20_writeUserRegister(u8 val)
{
    i2cPort->beginTransmission(SLAVE_ADDRESS);
    i2cPort->write(WRITE_USER_REG);
    i2cPort->write(val);
    i2cPort->endTransmission();
}

u8 SHT20_checkCRC(u16 message_from_sensor, u8 check_value_from_sensor)
{
	u8 i;
    u32 divsor = (u32)SHIFTED_DIVISOR;
    u32 remainder = (u32)message_from_sensor << 8;

    remainder |= check_value_from_sensor;
    for(i = 0 ; i < 16 ; i++)
	{
        if(remainder & (u32)1 << (23 - i))
            remainder ^= divsor;
        divsor >>= 1;
    }
    return (u8)remainder;
}

u8 SHT20_checkBattery(void)
{
    u8 reg = SHT20_readUserRegister();
    // End of battery ? 1=Yes, 0=No
	return (reg & USER_REGISTER_END_OF_BATTERY);
}

u8 SHT20_checkHeater(void)
{
    u8 reg = SHT20_readUserRegister();
    // Heater enabled ? 1=Yes, 0=No
	return (reg & USER_REGISTER_HEATER_ENABLED);
}

u8 SHT20_checkOTP(void)
{
    u8 reg = SHT20_readUserRegister();
    // Disable OTP reload ? 1=Yes, 0=No
	return (reg & USER_REGISTER_DISABLE_OTP_RELOAD);
}
