/*--------------------------------------------------------------------------
	blink built-in led with help from interrupt library
----------------------------------------------------------------------------
	author:         Régis Blanchot
	first release:  19/12/2010
	last update:    15/01/2015
	pinguino ide:   > 9.5
 	boards:         ALL
--------------------------------------------------------------------------*/

u32 i;

void myBlink() { toggle(USERLED); }
//void myBlink0() { toggle(0); }
//void myBlink1() { toggle(1); }
//void myBlink2() { toggle(2); }
//void myBlink3() { toggle(3); }
//void myBlink4() { toggle(4); }
//void myBlink5() { toggle(5); }
//void myBlink6() { toggle(6); }
//void myBlink8() { toggle(7); }

void setup()
{
    //Serial2.begin(9600); // OLIMEX PINGUINO
    Serial.begin(9600);
    pinMode(USERLED, OUTPUT);

    /* 8-bit Pinguino Only */
    //OnTimer0(myBlink0, INT_MILLISEC, 500);  // Use Timer0 to toggle the USERLED every 500 ms

    /* All Pinguino */
    //OnTimer1(myBlink, INT_MILLISEC, 500);    // Use Timer1 to toggle the USERLED every 500 ms
    //OnTimer2(myBlink, INT_MILLISEC, 500); // Use Timer2 to toggle the USERLED every 1000 us
    //OnTimer3(myBlink, INT_MILLISEC, 500);         // Use Timer3 to toggle the USERLED every 5 s
    
    /* 32-bit Pinguino, x6j50 and xxj53 only */
    //OnTimer4(myBlink, INT_MILLISEC, 500);  // Use Timer4 to toggle the USERLED every 500 ms
    
    /* 32-bit Pinguino and xxj53 only */
    OnTimer5(myBlink, INT_MILLISEC, 500);  // Use Timer5 to toggle the USERLED every 600 ms
    
    /* Pinguino xxj53 only */
    //OnTimer6(myBlink6, INT_MILLISEC, 700);  // Use Timer6 to toggle the USERLED every 700 ms
    //OnTimer8(myBlink8, INT_MILLISEC, 800);  // Use Timer8 to toggle the USERLED every 800 ms
}

void loop()
{
    // Whatever you want here

    //Serial2.printf("uptime %dms\r\n", i++);
    //Serial2.printf("uptime %dms\r\n", millis());
    //Serial2.printf("F=%dHz\r\n", System.getPeripheralFrequency()/1000/2);

    Serial.printf("i=%d\r\n", i++);
    //Serial.printf("uptime %dms\r\n", millis());
    //Serial.printf("F=%dHz\r\n", System.getPeripheralFrequency()/1000/2);

    //toggle(USERLED);
    //delay(500);
}
