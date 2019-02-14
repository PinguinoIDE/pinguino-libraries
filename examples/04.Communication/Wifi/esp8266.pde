/**
 * ESP8266 Demo - 2018
 * Regis Blanchot <rblanchot@gmail.com>
 * 
 * An ESP8266 may need a 250mA current for data transferring.
 * Therefore, we may need at least 500mA for powering the ESP8266
 * if we want to have a good margin.
 * Powering an ESP8266 with USB cables that get their power from a
 * computer can lead to instability.
 */

const u8* ip = "192.168.1.123";//type your ssid
const u8* ssid = "myssid";//type your ssid
const u8* pass = "mypassword";//type your password

void setup()
{
    u8 r;
    pinMode(USERLED, OUTPUT);
    Serial.begin(9600);         // UART1
    Serial.print("\f\f\f");

    // Try to connect
    do
    {
        r = Wifi.init(UART2);
        Serial.printx("r=", r, DEC);
        toggle(USERLED);
        delay(500);
    }
    while(!r);
    Serial.println("ESP is up and running.");
    delay(1500);
/*
    while(!Wifi.connect(UART2, ssid, pass))
        toggle(USERLED);
    Serial.println("Wifi is connected");
    delay(1500);
    
    // Set up an Access Point
    Wifi.mode(UART2, ESP8266_SOFTAP);
    // Configure it as a Web server
    Server.create(UART2, 80);
    // Start name and password
    Server.start(UART2, ESP8266_TCP, ip, 80);
    Serial.println("AP configured");
    delay(1500);
*/
}

void loop()
{
}
