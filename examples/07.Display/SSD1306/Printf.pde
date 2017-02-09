/**
        Author: 	Régis Blanchot (Mar. 2014)
        Tested on:	Pinguino 47J53A & Pinguino 32MX250
        Output:	Oled 0.96" with SSD1306 Controller

        2 size available : SSD1306_128X64 or SSD1306_128X32
        
        Wiring :
        
        if SSD1306_6800
            if SSD1306_PMP
                OLED CS#     connected to GND
                OLED RES#   connected to any GPIO (D3)
                OLED D/C#   connected to Pinguino PMA1 (D4)
                OLED W/R#  connected to Pinguino PMRD/PMWR (D13)
                OLED E/RD# connected to GND
                OLED D[7:0]  connected to Pinguino PMD[7:0] (D[31:24])
            else
                OLED CS#     connected to GND
                OLED RES#   connected to any GPIO (D0)
                OLED D/C#   connected to any GPIO (D1)
                OLED W/R#  connected to any GPIO (D2)
                OLED E/RD# connected to GND
                OLED D[7:0]  connected to Pinguino D[31:24]
        if SSD1306_8080 
            if SSD1306_PMP
                OLED CS#     connected to GND
                OLED RES#   connected to any GPIO (D3)
                OLED D/C#   connected to Pinguino PMA1 (D4)
                OLED W/R#  connected to Pinguino PMWR (D14)
                OLED E/RD# connected to GND
                OLED D[7:0]  connected to Pinguino PMD[7:0]
            else
                OLED CS#     connected to GND
                OLED RES#   connected to any GPIO (D0)
                OLED D/C#   connected to any GPIO (D1)
                OLED W/R#  connected to any GPIO (D2)
                OLED E/RD# connected to GND
                OLED D[7:0]  connected to Pinguino D[31:24]
        if SSD1306_I2C
        if SSD1306_SPI3
        if SSD1306_SPI4
**/

/**
    Load one or more fonts and active them with SSD1306.setFont()
**/

#include <fonts/font5x7.h>

const u8 intf = SSD1306_I2C1;

u8 i=0;

void setup()
{
    //On Pingino 32MX250 USERLED is on pin 13
    //which is also used by the PMP Data bus 
    pinMode(USERLED, OUTPUT);
    // 0x3C<<1|0x00 = 0x78 (0b0111.1000)
    SSD1306.init(intf, 0x78);
    SSD1306.clearScreen(intf);
    SSD1306.setFont(intf, font5x7);
}

void loop()
{
    SSD1306.printf(intf, "i=%03d\r\n",i++);
    /*
    SSD1306.print(intf, "i=");
    SSD1306.printNumber(intf, i++, DEC);
    SSD1306.print(intf, "\r\n");
    */
    SSD1306.refresh(intf);
    toggle(USERLED);
    delay(100);
}
