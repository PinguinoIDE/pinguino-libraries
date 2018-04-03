/*  --------------------------------------------------------------------
    FILE:           manchester.c
    PROJECT:        Pinguino
    PURPOSE:        Encode/Decode bytes the Manchester way
    PROGRAMER:      Regis Blanchot <rblanchot@gmail.com>
    REFERENCES:     http://www.quickbuilder.co.uk/qb/articles/index.htm
                    http://www.erg.abdn.ac.uk/users/gorry/course/phy-pages/man.html
    --------------------------------------------------------------------
    CHANGELOG:
    12 Apr. 2017 -  Régis Blanchot - first release
    --------------------------------------------------------------------
    TODO:
    --------------------------------------------------------------------
    USAGE:

    void main()
    {
        u8 rxbyte;          // this is where the decoded byte goes
        u8 txbyte = 0x41;   // this is the inital byte to be encoded
        u8 encoded[2];      // this is an array of two bytes manchester encoded

        // first entry is the byte to encode the second is an array for the encoded output
        Manchester_encode(txbyte, encoded);

        // takes in a two part array and returns a singal byte. 
        rxbyte = Manchester_decode(encoded);

        // ???
        rxbyte = rxbyte + 4;
    }
    --------------------------------------------------------------------
    Manchester encoding

    Manchester encoding (also known as Biphase Code) is a synchronous
    clock encoding technique used to encode the clock and data of a
    synchronous bit stream. In this technique, the actual binary data to
    be transmitted over the cable or RF link are not sent as a sequence
    of logic 1's and 0's as in RS-232 (known technically as Non Return
    to Zero (NRZ)). Instead, the bits are translated u16o a slightly
    different format that has a number of advantages over using straight
    binary encoding (ie NRZ).

    The main advantages of using Manchester encoding are:
    1. Serial bit stream has a DC component of zero
    2. Error detection is simple to implement

    In general, when transmitting serial data to a radio receiver, a DC
    component of zero must be mau16ained (over a finite time). This is
    so the demodulator in the receiver can properly u16erpret
    (discriminate) the received data as 1's and 0's. Manchester encoding
    allows us to do this.

    Manchester encoding follows the rules:
    1. If the original data is a Logic 0, the Manchester code is: 0 to 1 (upward transition at bit centre)
    2. If the original data is a Logic 1, the Manchester code is: 1 to 0 (downward transition at bit centre)
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

#ifndef __MANCHESTER_C
#define __MANCHESTER_C

#include <typedef.h>
#include <macro.h>

u8 Manchester_nibbler(u8);
u8 Manchester_decode(u8*);
void Manchester_encode (u8, u8*);

//u8   RF433MHz_decodeMessage(u16 m, u8 &id, u8 &data); //decode 8 bit payload and 4 bit ID from the message, return 1 of checksum is correct, otherwise 0
//u16  RF433MHz_encodeMessage(u8 id, u8 data); //encode 8 bit payload, 4 bit ID and 4 bit checksum into 16 bit


/*  --------------------------------------------------------------------
    Encode
    u8  txbyte      the byte to be encoded
    u8* encoded     an array of two bytes that represent the manchester
                    encoded byte.
    ------------------------------------------------------------------*/

void Manchester_encode(u8 txbyte, u8* encoded)
{
    u16 i, j, b, me;

    b = txbyte;

    for (i = 0; i<2; i++)
    {
        me = 0; // manchester encoded txbyte
        for(j = 0; j<4; j++)
        {
            me >>=2;
            if (b&1) 
                me |= 0b01000000; // 1->0
            else
                me |= 0b10000000; // 0->1
            b >>= 1;
        }
        encoded[i] = me;
    }
}

/*  --------------------------------------------------------------------
    Decode
    u8* encoded     an array of two bytes that represent the manchester
                    encoded byte.
    ------------------------------------------------------------------*/

u8 Manchester_decode(u8* received)
{
    return ( (Manchester_nibbler(received[1])*16)^(Manchester_nibbler(received[0])) );
}

u8 Manchester_nibbler(u8 encoded)
{
    u8 i, dec, enc, pattern;

    enc = encoded;
    if (enc == 0xF0)
        return 0xF0;

    dec = 0;
    for(i = 0; i<4; i++) 
    {
        dec >>=1;
        pattern = enc & 0b11;
        if (pattern == 0b01)
            BitSet(dec, 3);
        else if(pattern == 0b10)
            BitClear(dec, 3);
        else // illegal code
            return 0xFF;
        enc >>=2;
    }
    return dec;
}

#endif // __MANCHESTER_C
