/*
        Author: 	Régis Blanchot (Feb. 2014)
        Descr:      SPI-drived 8x8 Led-Matrix
        Output:	MAX72XX + 8x8 Led-Matrix
        
        2 modes available :
        - Hardware SPI
            . default mode
            . SPI operations are handled by the CPU
            . pins have to be the CPU SPI pins
            . PINGUINO 32 have up to 4 SPI module (SPI1 to  SPI4)
            . PINGUINO 8  have up to 2 SPI module (SPI1 and SPI2)
        - Software SPI
            . SPISW
            . SPI operations are handled by the SPI library
            . pins can be any digital pin
        
        MAX72XX   Pinguino
        ---------------------------------------
        DIN       SDOx
        CLK       SCKx
        CS        SSx
     
        SPIx      Microchip    Pinguino 1459
        ---------------------------------------
        SDO1      RC7          Pin 6
        SCK1      RB6          Pin 13
        SDI1      RB4          Pin 11
        SS1       RC6          Pin 5

        SPIx      Microchip    Pinguino x5(K)50
        ---------------------------------------
        SDO1      RC7          Pin 23
        SCK1      RB1          Pin 1
        SDI1      RB0          Pin 0
        SS1       RA5          Pin 13 

        SPIx      Microchip    Pinguino 47J53
        ---------------------------------------
        SDO1      RC7          Pin 23
        SCK1      RB4          Pin 4
        SDI1      RB5          Pin 5
        SS1       RB6          Pin 6
        ---------------------------------------
        SDO2      RB1          Pin 1
        SCK2      RB2          Pin 2
        SDI2      RB3          Pin 3
        SS2       RB0          Pin 0

        SPIx      Microchip    Pinguino 32MX2x0
        ---------------------------------------
        SDO1                   Pin 7
        SCK1                   Pin 1
        SDI1                   Pin 8
        SS1
        ---------------------------------------
        SDO2                   Pin 4
        SCK2                   Pin 0
        SDI2                   Pin 2
        SS2
*/

#define NBMATRIX 3 // number of led-matrix

u8 i;

void setup()
{
    // SPI SOFTWARE
    // pin 23 (SDO) is connected to the DataIn 
    // pin 1  (SCK) is connected to the CLK 
    // pin 13 (SS)  is connected to the CS  
    //LedControl.init(SPISW, 23, 1, 13, NBMATRIX);
    //LedControl.init(SPISW, SDO, SCK, SS, NBMATRIX);

    // SPI HARDWARE
    LedControl.init(SPI1, NBMATRIX);

    // MAX72XX are in power-saving mode on startup,
    // we have to do a wakeup call
    LedControl.powerOn();
    // Set brightness (0~15 possible values)
    //LedControl.setIntensity(0);
    // Clear all the displays
    //LedControl.clearAll();
}

void loop()
{
    // display the number with printf
    LedControl.printf("%03d", i++);
}
