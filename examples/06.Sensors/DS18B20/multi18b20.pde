/* -----------------------------------------------------------------------
    Pinguino example to read ds18b20 1wire temperature sensor
    Result is sent on usb-serial bus and can be read with index.php
    author	Régis Blanchot
    first release	14/09/2010
    last update	22/05/2016
    IDE		Pinguino version >= 12
    -----------------------------------------------------------------------
    DS18B20 wiring
    -----------------------------------------------------------------------
    pin 1: GND
    pin 2: DQ (Data in/out) must be connected to the PIC
    pin 3: VDD (+5V)
    NB : 1-wire bus (DQ line) must have 4K7 pull-up resistor (connected to +5V)
    ----------------------------------------------------------------------*/

#define ONEWIREBUS	0    // DQ line						

TEMPERATURE t;
u8 numsensor;

void setup()
{
    u8 i;

    pinMode(USERLED, OUTPUT);
    Serial.begin(9600);
    delay(1000);
    Serial.println("\f*** Multi DS18x20 Demo ***");
    
    // Find all sensors present on the bus
    DS18x20.find(ONEWIREBUS);
    numsensor = DS18x20.deviceCount();
    
    // Optional : set alarms and resolution
    for (i=1; i<=numsensor; i++)
        DS18x20.configure(ONEWIREBUS, i, -40, 60, RES12BIT);
    // Or simpler, address them all with SKIPROM
    //DS18B20.configure(ONEWIREBUS, SKIPROM, -40, 60, RES12BIT);
    
    Serial.printNumber(numsensor, DEC);
    Serial.print(" device(s) detected\r\n");
}

void loop()
{
    u8 i;
    
    for (i=1; i<=numsensor; i++)
    {
        if (DS18x20.read(ONEWIREBUS, i, &t))
        {
            //Serial.printf("Sensor #%d : ", i);
            Serial.print("Sensor #");
            Serial.printNumber(i, DEC);
            Serial.print(" : ");
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
