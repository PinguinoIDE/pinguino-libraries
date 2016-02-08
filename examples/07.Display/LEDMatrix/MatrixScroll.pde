#define MATRIX 3 // number of matrix

u8 i;

void setup()
{
    // SPI SOFTWARE
    // pin 0 is connected to the DataIn 
    // pin 1 is connected to the CLK 
    // pin 2 is connected to the CS  
    //LedControl.init(SPISW, 0, 1, 2, MATRIX);

    // SPI HARDWARE
    LedControl.init(SPI1, MATRIX);

    // MAX72XX are in power-saving mode on startup
    LedControl.powerOn();
    // Set the brightness (0~15 possible values)
    //LedControl.setIntensity(1);
}

void loop()
{
    LedControl.scroll("   www.pinguino.cc");
}
