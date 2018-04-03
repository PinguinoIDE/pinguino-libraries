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

/**
    Load one or more fonts and active them with OLED.setFont()
**/

//#include <fonts/font6x8.h>
//#include <fonts/font8x8.h>          // wrong direction
//#include <fonts/font10x14.h>        // ???
//#include <fonts/font12x8.h>         // wrong direction
//#include <fonts/font16x8.h>         // wrong direction
//#include <fonts/font16x16.h>        // ???

//const u8 intf = OLED_I2C1;
const u8 intf = OLED_SPI2;

u16 sa, ma, ha;     // angles
u8 shx, shy;        // seconds hand coordinates
u8 mhx, mhy;        // minutes hand coordinates
u8 hhx, hhy;        // hours hand coordinates
u8 xo, yo;          // center of the screen
u8 lh;              // hands length
u8 old=255;

//u8 Day[7][5]    = {"Sat","Sun","Mon","Tue","Wed","Thu","Fri"};
//u8 Month[13][5] = {"  ","Jan","Feb","Mar","Apr","Mei","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

void setup()
{
    u32 Tm  = 0x00090000;   // 09hr, 00 min, 00 sec
    u32 Dt  = 0x14031406;   // Friday (day 6 of the week), 14 March 2014
    u16 drift = 200;        // add 200 pulse every minute to adjust time

    pinMode(USERLED, OUTPUT);
    OLED.init(intf, 4, 5); // DC, RST
    RTCC.init(Tm, Dt, drift);

    // Screen's center
    xo = OLED.screen.width/2;
    yo = OLED.screen.height/2;
    lh = OLED.screen.height/2 - 2;
}

void loop()
{
    rtccTime cT;
    //rtccDate cD;

    RTCC.getTime(&cT);
    //RTCC.getDate(&cD);

    if (cT.seconds != old)
    {
        // hands angles
        sa = (270 + cT.seconds * 360 / 60) % 360;
        ma = (270 + cT.minutes * 360 / 60) % 360;
        ha = (270 + cT.hours   * 360 / 12) % 360;

        // calculate hands position
        // cos100 and sin100 return integer values in range [-100, 100] 
        shx = xo + lh * cos100(sa) / 100;
        shy = yo + lh * sin100(sa) / 100;
        mhx = xo + lh * cos100(ma) / 100;
        mhy = yo + lh * sin100(ma) / 100;
        hhx = xo + lh * cos100(ha) / 100;
        hhy = yo + lh * sin100(ha) / 100;

        OLED.clearScreen(intf);

        // display time
        //OLED.setCursor(intf, 5, 5);
        //OLED.printf(intf, "%02d:%02d:%02d\r\n", cT.hours, cT.minutes, cT.seconds);

        // display date
        //OLED.setCursor(intf, 0, 3);
        //OLED.printf(intf, "%3s %02d %3s. %04d\r\n", Day[cD.dayofweek], cD.dayofmonth, Month[cD.month], cD.year+2000);

        // draw hands
        OLED.drawCircle(intf, xo, yo, lh);
        // draw hands
        OLED.drawLine(intf, xo, yo, shx, shy);
        OLED.drawLine(intf, xo, yo, mhx, mhy);
        OLED.drawLine(intf, xo, yo, hhx, hhy);

        // update screen
        OLED.refresh(intf);

        // store last second value
        old = cT.seconds;

        toggle(USERLED);
    }
}
