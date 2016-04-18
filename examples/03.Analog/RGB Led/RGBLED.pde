// RGB LED Driver Demo
// 2016 - Régis Blanchot

#define MYRGBLED 0
#define RED      0xFF0000
#define GREEN    0x00FF00
#define BLUE     0x0000FF
#define YELLOW   0xFFFF00

u8 c;

void setup()
{
    // Anything over 120 Hz is probably fast enough for a RGB LED
    // Let's say 200 Hz to be sure
    LedRGB.setFrequency(200);
    // Attach LED #0 to pin 2, 3 and 4
    LedRGB.attach(MYRGBLED, 2, 3, 4);
}

void loop()
{
    /*
    LedRGB.setRGBColor(MYRGBLED, 255, 0, 0);  // red
    delay(1000);
    LedRGB.setHexColor(MYRGBLED, 0x00FF00);  // green
    delay(1000);
    LedRGB.setRGBColor(MYRGBLED, 0, 0, 255);  // blue
    delay(1000);
    delay(1000);
    LedRGB.setHexColor(MYRGBLED, 0xFFFF00);  // yellow
    delay(1000);
    LedRGB.setHexColor(MYRGBLED, 0x00FFFF);  // aqua
    delay(1000);
    LedRGB.setRGBColor(MYRGBLED, 80, 0, 80);  // purple
    delay(1000);
    */
    
    LedRGB.setGradient(MYRGBLED, GREEN, RED, 50);
    LedRGB.setGradient(MYRGBLED, RED, YELLOW, 50);
    LedRGB.setGradient(MYRGBLED, YELLOW, BLUE, 50);
    LedRGB.setGradient(MYRGBLED, BLUE, GREEN, 50);
}
