/*  --------------------------------------------------------------------
    RF Blink - Transmit program
    Author: Régis Blanchot
    Transmitter: FS1000A/MX-FS-03V
    Description: A simple example used to test RF transmission.
    ------------------------------------------------------------------
    Wiring :
    Transmitter    Pinguino
    ATAD           D4
    VCC            3V
    GND            GND
    ------------------------------------------------------------------
    Antenna's length:
    = 1/4 * c/f * vf
    = 1/4 * 299792458 m/s / 433.92E6 Hz * 0.951
    = 0.164 m
    = 16.4 cm
    = 6.45 inches
    Note : c is speed of light, f is frequency and vf is velocity factor 
    ------------------------------------------------------------------*/

#define TXPIN 0  //RF Transmitter pin = digital pin 0

u8 string[6] = {'T', 'E', 'S', 'T', '\r', '\n', };

void setup()
{
    pinMode(USERLED, OUTPUT);    
    //pinMode(TXPIN, OUTPUT);     
    //digitalWrite(TXPIN, LOW);     // Transmit a HIGH signal
    //digitalWrite(USERLED, LOW);           // Turn the LED on
    RF433MHz.init(TXPIN, 1200);
}

void loop()
{
    RF433MHz.print("PINGUINO\r\n");
    RF433MHz.write(string, 6);
    toggle(USERLED);
    delay(1000);                           // Variable delay
    /*
    digitalWrite(TXPIN, HIGH);     // Transmit a HIGH signal
    digitalWrite(USERLED, HIGH);           // Turn the LED on
    delay(1000);                           // Wait for 1 second

    digitalWrite(TXPIN,LOW);       // Transmit a LOW signal
    digitalWrite(USERLED, LOW);            // Turn the LED off
    delay(1000);                           // Variable delay
    */
}
