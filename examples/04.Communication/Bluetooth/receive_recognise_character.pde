/*
	Pinguino and MOD-BT example
	Receive and recognise characters
*/


/********** Macros & Constants ******/

//Definitions for PIC32-PINGUINO-MX220
#define RLED 9
#define GLED 13
#define BUTTON1 8

//RED LED is A10
#define RLEDINIT   TRISACLR = 0x400;
#define RLED1      PORTASET = 0x400;
#define RLED0      PORTACLR = 0x400;
#define RLEDSWITCH PORTA   ^= 0x400;
	
//GREEN LED is B15
#define GLEDINIT  	TRISBCLR = 0x8000;
#define GLED1 		PORTBSET = 0x8000;
#define GLED0 		PORTBCLR = 0x8000;
#define GLEDSWITCH  PORTB   ^= 0x8000;
	
	
/********** Main Functions  *********/

void setup()
{
    RLEDINIT;
    GLEDINIT;
    RLED0;
    GLED0;
		
    //CDC.getKey();
    //CDC.println("Press a key to init MOD-BT:");
    //CDC.getKey();

    // Bluetooth module is connected on UART1
    BT.init(UART1, 115200);
		
    //CDC.println("Device is in data mode.");
}
		
		
void loop()
{
    //reads every received character from the BT module
    //Also, the characters '1' and '2' toggle the two LEDS on the board
				
    //waits until a character is received and stores it
    char c = BT.read(UART1);
		
    //Uncomment if you want the board to echo characters on the terminal
    //CDC.printf("%c\r\n", comm);
		
    // A simple switch that performs different commands
    // You can make this into a function for more sophisticated examples
		
    switch(c)
    {
    	case '1': 
              RLEDSWITCH;
	    break;
	case '2':
	    GLEDSWITCH;
	    break;
	//default:
	    //CDC.println("\r\n Unknown command");
    }	
}
		