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

//#define SD_DEBUG
//#define ST7735PRINTF

#define SPISD  SPI1
#define SPITFT SPI2

void setup()
{
    ST7735.init(SPITFT, 7); // DC
    ST7735.setFont(SPITFT, font6x8);
    ST7735.setBackgroundColor(SPITFT, ST7735_BLACK);
    ST7735.setColor(SPITFT, ST7735_YELLOW);
    ST7735.setOrientation(SPITFT, 90);
    SD.mount(SPISD);
}

void loop()
{
    SD_FILE  f;      // Directory object
    SD_ERROR rc;     // Return code
    SD_INFO  content;// File information object
    
    u8 Buff[65];     // File read buffer
    u16 nbr;         // Number of bytes returned
    u8 sizebuffer = sizeof(Buff) - 1;

    ST7735.clearScreen(SPITFT);

    rc = SD.open(SPISD, &f, "lorem.txt", SD_READ); // open file
    if (rc == SD_OK)
    {
        // read file and print it until it ends
        ST7735.printf(SPITFT, "File content :\r\n");
        do {
            // Read a chunk of file
            rc = SD.read(SPISD, &f, Buff, sizebuffer, &nbr);
            if (rc || !nbr)
                break;	// Error or end of file
            else
            {		// Type the data
                Buff[nbr] = 0;
                ST7735.printf(SPITFT, "%s", Buff);
                if ( nbr < sizebuffer)
                   ST7735.printf(SPITFT, "\r\n");
            }
        } while (nbr == sizebuffer);

        SD.close(SPISD, &f);
    }
    else
    {
        ST7735.printf(SPITFT, "Failed %s\r\n", SD.getError(rc));
        //while(1);
    }
    delay(5000);
}
