/**
    Author:    Régis Blanchot (Apr. 2014)
    Library :  Thomas Missonier (sourcezax@users.sourceforge.net).
    Tested on: Pinguino 32MX250 and all 8-bit Pinguino
    Output:    Nokia 5110 84x48 LCD Display with PCD8544 Controller
    Wiring:    see also http://wiki.pinguino.cc/index.php/SPI_pins

                                           47J53 45k50
    1 NOKIA_RST    // any pin              4     2
    2 NOKIA_SCE    // Pinguino CS or SS    0     13
    3 NOKIA_DC     // any pin              5     3
    4 NOKIA_SDIN   // Pinguino SDO         1     23
    5 NOKIA_SCLK   // Pinguino SCK         2     1
    6 NOKIA_VCC    // 3.3V 
    7 NOKIA_LIGHT  // GND or 3.3V depends on models                                      
    8 NOKIA_GND    // GND 
**/

/// Load one or more fonts and active them with PCD8544.setFont()
#include <fonts/font6x8.h>

/// SPI Module
//#define SPIMODULE SPISW
#define SPIMODULE SPI1
//#define SPIMODULE SPI2 // PIC32 and PIC18FxxJ5x only

u8 i=0;

void setup()
{
    pinMode(USERLED, OUTPUT);

    /// Screen init

    // if SPIMODULE = SPISW (SPI Software)
    //PCD8544.init(SPIMODULE, 3, 2, 23, 1, 13); // DC, RST, SDO, SCK and CS pins
    PCD8544.init(SPIMODULE, 3, 2); // DC and RST pin 45K50
    //PCD8544.init(SPIMODULE, 4, 5); // DC and RST pin 47J53
    //PCD8544.init(SPIMODULE, 0, 2); // DC and RST pin 32MX2x0 SPI1
    //PCD8544.init(SPIMODULE, 5, 6); // DC and RST pin 32MX2x0 SPI2
 
    PCD8544.setFont(SPIMODULE, font6x8);
    PCD8544.setContrast(SPIMODULE, 40); // Change the contrast (0 to 127, default is 40)
    //PCD8544.invertDisplay(SPIMODULE);
    PCD8544.clearScreen(SPIMODULE);  // Clear screen buffer
    //PCD8544.home(SPIMODULE);
    //PCD8544.setCursor(SPIMODULE, 0, 2);
    PCD8544.print(SPIMODULE, "Hello World");
    PCD8544.refresh(SPIMODULE);
    delay(2500);
    PCD8544.clearScreen(SPIMODULE);  // Clear screen buffer
}

void loop()
{
    PCD8544.printf(SPIMODULE, "i=%d\r\n", i++);
    //
    //PCD8544.print(SPIMODULE, "i=");
    //PCD8544.printNumber(SPIMODULE, i++, DEC);
    //PCD8544.print(SPIMODULE, "\r\n");
    //
    PCD8544.refresh(SPIMODULE);
    // Show that Pinguino is still alive
    toggle(USERLED);
    delay(500);
}
