/*    --------------------------------------------------------------------
    FILE:           18b20.h
    PROJECT:        Pinguino
    PURPOSE:        One wire driver to use with DS18x20 digital temperature sensor.
    PROGRAMER:      Regis blanchot <rblanchot@gmail.com>
    --------------------------------------------------------------------
    31 Oct 2016     Regis Blanchot 
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

#ifndef __DS18x20_H
    #define __DS18x20_H

    #include <typedef.h>

    typedef struct
    {
        u8  sign;                       // sign (1=negative)
        u8  integer;                    // integer part
        u16 fraction;                   // fractional part
    } DS18x20_Temperature;

    #define MAX_SENSORS_NUM 10

    /// DS18x20 ROM COMMANDS
    #define SEARCHROM           0xF0    //
    #define READROM             0x33    //
    #define MATCHROM            0x55    //
    #define SKIPROM             0xCC    //
    #define ALARM_SEARCH        0xEC    //

    /// DS18x20 FUNCTION COMMANDS
    #define CONVERT_T           0x44    // Initiates temperature conversion
    #define WRITE_SCRATCHPAD    0x4E    // Writes data into scratchpad bytes 2, 3, and 4 (TH, TL and configuration registers)
    #define READ_SCRATCHPAD     0xBE    // Reads the entire scratchpad including the CRC byte
    #define COPY_SCRATCHPAD     0x48    // Copies TH, TL, and configuration register data from the scratchpad to EEPROM
    #define RECALL_E2           0xB8    // Recalls TH, TL, and configuration register data from EEPROM to the scratchpad
    #define READ_POWER_SUPPLY   0xB4    // Signals DS18x20 power supply mode to the master

    /// RESOLUTIONS / MODES
    #define RES9BIT             0x1F    // 9 bit
    #define RES10BIT            0x3F    // 10 bit
    #define RES11BIT            0x5F    // 11 bit
    #define RES12BIT            0x7F    // 12 bit

    /// DS18x20 FAMILY CODE
    #define FAMILY_CODE_DS18B20 0x28
    #define FAMILY_CODE_DS18S20 0x10
    #define FAMILY_CODE_DS1822  0x22    // unused at the moment

    /// PROTOTYPES
    u8 DS18x20Configure(u8, u8, u8, u8, u8);
    u8 DS18x20Select(u8, u8);
    u8 DS18x20SendCommand(u8, u8);
    u8 DS18x20StartMeasure(u8, u8);
    ///-----------------------------------------------------------------
    u8 DS18x20Read(u8, u8, DS18x20_Temperature *);
    u8 DS18x20ReadFahrenheit(u8, u8, DS18x20_Temperature *);
    u8 DS18x20ReadMeasure(u8, u8, DS18x20_Temperature *);
    u8 DS18B20ReadMeasure(u8, u8, DS18x20_Temperature *);
    u8 DS18S20ReadMeasure(u8, u8, DS18x20_Temperature *);
    ///-----------------------------------------------------------------
    u8 DS18x20MatchRom(u8, u8);
    u8 DS18x20ReadRom(u8, u8, u8 *);
    u8 DS18x20ReadFamilyCode(u8, u8);
    u8 DS18x20Find(u8);
    u8 DS18x20GetFirst(u8);
    u8 DS18x20GetNext(u8);
    //u8 DS18x20CRC(u8);
    void DS18x20CRC(u8);
    void DS18x20Wait(u8);

    // Skip ROM check, address all devices
    #define DS18x20SkipRom(bus)        OneWireWriteByte(bus, SKIPROM)

    // Returns the number of devices available on the bus
    #define DS18x20DeviceCount()    (gNumROMs)
    // TO REPLACE BY #define DS18x20DeviceCount(bus) DS18x20Find(bus) ?

#endif /* __DS18x20_C */
