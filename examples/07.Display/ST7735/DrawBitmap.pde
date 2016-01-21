/**
        Author: 	Régis Blanchot (Mar. 2014)
        Descr:      Use of 2 SPI devices (screen and SD) at the same time
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
        
        ST7735 TFT    PINGUINO
        ---------------------------------------
        LED           VSS (backlight on)
        SCK           SCK2
        SDA           SDO2
        A0 (DC)       can be connected to any digital pin
        RESET         VSS or RST pin (ST7735 use software reset)
        CS TFT        SS2
        GND           GND
        VSS           VSS (+5V or +3.3V)

        ST7735 SD     PINGUINO
        ---------------------------------------
        CS SD         SS1
        MOSI          SDO1
        MISO          SDI1
        SCK           SCK1

        Function  Microchip    Pinguino 47J53
        ---------------------------------------
        SDO1      RC7          Pin 23
        SCK1      RB4          Pin 4
        SDI1      RB5          Pin 5
        SS1       RB6          Pin 6
        ---------------------------------------
        SDO2      RB1          Pin 1
        SCK2      RB2          Pin 2
        SDI2      RB3          Pin 3
        SS2       RB0          Pin 0

        Function  Microchip    Pinguino 32MX2x0
        ---------------------------------------
        SDO1                   Pin 7
        SCK1                   Pin 1
        SDI1                   Pin 8
        SS1
        ---------------------------------------
        SDO2                   Pin 4
        SCK2                   Pin 0
        SDI2                   Pin 2
        SS2
**/

// Load one or more fonts and active them with ST7735.setFont()
#include <fonts/font6x8.h>

//#define SD_DEBUG
#define ST7735PRINTF

#define SPISD  SPI1
// if your board has only 1 SPI module, use SPISW as 2nd one
//#define SPITFT SPISW
#define SPITFT SPI2

SD_FATFS fat;   // File System object
//SD_FILE  f;     // File object
SD_ERROR e;     // Return code

void setup()
{
    // if use of SPISW
    //ST7735.init(SPITFT, 7,8,9); // DC,SDA,SCK
    ST7735.init(SPITFT, 7); // DC
    ST7735.setFont(SPITFT, font6x8);
    ST7735.setBackgroundColor(SPITFT, ST7735_BLACK);
    ST7735.setColor(SPITFT, ST7735_YELLOW);
    //ST7735.setOrientation(SPITFT, 90);
    //ST7735.clearScreen(SPITFT);
  
    e = SD.mount(SPISD, &fat);

    if (e != FR_OK)
    {
        ST7735.printf(SPITFT, "Failed %s\r\n", SD.getError(e)); 
        while(1);
    }
    ST7735.drawBitmap(SPITFT, SPISD, "img/parrot.bmp", 0, 0);
}   

void loop()
{
}
