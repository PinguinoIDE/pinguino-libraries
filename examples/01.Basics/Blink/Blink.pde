/*
	Blink a LED
*/

void setup()
{
    // initialize the onboard USERLED as an output.
    pinMode(USERLED, OUTPUT);   
}

void loop()
{
    toggle(USERLED);    // alternate ON and OFF
    delay(1000);        // wait for 1 sec

/*  Or ...
    digitalWrite(USERLED, HIGH);
    delay(500);		     // wait for 500ms
    digitalWrite(USERLED, LOW);
    delay(500);		     // wait for 500ms
*/
}
