/*
  Mega multple serial test
  Receives from the main serial port, sends to the others. 
  This example works only on Pinguino with at least 2 serial ports
 
  The circuit: 
  * Any serial device attached to Serial port 1
  * Serial monitor open on Serial port 0:
 
  created 30 Dec. 2008
  by Tom Igoe
 
  This example code is in the public domain.
 
*/


void setup()
{
    // initialize both serial ports:
    Serial1.begin(9600);
    Serial2.begin(9600);
}

void loop()
{
    u8 inByte;
    
    if (Serial2.available())
    {
        // read a byte from port 2
        inByte = Serial2.read();
        // send it to port 1
        Serial1.write(inByte); 
    }
}
