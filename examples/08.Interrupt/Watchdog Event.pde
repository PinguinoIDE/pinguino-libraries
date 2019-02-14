//--	watchdog-test.pde

void setup()
{
    Watchdog.clear();

    pinMode(USERLED, OUTPUT);

    // turn off all led
    digitalWrite(USERLED, LOW);

    // led indicates pinguino reset by watchdog
    if (Watchdog.event())	
    {
        digitalWrite(USERLED, HIGH);
        delay(2000);
        digitalWrite(USERLED, LOW);
    }

    // enable watchdog, start full wdt period
    Watchdog.enable();
    Watchdog.clear();
}

void loop()
{
    // blink led every second
    digitalWrite(USERLED, HIGH);
    delay(500);
    digitalWrite(USERLED, LOW);
    delay(500);

    // watchdog should reset pinguino due to lack of clear
}
