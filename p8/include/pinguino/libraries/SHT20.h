#ifndef __SHT20_H
#define __SHT20_H

#include <typedef.h>

#define ERROR_I2C_TIMEOUT                     998
#define ERROR_BAD_CRC                         999
#define SLAVE_ADDRESS                         0x40 
#define TRIGGER_TEMP_MEASURE_HOLD             0xE3
#define TRIGGER_HUMD_MEASURE_HOLD             0xE5
#define TRIGGER_TEMP_MEASURE_NOHOLD           0xF3
#define TRIGGER_HUMD_MEASURE_NOHOLD           0xF5
#define WRITE_USER_REG                        0xE6
#define READ_USER_REG                         0xE7
#define SOFT_RESET                            0xFE
#define USER_REGISTER_RESOLUTION_MASK         0x81
#define USER_REGISTER_RESOLUTION_RH12_TEMP14  0x00
#define USER_REGISTER_RESOLUTION_RH8_TEMP12   0x01
#define USER_REGISTER_RESOLUTION_RH10_TEMP13  0x80
#define USER_REGISTER_RESOLUTION_RH11_TEMP11  0x81
#define USER_REGISTER_END_OF_BATTERY          0x40
#define USER_REGISTER_HEATER_ENABLED          0x04
#define USER_REGISTER_DISABLE_OTP_RELOAD      0x02
#define MAX_WAIT                              100
#define DELAY_INTERVAL                        10
#define SHIFTED_DIVISOR                       0x988000
#define MAX_COUNTER                           (MAX_WAIT/DELAY_INTERVAL)

//public:
void  SHT20_setResolution(u8 resBits);
void  SHT20_init(TwoWire &wirePort = Wire);

float SHT20_readHumidity(void);
float SHT20_readTemperature(void);

void  SHT20_writeUserRegister(u8 val);
u8    SHT20_readUserRegister(void);

u8    SHT20_checkBattery(void);
u8    SHT20_checkHeater(void);
u8    SHT20_checkOTP(void);

//private:
u8    SHT20_checkCRC(u16 message_from_sensor, u8 check_value_from_sensor);
u16   SHT20_readValue(u8 cmd);

#endif /* __SHT20_H */