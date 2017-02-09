/*
    Membrane Keypas example
*/

#define ROWS 4
#define COLS 3

// create keypad matrix
u8 keymap[ROWS][COLS] = {
    {'1','2','3'},
    {'4','5','6'},
    {'7','8','9'},
    {'*','0','#'}
};

// pin numbers down the right side of the keypad
u8 rowpins[ROWS] = {6, 5, 4, 3};
// pin numbers across the bottom of the keypad
u8 colpins[COLS] = {2, 1, 0};

void setup ()
{
    Keypad.init(*keymap, rowpins, colpins, ROWS, COLS);
    Serial.begin(9600);
    Serial.print("\f*** Keypad test ***\r\n");
    
    // costumize debounce
    Keypad.setDebounceTime(50);
    Keypad.setHoldTime(1000);
}

void loop ()
{
    u8 key = Keypad.getKey();

    if (key != NO_KEY)
    {
        Serial.print("You pressed key: ");
        Serial.printChar(key);
        Serial.print(", status = ");
        if (Keypad.getStatus() == HOLD)
            Serial.print("HOLD\r\n");
        if (Keypad.getStatus() == PRESSED)
            Serial.print("PRESSED\r\n");
        if (Keypad.getStatus() == RELEASED)
            Serial.print("RELEASED\r\n");
    }
}
