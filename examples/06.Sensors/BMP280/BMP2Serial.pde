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

const u8 MODULE = BMP280SPI2;
//const u8 MODULE = BMP280I2C1;
u8 failed = false;

void setup()
{
    pinMode (USERLED, OUTPUT); 
    Serial2.begin(9600);
    Serial2.print("\f\f\f");// CLS
    Serial2.println("*** BMP280 ***");

    // If SPI is used (CSB connected to ground)
    if (!BMP280.begin(MODULE))
    // If I2C is used (CSB connected to VDDIO)
    // 0x76 because SDO is not connected or connected to ground 
    //if (!BMP280.begin(MODULE, 0x76))
    {
        Serial2.println("Could not find a valid BMP280 sensor, check wiring!");
        failed = true;
    }
}

void loop()
{
    s32 TC, TF;         // Temperatures in 0.01 degC and 0.01 degF
    u32 P, P0 = 101325; // Pressure and Pressure at sea level in Pa
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
        // Oversampling: 0 to 4
        // the higher the number, the higher the resolution outputs
        // the higher the number, the slower the measurment
        // the higher the number, the higher the power consumption
        BMP280.startMeasurment(MODULE, 4); // max. resolution from 0 to 4
        TC = BMP280.getTemperatureCelsius(MODULE);   // in 0.01 DegC
        TF = BMP280.getTemperatureFahrenheit(MODULE);// in 0.01 DegF
        P  = BMP280.getPressure(MODULE);             // in Pa (Note : 1 Bar = 100000 Pa)
        A  = BMP280.getAltitude(MODULE, P, P0);
        //P0 = BMP280.getPressureAtSealevel(MODULE, P, 0); // Sea level = 0 m

        Serial2.print("\f"); // CLS

        Serial2.print("T = ");
        Serial2.printNumber(TC/100, DEC);
        Serial2.printChar('.');
        Serial2.printNumber((u32)TC%100, DEC);
        Serial2.println(" degC");

        Serial2.print("T = ");
        Serial2.printNumber(TF/100, DEC);
        Serial2.printChar('.');
        Serial2.printNumber((u32)TF%100, DEC);
        Serial2.println(" degF");

        Serial2.print("P = ");
        Serial2.printNumber(P/100,  DEC);
        Serial2.println(" hPa");

        Serial2.print("A = ");
        Serial2.printNumber(A,  DEC);
        Serial2.println(" m");

        // Wait 5 sec. before the next reading
        toggle(USERLED);
        delay(5000);
    }
}
