/*-----------------------------------------------------
Author:      Regis Blanchot - <rblanchot@gmail.com>
Date:        2016-06-17
Description: I2C Scanner
-----------------------------------------------------*/

u8 adr;

void setup()
{
    // initialize the digital pin USERLED as an output.
    pinMode(USERLED, OUTPUT);
    // initialize the Serial port   
    Serial1.begin(9600);
    // initialize the I2C port   
    I2C1.init();
}

void loop()
{
    I2C1.start();
    if (!I2C1.writechar(adr | I2C_WRITE))
    {
        Serial1.print("I2C address is 0x");
        Serial1.printNumber(adr, HEX);
        Serial1.print("\r\n");
    }
    else
    {
        Serial1.print("0x");
        Serial1.printNumber(adr, HEX);
        Serial1.print(" not valid\r\n");
    }
    I2C1.stop();
    
    toggle(USERLED);
    delay(500);
    adr++;
}
