/*	-----------------------------------------------------------------------
	Pinguino example to read ds18b20 1wire temperature sensor
	Result is sent on usb-serial bus and can be read with index.php
	author			Régis Blanchot
	first release	14/09/2010
	last update		10/06/2011
	IDE				Pinguino > b9.5
	-----------------------------------------------------------------------
	DS18B20 wiring
	-----------------------------------------------------------------------
	pin 1: GND
	pin 2: DQ (Data in/out) must be connected to the PIC
	pin 3: VDD (+5V)
	NB : 1-wire bus (DQ line) must have 4K7 pull-up resistor (connected to +5V)
	-----------------------------------------------------------------------
	Data's are sent to /dev/ttyACM0
	Make sure you have persmission on it : sudo chmod 777 /dev/ttyACM0
	Maybe you will have to add your user name to the dialup group
	----------------------------------------------------------------------*/

#define ONEWIREBUS	14						// DQ line
						
int ifar;
int ffar;

void setup()
{
}

void loop()
{
	TEMPERATURE t;

	ifar = t.integer * 100;
    ifar += t.fraction;
    ifar = ((ifar * 9) / 5) + 3200;
    ffar = ifar % 100;   
    ifar /= 100;

	if (DS18B20.read(ONEWIREBUS, SKIPROM, RES12BIT, &t))
	{
		if (t.sign) CDC.printf("-");
		CDC.printf("%d.%dC \r", t.integer, t.fraction);
		CDC.printf("%d.%dF \r", ifar, ffar);
	}

	delay(1000);
}
