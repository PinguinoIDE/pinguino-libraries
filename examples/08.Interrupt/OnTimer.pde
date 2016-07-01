/*  -------------------------------------------------------
    interrupt library demo
    -------------------------------------------------------
    author:			Régis Blanchot
    first release:	19/12/2010
    last update:	24/12/2010
    pinguino ide:	>9.5
    boards:	all
    wiring:	leds on pin 0 to 8
		ex.: pin0 --- 470 Ohms ---<|--- GND
    -----------------------------------------------------*/

#define myLED0	USERLED
//#define myLED1	1
//#define myLED2	2
//#define myLED3	3
//PINGUINO x6J50 and x7j53 only
//#define myLED4	4
//PINGUINO x7j53 only
//#define myLED5	5
//#define myLED6	6
//#define myLED8	8

void blink0() {	toggle(myLED0); }
//void blink1() {	toggle(myLED1); }
//void blink2() {	toggle(myLED2); }
//void blink3() {	toggle(myLED3); }
//PINGUINO x6J50 and x7j53 only
//void blink4() {	toggle(myLED4); }
//PINGUINO x7j53 only
//void blink5() {	toggle(myLED5); }
//void blink6() {	toggle(myLED6); }
//void blink8() {	toggle(myLED8); }

void setup()
{
    pinMode(myLED0, OUTPUT);
    //pinMode(myLED1, OUTPUT);
    //pinMode(myLED2, OUTPUT);
    //pinMode(myLED3, OUTPUT);
    //PINGUINO x6J50 and x7j53 only
    //pinMode(myLED4, OUTPUT);
    //PINGUINO x7j53 only
    //pinMode(myLED5, OUTPUT);
    //pinMode(myLED6, OUTPUT);
    //pinMode(myLED8, OUTPUT);
    
    OnTimer0(blink0, INT_MILLISEC, 500);	// Use Timer0 to toggle pin 0 every 500 ms
    //OnTimer1(blink1, INT_MILLISEC, 500);	// Use Timer1 to toggle pin 0 every 500 ms
    //OnTimer2(blink2, INT_MILLISEC, 500);	// Use Timer2 to toggle pin 0 every 500 ms
    //OnTimer3(blink3, INT_MILLISEC, 500);	// Use Timer3 to toggle pin 0 every 500 ms
    //PINGUINO x6J50 and x7j53 only
    //OnTimer4(blink4, INT_MILLISEC, 500);	// Use Timer4 to toggle pin 0 every 500 ms
    //PINGUINO x7j53 only
    //OnTimer5(blink5, INT_MILLISEC, 500);	// Use Timer5 to toggle pin 0 every 500 ms
    //OnTimer6(blink6, INT_MILLISEC, 500);	// Use Timer6 to toggle pin 0 every 500 ms
    //OnTimer8(blink8, INT_MILLISEC, 500);	// Use Timer8 to toggle pin 0 every 500 ms
}

void loop()
{
}
