// RGB LED Driver Demo
// 2016 - Régis Blanchot

#define MYRGBLED 0
#define RED      0xFF0000
#define GREEN    0x00FF00
#define BLUE     0x0000FF
#define YELLOW   0xFF4000

u8 c;

void setup()
{
    // Anything over 120 Hz is probably fast enough for a RGB LED
    // Let's say 200 Hz to be sure
    LedRGB.setFrequency(200);
    // Attach LED #0 to pin 0, 1, 2
    LedRGB.attach(MYRGBLED, COMMON_CATHODE, 0, 1, 2);
    // Some tests
    LedRGB.setRGBColor(MYRGBLED, 0, 0, 255);  // blue
    delay(1000);
    LedRGB.setRGBColor(MYRGBLED, 255, 0, 0);  // red
    delay(1000);
    LedRGB.setHexColor(MYRGBLED, 0x00FF00);  // green
    delay(1000);
    LedRGB.setHexColor(MYRGBLED, YELLOW);  // green
    delay(1000);
}

void loop()
{
    // From Green to Red with a 10ms delay
    LedRGB.setGradient(MYRGBLED, GREEN, RED, 10);
    LedRGB.setGradient(MYRGBLED, RED, YELLOW, 10);
    LedRGB.setGradient(MYRGBLED, YELLOW, BLUE, 10);
    LedRGB.setGradient(MYRGBLED, BLUE, GREEN, 10);
}
