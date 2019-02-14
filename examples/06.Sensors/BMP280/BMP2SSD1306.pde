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

#include <fonts/ArialBold14.h>

const u8 OLED = SSD1306_I2C1;
const u8 MODULE = BMP280I2C2;
u8 failed = false;

void setup()
{
    pinMode (USERLED, OUTPUT); 

    // Init. the OLED screen 
    SSD1306.init(OLED, 0x78);
    SSD1306.clearScreen(OLED);
    SSD1306.setFont(OLED, ArialBold14);

    // If SPI is used (CSB connected to ground)
    //if (!BMP280.begin(MODULE))
    // If I2C is used (CSB connected to VDDIO)
    // 0x76 because SDO is not connected or connected to ground 
    if (!BMP280.begin(MODULE, 0x76))
    {
        SSD1306.println(OLED, "Could not find a valid BMP280 sensor, check wiring!");
        failed = true;
    }

    // Enable Watchdog Timer
    // Watchdog is driven by the Internal Oscillator (8MHz)
    // Nominal period of Watchdog is 4ms
    // Watchdog postscaler is set to 1:32768 by config. bits
    // Watchdog timer will overload after 32768*4ms = 135sec 
    Watchdog.enable();
}

void loop()
{
    s32 TC;             // Temperatures in 0.01 degC
    u32 P, P0 = 102700; // Pressure and Pressure at sea level in Pa
    s32 A;              // Altitude in meters

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
        BMP280.startMeasurment(MODULE, 4); // max. resolution from 0 to 4
        TC = BMP280.getTemperatureCelsius(MODULE);   // in 0.01 DegC
        P  = BMP280.getPressure(MODULE);             // in Pa (Note : 1 Bar = 100000 Pa)
        //A  = BMP280.getAltitude(MODULE, P, P0);
        //P0 = BMP280.getPressureAtSealevel(MODULE, P, 0); // Sea level = 0 m

        SSD1306.clearScreen(OLED);

        SSD1306.print(OLED, "T = ");
        SSD1306.printNumber(OLED, TC/100, DEC);
        SSD1306.printChar(OLED, '.');
        SSD1306.printNumber(OLED, (u32)TC%100, DEC);
        SSD1306.println(OLED, " degC");

        SSD1306.print(OLED, "P = ");
        SSD1306.printNumber(OLED, P/100,  DEC);
        SSD1306.println(OLED, " hPa");
        /*
        SSD1306.print(OLED, "A = ");
        SSD1306.printNumber(OLED, A,  DEC);
        SSD1306.println(OLED, " m");
        */
        SSD1306.refresh(OLED);

        digitalwrite(USERLED, LOW);
        // Enter Sleep Mode before the next reading
        System.sleep();         //  wait for watchdog timer overload
    }
}
