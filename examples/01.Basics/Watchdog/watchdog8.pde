/*--------------------------------------------------------------	    blink built-in led with help from watchdog
  --------------------------------------------------------------	    author : Régis Blanchot
  ------------------------------------------------------------*/

void setup()
{
    //System.intOsc(_31KHZ_);
    pinMode(USERLED, OUTPUT);
    // Enable Watchdog Timer
    // Watchdog is driven by the Internal Oscillator (8MHz)
    // Nominal period of Watchdog is 4ms
    // Watchdog postscaler is set to 1:32768 by config. bits
    // Watchdog timer will overload after 32768*4ms = 135sec = 2.25min 
    System.watchdog();
}

void loop()
{
    System.clearWatchdog(); // clear watchdog timer
    // Enter Sleep Mode
    System.sleep();         //  wait for watchdog timer overload
    // Back to Run Mode
    toggle(USERLED);          // toggle the led
}
