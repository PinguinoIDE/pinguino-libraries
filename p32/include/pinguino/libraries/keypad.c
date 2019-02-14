/*  --------------------------------------------------------------------
    FILE:           keypad.c
    PROJECT:        pinguino
    PURPOSE:        matrix keypad management
    PROGRAMER:      Fabio Malagas <fabio.malagas@gmail.com>
    --------------------------------------------------------------------
    CHANGELOG :
    10 Apr. 2013 - Fabio Malagas  - first release
    03 Feb. 2016 - Régis Blanchot - adpated and fixed for 8-bit
    02 Nov. 2016 - Régis Blanchot - fixed the whole lib. according 
                                    https://electrosome.com/matrix-keypad-pic-microcontroller/
    --------------------------------------------------------------------
    TODO :
    * Keypad_isPressed(char keyChar)
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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
    ------------------------------------------------------------------*/

#ifndef _KEYPAD_C_
#define _KEYPAD_C_
#define __KEYPAD__

#include <typedef.h>
#include <const.h>

#ifndef __PIC32MX__
#include <digitalp.c>
#include <digitalr.c>
#endif
#include <digitalw.c>

#include <keypad.h>
#define  __MILLIS__
#include <millis.c>

/*  --------------------------------------------------------------------
    ---------- Global variables
    ------------------------------------------------------------------*/

Keypad keypad;

/*  --------------------------------------------------------------------
    ---------- getStatus
    --------------------------------------------------------------------
    Description : returns the keypad status
    ------------------------------------------------------------------*/

#define Keypad_getStatus()  (keypad.status)

/*  --------------------------------------------------------------------
    ---------- setStatus
    --------------------------------------------------------------------
    Description : returns the keypad status
    ------------------------------------------------------------------*/

#define Keypad_setStatus(s) keypad.status = (KeypadStatus)s

/*  --------------------------------------------------------------------
    ---------- setDebounceTime
    ------------------------------------------------------------------*/

#define Keypad_setDebounceTime(t)   keypad.debounceTime = (u16)t

/*  --------------------------------------------------------------------
    ---------- setHoldTime
    ------------------------------------------------------------------*/

#define Keypad_setHoldTime(t)   keypad.holdTime = (u16)t

/*  --------------------------------------------------------------------
    ---------- getKey
    ------------------------------------------------------------------*/

//#define Keypad_getKey()         (keypad.lastKey)

/*  --------------------------------------------------------------------
    ---------- init
    --------------------------------------------------------------------
    Description : Initialization
    Parameters :  keypadmap - pointer on a custom keymap
                  rowp - pointer on row pins
                  colp - pointer on column pins
                  rows, cols - keypad size
    Returns :     none
    ------------------------------------------------------------------*/

void Keypad_init(u8 *map, u8 *rowp, u8 *colp, u8 rows, u8 cols)
{
    u8 p;

    keypad.map = map;
    keypad.rowPins = rowp;
    keypad.columnPins = colp;

    keypad.rows = rows;
    keypad.columns = cols;

    keypad.lastKey = NO_KEY;
    keypad.debounceTime = 50;
    keypad.holdTime = 1000;
    keypad.lastUpdate = 0;
    keypad.status = IDLE;

    // all column connections are made High Impedance (INPUT)
    for (p = 0; p < keypad.columns; p++)
    {
        pinmode(keypad.columnPins[p], INPUT);
        digitalwrite(keypad.columnPins[p], LOW);
    }

    // all row connections are output pins
    for (p = 0; p < keypad.rows; p++)
    {
        pinmode(keypad.rowPins[p], OUTPUT);
        digitalwrite(keypad.rowPins[p], HIGH);
    }
}

/*  --------------------------------------------------------------------
    ---------- getKey
    --------------------------------------------------------------------
    Description : 1/ Apply a logic LOW signal to each Column (output).
                  2/ Scan each row (input).
                  If any of the column's key is pressed, the Logic LOW
                  signal from the column will pass to that row.
                  Through we can detect the key.
    Returns :     the keykode of the pressed key
                  or NO_KEY if no key is pressed
    ------------------------------------------------------------------*/

char Keypad_getKey()
{
    u8 c,r;
    u8 currentKey;

    for (c = 0; c < keypad.columns; c++)
    {
        // Select the current column.
        pinmode(keypad.columnPins[c], OUTPUT);
        
        // Scan all the rows of this column
        for (r = 0; r < keypad.rows; r++)
        {
            currentKey = keypad.map[c + r * keypad.columns];
            
            // Check the last button pressed.
            // Has it been released or is it still pressed ?
            if (keypad.lastKey == currentKey)
            {
                if ((millis() - keypad.lastUpdate) >= keypad.debounceTime && digitalread(keypad.rowPins[r]) == HIGH)
                {
                    keypad.status = RELEASED;
                    keypad.lastKey = NO_KEY;
                }

                if ((millis() - keypad.lastUpdate) >= keypad.holdTime && digitalread(keypad.rowPins[r]) == LOW)
                {
                    keypad.status = HOLD;
                    pinmode(keypad.columnPins[c], INPUT);
                    return currentKey;
                }
            }

            // New button pressed ?
            else if ((millis() - keypad.lastUpdate) >= keypad.debounceTime && digitalread(keypad.rowPins[r]) == LOW)
            {
                keypad.status = PRESSED;
                keypad.lastKey = currentKey;
                keypad.lastUpdate = millis();
                pinmode(keypad.columnPins[c], INPUT);
                return currentKey;
            }
        }

        // De-select the current column.
        pinmode(keypad.columnPins[c], INPUT);
    }

    return NO_KEY;
}

#endif // _KEYPAD_C_
