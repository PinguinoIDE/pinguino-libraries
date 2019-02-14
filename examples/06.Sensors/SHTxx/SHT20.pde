/*
 * SHT20.pde
 * SHT20 Humidity And Temperature Sensor Module
 * This example demonstrates how to read the user registers to display resolution and other settings.
 * Uses the SHT20 library to display the current humidity and temperature.
 * Open serial monitor at 9600 baud to see readings.
 * Errors 998 if not sensor is detected. Error 999 if CRC is bad.
 * Hardware Connections:
 * -VCC = 3.3V
 * -GND = GND
 * -SDA = 4 (use inline 330 ohm resistor if your board is 5V)
 * -SCL = 5 (use inline 330 ohm resistor if your board is 5V)
 * To avoid signal contention the micro-controller unit (MCU) must only drive SDA and SCL low.
 * External pull-up resistors (e.g. 10kΩ), are required to pull the signal high. 
 */

void setup()
{
    Serial.begin(9600);
    Serial.println("SHT20 Example!");
    SHT20.init();                                  // Init SHT20 Sensor
    delay(100);
	// Check battery
    Serial.printf("End of battery: %d\r\n", SHT20.checkBattery());
	// Check heater
    Serial.printf("Heater enabled: %d\r\n", SHT20.checkHeater());
	// Check OTP
    Serial.printf("Disable OTP reload: %d\r\n", SHT20.checkOTP());
}

void loop()
{
    float humd = SHT20.readHumidity();             // Read Humidity
    float temp = SHT20.readTemperature();          // Read Temperature
    Serial.printf("Time: %l", millis());
    Serial.printf(" Temperature:%fC", temp);
    Serial.printf(" Humidity:%f%\r\n", humd);
    delay(1000);
}
