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

const char* ssid = "YOUR_SSID";//type your ssid
const char* password = "YOUR_PASSWORD";//type your password

WiFiServer server(80);//Service Port

void setup()
{
    Serial.begin(9600);
    Wifi.init(UART2, 115200);

    pinMode(USERLED, OUTPUT);
    digitalWrite(USERLED, LOW);

    // Connect to WiFi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.connect(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");

    // Start the server
    Server.init();
    Serial.println("Server started");

    // Print the IP address
    Serial.print("Use this URL to connect: ");
    Serial.print("http://");
    Serial.print(WiFi.getlocalIP());
    Serial.println("/");
}

void loop()
{
    int value = LOW;

    // Check if a Client has connected
    WiFiClient Client = server.available();
    if (!Client)
        return;

    // Wait until the Client sends some data
    Serial.println("new Client");
    while (!Client.available());

    // Read the first line of the request
    String request = Client.readStringUntil('\r');
    Serial.println(request);
    Client.flush();

    // Match the request

    if (request.indexOf("/LED=ON") != -1)
    {
        digitalWrite(USERLED, HIGH);
        value = HIGH;
    } 
    if (request.indexOf("/LED=OFF") != -1)
    {
        digitalWrite(USERLED, LOW);
        value = LOW;
    }

    //Set USERLED according to the request
    //digitalWrite(USERLED, value);

    // Return the response
    Client.println("HTTP/1.1 200 OK");
    Client.println("Content-Type: text/html");
    Client.println(""); //  do not forget this one
    Client.println("<!DOCTYPE HTML>");
    Client.println("<html>");

    Client.print("Led pin is now: ");
    Client.print((value) ? "On":"Off");

    Client.println("<br><br>");
    Client.println("Click <a href=\"/LED=ON\">here</a> turn the USERLED ON<br>");
    Client.println("Click <a href=\"/LED=OFF\">here turn the USERLED OFF<br>");
    Client.println("</html>");

    Serial.println("Client disconnected");
    Serial.println("");
}
