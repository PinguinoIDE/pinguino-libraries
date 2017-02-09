/*	--------------------------------------------------------------------
    FILE:           i2cmaster.c
    PROJECT:        Pinguino
    PURPOSE:        Functions to handle I2C communication by Slave
    PROGRAMMER:     A. GENTRIC
    --------------------------------------------------------------------
    CHANGELOG:
    20 Jan. 2017    R. Blanchot - added function prototypes
    25 Nov. 2016    R. Blanchot - moved defines in i2c.h
    01 Apr. 2013    A. GENTRIC  - first release
    --------------------------------------------------------------------
    TODO:
    * i2c.c core functions need i2c module
    --------------------------------------------------------------------
    Adapted from Nicholas Zambetti Todd Krein's programs 
    TWI/I2C library for Wiring & Arduino
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
    ----------------------------------------------------------------- */

#ifndef __I2CMASTER_C
#define __I2CMASTER_C

#include <stdlib.h>
#include <i2c.h>

// i2c buffer length
#ifndef _I2CBUFFERLENGTH_
#define _I2CBUFFERLENGTH_ 32
#endif

// Prototypes
u8 I2C_readFrom(u8, u8 ,u8* , u8 , u8 );
u8 I2C_writeTo(u8, u8 ,u8* , u8 , u8, u8 );
u8 I2C_transmit(u8, const u8*, u8);
void I2C_begin(u8, u8, u16);
u8 I2C_requestFrom(u8, u8, u8);
void I2C_beginTransmission(u8, u8);
u8 I2C_endTransmission(u8, u8);
u8 I2C_inBuffer(u8, u8);
u8 I2C_inBufferStr(u8, u8 *);
u8 I2C_inBufferS(u8 module, u8 *data, u8 quantity);
u8 I2C_available(u8 module);
u8 I2C_outBuffer(u8 module);
int peek(u8 module);

#define I2C1_begin(m, a, s)         I2C_begin(I2C1, a, s)
#define I2C1_inBuffer(m, d)         I2C_inBuffer(I2C1, d)
#define I2C1_inBufferS(m, s, l)     I2C_inBufferS(I2C1, s, l)
#define I2C1_inBufferStr(m, s)      I2C_inBufferStr(I2C1, s)
#define I2C1_outBuffer(m)           I2C_outBuffer(I2C1)
#define I2C1_outBuffer(m)           I2C_outBuffer(I2C1)
#define I2C1_beginTransmission(m,a) I2C_beginTransmission(I2C1, a)
#define I2C1_endTransmission(m, s)  I2C_endTransmission(I2C1, s)
#define I2C1_available(m)           I2C_available(I2C1)
#define I2C1_requestFrom(m, a, l)   I2C_requestFrom(I2C1, a, l)
#define I2C1_onRequest(f)           I2C_onRequest(I2C1, f)
#define I2C1_onReceive(f)           I2C_onReceive(I2C1, f)

#define I2C2_begin(m, a, s)         I2C_begin(I2C2, a, s)
#define I2C2_inBuffer(m, d)         I2C_inBuffer(I2C2, d)
#define I2C2_inBufferS(m, s, l)     I2C_inBufferS(I2C2, s, l)
#define I2C2_inBufferStr(m, s)      I2C_inBufferStr(I2C2, s)
#define I2C2_outBuffer(m)           I2C_outBuffer(I2C2)
#define I2C2_outBuffer(m)           I2C_outBuffer(I2C2)
#define I2C2_beginTransmission(m,a) I2C_beginTransmission(I2C2, a)
#define I2C2_endTransmission(m, s)  I2C_endTransmission(I2C2, s)
#define I2C2_available(m)           I2C_available(I2C2)
#define I2C2_requestFrom(m, a, l)   I2C_requestFrom(I2C2, a, l)
#define I2C2_onRequest(f)           I2C_onRequest(I2C2, f)
#define I2C2_onReceive(f)           I2C_onReceive(I2C2, f)

#define Wire_begin(m, a, s)         I2C_begin(m, a, s)
#define Wire_writeChar(m, d)        I2C_inBuffer(m, d)
#define Wire_writeCharS(m, s, l)    I2C_inBufferS(m, s, l)
#define Wire_writeCharStr(m, s)     I2C_inBufferStr(m, s)
#define Wire_readChar(m)            I2C_outBuffer(m)
#define Wire_receive(m)             I2C_outBuffer(m)
#define Wire_beginTransmission(m,a) I2C_beginTransmission(m, a)
#define Wire_endTransmission(m, s)  I2C_endTransmission(m, s)
#define Wire_available(m)           I2C_available(m)
#define Wire_requestFrom(m, a, l)   I2C_requestFrom(m, a, l)
#define Wire_onRequest(m, f)        I2C_onRequest(m, f)
#define Wire_onReceive(m, f)        I2C_onReceive(m, f)

static void I2C_onRequest (void(*func)(void));
static void I2C_onReceive (void(*func)( u8));
static void (*_i2c_onRequest_function)(void);
static void (*_i2c_onReceive_function)( u8);

static volatile u8 i2c_state;
static volatile u8 i2c_slarw;
static volatile u8 i2c_sendStop;			// should the transaction end with a stop
static volatile u8 i2c_inRepStart;			// in the middle of a repeated start
static volatile u8 i2c_error;

static volatile u8 _i2c_txBuffer[_I2CBUFFERLENGTH_];              // i2c tx buffer
static volatile u8 _i2c_rxBuffer[_I2CBUFFERLENGTH_];              // i2c rx buffer
static volatile u8 _i2c_rxBufferIndex = 0;
static volatile u8 _i2c_rxBufferLength = 0;

static volatile u8 _i2c_txAddress = 0;
static volatile u8 _i2c_txBufferIndex = 0;
static volatile u8 _i2c_txBufferLength = 0;
static volatile u8 _i2c_transmitting = 0;

static volatile u8 _i2c_masterBuffer[_I2CBUFFERLENGTH_];
static volatile u8 _i2c_masterBufferIndex;
static volatile u8 _i2c_masterBufferLength;

#ifdef I2CINT
#include <i2cslave.c>
#endif

/* 
 * Function I2C_readFrom
 * Desc     attempts to become I2C bus master and read a
 *          series of bytes from a device on the bus
 * Input    address: 8bit i2c device address
 *          data: pointer to byte array
 *          length: number of bytes to read into array
 *          sendStop: indicating whether to send a stop at the end
 * Output   number of bytes read
 */
u8 I2C_readFrom(u8 module, u8 address, u8* data, u8 length, u8 sendStop)
{
    u8 i;
    u8 resp_sla;

    // ensure data will fit into buffer
    if( length > _I2CBUFFERLENGTH_)
        return 0;

    // wait until twi is ready, become master receiver
    while(i2c_state != I2C_READY)
        continue;

    i2c_state = I2C_MRX;// Master Receiver
    i2c_sendStop = sendStop;
    // reset error state (0xFF.. no error occured)
    i2c_error = 0xFF;

    // initialize buffer iteration vars
    _i2c_masterBufferIndex = 0;
    _i2c_masterBufferLength = length;  // This is not intuitive, read on...
    // On receive, the previously configured ACK/NACK setting is transmitted in
    // response to the received byte before the interrupt is signalled. 
    // Therefor we must actually set NACK when the _next_ to last byte is
    // received, causing that NACK to be sent in response to receiving the last
    // expected byte of data.

    // build sla+w, slave device address + w bit
    i2c_slarw = (address)|1; //I2C_READ;

    // if we're in the repeated start state, then we've already sent the start,
    // (@@@ we hope), and the TWI statemachine is just waiting for the address byte.
    // We need to remove ourselves from the repeated start state before we enable interrupts,
    // since the ISR is ASYNC, and we could get confused if we hit the ISR before cleaning
    // up. Also, don't enable the START interrupt. There may be one pending from the 
    // repeated start that we sent outselves, and that would really confuse things.
    if (i2c_inRepStart)
    {
        // remember, we're dealing with an ASYNC ISR
        i2c_inRepStart = false;
        I2C_restart(module);
        resp_sla = I2C_writeChar(module, i2c_slarw);
    }
    else
    {
        // send start condition
        I2C_start(module);
        resp_sla = I2C_writeChar(module, i2c_slarw);
    }

    // address sent, ack received
    // put byte into buffer
    if (resp_sla)
    {
        while (_i2c_masterBufferIndex < _i2c_masterBufferLength)
        {
            _i2c_masterBuffer[_i2c_masterBufferIndex++] = I2C_readChar(module);
            // data received, ack sent
            // ack if more bytes are expected, otherwise nack
            if (_i2c_masterBufferIndex == _i2c_masterBufferLength)
                I2C_sendNack(module);
            else
                I2C_sendAck(module);
        }
    }


    if (_i2c_masterBufferIndex < length)
        length = _i2c_masterBufferIndex;

    // copy i2c buffer to data
    for(i = 0; i < length; ++i)
        data[i] = _i2c_masterBuffer[i];

    if (i2c_sendStop)
        I2C_stop(module);
    else
        i2c_inRepStart = true;

    i2c_state = I2C_READY;
    return length;
}

/* 
 * Function I2C_writeTo
 * Desc     attempts to become twi bus master and write a
 *          series of bytes to a device on the bus
 * Input    address: 7bit i2c device address
 *          data: pointer to byte array
 *          length: number of bytes in array
 *          wait: boolean indicating to wait for write or not
 *          sendStop: boolean indicating whether or not to send a stop at the end
 * Output   0 .. success
 *          1 .. length to long for buffer
 *          2 .. address send, NACK received
 *          3 .. data send, NACK received
 *          4 .. other twi error (lost bus arbitration, bus error, ..)
 */
u8 I2C_writeTo(u8 module, u8 address, u8* data, u8 length, u8 wait, u8 sendStop)
{
    u8 i;
    u8 resp_sla;
    u8 resp_dat;
    
    // ensure data will fit into buffer
    if(length > _I2CBUFFERLENGTH_)
        return 1;

    // wait until twi is ready, become master transmitter
    while(i2c_state != I2C_READY)
        continue;

    i2c_state = I2C_MTX;
    i2c_sendStop = sendStop;
    // reset error state (0xFF.. no error occured)
    i2c_error = 0xFF;

    // initialize buffer iteration vars
    _i2c_masterBufferIndex = 0;
    _i2c_masterBufferLength = length;

    // copy data to twi buffer
    for(i = 0; i < length; ++i)
        _i2c_masterBuffer[i] = data[i];

    // build sla+w, slave device address + w bit
    i2c_slarw = address;  
    // if we're in a repeated start, then we've already sent the START
    // in the ISR. Don't do it again.

    // if we're in the repeated start state, then we've already sent the start,
    // (@@@ we hope), and the TWI statemachine is just waiting for the address byte.
    // We need to remove ourselves from the repeated start state before we enable interrupts,
    // since the ISR is ASYNC, and we could get confused if we hit the ISR before cleaning
    // up. Also, don't enable the START interrupt. There may be one pending from the 
    // repeated start that we sent outselves, and that would really confuse things.
    if (i2c_inRepStart  == true)
    {
        // remember, we're dealing with an ASYNC ISR
        i2c_inRepStart = false;
        resp_sla = I2C_writeChar(module, i2c_slarw);
    }
    else
    {
        // send start condition
        I2C_start(module);
        resp_sla = I2C_writeChar(module, i2c_slarw);
    }

    if (resp_sla)
    {
        //addr ack
        while(_i2c_masterBufferIndex < _i2c_masterBufferLength)
        {
            // copy data to output register and ack
            resp_dat= I2C_writeChar(module, _i2c_masterBuffer[_i2c_masterBufferIndex++]);
            if (!resp_dat)
                break;
        }


        if (i2c_sendStop)
            I2C_stop(module);
        else
            // We'll generate the start, but we avoid handling the
            // interrupt until we're in the next transaction,
            // at the point where we would normally issue the start.
            i2c_inRepStart = true;

        i2c_state = I2C_READY;
    }
    
    if (i2c_error == 0xFF)
        return 0;	// success
    else
        return 4;	// i2c error
}

/* 
 * Function I2C_transmit
 * Desc     fills slave tx buffer with data
 *          must be called in slave tx event callback
 * Input    data: pointer to byte array
 *          length: number of bytes in array
 * Output   1 length too long for buffer
 *          2 not slave transmitter
 *          0 ok
 */
#ifdef I2CINT
u8 I2C_transmit(u8 module, const u8* data, u8 length)
{
    u8 i;

    // ensure data will fit into buffer
    if (length > _I2CBUFFERLENGTH_)
        return 1;

    // ensure we are currently a slave transmitter
    if (i2c_state != I2C_STX)
        return 2;

    // set length and copy data into tx buffer
    _i2c_txBufferLength = length;

    for (i = 0; i < length; ++i)
        _i2c_txBuffer[i] = data[i];

    //serial_printf("\r\n%2X",length); 
    return 0;
}
#endif

void I2C_begin(u8 module, u8 address, u16 speed)
{
    if (address !=0)
    {
        //  SSPADD = address; done in I2C_init
        //  I2C_slave((u16)address);
        #ifdef I2CINT
        i2c_hw_slave_init(address);
        #endif
    }
    
    _i2c_rxBufferIndex = 0;
    _i2c_rxBufferLength = 0;

    _i2c_txBufferIndex = 0;
    _i2c_txBufferLength = 0;

    // initialize state
    i2c_state = I2C_READY;
    i2c_sendStop = true;		// default value
    i2c_inRepStart = false;

    if (address ==0)
        I2C_master(module, speed);
}

/*  --------------------------------------------------------------------
    ---------- requestFrom
    --------------------------------------------------------------------
    ------------------------------------------------------------------*/

u8 I2C_requestFrom(u8 module, u8 address, u8 quantity)
{
    u8 read;
    u8 sendStop;
    
    // clamp to buffer length
    if (quantity > _I2CBUFFERLENGTH_)
        quantity = _I2CBUFFERLENGTH_;

    // perform blocking read into buffer
    sendStop = true;
    read = I2C_readFrom(module, address, _i2c_rxBuffer, quantity, sendStop);

    // set rx buffer iterator vars
    _i2c_rxBufferIndex = 0;
    _i2c_rxBufferLength = read;

    return read;
}


/*  --------------------------------------------------------------------
    ---------- beginTransmission
    --------------------------------------------------------------------
    ------------------------------------------------------------------*/

void I2C_beginTransmission(u8 module, u8 address)
{
    // indicate that we are transmitting
    _i2c_transmitting = 1;
    // set address of targeted slave
    _i2c_txAddress = address;
    // reset tx buffer iterator vars
    _i2c_txBufferIndex = 0;
    _i2c_txBufferLength = 0;
}


/*  --------------------------------------------------------------------
    ---------- endTransmission
    --------------------------------------------------------------------
    ------------------------------------------------------------------*/

u8 I2C_endTransmission(u8 module, u8 sendStop )
{
    //u8 sendStop = true;
    u8 ret;

    // transmit buffer (blocking)
    ret = I2C_writeTo(module, _i2c_txAddress, _i2c_txBuffer, _i2c_txBufferLength, 1, sendStop);
    // reset tx buffer iterator vars
    _i2c_txBufferIndex = 0;
    _i2c_txBufferLength = 0;
    // indicate that we are done transmitting
    _i2c_transmitting = 0;

    return ret;
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
// *****
u8 I2C_inBuffer(u8 module, u8 data)
{
    if (_i2c_transmitting)
    {
        // in master transmitter mode
        // don't bother if buffer is full
        if (_i2c_txBufferLength >= _I2CBUFFERLENGTH_)
          return 0;

        // put byte in tx buffer
        _i2c_txBuffer[_i2c_txBufferIndex] = data;
        ++_i2c_txBufferIndex;
        // update amount in buffer
        //serial_printf("\r\n%2X",data);	
        _i2c_txBufferLength = _i2c_txBufferIndex;
    }

    #ifdef I2CINT
    else
    {
        // in slave send mode
        // reply to master
        I2C_transmit(module, &data, 1);
    }
    #endif

    return 1;
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
u8 I2C_inBufferS(u8 module, u8 *data, u8 quantity)
{
    u8 i;

    // in master transmitter mode
    if (_i2c_transmitting)
        for( i = 0; i < quantity; ++i)
            I2C_inBuffer(module, data[i]);

    #ifdef I2CINT
    // in slave send mode
    else
        I2C_transmit(module, data, quantity);
    #endif

    return quantity;
}

u8 I2C_inBufferStr(u8 module, u8 *data)
{
    u8 i;

    if(_i2c_transmitting)
    {
        // in master transmitter mode
        i = 0;
        while (data[i] != '\0')
        {
            I2C_inBuffer(module, data[i]);
            ++i;
        }
    }
    #ifdef I2CINT
    else
    {
        i=0;
        while (data[i] != '\0') i++;
        // in slave send mode
        // reply to master

        I2C_transmit(module, data,i);
    }
    #endif

    return i;
}

/*  --------------------------------------------------------------------
    ---------- available
    --------------------------------------------------------------------
    must be called in:
    slave rx event callback
    or after requestFrom(address, numBytes)
    ------------------------------------------------------------------*/

u8 I2C_available(u8 module)
{
    return _i2c_rxBufferLength - _i2c_rxBufferIndex;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
/*int available(void)
{
  return rxBufferLength - rxBufferIndex;
}
*/

/*  --------------------------------------------------------------------
    ---------- receive
    --------------------------------------------------------------------
    must be called in:
    slave rx event callback
    or after requestFrom(address, numBytes)
    ------------------------------------------------------------------*/

u8 I2C_outBuffer(u8 module)
{
    // default to returning null char
    // for people using with char strings
    u8 value = '\0';
    // get each successive byte on each call
    if (_i2c_rxBufferIndex < _i2c_rxBufferLength)
    {
        value = _i2c_rxBuffer[_i2c_rxBufferIndex];
        ++_i2c_rxBufferIndex;
    }
    return value;
}

// must be called in:
// slave rx event callback
// or after requestFrom(address, numBytes)
int peek(u8 module)
{
    int value = -1;

    if(_i2c_rxBufferIndex < _i2c_rxBufferLength)
        value = _i2c_rxBuffer[_i2c_rxBufferIndex];

    return value;
}

/*	----------------------------------------------------------------------------
    ---------- OnRequest
    ----------------------------------------------------------------------------
    --------------------------------------------------------------------------*/
#ifdef I2CINT

void I2C_onRequest(void (* func) ())
{
    _i2c_onRequest_function = func;
    i2c_state = I2C_STX;
}

void I2C_onReceive(void (*func) (u8))
{
    // alert user program
    _i2c_onReceive_function = func;
    i2c_state = I2C_SRX;
    _i2c_rxBufferIndex = 0;
    I2C_sendAck();
}
#endif

#endif //__I2CMASTER_C
