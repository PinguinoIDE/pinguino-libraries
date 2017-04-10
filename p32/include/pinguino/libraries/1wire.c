/*  ----------------------------------------------------------------------------
    FILE:           1wire.c
    PROJECT:        Pinguino
    PURPOSE:        One wire Interface Functions
    PROGRAMER:      Regis Blanchot <rblanchot@gmail.com>
    FIRST RELEASE:  28 Sept. 2010
    LAST RELEASE:   14 jan. 2011
    ----------------------------------------------------------------------------
    CHANGELOG:
    28 Sep 2010 Régis Blanchot      first release based on Maxim AN162 and Microchip AN1199
    28 May 2014 Régis Blanchot      fixed OneWireRead  -> OneWireReadByte
                                    fixed OneWireWrite -> OneWireWriteByte
    12 Oct 2015 Luca (aka Brikker)  disabled interrupts in OneWireReset, OneWireReadBit,
                                    and OneWireWriteBit functions
    23 Mai 2016 Régis Blanchot      disabled/enabled interrupts when necessary
                                    used Maxim's official timings
    29 Mar 2017 Régis Blanchot      reported interrupt management to delay functions
                                    updated timings to work with SDCC and XC8
    ----------------------------------------------------------------------------
    TODO :
    ----------------------------------------------------------------------------
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
    --------------------------------------------------------------------------*/

#ifndef __ONEWIRE_C
    #define __ONEWIRE_C

    #include <typedef.h>
    #include <macro.h>              // interrupt(), noInterrupt(), ...

    #ifndef __PIC32MX__
    #include <delayus.c>            // Delayus
    #include <digitalp.c>           // pinmode
    #include <digitalr.c>           // digitalread
    #include <digitalw.c>           // digitalwrite
    #else
    #include <delay.c>              // Delayus
    #include <digitalw.c>           // digitalwrite
    #endif

    // Standard 1-Wire timing
    // https://www.maximintegrated.com/en/app-notes/index.mvp/id/126
    #define TimeA   10      // 6
    #define TimeB   55      // 64
    #define TimeC   65      // 60
    #define TimeD   5       // 10
    #define TimeE   10      // 9
    #define TimeF   53      // 55
    #define TimeG   3       // 0
    #define TimeH   1000    // 480 min
    #define TimeI   100     // 70
    #define TimeJ   410

    // private
    /*
    void OneWireLow(u8, u16);
    void OneWireHigh(u8, u16);
    u8 OneWireRead(u8, u16);
    */
    // public
    u8 OneWireReset(u8);
    u8 OneWireReadBit(u8);
    void OneWireWriteBit(u8, u8);
    u8 OneWireReadByte(u8);
    void OneWireWriteByte(u8, u8);

/*  ----------------------------------------------------------------------------
    ---------- Force the DQ line to a logic low
    --------------------------------------------------------------------------*/
/*
    void OneWireLow(u8 DQpin, u16 t)
    {
        pinmode(DQpin, OUTPUT);
        digitalwrite(DQpin, LOW);
        if (t) Delayus(t);
    }
*/
/*  ----------------------------------------------------------------------------
    ---------- Force the DQ line into a high impedance state
    --------------------------------------------------------------------------*/
/*
    void OneWireHigh(u8 DQpin, u16 t)
    {
        pinmode(DQpin, INPUT);
        if (t) Delayus(t);
    }
*/
/*  --------------------------------------------------------------------
    ---------- Read the one-wire bus and return it.
    ------------------------------------------------------------------*/
/*
    u8 OneWireRead(u8 DQpin, u16 t)
    {
        u8 b;
        
        //pinmode(DQpin, INPUT);
        b = digitalread(DQpin);         // get signal
        if (t) Delayus(t);              // wait for rest of timeslot

        return b;                       // return value of DQ line
    }
*/
/*  ----------------------------------------------------------------------------
    ---------- Initiates the one wire bus
    ----------------------------------------------------------------------------
    Performs a reset on the one-wire bus and returns the presence detect.
    Reset is 2*480us long, presence checked another 60us later.
    returns 0=presence, 1=no part
    --------------------------------------------------------------------------*/

    u8 OneWireReset(u8 DQpin)
    {
        u8 dq;
        /*
        u8 status = isInterrupts();
        
        if (status) noInterrupts();
        OneWireLow(DQpin, TimeH);       // pull DQ line low
        OneWireHigh(DQpin, TimeI);      // allow line to return high
        dq=OneWireRead(DQpin, TimeJ);   // get presence signal
        if (status) interrupts();
        */
        //u8 status = isInterrupts();
        //if (status) noInterrupts();
        digitalwrite(DQpin, LOW);
        pinmode(DQpin, OUTPUT);
        Delayus(TimeH);
        pinmode(DQpin, INPUT);
        Delayus(TimeI);        
        dq = digitalread(DQpin);         // get signal
        Delayus(TimeJ);        
        //if (status) interrupts();
        return dq;
    }

/*  ----------------------------------------------------------------------------
    ---------- Read a bit from the one-wire bus and return it.
    --------------------------------------------------------------------------*/

    u8 OneWireReadBit(u8 DQpin)
    {
        u8 dq;
        /*
        u8 status = isInterrupts();
        
        if (status) noInterrupts();
        OneWireLow(DQpin, TimeG);       // pull DQ line low
        OneWireHigh(DQpin, TimeE);      // allow line to return high
        dq = OneWireRead(DQpin, TimeF); // Get the bit
        if (status) interrupts();
        */
        //u8 status = isInterrupts();
        //if (status) noInterrupts();
        pinmode(DQpin, OUTPUT);
        digitalwrite(DQpin, LOW);
        //Delayus(TimeG);
        pinmode(DQpin, INPUT);
        Delayus(TimeE);        
        dq = digitalread(DQpin);         // get signal
        Delayus(TimeF);        
        //if (status) interrupts();
        return dq;                      // return value of DQ line
    }

/*  ----------------------------------------------------------------------------
    ---------- Read a byte from the one-wire bus and return it.
    --------------------------------------------------------------------------*/

    u8 OneWireReadByte(u8 DQpin)
    {
        /*
        u8 i, val = 0;
        
        for (i=0; i<8; i++)
            if (OneWireReadBit(DQpin))
                BitSet(val, i);
        */
        
        u8 bitMask, val = 0;

        for (bitMask = 0x01; bitMask; bitMask <<= 1)
            if (OneWireReadBit(DQpin))
                val |= bitMask;

        return val;
    }

/*  ----------------------------------------------------------------------------
    ---------- Writes a bit to the one-wire bus, passed in bitval.
    --------------------------------------------------------------------------*/

    void OneWireWriteBit(u8 DQpin, u8 bitval)
    {
        //u8 status = isInterrupts();
        
        //if (status) noInterrupts();
        if (bitval)
        {
            /*
            OneWireLow(DQpin, TimeA);   // pull DQ line low
            OneWireHigh(DQpin, TimeB);  // allow line to return high
            */
            digitalwrite(DQpin, LOW);
            pinmode(DQpin, OUTPUT);
            Delayus(TimeA);
            pinmode(DQpin, INPUT);
            Delayus(TimeB);        
        }
        else
        {
            /*
            OneWireLow(DQpin, TimeC);   // pull DQ line low
            OneWireHigh(DQpin, TimeD);  // allow line to return high
            */
            digitalwrite(DQpin, LOW);
            pinmode(DQpin, OUTPUT);
            Delayus(TimeC);
            pinmode(DQpin, INPUT);
            Delayus(TimeD);        
        }
        //if (status) interrupts();
    }

/*  ----------------------------------------------------------------------------
    ---------- Writes a byte to the one-wire bus.
    --------------------------------------------------------------------------*/

    void OneWireWriteByte(u8 DQpin, u8 val)
    {
        /*
        u8 n;
        for (n=0; n<8; n++)
            OneWireWriteBit(DQpin, BitRead(val, n));
        */
        u8 bitMask;
        for (bitMask = 0x01; bitMask; bitMask <<= 1)
            OneWireWriteBit(DQpin, (bitMask & val)?1:0 );
    }

#endif
