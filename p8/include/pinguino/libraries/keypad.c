/*  --------------------------------------------------------------------
    FILE:           keypad.c
    PROJECT:        pinguino
    PURPOSE:        matrix keypad management
    PROGRAMER:      Fabio Malagas <fabio.malagas@gmail.com>
    --------------------------------------------------------------------
    CHANGELOG :
    10 Apr. 2013 - Fabio Malagas  - first release
    03 Feb. 2016 - Régis Blanchot - adpated and fixed for 8-bit
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

#include <typedef.h>
#include <const.h>
#include <digitalp.c>
#include <digitalr.c>
#include <digitalw.c>
#include <millis.c>
#include <keypad.h>

//-------------------
// Variables 
//-------------------

u8 keypad_rows;
u8 keypad_columns;
u8 *rowPins;
u8 *columnPins;

char currentKey;
char *userKeymap;

KeypadState state;
unsigned long lastUpdate;
unsigned int debounceTime;
unsigned int holdTime;


//-----------------
// Funtions
//-----------------


KeypadState Keypad_getState()
{
    return state;
}

void Keypad_setDebounceTime(unsigned int debounce)
{
    debounceTime = debounce;
}

void Keypad_setHoldTime(unsigned int hold)
{
    holdTime = hold;
}

/*
void Keypad_addEventListener(void (*listener)(char))
{
    keypadEventListener = listener;
}
*/

//private function
void Keypad_transitionTo(KeypadState newState)
{
    if (state!=newState)
    {
        state = newState;
        /*
        if (keypadEventListener!=NULL)
        { 
            keypadEventListener(currentKey);
        }
        */
    }
}

//private function
void Keypad_initializePins()
{
    u8 c,r;

    for (r=0; r<keypad_rows; r++)
    {
        for (c=0; c<keypad_columns; c++)
        {
            pinmode(columnPins[c],OUTPUT);
            digitalwrite(columnPins[c],HIGH);
        }
        //configure row pin modes and states
        pinmode(rowPins[r],INPUT);
        digitalwrite(rowPins[r],HIGH);
    }
}

// Initialization of Allows custom keymap. pin configuration and keypad size
void Keypad_init(char *keypadmap, u8 *rowp, u8 *colp, u8 rows, u8 cols)
{
    userKeymap = keypadmap;
    rowPins = rowp;
    columnPins = colp;
    keypad_rows = rows;
    keypad_columns = cols;         

    lastUpdate = 0;
    debounceTime = 50;
    holdTime = 1000;
    //keypadEventListener = 0;
    currentKey = NO_KEY;
    state = IDLE;
    
    Keypad_initializePins();
}


// Returns the keykode of the pressed key, or NO_KEY if no key is pressed
char Keypad_getKey()
{
    // Assume that no key is pressed, this is the default return for getKey()
    char key = NO_KEY;
    u8 c,r;    

    for (c=0; c<keypad_columns; c++)
    {
        // Activate the current column.
        digitalwrite(columnPins[c],LOW);
        // Scan all the rows for a key press.
        for (r=0; r<keypad_rows; r++)
        {
            //  The user pressed a button for more then debounceTime microseconds.
            if (currentKey == userKeymap[c+(r*keypad_columns)])
            {
                // Button hold
                if (((millis()-lastUpdate) >= holdTime) && digitalread(rowPins[r]) == LOW)
                {
                    Keypad_transitionTo(HOLD);
                }
                // Button release
                if (((millis()-lastUpdate) >= debounceTime) && digitalread(rowPins[r]) == HIGH)
                {
                    Keypad_transitionTo(RELEASED);
                    currentKey = NO_KEY;
                }
            } 
            // Button pressed event.  The user pressed a button.
            else if (((millis()-lastUpdate) >= debounceTime) && digitalread(rowPins[r]) == LOW)
            {
                // De-activate the current column.
                digitalwrite(columnPins[c],HIGH);
                key = userKeymap[c+(r*keypad_columns)];
                lastUpdate = millis();
                // Save resources and do not attempt to parse two keys at a time
                goto EVALUATE_KEY;
            } 
        }
        // De-activate the current column.
        digitalwrite(columnPins[c],HIGH);
    }
    
    EVALUATE_KEY:
    if (key != NO_KEY && key != currentKey)
    { 
        currentKey = key;
        Keypad_transitionTo(PRESSED);
        return currentKey;
    } 
    else
    {
        return NO_KEY;
    }
}

#endif // _KEYPAD_C_
