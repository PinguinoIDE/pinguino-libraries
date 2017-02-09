/**
    Author:    Régis Blanchot (Apr. 2014)
    Tested on: Pinguino 32MX250 and Pinguino 47J53
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
#define SPILCD 2 

//#include <fonts/font6x8.h>

u16 a=0;              // rotation angle
u8 x, y;
u8 xo, yo;            // screen center
u8 ra;                // radius

void setup()
{
    /// Screen init

    // if SPILCD = SPISW (SPI Software)
    //PCD8544.init(SPILCD, 6, 7, 0, 1, 2); // DC, RST, SDO, SCK and CS pins
    //PCD8544.init(SPILCD, 0, 1); // DC and RST pin 47J53
    //PCD8544.init(SPILCD, 0, 2); // DC and RST pin 32MX2x0 SPI1
    PCD8544.init(SPILCD, 5, 6); // DC and RST pin 32MX2x0 SPI2
    PCD8544.setContrast(SPILCD, 40); // 0 to 127
    //PCD8544.setFont(SPILCD, font6x8);

    xo = PCD8544[SPILCD].screen.width  / 2;
    yo = PCD8544[SPILCD].screen.height / 2;
    ra = PCD8544[SPILCD].screen.height / 2;
}

void loop()
{
    x = xo + (ra * cos100(a))/100;
    y = yo + (ra * sin100(a))/100;
    
    // display
    PCD8544.clearScreen(SPILCD);
    PCD8544.drawLine(SPILCD, xo, yo, x, y);
    //PCD8544.printNumber(SPILCD, a, DEC);
    //PCD8544.print(SPILCD, "\r\n");
    PCD8544.refresh(SPILCD);
    delay(100);
    // increments angle
    a = (a + 1) % 360;
}
