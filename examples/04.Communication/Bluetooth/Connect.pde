/*	---------------------------------------------------------------------------
	Connect a Bluetooth Device to your Pinguino
	2011 Regis Blanchot <rblanchot@gmail.com>
	http://www.pinguino.cc
	---------------------------------------------------------------------------
	tested with Olimex PIC32-PINGUINO & MOD-BT Bluetooth Transceiver Module with BGB203
	output : sudo minicom -o -D /dev/ttyACM0
	---------------------------------------------------------------------------*/

/*
char * Serial2GetString()
{
	u8 i = 0;
	char c;
	static char buffer[80];
	
	do {
		while (!Serial2.available());
		c = Serial2.read();
		CDC.printf("%c\r\n", c);
		buffer[i++] = c;
	} while (c != '\r');
	buffer[i] = '\0';
	Serial2.flush();

	return (buffer);
}
*/

void setup()
{
    u8 i;
    pinMode(USERLED, OUTPUT);
    CDC.printf("TEST\r\n");
    CDC.printf("TEST\r\n");
    CDC.printf("TEST\r\n");
    for (i=0; i<10; i++)
    {    
        toggle(USERLED);
        delay(100);
    }
    // BT Module BGB203 is connected to UART2
    while (BT.begin(BT_BGB203, UART2, 9600)!=BT_OK)
    {
        CDC.printf("Init failed\r\n");
        delay(1000);
    }
    CDC.printf("Init OK\r\n");

    BT.search(UART2, 5);

    // Device bluetooth address you want to be connected to
    //BT.connect(UART2, "9463D13C680F");
    //BT.connect(UART2, "BDB203005511");
    while (BT.pair(UART2, "B0C559977340")!=BT_OK)
        CDC.printf("Pairing failed\r\n");
    CDC.printf("Pairing OK\r\n");
}

void loop()
{
    // Take a look on your phone if it recognizes your Pinguino Device
    // Now you are in Data Mode
    // Use Serial2 functions to send and/or receive data
    BT.send(UART2, "It works !!!\r\n");
//	Serial2.printf("AT");
//	CDC.printf("received [%s]|r|n", Serial2GetString() );
//	Serial2.printf("It works !!!\r\n");
//	CDC.printf("It works !!!\r\n");
    toggle(USERLED);
    delay(1000);
}
