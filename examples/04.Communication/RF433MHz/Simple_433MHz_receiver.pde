/*  --------------------------------------------------------------------
    RF Blink - Receiver program 
    Author: Régis Blanchot
    Receiver: MX-JS-05V
    Description: A simple example used to test RF transmission/receiver.          
    ------------------------------------------------------------------
    Wiring:
    Receiver    Pinguino
    GND         GND
    DATA0       A0
    DATA1       NC
    VCC         5V
    ------------------------------------------------------------------
    Antenna's length:
    = 1/4 * c/f * vf
    = 1/4 * 299792458 m/s / 433.92E6 Hz * 0.951
    = 0.164 m
    = 16.4 cm
    = 6.45 inches
    Note : c is speed of light, f is frequency and vf is velocity factor 
    ------------------------------------------------------------------*/

#define RXPIN 0                  // RF Receiver pin = Digital pin 0
#define BUFSIZE 10

void setup()
{
    pinMode(USERLED, OUTPUT);
    RF433MHz.init(RXPIN, 1200);
    Serial.begin(9600);
}

void loop()
{
    u8 i, receivedSize = 0;
    RF433MHz.beginReceive();
    if (RF433MHz.receiveComplete()) 
    {
        // Do something with the data in 'buffer' here before you start receiving to the same buffer again
        receivedSize = buffer[0];
        for (i=1; i<receivedSize; i++)
            Serial.write(buffer[i]);

        RF433MHz.beginReceiveBytes(BUFSIZE, buffer);
        toggle(USERLED);
    }
}
