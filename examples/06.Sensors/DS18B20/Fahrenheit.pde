/*
    Pinguino example to read DS18B20 1wire temperature sensor
    Result is in Fahrenheit
    and sent on Serial bus
    author          Régis Blanchot
    first release   30/10/2016
    last update     30/10/2016
    IDE             Pinguino version >= 12

    DS18B20 Connection
    ------------------
    pin 1: GND
    pin 2: DQ (Data in/out) must be connected to the PIC
    pin 3: VDD (+5V)
    NB : 1-wire bus (DQ line) must have 4K7 pull-up resistor (connected to +5V)
*/

#define ONEWIREBUS	0    // 1-wire bus is on pin 0 - can be on any other port

void setup()
{
    pinMode(USERLED, OUTPUT);
    Serial.begin(9600);
}

void loop()
{
    TEMPERATURE t;

    toggle(USERLED);
    DS18x20.readFahrenheit(ONEWIREBUS, SKIPROM, &t);
    Serial.printf("%d.%d degF\r\n", t.integer, t.fraction);
    delay(1000);
}
