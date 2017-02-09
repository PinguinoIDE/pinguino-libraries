/**
    Author:    Régis Blanchot (Apr. 2014)
    Library :  Thomas Missonier (sourcezax@users.sourceforge.net).
    Tested on: Pinguino 32MX250 and all 8-bit Pinguino
    Output:    Nokia 5110 84x48 LCD Display with PCD8544 Controller
    Wiring:    see also http://wiki.pinguino.cc/index.php/SPI_pins

    1 NOKIA_RST    // any pin              4
    2 NOKIA_SCE    // Pinguino CS or SS    0
    3 NOKIA_DC     // any pin              3
    4 NOKIA_SDIN   // Pinguino SDO         1
    5 NOKIA_SCLK   // Pinguino SCK         2
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
#define SPILCD 2

void setup()
{
    pinMode(USERLED, OUTPUT);

    /// Screen init

    // if SPILCD = SPISW (SPI Software)
    //PCD8544.init(SPILCD, 6, 7, 0, 1, 2); // DC, RST, SDO, SCK and CS pins
    //PCD8544.init(SPILCD, 0, 1); // DC and RST pin 47J53
    //PCD8544.init(SPILCD, 0, 2); // DC and RST pin 32MX2x0 SPI1
    PCD8544.init(SPILCD, 5, 6); // DC and RST pin 32MX2x0 SPI2
 
    PCD8544.setContrast(SPILCD, 40); // Change the contrast (0 to 127)
    PCD8544.setFont(SPILCD, font6x8);
    PCD8544.setTextColor(SPILCD, BLACK);
    PCD8544.setTextSize(SPILCD, 1);
    PCD8544.refresh(SPILCD);          // show Pinguino splashscreen
    delay(3000);

    PCD8544.clearScreen(SPILCD);      // Clear screen buffer
    PCD8544.setCursor(SPILCD, 1, 2);
    PCD8544.print(SPILCD, "Hello World!");
    PCD8544.refresh(SPILCD);
}

void loop()
{
    // Show that Pinguino is still alive
    toggle(USERLED);
    delay(1000);
}
