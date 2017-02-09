/*  -----------------------------------------------------------------------
    Description      Reads DS18B20 1-wire temperature sensor
    Author           Régis Blanchot
    First release    14/09/2010
    Last update      22/05/2016
    IDE              Pinguino version >= 12
    -----------------------------------------------------------------------
    DS18B20 wiring
    -----------------------------------------------------------------------
    pin 1: GND
    pin 2: DQ (Data in/out) must be connected to the PIC
    pin 3: VDD (+5V)
    NB : 1-wire bus (DQ line) must have 4K7 pull-up resistor (connected to +5V)
    ----------------------------------------------------------------------*/

#define ONEWIREBUS	0   // DQ line						

void setup()
{
    pinMode(USERLED, OUTPUT);
    Serial.begin(9600);
    Serial.println("\f*** Single DS18x20 Demo ***");
}

void loop()
{
    TEMPERATURE t;
 
    if (DS18x20.read(ONEWIREBUS, SKIPROM, &t))
    {
        if (t.sign)
            Serial.printChar('-');
        Serial.printf("%2d.%02dC \r\n", t.integer, t.fraction);
        //Serial.printNumber(t.integer, DEC);
        //Serial.printChar('.');
        //Serial.printNumber(t.fraction, DEC);
        //Serial.print("C \r\n");
    }
    
    delay(1000);
    toggle(USERLED);
}
