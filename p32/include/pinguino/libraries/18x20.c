/*    --------------------------------------------------------------------
    FILE:           18x20.c
    PROJECT:        Pinguino
    PURPOSE:        One wire driver to use with DS18x20 digital temperature sensor.
    PROGRAMER:      regis blanchot <rblanchot@gmail.com>
    FIRST RELEASE:  28 Sep 2010
    LAST RELEASE:   27 Mar 2014
    --------------------------------------------------------------------
    02 Jun 2011	Jean-Pierre Mandon  fixed a bug in decimal part of the measure
    17 Jan 2012	Mark Harper         update to deal correctly with negative temperatures
    29 Jun 2012 Régis Blanchot      changed CRC calculation to save 8-bit Pinguino's RAM
    24 Oct 2012 Régis Blanchot      renamed variable num to rom for better understanding
    10 Jun 2013 Moreno Manzini      added DS18x20StartMeasure and DS18x20ReadMeasure
                                    to acquire and read temperature in non-blocking mode
    27 Mar 2014 Brikker             added reading until CRC is found valid
    28 May 2014 Régis Blanchot      fixed OneWireRead / OneWireWrite in 1wire.c
    
    18 Oct 2015 Brikker             Added CRC calc in DS18x20ReadRom()
                                    Added a global var SCRATCHPAD[9] in order to have the scratchpad available in main program; only for debug
                                    Various changes on DS18x20Read() function: 
                                        Removed DS18x20Configure() to fix resolution problem
                                        Fix resolution problem on temp. calculation
                                        Added Timeout to avoid infinite loop.
                                    Added DS18S20Read() function.
                                    DS18x20Configure():
                                        fix prototype: s8 TH, s8 TL -> DS18x20Configure(u8 pin, u8 rom, s8 TH, s8 TL, u8 config) 
                                        check TH and TL in range
                                        Store TH, TL and config on non-volatile memory with command COPY_SCRATCHPAD
                                    Added DS18x20GetTemperatureByIndex() function
                                    Added DS18x20DeviceCount() function.
                                    Added DS18x20Begin() function.
    15 Mai 2016 Régis Blanchot      DS18x20Find returns # of devices found
                                    Added various #define in 1wire.pdl to spare memory
                                    Mixed Moreno and Brikker solutions
                                    Added DS18x20Begin() in 1wire.pdl
                                    Turned some functions to macro
    30 Oct 2016 Régis Blanchot      Added Fahrenheit degrees to Celsius degrees conversion
                                    Updated DS18B20.function() to DS18x20.function()
                                    Completed 18B20 and 18S20 reading
                                    Updated DS18x20Wait() to a non-blocking loop
    --------------------------------------------------------------------
    TODO :
    * DS1822 support
    * replace
      #define DS18x20DeviceCount()      (gNumROMs)
      by
      #define DS18x20DeviceCount(bus)   DS18x20Find(bus)
    --------------------------------------------------------------------
    this file is based on Maxim AN162 and Microchip AN1199
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

#ifndef __DS18x20_C
    #define __DS18x20_C
    #define __DS18X20__
    //#define DS18X20DEBUG

    #include <const.h>                  // false, true, ...
    #include <macro.h>
    #include <typedef.h>
    #include <18x20.h>
    #include <1wire.c>
    //#include <delayms.c>
    #ifdef DS18X20DEBUG
    #define SERIALPRINTF
    #include <serial.c>
    #endif

/*  --------------------------------------------------------------------
    ---------- GLOBAL VARIABLES
    ------------------------------------------------------------------*/

    u8 DS18X20ROM[MAX_SENSORS_NUM][8];  // table of found ROM codes
    u8 gROMCODE[8];                      // ROM Bit
    u8 gSCRATCHPAD[9];                   // Scratchpad
    u8 gLastDiscrep = 0;                // last discrepancy
    u8 gDoneFlag = 0;                   // Done flag
    u8 gNumROMs;                        // Number of devices
    u8 gDowCRC = 0;
    u8 gFamily = 0;
    
    /*
    const u8 dscrc_table[] = {
        0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
        157,195, 33,127,252,162, 64, 30, 95, 1,227,189, 62, 96,130,220,
        35,125,159,193, 66, 28,254,160,225,191, 93, 3,128,222, 60, 98,
        190,224, 2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
        70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89, 7,
        219,133,103, 57,186,228, 6, 88, 25, 71,165,251,120, 38,196,154,
        101, 59,217,135, 4, 90,184,230,167,249, 27, 69,198,152,122, 36,
        248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91, 5,231,185,
        140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
        17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
        175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
        50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
        202,148,118, 40,171,245, 23, 73, 8, 86,180,234,105, 55,213,139,
        87, 9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
        233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
        116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53};
    */

/*  --------------------------------------------------------------------
    ---------- DS18x20Wait()
    --------------------------------------------------------------------
    * Description:  Wait while the 1-wire bus is busy (ie bus is low)
                    DS18B20 will respond by transmitting 0 while the
                    temperature conversion is in progress and 1 when the
                    conversion is done.
    * Arguments:    bus = pin number where the 1-wire bus is connected.
    ------------------------------------------------------------------*/

    void DS18x20Wait(u8 bus)
    {
        //u16 timeout = 3000;
        
        //while (!OneWireReadByte(bus) && (timeout++));
        //while (!OneWireReadBit(bus) && (timeout--));
        while (!OneWireReadBit(bus));
    }
        
/*  --------------------------------------------------------------------
    ---------- DS18x20Select()
    --------------------------------------------------------------------
    * Description:  Reset the bus and send a command
    * Arguments:    bus = pin number where the 1-wire bus is connected.
                    index = index of the sensor or SKIPROM
    ------------------------------------------------------------------*/

    u8 DS18x20Select(u8 bus, u8 index)
    {
        // Exit if no device is present
        if (OneWireReset(bus))
            return false;
            
        // Skip ROM, address all devices
        if (index == SKIPROM)
            DS18x20SkipRom(bus);

        // Talk to a particular device
        else
            if (!DS18x20MatchRom(bus, index))
                return false;
        return true;
    }

/*  --------------------------------------------------------------------
    ---------- DS18x20SendCommand()
    --------------------------------------------------------------------
    * Description:  Reset the bus and send a command
    * Arguments:    bus = pin number where the 1-wire bus is connected.
                    index = index of the sensor or SKIPROM
                    cmd = command to send
    ------------------------------------------------------------------*/

    u8 DS18x20SendCommand(u8 bus, u8 cmd)
    {
        // Exit if no device is present
        if (OneWireReset(bus))
            return false;

        // Send Command
        OneWireWriteByte(bus, cmd);

        return true;
    }

/*  --------------------------------------------------------------------
    ---------- DS18x20StartMeasure()
    --------------------------------------------------------------------
    * Description:  reads the ds18x20 device on the 1-wire bus and
                    starts the temperature acquisition
    * Arguments:    bus = pin number where the 1-wire bus is connected.
                    index = index of the sensor or SKIPROM
    ------------------------------------------------------------------*/

    u8 DS18x20StartMeasure(u8 bus, u8 index)
    {
        // Reset, select sensor and start temperature conversion
        /*
        if (!DS18x20Select(bus, index))
            return false;
        if (!DS18x20SendCommand(bus, CONVERT_T))
            return false;
        */

        // Exit if no device is present
        if (OneWireReset(bus))
            return false;
            
        // Skip ROM, address all devices
        if (index == SKIPROM)
            DS18x20SkipRom(bus);

        // Talk to a particular device
        else
        {
            if (!DS18x20MatchRom(bus, index))
                return false;
            // Exit if no device is present
            if (OneWireReset(bus))
                return false;
        }

        // Send Command
        OneWireWriteByte(bus, CONVERT_T);
        
        #ifdef DS18X20DEBUG
        Serial_printf("Conversion started ...\r\n");
        #endif
        return true;
    }

/*  --------------------------------------------------------------------
    ---------- DS18x20ReadMeasure()
    --------------------------------------------------------------------
    * Description:  Select the proper DS18x20ReadMeasure() 
                    according device's family code
    * Arguments:    pin = pin number where the 1-wire bus is connected.
                    index = index of the sensor
                    t = temperature pointer	
    ------------------------------------------------------------------*/

    u8 DS18x20ReadMeasure(u8 bus, u8 index, DS18x20_Temperature * t)
    {
        #ifdef DS18X20DEBUG
        Serial_printf("Fam. = 0x%X\r\n", gFamily);
        #endif

        if (gFamily == FAMILY_CODE_DS18B20)
            return (DS18B20ReadMeasure(bus, index, t));

        if (gFamily == FAMILY_CODE_DS18S20)
            return (DS18S20ReadMeasure(bus, index, t));

        // Unsupported sensor, to do.
        if (gFamily == FAMILY_CODE_DS1822)
            return false;

        // If we don't know which type of sensor it is, we look for it.
        gFamily = DS18x20ReadFamilyCode(bus, index);
        return false;
    }

/*  --------------------------------------------------------------------
    ---------- DS18x20Read()
    --------------------------------------------------------------------
    * Description:  reads the ds18x20 device on the 1-wire bus
    * Arguments:    bus = pin number where the 1-wire bus is connected.
                    index = index of the sensor or SKIPROM
                    t = temperature pointer
    * Note          temperature is in Celsius
                    resolution can be change with DS18x20Configure
    * Returns       true if OK, false if error
    ------------------------------------------------------------------*/

    u8 DS18x20Read(u8 bus, u8 index, DS18x20_Temperature * t)
    {
        // Start the temperature acquisition
        if (!DS18x20StartMeasure(bus, index))
            return false;
            
        // Wait while the 1-wire bus is busy
        DS18x20Wait(bus);

        // Read temperature until CRC is valid
        return (DS18x20ReadMeasure(bus, index, t));
    }

/*  --------------------------------------------------------------------
    ---------- DS18x20ReadFahrenheit()
    --------------------------------------------------------------------
    * Description:  reads the ds18x20 device on the 1-wire bus
    * Arguments:    bus = pin number where the 1-wire bus is connected.
                    index = index of the sensor or SKIPROM
                    resolution = 9 to 12 bit resolution
                    t = temperature pointer
    * Note          temperature is in Fahrenheit
    * Returns       true if OK, false if error
    ------------------------------------------------------------------*/

    #ifdef DS18x20READFAHRENHEIT
    u8 DS18x20ReadFahrenheit(u8 bus, u8 index, DS18x20_Temperature * t)
    {
        DS18x20Read(bus, index, t);
        // Fahrenheit degrees to Celsius degrees conversion.
        // Formula : F = C * 9 / 5 + 32
        t->integer  = t->integer  * 9 / 5 + 32;
        t->fraction = t->fraction * 9 / 5;

        return true;
    }
    #endif // DS18x20READFAHRENHEIT

/*  --------------------------------------------------------------------
    ---------- Reads ROM or SCRATCHPAD from a device
    --------------------------------------------------------------------
    * Description:  reads and store the device's ROM code
    * Arguments:    bus = pin number where the 1-wire bus is connected.
                    index = index of the sensor or SKIPROM
                    cmd = READROM or READ_SCRATCHPAD
                    mem = array pointer
    --------------------------------------------------------------------
    This command can only be used when there is only ONE slave on the bus.
    It allows the bus master to read the slave’s 64-bit ROM code without
    using the Search ROM procedure.
    If this command is used when there is more than one slave present on
    the bus, a data collision will occur when all the slaves attempt to
    respond at the same time.
    ------------------------------------------------------------------*/

    u8 DS18x20ReadMemory(u8 bus, u8 index, u8 cmd, u8 *mem)
    {
        u8 i, d;
        u8 crc_flag = 0;
        u8 timeout = 0;

        // Exit if no device is present
        if (OneWireReset(bus))
            return false;
            
        // Skip ROM, address all devices
        if (index == SKIPROM)
            DS18x20SkipRom(bus);

        // Talk to a particular device
        else
        {
            if (!DS18x20MatchRom(bus, index))
                return false;
            // Exit if no device is present
            if (OneWireReset(bus))
                return false;
        }

        // Send Command
        OneWireWriteByte(bus, cmd);
        
        while (crc_flag == 0)
        {
            gDowCRC = 0;

             // Read bytes from the device
            for (i = 0; i < 8; i++)
            {
                mem[i] = OneWireReadByte(bus);
                #ifdef DS18X20DEBUG
                Serial_printf("[0x%X]", mem[i]);
                #endif
                DS18x20CRC(mem[i]);
            }
            
            // Read the 9th byte
            mem[8] = OneWireReadByte(bus);
            #ifdef DS18X20DEBUG
            Serial_printf("[0x%X] CRC=0x%X\r\n", mem[8], gDowCRC);
            #endif

            // Compare calculated CRC with the 9th byte received
            if (gDowCRC == mem[8])
                crc_flag = 1;

            if (timeout++ > 20)
                return false;
        }

        return true;
    }

/*  --------------------------------------------------------------------
    ---------- Reads the ROM Code from a device (when there is only one)
    --------------------------------------------------------------------
    * Description:  reads and store the device's ROM code
    * Arguments:    bus = pin number where the 1-wire bus is connected.
                    index = index of the sensor or SKIPROM
                    romcode = pointer where identification code of device
                    will be returned.
    --------------------------------------------------------------------
    This command can only be used when there is only ONE slave on the bus.
    It allows the bus master to read the slave’s 64-bit ROM code without
    using the Search ROM procedure.
    If this command is used when there is more than one slave present on
    the bus, a data collision will occur when all the slaves attempt to
    respond at the same time.
    ------------------------------------------------------------------*/

    u8 DS18x20ReadRom(u8 bus, u8 index, u8 *romcode)
    {
        return DS18x20ReadMemory(bus, index, READROM, romcode);
    }
    
/*  --------------------------------------------------------------------
    ---------- Reads the ROM Family Code from a device (when there is only one)
    --------------------------------------------------------------------
    * Arguments:    bus = pin number where the 1-wire bus is connected.
                    romcode = identification code of device.
    * Description:  reads and returns a byte of data from the device.
    --------------------------------------------------------------------
    Each DS18x20 contains a unique 64–bit code stored in ROM.
    The least significant 8 bits of the ROM code contains the DS18x20's
    1-Wire family code

    Note : The READROM command can only be used when there is only ONE
    slave on the bus. It allows the bus master to read the slave’s
    64-bit ROM code without using the Search ROM procedure.
    ------------------------------------------------------------------*/

    u8 DS18x20ReadFamilyCode(u8 bus, u8 index)
    {
        u8 i,b;
        if (index == SKIPROM)
        {
            DS18x20SendCommand(bus, READROM);
            return (OneWireReadByte(bus));
        }
        else
        {
            DS18x20MatchRom(bus, index);
            return DS18X20ROM[index][0];
        }
    }

/*  --------------------------------------------------------------------
    ---------- DS18B20ReadMeasure()
    --------------------------------------------------------------------
    * Description:  reads the DS18B20 device on the 1-wire bus
    * Arguments:    bus = pin number where the 1-wire bus is connected.
                    index = index of the sensor or SKIPROM
                    t = temperature pointer
    * Return:       the temperature previously acquired
    ------------------------------------------------------------------*/

    u8 DS18B20ReadMeasure(u8 bus, u8 index, DS18x20_Temperature * t)
    {
        u8  temp_lsb, temp_msb;
        u16 temp;

        if (DS18x20ReadMemory(bus, index, READ_SCRATCHPAD, gSCRATCHPAD))
        {
            temp_lsb = gSCRATCHPAD[0];  // byte 0 of scratchpad : temperature lsb
            temp_msb = gSCRATCHPAD[1];  // byte 1 of scratchpad : temperature msb

            // Calculation
            // -----------------------------------------------------
            //  Temperature Register Format
            //          BIT7    BIT6    BIT5    BIT4    BIT3    BIT2    BIT1    BIT0
            //  LSB     2^3     2^2     2^1     2^0     2^-1    2^-2    2^-3    2^-4
            //          BIT15   BIT14   BIT13   BIT12   BIT11   BIT10   BIT9    BIT8
            //  MSB     S   S       S       S       S       2^6     2^5     2^4
            //  S = SIGN

            // combine msb & lsb into 16 bit variable
            temp = (temp_msb << 8) + temp_lsb;
            
            // test if sign is set, i.e. negative
            if (temp_msb & 0b11111000)
            {
                t->sign = 1;
                temp = (temp ^ 0xFFFF) + 1;	// 2's complement conversion
            }
            else
            {
                t->sign = 0;
            }

            // fractional part is removed, leaving only integer part
            t->integer = (temp >> 4) & 0x7F;
            t->fraction = (temp & 0x0F) * 625;
            // two digits after decimal 
            //t->fraction /= 100;
            #ifdef DS18X20DEBUG
            Serial_printf("Temp. = %02d.%02d\176C\r\n", t->integer, t->fraction);
            #endif
            return true;
        }
        return false;
    }

/*  --------------------------------------------------------------------
    ---------- DS18S20ReadMeasure()
    --------------------------------------------------------------------
    * Description:  reads the DS18S20 device on the 1-wire bus
    * Arguments:    bus = pin number where the 1-wire bus is connected.
                    index = index of the sensor or SKIPROM
                    t = temperature pointer
    
                    Scratchpad Memory Layout
                    Byte  Register
                    0     Temperature_LSB
                    1     Temperature_MSB
                    2     Temp Alarm High / User Byte 1
                    3     Temp Alarm Low / User Byte 2
                    4     Reserved
                    5     Reserved
                    6     Count_Remain
                    7     Count_per_C
                    8     CRC
    * Returns:      the temperature previously acquired
    ------------------------------------------------------------------*/

    u8 DS18S20ReadMeasure(u8 bus, u8 index, DS18x20_Temperature * t)
    {
        u8  temp_lsb, temp_msb;
        u8  count_remain, count_per_c;
        u16 temp;

        if (DS18x20ReadMemory(bus, index, READ_SCRATCHPAD, gSCRATCHPAD))
        {
            temp_lsb = gSCRATCHPAD[0];       // byte 0 of scratchpad : temperature lsb
            temp_msb = gSCRATCHPAD[1];       // byte 1 of scratchpad : temperature msb
            count_remain = gSCRATCHPAD[6];
            count_per_c = gSCRATCHPAD[7];

            temp = temp_msb;
            temp = (temp << 8) + temp_lsb;  // combine msb & lsb into 16 bit variable
                    
            if (temp_msb & 0b11111111)      // test if sign is set, i.e. negative
            {
                t->sign = 1;
                temp = (temp ^ 0xFFFF) + 1; // 2's complement conversion
            }
            else
            {
                t->sign = 0;
            }
            
            /*
            CALCULATING STANDARD RESOLUTION: 9bit resolution in 0.5 step per degree
            */
            //t->integer = (temp >> 1) & 0x00FF;	// fractional part is removed, leaving only integer part
            //t->fraction = (temp & 0x01) * 50;
            
            /*
            CALCULATING EXTENDED RESOLUTION:
            
                                            (Count_per_C - Count_Remain)
            Temperature = temp_read - 0.25 + ---------------------------
                                                    Count_per_C

            Where temp_read is the value from the temp_MSB and temp_LSB with
            the least significant bit removed (the 0.5C bit).
            
            *** Smart calculation
            Source: http://myarduinotoy.blogspot.it/2013/02/12bit-result-from-ds18s20.html
            Let's implement it with integers to calculate the 1/16 of the Celsius degree value, 
            this is the integer result we get from any other device.
            
            1) Truncating the 0.5 bit - use a simple & mask: raw & 0xFFFE
            2) Convert to 12 bit value (1/16 of °C) - shift left: (raw & 0xFFFE)<<3
            3) Subtracting 0.25 (1/4 °C of 1/16) or 0.25/0.0625 = 4: ((raw & 0xFFFE)<<3)-4
            4) Add the count (count per c - count remain), count per c is constant of 16, 
               and no need to dived by 16 since we are calculating to the 1/16 of °C: +16 - COUNT_REMAIN

            Full expression: ((rawTemperature & 0xFFFE) << 3) - 4 + 16 - scratchPad[COUNT_REMAIN]
            We can simplify it to: ((rawTemperature & 0xFFFE) << 3) + 12 - scratchPad[COUNT_REMAIN]
            */
            temp = ((temp & 0xFFFE) << 3) + 12 - count_remain;

            // fractional part is removed, leaving only integer part
            t->integer = (temp >> 4) & 0x00FF;
            t->fraction = (temp & 0x0F) * 625;
            // two digits after decimal 
            //t->fraction /= 100;
            return true;
        }
        return false;
    }

/*  --------------------------------------------------------------------
    ---------- DS18x20Configure()
    --------------------------------------------------------------------
    * Description:  writes configuration data to the DS18x20 device
    * Arguments:
                    bus = pin number where the 1-wire bus is connected.
                    index = index of the sensor or SKIPROM
                    TH = Alarm Trigger High [-55,+125]
                    TL = Alarm Trigger Low  [-55,+125]
                    res = resolution : RES12BIT, RES10BIT, ...
    * Note1: Data must be transmitted least significant bit first
    * Note2: DS18S20 devices doesn't have a config register 
    ------------------------------------------------------------------*/

    #if defined(DS18x20CONFIGURE)
    u8 DS18x20Configure(u8 bus, u8 index, u8 TL, u8 TH, u8 res)
    {
        // make sure the alarm temperature is within the device's range
        if (TH > 125) TH = 125;
        else if (TH < -55) TH = -55;
        
        if (TL > 125) TL = 125;
        else if (TL < -55) TL = -55;

        // Reset, select the sensor and prepare to write bytes of data to the scratchpad
        if (!DS18x20Select(bus, index))
            return false;
        if (!DS18x20SendCommand(bus, WRITE_SCRATCHPAD))
            return false;
            
        // The first data byte is written into the TH register (byte 2 of the scratchpad)
        OneWireWriteByte(bus, TH);
        // The second byte is written into the TL register (byte 3)
        OneWireWriteByte(bus, TL);
        // The third byte is written into the configuration register (byte 4)
        OneWireWriteByte(bus, res);

        // Store data into EEprom
        if (!DS18x20Select(bus, index))
            return false;
        if (!DS18x20SendCommand(bus, COPY_SCRATCHPAD))
            return false;

        // Wait while busy
        DS18x20Wait(bus);
        
        return true;
    }
    #endif
    
/*  --------------------------------------------------------------------
    ---------- Address a specific slave device on a multidrop or single-drop bus
    --------------------------------------------------------------------
    * Arguments:    bus = pin number where the 1-wire bus is connected.
                    index = index of the sensor
    * Description:  reads and returns a byte of data from the device.
    --------------------------------------------------------------------
    The match ROM command followed by a 64-bit ROM code sequence allows the bus
    master to address a specific slave device on a multidrop or single-drop bus.
    Only the slave that exactly matches the 64-bit ROM code sequence will respond
    to the function command issued by the master; all other slaves on the bus
    will wait for a reset pulse.
    Note1: called form DS18x20Select() if index <> SKIPROM
    Note2: DS18x20Find() must have been called previously
    ------------------------------------------------------------------*/

    u8 DS18x20MatchRom(u8 bus, u8 index)
    {
        u8 i;
        
        // Send Match Rom Command
        OneWireWriteByte(bus, MATCHROM);

        // followed by the Address ROM Code of the targeted device
        for (i = 0; i < 8; i++)
            OneWireWriteByte(bus, DS18X20ROM[index][i]);

        return true;
    }

#if defined(DS18x20FIND)

/*  --------------------------------------------------------------------
    ---------- Find Devices on the one-wire bus
    --------------------------------------------------------------------
    * Description:  detects devices and saves their rom code.
    * Arguments:    pin number where the 1-wire bus is connected.
    * Returns :     false or true
    ------------------------------------------------------------------*/

    u8 DS18x20Find(u8 bus)
    {
        u8 m;

        // Detects presence of devices
        if (OneWireReset(bus)) return false;
        
        // Begins when at least one part is found
        if (!DS18x20GetFirst(bus)) return false;

        gNumROMs=0;
        // Continues until no additional devices are found
        do {
            gNumROMs++;
            // serialprint("Device #");
            // Identifies ROM number on found device
            for(m = 0; m < 8; m++)
                DS18X20ROM[gNumROMs][m] = gROMCODE[m];
        } while (DS18x20GetNext(bus) && (gNumROMs<MAX_SENSORS_NUM));

        //return gNumROMs;
        return true;
    }

/*  --------------------------------------------------------------------
    ---------- First
    --------------------------------------------------------------------
    * Arguments: pin number where the 1-wire bus is connected.
    * Description: resets the current state of a ROM search and calls Next to
    find the first device on the 1-wire bus.
    ------------------------------------------------------------------*/

    u8 DS18x20GetFirst(u8 bus)
    {
        // reset the rom search last discrepancy global
        gLastDiscrep = 0;
        gDoneFlag = false;
        // call Next and return its return value
        return DS18x20GetNext(bus);
    }

/*  --------------------------------------------------------------------
    ---------- Next
    --------------------------------------------------------------------
    * Arguments: pin number where the 1-wire bus is connected.
    * Description: searches for the next device on the 1-wire bus. If
    there are no more devices on the 1-wire then false is returned.
    ------------------------------------------------------------------*/

    u8 DS18x20GetNext(u8 bus)
    {
        u8 m = 1;                   // ROM Bit index
        u8 n = 0;                   // ROM Byte index
        u8 k = 1;                   // bit mask
        u8 x = 0;
        u8 discrepMarker = 0;       // discrepancy marker
        u8 g;                       // Output bit
        u8 nxt;                     // return value
        u8 flag;

        nxt = false;                // set the next flag to false
        gDowCRC = 0;                // reset the gDowCRC

        flag = OneWireReset(bus);   // reset the 1-wire
        if (flag||gDoneFlag)        // no parts -> return false
        {
            gLastDiscrep = 0;       // reset the search
            return false;
        }
        
        // send SearchROM command for all eight bytes
        OneWireWriteByte(bus, SEARCHROM);
        
        do {
            x = 0;
            if(OneWireReadBit(bus) == 1)
                x = 2;
            //Delayus(120);
            if(OneWireReadBit(bus) == 1 )
                x |= 1;

            if (x == 3)
                break;
                
            else
            {
                if(x > 0)           // all devices coupled have 0 or 1
                    g = x >> 1;     // bit write value for search
                else
                {
                    // if this discrepancy is before the last
                    // discrepancy on a previous Next then pick
                    // the same as last time
                    if(m < gLastDiscrep)
                        g = ((gROMCODE[n] & k) > 0);

                    // if equal to last pick 1
                    else
                        g = (m == gLastDiscrep);	// if not then pick 0

                    // if 0 was picked then record position with mask k
                    if (g == 0)
                        discrepMarker = m;
                }

                // isolate bit in gROMCODE[n] with mask k
                if (g == 1)
                    gROMCODE[n] |= k;
                else
                    gROMCODE[n] &= ~k;

                #ifdef DS18X20DEBUG
                Serial_printf("gROMCODE[%d] = 0x%X\r\n", n, gROMCODE[n]);
                #endif

                // ROM search write
                OneWireWriteBit(bus, g);
                m++;                // increment bit counter m
                k = k << 1;         // and shift the bit mask k
                if (k == 0)         // if the mask is 0 then go to new ROM
                {                   // byte n and reset mask
                    DS18x20CRC(gROMCODE[n]);// accumulate the CRC
                    n++;
                    k++;
                }
            }
        } while (n < 8);            // loop until through all ROM bytes 0-7
        
        if (m < 65 || gDowCRC)      // if search was unsuccessful then
            gLastDiscrep=0;         // reset the last discrepancy to 0
        else                        // search was successful, so set gLastDiscrep, lastOne, nxt
        {
            gLastDiscrep = discrepMarker;
            gDoneFlag = (gLastDiscrep == 0);
            nxt = true;             // indicates search is not complete yet, more parts remain
        }
        return nxt;
    }

#endif // DS18B20FIND

/*  --------------------------------------------------------------------
    ---------- CRC
    --------------------------------------------------------------------
    * Arguments:    x
    * Global:       gDowCRC - global crc stored here
    * Description:  update the CRC for transmitted and received data using
                    8-bit CRC equivalent polynomial function : x^8 + x^5 + x + 1
    --------------------------------------------------------------------
    * 1rst method : + very fast / - waste a lot of memory 
    --------------------------------------------------------------------
        gDowCRC = dscrc_table[gDowCRC^x];
    --------------------------------------------------------------------
    * 2nd method : + slow / - waste of memory
    --------------------------------------------------------------------
        unsigned char r1[16] = {
        0x00, 0x5e, 0xbc, 0xe2, 0x61, 0x3f, 0xdd, 0x83, 
        0xc2, 0x9c, 0x7e, 0x20, 0xa3, 0xfd, 0x1f, 0x41};
        unsigned char r2[16] = {
        0x00, 0x9d, 0x23, 0xbe, 0x46, 0xdb, 0x65, 0xf8,
        0x8c, 0x11, 0xaf, 0x32, 0xca, 0x57, 0xe9, 0x74};
        u8 i = (x ^ gDowCRC) & 0xff;
        gDowCRC = r1[i & 0xf] ^ r2[i>>4];
    --------------------------------------------------------------------
    * 3rd method : + fast / - waste very few memory
    --------------------------------------------------------------------
        // from https://www.ccsinfo.com/forum/viewtopic.php?t=37015
        u8 i = (x ^ gDowCRC) & 0xff;
        gDowCRC = 0;
        if(i & 0x01) gDowCRC ^= 0x5e;
        if(i & 0x02) gDowCRC ^= 0xbc;
        if(i & 0x04) gDowCRC ^= 0x61;
        if(i & 0x08) gDowCRC ^= 0xc2;
        if(i & 0x10) gDowCRC ^= 0x9d;
        if(i & 0x20) gDowCRC ^= 0x23;
        if(i & 0x40) gDowCRC ^= 0x46;
        if(i & 0x80) gDowCRC ^= 0x8c;
    ------------------------------------------------------------------*/

    void DS18x20CRC(u8 x)
    {
        u8 i = (x ^ gDowCRC) & 0xff;

        gDowCRC = 0;

        if (i & 0x01) gDowCRC ^= 0x5e;
        if (i & 0x02) gDowCRC ^= 0xbc;
        if (i & 0x04) gDowCRC ^= 0x61;
        if (i & 0x08) gDowCRC ^= 0xc2;
        if (i & 0x10) gDowCRC ^= 0x9d;
        if (i & 0x20) gDowCRC ^= 0x23;
        if (i & 0x40) gDowCRC ^= 0x46;
        if (i & 0x80) gDowCRC ^= 0x8c;
    }

#endif /* __DS18x20_C */
