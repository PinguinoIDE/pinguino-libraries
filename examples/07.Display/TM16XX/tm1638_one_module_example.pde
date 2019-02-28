/*
    TM16XX demo.
    Press a button and see the result on the display
*/

#define NB_OF_DISPLAYS      8 // TM1638
#define NB_OF_KEYS          NB_OF_DISPLAYS

void setup()
{
    // Define a module with data pin 5, clock pin 6 and strobe pin 7
    TM16XX.init(NB_OF_DISPLAYS, 5, 6, 7);
}

void loop()
{
    u8 i;
    // One bit per button
    u8 keys = TM16XX.getButtons();

    TM16XX.clearAll();

    for (i=0; i<NB_OF_KEYS; i++)
    {
        // if key was pressed
        if (BitTest(keys, i) == 1)
        {
            // LED on
            TM16XX.setLED(i, TM16XX_RED);
            delay(300);

            // Display button number
            TM16XX.print("Button ");
            TM16XX.printNumber(i, DEC);
            // or with printf
            //TM16XX.printf("Button %d", i);

            // LED off
            TM16XX.setLED(i, 0);
            delay(300);
        }
    }
}
