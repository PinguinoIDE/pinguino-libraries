/*  --------------------------------------------------------------------
    FILE:        TM16XX.c
    PROJECT:     Pinguino
    PURPOSE:     Library implementation for TM16XX displays
    PROGRAMERS:  Ricardo Batista <rjbatista@gmail.com>
                 Regis blanchot <rblanchot@gmail.com>
    --------------------------------------------------------------------
    CHANGELOG
                 - Ricardo Batista - first release
    30 Dec. 2016 - Régis Blanchot  - fixed for Pinguino 8- and 32-bit
    02 Jan. 2017 - Régis Blanchot  - added new functions
    --------------------------------------------------------------------
    This program is free software: you can redistribute it and/or modify
    it under the terms of the version 3 GNU General Public License as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    ------------------------------------------------------------------*/

#ifndef TM16XX_C
#define TM16XX_C

#include <typedef.h>
#include <const.h>
#include <macro.h>
#include <TM16XX.h>

#ifdef __PIC32MX__
#include <digitalw.c>
#else
#include <digital.h>
#include <digitalw.c>
#include <digitalr.c>
#include <digitalp.c>
#endif

// Printf
#ifdef TM16XXPRINTF
    #include <printFormated.c>
#endif

// PrintFloat
#if defined(TM16XXPRINTFLOAT)
    #include <printFloat.c>
#endif

// PrintNumber
#if defined(TM16XXPRINTNUMBER) || defined(TM16XXPRINTFLOAT)
    #include <printNumber.c>
#endif

u8 gTM16XX_displays;
u8 gTM16XX_dataPin;
u8 gTM16XX_clockPin;
u8 gTM16XX_strobePin; 
u8 gTM16XX_intensity = 7;
u8 gTM16XX_pos = 0;
u8 gTM16XX_buffer[16];

/*  --------------------------------------------------------------------
    Init. Functions
    ------------------------------------------------------------------*/

void TM16XX_init(u8 displays, u8 dataPin, u8 clockPin, u8 strobePin)
{
    u8 i;

    gTM16XX_dataPin = dataPin;
    gTM16XX_clockPin = clockPin;
    gTM16XX_strobePin = strobePin;
    gTM16XX_displays = displays;

    pinmode(dataPin, OUTPUT);
    pinmode(clockPin, OUTPUT);
    pinmode(strobePin, OUTPUT);

    digitalwrite(strobePin, HIGH);
    digitalwrite(clockPin, HIGH);

    // Write Data to the display register
    TM16XX_sendCommand(TM16XX_DATACMD | TM16XX_WRITE);
    
    TM16XX_sendCommand(TM16XX_DISPCMD | TM16XX_DISPLAYON | gTM16XX_intensity);

    digitalwrite(strobePin, LOW);
    TM16XX_send(TM16XX_ADDRCMD);
    for (i = 0; i < 16; i++)
        TM16XX_send(0x00);

    digitalwrite(strobePin, HIGH);
}

void TM16XX_setIntensity(u8 intensity)
{
    u8 i;
    
    gTM16XX_intensity = intensity;

    digitalwrite(gTM16XX_strobePin, HIGH);
    digitalwrite(gTM16XX_clockPin, HIGH);

    // Write Data to the display register
    TM16XX_sendCommand(TM16XX_DATACMD | TM16XX_WRITE);
    
    TM16XX_sendCommand(TM16XX_DISPCMD | TM16XX_DISPLAYON | gTM16XX_intensity);

    digitalwrite(gTM16XX_strobePin, LOW);
    TM16XX_send(TM16XX_ADDRCMD);
    for (i = 0; i < 16; i++)
        TM16XX_send(0x00);

    digitalwrite(gTM16XX_strobePin, HIGH);
}

/*
void TM16XX_setFont(const u8* font)
{
    gTM16XX_font = font;
}

void TM16XX_setupDisplay(u8 active, u8 intensity)
{
    TM16XX_sendCommand(TM16XX_DISPCMD | (active ? TM16XX_DISPLAYON : TM16XX_DISPLAYOFF) | gTM16XX_intensity);

    // necessary for the TM1640
    digitalwrite(gTM16XX_strobePin, LOW);
    digitalwrite(gTM16XX_clockPin, LOW);
    digitalwrite(gTM16XX_clockPin, HIGH);
    digitalwrite(gTM16XX_strobePin, HIGH);
}

void TM16XX_setDisplay(u8 *values, u8 len)
{
    u8 i;
    
    for (i = 0; i < len; i++)
        TM16XX_sendChar(i, values[i], 0);
}

void TM16XX_setDisplayDigit(u8 digit, u8 pos, u8 dot, const u8 *numberFont)
{
    TM16XX_sendChar(pos, numberFont[digit & 0xF], dot);
}

void TM16XX_clearDisplay(u8 pos, u8 dot)
{
    TM16XX_sendChar(pos, 0, dot);
}

void TM16XX_clearAll()
{
    u8 i; 
    
    for (i = 0; i < gTM16XX_displays; i++)
        TM16XX_sendData(i << 1, 0);
}

*/

/*  --------------------------------------------------------------------
    Print Functions
    ------------------------------------------------------------------*/

/*
void TM16XX_sendChar(u8 pos, u8 data, u8 dot)
{
    TM16XX_sendData(pos << 1, data | (dot ? 0b10000000 : 0));
}

void TM16XX_setDisplayToError()
{
    u8 i;
   
    TM16XX_setDisplay(ERROR_DATA, 8);
    for (i = 8; i < gTM16XX_displays; i++)
        TM16XX_clearDisplayDigit(i, 0);
}

void TM16XX_setDisplayToString(const u8* string, const word dots, const u8 pos, const u8 *font)
{
    u8 i;
    
    for (i = 0; i < gTM16XX_displays - pos; i++)
    {
        if (string[i] != '\0')
            TM16XX_sendChar(i + pos, font[string[i] - 32], (dots & (1 << (gTM16XX_displays - i - 1))) != 0);
        else
            break;
    }
}
*/

#define TM16XX_sendChar(pos, c, dot)    TM16XX_sendData(pos << 1, c | (dot ? 0b10000000 : 0))
#define TM16XX_clearDisplay(n)          TM16XX_sendData(n << 1, 0)

void TM16XX_clearAll()
{
    u8 i; 
    
    for (i = 0; i < gTM16XX_displays; i++)
        TM16XX_clearDisplay(i);
    gTM16XX_pos = 0;
}

void TM16XX_printChar(u8 c)
{
    if (gTM16XX_pos >> gTM16XX_displays)
        gTM16XX_pos = 0;

    if (c == '.')
    {
        c = gTM16XX_buffer[gTM16XX_pos - 1];
        TM16XX_sendChar(gTM16XX_pos - 1, c, 1);
    }
    else
    {
        gTM16XX_buffer[gTM16XX_pos] = c;
        TM16XX_sendData(gTM16XX_pos << 1, TM16XX_font[c - 32]);
        gTM16XX_pos++;
    }
}
    
void TM16XX_print(const u8* string)
{
    while (*string)
        TM16XX_printChar(*string++);
}

#if defined(TM16XXPRINTNUMBER) || defined(TM16XXPRINTFLOAT)
void TM16XX_printNumber(s32 value, u8 base)
{
    /*
    if (abs(value) > 99999999L)
    {
        //TM16XX_setDisplayToError();
        TM16XX_clearAll();
        TM16XX_print("Error");
    }
    else
    */
        printNumber(TM16XX_printChar, value, base);
}
#endif

#if defined(TM16XXPRINTFLOAT)
void TM16XX_printFloat(float number, u8 digits)
{ 
    printFloat(TM16XX_printChar, number, digits);
}
#endif

#if defined(TM16XXPRINTF)
void TM16XX_printf(const u8 *fmt, ...)
{
    u8 *str=(char*)&gTM16XX_buffer;
    va_list	args;

    va_start(args, fmt);
    psprintf2(gTM16XX_buffer, fmt, args);
    va_end(args);

    while (*str)
        TM16XX_printChar(*str++);
}
#endif

/*
void TM16XX_setDisplayToDecNumber(unsigned long number, u8 dots, u8 startingPos, u8 leadingZeros, u8 *numberFont)
{
    u8 i;

    if (number > 99999999L)
    {
        //TM16XX_setDisplayToError();
        TM16XX_clearAll();
        TM16XX_print("Error");
    }
    else
    {
        for (i = 0; i < gTM16XX_displays - startingPos; i++)
        {
            if (number != 0)
            {
                TM16XX_setDisplayDigit(number % 10, gTM16XX_displays - i - 1, (dots & (1 << i)) != 0, numberFont);
                number = number / 10;
            }
            else
            {
                if (leadingZeros)
                    TM16XX_setDisplayDigit(0, gTM16XX_displays - i - 1, (dots & (1 << i)) != 0, numberFont);
                else
                    TM16XX_clearDisplayDigit(gTM16XX_displays - i - 1, (dots & (1 << i)) != 0);
            }
        }
    }
}

void TM16XX_setDisplayToHexNumber(unsigned long number, char dots, char leadingZeros, char *numberFont)
{
    int i;
    for (i = 0; i < gTM16XX_displays; i++)
    {
        if (!leadingZeros && number == 0)
            TM16XX_clearDisplayDigit(gTM16XX_displays - i - 1, (dots & (1 << i)) != 0);
        else
        {
            TM16XX_setDisplayDigit(number & 0xF, gTM16XX_displays - i - 1, (dots & (1 << i)) != 0, numberFont);
            number >>= 4;
        }
    }
}

void TM16XX_setDisplayToSignedDecNumber(signed long number, char dots, char leadingZeros, char *numberFont)
{
    if (number >= 0)
    {
        TM16XX_setDisplayToDecNumberAt(number, dots, 0, leadingZeros, numberFont);
    }
    else
    {
        if (-number > 9999999L)
        {
            TM16XX_setDisplayToError();
        }
        else
        {
            TM16XX_setDisplayToDecNumberAt(-number, dots, 1, leadingZeros, numberFont);
            TM16XX_sendChar(0, MINUS, (dots & (0x80)) != 0);
            //TM16XX_sendData(pos << 1, data | (dot ? 0b10000000 : 0));
        }
    }
}

void TM16XX_setDisplayToBinNumber(char number, char dots, char *numberFont)
{
    int i;

    for (i = 0; i < gTM16XX_displays; i++)
        TM16XX_setDisplayDigit((number & (1 << i)) == 0 ? 0 : 1, gTM16XX_displays - i - 1, (dots & (1 << i)) != 0, numberFont);
}
*/

/*  --------------------------------------------------------------------
    Core Functions
    ------------------------------------------------------------------*/

void TM16XX_sendCommand(u8 cmd)
{
    digitalwrite(gTM16XX_strobePin, LOW);
    TM16XX_send(cmd);
    digitalwrite(gTM16XX_strobePin, HIGH);
}

void TM16XX_sendData(u8 address, u8 data)
{
    TM16XX_sendCommand(TM16XX_DATACMD|TM16XX_FIXEDADDR);
    digitalwrite(gTM16XX_strobePin, LOW);
    TM16XX_send(TM16XX_ADDRCMD | address);
    TM16XX_send(data);
    digitalwrite(gTM16XX_strobePin, HIGH);
}

void TM16XX_send(u8 data)
{
    u8 i;
    
    for (i = 0; i < 8; i++)
    {
        digitalwrite(gTM16XX_clockPin, LOW);
        digitalwrite(gTM16XX_dataPin, data & 1 ? HIGH : LOW);
        data >>= 1;
        digitalwrite(gTM16XX_clockPin, HIGH);
    }
}

u8 TM16XX_receive()
{
    u8 i, temp = 0;

    // Pull-up on
    pinmode(gTM16XX_dataPin, INPUT);
    digitalwrite(gTM16XX_dataPin, HIGH);

    for (i = 0; i < 8; i++)
    {
        temp >>= 1;

        digitalwrite(gTM16XX_clockPin, LOW);
        if (digitalread(gTM16XX_dataPin))
            temp |= TM16XX_DISPCMD;
        digitalwrite(gTM16XX_clockPin, HIGH);
    }

    // Pull-up off
    pinmode(gTM16XX_dataPin, OUTPUT);
    digitalwrite(gTM16XX_dataPin, LOW);

    return temp;
}

void TM16XX_setLED(u8 pos, u8 color)
{
    TM16XX_sendData((pos << 1) + 1, color);
}

void TM16XX_setLEDs(unsigned int leds)
{
    u8 i, color;

    for (i = 0; i < gTM16XX_displays; i++)
    {
        color = 0;

        if ((leds & (1 << i)) != 0)
            color |= TM16XX_RED;

        if ((leds & (1 << (i + 8))) != 0)
            color |= TM16XX_GREEN;

        TM16XX_setLED(color, i);
    }
}

int TM16XX_getButtons(void)
{
    u8 i;
    int keys = 0;

    digitalwrite(gTM16XX_strobePin, LOW);
    TM16XX_send(TM16XX_DATACMD | TM16XX_READKEY);
    for (i = 0; i < 4; i++)
        keys |= TM16XX_receive() << i;
    digitalwrite(gTM16XX_strobePin, HIGH);

    if (keys < -64)
        keys = 128;
    else
        keys = keys * -1;

    return keys;
}

#endif // TM16XX_C
