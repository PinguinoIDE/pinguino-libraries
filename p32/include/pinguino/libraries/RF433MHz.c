/*  --------------------------------------------------------------------
    This code is based on the Atmel Corporation Manchester
    Coding Basics Application Note.

    http://www.atmel.com/dyn/resources/prod_documents/doc9164.pdf

    Quotes from the application note:

    "Manchester coding states that there will always be a transition
    of the message signal at the mid-point of the data bit frame.
    What occurs at the bit edges depends on the state of the previous
    bit frame and does not always produce a transition.
    A logical '1' is defined as a mid-point transition from low to
    high and a '0' is a mid-point transition from high to low.

    We use Timing Based Manchester Decode.
    In this approach we will capture the time between each
    transition coming from the demodulation circuit."

    Timer 2 is used with
    Timer 1 is used with

    This code gives a basic data rate as 1200 bauds.
    In manchester encoding we send 1 0 for a data bit 0.
    We send 0 1 for a data bit 1. This ensures an average over time
    of a fixed DC level in the TX/RX.
    This is required by the ASK RF link system to ensure its correct
    operation. The data rate is then 600 bits/s.
    --------------------------------------------------------------------
    TODO:
    * use repairing codes perhabs ?
    * Cf. http://en.wikipedia.org/wiki/Hamming_code
      format of the message including checksum and ID
        
    [0][1][2][3][4][5][6][7][8][9][a][b][c][d][e][f]
    [        ID        ][ checksum ][                 data                 ]            
    checksum = ID xor data[7:4] xor data[3:0] xor 0b0011
    ------------------------------------------------------------------*/

#ifndef __RF433MHZ_C
#define __RF433MHZ_C

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
//#include <manchester.c>

// Printf
#ifdef RF433MHZPRINTF
    #include <printFormated.c>
#endif

// PrintFloat
#if defined(RF433MHZPRINTFLOAT)
    #include <printFloat.c>
#endif

// PrintNumber
#if defined(RF433MHZPRINTNUMBER) || defined(RF433MHZPRINTFLOAT)
    #include <printNumber.c>
#endif

#if defined(RF433MHZPRINTCHAR)   || \
    defined(RF433MHZPRINT)       || defined(RF433MHZPRINTLN)    || \
    defined(RF433MHZPRINTNUMBER) || defined(RF433MHZPRINTFLOAT) || \
    defined(RF433MHZPRINTCENTER)
    #ifndef RF433MHZTRANSMITTER
    #define RF433MHZTRANSMITTER
    #endif
#endif

#if defined(RF433MHZRECEIVER) && defined(RF433MHZTRANSMITTER)
#error "I CAN'T BE RECEIVER AND TRANSMITTER AT THE SAME TIME"
#endif

RF433MHZ_t RF433MHZ;

//----------------------------------------------------------------------
#ifdef RF433MHZTRANSMITTER
//----------------------------------------------------------------------

void RF433MHz_init(u8 pin, u8 bauds)
{
    // TX a digital pin as output
    RF433MHZ.TxPin = pin;
    pinmode(pin, OUTPUT); 

    // Baud rate is defined as the number of symbols sent or received per second
    RF433MHZ.bauds = bauds;
    
    // Half a bit time in us
    // 1s = 1.000.000 us
    RF433MHZ.half_bit_interval_us = 500000 / bauds;
}

void RF433MHz_sendZero(void)
{
    Delayus(RF433MHZ.half_bit_interval_us);
    digitalwrite(RF433MHZ.TxPin, HIGH);

    Delayus(RF433MHZ.half_bit_interval_us);
    digitalwrite(RF433MHZ.TxPin, LOW);
}

void RF433MHz_sendOne(void)
{
    Delayus(RF433MHZ.half_bit_interval_us);
    digitalwrite(RF433MHZ.TxPin, LOW);

    Delayus(RF433MHZ.half_bit_interval_us);
    digitalwrite(RF433MHZ.TxPin, HIGH);
}

/*
void RF433MHz_transmit(u16 data)
{
    u8 byteData[2];

    byteData[0] = data >> 8;
    byteData[1] = data & 0xFF;
    RF433MHz_transmitArray(2, byteData);
}
*/

/*  --------------------------------------------------------------------
    The 433.92 Mhz receivers have AGC, if no signal is present the gain
    will be set to its highest level.

    In this condition it will switch high to low at random intervals due
    to input noise. A CRO connected to the data line looks like 433.92
    is full of transmissions.

    Any ASK transmission method must first sent a capture signal of 101010........
    When the receiver has adjusted its AGC to the required level for the
    transmisssion the actual data transmission can occur.

    We send 14 0's 1010... It takes 1 to 3 10's for the receiver to
    adjust to the transmit level.

    The receiver waits until we have at least 10 10's and then a start
    pulse 01. The receiver is then operating correctly and we have
    locked onto the transmission.
    ------------------------------------------------------------------*/

void RF433MHz_write(u8 numBytes, u8 *data)
{
    u8 i, j, d;
    u16 mask;
            
    #if SYNC_BIT_VALUE

    for(i = 0; i < SYNC_PULSE_DEF; i++) //send capture pulses
        RF433MHz_sendOne(); //end of capture pulses
    RF433MHz_sendZero(); //start data pulse
    
    #else
    
    for(i = 0; i < SYNC_PULSE_DEF; i++) //send capture pulses
        RF433MHz_sendZero(); //end of capture pulses
    RF433MHz_sendOne(); //start data pulse
    
    #endif
 
    // Send the user data
    for (i = 0; i < numBytes; i++)
    {
        mask = 0x01; //mask to send bits
        d = data[i] ^ DECOUPLING_MASK;
        for (j = 0; j < 8; j++)
        {
            if ((d & mask) == 0)
                RF433MHz_sendZero();
            else
                RF433MHz_sendOne();
            mask <<= 1; //get next bit
        }
    }

    // Send 3 terminatings 0's to correctly terminate the previous bit
    // and to turn the transmitter off
    #if SYNC_BIT_VALUE

    RF433MHz_sendOne();
    RF433MHz_sendOne();
    RF433MHz_sendOne();

    #else

    RF433MHz_sendZero();
    RF433MHz_sendZero();
    RF433MHz_sendZero();

    #endif
}

#if defined(RF433MHZPRINTCHAR)   || \
    defined(RF433MHZPRINT)       || defined(RF433MHZPRINTLN)    || \
    defined(RF433MHZPRINTNUMBER) || defined(RF433MHZPRINTFLOAT) || \
    defined(RF433MHZPRINTCENTER)

void RF433MHz_printChar(u8 c)
{
    u8 i;
            
    #if SYNC_BIT_VALUE

    for(i = 0; i < SYNC_PULSE_DEF; i++) //send capture pulses
        RF433MHz_sendOne(); //end of capture pulses
    RF433MHz_sendZero(); //start data pulse
    
    #else
    
    for(i = 0; i < SYNC_PULSE_DEF; i++) //send capture pulses
        RF433MHz_sendZero(); //end of capture pulses
    RF433MHz_sendOne(); //start data pulse
    
    #endif
 
    // Send the user data
    c = c ^ DECOUPLING_MASK;
    for (i = 0; i < 8; i++)
    {
        if (c & (1 << i))
            RF433MHz_sendOne();
        else
            RF433MHz_sendZero();
    }

    // Send 3 terminatings 0's to correctly terminate the previous bit
    // and to turn the transmitter off
    #if SYNC_BIT_VALUE

    RF433MHz_sendOne();
    RF433MHz_sendOne();
    RF433MHz_sendOne();

    #else

    RF433MHz_sendZero();
    RF433MHz_sendZero();
    RF433MHz_sendZero();

    #endif
}
#endif

#if defined(RF433MHZPRINT)       || defined(RF433MHZPRINTLN)    || \
    defined(RF433MHZPRINTNUMBER) || defined(RF433MHZPRINTFLOAT) || \
    defined(RF433MHZPRINTCENTER)
void RF433MHz_print(const u8 *string)
{
    while (*string != 0)
        RF433MHz_printChar(*string++);
}
#endif

#if defined(RF433MHZPRINTLN)
void RF433MHz_println(const u8 *string)
{
    RF433MHz_print(string);
    RF433MHz_print((u8*)"\n\r");
}
#endif

#if defined(RF433MHZPRINTNUMBER) || defined(RF433MHZPRINTFLOAT)
void RF433MHz_printNumber(long value, u8 base)
{
    printNumber(RF433MHz_printChar2, value, base);
}
#endif

#if defined(RF433MHZPRINTFLOAT)
void RF433MHz_printFloat(float number, u8 digits)
{ 
    printFloat(RF433MHz_printChar2, number, digits);
}
#endif

#if defined(RF433MHZPRINTF)
void RF433MHz_printf(const u8 *fmt, ...)
{
    static u8 buffer[25];
    u8 *c=(char*)&buffer;
    va_list	args;

    va_start(args, fmt);
    psprintf2(buffer, fmt, args);
    va_end(args);

    while (*c)
        RF433MHz_printChar(*c++);
}
#endif

#endif // RF433MHZTRANSMITTER

//----------------------------------------------------------------------
#ifdef RF433MHZRECEIVER
//----------------------------------------------------------------------

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

void RF433MHz_beginReceiveBytes(u8 maxBytes, u8 *data)
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

void RF433MHz_interrupt()
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
            {
                RF433MHZ.rx_sample = rx_sample_1;
            }
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

#endif // __RF433MHZ_C
