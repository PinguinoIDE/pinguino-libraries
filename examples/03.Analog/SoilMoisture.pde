u16 val = 0;  //variable for storing moisture value 
int soilpin = 13;  //variable for the soil moisture sensor pin
int powerpin = 0;  //variable for soil moisture power pin

void setup() 
{
  Serial.begin(9600);
  pinMode(powerpin, OUTPUT);
  digitalWrite(powerpin, LOW); //Set to LOW so no power is flowing through the sensor
}

void loop() 
{
    digitalWrite(powerpin, HIGH);
    delay(10);//wait 100 milliseconds 
    val = analogRead(soilpin);//Read the value form sensor 
    digitalWrite(powerpin, LOW); //turn D0 Off
    if (val <= 390) 
        Serial.println ("Soil Moisture Level = Very Wet!");
    if ((val  >= 391) && (val <= 420))
        Serial.println("Soil Moisture Level = Pretty Wet");
    if ((val >=421) && (val <=460))
        Serial.println("Soil Mositure Level = Fairly Wet");
    if ((val >=461) && (val <= 512))
        Serial.println("Soil Mositure Level = Moist");
    if ((val >=513) && (val <=600))
        Serial.println("Soil Moisture Level = Damp");
    if ((val >=601) && (val<=700)) 
        Serial.println("Soil Moisture Level = Dry");
    if ((val >= 701) && (val <1024))
        Serial.println("Soil Moisture Level = Very DRY!");
    toggle(USERLED);
    delay(2000);
}