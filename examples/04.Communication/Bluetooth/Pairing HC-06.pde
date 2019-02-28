/*  --------------------------------------------------------------------
    Pairing a Bluetooth Device to your Pinguino
    2017 Regis Blanchot <rblanchot@gmail.com>
    http://www.pinguino.cc
    --------------------------------------------------------------------
    Tested with JY-MCU Bluetooth HC-06 board (slave only module)
    Default name : Pinguino
    Default pin code : 1234
    ------------------------------------------------------------------*/

u8 i;
u8 buffer[64];  

void setup()
{
    pinMode(USERLED, OUTPUT);
    // BT Module HC-06 is connected to UART2
    BT.begin(BT_HC06, UART2, 9600);
    BT.setDeviceName(UART2, "Pinguino");
    BT.setPincode(UART2, 3333);
    // Now go to your Bluetooth device (smartphone for ex.)
    // and look for the Pinguino Bluetooth device,
    // code pin is 3333
}

void loop()
{
    // Take a look on your phone if it recognizes your Pinguino Device
    // Now you are in Data Mode
    // Use Serial2 functions to send and/or receive data
    BT.send(UART2, "i=%03d\r\n", i++);
    toggle(USERLED);
}
