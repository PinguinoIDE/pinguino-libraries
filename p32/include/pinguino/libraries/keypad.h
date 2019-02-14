/*
    Pinguino Library for PS2 Keypads
*/

#ifndef KEYPAD_H
#define KEYPAD_H

#include <typedef.h>

typedef enum {
    IDLE=0, 
    PRESSED, 
    RELEASED, 
    HOLD
} KeypadStatus;

typedef struct {
    u8 rows;
    u8 columns;
    u8 lastKey;

    u8 *rowPins;
    u8 *columnPins;
    u8 *map;

    u16 debounceTime;
    u16 holdTime;
    u32 lastUpdate;

    KeypadStatus status;
} Keypad;

const u8 NO_KEY = '\0';

#define KEY_RELEASED NO_KEY

void Keypad_init(u8*, u8*, u8*, u8, u8);
char Keypad_getKey();
//KeypadStatus Keypad_getStatus();
void Keypad_setDebounceTime(u16);
void Keypad_setHoldTime(u16);

#endif // KEYPAD_H
