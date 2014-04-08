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

u16 alpha=0;                // rotation angle
u8 x, y;
u8 xo, yo;

void setup()
{
    pinMode(NOKIA_VCC,   OUTPUT);
    pinMode(NOKIA_LIGHT, OUTPUT);
    pinMode(NOKIA_GND,   OUTPUT);
    
    digitalWrite(NOKIA_VCC,   HIGH); // LCD NOKIA_VCC to 3.3V
    digitalWrite(NOKIA_LIGHT, LOW);  // LCD BackNOKIA_LIGHT On
    digitalWrite(NOKIA_GND,   LOW);  // LCD NOKIA_GND to NOKIA_GND

    // Screen init
    // NOKIA_SCE pin is optional, replace by -1 if not necessary and connect pin to the Ground
    PCD8544.init(NOKIA_SCLK, NOKIA_SDIN, NOKIA_DC, NOKIA_SCE, NOKIA_RST);
    PCD8544.setContrast(127); // 0 to 127
    //PCD8544.refresh(); // show Pinguino splashscreen
    //delay(1000);
    
    xo = PCD8544.screen.width  / 2;
    yo = PCD8544.screen.height / 2;
}

void loop()
{
    x = xo + 24.0f * cosr(alpha);
    y = yo + 24.0f * sinr(alpha);
    
    // display
    PCD8544.clearScreen();
    PCD8544.drawLine(xo, yo, x, y);
    PCD8544.refresh();
    
    // increments angle
    alpha = (alpha + 1) % 360;
}
