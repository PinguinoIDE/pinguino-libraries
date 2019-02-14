/*
 * Ultrasonic Sensor HC-SR04
 *
 * Crated by Dejan Nedelkovski,
 * www.HowToMechatronics.com
 *
 */

// defines pins numbers ( it can be any digital pin)
const int trigPin = 0;
//const int echoPin = CCP4;

void setup()
{
    pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
    //pinMode(echoPin, INPUT); // Sets the echoPin as an Input
    Serial.begin(9600); // Starts the serial communication
}

void loop()
{
    // defines variables
    u16 duration;
    u16 distance;

    // Clears the trigPin
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Reads the echoPin
    // Returns the sound wave travel time in microseconds
    // timeout = 10000
    duration = pulseIn(CCP4, HIGH, 10000);

    if (duration)
    {
        // Calculating the distance
        // Speed of sound = 340 m/s = 340*100/1000000 cm/us = 34/1000
        // Distance = Time * Speed
        // The sound wave needs to travel forward and bounce backward
        // so Distance is divided by 2
        distance = duration * 34 / 1000 / 2;

        // Prints the distance on the Serial Monitor
        Serial.print("Distance: ");
        Serial.printNumber(distance, DEC);
        Serial.println(" cm");
    }
}
