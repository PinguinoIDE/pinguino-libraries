/*
    The SHT7 is a temperature and relative humidity sensor.
    It works almost like an I2C device (but it is not).
    This code use float to ascii conversion and print a float result
    on USB CDC port.

    CHANGELOG:
           2011 - Jean-Pierre Mandon - first version
    12 Mai 2017 - Régis Blanchot     - updated
*/

#define DATApin  0            // SDA is connected to pin 0 of Pinguino
#define CLOCKpin 1            // SCK is connected to pin 1 of Pinguino

void setup()
{
    pinMode(DATApin,OUTPUT);
    pinMode(CLOCKpin,OUTPUT);
    digitalWrite(DATApin,1);
    digitalWrite(CLOCKpin,0);
}

u16 send_command(unsigned char value)
{
    u8 i;
    u8 lowbyte;
    u8 highbyte;

    // start condition
    digitalWrite(DATApin,1);
    delay(1);
    digitalWrite(CLOCKpin,HIGH);
    delay(1);
    digitalWrite(DATApin,LOW);
    delay(1);
    digitalWrite(CLOCKpin,LOW);
    delay(1);
    digitalWrite(CLOCKpin,HIGH);
    delay(1);
    digitalWrite(DATApin,HIGH);
    delay(1);
    digitalWrite(CLOCKpin,LOW);
    delay(1);

    // send command
    for (i=0;i<8;i++)
    {
        if ((value&0x80)==0x80)
            digitalWrite(DATApin,1);
        else
            digitalWrite(DATApin,0);
        value=value<<1;
        delay(1);
        digitalWrite(CLOCKpin,1);
        delay(1);
        digitalWrite(CLOCKpin,0);
        delay(1);
    }

    // wait for acknowledge   
    pinMode(DATApin,INPUT);
    digitalWrite(CLOCKpin,1);
    delay(5);
    if (digitalRead(DATApin)) 
        return 0xFFFF;    // bad acknowledge
    digitalWrite(CLOCKpin,0);

    // wait for conversion time
    delay(500);

    // check the DATApin line to know if measurement is OK
    if (digitalRead(DATApin))
        return 0xFFFF;

    // reading measurement value high byte
    highbyte=0;
    for (i=0;i<8;i++)
    {
        digitalWrite(CLOCKpin,1);
        delay(1);
        if (digitalRead(DATApin))
            highbyte=highbyte+1;   
        digitalWrite(CLOCKpin,0);
        delay(1);
        highbyte=highbyte<<1;
        delay(1);
    }

    // acknowledge MSB   
    pinMode(DATApin,OUTPUT);
    digitalWrite(DATApin,0);
    digitalWrite(CLOCKpin,1);
    delay(1);
    digitalWrite(CLOCKpin,0);
    delay(1);

    // reading measurement value low byte
    pinMode(DATApin,INPUT);
    lowbyte=0;
    for (i=0;i<8;i++)
    {
        digitalWrite(CLOCKpin,1);
        delay(1);
        if (digitalRead(DATApin))
           lowbyte=lowbyte+1;       
        digitalWrite(CLOCKpin,0);
        delay(1);
        lowbyte=lowbyte<<1;
        delay(1);
    }

    // Set High to DATApin line to end communication   
    pinMode(DATApin,OUTPUT);
    digitalWrite(DATApin,1);   
    return(lowbyte+(highbyte*256));
}

// reset sensor
void reset_SHT7()
{
    u8 i;

    pinMode(DATApin,OUTPUT);   
    digitalWrite(DATApin,1);
    for (i=0;i<10;i++)
    {
        digitalWrite(CLOCKpin,1);
        delay(1);
        digitalWrite(CLOCKpin,0);
        delay(1);
    }
}

void loop()
{
    u16 i;
    float t;

    delay(3000);
    CDC.print("start conversion ...\n\r");
    while(1)
    {
        delay(500);
        // send command 3 to SHT7 (read temperature)
        i = send_command(3);
        if (i < 65535)  // if no error
        {
            // convert measurement to real value
            t = (-40 + (0.01 * i)) / 4;
            CDC.printf("temperature is %f\r\n", t);
        }
        reset_SHT7();
    }
}
