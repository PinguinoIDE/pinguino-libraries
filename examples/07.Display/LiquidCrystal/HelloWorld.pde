/*
  LiquidCrystal Library - Hello World
 
 Demonstrates the use a 16x2 LCD display.  The LiquidCrystal
 library works with all LCD displays that are compatible with the 
 Hitachi HD44780 driver.

 This sketch prints "Hello World!" to the LCD and shows the time.
 
 The circuit:
 * LCD RS pin to digital pin 0
 * LCD E  pin to digital pin 1
 * LCD D4 pin to digital pin 2
 * LCD D5 pin to digital pin 3
 * LCD D6 pin to digital pin 4
 * LCD D7 pin to digital pin 5
 * LCD R/W pin to ground
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 
 Pinguino port by Marcus Fazzi <marcus@fazzi.eng.br>
 http://fazzi.eng.br
 */

void setup()
{
    pinMode(USERLED, OUTPUT);
    // initialize the library with the numbers of the interface pins
    //lcd.pins(RS, E,  0,  0,  0,  0, D4, D5, D6, D7); //4bits
    //lcd.pins(RS, E, D0, D1, D2, D3, D4, D5, D6, D7); //8bits
    lcd.pins(0, 1, 0, 0, 0, 0, 2, 3, 4, 5); // RS, E, D4 ~ D8	

    // set up the LCD's number of columns and rows: 
    lcd.begin(2, 0);
    //lcd.home(); // 0, 0
    // Print a message to the LCD.
    lcd.print("Hello, World!");
}

void loop()
{
    toggle(USERLED);
    delay(1000);
}
