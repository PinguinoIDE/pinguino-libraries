/*-----------------------------------------------------
Author:      Regis Blanchot <rblanchot@pinguino.cc>
Date: 	   2017-01-20
Description: Get Temperature, Pressure and Altitude
	   From the Bosch BMP280 Sensor
-----------------------------------------------------*/

const u8 MODULE = BMP280SPI1;

void setup()
{
  Serial.begin(9600);
  if(!BMP280.begin(MODULE))
  {
    Serial.println("BMP init failed!");
    while(1);
  }
  else
    Serial.println("BMP init success!");
}

void loop()
{
  float TC, TF, P, A, S;
  float P0 = 1013.25; // Pressure at sea level

  // Oversampling: 0 to 4
  // the higher the number, the higher the resolution outputs
  // the higher the number, the slower the measurment
  BMP280.startMeasurment(MODULE, 4); // max. resolution
  TC = BMP280.getTemperatureCelsius(MODULE);
  TF = BMP280.getTemperatureFahrenheit(MODULE);
  P  = BMP280.getPressure(MODULE);
  A  = BMP280.getAltitude(MODULE, P, P0);
  P0 = BMP280.getPressureAtSealevel(MODULE, P, A);
        
  //Serial.print("T = \t"); Serial.printFloat(TF, 2); Serial.println(" degF\t");
  //Serial.print("P = \t"); Serial.printFloat(P, 2); Serial.println(" mBar\t");
  //Serial.print("A = \t"); Serial.printFloat(A, 2); Serial.println(" m");
  delay(1000); // Wait 1 sec. before the next reading
}
