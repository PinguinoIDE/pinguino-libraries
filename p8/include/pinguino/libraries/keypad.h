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
} KeypadState;


const char NO_KEY = '\0';

#define KEY_RELEASED NO_KEY

//public functions: 
#define makeKeymap(x) ((char *)x)

void Keypad_init(char *keypadmap, u8 *rowp, u8 *colp, u8 rows, u8 cols);
char Keypad_getKey();
KeypadState Keypad_getState();
void Keypad_setDebounceTime(unsigned int debounce);
void Keypad_setHoldTime(unsigned int hold);
//void addEventListener(void (*listener)(char));

//private functions:
void Keypad_transitionTo(KeypadState newState);
void Keypad_initializePins();

#endif // KEYPAD_H
