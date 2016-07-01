/**
    Author:    Régis Blanchot (Apr. 2014)
    Library :  Thomas Missonier (sourcezax@users.sourceforge.net).
    Tested on: Pinguino 32MX250 and Pinguino 47J53A
    Output:    Nokia 5110 84x48 LCD Display  with PCD8544 Controller
    Wiring:

    1 NOKIA_RST    // any pin 
    2 NOKIA_SCE    // Pinguino CS or SS
    3 NOKIA_DC     // any pin
    4 NOKIA_SDIN   // Pinguino SDO
    5 NOKIA_SCLK   // Pinguino SCK 
    6 NOKIA_VCC    // 3.3V 
    7 NOKIA_LIGHT  // GND or 3.3V depends on models                                      
    8 NOKIA_GND    // GND 
**/

// SPI Module
#define SPILCD 1 

u16 alpha=0;                // rotation angle
u8 x, y;
u8 xo, yo;

/* Pin configuration */

void setup()
{
    pinMode(USERLED, OUTPUT);

    /// Screen init

    // if SPILCD = SPISW (SPI Software)
    //PCD8544.init(SPILCD, 6, 7, 0, 1, 2); // DC, RST, SDO, SCK and CS pins
    PCD8544.init(SPILCD, 0, 2); // DC and RST pin

    PCD8544.setContrast(SPILCD, 40); // 0 to 127
    
    xo = PCD8544[SPILCD].screen.width  / 2;
    yo = PCD8544[SPILCD].screen.height / 2;
}

void loop()
{
    x = xo + (24.0 * cosr(alpha));
    y = yo + (24.0 * sinr(alpha));
    
    // display
    PCD8544.clearScreen(SPILCD);
    PCD8544.drawLine(SPILCD, xo, yo, x, y);
    PCD8544.refresh(SPILCD);
    
    // increments angle
    alpha = (alpha + 1) % 360;
}
