/**
        Author: 	Régis Blanchot (Mar. 2014)
        Tested on:	Pinguino 47J53A & Pinguino 32MX250
        Output:	Oled 0.96" with SSD1306 Controller

        2 size available : OLED_128X64 or OLED_128X32
        
        Wiring :
        
        if OLED_6800
            if OLED_PMP
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
        if OLED_8080 
            if OLED_PMP
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
        if OLED_I2C
        if OLED_SPI3
        if OLED_SPI4
**/

/*DISPLAY CONTROLLER*******************************************/
#define OLED_SH1106
//#define OLED_SSD1306
/*DISPLAY SIZE*************************************************/
//#define OLED_128X32
#define OLED_128X64
/**************************************************************/

u16 alpha=0;                // rotation angle
u16 x, y;
u16 xo, yo;

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
    
    OLED.clearScreen(intf);
    
    xo = OLED.screen.width  / 2;
    yo = OLED.screen.height / 2;
}

void loop()
{
    x = xo + 32.0f * cosr(alpha);
    y = yo + 32.0f * sinr(alpha);
    
    // display
    OLED.clearScreen(intf);
    OLED.drawLine(intf, xo, yo, x, y);
    OLED.refresh(intf);
    
    // increments angle
    alpha = (alpha + 1) % 360;
}
