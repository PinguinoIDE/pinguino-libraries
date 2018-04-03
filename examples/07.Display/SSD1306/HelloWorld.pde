/*
        Author: 	Régis Blanchot (Mar. 2014)
        Tested on:	Pinguino 47J53A & Pinguino 32MX250
        Output:	Oled 0.96" with SSD1306 Controller

        Wiring :
        
        if OUTPUT = OLED_PMP6800
                OLED CS#    connected to GND
                OLED RES#   connected to any GPIO (res)
                OLED D/C#   connected to Pinguino PMA[0:15] (dc)
                OLED W/R#   connected to Pinguino PMRD/PMWR
                OLED E/RD#  connected to GND
                OLED D[7:0] connected to Pinguino PMD[7:0]
                OLED.init(PMP6800, rst, dc);
                
        if OUTPUT = OLED_PMP8080
                OLED CS#    connected to GND
                OLED RES#   connected to any GPIO (res)
                OLED D/C#   connected to Pinguino PMA1
                OLED W/R#   connected to Pinguino PMWR
                OLED E/RD#  connected to GND
                OLED D[7:0] connected to Pinguino PMD[7:0]
                OLED.init(PMP6800, rst);
                
        if OUTPUT = OLED_PORT6800
                OLED CS#    connected to GND
                OLED RES#   connected to any GPIO
                OLED D/C#   connected to any GPIO
                OLED W/R#   connected to any GPIO
                OLED E/RD#  connected to GND
                OLED D[7:0] connected to Pinguino D[0:7]
                
        if OUTPUT = OLED_PORT8080 
                OLED CS#    connected to GND
                OLED RES#   connected to any GPIO (D0)
                OLED D/C#   connected to any GPIO (D1)
                OLED W/R#   connected to any GPIO (D2)
                OLED E/RD#  connected to GND
                OLED D[7:0] connected to Pinguino D[31:24]
                
        if OUTPUT = OLED_I2Cx (I2C1, I2C2)
                OLED.init(I2Cx, address);
        
        if OUTPUT = OLED_SPISW
                OLED.init(SPISW, rst, dc, sda, sck, cs);
        
        if OUTPUT = OLED_SPIx (SPI1, SPI2, ...)
                OLED.init(SPIx, rst, dc);
        
    ------------------------------------------------------------------*/

/*DISPLAY CONTROLLER*******************************************/
#define OLED_SH1106
//#define OLED_SSD1306
/*DISPLAY SIZE*************************************************/
//#define OLED_128X32
#define OLED_128X64
/**************************************************************/

/*
    Load one or more fonts and active them with OLED.setFont()
*/

#include <fonts/font5x7.h>        // System font
//#include <fonts/Corsiva12.h>      // font definition for 12 points Corsiva font.
//#include <fonts/Arial14.h>        // font definition for 14 points Arial font.
//#include <fonts/ArialBold14.h>    // font definition for 14 points Arial Bold font.
//#include <fonts/VerdanaBold28.h>  // font definition for 28 points Verdana Bold font.

u8 i;
//const u8 intf = OLED_I2C1;
const u8 intf = OLED_SPI2;

void setup()
{
    //NB : On Pingino 32MX250 USERLED is on pin 13 which is also used by the PMP Data bus
    // so comment the next line if you use 32MX250 and PMP mode
    pinMode(USERLED, OUTPUT);
    
    // if 6800- or 8080-interface and PMP is used
    //OLED.init(intf, 1, PMA3); // RST on D1, DC on PMA3 (D2 on a 47J53A)
    
    // if i2c interface is used
    //OLED.init(intf, 0x78); // i2c address of the display
    
    // if 6800- or 8080-interface (but not PMP) is used
    //void OLED.init(u8 intf, u8 rst, u16 dc)
    //OLED.init(intf, 8, 9);

    // if SPI Hardware is used (you need to connect pins SDO, SCK and CS if needed)
    OLED.init(intf, 4, 5); // DC, RST

    // if SPI Software is used
    //OLED.init(intf, 0,1,2,3); // SDO, SCK, CS, DC and optionnaly RST
    
    //OLED.setFont(intf, ArialBold14);
    OLED.setFont(intf, font5x7);

    OLED.clearScreen(intf);
}   

void loop()
{
    OLED.print(intf, "Hello World!\r\n");
    OLED.refresh(intf);
    toggle(USERLED);
    delay(1000);
}
