/*  ----------------------------------------------------- 
    Author:  A. Gentric
    Date: Aug 24 2014
    Description: Example for use of Soft serial
    This replaces or complements your UART and pins RC6-RC7 
    Bauds : 4800, 9600, 19200, 38400 or 57600
    other baudrates 115200, 2400 and 1200 were tried but did not work perfectly 
    Rx & Tx : any digital pin
    -----------------------------------------------------*/

void setup()
{
    SwSerial.begin(9600, 4, 5);         // Rx on pin 4, Tx on pin 5
    SwSerial.print("Hello !\n\r");      // print replaceable by printf
    SwSerial.printf("Enter your name or everything you want : ");
}

void loop()
{
    u8 c = SwSerial.read();
    SwSerial.write(c);
    //write replaceable by printChar or printf :
    SwSerial.printChar(c);
    SwSerial.printf("%c", c);
}
