/*-----------------------------------------------------
Author:  --<Dave Maners>
Date: 2017-09-20
Description: 10K Thermistor
Tested with : Pinguino 4550, Pinguino 32MX270 
-----------------------------------------------------*/

#define AA 0.001129148f
#define BB 0.000234125f
#define CC 8.76741E-08f
#define RR 10000.0f        // 10K Thermistor

void setup()
{            
    Serial.begin(9600);
    pinMode(USERLED, OUTPUT);
}

float ThermistorResistance(u8 pin)
{  
    float RawADC = 500.0;
    //float RawADC = analogRead(pin);
    //float Re = RR / (1024.0 / RawADC - 1.0);
    float Re = RR * (1024.0 / RawADC - 1.0);
    //Serial.printf("Resistance = %f Ohm\r\n", Re);
    return Re;
}

// Steinhart-Hart Thermistor Equation:
// Temperature in Kelvin = 1 / {A + B[ln(R)] + C[ln(R)]^3} 
float ThermistorKelvin(u8 pin)
{  
    float Re = ThermistorResistance(pin);
    float temp = fastln(Re);
    //Serial.printf("ln(Resistance) = %f\r\n", temp);
    float Ke = 1.0 / (AA + (BB * temp) + (CC * temp * temp * temp));
    //Serial.printf("Kelvin = %f K\r\n", Ke);
    return Ke;
}

float ThermistorCelsius(u8 pin)
{  
    float Ke = ThermistorKelvin(pin);
    // Convert Kelvin to Celsius
    float Ce = Ke - 273.15;
    //Serial.printf("Celsius = %f\176C\r\n", Ce);
    return Ce;
}

float ThermistorFahrenheit(u8 pin)
{  
    float Ce = ThermistorCelsius(pin);
    // Convert Celsius to Fahrenheit
    float Fa = Ce * 1.8 + 32.0;
    //Serial.printf("Fahrenheit = %f\176F\r\n", Fa);
    return Fa;
}
 
void loop()
{
    float Te;

    digitalWrite(USERLED, HIGH);             
    // Read the analog port A0 and
    // Convert the value in temperature
    Te = ThermistorCelsius(0);
    // Print the value to the serial port
    Serial.printf("Temp = %f\176C\r\n", Te);
    digitalWrite(USERLED, LOW);             
    // Wait one second
    delay(1000);
}
