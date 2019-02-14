// Output on Linux : sudo cat /dev/ttyACM0
// Outout on Windows : 
// Note : Olimex PIC32-Pinguino users
// - Serial1 on pins D0=RX/D1=TX
// - Serial2 on UEXT port (pin3=TX, pin4=RX)

u8 i=0;

void setup()
{
    pinMode(USERLED, OUTPUT);
    Serial.begin(9600);
}

void loop()
{
    Serial.printf("i=%03d\r\n", i++);
    //Serial.printx("i=", i++, DEC);
    toggle(USERLED);
    delay(100);
}
