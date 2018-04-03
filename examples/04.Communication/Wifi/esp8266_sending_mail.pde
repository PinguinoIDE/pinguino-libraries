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

void setup()
{
    Serial.println("Pinguino Mail using ESP");
    delay(1500);

    /*Check if the communication is successful*/
    // Wait till the ESP send back "OK"
    do
    {
        Serial.println("ESP not found");
    }
    while (!Wifi.init(UART2, 115200));
    Serial.println("ESP is connected");
    delay(1500);
    
    /*Put the module in AP+STA*/
    Wifi.mode(ESP8266_STATION|ESP8266_SOFTAP);
    Serial.println("ESP set AP+STA");
    delay(1500);
    
    /*Connect to a AccesPoint*/
    Wifi.connect("BPAS home","cracksun"); //Enter you WiFi name and password here, here BPAS home is the name and cracksun is the pas
    Serial.println("Connected 2 WIFI"); //Print on LCD for debugging. 
    delay(1500);
    
    Wifi.enaleMUX(); //Enable multiple connections
    Wifi.createServer(); //Create a server on port 80
    Wifi.connectSMPT2GO(); //Establish TCP connection with SMPT2GO
    
    /* LOG IN with your SMPT2GO approved mail ID
     * Visit the page https://www.smtp2go.com/ and sign up using any Gmail ID
     * Once you gmail ID is SMPT2GO approved convert your mail ID and password in 64 base format
     * visit https://www.base64encode.org/ for converting 64 base format online
     * FORMAT -> Wifi.login_mail("mailID in base 64","Password in base 64");
     * This program uses the ID-> aswinthcd@gmail.com and password -> circuitdigest as an example
     */
    Wifi.mailLogin("YXN3aW50aGNkQGdtYWlsLmNvbQ==","Y2lyY3VpdGRpZ2VzdA==");
    Serial.println("Login Successful"); //display on LCD for debugging
    delay(1500);
    
    // The sender mail ID
    Wifi.mailFrom("rblanchot@gmail.com");
    // The sender mail ID
    Wifi.mailTo("mailtoaswinth@gmail.com");

    // The Receiver mail ID
    Wifi.mailStart();
    // Enter the subject of your mail
    Wifi.mailSubject("Mail from Pinguino Wifi");
    // Enter the body of your mail
    Wifi.mailBody("Testing Success");
    Wifi.mailEnd();
    
    Wifi.disconnectSMPT2GO();
    
    Serial.println("Mail Sent"); //Print on LCD for debugging
}

void loop()
{
    // do nothing 
}
