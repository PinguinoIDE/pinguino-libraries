/*
 * Project name:
     FatFs
 * Copyright:
     (c) Jonas Andersson (jonas@haksberg.net) 2009
 * Revision History:
 Thu Nov 19 2013
 	modified by Pinguino Team according to Pinguino Project
 SD Library requested 
   * Description:
     This is similar to serialtffdemo.pde replacing serial prints by CDC prints.
	 Refer to this example to know more.
 * Test configuration:
     MCU:             PIC18F26J50 & 18F47J53
     Dev.Board:       pinguino 18F26J50 & 18F47J53-A or clone
     Oscillator:      HSPLL 8 or 20 MHz  (raised with PLL to 48.000MHz)
 * NOTES:
*/
    /** I/O pin definitions ****************************************/
/*	circuit:
	* SD card attached to SPI bus as follows:
	** MOSI - pin 1 (RB1)
	** MISO - pin 3 (RB3)
	** CLK - pin 2  (RB2)
	** CS - pin 0   (RB0)
*/

#include <fonts/font6x8.h>

#define SD_DEBUG
#define ST7735PRINTNUMBER
#define ST7735PRINTCHAR
#define ST7735PRINTF
#define ST7735PRINT

#define SPISD  SPI1
#define SPITFT SPI2

    SD_DIR   d;      // Directory object
    SD_ERROR rc;     // Return code
    SD_INFO  content;// File information object

void setup()
{
    pinMode(USERLED, OUTPUT);
    ST7735.init(SPITFT, 7); // DC
    ST7735.setFont(SPITFT, font6x8);
    ST7735.setBackgroundColor(SPITFT, ST7735_BLACK);
    ST7735.setColor(SPITFT, ST7735_WHITE);
    ST7735.setOrientation(SPITFT, 90);
    ST7735.clearScreen(SPITFT);
    SD.mount(SPISD);
}

void loop()
{
    ST7735.clearScreen(SPITFT);

    rc = SD.openDir(SPISD, &d, "/");
    if (rc == SD_OK)
    {
        // Read the content of the current directory
        // While there is no error and still files or directories in
        while (SD.readDir(SPISD, &d, &content) == SD_OK &&
               SD.isNotEmpty(&content))
        {
            if (SD.isDir(&content))
                ST7735.printf(SPITFT, "   <dir>  %s\r\n",
                    SD.getName(&content));
            else
                ST7735.printf(SPITFT, "%8lu  %s\r\n",
                    SD.getSize(&content),
                    SD.getName(&content));
        }  
        //SD.unmount(SPISD);
    }
    else
    {
        ST7735.printf(SPITFT, "Failed %s\r\n", SD.getError(rc));
        //while(1);
    }
    toggle(USERLED);
    delay(5000);
}
