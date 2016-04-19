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
    // Attach LED #0 to pin 2, 3 and 4
    LedRGB.attach(MYRGBLED, COMMON_CATHODE, 2, 3, 4);

    LedRGB.setRGBColor(MYRGBLED, 0, 0, 255);  // blue
    delay(300);
    LedRGB.setRGBColor(MYRGBLED, 255, 0, 0);  // red
    delay(300);
    LedRGB.setHexColor(MYRGBLED, 0x00FF00);  // green
    delay(300);
}

void loop()
{
    LedRGB.setGradient(MYRGBLED, GREEN, RED, 10);
    LedRGB.setGradient(MYRGBLED, RED, YELLOW, 10);
    LedRGB.setGradient(MYRGBLED, YELLOW, BLUE, 10);
    LedRGB.setGradient(MYRGBLED, BLUE, GREEN, 10);
}
