/*
 * IRtransDemo - demonstrates the IR remote library
 * An IR led must be connected to a valid PWM pin through a 100 Ohm resistor.
 * Send Power Off command when push button is pressed
 * Target : Philips TV
 * Protocol : RC5
 */

#define SEND_PIN 11    // must be a valid PWM pin

#define TV1_POWER_OFF 0b11100000001100

void setup()
{
    pinMode(USERLED, OUTPUT);
    digitalWrite(USERLED, ON);    
    IRremote.enabeIROut(SEND_PIN);     // Defined the Output pin
}

void loop()
{
    digitalWrite(USERLED, ON);    
    delay(50);
    IRremote.sendRC5(TV1_POWER_OFF, 14);
    digitalWrite(USERLED, OFF);    
    delay(1000);
}
