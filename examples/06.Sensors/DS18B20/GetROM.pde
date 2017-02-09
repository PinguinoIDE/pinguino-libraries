/* -----------------------------------------------------------------------
    Pinguino example to read DS18x20 ROM Code
    author	Régis Blanchot
    first release	06/11/2016
    last update	06/11/2016
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
    u8 i, j;
    u8 romcode[8];

    pinMode(USERLED, OUTPUT);
    Serial.begin(9600);
    delay(1000);
    Serial.println("\f*** DS18x20 Demo ***");
    
    // Find all sensors present on the bus
    DS18x20.find(ONEWIREBUS);
    numsensor = DS18x20.deviceCount();
    Serial.printNumber(numsensor, DEC);
    Serial.print(" device(s) detected\r\n");
    
    // Print ROM code of each devices
    for (i=1; i<=numsensor; i++)
    {
        Serial.print("Sensor #");
        Serial.printNumber(i, DEC);
        Serial.print(" Hex ROM Code' : 0x");
        DS18x20.readRom(ONEWIREBUS, i, &romcode);
        for (j=0; j<8; j++)
        {
            Serial.printf("%X", romcode[j]);
        }
        Serial.print("\r\n");
    }
}

void loop()
{
    u8 i;
    
    for (i=1; i<numsensor; i++)
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
