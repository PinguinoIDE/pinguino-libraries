/*
Library examples for TM16XX.

Copyright (C) 2011 Ricardo Batista <rjbatista at gmail dot com>

This program is free software: you can redistribute it and/or modify
it under the terms of the version 3 GNU General Public License as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define NB_OF_DISPLAYS      8 
#define NB_OF_KEYS          NB_OF_DISPLAYS

void setup() {
    
    // define a module on data pin 5, clock pin 6 and strobe pin 7
   TM16XX.init(NB_OF_DISPLAYS, 5, 6, 7);
   TM16XX.clearAll();
       delay(300);
   TM16XX.printf("Start", 0, 0 );
       delay(3000);
}

void loop() {

int i;
  for( i=0; i<=7; i++){
    int keys = TM16XX.getButtons();
    TM16XX.clearAll();
    switch (keys){
        case -1:
                TM16XX.printf("Button 1", 0, 0);
                TM16XX.setLED(0, TM16XX_RED);
          
            break;
        case -2:
                TM16XX.printf("Button 2", 0, 0);
                TM16XX.setLED(1,TM16XX_RED); 
            break;
        case -4:
                TM16XX.printf("Button 3", 0, 0);
                TM16XX.setLED(2,TM16XX_RED); 
            break;            
        case -8:
                TM16XX.printf("Button 4", 0, 0);
                TM16XX.setLED(3,TM16XX_RED); 
            break;
        case -16:
                TM16XX.printf("Button 5", 0, 0);
                TM16XX.setLED(4,TM16XX_RED); 
            break;           
        case -32:
                TM16XX.printf("Button 6", 0, 0);
                TM16XX.setLED(5,TM16XX_RED); 
            break;
        case -64:
                TM16XX.printf("Button 7", 0, 0);
                TM16XX.setLED(6,TM16XX_RED); 
            break;
        case -128:
                TM16XX.printf("Button 8", 0, 0);
                TM16XX.setLED(7,TM16XX_RED); 
            break;
            
            default:
                TM16XX.setLED(i,0); 
        }
            
    //CDC.println("Keys: %c\r\n", keys);
   //CDC.printf("character = %i \r\n", keys);
   }
}
