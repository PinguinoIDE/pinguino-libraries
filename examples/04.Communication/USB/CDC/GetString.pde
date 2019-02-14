//	Input/Output on GNU/Linux:	sudo minicom -o -D /dev/ttyACM0

u8 firstname[10];
u8 lastname[10];

void setup()
{
    pinMode(USERLED, OUTPUT);
}

void loop()
{
    toggle(USERLED);

    CDC.print("First Name ? ");
    CDC.getString(firstname);
    CDC.print("\r\n");
    //CDC.printf(">[%s]\r\n", firstname);

    CDC.print("Last Name ? ");
    CDC.getString(lastname);
    CDC.print("\r\n");
    //CDC.printf(">[%s]\r\n", lastname);
    
    CDC.printf("Hello %s %s !\r\n", firstname, lastname);
}
