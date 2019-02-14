///
/// \file RDA5807M.h
/// \brief Library header file for the radio library to control the RDA5807M radio chip.
///
/// \author Matthias Hertel, http://www.mathertel.de
/// \copyright Copyright (c) 2014-2015 by Matthias Hertel.\n
/// This work is licensed under a BSD style license.\n
/// See http://www.mathertel.de/License.aspx
/// 
/// \details
/// This library enables the use of the Radio Chip RDA5807M from http://www.rdamicro.com/ that supports FM radio bands and RDS data.
///
/// More documentation and source code is available at http://www.mathertel.de/Arduino
///
/// History:
/// --------
/// * 12.05.2014 creation of the RDA5807M library.
/// * 28.06.2014 running simple radio
/// * 08.07.2014 RDS data receive function can be registered.

// multi-Band enabled

// - - - - -
// help from: http://arduino.vom-kuhberg.de/index.php
//   http://projects.qi-hardware.com/index.php/p/qi-kernel/source/tree/144e9c2530f863e32a3538b06c63484401bbe314/drivers/media/radio/radio-rda5807.c


#ifndef RDA5807M_H
#define RDA5807M_H

#include <typedef.h>

// ----- Register Definitions -----

// Register file origins for sequential mode
#define RADIO_REG_FIRST_WRITE    0x02
#define RADIO_REG_FIRST_READ     0x0A
#define RADIO_REG_LAST_READ      0x3A

#define RADIO_REG_CHIPID            0x00
#define RADIO_REG_R0                0x00

#define RADIO_REG_R2                0x02
#define RADIO_REG_R2_OUTPUT         0x8000 // bit 15
#define RADIO_REG_R2_UNMUTE         0x4000 // bit 14
#define RADIO_REG_R2_MONO           0x2000 // bit 13
#define RADIO_REG_R2_BASS           0x1000 // bit 12
//      RADIO_REG_R2_RCLK                  // bit 11
//      RADIO_REG_R2_RCLK                  // bit 10
#define RADIO_REG_R2_SEEKUP         0x0200 // bit 9
#define RADIO_REG_R2_SEEK           0x0100 // bit 8
#define RADIO_REG_R2_SEEKMODE       0x0080 // bit 7
//      RADIO_REG_R2_CLOCK                 // bit[6:4]
#define RADIO_REG_R2_RDS            0x0008 // bit 3
#define RADIO_REG_R2_SENSITIVITY    0x0004 // bit 2
#define RADIO_REG_R2_SOFTRESET      0x0002 // bit 1
#define RADIO_REG_R2_ENABLE         0x0001 // bit 0

#define RADIO_REG_R3                0x03
#define RADIO_REG_R3_SPACE_100      0x0000 // 100 kHz
#define RADIO_REG_R3_SPACE_200      0x0001 // 200 kHz
#define RADIO_REG_R3_SPACE_50       0x0002 //  50 kHz
#define RADIO_REG_R3_SPACE_25       0x0003 //  25 kHz
#define RADIO_REG_R3_BAND_MASK      0x000C // bit[3:2]
#define RADIO_REG_R3_BAND_FM        0x0000 // 87-108 MHz (US/Europe)
#define RADIO_REG_R3_BAND_FMJAPAN   0x0004 // 76-91  MHz (Japan)
#define RADIO_REG_R3_BAND_FMWORLD   0x0008 // 76-108 MHz (World wide)
#define RADIO_REG_R3_BAND_FMEAST    0x000C // 65-76  MHz (East Europe)
#define RADIO_REG_R3_TUNE           0x0010 // bit 4
#define RADIO_REG_R3_TEST           0x0020 // bit 5
#define RADIO_REG_R3_FREQ_MASK      0x01FF // bit[9:0]

#define RADIO_REG_R4                0x04
#define RADIO_REG_R4_EM50           0x0800
//      RADIO_REG_R4_RES            0x0400
#define RADIO_REG_R4_SOFTMUTE       0x0200
#define RADIO_REG_R4_AFC            0x0100

#define RADIO_REG_R5                0x05
#define RADIO_REG_R5_VOLMAX         0x000F // bit[3:0]
#define RADIO_REG_R5_VOL_MASK       0x000F // bit[3:0]
#define RADIO_REG_R5_RSVD2          0x0000 // bit[5:4]
#define RADIO_REG_R5_SEEKTH         0x0800 // bit[11:8]
#define RADIO_REG_R5_RSVD1          0x0000 // bit[14:12]
#define RADIO_REG_R5_INTMODE        0x8000 // bit 15

#define RADIO_REG_R6                0x06

#define RADIO_REG_R7                0x07
#define RADIO_REG_R7_SOFTBLEND      0x4000
#define RADIO_REG_R7_65M50M         0x0200
#define RADIO_REG_R7_SOFTBLEND_EN   0x02

#define RADIO_REG_R8                0x08
#define RADIO_REG_R9                0x09

#define RADIO_REG_RA                0x0A
#define RADIO_REG_RA_RDS            0x8000
#define RADIO_REG_RA_STC            0x4000
#define RADIO_REG_RA_RDSBLOCK       0x0800
#define RADIO_REG_RA_STEREO         0x0400
#define RADIO_REG_RA_NR             0x03FF
#define RADIO_REG_RA_FREQ_MASK      0x01FF

#define RADIO_REG_RB                0x0B
#define RADIO_REG_RB_FMTRUE         0x0100
#define RADIO_REG_RB_FMREADY        0x0080

#define RADIO_REG_RDSA              0x0C
#define RADIO_REG_RDSB              0x0D
#define RADIO_REG_RDSC              0x0E
#define RADIO_REG_RDSD              0x0F

// The command byte includes a 7-bit chip address and a R/W bit. 
// RDA5807M have two access modes (sequential and random)
// each with its own slaveaddress.
// In sequential mode (slaveaddress 0x10)
// The I2C interface has a fixed start register address (0x02h
// for write transfer and 0x0Ah for read transfer), and
// an internal incremental address counter. 
#define RADIO_I2C_SEQ_ADDR          0x10
// In random mode (slaveaddress 0x11), reading starts at
// a previously selected Register address and autoincrements from there.
#define RADIO_I2C_RND_ADDR          0x11

#define RADIO_CHIP_ID               0x5804

// ----- type definitions -----

/// Band datatype.
/// The BANDs a receiver probably can implement.
/// RDA5807M chip only supports FM mode
typedef enum
{
    RADIO_BAND_NONE    = 0,         // No band selected.
    RADIO_BAND_FM      = 1,         // FM band 87.5 - 108 MHz (USA, Europe) selected.
    RADIO_BAND_FMWORLD = 2,         // FM band 76 - 108 MHz (Japan, Worldwide) selected.
    RADIO_BAND_FMJAPAN = 3,         // FM band 76 - 108 MHz (Japan, Worldwide) selected.
    RADIO_BAND_FMEAST1 = 4,         // FM band 76 - 108 MHz (Japan, Worldwide) selected.
    RADIO_BAND_FMEAST2 = 5,         // FM band 76 - 108 MHz (Japan, Worldwide) selected.
    RADIO_BAND_AM      = 6,         // AM band selected.
    RADIO_BAND_KW      = 7,         // KW band selected.

    RADIO_BAND_MAX     = 8          // Maximal band enumeration value.
}   RADIO_BAND;

/// callback function for passing RDS data.
//typedef void(*receiveRDSFunction)(u16 block1, u16 block2, u16 block3, u16 block4);

/// Frequency data type.
/// Only 16 bits are used for any frequency value (not the real one)
typedef u16 RADIO_FREQ;

/// A structure that contains information
/// about the radio and audio features from the chip.
typedef struct
{
    /// radio features
    u8 active;                      // Receiving is active.
    u8 rssi;                        // Radio Station Strength Information.
    u8 snr;                         // Signal Noise Ratio.
    u8 rds;                         // RDS information is available.
    u8 tuned;                       // A stable frequency is tuned.
    u8 mono;                        // Mono mode is on.
    u8 stereo;                      // Stereo audio is available

    /// audio features
    u8 volume;                      // Volume level.
    u8 bassBoost;                   // Bass Boost effect.
    u8 mute;                        // Mute effect.
    u8 softMute;                    // SoftMute effect.

    //char formatedFreq[12];          // Last formated frequency
    char formatedFreq[7];          // Last formated frequency

    RADIO_BAND band;                // Last set band.
    RADIO_FREQ freq;                // Last set frequency.
    RADIO_FREQ freqLow;             // Lowest frequency of the current selected band.
    RADIO_FREQ freqHigh;            // Highest frequency of the current selected band.
    RADIO_FREQ freqSteps;           // Resolution of the tuner.

    //receiveRDSFunction _sendRDS;  // Registered RDS Function that is called on new available data.

    /// RDS features
    u8 lastTextAB;
    u8 lastTextIDX;
    u16 lastRDSMinutes;             // Last RDS time send to callback.

    // Program Service Name
    char radioName1[8 + 2];         // including trailing '\00' character.
    char radioName2[8 + 2];         // including trailing '\00' character.
    char radioName [8 + 2];         // found station name or empty. Is max. 8 character long.

    // Radio Text
    char radioText[64 + 2];

    // Radio Time
    char radioTime[5 + 2];          // '12:34' + trailing '\00' character
}   RADIO_INFO;

// ----- library definition -----

u8 RDA5807M_init(u8 module);
void RDA5807M_close(u8 module);
u16 RDA5807M_getChipID(u8 module);
  
// ----- Audio features -----

void RDA5807M_setVolume(u8 module, u8 newVolume);
u8 RDA5807M_getVolume(u8 module);
void RDA5807M_setBassBoost(u8 module, u8 switchOn);
u8 RDA5807M_getBassBoost(u8 module);
void RDA5807M_setMono(u8 module, u8 switchOn);
u8 RDA5807M_getMono(u8 module);
void RDA5807M_setMute(u8 module, u8 switchOn);
u8 RDA5807M_getMute(u8 module);
void RDA5807M_setSoftMute(u8 module, u8 switchOn);
u8 RDA5807M_getSoftMute(u8 module);
u8 RDA5807M_getStrength(u8 module);
u8 RDA5807M_getTuned(u8 module);

// ----- Receiver features -----
void RDA5807M_setBand(u8 module, RADIO_BAND band);
RADIO_BAND RDA5807M_getBand(u8 module);
void RDA5807M_setFrequency(u8 module, RADIO_FREQ freq);
RADIO_FREQ RDA5807M_getFrequency(u8 module);
RADIO_FREQ RDA5807M_getMinFrequency(u8 module);
RADIO_FREQ RDA5807M_getMaxFrequency(u8 module);
u8 RDA5807M_getSpacing(u8 module);
void RDA5807M_setSpacing(u8 module, u8 step);
u8* RDA5807M_formatedFrequency(u8 module);
void RDA5807M_int16_to_s(char *s, u16 val);
void RDA5807M_seekUp(u8 module, u8 toNextSender);   // start seek mode upwards
void RDA5807M_seekDown(u8 module, u8 toNextSender); // start seek mode downwards

// ----- Supporting RDS for RADIO_BAND_FM and RADIO_BAND_FMWORLD

u8  RDA5807M_getRDS(u8 module);
u8* RDA5807M_getRadioName(u8 module);
u8* RDA5807M_getRadioText(u8 module);
u8* RDA5807M_getRadioTime(u8 module);
//u8  RDA5807M_getRadioProgramType(u8 module);

void RDA5807M_clearRDS();
void RDA5807M_processRDS(u16 block1, u16 block2, u16 block3, u16 block4);

// ----- Status functions -----

u8 RDA5807M_getActive(u8 module);
u8 RDA5807M_getStereo(u8 module);
u8 RDA5807M_getTuned(u8 module);
u8 RDA5807M_getMono(u8 module);

// ----- debug Helpers send information to Serial port

//void RDA5807M_debugScan(u8 module);               // Scan all frequencies and report a status
//void RDA5807M_debugStatus(u8 module);             // DebugInfo about actual chip data available

// ----- low level communication to the chip using I2C bus

u16  RDA5807M_readRegister(u8 module, u8 reg);
void RDA5807M_readRegisters(u8 module);             // Read all status & data registers
void RDA5807M_writeRegister(u8 module, u8 reg, u16 val);        // Write 16 Bit Value on I2C-Bus
void RDA5807M_saveRegister(u8 module, u8 reg);      // Save one register back to the chip
void RDA5807M_saveRegisters(u8 module);             // Save writable registers back to the chip

#endif
