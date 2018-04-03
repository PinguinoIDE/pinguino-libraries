/* ---------------------------------------------------------------------
    Pinguino example to read DS18b20 1-wire temperature sensor
    author	        Régis Blanchot
    first release	14/09/2010
    last update	        10/11/2016
    IDE		        Pinguino > v12
    --------------------------------------------------------------------
    DS18B20 wiring
    --------------------------------------------------------------------
    pin 1: GND
    pin 2: DQ (Data in/out) must be connected to the PIC
    pin 3: VDD (+5V)
    NB : 1-wire bus (DQ line) must have 4K7 pull-up resistor (connected to +5V)
    ------------------------------------------------------------------*/

#define ONEWIREBUS	0    // DQ line on pin 0

TEMPERATURE t;
u8 sensor;

void setup()
{
    pinMode(USERLED, OUTPUT);
    Serial.begin(9600);
    delay(1000);
    
    sensor = DS18x20.find(ONEWIREBUS);
    Serial.println("\f*** 18x20 Demo ***");
    Serial.printNumber(sensor, DEC);
    Serial.print(" device(s) detected\r\n");
}

void loop()
{
    u8 i;
    
    for (i=1; i<=sensor; i++)
    {
        if (DS18x20.read(ONEWIREBUS, SKIPROM, &t))
        {
            if (t.sign)
                Serial.printChar('-');
            //Serial.printf("%d.%dC \r\n", t.integer, t.fraction);
            Serial.printNumber(t.integer, DEC);
            Serial.printChar('.');
            Serial.printNumber(t.fraction, DEC);
            Serial.print("C \r\n");
        }
    }
    delay(1000);
    toggle(USERLED);
}
