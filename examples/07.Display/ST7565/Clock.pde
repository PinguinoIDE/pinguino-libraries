/**
        Author: 	Régis Blanchot (Mar. 2014)
        Tested on:	Pinguino 47J53 & Pinguino 32MX250
        Output:	128x64 monochrome graphic TFT-LCD with ST7565 controller

        2 modes available :
        - Hardware SPI
            . default mode
            . SPI operations are handled by the CPU
            . pins have to be the CPU SPI pins
        - Software SPI
            . activated with #define SPISW
            . SPI operations are handled by the ST7565 library
            . pins can be any digital pin
        
        Wiring :
        
        ST7565    PINGUINO
        ---------------------------------------
        LED       VSS (backlight on)
        SCK       SCK
        SDA       SDO
        A0 (DC)   can be connected to any digital pin
        RESET     VSS
        CS        can be connected to any digital pin
        GND       GND
        VSS       VSS (+5V or +3.3V)
**/

#define SPITFT    SPI2

/**
    Load one or more fonts and active them with ST7565.setFont()
**/

#include <fonts/font6x8.h>

u8 xo, yo;
u8 old=0;

u8 Day[7][5]    = {"Sat","Sun","Mon","Tue","Wed","Thu","Fri"};
u8 Month[13][5] = {"  ","Jan","Feb","Mar","Apr","Mei","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

void setup()
{
    u32 Tm  = 0x00090000;   // 09hr, 00 min, 00 sec
    u32 Dt  = 0x14031406;   // Friday (day 6 of the week), 14 March 2014
    u16 drift = 0;        // add 200 pulse every minute to adjust time

    pinMode(USERLED, OUTPUT);
    
    // if SPISW is defined
    // ST7565_init(SPISW, sda, sck, cs, dc);

    ST7565.init(SPITFT, 7); // DC
    ST7565.setFont(SPITFT, font6x8);
    ST7565.clearScreen(SPITFT);

    // Init real time calendar
    RTCC.init(Tm, Dt, drift);

    xo = ST7565.screen.width  / 2;
    yo = ST7565.screen.height / 2;
}

void loop()
{
    u8 x, y;
    s16 angle;
    rtccTime cT;
    rtccDate cD;

    // Get Time and Date
    RTCC.getTime(&cT);
    RTCC.getDate(&cD);
    
    if (cT.seconds != old)
    {
        ST7565.clearScreen(SPITFT);
        
        /** DATE **/
        ST7565.setCursor(SPITFT, 3, 0);
        ST7565.printf(SPITFT, "%3s %02d %3s. %04d\r\n", Day[cD.dayofweek], cD.dayofmonth, Month[cD.month], cD.year+2000);

        /** TIME **/
        ST7565.setCursor(SPITFT, 6, 1);
        ST7565.printf(SPITFT, "%02d:%02d:%02d\r\n", cT.hours, cT.minutes, cT.seconds);

        /* */
        ST7565.drawCircle(SPITFT, xo, yo, 33);
        
        /** HOURS **/
        angle = cT.hours * 30 - 90;
        x = xo + 32.0f * cosr(angle);
        y = yo + 32.0f * sinr(angle);
        ST7565.drawLine(SPITFT, xo, yo, x, y);
        
        /** MINUTES **/
        angle = cT.minutes * 6 - 90;
        x = xo + 32.0f * cosr(angle);
        y = yo + 32.0f * sinr(angle);
        ST7565.drawLine(SPITFT, xo, yo, x, y);

        /** SECONDS **/
        angle = cT.seconds * 6 - 90;
        x = xo + 32.0f * cosr(angle);
        y = yo + 32.0f * sinr(angle);
        ST7565.drawLine(SPITFT, xo, yo, x, y);

        toggle(USERLED);
  
        // store last second value
        old = cT.seconds;
    } 
}
