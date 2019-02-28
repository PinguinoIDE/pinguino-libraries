/**
 *    getkey.pde
 *    @author (c)2010 - R. Blanchot <rblanchot@gmail.com>
 *    @desc   Read a character and print it back
 */

void setup()
{
    Serial.begin(9600);
    Serial.print("\f\f\f"); // Clear screen
}

void loop()
{
    u8 c;
    Serial.printf("Please, press a key\r\n");
    c = Serial.getKey();
    Serial.printf("You pressed key [%c]\r\n",c);
}
