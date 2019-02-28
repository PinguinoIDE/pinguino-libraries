/*  --------------------------------------------------------------------
    FILE:           i2c.c
    PROJECT:        pinguino32
    PURPOSE:        I2C functions
    PROGRAMER:      regis blanchot <rblanchot@gmail.com>
    --------------------------------------------------------------------
    CHANGELOG :
    --------------------------------------------------------------------
    04 Mar. 2011    Régis Blanchot - first release
    04/04/2012      RLmonitor      - tested on PIC32MX440 pinguino-OTG
                    added speed modes 100kHz, 400kHz, 1 MHz
                    changed completion flag polling to the various bits e.g. PEN, SEN
                    I2C_readChar 2nd argument - NACK or ACK
                    implemented init, start, restart, stop, writechar, readchar - don't need others
                    suggest 2 new functions - writebytes(module, address, *bytes, count),
                    readbytes(module, address, *bytes, count)
                    NO interrupt facility - not sure that it is needed?
    29/04/2014      Moreno Manzini - Create 2 versions of I2C_wait.
                    If definited I2CWAIT_WORKAROUND is used the second version which avoid potentially infinite loop.
                    I2CWAIT_WORKAROUND is defined as default.
    23/03/2015      Regis Blanchot - added I2C_master and I2C_slave functions
                    Added support to PIC32MX270F256B
    11/08/2015      Robert Teschner - added slave functions after Regis added
                    interrupt methods. Fixed PIC32MX220 freezing after interrupt enable.
    --------------------------------------------------------------------
    TODO : further slave modes improvement in case of slave writing
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

#include <p32xxxx.h>    // Always in first place to avoid conflict with const.h ON
#include <typedef.h>
#include <const.h>
#include <macro.h>
#include <i2c.h>
#include <system.c>
#include <interrupt.c>

/// GLOBALS

u8 gI2CMODE[2];

// 2 circular buffer for each bus
// @regis: Better to create a separate library for buffer with more functionality?
//         Circular buffer seems to create some bugs, if the overflow is detected.
volatile u8 gI2C1Buffer[I2C_BUFFER_LENGTH];
volatile u8 gI2C2Buffer[I2C_BUFFER_LENGTH];
volatile u8 I2C1wPtr = 0;    // I2C1 write index
volatile u8 I2C2wPtr = 0;    // I2C2 write index
volatile u8 I2C1rPtr = 0;    // I2C1 read index   
volatile u8 I2C2rPtr = 0;    // I2C2 read index  
    
/*	--------------------------------------------------------------------
    ---------- Set Master mode
    ------------------------------------------------------------------*/

void I2C_master(u8 module, u32 speed)   
{
    I2C_init(module, I2C_MASTER_MODE, speed);
}

/*	--------------------------------------------------------------------
    ---------- Slave functions
    ------------------------------------------------------------------*/

void I2C_slave(u8 module, u16 deviceID, u32 speed)   
{
    //Using a 7 bit slave address ?
    if ((deviceID >> 8) == 0)
        I2C1CONbits.A10M = 0;
    else
        I2C1CONbits.A10M = 1;

    I2C2ADD = deviceID;
    I2C2MSK = 0;
    
    I2C_init(module, I2C_SLAVE_MODE, speed);
}

/*	--------------------------------------------------------------------
    ---------- Open the I2C bus
    --------------------------------------------------------------------
    When the module is enabled it will assume control of the SDAx and SCLx pins.
    The module software need not be concerned with the state of the port I/O of the pins,
    the module overrides, the port state and direction.
    At initialization, the pins are tri-state (released).
    ex : I2C_init(I2C2, I2C_MASTER_MODE, I2C_400KHZ);
    ------------------------------------------------------------------*/

void I2C_init(u8 module, u8 mode, u32 speed)
{
    u32 pbclk = GetPeripheralClock();

    gI2CMODE[module] = mode;
    IntConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
    
    switch(module)
    {
    
        case I2C1:        

            IntSetVectorPriority(INT_I2C1_VECTOR, 7, 3);
            IntClearFlag(INT_I2C1_BUS_COLLISION_EVENT);  
            IntClearFlag(INT_I2C1_MASTER_EVENT);
            IntClearFlag(INT_I2C1_SLAVE_EVENT);     
                             
            switch (mode)
            {
                case I2C_MASTER_MODE:
                default:
                    I2C1BRG = (pbclk / (2*speed))-2; 
                    if ((speed >= I2C_400KHZ) && (speed < I2C_1MHZ))
                        I2C1CONbits.DISSLW = 0;    // Slew rate control enabled for High Speed mode
                    else
                        I2C1CONbits.DISSLW = 1;     // Slew rate control disabled for Standard Speed mode
                    
                    #if defined(__32MX220F032D__) || \
                    defined(__32MX220F032B__) || \
                    defined(__32MX250F128B__) || \
                    defined(__32MX270F256B__)
                    break;
                    
                    #else
                    //Enable interrupt    
                    IntEnable(INT_I2C1_MASTER_EVENT);
                    //IntEnable(INT_I2C1_BUS_COLLISION_EVENT);
                    break;
                    
                    #endif

                case I2C_MULTIMASTER_MODE:
                    // TODO
                    break;

                case I2C_SLAVE_MODE:
                    //Enable clock stretching
                    I2C1CONbits.STREN = 1;
                    
                    #if defined(__32MX220F032D__) || \
                    defined(__32MX220F032B__) || \
                    defined(__32MX250F128B__) || \
                    defined(__32MX270F256B__)
                    break;
                    
                    #else
                    //Enable interrupt
                    IntEnable(INT_I2C1_SLAVE_EVENT);
                    break;
                    
                    #endif
            }

            // Enables the I2C module and configures the SDA and SCL pins as serial port pins
            I2C1CONSET = (1 << 15);							// Set bit 15
            break;

        #if !defined(UBW32_460) && \
            !defined(UBW32_795) && \
            !defined(PIC32_PINGUINO_T795)
            
        case I2C2:

            IntSetVectorPriority(INT_I2C2_VECTOR, 7, 3);
            IntClearFlag(INT_I2C2_BUS_COLLISION_EVENT);
            IntClearFlag(INT_I2C2_MASTER_EVENT);
            IntClearFlag(INT_I2C2_SLAVE_EVENT);
            
            switch (mode)
            {
                case I2C_MASTER_MODE:
                default:
                    I2C2BRG = (pbclk / (2*speed))-2;
                    if ((speed >= I2C_400KHZ) && (speed < I2C_1MHZ))
                        I2C2CONbits.DISSLW = 0;    // Slew rate control enabled for High Speed mode
                    else
                        I2C2CONbits.DISSLW = 1;     // Slew rate control disabled for Standard Speed mode
                    
                    #if defined(__32MX220F032D__) || \
                    defined(__32MX220F032B__) || \
                    defined(__32MX250F128B__) || \
                    defined(__32MX270F256B__)
                    break;
                    
                    #else
                    //Enable interrupt
                    IntEnable(INT_I2C2_MASTER_EVENT);
                    break;
                    
                    #endif
                    
                case I2C_MULTIMASTER_MODE:
                    // TODO
                    break;

                case I2C_SLAVE_MODE:
                    //Enable clock stretching
                    I2C2CONbits.STREN = 1;
                    
                    #if defined(__32MX220F032D__) || \
                    defined(__32MX220F032B__) || \
                    defined(__32MX250F128B__) || \
                    defined(__32MX270F256B__)
                    break;
                    
                    #else
                    //Enable interrupt
                    IntEnable(INT_I2C2_SLAVE_EVENT);
                    break;
                    
                    #endif
            }

            // Enables the I2C module and configures the SDA and SCL pins as serial port pins
            //I2C2CONbits.ON = 1;
            //I2C2CON |= (1 << 15);
            I2C2CONSET = (1 << 15);							// Set bit 15
            break;
            
        #endif
    }
}

/*	--------------------------------------------------------------------
    ---------- Send byte and return ack bit
    --------------------------------------------------------------------
    Returns : 1 = OK, 0 = TIME_OUT
    TODO:
    * slave mode
    * timeout value calculation ?
    ------------------------------------------------------------------*/

u8 I2C_writeChar(u8 module, u8 value)
{
    u8 mode = gI2CMODE[module];
    
    switch(mode)
    {
        case I2C_MASTER_MODE:

            switch(module)
            {
                case I2C1:
                    // load the I2CxTRN with the data byte to transmit.
                    I2C1TRN = value;
                    // writing I2CxTRN sets the buffer full flag bit, TBF (I2CxSTAT<0>)
                    // and starts the data transmission

                    // wait for the completion of the transmit cycle including the ninth
                    // SCLx clock and setting of ACK/NACK by the slave indicated by the 
                    // module setting the I2CxMIF interrupt flag
                    I2C_wait(module);

                    return (!I2C1STATbits.ACKSTAT);

                #if !defined(UBW32_460) && \
                    !defined(UBW32_795) && \
                    !defined(PIC32_PINGUINO_T795)
                    
                case I2C2:
                    // load the I2CxTRN with the data byte to transmit.
                    I2C2TRN = value;

                    // wait for buffer empty
                    //while (I2C2STATbits.TBF == 1);
                    // writing I2CxTRN sets the buffer full flag bit, TBF (I2CxSTAT<0>)
                    // and starts the data transmission

                    // wait for the completion of the transmit cycle including the ninth
                    // SCLx clock and setting of ACK/NACK by the slave indicated by the 
                    // module setting the I2CxMIF interrupt flag
                    I2C_wait(module);

                    return (!I2C2STATbits.ACKSTAT);
                    
                #endif
            }

        case I2C_MULTIMASTER_MODE:
            // TODO
            break;

        case I2C_SLAVE_MODE:
            switch(module)
            {
                case I2C1:
                    
                    // wait receive buffer is empty
                    while (I2C1STATbits.RBF);
                    // try to write into buffer, checking collision
                    I2C1TRN = value;
//                    if (I2C1STATbits.IWCOL)
                    
                    // load the I2CxTRN with the data byte to transmit.
                    // writing I2CxTRN sets the buffer full flag bit, TBF (I2CxSTAT<0>)
                    // and starts the data transmission

                    // wait for the completion of the transmit cycle including the ninth
                    // SCLx clock and setting of ACK/NACK by the slave indicated by the 
                    // module setting the I2CxMIF interrupt flag
                    I2C_wait(module);

                    return (!I2C1STATbits.ACKSTAT);

                #if !defined(UBW32_460) && \
                    !defined(UBW32_795) && \
                    !defined(PIC32_PINGUINO_T795)
                    
                case I2C2:
                    // load the I2CxTRN with the data byte to transmit.
                    I2C2TRN = value;

                    // wait for buffer empty
                    //while (I2C2STATbits.TBF == 1);
                    // writing I2CxTRN sets the buffer full flag bit, TBF (I2CxSTAT<0>)
                    // and starts the data transmission

                    // wait for the completion of the transmit cycle including the ninth
                    // SCLx clock and setting of ACK/NACK by the slave indicated by the 
                    // module setting the I2CxMIF interrupt flag
                    I2C_wait(module);

                    return (!I2C2STATbits.ACKSTAT);
                    
                #endif
            }
    }
}

/*
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
*/

/*	--------------------------------------------------------------------
    ---------- Get a byte from the slave or the master
    --------------------------------------------------------------------
    Assumes write was successful and the peripheral is responding.
    In master mode, it up to the user to send NACK or ACK
    ------------------------------------------------------------------*/

u8 I2C_readChar(u8 module) //, u8 last)
{
    u8 mode = gI2CMODE[module];
    u8 value;
    
    switch(mode)
    {
        case I2C_MASTER_MODE:

            switch(module)
            {
                case I2C1:
                    // Writing the RCEN bit will start a master reception event.
                    I2C1CONbits.RCEN = 1;
                    // wait for the completion of receiving one byte indicated by the 
                    // module setting the I2CxMIF interrupt flag
                    I2C_wait(module);
                    
                    value = I2C1RCV;
                                            
                    //Master ACK or NACK
//                     I2C1CONbits.ACKDT = last?1:0;
//                     I2C1CONbits.ACKEN = 1;
                    // wait for the completion of sending ACK/NACK indicated by the 
                    // module setting the I2CxMIF interrupt flag
                    //I2C_wait(module);
                    return value;

                #if !defined(UBW32_460) && \
                    !defined(UBW32_795) && \
                    !defined(PIC32_PINGUINO_T795)
                    
                case I2C2:
                    // Writing the RCEN bit will start a master reception event.
                    I2C2CONbits.RCEN = 1;
                    // wait for the completion of receiving one byte indicated by the 
                    // module setting the I2CxMIF interrupt flag
                    I2C_wait(module);
                       
                    value = I2C2RCV;  
                                            
                    //Master ACK or NACK
//                     I2C2CONbits.ACKDT = last?1:0;
//                     I2C2CONbits.ACKEN = 1;
                    // wait for the completion of sending ACK/NACK indicated by the 
                    // module setting the I2CxMIF interrupt flag
                    //I2C_wait(module);
                    return value;

                #endif
            }

        case I2C_MULTIMASTER_MODE:
            // TODO
            break;

        case I2C_SLAVE_MODE:

            switch(module)
            {
                case I2C1:
                    while(!I2C1STATbits.RBF);
                    I2C1STATbits.I2COV = 0;
                    return(I2C1RCV);           
                    break;
                
                #if !defined(UBW32_460) && \
                    !defined(UBW32_795) && \
                    !defined(PIC32_PINGUINO_T795)
                    
                case I2C2:
                    while(!I2C2STATbits.RBF);
                    I2C2STATbits.I2COV = 0;
                    return(I2C2RCV);
                    break;

                #endif
            }
    }
}

/*
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

*/

/*	--------------------------------------------------------------------
    ---------- Get bytes from the slave or the master
    --------------------------------------------------------------------
    ------------------------------------------------------------------*/

u8 I2C_read(u8 module)
{
    u8 rxtmp;
    switch(module)
    {
        case I2C1:
            //if (I2C1Interrupt())
                rxtmp = gI2C1Buffer[I2C1rPtr];
            if (I2C1rPtr != I2C1wPtr)
                I2C1rPtr++;
            if (I2C1rPtr == I2C_BUFFER_LENGTH)
                I2C1rPtr = 0;    // Buffer overflow
            break;
    
        case I2C2:
            //if (I2C2Interrupt())
                rxtmp = gI2C2Buffer[I2C2rPtr];
            if (I2C2rPtr != I2C2wPtr)
                I2C2rPtr++;
            if (I2C2rPtr == I2C_BUFFER_LENGTH)
                I2C2rPtr = 0;    // Buffer overflow
            break;
    }
    return rxtmp;
}

/*	--------------------------------------------------------------------
    ---------- Send a byte to the slave
    --------------------------------------------------------------------
    RL	Would be better to have I2C_send(module, address, *byte, count) as 
    I2C peripherals are not always byte oriented. 
    ------------------------------------------------------------------*/

/*
u8 I2C_send(u8 module, u8 address, u8 value)
{
    u8 r;
    
    I2C_start(module);
    if ((r = I2C_writeChar(module, address | I2C_WRITE)))
        r = I2C_writeChar(module, value);
    I2C_stop(module);
    return r;
}
*/

/*	--------------------------------------------------------------------
    ---------- Get a byte from slave
    --------------------------------------------------------------------
    RL	Would be better to have I2C_get(module, address, *byte, count) as 
    I2C peripherals are not always byte oriented. 
    ------------------------------------------------------------------*/
/*
u8 I2C_get(u8 module, u16 adress)
{
    u8 value;

    I2C_sendID(module, adress, I2C_READ);
    I2C_restart(module);
    if (I2C_writeChar(module, adress) == 0)
        return (0);
    value = I2C_readChar(module, true);
    I2C_sendNack(module);
    I2C_stop(module);
    return (value);
}
*/

/*	--------------------------------------------------------------------
    ---------- Send the start condition, device address and r/w indication
    --------------------------------------------------------------------
    TODO:
        Sending a 10-bit device address involves sending 2 bytes to the slave. The first byte contains
        5 bits of the I2C device address reserved for 10-Bit Addressing modes and 2 bits of the 10-bit
        address. Because the next byte, which contains the remaining 8 bits of the 10-bit address, must
        be received by the slave, the R/W bit in the first byte must be ‘0’, indicating master transmission
        and slave reception. If the message data is also directed toward the slave, the master can con-
        tinue sending the data. However, if the master expects a reply from the slave, a Repeated Start
        sequence with the R/W bit at ‘1’ will change the R/W state of the message to a read of the slave.
    rw
        I2C_WRITE or 0
        I2C_READ or 1
    ----------------------------------------------------------------------------
    If the device is busy then it resends until accepted
    --------------------------------------------------------------------------*/
//deviceID should be device address<<1
//RL I don't use this function

/*
void I2C_sendID(u8 module, u16 deviceID, u8 rw)
{         
    u8 byte1, byte2;

    switch(module)
    {
        case I2C1:
            if (deviceID > 0x00FF)
            {         
                I2C1CONbits.A10M = 1;				// 1 = I2CxADD is a 10-bit slave address
                //byte1 = 
                //byte2 = 
                I2C_start(module);
                while (I2C_writeChar(module, deviceID | rw) != 1)
                    I2C_restart(module);
            }
            else
            {         
                I2C1CONbits.A10M = 0;				// 0 = I2CxADD is a 7-bit slave address
                I2C_start(module);
                while (I2C_writeChar(module, deviceID | rw) != 1)
                    I2C_restart(module);
            }
            break;

        #if !defined(UBW32_460) && \
            !defined(UBW32_795) && \
            !defined(PIC32_PINGUINO_T795)
            
        case I2C2:
            if (deviceID > 0x00FF)
            {         
                I2C2CONbits.A10M = 1;				// 1 = I2CxADD is a 10-bit slave address
                //byte1 = 
                //byte2 = 
                I2C_start(module);

                while (I2C_writeChar(module, deviceID | rw) != 1)
                    I2C_restart(module);
            }
            else
            {         
                I2C2CONbits.A10M = 0;				// 0 = I2CxADD is a 7-bit slave address
                I2C_start(module);
                while (I2C_writeChar(module, deviceID | rw) != 1)
                    I2C_restart(module);
            }
            break;
            
        #endif
    }
}
*/

/*	--------------------------------------------------------------------
    ---------- Wait for the module to finish its last action
    --------------------------------------------------------------------
    Master mode only
    the "i" loop is to prevent the program to freeze after 10-15 minutes
    ------------------------------------------------------------------*/

void I2C_wait(u8 module)
{
    u16 i = 5000;

    switch(module)
    {
        case I2C1:
        
            #if defined(__32MX220F032D__) || \
                defined(__32MX220F032B__) || \
                defined(__32MX250F128B__) || \
                defined(__32MX270F256B__)
                
            while (IFS1bits.I2C1MIF == 0 && i--);   // wait until interrupt request has a occurred
            IFS1bits.I2C1MIF = 0;                   // clear flag
            
            #else
            
            while (IFS0bits.I2C1MIF == 0 && i--);   // wait until interrupt request has a occurred
            IFS0bits.I2C1MIF = 0;                   // clear flag
            
            #endif
            break;

        #if !defined(UBW32_460) && \
            !defined(UBW32_795) && \
            !defined(PIC32_PINGUINO_T795)

        case I2C2:
            // __32MX220F032D__ or not, it's the same for all processors
            while (IFS1bits.I2C2MIF == 0 && i--);
            IFS1bits.I2C2MIF = 0;
            break;

        #endif
    }
}

/*	--------------------------------------------------------------------
    ---------- I2C start bit
    --------------------------------------------------------------------
    ------------------------------------------------------------------*/

void I2C_start(u8 module)
{
    switch(module)
    {
        case I2C1:
            // Set the Start Enabled bit, SEN, to initiate a Start event
            I2C1CONbits.SEN = 1;

            // wait for interrupt flag to be generated at end of Start condition
            I2C_wait(module);
            break;

        #if !defined(UBW32_460) && \
            !defined(UBW32_795) && \
            !defined(PIC32_PINGUINO_T795)
            
        case I2C2:
            // Set the Start Enabled bit, SEN, to initiate a Start event
            I2C2CONbits.SEN = 1;

            // wait for interrupt flag to be generated at end of Start condition
            I2C_wait(module);
            break;

        #endif
    }
}

/*	--------------------------------------------------------------------
    ---------- I2C stop bit
    ------------------------------------------------------------------*/

void I2C_stop(u8 module)
{
    switch(module)
    {
        case I2C1:
            // Set the Stop Enabled bit, PEN, to generate a master Stop sequence
            I2C1CONbits.PEN = 1;

            // wait for interrupt flag to be generated at end of Stop sequence
            I2C_wait(module);
            break;

        #if !defined(UBW32_460) && \
            !defined(UBW32_795) && \
            !defined(PIC32_PINGUINO_T795)

        case I2C2:
            // Set the Stop Enabled bit, PEN, to generate a master Stop sequence
            I2C2CONbits.PEN = 1;

            // wait for interrupt flag to be generated at end of Stop sequence
            I2C_wait(module);
            break;

        #endif
    }
}

/*	--------------------------------------------------------------------
    ---------- I2C restart bit
    ------------------------------------------------------------------*/

void I2C_restart(u8 module)
{
    switch(module)
    {
        case I2C1:
            // Set the Repeated Start Enabled bit, RSEN, to generate a master 
            // Repeated Start sequence
            I2C1CONbits.RSEN=1;

            // wait for interrupt flag to be generated at end of Repeated Start
            // sequence
            I2C_wait(module);
            break;

        #if !defined(UBW32_460) && \
            !defined(UBW32_795) && \
            !defined(PIC32_PINGUINO_T795)

        case I2C2:
            // Set the Repeated Start Enabled bit, RSEN, to generate a master 
            // Repeated Start sequence
            I2C2CONbits.RSEN=1;

            // wait for interrupt flag to be generated at end of Repeated Start
            // sequence
            I2C_wait(module);
            break;

        #endif
    }
}

/*	--------------------------------------------------------------------
    ---------- Send a Not Acknowledge (NAck) to the slave
    ------------------------------------------------------------------*/

void I2C_sendNack(u8 module)
{
    switch(module)
    {
        case I2C1:
            I2C1CONbits.ACKDT = 1;
            I2C1CONbits.ACKEN = 1;
            I2C_wait(module);
            break;

        #if !defined(UBW32_460) && \
            !defined(UBW32_795) && \
            !defined(PIC32_PINGUINO_T795)

        case I2C2:
            I2C2CONbits.ACKDT = 1;
            I2C2CONbits.ACKEN = 1;
            I2C_wait(module);
            break;

        #endif
    }
}

/*	--------------------------------------------------------------------
    ---------- Send an Acknowledge (Ack) to the slave
    ------------------------------------------------------------------*/

void I2C_sendAck(u8 module)
{
    switch(module)
    {
        case I2C1:
            I2C1CONbits.ACKDT = 0;
            I2C1CONbits.ACKEN = 1;
            I2C_wait(module);
            break;
            
        #if !defined(UBW32_460) && \
            !defined(UBW32_795) && \
            !defined(PIC32_PINGUINO_T795)

        case I2C2:
            I2C2CONbits.ACKDT = 0;
            I2C2CONbits.ACKEN = 1;
            I2C_wait(module);
            break;

        #endif
    }
}

/*	--------------------------------------------------------------------
    ---------- Interrupt routines
    --------------------------------------------------------------------
    Check for MASTER, SLAVE and Bus events and respond accordingly

    The I2C slave is implemented as a state machine. This is based on
    application note AN734. There are different states based on the
    I2C status flags. The states are:
    State1: Master write operation, last byte sent was an Address byte
    State2: Master write operation, last byte sent was a Data byte
    State3: Master read operation, last byte sent was an Address byte
    State4: Master read operation, last byte sent was a Data byte
    State5: Master sends a NACK
    ------------------------------------------------------------------*/

u8 I2C1Interrupt()
{
    u8 temp;
    u8 dIndex1;
    
    //Usually 0, but indicates when new value has been written to the buffer.
    u8 newValInBuf = 0;
    
    if (IntGetFlag(INT_I2C1_MASTER_EVENT))
    {
        //TODO
        IntClearFlag(INT_I2C1_MASTER_EVENT);
        return newValInBuf;
    }

    if (IntGetFlag(INT_I2C1_BUS_COLLISION_EVENT))
    {
        //TODO
        IntClearFlag(INT_I2C1_BUS_COLLISION_EVENT);
        return newValInBuf;
    }
    
    // When the master is writing the slave stores the byte.
    // When the master is reading the slave sends back the last stored byte.
    
    if (IntGetFlag(INT_I2C1_SLAVE_EVENT))
    {
        // R/W bit = 0 --> indicates data transfer is input to slave
        // D/A bit = 0 --> indicates last byte was address  
        if ((I2C1STATbits.R_W == 0) && (I2C1STATbits.D_A == 0))
        {
            // reset any state variables needed by a message sequence
            // perform a dummy read of the address
            temp = I2C_readChar(I2C1);
            
            // release the clock to restart I2C
            I2C1CONbits.SCLREL = 1; // release the clock
        }

        // R/W bit = 0 --> indicates data transfer is input to slave
        // D/A bit = 1 --> indicates last byte was data
        else if ((I2C1STATbits.R_W == 0) && (I2C1STATbits.D_A == 1))
        {
            // writing data to our module
            gI2C1Buffer[I2C1wPtr] = I2C_readChar(I2C1);   
            if (I2C1wPtr < I2C_BUFFER_LENGTH-1)
                I2C1wPtr++;
            else
                I2C1wPtr = 0;    // Buffer overflow

            //Indicate, that new value has been written to the buffer.
            newValInBuf = 1;
            
            // release the clock to restart I2C
            I2C1CONbits.SCLREL = 1; // release clock stretch bit
        }

        // R/W bit = 1 --> indicates data transfer is output from slave
        // D/A bit = 0 --> indicates last byte was address
        else if ((I2C1STATbits.R_W == 1) && (I2C1STATbits.D_A == 0))
        {
            // read of the slave device, read the address 
            temp = I2C_readChar(I2C1);
            dIndex1 = 0;
            //I2C_writeChar(I2C1, dataRead);
        }
        
        // R/W bit = 1 --> indicates data transfer is output from slave
        // D/A bit = 1 --> indicates last byte was data
        else if ((I2C1STATbits.R_W == 1) && (I2C1STATbits.D_A == 1))
        {
            // output the data until the MASTER terminates the
            // transfer with a NACK, continuing reads return 0
            
            //TODO, following code might be wrong.
            
            if (dIndex1 < I2C_BUFFER_LENGTH-1)
            {
                I2C_writeChar(I2C1, gI2C1Buffer[dIndex1]);
                dIndex1++;
            }
        }
        
        // finally clear the slave interrupt flag
        IntClearFlag(INT_I2C1_SLAVE_EVENT);
        return newValInBuf;
    }
}

u8 I2C2Interrupt()
{
    u8 temp;
    u8 dIndex2;
    
    //Usually 0, but indicates when new value has been written to the buffer.
    u8 newValInBuf = 0;

    if (IntGetFlag(INT_I2C2_MASTER_EVENT))
    {
        IntClearFlag(INT_I2C2_MASTER_EVENT);
        return newValInBuf;
    }

    if (IntGetFlag(INT_I2C2_BUS_COLLISION_EVENT))
    {
        IntClearFlag(INT_I2C2_BUS_COLLISION_EVENT);
        return newValInBuf;
    }
    
    // When the master is writing the slave stores the byte.
    // When the master is reading the slave sends back the last stored byte.
    
    if (IntGetFlag(INT_I2C2_SLAVE_EVENT))
    {
        // R/W bit = 0 --> indicates data transfer is input to slave
        // D/A bit = 0 --> indicates last byte was address  
        if ((I2C2STATbits.R_W == 0) && (I2C2STATbits.D_A == 0))
        {
            // reset any state variables needed by a message sequence
            // perform a temp read of the address
            temp = I2C_readChar(I2C2);
            // release the clock to restart I2C
            I2C2CONbits.SCLREL = 1; // release the clock
        }

        // R/W bit = 0 --> indicates data transfer is input to slave
        // D/A bit = 1 --> indicates last byte was data
        else if ((I2C2STATbits.R_W == 0) && (I2C2STATbits.D_A == 1))
        {
            // writing data to our module
            gI2C2Buffer[I2C2wPtr] = I2C_readChar(I2C2);   
            if (I2C2wPtr < I2C_BUFFER_LENGTH-1)
                I2C2wPtr++;
            else
                I2C2wPtr = 0;    // Buffer overflow
            
            //Indicate, that new value has been written to the buffer.
            newValInBuf = 1;
            
            // release the clock to restart I2C
            I2C2CONbits.SCLREL = 1; // release clock stretch bit
        }

        // R/W bit = 1 --> indicates data transfer is output from slave
        // D/A bit = 0 --> indicates last byte was address
        else if ((I2C2STATbits.R_W == 1) && (I2C2STATbits.D_A == 0))
        {
            // read of the slave device, read the address 
            temp = I2C_readChar(I2C2);
            dIndex2 = 0;
            //I2C_writeChar(I2C2, dataRead);
        }
        
        // R/W bit = 1 --> indicates data transfer is output from slave
        // D/A bit = 1 --> indicates last byte was data
        else if ((I2C2STATbits.R_W == 1) && (I2C2STATbits.D_A == 1))
        {
            // output the data until the MASTER terminates the
            // transfer with a NACK, continuing reads return 0
            
            //TODO, following code might be wrong.
            
            if (dIndex2 < I2C_BUFFER_LENGTH-1)
            {
                I2C_writeChar(I2C2, gI2C2Buffer[dIndex2]);
                dIndex2++;
            }
        }
        
        // finally clear the slave interrupt flag
        IntClearFlag(INT_I2C2_SLAVE_EVENT);
        return newValInBuf;
    }
}

#endif	/* __I2C_C */
