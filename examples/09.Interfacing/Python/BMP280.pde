/*-----------------------------------------------------
Author:      Regis Blanchot <rblanchot@pinguino.cc>
Date: 	   2017-12-09
Description: Get Temperature, Pressure and Altitude
	   From the Bosch BMP280 Sensor
Note1:       If CSB is connected to VDDIO, the I2C interface is active.
             Some boards have a 10k pull up on CSB,
             so in I2C you just leave that pin not connected.
             If CSB is pulled down, the SPI interface is activated. 
Note2:       Connecting SDO to GND  results in I2C slave 7-bit address 1110110 (0x76)
             Connecting it to VDDIO results in I2C slave 7-bit address 1110111 (0x77)
Note3:       The SPI interface uses the following pins:
             CSB: chip select to pin CS of your Pinguino
             SCK: serial clock to pin SCK of your Pinguino
             SDI (or SDA): serial data input to pin SDO of your Pinguino
             SDO: serial data output to pin SDI of your Pinguino
-----------------------------------------------------*/

const u8 MODULE = BMP280I2C2;
u8 failed = false;

void setup()
{
    pinMode (USERLED, OUTPUT); 

    // If SPI is used (CSB connected to ground)
    //if (!BMP280.begin(MODULE))
    // If I2C is used (CSB connected to VDDIO)
    // 0x76 because SDO is not connected or connected to ground 
    if (!BMP280.begin(MODULE, 0x76))
        failed = true;

    // Enable Watchdog Timer
    // Watchdog is driven by the Internal Oscillator (8MHz)
    // Nominal period of Watchdog is 4ms
    // Watchdog postscaler is set to 1:32768 by config. bits
    // Watchdog timer will overload after 32768*4ms = 135sec 
    //Watchdog.enable();
}

void loop()
{
    u32 p;
    s32 t;

    if (failed)
    {
        digitalwrite(USERLED, HIGH);
        delay(100);
        digitalwrite(USERLED, LOW);
        delay(300);
    }
    else
    {
        digitalwrite(USERLED, HIGH);
        // Oversampling: 0 to 4
        // the higher the number, the higher the resolution outputs
        // the higher the number, the slower the measurment
        // the higher the number, the higher the power consumption
        BMP280.startMeasurment(MODULE, 4);
        // Get Pressure in Pa
        p = BMP280.getPressure(MODULE);
        // Get Temperature in 0.01 degC
        t = BMP280.getTemperatureCelsius(MODULE);

        // Send start condition (5 bytes) on the USB bus
        USB.send("START", 5);
        //delay(100);
        // Send pressure (4 bytes) on the USB bus
        USB.send(&p, 4);
        //delay(100);
        // Send temperature (4 bytes) on the USB bus
        USB.send(&t, 4);
        //delay(100);

        digitalwrite(USERLED, LOW);
        delay(1000);
        // Enter Sleep Mode before the next reading
        // Wait for watchdog timer overload
        //System.sleep();
    }
}
