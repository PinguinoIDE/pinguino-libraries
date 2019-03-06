/// \file RDA5807M.cpp
/// \brief Implementation for the radio library to control the RDA5807M radio chip.
///
/// \author Matthias Hertel, http://www.mathertel.de
/// \copyright Copyright (c) 2014-2015 by Matthias Hertel.\n
/// This work is licensed under a BSD style license.\n
/// See http://www.mathertel.de/License.aspx
///
/// This library enables the use of the radio chip RDA5807M from http://www.rdamicro.com/.
///
/// More documentation and source code is available at http://www.mathertel.de/Arduino
///
/// History:
/// --------
/// * 05.08.2014 created.
/// * 01.03.2018 adapted for Pinguino

#ifndef __PIC32MX__
#include <compiler.h>
#endif

#include <typedef.h>
#include <macro.h>
#include <const.h>
#include <stdarg.h>
#include "MPU9250.h"

//#if defined(__PIC32MX__)
//#include <digitalw.c>
//#include <delay.c>
//#else
//#include <digitalw.c>
//#include <digitalp.c>
//#include <digitalt.c>
//#include <delayms.c>
//#include <delayus.c>
//#endif

#include <i2c.h>
#include <i2c.c>

#include <RDA5807M.h>

u16 RDA5807M_REG[0x10];  // memory representation of the registers
RADIO_INFO RDA5807M;
const u8 RDA5807M_MAXVOLUME = 15;   ///< max volume level for radio implementations.

u8 RDA5807M_init(u8 module)
{
    u8 i;
    
    // Reset all shadow registers

    for (i = 0x00; i < 0x10; i++)
        RDA5807M_REG[i] = 0x0000;

    // The RDA5807M only support I2C control interface bus mode.
    // Max. I2C speed is 400KHz but it doesn't work !

    I2C_master(module, I2C_100KHZ);

    // Check the Chip ID

    if (!RDA5807M_getChipID(module))
        return false;

    // Reset the chip
    
    RDA5807M_REG[RADIO_REG_R2] |= RADIO_REG_R2_SOFTRESET;
    RDA5807M_saveRegister(module, RADIO_REG_R2);
    RDA5807M_REG[RADIO_REG_R2] &= (~RADIO_REG_R2_SOFTRESET);
    RDA5807M_saveRegister(module, RADIO_REG_R2);

    // Initialize the RDA5807M

    RDA5807M_REG[RADIO_REG_R7] |= RADIO_REG_R7_SOFTBLEND_EN;
    RDA5807M_REG[RADIO_REG_R7] |= RADIO_REG_R7_SOFTBLEND;
    RDA5807M_REG[RADIO_REG_R7] |= RADIO_REG_R7_65M50M;
    RDA5807M_saveRegister(module, RADIO_REG_R7);

    RDA5807M_REG[RADIO_REG_R5] |= RADIO_REG_R5_INTMODE;
    RDA5807M_REG[RADIO_REG_R5] |= RADIO_REG_R5_SEEKTH;
    RDA5807M_REG[RADIO_REG_R5] |= RADIO_REG_R5_VOLMAX;
    RDA5807M_saveRegister(module, RADIO_REG_R5);

    RDA5807M_REG[RADIO_REG_R4] |= RADIO_REG_R4_EM50;
    RDA5807M_REG[RADIO_REG_R4] |= RADIO_REG_R4_SOFTMUTE;
    RDA5807M_saveRegister(module, RADIO_REG_R4);

    RDA5807M_REG[RADIO_REG_R3] |= RADIO_REG_R3_BAND_FM;
    RDA5807M.freqLow = 8700;
    RDA5807M.freqHigh = 10800;
    RDA5807M_REG[RADIO_REG_R3] |= RADIO_REG_R3_SPACE_100;
    RDA5807M.freqSteps = 100;
    RDA5807M_REG[RADIO_REG_R3] |= RADIO_REG_R3_TUNE;
    RDA5807M_REG[RADIO_REG_R3] |= ((((10630 - RDA5807M.freqLow) / RDA5807M.freqSteps * 10) & RADIO_REG_R3_FREQ_MASK) << 6);
    RDA5807M_saveRegister(module, RADIO_REG_R3);

    RDA5807M_REG[RADIO_REG_R2] |= RADIO_REG_R2_OUTPUT;
    RDA5807M_REG[RADIO_REG_R2] |= RADIO_REG_R2_UNMUTE;
    //RDA5807M_REG[RADIO_REG_R2] |= RADIO_REG_R2_MONO;
    RDA5807M_REG[RADIO_REG_R2] |= RADIO_REG_R2_SEEKUP;
    //RDA5807M_REG[RADIO_REG_R2] |= RADIO_REG_R2_SEEK;
    //RDA5807M_REG[RADIO_REG_R2] |= RADIO_REG_R2_SEEKMODE;
    //RDA5807M_REG[RADIO_REG_R2] |= RADIO_REG_R2_BASS;
    //RDA5807M_REG[RADIO_REG_R2] |= RADIO_REG_R2_SENSITIVITY;
    //RDA5807M_REG[RADIO_REG_R2] |= RADIO_REG_R2_RDS;
    RDA5807M_REG[RADIO_REG_R2] |= RADIO_REG_R2_ENABLE;
    RDA5807M_saveRegister(module, RADIO_REG_R2);
    
    return true;
}

// switch the power off
void RDA5807M_close(u8 module)
{
    RDA5807M_setVolume(module, 0);
    //RDA5807M_writeRegister(module, RADIO_REG_R2, 0x0000);
    RDA5807M_REG[RADIO_REG_R2] = 0x0000;   // all bits off
    RDA5807M_saveRegisters(module);
}

// get the Chip ID
u16 RDA5807M_getChipID(u8 module)
{
    RDA5807M_readRegister(module, RADIO_REG_R0);
    return (RDA5807M_REG[RADIO_REG_R0] == RADIO_CHIP_ID);
}

// ----- Volume control -----

void RDA5807M_setVolume(u8 module, u8 volume)
{
    RDA5807M_writeRegister(module, RADIO_REG_R5, volume & RADIO_REG_R5_VOL_MASK);
}

u8 RDA5807M_getVolume(u8 module)
{
    RDA5807M_readRegister(module, RADIO_REG_R5);
    RDA5807M.volume = RDA5807M_REG[RADIO_REG_R5] & RADIO_REG_R5_VOL_MASK;
    return RDA5807M.volume;
}

u8 RDA5807M_getActive(u8 module)
{
    RDA5807M.active = true;
    return RDA5807M.active;
}

void RDA5807M_setBassBoost(u8 module, u8 switchOn)
{
    RDA5807M.bassBoost = switchOn;
    if (switchOn)
        RDA5807M_REG[RADIO_REG_R2] |= RADIO_REG_R2_BASS;
    else
        RDA5807M_REG[RADIO_REG_R2] &= (~RADIO_REG_R2_BASS);
    RDA5807M_saveRegister(module, RADIO_REG_R2);
    //RDA5807M_saveRegisters(module);
}

// 1 = Bass boost is set
// 0 = Bass boost is not set
u8 RDA5807M_getBassBoost(u8 module)
{
    RDA5807M_readRegister(module, RADIO_REG_R2);
    //RDA5807M_readRegisters(module);
    RDA5807M.bassBoost = RDA5807M_REG[RADIO_REG_R2] & RADIO_REG_R2_BASS;
    return RDA5807M.bassBoost;
}

// Mono / Stereo
void RDA5807M_setMono(u8 module, u8 switchOn)
{
    RDA5807M_REG[RADIO_REG_R2] &= (~RADIO_REG_R2_SEEK);
    if (switchOn)
        RDA5807M_REG[RADIO_REG_R2] |= RADIO_REG_R2_MONO;
    else
        RDA5807M_REG[RADIO_REG_R2] &= (~RADIO_REG_R2_MONO);
    RDA5807M_saveRegister(module, RADIO_REG_R2);
    //RDA5807M_saveRegisters(module);
}

// 1 = the current channel is mono
// 0 = the current channel is not mono
u8 RDA5807M_getMono(u8 module)
{
    RDA5807M_readRegister(module, RADIO_REG_R2);
    RDA5807M.mono = RDA5807M_REG[RADIO_REG_R2] & RADIO_REG_R2_MONO;
    return RDA5807M.mono;
}

// 1 = the current channel is stereo
// 0 = the current channel is not stereo
// *** BUG ***
u8 RDA5807M_getStereo(u8 module)
{
    RDA5807M_readRegister(module, RADIO_REG_RA);
    //RDA5807M_readRegisters(module);
    RDA5807M.stereo = RDA5807M_REG[RADIO_REG_RA] & RADIO_REG_RA_STEREO;
    return RDA5807M.stereo;
}

// 1 = the current channel is a station
// 0 = the current channel is not a station
u8 RDA5807M_getTuned(u8 module)
{
    RDA5807M_readRegister(module, RADIO_REG_RB);
    //RDA5807M_readRegisters(module);
    RDA5807M.tuned = RDA5807M_REG[RADIO_REG_RB] & RADIO_REG_RB_FMTRUE;
    return RDA5807M.tuned;
}

#define RDA5807M_getStation(module) RDA5807M_getTuned(module)

// Switch mute mode.
void RDA5807M_setMute(u8 module, u8 switchOn)
{
    if (switchOn)    // now don't unmute
        RDA5807M_REG[RADIO_REG_R2] &= (~RADIO_REG_R2_UNMUTE);
    else             // now unmute
        RDA5807M_REG[RADIO_REG_R2] |= RADIO_REG_R2_UNMUTE;
    RDA5807M_saveRegister(module, RADIO_REG_R2);
    //RDA5807M_saveRegisters(module);
}

u8 RDA5807M_getMute(u8 module)
{
    RDA5807M_readRegister(module, RADIO_REG_R2);
    //RDA5807M_readRegisters(module);
    if (RDA5807M_REG[RADIO_REG_R2] & RADIO_REG_R2_UNMUTE)
        RDA5807M.mute = false;
    else
        RDA5807M.mute = true;
    
    return RDA5807M.mute;
}

// Set the soft mute mode (mute on low signals) on or off.
void RDA5807M_setSoftMute(u8 module, u8 switchOn)
{
    if (switchOn)
        RDA5807M_REG[RADIO_REG_R4] |= (RADIO_REG_R4_SOFTMUTE);
    else
        RDA5807M_REG[RADIO_REG_R4] &= (~RADIO_REG_R4_SOFTMUTE);
    RDA5807M_saveRegister(module, RADIO_REG_R4);
    //RDA5807M_saveRegisters(module);
}

u8 RDA5807M_getSoftmute(u8 module)
{
    RDA5807M_readRegister(module, RADIO_REG_R4);
    //RDA5807M_readRegisters(module);
    RDA5807M.softMute = RDA5807M_REG[RADIO_REG_R4] & RADIO_REG_R4_SOFTMUTE;
    return RDA5807M.softMute;
}

// ----- Band and frequency control methods -----

// tune to new band.
void RDA5807M_setBand(u8 module, RADIO_BAND band)
{
    switch(band)
    {
        case RADIO_BAND_FM:
            RDA5807M.freqLow = 8700;
            RDA5807M.freqHigh = 10800;
            RDA5807M_REG[RADIO_REG_R3] |= RADIO_REG_R3_BAND_FM;
            //RDA5807M_saveRegister(module, RADIO_REG_R3);
            break;

        case RADIO_BAND_FMJAPAN:
            RDA5807M.freqLow = 7600;
            RDA5807M.freqHigh = 9100;
            RDA5807M_REG[RADIO_REG_R3] |= RADIO_REG_R3_BAND_FMJAPAN;
            //RDA5807M_saveRegister(module, RADIO_REG_R3);
            break;

        case RADIO_BAND_FMEAST1:
            RDA5807M.freqLow = 6500;
            RDA5807M.freqHigh = 7600;
            RDA5807M_REG[RADIO_REG_R3] |= RADIO_REG_R3_BAND_FMEAST;
            //RDA5807M_saveRegister(module, RADIO_REG_R3);
            RDA5807M_REG[RADIO_REG_R7] |= RADIO_REG_R7_65M50M;  // 65~76 MHz;
            //RDA5807M_saveRegister(module, RADIO_REG_R7);
            break;

        case RADIO_BAND_FMEAST2:
            RDA5807M.freqLow = 5000;
            RDA5807M.freqHigh = 7600;
            RDA5807M_REG[RADIO_REG_R3] |= RADIO_REG_R3_BAND_FMEAST;
            //RDA5807M_saveRegister(module, RADIO_REG_R3);
            RDA5807M_REG[RADIO_REG_R7] &= (~RADIO_REG_R7_65M50M); // 50~76 MHz;
            //RDA5807M_saveRegister(module, RADIO_REG_R7);
            break;
    }
    RDA5807M_saveRegister(module, RADIO_REG_R3);
    RDA5807M_saveRegister(module, RADIO_REG_R7);
}

RADIO_BAND RDA5807M_getBand(u8 module)
{
    u8 band;
    
    RDA5807M_readRegister(module, RADIO_REG_R3);
    //RDA5807M_readRegisters(module);
    band = RDA5807M_REG[RADIO_REG_R3] & RADIO_REG_R3_BAND_MASK;
    if (band == RADIO_REG_R3_BAND_FM)
        RDA5807M.band = RADIO_BAND_FM;
    if (band == RADIO_REG_R3_BAND_FMWORLD)
        RDA5807M.band = RADIO_BAND_FMWORLD;
    return RDA5807M.band;
}

// Frequency = Channel Spacing (kHz) x RADIO_REG_R3[15:6] + RDA5807M.freqLow (MHz)
// The tune bit is reset to low automatically when the tune operation is complete
// so we set it high each time we set a new frequency.
void RDA5807M_setFrequency(u8 module, RADIO_FREQ freq)
{
    u16 ch;

    if (freq < RDA5807M.freqLow)  freq = RDA5807M.freqLow;
    if (freq > RDA5807M.freqHigh) freq = RDA5807M.freqHigh;
    ch = ((freq - RDA5807M.freqLow) / RDA5807M.freqSteps) * 10;
    ch &= RADIO_REG_R3_FREQ_MASK;
    ch <<= 6;

    //RDA5807M_writeRegister(module, RADIO_REG_R2, RADIO_REG_R2_OUTPUT|RADIO_REG_R2_UNMUTE|RADIO_REG_R2_RDS|RADIO_REG_R2_ENABLE);
    RDA5807M_writeRegister(module, RADIO_REG_R3, ch | RADIO_REG_R3_TUNE);
}

// Retrieve the real frequency from the chip after automatic tuning.
// Frequency = Channel Spacing (kHz) x RADIO_REG_RA[9:0] + RDA5807M.freqLow (MHz)
RADIO_FREQ RDA5807M_getFrequency(u8 module)
{
    u16 ch;
    
    // check register A
    ch = RDA5807M_readRegister(module, RADIO_REG_RA) & RADIO_REG_RA_FREQ_MASK;
    RDA5807M.freq = RDA5807M.freqLow + (ch * RDA5807M.freqSteps) / 10;
    
    return RDA5807M.freq;
}

RADIO_FREQ RDA5807M_getMinFrequency(u8 module)
{
    return RDA5807M.freqLow;
}

RADIO_FREQ RDA5807M_getMaxFrequency(u8 module)
{
    return RDA5807M.freqHigh;
}

// step : 25, 50, 100 or 200 kHz
void RDA5807M_setSpacing(u8 module, u8 step)
{
    u8 spacing;
    RDA5807M.freqSteps = step;
    switch (step)
    {
        case 25:
            spacing = RADIO_REG_R3_SPACE_25; break;
        case 50:
            spacing = RADIO_REG_R3_SPACE_50; break;
        case 100:
            spacing = RADIO_REG_R3_SPACE_100; break;
        case 200:
            spacing = RADIO_REG_R3_SPACE_200; break;
    }
    RDA5807M_writeRegister(module, RADIO_REG_R3, spacing);
}

u8 RDA5807M_getSpacing(u8 module)
{
    return (RDA5807M.freqSteps/10);
}

// format the current frequency for display and printing
// return a pointer on the formated string
//void RDA5807M_formatFrequency(char *s, u8 length)
u8* RDA5807M_formatedFrequency(u8 module)
{
    RADIO_BAND b = RDA5807M_getBand(module);
    RADIO_FREQ f = RDA5807M_getFrequency(module);

    //if ((s != NULL) && (length > 10))
    *RDA5807M.formatedFreq = '\0';

    if ((b == RADIO_BAND_FM) || (b == RADIO_BAND_FMWORLD))
    {
        // int16 to " ffff" or "fffff"
        RDA5807M_int16_to_s(RDA5807M.formatedFreq, (u16)f);

        // Uncomment if 2 digit after decimal point are needed
        //RDA5807M.formatedFreq[5] = RDA5807M.formatedFreq[4];

        // set only 1 digit after decimal point
        RDA5807M.formatedFreq[4] = RDA5807M.formatedFreq[3];

        // insert decimal point : " ff.f" or "fff.f"
        RDA5807M.formatedFreq[3] = '.';
        
        // append units : " ff.f MHz" or "fff.f MHz"
        //strcpy(RDA5807M.formatedFreq + 6, " MHz");
    }
    
    return RDA5807M.formatedFreq;
}

// This is a special format routine used to format
// frequencies as strings with leading blanks.
// up to 5 digits only ("    0".."99999")
// *s MUST be able to hold the characters
void RDA5807M_int16_to_s(char *s, u16 val)
{
    u8 n = 5;

    while (n > 0)
    {
        n--;
        if ((n == 4) || (val > 0))
        {
            s[n] = '0' + (val % 10);
            val = val / 10;
        }
        else
        {
            s[n] = ' ';
        }
    }
}

// start seek mode upwards
void RDA5807M_seekUp(u8 module, u8 toNextSender)
{
    RDA5807M_writeRegister(module, RADIO_REG_R2, RADIO_REG_R2_SEEK|RADIO_REG_R2_SEEKUP);

    if (!toNextSender)
    {
        // stop scanning right now
        RDA5807M_REG[RADIO_REG_R2] &= (~RADIO_REG_R2_SEEK);
        RDA5807M_saveRegister(module, RADIO_REG_R2);
    }
}

// start seek mode downwards
void RDA5807M_seekDown(u8 module, u8 toNextSender)
{
    RDA5807M_REG[RADIO_REG_R2] &= (~RADIO_REG_R2_SEEKUP);
    RDA5807M_REG[RADIO_REG_R2] |= RADIO_REG_R2_SEEK;
    RDA5807M_saveRegister(module, RADIO_REG_R2);

    if (!toNextSender)
    {
        // stop scanning right now
        RDA5807M_REG[RADIO_REG_R2] &= (~RADIO_REG_R2_SEEK);
        RDA5807M_saveRegister(module, RADIO_REG_R2);
    }
}

/*
 * There is no visible register address in I2C interface transfers.
 * The I2C interface has a fixed start register address (0x02h) for
 * write transfer and 0x0Ah for read transfer),
 * and an internal incremental address counter.
 * If register address meets the end of register file, 0x3Ah,
 * register address will wrap back to 0x00h.
 * For write transfer, MCU programs registers from register 0x02h high byte,
 * then register 0x02h low byte, then register 0x03h high byte, till the last register.
 * RDA5807M always gives out ACK after every byte, and MCU gives out
 * STOP condition when register programming is finished.
 * For read transfer, after command byte from MCU, RDA5807M sends out
 * register 0x0Ah high byte, then register 0x0Ah low byte, then register
 * 0x0Bh high byte, till receives NACK from MCU. 
*/

// Load RDA5807M_REG from the chip in random mode (slaveaddress 0x11),
// reading starts at a previously selected Register (reg) address
u16 RDA5807M_readRegister(u8 module, u8 reg)
{
    I2C_start(module);
    I2C_write(module, (RADIO_I2C_RND_ADDR << 1) & 0xFE); // write : bit 0 = 0
    I2C_write(module, reg);
    //I2C_stop(module);

    I2C_restart(module);
    I2C_write(module, (RADIO_I2C_RND_ADDR << 1) | 0x01); // read : bit 0 = 1
    RDA5807M_REG[reg] = I2C_read(module) << 8;
    I2C_sendAck(module);
    RDA5807M_REG[reg] |= I2C_read(module);
    I2C_sendNack(module);
    I2C_stop(module);

    return RDA5807M_REG[reg];
}

// Load RDA5807M_REG from the RDA5807M chip
// using sequential mode (slaveaddress 0x10)
// from register 0x0A to the end of register file (0x3A)
// first register 0x0Ah high byte, then register 0x0Ah low byte,
// then register 0x0Bh high byte, ...
// till RDA5807M receives NACK from MCU. 
void RDA5807M_readRegisters(u8 module)
{
    u8 reg, high, low;
    
    I2C_start(module);
    I2C_write(module, (RADIO_I2C_SEQ_ADDR << 1) | 0x01); // read : bit 0 = 1

    // read from 0x0A to 0x3A
    for (reg = RADIO_REG_FIRST_READ; reg < RADIO_REG_LAST_READ; reg++)
    {
        high = I2C_read(module);
        I2C_sendAck(module);
        low = I2C_read(module);
        I2C_sendAck(module);
        
        if (reg < 0x10)
            RDA5807M_REG[reg] = (u16)(high << 8) | low;
    }

    // then read from 0x00 to 0x09
    for (reg = RADIO_REG_R0; reg < RADIO_REG_FIRST_READ; reg++)
    {
        high = I2C_read(module);
        I2C_sendAck(module);
        low = I2C_read(module);
        if (reg == (RADIO_REG_FIRST_READ - 1))
            I2C_sendNack(module);
        else
            I2C_sendAck(module);
        
        RDA5807M_REG[reg] = (u16)(high << 8) | low;
    }

    I2C_stop(module);
}

// Write a register value (2 bytes)
// using the random write access mode
void RDA5807M_writeRegister(u8 module, u8 reg, u16 val)
{
    RDA5807M_REG[reg] |= val;

    I2C_start(module);
    I2C_write(module, (RADIO_I2C_RND_ADDR << 1) & 0xFE); // write : bit 0 = 0
    I2C_write(module, reg);
    I2C_write(module, val >> 8);
    I2C_write(module, val & 0xFF);
    I2C_stop(module);
}

// Save one register back to the chip
// using the random write access mode
void RDA5807M_saveRegister(u8 module, u8 reg)
{
    RDA5807M_writeRegister(module, reg, RDA5807M_REG[reg]);
}

/*
 * Save the 5 writable registers (0x02 through 0x07) back to the chip.
 * There is no visible register address in I2C interface transfers.
 * The I2C interface has a fixed start register address (0x02h) for
 * write transfer and an internal incremental address counter.
 * If register address meets the end of register file, 0x3Ah,
 * register address will wrap back to 0x00h.
 * For write transfer, MCU programs registers from register 0x02h high byte,
 * then register 0x02h low byte, then register 0x03h high byte, till the last register.
 * RDA5807M always gives out ACK after every byte, and MCU gives out
 * STOP condition when register programming is finished.
*/
void RDA5807M_saveRegisters(u8 module)
{
    u8 reg;
    
    I2C_start(module);
    I2C_write(module, (RADIO_I2C_SEQ_ADDR << 1) & 0xFE); // write : bit 0 = 0
    for (reg = 0x02; reg < 0x08; reg++)
    {
        I2C_write(module, RDA5807M_REG[reg] >> 8);
        I2C_write(module, RDA5807M_REG[reg] & 0xFF);
    }
    I2C_stop(module);
}

// return current Radio Station Strength Information
// RSSI scale is logarithmic.
// from 0b000000 = min to 0b111111 = max
u8 RDA5807M_getStrength(u8 module)
{
    RDA5807M_readRegister(module, RADIO_REG_RB);
    //RDA5807M_readRegisters(module);
    RDA5807M.rssi = RDA5807M_REG[RADIO_REG_RB] >> 10;
    return RDA5807M.rssi;
}

u8 RDA5807M_getRDS(u8 module)
{
    u16 A,B,C,D;
    u8  result = false;

    // check for new RDS data available
    //RDA5807M_readRegister(module, RADIO_REG_RA);
    RDA5807M_readRegisters(module);
    A = RDA5807M_REG[RADIO_REG_RDSA];
    B = RDA5807M_REG[RADIO_REG_RDSB];
    C = RDA5807M_REG[RADIO_REG_RDSC];
    D = RDA5807M_REG[RADIO_REG_RDSD];
    
    if (RDA5807M_REG[RADIO_REG_RA] & RADIO_REG_RA_RDS)
    {
        RDA5807M.rds = true;
        RDA5807M_readRegisters(module);
        
        if (A != RDA5807M_REG[RADIO_REG_RDSA])
            result = true;
        if (B != RDA5807M_REG[RADIO_REG_RDSB])
            result = true;
        if (C != RDA5807M_REG[RADIO_REG_RDSC])
            result = true;
        if (D != RDA5807M_REG[RADIO_REG_RDSD])
            result = true;

        // If there is new data, send it to the RDS decoder
        if (result)
            RDA5807M_processRDS(A, B, C, D);
    }
    else
        RDA5807M.rds = false;

    return RDA5807M.rds;
}

/// Send a 0.0.0.0 to the RDS receiver if there is any attached.
/// This is to point out that there is a new situation and all existing data should be invalid from now on.
void RDA5807M_clearRDS()
{ 
    RDA5807M_processRDS(0, 0, 0, 0);
}

// send valid and good data to the RDS processor via frequnction
// remember the RDS function
/*
void RDA5807M_attachReceiveRDS(receiveRDSFunction frequnction)
{
    RDA5807M.sendRDS = frequnction;
}
*/

void RDA5807M_processRDS(u16 block1, u16 block2, u16 block3, u16 block4)
{
    u8 idx;     // index of radioText
    u8 h, m;    // hours, minutes
    u8 c1, c2;  // characters

    u16 mins;   // RDS time in minutes
    u8 off;     // RDS time offset and sign

    u8 rdsGroupType;
    //u8 rdsTP;           // Traffic Program
    //u8 rdsPTY;          // Program Type (ex. sport)
    u8 textAB;


    if (block1 == 0)
    {
        // Reset all the RDS info.
        strcpy(RDA5807M.radioName1, "--------");
        strcpy(RDA5807M.radioName2, "--------");
        strcpy(RDA5807M.radioName,  "        ");
        memset(RDA5807M.radioText, 0, sizeof(RDA5807M.radioText));
        RDA5807M.lastTextIDX = 0;
        // Send out empty data
        return;
    }

    // Analyzing Block 2
    rdsGroupType = 0x0A | ((block2 & 0xF000) >> 8) | ((block2 & 0x0800) >> 11);
    //RDA5807M.rdsTP = (block2 & 0x0400);
    //RDA5807M.rdsPTY = (block2 & 0x0400);

    switch (rdsGroupType)
    {
        // -------------------------------------------------------------
        // The data received is part of the Service Station Name
        // -------------------------------------------------------------
        case 0x0A:
        case 0x0B:
            idx = 2 * (block2 & 0x0003);

            // new data is 2 chars from block 4
            c1 = block4 >> 8;
            c2 = block4 & 0x00FF;

            // check that the data was received successfully twice
            // before publishing the station name

            if ((RDA5807M.radioName1[idx] == c1) && (RDA5807M.radioName1[idx + 1] == c2))
            {
                // retrieved the text a second time: store to radioName2
                RDA5807M.radioName2[idx] = c1;
                RDA5807M.radioName2[idx + 1] = c2;
                RDA5807M.radioName2[8] = '\0';

                // save station name
                if ((idx == 6) && strcmp(RDA5807M.radioName1, RDA5807M.radioName2) == 0)
                    if (strcmp(RDA5807M.radioName2, RDA5807M.radioName) != 0)
                        strcpy(RDA5807M.radioName, RDA5807M.radioName2);
            }

            if ((RDA5807M.radioName1[idx] != c1) || (RDA5807M.radioName1[idx + 1] != c2))
            {
                RDA5807M.radioName1[idx] = c1;
                RDA5807M.radioName1[idx + 1] = c2;
                RDA5807M.radioName1[8] = '\0';
            }
            break;

        // -------------------------------------------------------------
        // The data received is part of the RDS Text.
        // -------------------------------------------------------------
        case 0x2A:
            textAB = (block2 & 0x0010);
            idx = 4 * (block2 & 0x000F);

            if (idx < RDA5807M.lastTextIDX)
            {
                // the existing text might be complete
                // because the index is starting at the beginning again.
                // now send it to the possible listener.
                //if (_sendText)
                //    _sendText(_radioText);
            }
            RDA5807M.lastTextIDX = idx;

            if (textAB != RDA5807M.lastTextAB)
            {
                // when this bit is toggled the whole buffer should be cleared.
                RDA5807M.lastTextAB = textAB;
                memset(RDA5807M.radioText, 0, sizeof(RDA5807M.radioText));
            }

            // new data is 2 chars from block 3
            RDA5807M.radioText[idx] = (block3 >> 8);
            idx++;
            RDA5807M.radioText[idx] = (block3 & 0x00FF);
            idx++;

            // new data is 2 chars from block 4
            RDA5807M.radioText[idx] = (block4 >> 8);
            idx++;
            RDA5807M.radioText[idx] = (block4 & 0x00FF);
            idx++;

            break;

        // -------------------------------------------------------------
        // The data received is clock time and date
        // -------------------------------------------------------------
        case 0x4A:
            off =  block4 & 0x3F; // 6 bits
            mins = (block4 >> 6) & 0x3F; // 6 bits
            mins += 60 * (((block3 & 0x0001) << 4) | ((block4 >> 12) & 0x0F));

            // adjust offset
            if (off & 0x20)
                mins -= 30 * (off & 0x1F);
            else
                mins += 30 * (off & 0x1F);

            if (mins != RDA5807M.lastRDSMinutes)
            {
                RDA5807M.lastRDSMinutes = mins;
                h = mins / 60;
                m = mins % 60;
                RDA5807M.radioTime[0]= '0' + (h / 10);
                RDA5807M.radioTime[1]= '0' + (h % 10);
                RDA5807M.radioTime[2]= ':';
                RDA5807M.radioTime[3]= '0' + (m / 10);
                RDA5807M.radioTime[4]= '0' + (m % 10);
                RDA5807M.radioTime[5]= '\0';
                RDA5807M.radioTime[6]= '\0';
              }

            break;

        // -------------------------------------------------------------
        // TODO
        // -------------------------------------------------------------
        /*
        case 0x6A:
            // IH
            break;

        case 0x8A:
            // TMC
            break;

        case 0xAA:
            // TMC
            break;

        case 0xCA:
            // TMC
            break;

        case 0xEA:
            // IH
            break;

        default:
            // Serial.print("RDS_GRP:"); Serial.println(rdsGroupType, HEX);
            break;
        */
    }
}

u8* RDA5807M_getRadioName(u8 module)
{
    return (u8*)RDA5807M.radioName;
}

u8* RDA5807M_getRadioText(u8 module)
{
    return (u8*)RDA5807M.radioText;
}

u8* RDA5807M_getRadioTime(u8 module)
{
    return (u8*)RDA5807M.radioTime;
}

/*
u8  RDA5807M_getRadioProgramType(u8 module)
{
    return RDA5807M.radioProgramType;
}
*/
