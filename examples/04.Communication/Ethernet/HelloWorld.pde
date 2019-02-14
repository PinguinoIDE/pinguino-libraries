// A simple web server that always just says "Hello World"
// Note : About the cable, if you are planning to connect
// your board directly to a PC you'll need a crossover cable,
// if you will connect to a switch/hub/router you'll need a
// straight cable.

#include <fonts/font6x8.h>
#define SPILCD SPI1
#define SPIETH SPI2

// The MAC address of your device.
// This just needs to be unique for your network, 
// so unless you have more than one of these boards
// connected, you should be fine with this value.
static u8 mac[6] = {0x54, 0x55, 0x58, 0x10, 0x00, 0x24};
                                                           
// The IP address for your board.
// Check your home hub to find an IP address not in use.
// You can scan used IPs with : (Linux) nmap -T4 -sP 192.168.1.0/24
static u8 ip[4] = {192, 168, 1, 211};

// Use port 80 - the standard for HTTP
static u16 http_port = 80;

void setup()
{ 
    pinMode (USERLED, OUTPUT); 

    Ethernet.init(SPIETH, mac, ip, http_port);

    ST7735.init(SPILCD, 14); // DC=TX1 for Pinguino torda UEXT1
    //ST7735.init(SPILCD, 24); // DC=SCL2 for Pinguino torda UEXT2
    //ST7735.init(SPILCD, 19); // DC=SCL1 for Olimex PIC32 Pinguino UEXT
    ST7735.setFont(SPILCD, font6x8);
    ST7735.setColor(SPILCD, ST7735_YELLOW);
    ST7735.setBackgroundColor(SPILCD, ST7735_BLACK);
    //ST7735.setOrientation(SPILCD, 90);
    ST7735.clearScreen(SPILCD);
}

void loop()
{
    u8 rev, i;

    rev = ENC28J60getrev(SPIETH);
    ST7735.printf(SPILCD, "%03d - Rev. %d\r\n", i++, rev);

    if (Ethernet.serviceRequest(SPIETH))
    {
        Ethernet.print("<H1>Hello World</H1>");
        Ethernet.respond(SPIETH);
        toggle(USERLED);
    }
    //delay(1000);
}
