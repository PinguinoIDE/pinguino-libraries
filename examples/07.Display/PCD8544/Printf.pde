/**
        Author: 	    Régis Blanchot (Apr. 2014)
        Library :     Thomas Missonier (sourcezax@users.sourceforge.net).
        Tested on:	Pinguino 32MX250 and Pinguino 47J53A
        Output:	    Nokia 5110 84x48 LCD Display  with PCD8544 Controller
**/

/* Pin configuration */

#define NOKIA_RST     0  // LCD RST 
#define NOKIA_SCE     1  // LCD CS/CE  
#define NOKIA_DC      2  // LCD Dat/Com
#define NOKIA_SDIN    3  // LCD SPIDat/DIN/NOKIA_SDIN
#define NOKIA_SCLK    4  // LCD SPIClk/CLK 
#define NOKIA_VCC     5  // LCD NOKIA_VCC 3.3V 
#define NOKIA_LIGHT   6  // LCD BACKNOKIA_LIGHT : GROUND or NOKIA_VCC 3.3V depends on models                                      
#define NOKIA_GND     7  // LCD GROUND 

/**
    Load one or more fonts and active them with SSD1306.setFont()
**/

#include <fonts/font6x8.h>
//#include <fonts/font8x8.h>          // wrong direction
//#include <fonts/font10x14.h>        // ???
//#include <fonts/font12x8.h>         // wrong direction
//#include <fonts/font16x8.h>         // wrong direction
//#include <fonts/font16x16.h>        // ???

u8 i=0;

void setup()
{
    pinMode(USERLED,     OUTPUT);
    pinMode(NOKIA_VCC,   OUTPUT);
    pinMode(NOKIA_LIGHT, OUTPUT);
    pinMode(NOKIA_GND,   OUTPUT);
    
    digitalWrite(NOKIA_VCC,   HIGH); // LCD NOKIA_VCC to 3.3V
    digitalWrite(NOKIA_LIGHT, LOW); // LCD BackNOKIA_LIGHT On
    digitalWrite(NOKIA_GND,   LOW); // LCD NOKIA_GND to NOKIA_GND

    // Screen init
    // NOKIA_SCE pin is optional, replace by -1 if not necessary and connect pin to the Ground
    PCD8544.init(NOKIA_SCLK, NOKIA_SDIN, NOKIA_DC, NOKIA_SCE, NOKIA_RST);

    // Change the contrast around to adapt the display for the best viewing!
    PCD8544.setContrast(127); // 0 to 127
    PCD8544.setTextColor(BLACK);
    PCD8544.setFont(font6x8);

    // Clear screen buffer
    PCD8544.clearScreen();
}

void loop()
{
    //PCD8544.clearScreen();
    PCD8544.printf("i=%03d\r\n", i++);
    PCD8544.refresh();
    toggle(USERLED);
    delay(500);
}
