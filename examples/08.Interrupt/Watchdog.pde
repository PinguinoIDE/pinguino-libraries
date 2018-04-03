/*	----------------------------------------------------------------------------
    Blink the built-in led with help from watchdog timer and sleep mode
    ----------------------------------------------------------------------------
    author:	Régis Blanchot
    --------------------------------------------------------------------------*/

void setup()
{
    // We want to use the Built-in led
    pinMode(USERLED, OUTPUT);
    // Enable Watchdog Timer
    // Watchdog is driven by the Internal Oscillator
    // Nominal period of Watchdog is 4ms
    // Watchdog postscaler is set to 
    // 1:32768 by config. bits (bootloader version <= 4.8)
    // 1:256 (bootloader version > 4.8)
    // Watchdog timer will overload after
    // 32768*4ms = 135sec = 2.25min
    // or 256*4ms = 1024 ms = 1 sec. 
    Watchdog.enable();
}

void loop()
{
    // clear watchdog timer
    Watchdog.clear();
    // Enter Sleep Mode and wait for watchdog timer overload
    System.sleep();
    // Back to Run Mode and toggle the led
    toggle(USERLED);
}
