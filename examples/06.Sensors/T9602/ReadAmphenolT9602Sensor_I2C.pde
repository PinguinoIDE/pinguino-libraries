/*-----------------------------------------------------
  Author: --<Josep M. Mirats Tur>
  Company: --<INLOC ROBOTICS S.L>
  Project: UsbComm
  Date: 2017-08-02 18:55:38
  Description: Reads Tª and Humidity from I2C Amphenol Sensor at 5.5 seconds rate
               It checks the status bits read from the sensor. Can react to a unexpected
               state on "command mode" from the sensor.
               Code tested on PIC32-PINGUINO-OTG Board -> PIC32MX440256H
               It uses I2C2 on CON3 (not provided with the board)
               It should work on all Pinguino boards

  Notes: Interrupts are configured as Multi Vector when calling CDC_begin() in usbcdc.c
-----------------------------------------------------*/

// Define variables
u8 SensorAddress = 0x28; //Telaire App Note 916-127- Datasheet, Table 16, page 43.
u8 value[4];
u16 nloop=0;


void LedFastTick(u8 k)
{
  while(k--)
  {
    toggle(USERLED);
    delay(100);
  }
  digitalWrite(USERLED, LOW); //Be sure we leave the function with the led OFF
}


void setup()
{
  pinMode(USERLED, OUTPUT);

  // Init. I2C comms on port 2
  I2C2.master(I2C_100KHZ);

  //Say Hello blinking the yellow LED for 1 second
  digitalWrite(USERLED, HIGH);
  delay(1000);
  digitalWrite(USERLED, LOW);
  delay(500);
}


void loop()
{
  I2C2.start();
  //Data Fetch (DF) command - 0x51 = Slave address 0x28 + Read bit (1)
  I2C2.writeChar((SensorAddress<<1) + 1);

  value[0]=I2C2.readChar();
  I2C2.sendAck();

  value[1]=I2C2.readChar();
  I2C2.sendAck();

  value[2]=I2C2.readChar();
  I2C2.sendAck();

  value[3]=I2C2.readChar();
  I2C2.sendNack();

  I2C2.stop();

  u8 status=0xFF;
  u16 aux;
  float humidity;
  float tempC;

  status = value[0] >> 6;

  switch(status){
    //Valid data from sensor
    case 0:
     aux=(value[0] & 63)*256 + value[1];
     humidity = (float)aux/16384*100;
     aux= value[2]*64+(value[3] & 252);
     tempC = (((float)aux/16384)*165) - 40;
     CDC.printf("H: %f; ", humidity);
     CDC.printf("T: %f; ", tempC);
     break;

    //Data sensor was already read. Nothing to do, will just wait next reading
    case 1:
      //CDC.printf("No new data\r\n");
      //YellowLedFastTick(1);
      break;

    //Sensor is in command mode for whatever reason. Try to put the sensor in Update mode
    case 2:
      LedFastTick(5);
      I2C2.start();
      //0x50 = Slave address 0x28 + Write bit (0)
      I2C2.writeChar((SensorAddress<<1) + 0);
      I2C2.writeChar(0x80); //Escape mode
      I2C2.writeChar(0x00); //Data is 0
      I2C2.writeChar(0x00); //Data is 0
      I2C2.stop();
      //After a escape command is issued we need to perform a Data Fetch
      I2C2.start();
      //0x51 = Slave address 0x28 + Read bit (1)
      I2C2.writeChar((SensorAddress<<1) + 1);
      u8 dum = I2C2.readChar();
      I2C2.sendAck();
      dum = I2C2.readChar();
      I2C2.sendAck();
      dum = I2C2.readChar();
      I2C2.sendAck();
      dum = I2C2.readChar();
      I2C2.sendNack();
      I2C2.stop();
      break;

    //Unknown status. Just inform
    default:
      //CDC.printf("Unknown status\r\n");
      break;
  }  

  CDC.printf("stat %u; ",status);
  CDC.printf("loop %u\r\n",nloop++);
  LedFastTick(5);
  delay(5000);
}

