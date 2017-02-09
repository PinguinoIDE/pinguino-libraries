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
        A0 (DC)   can be connected to any digital pin
        RESET     VSS
        CS        can be connected to any digital pin
        GND       GND
        VSS       VSS (+5V or +3.3V)
**/

#define SPIMODULE SPI2

void setup()
{
    // init. the random number generator
    int seed;
    seed = millis();
    randomSeed(seed);
    
    ST7565.init(SPIMODULE, 7); // DC
    ST7565.clearScreen(SPIMODULE);
}   

void loop()
{
    u16 x1 = random(0, 159);     // coordinate x E [0,159]
    u16 y1 = random(0, 127);     // coordinate y E [0, 127]
    u16 x2 = random(0, 159);     // coordinate x E [0,159]
    u16 y2 = random(0, 127);     // coordinate y E [0, 127]

    ST7565.drawLine(SPIMODULE, x1, y1, x2, y2);
    
    // delay
    delay(150);
}
