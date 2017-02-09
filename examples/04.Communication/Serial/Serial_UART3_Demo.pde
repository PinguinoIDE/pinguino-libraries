/**
 * UART3 Pinguino test
 * For use with Pinguino with at least 3 Serial Port (UART) !
 * Marcus Fazzi (GPLv2) 2011
*/

void setup()
{
    Serial3.begin(9600);
    pinMode(USERLED, OUTPUT);
}

void loop()
{
    char c;
    
    if(Serial3.Available())
    {
        c = Serial3.Read();
    
        Serial3.printChar(c);

        digitalWrite(USERLED, HIGH);
        delay(50);
    }
    else
    {
        digitalWrite(USERLED, LOW);
    }
}
