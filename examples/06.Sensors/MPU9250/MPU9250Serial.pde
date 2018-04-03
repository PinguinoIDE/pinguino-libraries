/*-----------------------------------------------------
Author:      Regis Blanchot <rblanchot@pinguino.cc>
Date: 	   2018-01-18
Description: Get Accelerometer, Magnetomater, Gyroscope and Temperature
	   From the InvenSense MPU9250 Sensor
Note1:       If MCS is connected to VDD, the I2C interface is active.
             Some boards have a 10k pull up on MCS,
             so in I2C you just leave that pin not connected.
             If MCS is pulled down, the SPI interface is activated. 
Note2:       MCS: chip select to pin CS (SPI) of your Pinguino
             SCL/SCK: Serial CLocK to pin SCL (I2C) or SCK (SPI) of your Pinguino
             SDA/SDI: Serial Data Input to pin SDA (I2C) or SDO (SPI) of your Pinguino
             AD0/SDO: Serial Data Output to pin SDI (SPI) of your Pinguino
                      Connecting ADO to GND results in I2C slave 7-bit address 1110000 (0x70)
                      Connecting ADO to VDD results in I2C slave 7-bit address 1110001 (0x71)
-----------------------------------------------------*/

//const u8 MODULE = MPU9250SPI1;
const u8 MODULE = MPU9250SPI2;
//const u8 MODULE = MPU9250SPISW;

void setup()
{
    //u8 r = MPU9250.init(MODULE);
    pinMode(USERLED, OUTPUT);

    Serial2.begin(9600);
    Serial2.print("\f\f\f");

    //while(1)
    //{
    //    Serial2.printx("Dev. ID = ", r, HEX);
    //    delay(1000);
    //}

    //if (!MPU9250.init(MODULE, 1, 3, 2, 0)) // SDO, SDI, SCK, CS
    if (!MPU9250.init(MODULE))
    {
        Serial2.println("Connection failed.");
        noEndLoop();
    }
    Serial2.println("Yaw\tPitch\tRoll");
}

void loop()
{
    // We need acc, gyr and mag to calculate yaw, pitch and roll
    float * acc = MPU9250.getAcceleration(MODULE);
    float * gyr = MPU9250.getGyration(MODULE);
    float * mag = MPU9250.getMagnetometer(MODULE);
    //float tc    = MPU9250.getTemperatureCelsius(MODULE);
    //float tf    = MPU9250.getTemperatureFahrenheit(MODULE);

    // Sensors x (y)-axis of the accelerometer is aligned with the y (x)-axis of the magnetometer;
    // the magnetometer z-axis (+ down) is opposite to z-axis (+ up) of accelerometer and gyro!
    // We have to make some allowance for this orientation mismatch in feeding the output
    // to the quaternion filter.
    // For the MPU-9250, we have chosen a magnetic rotation that keeps the sensor forward along the x-axis.
    // This rotation can be modified to allow any convenient orientation convention.
    // This is ok by aircraft orientation standards!  
    // Pass gyro rate as rad/s
    // MadgwickQuaternionUpdate(acc[0], acc[1], acc[2], gyr[0]*PI/180.0f, gyr[1]*PI/180.0f, gyr[2]*PI/180.0f, mag[1], mag[0], mag[2]);
    MahonyQuaternionUpdate(acc[0], acc[1], acc[2], gyr[0]*PI/180.0f, gyr[1]*PI/180.0f, gyr[2]*PI/180.0f, mag[1], mag[0], mag[2]);

    float yaw   = MPU9250.getYaw(MODULE, 1.36f); // declination
    float pitch = MPU9250.getPitch(MODULE);
    float roll  = MPU9250.getRoll(MODULE);

    /*
    Serial.print("\f");
    Serial.println("----------");
    Serial.println("GyrX\tGyrY\tGyrZ");
    Serial.printf("%f\t%f\t%f\t\r\n", gyr[0], gyr[1], gyr[2]);
    Serial.println("----------");
    Serial.println("AccX\tAccY\tAccZ");
    Serial.printf("%f\t%f\t%f\t\r\n", acc[0], acc[1], acc[2]);
    Serial.println("----------");
    Serial.println("MagX\tMagY\tMagZ");
    Serial.printf("%f\t%f\t%f\t\r\n", mag[0], mag[1], mag[2]);
    Serial.println("----------");
    Serial.println("Tc\tTf");
    Serial.printf("%f\t%f\r\n", tc, tf);
    Serial.println("----------");
    */

    Serial2.printf("%f\t%f\t%f\r", yaw, pitch, roll);

    toggle(USERLED);
    //delay(1000);
}
