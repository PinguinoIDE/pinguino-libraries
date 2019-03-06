/*    ---------------------------------------------------------------------------
    Send AT commands to your bluetooth module and check what it returns.
    2011 Regis Blanchot    <rblanchot@gmail.com>
    http://www.pinguino.cc
    ---------------------------------------------------------------------------
    tested with :
    - Olimex PIC32-PINGUINO & MOD-BT Bluetooth Transciever Module with BGB203
    - JY-MCU Bluetooth HC-06 board (slave only module)
    ---------------------------------------------------------------------------
    Note : don't pair your device or you won't be able to send commands    
    ---------------------------------------------------------------------------*/

u8 *cmd;
u8 recvChar;  

void setup()
{
    pinMode(USERLED, OUTPUT);

    // Output/Input interface
    Serial1.begin(9600);

    // BT Module HC-06 is connected to UART2
    //BT.begin(BT_HC06, UART2, 9600);

    // BT Module BGB203 is connected to UART2
    BT.begin(BT_BGB203, UART2, 115200);
}

void loop()  
{  
    // Get a command from the interface
    // HC06 : AT, AT+NAME
    // BGB203 : ATI, AT+BTLNM?, AT+BTBDA?
    cmd = Serial1.getString();
    Serial1.print("\r\n");  

    // Send the command to the Bluetooth module  
    //Serial2.print(cmd);
    //Serial2.print("+++\r");
    Serial2.print("ATI\r");
    //delay(1000);

    // wait until something is received
    while (!Serial2.available());

    // Get the response and write it on the interface
    while (Serial2.available())
    {  
        recvChar = Serial2.read();  
        //Serial1.write(recvChar); 
        Serial1.printf("[%c]", recvChar);  
    }
    Serial1.print("\r\n");  
}
