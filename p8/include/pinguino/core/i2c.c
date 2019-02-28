/*  --------------------------------------------------------------------
    FILE:       i2c.c
    PROJECT:    Pinguino
    PURPOSE:    I2C communication for Master and Slave
    PROGRAMER:  Régis Blanchot
    --------------------------------------------------------------------
    03 Apr. 2010 - Régis blanchot - first release
    26 Sep. 2014 - Régis blanchot - fixed x5k50 and xxj53 support
    04 Feb. 2016 - Régis blanchot - added 16F1459 support
    25 Nov. 2016 - Régis blanchot - added multi-module support (I2C1, I2C2)
                                  - renamed write and read to writeChar and readChar
                                    for compatibility reason with the 32-bit version
    --------------------------------------------------------------------
    TODO
    * Slave 10-bit address
    * SSPADD = Fcpu / (4 * Fi2c) - 1
    --------------------------------------------------------------------
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
    ------------------------------------------------------------------*/

#ifndef __I2C_C
#define __I2C_C

#define __I2C__

#include <compiler.h>
#include <typedef.h>
//#include <macro.h>              // interrupt(), noInterrupt(), ...
#include <const.h>
#include <i2c.h>

#ifdef WIRE
#include <i2cmaster.c>
#endif

/*  --------------------------------------------------------------------
    ---------- Initialisation Functions for Master and Slave
    ------------------------------------------------------------------*/

void I2C_master(u8 module, u16 speed)
{
    I2C_init(module, I2C_MASTER_MODE, speed);
}

void I2C_slave(u8 module, u16 DeviceID)
{
    I2C_init(module, I2C_SLAVE_MODE, DeviceID);
}

/*  --------------------------------------------------------------------
    ---------- Open the I2C bus
    --------------------------------------------------------------------
    SSPSTAT.SMP: Slew Rate Control bit
    In Master or Slave mode:
    1 = Slew Mode Off = Standard Speed mode (100 kHz and 1 MHz)
    0 = Slew Mode On = High-Speed mode (400 kHz)
    * u8 mode = I2C_MASTER_MODE or I2C_SLAVE_MODE
    * u16 sora = means s(peed) or a(ddress)
               = speed (100, 400 or 1000 KHz) in master mode
               = address in slave mode
    ------------------------------------------------------------------*/

void I2C_init(u8 module, u8 mode, u16 sora)
{
    u8 conf;

    // Datasheet 22.4.3 SDA AND SCL PINS
    // Selection of any I2C mode with the SSPEN bit set, forces the SCL
    // and SDA pins to be open-drain. These pins should be set by the
    // user to inputs by setting the appropriate TRIS bits.
    
    if (module == I2C1)
    {
        #if defined(__16F1459)  || \
            defined(__18f13k50) || defined(__18f14k50)
        
        TRISBbits.TRISB4 = INPUT;           // SDA = INPUT
        TRISBbits.TRISB6 = INPUT;           // SCL = INPUT
        
        #elif defined(__18f26j50) || defined(__18f46j50) || \
              defined(__18f26j53) || defined(__18f46j53) || \
              defined(__18f27j53) || defined(__18f47j53)

        TRISBbits.TRISB5 = INPUT;           // SDA = INPUT
        TRISBbits.TRISB4 = INPUT;           // SCL = INPUT

        #else // x550 and x5k50

        TRISBbits.TRISB0 = INPUT;           // SDA = INPUT
        TRISBbits.TRISB1 = INPUT;           // SCL = INPUT

        #endif
    }            

    #if defined(__18f46j50) || defined(__18f46j53) || defined(__18f47j53)
    else if (module == I2C2)
    {
        TRISDbits.TRISD1 = INPUT;           // SDA2 = INPUT
        TRISDbits.TRISD0 = INPUT;           // SCL2 = INPUT
    }
    #endif

    switch (mode)
    {
        case I2C_SLAVE_MODE:

            //intUsed[INT_SSP] = INT_USED;
            // Enabling interrupts (peripheral & general)
            //INTCONbits.PEIE=1;
            //INTCONbits.GIE=1;
            if (sora > 0x80)
                conf = 0b00101111;      // Slave mode, 10-bit address with Start and Stop bit interrupts enabled
            else
                conf = 0b00101110;      // 00101110Slave mode,  7-bit address with Start and Stop bit interrupts enabled

            if (module == I2C1)
            {
                #if defined(__16F1459)  || \
                    defined(__18f25k50) || defined(__18f45k50) || \
                    defined(__18f26j53) || defined(__18f46j53) || \
                    defined(__18f27j53) || defined(__18f47j53)
                SSP1CON1 = conf;
                SSP1ADD = sora;             // Slave 7-bit address
                #else
                SSPCON1 = conf;
                SSPADD = sora;              // Slave 7-bit address
                #endif
            }

            #if defined(__18f46j50) || defined(__18f46j53) || defined(__18f47j53)
            else if (module == I2C2)
            {
                SSP2CON1 = conf;
                SSP2ADD = sora;             // Slave 7-bit address
            }
            #endif

        case I2C_MASTER_MODE:
        default:// I2C_MASTER_MODE

            if (module == I2C1)
            {
                #if defined(__16F1459)  || \
                    defined(__18f25k50) || defined(__18f45k50) || \
                    defined(__18f26j53) || defined(__18f46j53) || \
                    defined(__18f27j53) || defined(__18f47j53)
                SSP1CON1= 0b00101000;       // Master Mode, clock = FOSC/(4 * (SSPADD + 1))
                #else
                SSPCON1= 0b00101000;        // Master Mode, clock = FOSC/(4 * (SSPADD + 1))
                #endif
                // datasheet p208
                switch (sora)
                {
                    case I2C_1MHZ:
                        // SMP = 1 = Slew rate control disabled for Standard Speed mode (100 kHz and 1 MHz)
                        #if defined(__16F1459)  || \
                            defined(__18f25k50) || defined(__18f45k50) || \
                            defined(__18f26j53) || defined(__18f46j53) || \
                            defined(__18f27j53) || defined(__18f47j53)
                        SSP1STATbits.SMP =1;// Slew Mode Off
                        SSP1ADD= 11;        // 1MHz = FOSC/(4 * (SSPADD + 1))
                        #else
                        SSPSTATbits.SMP = 1;// Slew Mode Off
                        SSPADD= 11;         // 1MHz = FOSC/(4 * (SSPADD + 1))
                        #endif
                                            // SSPADD = 48 000 / (4*1000) - 1
                        break;
                        
                    case I2C_400KHZ:
                        // SMP = 0 = Slew rate control enabled for High-Speed mode (400 kHz)
                        #if defined(__16F1459)  || \
                            defined(__18f25k50) || defined(__18f45k50) || \
                            defined(__18f26j53) || defined(__18f46j53) || \
                            defined(__18f27j53) || defined(__18f47j53)
                        SSP1STATbits.SMP =0;// Slew Mode On
                        SSP1ADD= 29;        // 400kHz = FOSC/(4 * (SSPADD + 1))
                        #else
                        SSPSTATbits.SMP = 0;// Slew Mode On
                        SSPADD= 29;         // 400kHz = FOSC/(4 * (SSPADD + 1))
                        #endif
                                            // SSPADD = 48 000 / (4*400) - 1
                        break;

                    case I2C_100KHZ:
                    default:
                        // SMP = 1 = Slew rate control disabled for Standard Speed mode (100 kHz and 1 MHz)
                        #if defined(__16F1459)  || \
                            defined(__18f25k50) || defined(__18f45k50) || \
                            defined(__18f26j53) || defined(__18f46j53) || \
                            defined(__18f27j53) || defined(__18f47j53)
                        SSP1STATbits.SMP =1;// Slew Mode Off
                        SSP1ADD= 119;       // 100kHz = FOSC/(4 * (SSPADD + 1))
                        #else
                        SSPSTATbits.SMP = 1;// Slew Mode Off
                        SSPADD= 119;        // 100kHz = FOSC/(4 * (SSPADD + 1))
                        #endif
                                            // SSPADD = 48 000 / (4*100) - 1
                        break;
                }
            }
                    
            #if defined(__18f46j50) || defined(__18f46j53) || defined(__18f47j53)
            else if (module == I2C2)
            {
                SSP2CON1= 0b00101000;       // Master Mode, clock = FOSC/(4 * (SSPADD + 1))

                // datasheet p208
                switch (sora)
                {
                    case I2C_1MHZ:
                        // SMP = 1 = Slew rate control disabled for Standard Speed mode (100 kHz and 1 MHz)
                        SSP2STATbits.SMP =1;// Slew Mode Off
                        SSP2ADD= 11;        // 1MHz = FOSC/(4 * (SSPADD + 1))
                                            // SSPADD = 48 000 / (4*1000) - 1
                        break;
                        
                    case I2C_400KHZ:
                        SSP2STATbits.SMP =0;// Slew Mode On
                        SSP2ADD= 29;        // 400kHz = FOSC/(4 * (SSPADD + 1))
                                            // SSPADD = 48 000 / (4*400) - 1
                        break;

                    case I2C_100KHZ:
                    default:
                        // SMP = 1 = Slew rate control disabled for Standard Speed mode (100 kHz and 1 MHz)
                        SSP2STATbits.SMP =1;// Slew Mode Off
                        SSP2ADD= 119;       // 100kHz = FOSC/(4 * (SSPADD + 1))
                                            // SSPADD = 48 000 / (4*100) - 1
                        break;
                }
            }
            #endif
    }
    
    if (module == I2C1)
    {
        #if defined(__16F1459)  || \
            defined(__18f25k50) || defined(__18f45k50) || \
            defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
        SSP1CON2 = 0;
        #else
        SSPCON2 = 0;
        #endif
       
        #if defined(__16F1459)  || \
            defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
        PIR1bits.SSP1IF = 0;                // MSSP Interrupt Flag
        PIR2bits.BCL1IF = 0;                // Bus Collision Interrupt Flag
        #else
        PIR1bits.SSPIF = 0;                 // MSSP Interrupt Flag
        PIR2bits.BCLIF = 0;                 // Bus Collision Interrupt Flag
        #endif
        //Delayms(1000);
    }
            
    #if defined(__18f46j50) || defined(__18f46j53) || defined(__18f47j53)
    else if (module == I2C2)
    {    
        SSP2CON2 = 0;
        PIR3bits.SSP2IF = 0;                // MSSP Interrupt Flag
        PIR3bits.BCL2IF = 0;                // Bus Collision Interrupt Flag
    }
    #endif
}

/*  --------------------------------------------------------------------
    ---------- Send byte and return ack bit
    --------------------------------------------------------------------
    WCOL: Write Collision Detect bit, in Master Transmit mode:
    1 = A write to the SSPxBUF register was attempted while the I2C conditions
    were not valid for a transmission to be started (must be cleared in software)
    0 = No collision

    BF: Buffer Full Status bit, in Transmit mode:
    1 = SSPxBUF is full
    0 = SSPxBUF is empty

    A slave sends an Acknowledge when it has recognized its address
    (including a general call), or when the slave has properly received its data.
    If the master receives an Acknowledge, ACKSTAT is cleared;
    if not, the bit is set.
    
    !!! ACK is received BEFORE BF is cleared !!!
    (Datasheet 19.5.10 I2C MASTER MODE TRANSMISSION)
    On the falling edge of the ninth clock :
    * the status of the ACK bit is loaded into the ACKSTAT status bit
    * following the SSPxIF flag is set
    * then the BF flag is cleared

    Datasheet, figure 19-23
    ------------------------------------------------------------------*/

u8 I2C_write(u8 module, u8 value)
{
    I2C_idle(module);                   // Wait the MSSP module is inactive
    
    if (module == I2C1)
    {
        #if defined(__16F1459)  || \
            defined(__18f25k50) || defined(__18f45k50) || \
            defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
            
        
        SSP1BUF = value;                    // Write byte to SSPBUF (BF is set to 1)
        return (!SSP1CON2bits.ACKSTAT);     // 1 if Ack, 0 if NAck

        #else

        SSPBUF = value;                     // Write byte to SSPBUF (BF is set to 1)
        return (!SSPCON2bits.ACKSTAT);      // 1 if Ack, 0 if NAck

        #endif
    }
    
    #if defined(__18f46j50) || defined(__18f46j53) || defined(__18f47j53)
    else if (module == I2C2)
    {
        SSP2BUF = value;                    // Write byte to SSPBUF (BF is set to 1)
        return (!SSP2CON2bits.ACKSTAT);     // 1 if Ack, 0 if NAck
    }
    #endif

/*
    I2C_idle(module);                   // Wait the MSSP module is inactive

    while (SSPCON1bits.WCOL)            // Send again if write collision occurred 
    {
        LATCbits.LATC2 = 1;
        SSPCON1bits.WCOL = 0;           // Must be cleared by software
        SSPBUF = value;                 // Write byte to SSPBUF (BF is set to 1)
    }
    while (SSPSTATbits.BF);           // Wait until buffer is empty (BF set to 0)
*/
    return 0;
}

// i2caddress has to be a 7-bit address
u8 I2C_writeChar(u8 module, u8 i2caddress, u8 writefrom, u8 val)
{
    u8 r;
    
    I2C_start(module);
    I2C_write(module, (i2caddress << 1) & 0xFE); // write
    I2C_write(module, writefrom);
    r = I2C_write(module, val);
    I2C_stop(module);
    
    return r;
}

// i2caddress has to be a 7-bit address
u8 I2C_writeBytes(u8 module, u8 i2caddress, u8 writefrom, u8 *buffer, u8 length)
{
    u8 i, r;

    I2C_start(module);
    r = I2C_write(module, (i2caddress << 1) & 0xFE); // write
    if (r)
    {
        I2C_write(module, writefrom);
        for (i = 0; i < length; i++)
            I2C_write(module, buffer[i]);
    }
    I2C_stop(module);
    return r;
}

/*  --------------------------------------------------------------------
    ---------- Get a byte from the slave
    --------------------------------------------------------------------
    RCEN = Receive Enable bit
    The MSSP module must be in an inactive state before the RCEN bit is set
    or the RCEN bit will be disregarded.
    
    In receive operation, the BF bit is set to :
    - 1 when SSPxBUF is full (does not include the ACK and Stop bits)
    - 0 when SSPxBUF is empty (does not include the ACK and Stop bits)

    Datasheet, figure 19-24
    
    Fixed by Rolf Ziegler
    ------------------------------------------------------------------*/

u8 I2C_read(u8 module)
{
    I2C_idle(module);                   // Wait the MSSP module is inactive

    if (module == I2C1)
    {
        #if defined(__16F1459)  || \
            defined(__18f25k50) || defined(__18f45k50) || \
            defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
            
        SSP1CON2bits.RCEN = 1;              // Initiate reception of byte
        
        #else
        
        SSPCON2bits.RCEN = 1;               // Initiate reception of byte
        
        #endif

        #if defined(__16F1459)  || \
            defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
            
        PIR1bits.SSP1IF = 0;                // Clear SSP interrupt flag
        while (!PIR1bits.SSP1IF);           // Wait the interrupt flag is set
        PIR1bits.SSP1IF=0;                  // ROlf clear SSPIF
        PIR1bits.SSP1IF=0;                  // ROlf clear SSPIF
        
        #else
        
        PIR1bits.SSPIF = 0;                 // Clear SSP interrupt flag
        while (!PIR1bits.SSPIF);            // Wait the interrupt flag is set
        PIR1bits.SSPIF=0;                   // ROlf clear SSPIF
        PIR1bits.SSPIF=0;                   // ROlf clear SSPIF
        
        #endif

        #if defined(__16F1459)  || \
            defined(__18f25k50) || defined(__18f45k50) || \
            defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
        return SSP1BUF;
        #else
        return SSPBUF;
        #endif
    }
            
    #if defined(__18f46j50) || defined(__18f46j53) || defined(__18f47j53)
    else if (module == I2C2)
    {
        SSP2CON2bits.RCEN = 1;              // Initiate reception of byte
        PIR3bits.SSP2IF = 0;                // Clear SSP interrupt flag
        while (!PIR3bits.SSP2IF);           // Wait the interrupt flag is set
        PIR3bits.SSP2IF=0;                  // ROlf clear SSPIF
        PIR3bits.SSP2IF=0;                  // ROlf clear SSPIF
        return SSP2BUF;
    }
    #endif

    return 0;
}

// i2caddress has to be a 7-bit address
u8 I2C_readChar(u8 module, u8 i2caddress, u8 readfrom)
{
    u8 val;
    
    I2C_start(module);
    I2C_write(module, (i2caddress << 1) & 0xFE); // write
    I2C_write(module, readfrom);
    //I2C_stop(module);
    I2C_start(module);
    I2C_write(module, (i2caddress << 1) | 0x01); // read
    val = I2C_read(module);
    I2C_sendNack(module);
    I2C_stop(module);
    
    return val;
}

// i2caddress has to be a 7-bit address
u8 I2C_readBytes(u8 module, u8 i2caddress, u8 readfrom, u8 *buffer, u8 length)
{
    u8 i, r;

    I2C_start(module);
    r = I2C_write(module, (i2caddress << 1) & 0xFE); // write
    if (r)
    {
        I2C_write(module, readfrom);
        //I2C_stop(module);
        I2C_start(module);
        I2C_write(module, (i2caddress << 1) | 0x01); // read
        for(i = 0; i < length; i++)
        {
            buffer[i] = I2C_read(module);
            (i == length - 1) ? I2C_sendNack(module) : I2C_sendAck(module);
        }
    }
    I2C_stop(module);
    return r;
}

/*  --------------------------------------------------------------------
    ---------- Wait for the slave to finish its last action
    --------------------------------------------------------------------
    Application note AN245 page 5, Note 1:
    The master needs to wait for I2C bus idle to indicate that the MSSP
    has finished its last task. The SSPIF interrupt could be used
    instead of the wait for idle.

    The following events will cause the MSSP Interrupt Flag bit to be set:
    - Start condition
    - Stop condition
    - Repeated Start
    - Data transfer byte transmitted/received
    - Acknowledge transmitted
    ------------------------------------------------------------------*/

void I2C_wait(u8 module)
{
    if (module == I2C1)
    {
        #if defined(__16F1459)  || \
            defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
        while (!PIR1bits.SSP1IF);           // Wait the interrupt flag is set
        PIR1bits.SSP1IF = 0;                // Clear SSP interrupt flag
        #else
        while (!PIR1bits.SSPIF);            // Wait the interrupt flag is set
        PIR1bits.SSPIF = 0;                 // Clear SSP interrupt flag
        #endif
    }
    
    #if defined(__18f46j50) || defined(__18f46j53) || defined(__18f47j53)
    else if (module == I2C2)
    {
        while (!PIR3bits.SSP2IF);           // Wait the interrupt flag is set
        PIR3bits.SSP2IF = 0;                // Clear SSP interrupt flag
    }
    #endif
}

/*  --------------------------------------------------------------------
    ---------- Wait until module is no longuer active
    --------------------------------------------------------------------
    This function waits until 
    * all the five low bits of SSPCON2 (SEN, RSEN, PEN, RCEN or ACKEN) are 0
    AND
    * bit 2 of SSPSTAT (R_W) is 0
    
    If either 1 bit of SSPCON2 <4:0> is set or R_W bit is set then
    operation is in progress.

    Source = Datasheet : ORing R_W bit with SEN, RSEN, PEN, RCEN or ACKEN
    will indicate if the MSSP is in Active mode
    ------------------------------------------------------------------*/

void I2C_idle(u8 module)
{
    if (module == I2C1)
    {
        #if defined(__16F1459)
        
        while (((SSP1CON2 & 0x1F) > 0) | (SSP1STATbits.R_nW));

        #elif defined(__18f25k50) || defined(__18f45k50) || \
              defined(__18f26j53) || defined(__18f46j53) || \
              defined(__18f27j53) || defined(__18f47j53)

        while (((SSP1CON2 & 0x1F) > 0) | (SSP1STATbits.R_NOT_W));

        #else

        while (((SSPCON2 & 0x1F) > 0) | (SSPSTATbits.R_W));

        #endif
    }
    
    #if defined(__18f46j50) || defined(__18f46j53) || defined(__18f47j53)
    else if (module == I2C2)
    {
        while (((SSP2CON2 & 0x1F) > 0) | (SSP2STATbits.R_NOT_W));
    }
    #endif
}

/*  --------------------------------------------------------------------
    ---------- Wait for Acknowledge (Ack) from the slave
    --------------------------------------------------------------------
    In Transmit mode, the ACKSTAT bit (SSPxCON2<6>)
    is cleared when the slave has sent an Acknowledge
    (ACK = 0) and is set when the slave does not Acknowl-
    edge (ACK = 1). A slave sends an Acknowledge when
    it has recognized its address (including a general call),
    or when the slave has properly received its data.
    ------------------------------------------------------------------*/
/*
u8 I2C_waitAck(u8 module)
{
    u8 i=0;

    while (!SSPCON2bits.ACKSTAT)
    {
        i++;
        if (i==0) return 0;
    }
    return 1;
}
*/

/*  --------------------------------------------------------------------
    ---------- Send start bit
    --------------------------------------------------------------------
    Start condition is issued to indicate the beginning of a serial transfer.
    If the I2C module is active, this bit may not be set.
    ------------------------------------------------------------------*/

void I2C_start(u8 module)
{
    I2C_idle(module);                   // Wait module is inactive

    if (module == I2C1)
    {
        #if defined(__16F1459)  || \
            defined(__18f25k50) || defined(__18f45k50) || \
            defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
        //PIR1bits.SSPIF = 0;               // Clear SSP interrupt flag
        SSP1CON2bits.SEN = 1;               // Send start bit
        while (SSP1CON2bits.SEN);           // Wait until SEN is cleared 
        //while (!PIR1bits.SSPIF);          // Wait the interrupt flag is set
        #else
        //PIR1bits.SSPIF = 0;               // Clear SSP interrupt flag
        SSPCON2bits.SEN = 1;                // Send start bit
        while (SSPCON2bits.SEN);            // Wait until SEN is cleared 
        //while (!PIR1bits.SSPIF);          // Wait the interrupt flag is set
        #endif
    }
            
    #if defined(__18f46j50) || defined(__18f46j53) || defined(__18f47j53)
    else if (module == I2C2)
    {
        //PIR3bits.SS2PIF = 0;              // Clear SSP interrupt flag
        SSP2CON2bits.SEN = 1;               // Send start bit
        while (SSP2CON2bits.SEN);           // Wait until SEN is cleared 
        //while (!PIR3bits.SS2PIF);         // Wait the interrupt flag is set
    }
    #endif
}

/*  --------------------------------------------------------------------
    ---------- Send stop bit
    --------------------------------------------------------------------
    Stop condition is issued to indicate the end of a serial transfer.
    If the I2C module is active, this bit may not be set.
    When the PEN bit is cleared, the SSPIF bit is set. 
    ------------------------------------------------------------------*/

void I2C_stop(u8 module)
{
    I2C_idle(module);                   // Wait module is inactive

    if (module == I2C1)
    {
        #if defined(__16F1459)  || \
            defined(__18f25k50) || defined(__18f45k50) || \
            defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
        //PIR1bits.SSPIF = 0;               // Clear SSP interrupt flag
        SSP1CON2bits.PEN = 1;               // Send stop bit
        while (SSP1CON2bits.PEN);           // Wait until PEN is cleared 
        //while (!PIR1bits.SSPIF);          // Wait the interrupt flag is set
        #else
        //PIR1bits.SSPIF = 0;               // Clear SSP interrupt flag
        SSPCON2bits.PEN = 1;                // Send stop bit
        while (SSPCON2bits.PEN);            // Wait until PEN is cleared 
        //while (!PIR1bits.SSPIF);          // Wait the interrupt flag is set
        #endif
    }
                
    #if defined(__18f46j50) || defined(__18f46j53) || defined(__18f47j53)
    else if (module == I2C2)
    {
        //PIR3bits.SS2PIF = 0;               // Clear SSP interrupt flag
        SSP2CON2bits.PEN = 1;                // Send stop bit
        while (SSP2CON2bits.PEN);            // Wait until PEN is cleared 
        //while (!PIR3bits.SSP2IF);          // Wait the interrupt flag is set
    }
    #endif
}

/*  --------------------------------------------------------------------
    ---------- Send restart bit
    --------------------------------------------------------------------
    If the I2C module is active, this bit may not be set.
    ------------------------------------------------------------------*/

void I2C_restart(u8 module)
{
    I2C_idle(module);                   // Wait module is inactive
    
    if (module == I2C1)
    {
        #if defined(__16F1459)  || \
            defined(__18f25k50) || defined(__18f45k50) || \
            defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
        //PIR1bits.SSPIF = 0;               // Clear SSP interrupt flag
        SSP1CON2bits.RSEN = 1;              // Send restart bit
        while (SSP1CON2bits.RSEN);          // Wait until RSEN is cleared  
        //while (!PIR1bits.SS1PIF);         // Wait the interrupt flag is set
        #else
        //PIR1bits.SSPIF = 0;               // Clear SSP interrupt flag
        SSPCON2bits.RSEN = 1;               // Send restart bit
        while (SSPCON2bits.RSEN);           // Wait until RSEN is cleared  
        //while (!PIR1bits.SSPIF);          // Wait the interrupt flag is set
        #endif
    }            
    
    #if defined(__18f46j50) || defined(__18f46j53) || defined(__18f47j53)
    else if (module == I2C2)
    {
        //PIR3bits.SS2PIF = 0;               // Clear SSP interrupt flag
        SSP2CON2bits.RSEN = 1;                // Send stop bit
        while (SSP2CON2bits.RSEN);            // Wait until PEN is cleared 
        //while (!PIR3bits.SSP2IF);          // Wait the interrupt flag is set
    }
    #endif
}

/*  --------------------------------------------------------------------
    ---------- Send an Acknowledge (Ack) to the slave
    --------------------------------------------------------------------
    If the I2C module is active, this bit may not be set.
    NB: the ACKEN bit is automatically cleared
    ------------------------------------------------------------------*/

void I2C_sendAck(u8 module)
{
    I2C_idle(module);                   // Wait module is inactive
    
    if (module == I2C1)
    {
        #if defined(__16F1459)  || \
            defined(__18f25k50) || defined(__18f45k50) || \
            defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
        SSP1CON2bits.ACKDT = 0;             // We want an Ack
        #else
        SSPCON2bits.ACKDT = 0;              // We want an Ack
        #endif
        
        #if defined(__16F1459)  || \
            defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
        PIR1bits.SSP1IF = 0;                // Clear SSP interrupt flag
        #else
        PIR1bits.SSPIF = 0;                 // Clear SSP interrupt flag
        #endif
        
        #if defined(__16F1459)  || \
            defined(__18f25k50) || defined(__18f45k50) || \
            defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
        SSP1CON2bits.ACKEN = 1;             // Send it now
        #else
        SSPCON2bits.ACKEN = 1;              // Send it now
        #endif

        #if defined(__16F1459)  || \
            defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
        while (!PIR1bits.SSP1IF);           // Wait the interrupt flag is set
        #else
        while (!PIR1bits.SSPIF);            // Wait the interrupt flag is set
        #endif
    }
                
    #if defined(__18f46j50) || defined(__18f46j53) || defined(__18f47j53)
    else if (module == I2C2)
    {
        SSP2CON2bits.ACKDT = 0;             // We want an Ack
        PIR3bits.SSP2IF = 0;                // Clear SSP interrupt flag
        SSP2CON2bits.ACKEN = 1;             // Send it now
        while (!PIR3bits.SSP2IF);           // Wait the interrupt flag is set
    }
    #endif
}

/*  --------------------------------------------------------------------
    ---------- Send a Not Acknowledge (NAck) to the slave
    --------------------------------------------------------------------
    If the I2C module is active, this bit may not be set.
    NB: the ACKEN bit is automatically cleared
    ------------------------------------------------------------------*/

void I2C_sendNack(u8 module)
{
    I2C_idle(module);                   // Wait module is inactive

    if (module == I2C1)
    {
        #if defined(__16F1459)  || \
            defined(__18f25k50) || defined(__18f45k50) || \
            defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
        SSP1CON2bits.ACKDT = 1;             // We want a no Ack
        #else
        SSPCON2bits.ACKDT = 1;              // We want a no Ack
        #endif
        
        #if defined(__16F1459)  || \
            defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
        PIR1bits.SSP1IF = 0;                // Clear SSP interrupt flag
        #else
        PIR1bits.SSPIF = 0;                 // Clear SSP interrupt flag
        #endif
        
        #if defined(__16F1459)  || \
            defined(__18f25k50) || defined(__18f45k50) || \
            defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
        SSP1CON2bits.ACKEN = 1;             // Send it now
        #else
        SSPCON2bits.ACKEN = 1;              // Send it now
        #endif

        #if defined(__16F1459)  || \
            defined(__18f26j53) || defined(__18f46j53) || \
            defined(__18f27j53) || defined(__18f47j53)
        while (!PIR1bits.SSP1IF);           // Wait the interrupt flag is set
        #else
        while (!PIR1bits.SSPIF);            // Wait the interrupt flag is set
        #endif
    }            
    
    #if defined(__18f46j50) || defined(__18f46j53) || defined(__18f47j53)
    else if (module == I2C2)
    {
        SSP2CON2bits.ACKDT = 1;             // We want an Ack
        PIR3bits.SSP2IF = 0;                // Clear SSP interrupt flag
        SSP2CON2bits.ACKEN = 1;             // Send it now
        while (!PIR3bits.SSP2IF);           // Wait the interrupt flag is set
    }
    #endif
}

#endif // __I2C_C
