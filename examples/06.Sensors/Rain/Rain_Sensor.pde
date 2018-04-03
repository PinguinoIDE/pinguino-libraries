/*-----------------------------------------------------
Author:  --<Dave Maners dmaners@gmail.com>
Date: 2018-03-07
Description: Rain Detector

-----------------------------------------------------*/

u16 val = 0;  //variable for storing moisture value 
int rainpin = 16;  //variable for the rain sensor pin
int powerpin = 0;  //variable forrain sensor power pin
void setup() 
{
  Serial1.begin(9600);
  pinMode(powerpin, OUTPUT);
  digitalWrite(powerpin, LOW); //Set to LOW so no power is flowing through the sensor
}

void loop() 
{
    digitalWrite(powerpin, HIGH);
    delay(100);//wait 100 milliseconds 
    val = analogRead(rainpin);//Read the value form sensor 
    digitalWrite(powerpin, LOW); //turn D0 Off
    delay(100);
    Serial1.printf("Val=%d\r\n", val);
    if (val <= 1024) 
        Serial1.println ("Rain Level = Raining Hard!");
    if ((val  >= 1025) && (val <= 2000))
        Serial1.println("Rain Level = Raining Fair");
    if ((val >=2001) && (val <=3001))
        Serial1.println("Rain Level = Light Rain");
    if ((val >=3002) && (val <= 3796))
        Serial1.println("Rain Level = Sprinkling");
    if ((val >=3797) && (val <= 4096))
        Serial1.println("Rain Level = NONE");
    
    toggle(USERLED);
    delay(10000);
}