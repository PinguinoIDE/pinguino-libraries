/*
 * IRremote.h
 * Version 0.1 July, 2009
 * Copyright 2009 Ken Shirriff
 * For details, see http://arcfn.com/2009/08/multi-protocol-infrared-remote-library.htm http://arcfn.com
 * Edited by Mitra to add new controller SANYO
 *
 * Interrupt code based on NECIRrcv by Joe Knapp
 * http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1210243556
 * Also influenced by http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino/
 *
 * JVC and Panasonic protocol added by Kristian Lauszus (Thanks to zenwheel and other people at the original blog post)
 */

#ifndef IRREMOTE_H
#define IRREMOTE_H

// The following are compile-time library options.
// If you change them, recompile the library.
// If DEBUG is defined, a lot of debugging output will be printed during decoding.
// TEST must be defined for the IRtest unittests to work.  It will make some
// methods virtual, which will be slightly slower, which is why it is optional.
// #define DEBUG
// #define TEST

// Results returned from the decoder
typedef struct
{
	int decode_type; // NEC, SONY, RC5, UNKNOWN
	unsigned int panasonicAddress; // This is only used for decoding Panasonic data
	unsigned long value; // Decoded value
	int bits; // Number of bits in decoded value
	volatile unsigned int *rawbuf; // Raw intervals in .5 us ticks
	int rawlen; // Number of records in rawbuf.
} decode_results;//_t;

// Defined in IRremote.cpp
//extern volatile decode_results_t results;

// Values for decode_type
#define NEC 1
#define SONY 2
#define RC5 3
#define RC6 4
#define DISH 5
#define SHARP 6
#define PANASONIC 7
#define JVC 8
#define SANYO 9
#define MITSUBISHI 10
#define UNKNOWN -1

// Decoded value for NEC when a repeat code is received
#define REPEAT 0xffffffff

// main class for receiving IR
//void IRrecv_IRrecv(int recvpin);
//void IRrecv_init(int recvpin, int outpin);

void IRrecv_blink13(u8 blinkflag);
int IRrecv_decode(decode_results *results);
void IRrecv_enableIRIn(u8 recvpin);
void IRrecv_resume();

// These are called by decode
int IRrecv_getRClevel(decode_results *results, int *offset, int *used, int t1);
long IRrecv_decodeNEC(decode_results *results);
long IRrecv_decodeSony(decode_results *results);
long IRrecv_decodeSanyo(decode_results *results);
long IRrecv_decodeMitsubishi(decode_results *results);
long IRrecv_decodeRC5(decode_results *results);
long IRrecv_decodeRC6(decode_results *results);
long IRrecv_decodePanasonic(decode_results *results);
long IRrecv_decodeJVC(decode_results *results);
long IRrecv_decodeHash(decode_results *results);
int IRrecv_compare(unsigned int oldval, unsigned int newval);

// private:
void IRsend_enableIROut(u8 outpin);
void IRsend_mark(unsigned int usec);
void IRsend_space(unsigned int usec);

void IRsend_IRsend();
void IRsend_sendNEC(unsigned long data, int nbits);
void IRsend_sendSony(unsigned long data, int nbits);
// Neither Sanyo nor Mitsubishi send is implemented yet
//  void sendSanyo(unsigned long data, int nbits);
//  void sendMitsubishi(unsigned long data, int nbits);
void IRsend_sendRaw(unsigned int buf[], int len, int hz);
void IRsend_sendRC5(unsigned long data, int nbits);
void IRsend_sendRC6(unsigned long data, int nbits);
void IRsend_sendDISH(unsigned long data, int nbits);
void IRsend_sendSharp(unsigned long data, int nbits);
void IRsend_sendPanasonic(unsigned int address, unsigned long data);
void IRsend_sendJVC(unsigned long data, int nbits, int repeat); // *Note instead of sending the REPEAT constant if you want the JVC repeat signal sent, send the original code value and change the repeat argument from 0 to 1. JVC protocol repeats by skipping the header NOT by sending a separate code value like NEC does.


// Some useful constants

#define USECPERTICK 50  // microseconds per clock interrupt tick
#define RAWBUF 100 // Length of raw duration buffer

// Marks tend to be 100us too long, and spaces 100us too short
// when received due to sensor lag.
#define MARK_EXCESS 100

#endif /* IRREMOTE_H */
