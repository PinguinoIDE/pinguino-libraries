
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
        SCK       SCKx
        SDA       SDOx
        A0 (DC)   can be connected to any digital pin
        RESET     VSS
        CS        can be connected to any digital pin
        GND       GND
        VSS       VSS (+5V or +3.3V)

        Function  Microchip    Pinguino 47J53
        ---------------------------------------
        SDO1      RC7          Pin 23
        SCK1      RB4          Pin 4
        SDI1      RB5          Pin 5
        ---------------------------------------
        SDO2      RB1          Pin 1
        SCK2      RB2          Pin 2
        SDI2      RB3          Pin 3
**/

// Load one or more fonts and active them with ST7735.setFont()
#include <fonts/font6x8.h>

#define SPIMODULE SPI2

void setup()
{
    pinMode(USERLED, OUTPUT);
    
    // SDA and SCK pins must be defined by user
    // if module used is SPISW (SPI Software)
    // ST7735.init(SPISW, 6, 5, 7, 1); // CS, DC, SDA, SCK

    ST7735.init(SPIMODULE, 0, 4, 0, 0); // CS and DC
    ST7735.setFont(SPIMODULE, font6x8);
    ST7735.setBackgroundColor(SPIMODULE, ST7735_BLACK);
    ST7735.setColor(SPIMODULE, ST7735_YELLOW);
    ST7735.setOrientation(SPIMODULE, 270);
    ST7735.clearScreen(SPIMODULE);
}   

void loop()
{
    ST7735.println(SPIMODULE, "Hello World!");
    toggle(USERLED);
    delay(1000);
}
