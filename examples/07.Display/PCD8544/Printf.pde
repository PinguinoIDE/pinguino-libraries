/**
    Author:       Régis Blanchot (Apr. 2014)
    Library :     Thomas Missonier (sourcezax@users.sourceforge.net).
    Tested on:    Pinguino 32MX250 and Pinguino 47J53A
    Output:       Nokia 5110 84x48 LCD Display  with PCD8544 Controller
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

/// Load one or more fonts and active them with PCD8544.setFont()

#include <fonts/font6x8.h>
//#include <fonts/font8x8.h>          // wrong direction
//#include <fonts/font10x14.h>        // ???
//#include <fonts/font12x8.h>         // wrong direction
//#include <fonts/font16x8.h>         // wrong direction
//#include <fonts/font16x16.h>        // ???

// SPI Module
#define SPILCD SPI1 

u8 i=0;

void setup()
{
    pinMode(USERLED,     OUTPUT);

    /// Screen init

    // if SPILCD = SPISW (SPI Software)
    //PCD8544.init(SPILCD, 6, 7, 0, 1, 2); // DC, RST, SDO, SCK and CS pins
    PCD8544.init(SPILCD, 0, 1); // DC and RST pin

    // Change the contrast around to adapt the display for the best viewing!
    PCD8544.setContrast(SPILCD, 40); // 0 to 127
    //PCD8544.setTextColor(SPILCD, BLACK);
    PCD8544.setFont(SPILCD, font6x8);

    // Clear screen buffer
    PCD8544.clearScreen(SPILCD);
}

void loop()
{
    //PCD8544.clearScreen();
    PCD8544.printf(SPILCD, "i=%03d\r\n", i++);
    PCD8544.refresh(SPILCD);
    toggle(USERLED);
    delay(500);
}
