/*
    Pinguino example to read DS18B20 1wire temperature sensor
    Result is sent on USB bus and can be read with thermometer.py
    author          Régis Blanchot
    first release   14/09/2010
    last update     22/05/2016
    IDE             Pinguino version >= 12

    DS18B20 Connection
    ------------------
    pin 1: GND
    pin 2: DQ (Data in/out) must be connected to the PIC
    pin 3: VDD (+5V)
    NB : 1-wire bus (DQ line) must have 4K7 pull-up resistor (connected to +5V)
*/

#define ONEWIREBUS	31    // 1-wire bus
                          // can be on any other pin 

void setup()
{
    pinMode(USERLED, OUTPUT);
    lcd.pins(0, 1, 0, 0, 0, 0, 2, 3, 4, 5); // RS, E, D4 ~ D8
    lcd.begin(16, 2);
    //lcd.home(); // 0, 0
    // Print a message to the LCD.
}

void loop()
{
    TEMPERATURE t;

    toggle(USERLED);
    //DS18x20.readFahrenheit(ONEWIREBUS, SKIPROM, &t);  //Fahrenheit reading
    DS18x20.readCelsius(ONEWIREBUS, SKIPROM, &t);  //Celsius reading
    lcd.home(); //lcd.setCursor(0,0);
    lcd.printf("Temp: %02d.%02d\337C", t.integer, t.fraction);
    delay(1000);
}
