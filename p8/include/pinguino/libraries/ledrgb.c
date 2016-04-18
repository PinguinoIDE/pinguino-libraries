/*	--------------------------------------------------------------------
    FILE:			ledrgb.c
    PROJECT:		pinguino
    PURPOSE:		RGB led driver
    PROGRAMER:		Régis Blanchot <rblanchot@gmail.com>
    --------------------------------------------------------------------
    Changelog :
    18 Apr. 2016 - Régis Blanchot - first release
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

#ifndef __LEDRGB__
#define __LEDRGB__

#include <compiler.h>       // sfr's
#include <typedef.h>        // u8, u16, u32, ...
#include <swpwm.c>          // Software PWM
#include <delayms.c>        // Delayms

//#define COMMON_ANODE
#define SMOOTH_MODE

u8 gRGBPin[8][3];

// PWM look-up table to linearize the output light
#ifdef SMOOTH_MODE
const u8 SWPWM_LUT[64]= {
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
    16,17,18,19,20,21,22,23,24,25,26,28,30,32,34,36,
    38,40,42,44,46,48,50,52,54,56,58,60,63,66,69,72,75,
    78,81,84,87,90,94,98,102,106,110,114,118,122,126,128 };
#endif

#define LedRGB_setFrequency(freq)    SWPWM_setFrequency(freq)

void LedRGB_attach(u8 RGBLed, u8 redPin, u8 greenPin, u8 bluePin)
{
    gRGBPin[RGBLed][0] = redPin;
    gRGBPin[RGBLed][1] = greenPin;
    gRGBPin[RGBLed][2] = bluePin;
}

void LedRGB_setRGBColor(u8 RGBLed, u8 r, u8 g, u8 b)
{
    #ifdef COMMON_ANODE
    r = 255 - r;
    g = 255 - g;
    b = 255 - b;
    #endif

    #ifdef SMOOTH_MODE
    SWPWM_setDutyCycle(gRGBPin[RGBLed][0], SWPWM_LUT[r >> 2]);
    SWPWM_setDutyCycle(gRGBPin[RGBLed][1], SWPWM_LUT[r >> 2]);
    SWPWM_setDutyCycle(gRGBPin[RGBLed][2], SWPWM_LUT[r >> 2]);
    #else
    SWPWM_setDutyCycle(gRGBPin[RGBLed][0], r >> 1);
    SWPWM_setDutyCycle(gRGBPin[RGBLed][1], g >> 1);
    SWPWM_setDutyCycle(gRGBPin[RGBLed][2], b >> 1);
    #endif
}

void LedRGB_setHexColor(u8 RGBLed, u32 hexColor)
{
    u8 r = hexColor >> 16 & 0xFF;
    u8 g = hexColor >> 8  & 0xFF;
    u8 b = hexColor       & 0xFF;
    LedRGB_setRGBColor(RGBLed, r, g, b);
}

void LedRGB_setGradient(u8 RGBLed, u32 color1, u32 color2, u16 time)
{
    u8 r1 = color1 >> 16 & 0xFF;
    u8 g1 = color1 >> 8  & 0xFF;
    u8 b1 = color1       & 0xFF;

    u8 r2 = color2 >> 16 & 0xFF;
    u8 g2 = color2 >> 8  & 0xFF;
    u8 b2 = color2       & 0xFF;

    u8 r, g, b, percent;
            
    for (percent=0; percent<100; percent++)
    {
        r = r1 + percent * (r2 - r1) / 100;
        g = g1 + percent * (g2 - g1) / 100;
        b = b1 + percent * (b2 - b1) / 100;
        LedRGB_setRGBColor(RGBLed, r, g, b);
        Delayms(time);
    }
}

#endif /* __LEDRGB__ */
