// Use timer 1 as a counter with external input
// Jean-Pierre Mandon 2010
// a 4,7k pull-up resistor is wired on pin 10

// For 8bit PICs.   

unsigned int counts=0;

void setup()
{
    // reset the timer value
    TMR1H = 0;
    TMR1L = 0;
    T1CON = 0b10000111;	// configure timer 1 in 16 bits counter mode
    // Pin 10 and 11 became input
    // Signal need to be applied on Pin 10
    // every rising edge increment the timer 3 register
    Serial.begin(9600); // to debug
}

void loop()
{
    counts = (TMR1H<<8)+TMR1L;
    Serial.print("Counter = ");
    Serial.printNumber(counts, DEC);
    Serial.print("\r\n");
}
