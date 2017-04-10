// Output on Linux : sudo cat /dev/ttyACM0
// Outout on Windows : 

u8 i=0;

void setup()
{
    pinMode(USERLED, OUTPUT);
}

void loop()
{
    CDC.printf("i=%03d \r\n", i++);
    /*
    CDC.print("i=");
    CDC.printNumber(i++, DEC);
    CDC.print("\r\n");
    */
    toggle(USERLED);
    delay(1000);
}
