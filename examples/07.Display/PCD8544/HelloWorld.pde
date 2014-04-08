/**
        Author: 	    Régis Blanchot (Apr. 2014)
        Library :     Thomas Missonier (sourcezax@users.sourceforge.net).
        Tested on:	Pinguino 32MX250 and Pinguino 47J53A
        Output:	    Nokia 5110 84x48 LCD Display  with PCD8544 Controller
**/


/// Load one or more fonts and active them with PCD8544.setFont()

#include <fonts/font6x8.h>
//#include <fonts/font8x8.h>          // wrong direction
//#include <fonts/font10x14.h>        // ???
//#include <fonts/font12x8.h>         // wrong direction
//#include <fonts/font16x8.h>         // wrong direction
//#include <fonts/font16x16.h>        // ???

/* Pin configuration */
//#define PCD8544_INTERFACE     (PCD8544_SPIHW)
#define PCD8544_INTERFACE       (PCD8544_SPISW | PCD8544_PORTB)
/*
define PCD8544_INTERFACE       (PCD8544_SPISW)
define NOKIA_RST               0  // LCD RST 
define NOKIA_SCE               1  // LCD CS/CE  
define NOKIA_DC                2  // LCD Dat/Com
define NOKIA_SDIN              3  // LCD SPIDat/DIN/NOKIA_SDIN
define NOKIA_SCLK              4  // LCD SPIClk/CLK 
define NOKIA_VCC               5  // LCD NOKIA_VCC 3.3V 
define NOKIA_LIGHT             6  // LCD BACKNOKIA_LIGHT : GROUND or NOKIA_VCC 3.3V depends on models                                      
define NOKIA_GND               7  // LCD GROUND 
*/

void setup()
{
    pinMode(USERLED,     OUTPUT);

/*
    // only if PCD8544_INTERFACE == PCD8544_SPISW
    pinMode(NOKIA_VCC,   OUTPUT);
    pinMode(NOKIA_LIGHT, OUTPUT);
    pinMode(NOKIA_GND,   OUTPUT);
    
    digitalWrite(NOKIA_VCC,   HIGH); // LCD NOKIA_VCC to 3.3V
    digitalWrite(NOKIA_LIGHT, LOW);  // LCD BackNOKIA_LIGHT On
    digitalWrite(NOKIA_GND,   LOW);  // LCD NOKIA_GND to NOKIA_GND
*/

    // Screen init
/*
    // only if PCD8544_INTERFACE == PCD8544_SPISW
    // NOKIA_SCE pin is optional, replace by -1 if not necessary and connect pin to the Ground
    PCD8544.init(NOKIA_SCLK, NOKIA_SDIN, NOKIA_DC, NOKIA_SCE, NOKIA_RST);
*/
    PCD8544.init();
    //PCD8544.setContrast(127); // Change the contrast (0 to 127)
    PCD8544.refresh();          // show Pinguino splashscreen
    delay(3000);

    PCD8544.clearScreen();      // Clear screen buffer
    PCD8544.setFont(font6x8);
    //PCD8544.setTextSize(1);
    //PCD8544.setTextColor(BLACK);
    PCD8544.setCursor(1,2);
    PCD8544.print("Hello World!");
    PCD8544.refresh();
}

void loop()
{
    // Show that Pinguino is still alive
    toggle(USERLED);
    delay(1000);
}
 