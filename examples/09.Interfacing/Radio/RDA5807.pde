/*
        Author: 	Régis Blanchot (Mar. 2018)
        Tested on:	Pinguino Torda (47J53B)
        Output:	Oled with SH1106 Controller

        Wiring :
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

/*INTERFACES (***************************************************/
const u8 dintf = OLED_SPI1;	// display
const u8 rintf = I2C2;	// radio

/*
    Load one or more fonts and active them with OLED.setFont()
*/

#include <fonts/font5x7.h>        // System font
//#include <fonts/Corsiva12.h>      // font definition for 12 points Corsiva font.
//#include <fonts/Arial14.h>        // font definition for 14 points Arial font.
//#include <fonts/ArialBold14.h>    // font definition for 14 points Arial Bold font.
//#include <fonts/VerdanaBold28.h>  // font definition for 28 points Verdana Bold font.

void setup()
{
    u8 reg;

    pinMode(USERLED, OUTPUT);

    // Initialize the SH1106 Display with SPI communication
    // (you need to connect pins SDO, SCK and CS)
    OLED.init(dintf, 14, 15); // DC, RST
    OLED.setFont(dintf, font5x7);
    OLED.clearScreen(dintf);
    //OLED.printx(dintf, "id=", RDA5807M.init(rintf), HEX);
    OLED.refresh(dintf);

    // Initialize the Radio chip
    if (!RDA5807M.init(rintf))
    {
        OLED.println(dintf, "No chip found");
        OLED.refresh(dintf);
        while(1)
        {
            digitalWrite(USERLED, HIGH);
            delay(100);
            digitalWrite(USERLED, LOW);
            delay(900);
        }
    }

    // set the sound properties
    RDA5807M.setVolume(rintf, 15); // max. is 15
    RDA5807M.setMono(rintf, false);
    RDA5807M.setMute(rintf, false);
    RDA5807M.setSoftMute(rintf, false); // mute on low signal
    RDA5807M.setBassBoost(rintf, true);

    // set the band that will be tuned
    RDA5807M.setBand(rintf, RADIO_BAND_FM);

    // set the frequency step when seeking a station
    RDA5807M.setSpacing(rintf, 100); // 25, 50, 100 or 200 kHz

    // set the frequency that will be tuned
    RDA5807M.setFrequency(rintf, 10630); //  106.30 MHz
    RDA5807M.seekUp(rintf, true);
    //RDA5807M.seekDown(rintf, true);

    // Debug
    for (reg=0x00; reg<0x08; reg++)
    {
        OLED.printf(dintf, "R%02X=0x%04X\r\n", reg, RDA5807M.readRegister(rintf, reg));
        OLED.refresh(dintf);
        delay(1500);
    }
}

void loop()
{
    u8 band;

    OLED.clearScreen(dintf);

    // Name of the radio
    //OLED.println(dintf, (RDA5807M.getRDS(rintf)) ? RDA5807M.getRadioName(rintf):"NO RDS"); 
    
    //OLED.println(dintf, (RDA5807M.getActive(rintf)) ? "ON":"OFF");
    //OLED.println(dintf, (RDA5807M.getTuned(rintf))  ? "TU":"NTU");

    // Band Freq.
    band = RDA5807M.getBand(rintf);
    if (band == RADIO_BAND_FM || band == RADIO_BAND_FMWORLD)
        OLED.print(dintf, "FM ");

    // Stereo/Mono
    OLED.print(dintf, (RDA5807M.getStereo(rintf)) ? "Stereo ":"Mono ");

    // Frequency
    OLED.println(dintf, RDA5807M.formatedFrequency(rintf));

    // Volume
    OLED.print(dintf, "Vol:");
    OLED.printNumber(dintf, 100*RDA5807M.getVolume(rintf)/15, DEC);
    //OLED.printx(dintf, "Vol:", RDA5807M.getVolume(rintf), DEC);
    OLED.print(dintf, "%\r\n");

    // Signal strength
    OLED.print(dintf, "Sig:"); 
    OLED.printNumber(dintf, 100*RDA5807M.getStrength(rintf)/63, DEC);
    OLED.print(dintf, "%\r\n");

    // RDS Time
    //OLED.print(dintf, (RDA5807M.getRDS(rintf)) ? RDA5807M.getRadioTime(rintf):"--:--:--"); 

    // Show the current chip data every 1 second.
    OLED.refresh(dintf);

    toggle(USERLED);
    delay(1000);
}
