/*  --------------------------------------------------------------------
    This code is based on the Atmel Corporation Manchester
    Coding Basics Application Note.

    http://www.atmel.com/dyn/resources/prod_documents/doc9164.pdf
    ------------------------------------------------------------------*/

#ifndef __RF433MHZ_H
#define __RF433MHZ_H

//timer scaling factors for different transmission speeds
#define MAN_300     0
#define MAN_600     1
#define MAN_1200    2
#define MAN_2400    3
#define MAN_4800    4
#define MAN_9600    5
#define MAN_19200   6
#define MAN_38400   7

// added by caoxp@github
// 
// the sync pulse amount for transmitting and receiving.
// a pulse means : HI,LO   or  LO,HI   
// usually SYNC_PULSE_MAX >= SYNC_PULSE_DEF + 2
//         SYNC_PULSE_MIN <= SYNC_PULSE_DEF + 2
//  consider the pulses rising when starting transmitting.
//  SYNC_PULSE_MIN should be much less than SYNC_PULSE_DEF
//  all maximum of 255
#define SYNC_PULSE_MIN  1
#define SYNC_PULSE_DEF  3
#define SYNC_PULSE_MAX  5

//#define       SYNC_PULSE_MIN  10
//#define       SYNC_PULSE_DEF  14
//#define       SYNC_PULSE_MAX  16

//define to use 1 or 0 to sync
// when using 1 to sync, sending SYNC_PULSE_DEF 1's , and send a 0 to start data.
//                       and end the transimitting by three 1's
// when using 0 to sync, sending SYNC_PULSE_DEF 0's , and send a 1 to start data.
//                       and end the transimitting by three 0's
#define SYNC_BIT_VALUE      0


/*
    Signal timing, we take sample every 8 clock ticks
    
    ticks:   [0]-[8]--[16]-[24]-[32]-[40]-[48]-[56]-[64]-[72]-[80]-[88]-[96][104][112][120][128][136]
    samples: |----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|----|
    single:  |                    [--------|----------]
    double:  |                                         [-----------------|--------------------]
    signal:  |_____________________________                               ______________________
             |                             |_____________________________|

*/

//setup timing for receiver
#define MinCount        33  //pulse lower count limit on capture
#define MaxCount        65  //pulse higher count limit on capture
#define MinLongCount    66  //pulse lower count on double pulse
#define MaxLongCount    129 //pulse higher count on double pulse

//setup timing for transmitter
//= 48 * 1024 * 1000000 / 16000000Hz microseconds for speed factor 0 (300baud)
#define HALF_BIT_INTERVAL 3072

//it's common to zero terminate a string or to transmit small numbers
//involving a lot of zeroes those zeroes may be mistaken for training pattern,
//confusing the receiver and resulting high packet lost, 
//therefore we xor the data with random decoupling mask
#define DECOUPLING_MASK 0b11001010 

#define RX_MODE_PRE 0
#define RX_MODE_SYNC 1
#define RX_MODE_DATA 2
#define RX_MODE_MSG 3
#define RX_MODE_IDLE 4

#define TimeOutDefault -1 //the timeout in msec default blocks

// core functions
void RF433MHz_sendZero(void);
void RF433MHz_sendOne(void);
void RF433MHz_start(void);
void RF433MHz_end(void);

// set up transmission
void RF433MHz_init(u8, u16);

// print functions
void RF433MHz_printChar(u8);
void RF433MHz_writeBytes(const u8 *, u8);
void RF433MHz_print(const u8 *);
void RF433MHz_println(const u8 *);
void RF433MHz_printNumber(long, u8);
void RF433MHz_printFloat(float, u8);
void RF433MHz_printf(const u8 *, ...);

// transmit 16 bits of data
void RF433MHz_transmit(u16);

// transmit array of bytes
void RF433MHz_transmitArray(u8, u8 *);

// begin receiving 16 bits
void RF433MHz_beginReceive(void);

// begin receiving a byte array
void RF433MHz_beginReceiveBytes(u8, u8 *);

// true if a complete message is ready
u8 RF433MHz_receiveComplete(void);

// fetch the received message
u16 RF433MHz_getMessage(void);

// stop receiving data
void RF433MHz_stopReceive(void);

// Decode, encode functions
//u8 RF433MHz_decodeMessage(u16, u8, u8);
//u16 RF433MHz_encodeMessage(u8, u8);
void RF433MHz_addManchesterBit(u16 *, u8 *, u8 *, u8 *, u8);

//
typedef struct
{
    #ifdef RF433MHZTRANSMITTER
    u8  TxPin;
    u8  bauds;
    u16 half_bit_interval_us;
    #else
    u8  RxPin;
    t16 period;
    u8  rx_sample;
    u8  rx_last_sample;
    u8  rx_count;
    u8  rx_sync_count;
    u8  rx_mode;
    u16 rx_manBits;               //the received manchester 32 bits
    u8  rx_numMB;                 //the number of received manchester bits
    u8  rx_curByte;
    u8  rx_maxBytes;
    u8  rx_default_data[2];
    u8* rx_data;
    #endif
} RF433MHZ_t;

#endif // __RF433MHZ_H
