/*
 * IRremote.c
 * Version 0.11 August, 2009
 * Copyright 2009 Ken Shirriff
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.html
 *
 * Modified by Paul Stoffregen <paul@pjrc.com> to support other boards and timers
 * Modified  by Mitra Ardron <mitra@mitra.biz> 
 * Added Sanyo and Mitsubishi controllers
 * Modified Sony to spot the repeat codes that some Sony's send
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 *
 * JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
 */

#include <IRremote.h>
#include <IRremoteInt.h>
#include <macro.h>
#include <interrupt.h>
#include <digitalw.c>
#include <delay.c>
#include <pwm.c>
#include <oscillator.c>

volatile unsigned int _t3_reload_val;   // Timer3 reload value
volatile irparams_t irparams;
//volatile decode_results_t results;

#define __IRREMOTE__        // used in main.c to init. the interrupt routine

// Macros
#define ENABLE_PWM    PWM_setPercentDutyCycle(irparams.outpin, 50) // duty cycle = 50% = Enable PWM output
#define DISABLE_PWM   PWM_setPercentDutyCycle(irparams.outpin, 0) // duty cycle = 0% = Disable PWM output

// These versions of MATCH, MATCH_MARK, and MATCH_SPACE are only for debugging.
// To use them, set DEBUG in IRremoteInt.h
// Normally macros are used for efficiency
#ifdef DEBUG
int MATCH(int measured, int desired) {
  serial_print("Testing: ");
  serial_print(TICKS_LOW(desired), DEC);
  serial_print(" <= ");
  serial_print(measured, DEC);
  serial_print(" <= ");
  serial_println(TICKS_HIGH(desired), DEC);
  return measured >= TICKS_LOW(desired) && measured <= TICKS_HIGH(desired);
}

int MATCH_MARK(int measured_ticks, int desired_us) {
  Serial_print("Testing mark ");
  Serial_print(measured_ticks * USECPERTICK, DEC);
  Serial_print(" vs ");
  Serial_print(desired_us, DEC);
  Serial_print(": ");
  Serial_print(TICKS_LOW(desired_us + MARK_EXCESS), DEC);
  Serial_print(" <= ");
  Serial_print(measured_ticks, DEC);
  Serial_print(" <= ");
  Serial_println(TICKS_HIGH(desired_us + MARK_EXCESS), DEC);
  return measured_ticks >= TICKS_LOW(desired_us + MARK_EXCESS) && measured_ticks <= TICKS_HIGH(desired_us + MARK_EXCESS);
}

int MATCH_SPACE(int measured_ticks, int desired_us) {
  Serial_print("Testing space ");
  Serial_print(measured_ticks * USECPERTICK, DEC);
  Serial_print(" vs ");
  Serial_print(desired_us, DEC);
  Serial_print(": ");
  Serial_print(TICKS_LOW(desired_us - MARK_EXCESS), DEC);
  Serial_print(" <= ");
  Serial_print(measured_ticks, DEC);
  Serial_print(" <= ");
  Serial_println(TICKS_HIGH(desired_us - MARK_EXCESS), DEC);
  return measured_ticks >= TICKS_LOW(desired_us - MARK_EXCESS) && measured_ticks <= TICKS_HIGH(desired_us - MARK_EXCESS);
}
#endif

void IRsend_sendNEC(unsigned long data, int nbits)
{
  int i;
  
  IRsend_enableIROut(38);
  IRsend_mark(NEC_HDR_MARK);
  IRsend_space(NEC_HDR_SPACE);
  
  for (i = 0; i < nbits; i++) {
    if (data & TOPBIT) {
      IRsend_mark(NEC_BIT_MARK);
      IRsend_space(NEC_ONE_SPACE);
    } 
    else {
      IRsend_mark(NEC_BIT_MARK);
      IRsend_space(NEC_ZERO_SPACE);
    }
    data <<= 1;
  }
  IRsend_mark(NEC_BIT_MARK);
  IRsend_space(0);
}

void IRsend_sendSony(unsigned long data, int nbits) {
  int i; 
  
  IRsend_enableIROut(40);
  IRsend_mark(SONY_HDR_MARK);
  IRsend_space(SONY_HDR_SPACE);
  data = data << (32 - nbits);

  for (i = 0; i < nbits; i++) {
    if (data & TOPBIT) {
      IRsend_mark(SONY_ONE_MARK);
      IRsend_space(SONY_HDR_SPACE);
    } 
    else {
      IRsend_mark(SONY_ZERO_MARK);
      IRsend_space(SONY_HDR_SPACE);
    }
    data <<= 1;
  }
}

void IRsend_sendRaw(unsigned int buf[], int len, int hz)
{
  int i;
  
  IRsend_enableIROut(hz);
  
  for (i = 0; i < len; i++) {
    if (i & 1) {
      IRsend_space(buf[i]);
    } 
    else {
      IRsend_mark(buf[i]);
    }
  }
  IRsend_space(0); // Just to be sure
}

// Note: first bit must be a one (start bit)
void IRsend_sendRC5(unsigned long data, int nbits)
{ 
  int i;
  
  IRsend_enableIROut(36);
  data = data << (32 - nbits);
  IRsend_mark(RC5_T1); // First start bit
  IRsend_space(RC5_T1); // Second start bit
  IRsend_mark(RC5_T1); // Second start bit
  
  for (i = 0; i < nbits; i++) {
    if (data & TOPBIT) {
      IRsend_space(RC5_T1); // 1 is space, then mark
      IRsend_mark(RC5_T1);
    } 
    else {
      IRsend_mark(RC5_T1);
      IRsend_space(RC5_T1);
    }
    data <<= 1;
  }
  IRsend_space(0); // Turn off at end
}

// Caller needs to take care of flipping the toggle bit
void IRsend_sendRC6(unsigned long data, int nbits)
{
  int t;
  int i;
  
  IRsend_enableIROut(36);
  data = data << (32 - nbits);
  IRsend_mark(RC6_HDR_MARK);
  IRsend_space(RC6_HDR_SPACE);
  IRsend_mark(RC6_T1); // start bit
  IRsend_space(RC6_T1);
  
  for (i = 0; i < nbits; i++) {
    if (i == 3) {
      // double-wide trailer bit
      t = 2 * RC6_T1;
    } 
    else {
      t = RC6_T1;
    }
    if (data & TOPBIT) {
      IRsend_mark(t);
      IRsend_space(t);
    } 
    else {
      IRsend_space(t);
      IRsend_mark(t);
    }

    data <<= 1;
  }
  IRsend_space(0); // Turn off at end
}
void IRsend_sendPanasonic(unsigned int address, unsigned long data) {
    int i;
	
	IRsend_enableIROut(35);
    IRsend_mark(PANASONIC_HDR_MARK);
    IRsend_space(PANASONIC_HDR_SPACE);
    
    for(i = 0; i<16; i++)
    {
        IRsend_mark(PANASONIC_BIT_MARK);
        if (address & 0x8000) {
            IRsend_space(PANASONIC_ONE_SPACE);
        } else {
            IRsend_space(PANASONIC_ZERO_SPACE);
        }
        address <<= 1;        
    }    
    for (i = 0; i < 32; i++) {
        IRsend_mark(PANASONIC_BIT_MARK);
        if (data & TOPBIT) {
            IRsend_space(PANASONIC_ONE_SPACE);
        } else {
            IRsend_space(PANASONIC_ZERO_SPACE);
        }
        data <<= 1;
    }
    IRsend_mark(PANASONIC_BIT_MARK);
    IRsend_space(0);
}
void IRsend_sendJVC(unsigned long data, int nbits, int repeat)
{
	int i;
	
    IRsend_enableIROut(38);
    data = data << (32 - nbits);
    if (!repeat){
        IRsend_mark(JVC_HDR_MARK);
        IRsend_space(JVC_HDR_SPACE); 
    }
	
    for (i = 0; i < nbits; i++) {
        if (data & TOPBIT) {
            IRsend_mark(JVC_BIT_MARK);
            IRsend_space(JVC_ONE_SPACE); 
        } 
        else {
            IRsend_mark(JVC_BIT_MARK);
            IRsend_space(JVC_ZERO_SPACE); 
        }
        data <<= 1;
    }
    IRsend_mark(JVC_BIT_MARK);
    IRsend_space(0);
}
void IRsend_mark(int time) {
  // Sends an IR mark for the specified number of microseconds.
  // The mark output is modulated at the PWM frequency.

  ENABLE_PWM; // Enable PWM output
  Delayus(time);
}

/* Leave pin off for time (given in microseconds) */
void IRsend_space(int time) {
  // Sends an IR space for the specified number of microseconds.
  // A space is no output, so the PWM output is disabled.

  DISABLE_PWM; // Disable PWM output
  Delayus(time);
}

//#define TIMER_DISABLE_INTR; //Timer2 Overflow Interrupt

void IRsend_enableIROut(int khz) {
  // Enables IR output.  The khz value controls the modulation frequency in kilohertz.
  // The IR output will be on pin 3 (OC2B).
  // This routine is designed for 36-40KHz; if you use it for other values, it's up to you
  // to make sure it gives reasonable results.  (Watch out for overflow / underflow / rounding.)
  // TIMER2 is used in phase-correct PWM mode, with OCR2A controlling the frequency and OCR2B
  // controlling the duty cycle.
  // There is no prescaling, so the output frequency is 16MHz / (2 * OCR2A)
  // To turn the output on and off, we leave the PWM running, but connect and disconnect the output pin.
  // A few hours staring at the ATmega documentation and this will all make sense.
  // See my Secrets of Arduino PWM at http://arcfn.com/2009/07/secrets-of-arduino-pwm.html for details.

  
  // Disable the Timer2 Interrupt (which is used for receiving IR)
  //TIMER_DISABLE_INTR; //Timer2 Overflow Interrupt
  
  //pinmode(TIMER_PWM_PIN, OUTPUT);
  //digitalwrite(TIMER_PWM_PIN, LOW); // When not sending PWM, we want it low
  
  // COM2A = 00: disconnect OC2A
  // COM2B = 00: disconnect OC2B; to send signal set to 10: OC2B non-inverted
  // WGM2 = 101: phase-correct PWM with OCRA as top
  // CS2 = 000: no prescaling
  // The top value for the timer.  The modulation frequency will be SYSCLOCK / 2 / OCR2A.
  //TIMER_CONFIG_KHZ(khz);

    PWM_setFrequency(1000*khz); // in Hz not kHz
}


//IRrecv_IRrecv(int recvpin)
void IRrecv_init(int recvpin, int outpin)
{
    irparams.recvpin = recvpin;
    irparams.outpin  = outpin;
    irparams.blinkflag = 0;
    IRrecv_enableIRIn();
}


// initialization
void IRrecv_enableIRIn()
{
/*
  cli();
  // setup pulse clock timer interrupt
  //Prescale /8 (16M/8 = 0.5 microseconds per tick)
  // Therefore, the timer interval can range from 0.5 to 128 microseconds
  // depending on the reset value (255 to 0)
  TIMER_CONFIG_NORMAL();

  //Timer2 Overflow Interrupt Enable
  TIMER_ENABLE_INTR;

  TIMER_RESET;

  sei();  // enable interrupts
*/

    // Configure Timer3 to overload every 50us

    // nb cycles for 1us
    _t3_reload_val = ( System_getPeripheralFrequency() / 1000 / 1000 );

    // nb cycles for 50us
    _t3_reload_val *= 50;
    _t3_reload_val = 0xFFFF - _t3_reload_val;
    T3CON = T3_OFF | T3_16BIT | T3_SYNC_EXT_OFF | T3_OSC_OFF | T3_PS_1_1 | T3_RUN_FROM_OSC;
    IPR2bits.TMR3IP = INT_LOW_PRIORITY;
    TMR3H = high8(_t3_reload_val);
    TMR3L =  low8(_t3_reload_val);
    PIR2bits.TMR3IF = 0;
    PIE2bits.TMR3IE = INT_ENABLE;
    INTCONbits.GIEH    = 1;   // Enable global HP interrupts
    INTCONbits.GIEL    = 1;   // Enable global LP interrupts

  // initialize state machine variables
  irparams.rcvstate = STATE_IDLE;
  irparams.rawlen = 0;

  // set pin modes
  pinmode(irparams.recvpin, INPUT);
}

// enable/disable blinking of USERLED on IR processing
void IRrecv_blink13(int blinkflag)
{
  irparams.blinkflag = blinkflag;
  if (blinkflag)
    pinmode(USERLED, OUTPUT);
}

// TIMER2 interrupt code to collect raw data.
// Widths of alternating SPACE, MARK are recorded in rawbuf.
// Recorded in ticks of 50 microseconds.
// rawlen counts the number of entries recorded so far.
// First entry is the SPACE between transmissions.
// As soon as a SPACE gets long, ready is set, state switches to IDLE, timing of SPACE continues.
// As soon as first MARK arrives, gap width is recorded, ready is cleared, and new logging starts

void irremote_interrupt() // called from main.c
{
    volatile uint8_t irdata;

    if (PIR2bits.TMR3IF) // Timer3 interrupt ?
    {
        //TIMER_RESET;
        TMR3H = high8(_t3_reload_val);
        TMR3L =  low8(_t3_reload_val);

        irdata = (uint8_t)digitalread(irparams.recvpin);

        irparams.timer++; // One more 50us tick
        if (irparams.rawlen >= RAWBUF) {
        // Buffer overflow
        irparams.rcvstate = STATE_STOP;
        }
        switch(irparams.rcvstate) {
        case STATE_IDLE: // In the middle of a gap
        if (irdata == MARK) {
          if (irparams.timer < GAP_TICKS) {
            // Not big enough to be a gap.
            irparams.timer = 0;
          } 
          else {
            // gap just ended, record duration and start recording transmission
            irparams.rawlen = 0;
            irparams.rawbuf[irparams.rawlen++] = irparams.timer;
            irparams.timer = 0;
            irparams.rcvstate = STATE_MARK;
          }
        }
        break;
        case STATE_MARK: // timing MARK
        if (irdata == SPACE) {   // MARK ended, record time
          irparams.rawbuf[irparams.rawlen++] = irparams.timer;
          irparams.timer = 0;
          irparams.rcvstate = STATE_SPACE;
        }
        break;
        case STATE_SPACE: // timing SPACE
        if (irdata == MARK) { // SPACE just ended, record it
          irparams.rawbuf[irparams.rawlen++] = irparams.timer;
          irparams.timer = 0;
          irparams.rcvstate = STATE_MARK;
        } 
        else { // SPACE
          if (irparams.timer > GAP_TICKS) {
            // big SPACE, indicates gap between codes
            // Mark current code as ready for processing
            // Switch to STOP
            // Don't reset timer; keep counting space width
            irparams.rcvstate = STATE_STOP;
          } 
        }
        break;
        case STATE_STOP: // waiting, measuring gap
        if (irdata == MARK) { // reset gap timer
          irparams.timer = 0;
        }
        break;
        }

        if (irparams.blinkflag) {
        if (irdata == MARK) {
          digitalwrite(USERLED, 1);  // turn USERLED on
        } 
        else {
          digitalwrite(USERLED, 0);  // turn USERLED off
        }
        }
        PIR2bits.TMR3IF = 0;
    }
}

void IRrecv_resume() {
  irparams.rcvstate = STATE_IDLE;
  irparams.rawlen = 0;
}

// Decodes the received IR message
// Returns 0 if no data ready, 1 if data ready.
// Results of decoding are stored in results
int IRrecv_decode(decode_results *results) {
  results->rawbuf = irparams.rawbuf;
  results->rawlen = irparams.rawlen;
  if (irparams.rcvstate != STATE_STOP) {
    return ERR;
  }
#ifdef DEBUG
  Serial_println("Attempting NEC decode");
#endif
  if (IRrecv_decodeNEC(results)) {
    return DECODED;
  }
#ifdef DEBUG
  Serial_println("Attempting Sony decode");
#endif
  if (IRrecv_decodeSony(results)) {
    return DECODED;
  }
#ifdef DEBUG
  Serial_println("Attempting Sanyo decode");
#endif
  if (IRrecv_decodeSanyo(results)) {
    return DECODED;
  }
#ifdef DEBUG
  Serial_println("Attempting Mitsubishi decode");
#endif
  if (IRrecv_decodeMitsubishi(results)) {
    return DECODED;
  }
#ifdef DEBUG
  Serial_println("Attempting RC5 decode");
#endif  
  if (IRrecv_decodeRC5(results)) {
    return DECODED;
  }
#ifdef DEBUG
  Serial_println("Attempting RC6 decode");
#endif 
  if (IRrecv_decodeRC6(results)) {
    return DECODED;
  }
#ifdef DEBUG
    Serial_println("Attempting Panasonic decode");
#endif 
    if (IRrecv_decodePanasonic(results)) {
        return DECODED;
    }
#ifdef DEBUG
    Serial_println("Attempting JVC decode");
#endif 
    if (IRrecv_decodeJVC(results)) {
        return DECODED;
    }
  // decodeHash returns a hash on any input.
  // Thus, it needs to be last in the list.
  // If you add any decodes, add them before this.
  if (IRrecv_decodeHash(results)) {
    return DECODED;
  }
  // Throw away and start over
  IRrecv_resume();
  return ERR;
}

// NECs have a repeat only 4 items long
long IRrecv_decodeNEC(decode_results *results) {
  long data = 0;
  int offset = 1; // Skip first space
  int i;
  // Initial mark
  if (!MATCH_MARK(results->rawbuf[offset], NEC_HDR_MARK)) {
    return ERR;
  }
  offset++;
  // Check for repeat
  if (irparams.rawlen == 4 &&
    MATCH_SPACE(results->rawbuf[offset], NEC_RPT_SPACE) &&
    MATCH_MARK(results->rawbuf[offset+1], NEC_BIT_MARK)) {
    results->bits = 0;
    results->value = REPEAT;
    results->decode_type = NEC;
    return DECODED;
  }
  if (irparams.rawlen < 2 * NEC_BITS + 4) {
    return ERR;
  }
  // Initial space  
  if (!MATCH_SPACE(results->rawbuf[offset], NEC_HDR_SPACE)) {
    return ERR;
  }
  offset++;
  
  for (i = 0; i < NEC_BITS; i++) {
    if (!MATCH_MARK(results->rawbuf[offset], NEC_BIT_MARK)) {
      return ERR;
    }
    offset++;
    if (MATCH_SPACE(results->rawbuf[offset], NEC_ONE_SPACE)) {
      data = (data << 1) | 1;
    } 
    else if (MATCH_SPACE(results->rawbuf[offset], NEC_ZERO_SPACE)) {
      data <<= 1;
    } 
    else {
      return ERR;
    }
    offset++;
  }
  // Success
  results->bits = NEC_BITS;
  results->value = data;
  results->decode_type = NEC;
  return DECODED;
}

long IRrecv_decodeSony(decode_results *results) {
  long data = 0;
  int offset = 0; // Dont skip first space, check its size

  if (irparams.rawlen < 2 * SONY_BITS + 2) {
    return ERR;
  }

  // Some Sony's deliver repeats fast after first
  // unfortunately can't spot difference from of repeat from two fast clicks
  if (results->rawbuf[offset] < SONY_DOUBLE_SPACE_USECS) {
    // Serial_print("IR Gap found: ");
    results->bits = 0;
    results->value = REPEAT;
    results->decode_type = SANYO;
    return DECODED;
  }
  offset++;

  // Initial mark
  if (!MATCH_MARK(results->rawbuf[offset], SONY_HDR_MARK)) {
    return ERR;
  }
  offset++;

  while (offset + 1 < irparams.rawlen) {
    if (!MATCH_SPACE(results->rawbuf[offset], SONY_HDR_SPACE)) {
      break;
    }
    offset++;
    if (MATCH_MARK(results->rawbuf[offset], SONY_ONE_MARK)) {
      data = (data << 1) | 1;
    } 
    else if (MATCH_MARK(results->rawbuf[offset], SONY_ZERO_MARK)) {
      data <<= 1;
    } 
    else {
      return ERR;
    }
    offset++;
  }

  // Success
  results->bits = (offset - 1) / 2;
  if (results->bits < 12) {
    results->bits = 0;
    return ERR;
  }
  results->value = data;
  results->decode_type = SONY;
  return DECODED;
}

// I think this is a Sanyo decoder - serial = SA 8650B
// Looks like Sony except for timings, 48 chars of data and time/space different
long IRrecv_decodeSanyo(decode_results *results) {
  long data = 0;
  int offset = 0; // Skip first space

  if (irparams.rawlen < 2 * SANYO_BITS + 2) {
    return ERR;
  }
  // Initial space  
  /* Put this back in for debugging - note can't use #DEBUG as if Debug on we don't see the repeat cos of the delay
  Serial_print("IR Gap: ");
  Serial_println( results->rawbuf[offset]);
  Serial_println( "test against:");
  Serial_println(results->rawbuf[offset]);
  */
  if (results->rawbuf[offset] < SANYO_DOUBLE_SPACE_USECS) {
    // Serial_print("IR Gap found: ");
    results->bits = 0;
    results->value = REPEAT;
    results->decode_type = SANYO;
    return DECODED;
  }
  offset++;

  // Initial mark
  if (!MATCH_MARK(results->rawbuf[offset], SANYO_HDR_MARK)) {
    return ERR;
  }
  offset++;

  // Skip Second Mark
  if (!MATCH_MARK(results->rawbuf[offset], SANYO_HDR_MARK)) {
    return ERR;
  }
  offset++;

  while (offset + 1 < irparams.rawlen) {
    if (!MATCH_SPACE(results->rawbuf[offset], SANYO_HDR_SPACE)) {
      break;
    }
    offset++;
    if (MATCH_MARK(results->rawbuf[offset], SANYO_ONE_MARK)) {
      data = (data << 1) | 1;
    } 
    else if (MATCH_MARK(results->rawbuf[offset], SANYO_ZERO_MARK)) {
      data <<= 1;
    } 
    else {
      return ERR;
    }
    offset++;
  }

  // Success
  results->bits = (offset - 1) / 2;
  if (results->bits < 12) {
    results->bits = 0;
    return ERR;
  }
  results->value = data;
  results->decode_type = SANYO;
  return DECODED;
}

// Looks like Sony except for timings, 48 chars of data and time/space different
long IRrecv_decodeMitsubishi(decode_results *results) {
  // Serial_print("?!? decoding Mitsubishi:");Serial.print(irparams.rawlen); Serial.print(" want "); Serial.println( 2 * MITSUBISHI_BITS + 2);
  long data = 0;
  int offset = 0; // Skip first space

  if (irparams.rawlen < 2 * MITSUBISHI_BITS + 2) {
    return ERR;
  }
  // Initial space  
  /* Put this back in for debugging - note can't use #DEBUG as if Debug on we don't see the repeat cos of the delay
  Serial_print("IR Gap: ");
  Serial_println( results->rawbuf[offset]);
  Serial_println( "test against:");
  Serial_println(results->rawbuf[offset]);
  */
  /* Not seeing double keys from Mitsubishi
  if (results->rawbuf[offset] < MITSUBISHI_DOUBLE_SPACE_USECS) {
    // Serial_print("IR Gap found: ");
    results->bits = 0;
    results->value = REPEAT;
    results->decode_type = MITSUBISHI;
    return DECODED;
  }
  */
  offset++;

  // Typical
  // 14200 7 41 7 42 7 42 7 17 7 17 7 18 7 41 7 18 7 17 7 17 7 18 7 41 8 17 7 17 7 18 7 17 7 

  // Initial Space
  if (!MATCH_MARK(results->rawbuf[offset], MITSUBISHI_HDR_SPACE)) {
    return ERR;
  }
  offset++;
  while (offset + 1 < irparams.rawlen) {
    if (MATCH_MARK(results->rawbuf[offset], MITSUBISHI_ONE_MARK)) {
      data = (data << 1) | 1;
    } 
    else if (MATCH_MARK(results->rawbuf[offset], MITSUBISHI_ZERO_MARK)) {
      data <<= 1;
    } 
    else {
      // Serial_println("A"); Serial.println(offset); Serial_println(results->rawbuf[offset]);
      return ERR;
    }
    offset++;
    if (!MATCH_SPACE(results->rawbuf[offset], MITSUBISHI_HDR_SPACE)) {
      // Serial_println("B"); Serial.println(offset); Serial_println(results->rawbuf[offset]);
      break;
    }
    offset++;
  }

  // Success
  results->bits = (offset - 1) / 2;
  if (results->bits < MITSUBISHI_BITS) {
    results->bits = 0;
    return ERR;
  }
  results->value = data;
  results->decode_type = MITSUBISHI;
  return DECODED;
}


// Gets one undecoded level at a time from the raw buffer.
// The RC5/6 decoding is easier if the data is broken into time intervals.
// E.g. if the buffer has MARK for 2 time intervals and SPACE for 1,
// successive calls to getRClevel will return MARK, MARK, SPACE.
// offset and used are updated to keep track of the current position.
// t1 is the time interval for a single bit in microseconds.
// Returns -1 for error (measured time interval is not a multiple of t1).
int IRrecv_getRClevel(decode_results *results, int *offset, int *used, int t1) {
  int width, val, correction, avail;

  if (*offset >= results->rawlen) {
    // After end of recorded buffer, assume SPACE.
    return SPACE;
  }
  width = results->rawbuf[*offset];
  val = ((*offset) % 2) ? MARK : SPACE;
  correction = (val == MARK) ? MARK_EXCESS : - MARK_EXCESS;

  if (MATCH(width, t1 + correction)) {
    avail = 1;
  } 
  else if (MATCH(width, 2*t1 + correction)) {
    avail = 2;
  } 
  else if (MATCH(width, 3*t1 + correction)) {
    avail = 3;
  } 
  else {
    return -1;
  }

  (*used)++;
  if (*used >= avail) {
    *used = 0;
    (*offset)++;
  }
#ifdef DEBUG
  if (val == MARK) {
    Serial_println("MARK");
  } 
  else {
    Serial_println("SPACE");
  }
#endif
  return val;   
}

long IRrecv_decodeRC5(decode_results *results) {
  int offset = 1; // Skip gap space
  long data = 0;
  int used = 0;
  int nbits;
  int levelA, levelB;

  if (irparams.rawlen < MIN_RC5_SAMPLES + 2) {
    return ERR;
  }
  // Get start bits
  if (IRrecv_getRClevel(results, &offset, &used, RC5_T1) != MARK) return ERR;
  if (IRrecv_getRClevel(results, &offset, &used, RC5_T1) != SPACE) return ERR;
  if (IRrecv_getRClevel(results, &offset, &used, RC5_T1) != MARK) return ERR;

  for (nbits = 0; offset < irparams.rawlen; nbits++) {
    levelA = IRrecv_getRClevel(results, &offset, &used, RC5_T1); 
    levelB = IRrecv_getRClevel(results, &offset, &used, RC5_T1);
    if (levelA == SPACE && levelB == MARK) {
      // 1 bit
      data = (data << 1) | 1;
    } 
    else if (levelA == MARK && levelB == SPACE) {
      // zero bit
      data <<= 1;
    } 
    else {
      return ERR;
    } 
  }

  // Success
  results->bits = nbits;
  results->value = data;
  results->decode_type = RC5;
  return DECODED;
}

long IRrecv_decodeRC6(decode_results *results) {
  int offset = 1; // Skip first space
  long data = 0;
  int used = 0;
  int nbits;
  int levelA, levelB; // Next two levels

  if (results->rawlen < MIN_RC6_SAMPLES) {
    return ERR;
  }
  // Initial mark
  if (!MATCH_MARK(results->rawbuf[offset], RC6_HDR_MARK)) {
    return ERR;
  }
  offset++;
  if (!MATCH_SPACE(results->rawbuf[offset], RC6_HDR_SPACE)) {
    return ERR;
  }
  offset++;
  // Get start bit (1)
  if (IRrecv_getRClevel(results, &offset, &used, RC6_T1) != MARK) return ERR;
  if (IRrecv_getRClevel(results, &offset, &used, RC6_T1) != SPACE) return ERR;

  for (nbits = 0; offset < results->rawlen; nbits++) {
    levelA = IRrecv_getRClevel(results, &offset, &used, RC6_T1); 
    if (nbits == 3) {
      // T bit is double wide; make sure second half matches
      if (levelA != IRrecv_getRClevel(results, &offset, &used, RC6_T1)) return ERR;
    } 
    levelB = IRrecv_getRClevel(results, &offset, &used, RC6_T1);
    if (nbits == 3) {
      // T bit is double wide; make sure second half matches
      if (levelB != IRrecv_getRClevel(results, &offset, &used, RC6_T1)) return ERR;
    } 
    if (levelA == MARK && levelB == SPACE) { // reversed compared to RC5
      // 1 bit
      data = (data << 1) | 1;
    } 
    else if (levelA == SPACE && levelB == MARK) {
      // zero bit
      data <<= 1;
    } 
    else {
      return ERR; // Error
    } 
  }
  // Success
  results->bits = nbits;
  results->value = data;
  results->decode_type = RC6;
  return DECODED;
}

long IRrecv_decodePanasonic(decode_results *results) {
    //unsigned long long data = 0; // PIC18F don't have 64-bit type
    unsigned long data = 0;
    int offset = 1;
    int i;
	
    if (!MATCH_MARK(results->rawbuf[offset], PANASONIC_HDR_MARK)) {
        return ERR;
    }
    offset++;
    if (!MATCH_MARK(results->rawbuf[offset], PANASONIC_HDR_SPACE)) {
        return ERR;
    }
    offset++;
    
    // decode address
    for (i = 0; i < PANASONIC_BITS; i++) {
        if (!MATCH_MARK(results->rawbuf[offset++], PANASONIC_BIT_MARK)) {
            return ERR;
        }
        if (MATCH_SPACE(results->rawbuf[offset],PANASONIC_ONE_SPACE)) {
            data = (data << 1) | 1;
        } else if (MATCH_SPACE(results->rawbuf[offset],PANASONIC_ZERO_SPACE)) {
            data <<= 1;
        } else {
            return ERR;
        }
        offset++;
    }
    results->value = (unsigned long)data;
    results->panasonicAddress = (unsigned int)(data >> 32);
    results->decode_type = PANASONIC;
    results->bits = PANASONIC_BITS;
    return DECODED;
}
long IRrecv_decodeJVC(decode_results *results) {
    long data = 0;
    int offset = 1; // Skip first space
    int i;
	// Check for repeat
    if (irparams.rawlen - 1 == 33 &&
        MATCH_MARK(results->rawbuf[offset], JVC_BIT_MARK) &&
        MATCH_MARK(results->rawbuf[irparams.rawlen-1], JVC_BIT_MARK)) {
        results->bits = 0;
        results->value = REPEAT;
        results->decode_type = JVC;
        return DECODED;
    } 
    // Initial mark
    if (!MATCH_MARK(results->rawbuf[offset], JVC_HDR_MARK)) {
        return ERR;
    }
    offset++; 
    if (irparams.rawlen < 2 * JVC_BITS + 1 ) {
        return ERR;
    }
    // Initial space 
    if (!MATCH_SPACE(results->rawbuf[offset], JVC_HDR_SPACE)) {
        return ERR;
    }
    offset++;
    for (i = 0; i < JVC_BITS; i++) {
        if (!MATCH_MARK(results->rawbuf[offset], JVC_BIT_MARK)) {
            return ERR;
        }
        offset++;
        if (MATCH_SPACE(results->rawbuf[offset], JVC_ONE_SPACE)) {
            data = (data << 1) | 1;
        } 
        else if (MATCH_SPACE(results->rawbuf[offset], JVC_ZERO_SPACE)) {
            data <<= 1;
        } 
        else {
            return ERR;
        }
        offset++;
    }
    //Stop bit
    if (!MATCH_MARK(results->rawbuf[offset], JVC_BIT_MARK)){
        return ERR;
    }
    // Success
    results->bits = JVC_BITS;
    results->value = data;
    results->decode_type = JVC;
    return DECODED;
}

/* -----------------------------------------------------------------------
 * hashdecode - decode an arbitrary IR code.
 * Instead of decoding using a standard encoding scheme
 * (e.g. Sony, NEC, RC5), the code is hashed to a 32-bit value.
 *
 * The algorithm: look at the sequence of MARK signals, and see if each one
 * is shorter (0), the same length (1), or longer (2) than the previous.
 * Do the same with the SPACE signals.  Hszh the resulting sequence of 0's,
 * 1's, and 2's to a 32-bit value.  This will give a unique value for each
 * different code (probably), for most code systems.
 *
 * http://arcfn.com/2010/01/using-arbitrary-remotes-with-arduino.html
 */

// Compare two tick values, returning 0 if newval is shorter,
// 1 if newval is equal, and 2 if newval is longer
// Use a tolerance of 20%
int IRrecv_compare(unsigned int oldval, unsigned int newval) {
  if (newval < oldval * .8) {
    return 0;
  } 
  else if (oldval < newval * .8) {
    return 2;
  } 
  else {
    return 1;
  }
}

// Use FNV hash algorithm: http://isthe.com/chongo/tech/comp/fnv/#FNV-param
#define FNV_PRIME_32 16777619
#define FNV_BASIS_32 2166136261

/* Converts the raw code values into a 32-bit hash code.
 * Hopefully this code is unique for each button.
 * This isn't a "real" decoding, just an arbitrary value.
 */
long IRrecv_decodeHash(decode_results *results) {
  int i;
  int value;
  long hash;
  
  // Require at least 6 samples to prevent triggering on noise
  if (results->rawlen < 6) {
    return ERR;
  }
  hash = FNV_BASIS_32;
  
  for (i = 1; i+2 < results->rawlen; i++) {
    value =  IRrecv_compare(results->rawbuf[i], results->rawbuf[i+2]);
    // Add value into the hash
    hash = (hash * FNV_PRIME_32) ^ value;
  }
  results->value = hash;
  results->bits = 32;
  results->decode_type = UNKNOWN;
  return DECODED;
}

/* Sharp and DISH support by Todd Treece ( http://unionbridge.org/design/ircommand )

The Dish send function needs to be repeated 4 times, and the Sharp function
has the necessary repeat built in because of the need to invert the signal.

Sharp protocol documentation:
http://www.sbprojects.com/knowledge/ir/sharp.htm

Here are the LIRC files that I found that seem to match the remote codes
from the oscilloscope:

Sharp LCD TV:
http://lirc.sourceforge.net/remotes/sharp/GA538WJSA

DISH NETWORK (echostar 301):
http://lirc.sourceforge.net/remotes/echostar/301_501_3100_5100_58xx_59xx

For the DISH codes, only send the last for characters of the hex.
i.e. use 0x1C10 instead of 0x0000000000001C10 which is listed in the
linked LIRC file.
*/

void IRsend_sendSharp(unsigned long data, int nbits) {
  unsigned long invertdata = data ^ SHARP_TOGGLE_MASK;
  int i;
  IRsend_enableIROut(38);
  
  for (i = 0; i < nbits; i++) {
    if (data & 0x4000) {
      IRsend_mark(SHARP_BIT_MARK);
      IRsend_space(SHARP_ONE_SPACE);
    }
    else {
      IRsend_mark(SHARP_BIT_MARK);
      IRsend_space(SHARP_ZERO_SPACE);
    }
    data <<= 1;
  }
  
  IRsend_mark(SHARP_BIT_MARK);
  IRsend_space(SHARP_ZERO_SPACE);
  Delayms(46);
  
  for (i = 0; i < nbits; i++) {
    if (invertdata & 0x4000) {
      IRsend_mark(SHARP_BIT_MARK);
      IRsend_space(SHARP_ONE_SPACE);
    }
    else {
      IRsend_mark(SHARP_BIT_MARK);
      IRsend_space(SHARP_ZERO_SPACE);
    }
    invertdata <<= 1;
  }
  IRsend_mark(SHARP_BIT_MARK);
  IRsend_space(SHARP_ZERO_SPACE);
  Delayms(46);
}

void IRsend_sendDISH(unsigned long data, int nbits)
{
  int i;
  IRsend_enableIROut(56);
  IRsend_mark(DISH_HDR_MARK);
  IRsend_space(DISH_HDR_SPACE);
  
  for (i = 0; i < nbits; i++) {
    if (data & DISH_TOP_BIT) {
      IRsend_mark(DISH_BIT_MARK);
      IRsend_space(DISH_ONE_SPACE);
    }
    else {
      IRsend_mark(DISH_BIT_MARK);
      IRsend_space(DISH_ZERO_SPACE);
    }
    data <<= 1;
  }
}
