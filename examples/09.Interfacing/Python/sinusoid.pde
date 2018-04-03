/*
    Pinguino example to send bytes on USB bus
    Results are displayed with sinusoid.py

    03/01/2018  - Régis Blanchot - First release
*/

u16 gAngle;

struct point
{
    s16 x;
    s16 y;
};

void setup()
{
    pinMode(USERLED, OUTPUT);    
}

void loop()
{
    struct point p;
    // Sinusoid calculation
    // We use Pinguino's Trigonometric Integers functions (trigo.c)
    // Returned values are in range [-100, 100] corresponding to [-1.0, 1.0]
    p.x = gAngle;
    p.y = sin100(gAngle);
    
    // Increment the angle and keep it in [0, 360[
    gAngle = (gAngle + 1) % 360;

    // Send p (4 bytes) on the USB bus
    USB.send((u8*)&p, 4);

    toggle(USERLED);
    delay(100);
}
