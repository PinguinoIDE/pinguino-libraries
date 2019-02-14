/*
    Display CPU and Peripheral frequency
*/

#include <fonts/font6x8.h>

#define SPITFT SPI2

u8 cpu, per, pll, plldiv, cpudiv, src;

void setup()
{
    pinMode(USERLED, OUTPUT);
    ST7735.init(SPITFT, 7); // DC
    ST7735.setFont(SPITFT, font6x8);
    ST7735.setBackgroundColor(SPITFT, ST7735_BLACK);
    ST7735.setColor(SPITFT, ST7735_YELLOW);
    ST7735.setOrientation(SPITFT, 90);
    ST7735.clearScreen(SPITFT);

    pll = System.getPLL();
    plldiv = System.getPLLDIV();
    cpudiv = System.getCPUDIV();
    src = System.getSource();
    cpu = System.getCpuFrequency() / MHZ;
    per = System.getPeripheralFrequency() / MHZ;
    
    ST7735.printCenter(SPITFT, "PINGUINO INFO.\r\n");
    ST7735.printf(SPITFT, "PROC   : %s\r\n", PROC_NAME);
    ST7735.printf(SPITFT, "BOOT   : v%d\r\n", BOOT_VER);
    ST7735.printf(SPITFT, "\r\n");
    ST7735.printf(SPITFT, "CPU    : %d MHz\r\n", cpu);
    ST7735.printf(SPITFT, "PERIPH : %d MHz\r\n", per);
    ST7735.printf(SPITFT, "\r\n");
    ST7735.printf(SPITFT, "\r\n");
    ST7735.printf(SPITFT, "PLL    : %d = %dMHz\r\n", pll, cpu = 96/pll);
    ST7735.printf(SPITFT, "PLLDIV : %d = %dMHz\r\n", plldiv, cpu = cpu/plldiv);
    ST7735.printf(SPITFT, "CPUDIV : %d = %dMHz\r\n", cpudiv, cpu = cpu/cpudiv);
    ST7735.printf(SPITFT, "SOURCE : %d\r\n", src);
}

void loop()
{
    u16 fps;                       // frame per second
    u32 timeEnd = millis() + 1000; // 1000 ms = 1 sec

    for (fps = 1; millis() < timeEnd; fps++);

    ST7735.setCursor(SPITFT, 0, 7);
    ST7735.printf(SPITFT, "MIPS   : %u\r\n", fps);

    toggle(USERLED);
}
