/**
        Author: 	Régis Blanchot (Mar. 2014)
        Tested on:	Pinguino 47J53 & Pinguino 32MX250
        Output:	262K-color graphic TFT-LCD with ST7735 controller

        2 modes available :
        - Hardware SPI
            . default mode
            . SPI operations are handled by the CPU
            . pins have to be the CPU SPI pins
            . PINGUINO 32 have up to 4 SPI module (SPI1 to SPI4)
            . PINGUINO 8  have only one SPI module (SPI1)
        - Software SPI
            . SPISW
            . SPI operations are handled by the SPI library
            . pins can be any digital pin
        
        Wiring :
        
        ST7735    PINGUINO
        ---------------------------------------
        LED       VSS (backlight on)
        SCK       SCK
        SDA       SDO
        A0 (DC)   can be connected to any digital pin
        RESET     VSS
        CS        CS or SS
        GND       GND
        VSS       VSS (+5V or +3.3V)
**/

// Load one or more fonts and active them with ST7735.setFont()
//#include <fonts/font6x8.h>
//#include <fonts/Corsiva12.h>      // font definition for 12 points Corsiva font.
//#include <fonts/Arial14.h>        // font definition for 14 points Arial font.
//#include <fonts/ArialBold14.h>    // font definition for 14 points Arial Bold font.
#include <fonts/VerdanaBold28.h>  // font definition for 28 points Verdana Bold font.

#define SPITFT SPI2

u8 i;

void setup()
{
    pinMode(USERLED, OUTPUT);
    ST7735.init(SPITFT, 7);
    ST7735.setFont(SPITFT, VerdanaBold28);
    ST7735.setBackgroundColor(SPITFT, ST7735_BLACK);
    ST7735.setColor(SPITFT, ST7735_GREEN);
    ST7735.setOrientation(SPITFT, 90);
    ST7735.clearScreen(SPITFT);
}

void loop()
{
    ST7735.setCursor(SPITFT, 4, 2);
    ST7735.printf(SPITFT, "i=%03u", i++);
    toggle(USERLED);
    //delay(1000);
}
