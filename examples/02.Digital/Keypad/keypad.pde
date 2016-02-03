 /*
   10-04-2013
   
   
   Version 1_0
   
   Keypad for PIC32 MX targets: 
    
    - This is a example adapted fom the p8 library and should work
    with all PIC32MX boards. 10K Pull-ups are required for the rows, in order
    to grant levels. 
    
    - It has also includes a patch in the library, in which you would pressed 2 keys
    and would write infinite the second key.
    
   
    fabio.malagas@gmail.com
      
*/

const u8 rows = 4;
const u8 cols = 3;
KeypadState state;
char currentKey;
unsigned long lastUpdate;
unsigned int debounceTime;
unsigned int holdTime;

// create keypad matrix
char keypadmap[4][3] = {
    {'1','2','3'},
    {'4','5','6'},
    {'7','8','9'},
    {'*', '0', '#'}
};

u8 rowp[4] = {0, 2,  5, 9}; //connect to the row pinouts of the keypad
u8 colp[3] = {3, 4, 1}; //connect to the column pinouts of the keypad

void setup ()
{
    Keypad.init(*keypadmap,rowp,colp,rows,cols);
    //Serial.begin(9600);
    
    // costumizar o debounce.
    Keypad.setDebounceTime(50);
    Keypad.setHoldTime(1000);
}

void loop ()
{
    u8 key = Keypad.getKey();

    if (key != NO_KEY )
    {
        // Serial.println(key);
        // CDC.write(state + 48);
        // CDC.write(' ');
        CDC.write(key);
        CDC.write('\r');
        CDC.write('\n');
    }
    delay(100);  
}
