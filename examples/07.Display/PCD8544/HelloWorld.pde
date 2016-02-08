/**
    Author:    Régis Blanchot (Apr. 2014)
    Library :  Thomas Missonier (sourcezax@users.sourceforge.net).
    Tested on: Pinguino 32MX250 and al 8-bit Pinguino
    Output:    Nokia 5110 84x48 LCD Display with PCD8544 Controller
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

/* Pin configuration */
#define SPILCD SPI1 

void setup()
{
    pinMode(USERLED, OUTPUT);

    /// Screen init

    // if SPILCD = SPISW (SPI Software)
    //PCD8544.init(SPILCD, 6, 7, 0, 1, 2); // DC, RST, SDO, SCK and CS pins
    PCD8544.init(SPILCD, 0, 1); // DC and RST pin
 
    PCD8544.setContrast(SPILCD, 40); // Change the contrast (0 to 127)
    //PCD8544.setFont(SPILCD, font6x8);
    PCD8544.setTextColor(SPILCD, BLACK);
    PCD8544.setTextSize(SPILCD, 1);
    PCD8544.refresh(SPILCD);          // show Pinguino splashscreen
    delay(3000);

    PCD8544.clearScreen(SPILCD);      // Clear screen buffer
    PCD8544.setCursor(SPILCD, 1, 2);
    //PCD8544.print(SPILCD, "Hello World!");
    PCD8544.refresh(SPILCD);
}

void loop()
{
    // Show that Pinguino is still alive
    toggle(USERLED);
    delay(1000);
}
