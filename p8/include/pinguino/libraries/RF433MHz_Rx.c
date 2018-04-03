/*  ----------------------------------------------------------------------------
    FILE:           RF433MHz_Rx.c
    PROJECT:        Pinguino
    PURPOSE:        433MHz Wireless Receiver Modules library
    PROGRAMER:      Regis Blanchot <rblanchot@gmail.com>
    --------------------------------------------------------------------
    CHANGELOG:
    2018-01-31 - Régis Blanchot -   first release
    --------------------------------------------------------------------
    TODO:
    * use repairing codes perhabs ?
    * Cf. http://en.wikipedia.org/wiki/Hamming_code
      format of the message including checksum and ID
        
    [0][1][2][3][4][5][6][7][8][9][a][b][c][d][e][f]
    [        ID        ][ checksum ][                 data                 ]            
    checksum = ID xor data[7:4] xor data[3:0] xor 0b0011
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

#ifndef __RF433MHZ_RX_C
#define __RF433MHZ_RX_C

#define __RF433MHZRX__      // used in main.c to trig the RF433MHZ interrupt 
#define RF433MHZRECEIVER


#if defined(RF433MHZRECEIVER) && defined(RF433MHZTRANSMITTER)
#error "I CAN'T BE RECEIVER AND TRANSMITTER AT THE SAME TIME"
#endif

#ifndef __PIC32MX__
#include <compiler.h>
#include <digitalw.c>
#include <digitalr.c>
#include <digitalp.c>
#include <delayus.c>
#include <delayms.c>
#else
#include <digitalw.c>
#include <delay.c>
#endif

#include <typedef.h>
#include <macro.h>
#include <stdarg.h>
#include <RF433MHz.h>
#include <manchester.c>

RF433MHZ_t RF433MHZ;

void RF433MHz_init(u8 pin, u8 bauds)
{
    // RX a digital pin as input
    RF433MHZ.RxPin = pin;
    pinmode(pin, INPUT); 

    // Baud rate is defined as the number of symbols sent or received per second
    RF433MHZ.bauds = bauds;
    
    // The configuration register TMR1 is setup to generate an interrupt
    // period equal to twice the desired Manchester Encoded bit rate.
    // half_bit_interval_us = 500000 / bauds;
    // pb_interval_us = 1000000 / (_cpu_clock_ / 4);
    // nb_cycles = half_bit_interval_us / pb_interval_us; 
    // nb_cycles = (500000 / bauds) / (1000000 / (_cpu_clock_ / 4)); 
    // nb_cycles = (500000 / bauds) / (4000000 / _cpu_clock_); 
    // nb_cycles = (500000 * _cpu_clock_) / (bauds * 4000000); 
    //nb_cycles = _cpu_clock_ / (8 * bauds); 
    RF433MHZ.period.w = 0xFFFF - (_cpu_clock_ / (8*bauds));
    
    // Disable global interrupts
    noInterrupts();
    
    // Interrupt config.
    #if defined(__16F1459) || defined(__16F1708)

    T1CON = 0b00000001;         // T1_SOURCE_FOSCDIV4 | T1_PS_1_1;
    T1GCONbits.TMR1GE = 0;      // Ignore T1DIG effection 
    TMR1H = RF433MHZ.period.h8;
    TMR1L = RF433MHZ.period.l8;
    PIR1bits.TMR1IF = 0;
    PIE1bits.TMR1IE = 1;

    #else

    T0CON = 0b00001000;         // T0_OFF | T0_16BIT | T0_SOURCE_INT | T0_PS_OFF;
    TMR0H = RF433MHZ.period.h8;
    TMR0L = RF433MHZ.period.l8;
    INTCON2bits.TMR0IP = 1;     // INT_HIGH_PRIORITY;
    INTCONbits.TMR0IF  = 0;
    INTCONbits.TMR0IE  = 1;     // INT_ENABLE;
    T0CONbits.TMR0ON   = 1;

    #endif

    // Enable global interrupts
    interrupts();

    // Init
    RF433MHZ.rx_sample = 0;
    RF433MHZ.rx_last_sample = 0;
    RF433MHZ.rx_count = 0;
    RF433MHZ.rx_sync_count = 0;
    RF433MHZ.rx_mode = RX_MODE_IDLE;
    // The received manchester 32 bits
    RF433MHZ.rx_manBits = 0;
    // The number of received manchester bits
    RF433MHZ.rx_numMB = 0;
    RF433MHZ.rx_curByte = 0;
    RF433MHZ.rx_maxBytes = 2;
    RF433MHZ.rx_data = RF433MHZ.rx_default_data;
}

void RF433MHz_beginReceiveBytes(u8 *data, u8 maxBytes)
{
    RF433MHZ.rx_maxBytes = maxBytes;
    RF433MHZ.rx_data = data;
    RF433MHZ.rx_mode = RX_MODE_PRE;
}

void RF433MHz_beginReceive(void)
{
    RF433MHZ.rx_maxBytes = 2;
    RF433MHZ.rx_data = RF433MHZ.rx_default_data;
    RF433MHZ.rx_mode = RX_MODE_PRE;
}

u8 RF433MHz_receiveComplete(void)
{
    return (RF433MHZ.rx_mode == RX_MODE_MSG);
}

u16 RF433MHz_getMessage(void)
{
    return (((u16)RF433MHZ.rx_data[0]) << 8) | (u16)RF433MHZ.rx_data[1];
}

void RF433MHz_stopReceive(void)
{
    RF433MHZ.rx_mode = RX_MODE_IDLE;
}

void rf433mhz_interrupt()
{
    volatile u8 rx_sample_0=0;
    volatile u8 rx_sample_1=0;
    volatile u8 transition;
 
    #if defined(__16F1459) || defined(__16F1708)
    
    if (PIR1bits.TMR1IF)
    {
        PIR1bits.TMR1IF = 0;
        TMR1H = RF433MHZ.period.h8;
        TMR1L = RF433MHZ.period.l8;

    #else

    if (INTCONbits.TMR0IF)
    {
        INTCONbits.TMR0IF = 0;
        TMR0H = RF433MHZ.period.h8;//0xD1;
        TMR0L = RF433MHZ.period.l8;//0x1F;

    #endif

        if (RF433MHZ.rx_mode < RX_MODE_MSG) //receiving something
        {
            // Increment counter
            RF433MHZ.rx_count += 8;
            
            // Check for value change
            //rx_sample = digitalRead(RxPin);
            // caoxp@github, 
            // add filter.
            // sample twice, only the same means a change.
            rx_sample_0 = 0;
            rx_sample_1 = digitalread(RF433MHZ.RxPin);
            if (rx_sample_1 == rx_sample_0)
                RF433MHZ.rx_sample = rx_sample_1;
            rx_sample_0 = rx_sample_1;

            //check sample transition
            transition = (RF433MHZ.rx_sample != RF433MHZ.rx_last_sample);
        
            if (RF433MHZ.rx_mode == RX_MODE_PRE)
            {
                // Wait for first transition to HIGH
                if (transition && (RF433MHZ.rx_sample == 1))
                {
                    RF433MHZ.rx_count = 0;
                    RF433MHZ.rx_sync_count = 0;
                    RF433MHZ.rx_mode = RX_MODE_SYNC;
                }
            }
            else if (RF433MHZ.rx_mode == RX_MODE_SYNC)
            {
                // Initial sync block
                if (transition)
                {
                    if (( (RF433MHZ.rx_sync_count < (SYNC_PULSE_MIN * 2) ) || (RF433MHZ.rx_last_sample == 1) ) &&
                        ( (RF433MHZ.rx_count < MinCount) || (RF433MHZ.rx_count > MaxCount)))
                    {
                        // First 20 bits and all 1 bits are expected to be regular
                        // Transition was too slow/fast
                        RF433MHZ.rx_mode = RX_MODE_PRE;
                    }
                    else if ((RF433MHZ.rx_last_sample == 0) &&
                            ((RF433MHZ.rx_count < MinCount) || (RF433MHZ.rx_count > MaxLongCount)))
                    {
                        // 0 bits after the 20th bit are allowed to be a double bit
                        // Transition was too slow/fast
                        RF433MHZ.rx_mode = RX_MODE_PRE;
                    }
                    else
                    {
                        RF433MHZ.rx_sync_count++;
                        
                        if ((RF433MHZ.rx_last_sample == 0) &&
                           (RF433MHZ.rx_sync_count >= (SYNC_PULSE_MIN * 2) ) &&
                           (RF433MHZ.rx_count >= MinLongCount))
                        {
                            // We have seen at least 10 regular transitions
                            // Lock sequence ends with unencoded bits 01
                            // This is encoded and TX as HI,LO,LO,HI
                            // We have seen a long low - we are now locked!
                            RF433MHZ.rx_mode    = RX_MODE_DATA;
                            RF433MHZ.rx_manBits = 0;
                            RF433MHZ.rx_numMB   = 0;
                            RF433MHZ.rx_curByte = 0;
                        }
                        else if (RF433MHZ.rx_sync_count >= (SYNC_PULSE_MAX * 2) )
                        {
                            RF433MHZ.rx_mode = RX_MODE_PRE;
                        }
                        RF433MHZ.rx_count = 0;
                    }
                }
            }
            else if (RF433MHZ.rx_mode == RX_MODE_DATA)
            {
                // Receive data
                if (transition)
                {
                    if ((RF433MHZ.rx_count < MinCount) || (RF433MHZ.rx_count > MaxLongCount))
                    {
                        // wrong signal lenght, discard the message
                        RF433MHZ.rx_mode = RX_MODE_PRE;
                    }
                    else
                    {
                        if (RF433MHZ.rx_count >= MinLongCount) // was the previous bit a double bit?
                        {
                            RF433MHz_addManchesterBit(&RF433MHZ.rx_manBits, &RF433MHZ.rx_numMB, &RF433MHZ.rx_curByte, RF433MHZ.rx_data, RF433MHZ.rx_last_sample);
                        }
                        if ((RF433MHZ.rx_sample == 1) && (RF433MHZ.rx_curByte >= RF433MHZ.rx_maxBytes))
                        {
                            RF433MHZ.rx_mode = RX_MODE_MSG;
                        }
                        else
                        {
                            // Add the current bit
                            RF433MHz_addManchesterBit(&RF433MHZ.rx_manBits, &RF433MHZ.rx_numMB, &RF433MHZ.rx_curByte, RF433MHZ.rx_data, RF433MHZ.rx_sample);
                            RF433MHZ.rx_count = 0;
                        }
                    }
                }
            }
            
            // Get ready for next loop
            RF433MHZ.rx_last_sample = RF433MHZ.rx_sample;
        }
    }
}

#endif // RF433MHZRECEIVER

/*
//decode 8 bit payload and 4 bit ID from the message, return true if checksum is correct, otherwise false
u8 RF433MHz_decodeMessage(u16 m, u8 id, u8 data)
{
    u8 ch, ech;
    
    //extract components
    data = (m & 0xFF);
    id = (m >> 12);
    ch = (m >> 8) & 0b1111; //checksum received
    //calculate checksum
    ech = (id ^ data ^ (data >> 4) ^ 0b0011) & 0b1111; //checksum expected
    return ch == ech;
}

//encode 8 bit payload, 4 bit ID and 4 bit checksum into 16 bit
u16 RF433MHz_encodeMessage(u8 id, u8 data)
{
    u8 chsum = (id ^ data ^ (data >> 4) ^ 0b0011) & 0b1111;
    u16 m = ((id) << 12) | (chsum << 8) | (data);
    return m;
}
*/

// called from the interrupt routine
void RF433MHz_addManchesterBit(u16 *manBits, u8 *numMB, u8 *curByte, u8 *data, u8 newBit)
{
    u8 newData, i;
    
    // Makes space for the new bit
    (*manBits) <<= 1;
    // Add the new bit
    (*manBits) |= newBit;
    // Increment counter
    (*numMB)++;

    if (*numMB == 16)
    {
        newData = 0;
        for (i = 0; i < 8; i++)
        {
            // ManBits holds 16 bits of manchester data
            // 1 = LO,HI
            // 0 = HI,LO
            // We can decode each bit by looking at the bottom bit of each pair.
            newData <<= 1;
            newData |= (*manBits & 1); // store the one
            *manBits = *manBits >> 2; //get next data bit
        }
        data[*curByte] = newData ^ DECOUPLING_MASK;
        (*curByte)++;

        // added by caoxp @ https://github.com/caoxp
        // compatible with unfixed-length data, with the data length defined by the first byte.
        // at a maximum of 255 total data length.
        if ((*curByte) == 1)
            RF433MHZ.rx_maxBytes = data[0];
        
        *numMB = 0;
    }
}

#endif // __RF433MHZ_RX_C
