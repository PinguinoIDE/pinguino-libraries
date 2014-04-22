// Simple serial test
// Jean-Pierre Mandon 2010

char caractere;
char chaine[5]={'T','E','S','T',0};

void setup()
{
    Serial.begin(9600);
}

void loop()
{
    Serial.print("TEST\n\r");
    if (Serial.available())
    {
        Serial.print("caractere \r\n");
        caractere=Serial.read();
        Serial.print(chaine);
        Serial.printNumber(caractere, DEC);
        delay(1000);
    }
    delay(100);
}