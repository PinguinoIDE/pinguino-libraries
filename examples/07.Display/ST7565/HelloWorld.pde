
/**
        Author: 	Régis Blanchot (Mar. 2014)
        Tested on:	Pinguino 47J53 & Pinguino 32MX250
        Output:	128x64 monochrome graphic TFT-LCD with ST7565 controller

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
        
        ST7565    PINGUINO
        ---------------------------------------
        LED       VSS (backlight on)
        SCK       SCK
        SDA       SDO
        A0        DC (any digital pin)
        RESET     VSS
        CS        CS or SS
        GND       GND
        VSS       VSS (+5V or +3.3V)
**/

// Load one or more fonts and active them with ST7565.setFont()
#include <fonts/font6x8.h>
//#include <fonts/Corsiva12.h>      // font definition for 12 points Corsiva font.
//#include <fonts/Arial14.h>        // font definition for 14 points Arial font.
//#include <fonts/ArialBold14.h>    // font definition for 14 points Arial Bold font.
//#include <fonts/VerdanaBold28.h>  // font definition for 28 points Verdana Bold font.

#define SPIMODULE SPI1

u8 i;

void setup()
{
    pinMode(USERLED, OUTPUT);
    
    // SDA and SCK pins must be defined by user
    // if module used is SPISW (SPI Software)
    //ST7565.init(SPIMODULE, 7, 1, 2, 0); // DC, SDA, SCK, CS
    ST7565.init(SPIMODULE, 7); // DC
    ST7565.clearScreen(SPIMODULE);

    ST7565.setFont(SPIMODULE, font6x8);
    ST7565.setCursor(SPIMODULE, 0, 3);
    ST7565.printCenter(SPIMODULE, "Hello World");
}   

void loop()
{
    toggle(USERLED);
    delay(1000);
}
