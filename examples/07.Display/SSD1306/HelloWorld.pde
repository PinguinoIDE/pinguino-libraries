/*
        Author: 	Régis Blanchot (Mar. 2014)
        Tested on:	Pinguino 47J53A & Pinguino 32MX250
        Output:	        Oled 0.96" with SSD1306 Controller

        Wiring :
        
        if OUTPUT = SSD1306_PMP6800
                OLED CS#    connected to GND
                OLED RES#   connected to any GPIO (res)
                OLED D/C#   connected to Pinguino PMA[0:15] (dc)
                OLED W/R#   connected to Pinguino PMRD/PMWR
                OLED E/RD#  connected to GND
                OLED D[7:0] connected to Pinguino PMD[7:0]
                SSD1306.init(PMP6800, rst, dc);
                
        if OUTPUT = SSD1306_PMP8080
                OLED CS#    connected to GND
                OLED RES#   connected to any GPIO (res)
                OLED D/C#   connected to Pinguino PMA1
                OLED W/R#   connected to Pinguino PMWR
                OLED E/RD#  connected to GND
                OLED D[7:0] connected to Pinguino PMD[7:0]
                SSD1306.init(PMP6800, rst);
                
        if OUTPUT = SSD1306_PORT6800
                OLED CS#    connected to GND
                OLED RES#   connected to any GPIO
                OLED D/C#   connected to any GPIO
                OLED W/R#   connected to any GPIO
                OLED E/RD#  connected to GND
                OLED D[7:0] connected to Pinguino D[0:7]
                
        if OUTPUT = SSD1306_PORT8080 
                OLED CS#    connected to GND
                OLED RES#   connected to any GPIO (D0)
                OLED D/C#   connected to any GPIO (D1)
                OLED W/R#   connected to any GPIO (D2)
                OLED E/RD#  connected to GND
                OLED D[7:0] connected to Pinguino D[31:24]
                
        if OUTPUT = SSD1306_I2Cx (I2C1, I2C2)
                SSD1306.init(I2C, address);
        
        if OUTPUT = SSD1306_SPISW
                SSD1306.init(SPISW, rst, dc, sda, sck, cs);
        
        if OUTPUT = SSD1306_SPIx (SPI1, SPI2, ...)
                SSD1306.init(SPI1, rst, dc);
        
    ------------------------------------------------------------------*/

/*
    Load one or more fonts and active them with SSD1306.setFont()
*/

#include <fonts/font5x7.h>        // System font
//#include <fonts/Corsiva12.h>      // font definition for 12 points Corsiva font.
//#include <fonts/Arial14.h>        // font definition for 14 points Arial font.
//#include <fonts/ArialBold14.h>    // font definition for 14 points Arial Bold font.
//#include <fonts/VerdanaBold28.h>  // font definition for 28 points Verdana Bold font.

u8 i;
const u8 intf = SSD1306_I2C1;
//const u8 intf = SSD1306_SPI1;

void setup()
{
    //NB : On Pingino 32MX250 USERLED is on pin 13 which is also used by the PMP Data bus
    // so comment the next line if you use 32MX250 and PMP mode
    pinMode(USERLED, OUTPUT);
    
    // if 6800- or 8080-interface and PMP is used
    //SSD1306.init(intf, 1, PMA3); // RST on D1, DC on PMA3 (D2 on a 47J53A)
    
    // if i2c interface is used
    SSD1306.init(intf, 0x78); // i2c address of the display
    
    // if 6800- or 8080-interface (but not PMP) is used
    //void SSD1306.init(u8 intf, u8 rst, u16 dc)
    //SSD1306.init(intf, 8, 9);

    // if SPI Hardware is used (you need to connect pins SDO, SCK and CS if needed)
    //SSD1306.init(intf, 4, 5); // DC, RST

    // if SPI Software is used
    //SSD1306.init(intf, 0,1,2,3); // SDO, SCK, CS, DC and optionnaly RST
    
    //SSD1306.setFont(intf, ArialBold14);
    SSD1306.setFont(intf, font5x7);
    SSD1306.print(intf, "Hello World!\r\n");
    SSD1306.refresh(intf);
    delay(2000);
    SSD1306.clearScreen(intf);
}   

void loop()
{
    toggle(USERLED);
    delay(1000);
}
