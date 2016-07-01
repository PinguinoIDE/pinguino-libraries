/*  ----------------------------------------------------------------------------
    FILE:           lcdi2c.pde
    PROJECT:        pinguino
    PURPOSE:        driving lcd display through i2c pcf8574 i/o expander
    PROGRAMMER:     Regis Blanchot <rblanchot@gmail.com>
                    André Gentric (adaptable pins relating to the i2c expander/pcf8574 configuration)
    FIRST RELEASE:  06 Apr. 2011
    LAST RELEASE:   09 Apr. 2016
    ----------------------------------------------------------------------------
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
    --------------------------------------------------------------------------*/

/*  ----------------------------------------------------------------------------
    ---------- PCF8574P (example for the DIY Pinguino module)
        This is a possible configuration among at least 3 ones :
        NB : other pin configurations are possible : look below at lcdi2c_init pins function
    ----------------------------------------------------------------------------

    +5V        A0	-|o    |-  VDD    +5V
    +5V        A1	-|     |-	 SDA    pull-up 1K8 au +5V
    +5V        A2	-|     |-	 SCL    pull-up 1K8 au +5V
    LCD_BL     P0   -|     |-	 INT
    LCD_RS     P1	-|     |-	 P7    LCD_D7
    LCD_RW     P2	-|     |-	 P6    LCD_D6
    LCD_EN     P3	-|     |-	 P5    LCD_D5
    GRND       VSS  -|     |-	 P4    LCD_D4

    SYMBOL  PIN DESCRIPTION					          NB
    A0                1   address input 0				
    A1                2   address input 1              A0, A1 et A2 connected to +5V
    A2                3   address input 2				
    P0                4   quasi-bidirectional I/O 0    LCD_BL(DIY module) or other
    P1                5   quasi-bidirectional I/O 1    LCD_RS or other
    P2                6   quasi-bidirectional I/O 2    LCD_RW or other
    P3                7   quasi-bidirectional I/O 3    LCD_EN or other
    VSS               8   supply ground
    P4 or P0          9   quasi-bidirectional I/O 4    LCD_D4
    P5 or P1          10  quasi-bidirectional I/O 5    LCD_D5
    P6 or P2          11  quasi-bidirectional I/O 6    LCD_D6
    P7 or P3          12  quasi-bidirectional I/O 7    LCD_D7
    INT               13  interrupt output (active LOW)
    SCL               14  serial clock line	     To Pinguino SCL
    SDA	   15	  serial data line		     To Pinguino SDA
    VDD	   16	  supply voltage

    Pinguino    x550    x6j50    32MX2x0
    SDA         DO      D5       D3 
    SDL         D1      D4       D4

    ----------------------------------------------------------------------------
    ---------- LCD 2x16 (GDM1602A with build-in Samsung KS0066/S6A0069)
    ----------------------------------------------------------------------------

    01 - VSS (GND)
    02 - VDD (+5V)
    03 - Vo (R = 1K to GND)
    04 - RS (P1)
    05 - RW (P2)
    06 - EN (P3)
    07 a 10 - D0 to D3 connected to GND (optional)
    11 a 16 - D4 to D7 connected to PCF8574
    15 - LED+ 380 Ohm to +5V depending from i2c mododule
    16 - LED- to GND depending from i2c mododule
    
    --------------------------------------------------------------------------*/
u16 i=0;

void setup()
{
    // initialize the digital pin USERLED as an output.
    pinMode(USERLED, OUTPUT);   

    // initialize the display.
    lcdi2c.init(16, 2, 0x27, 4, 3, 2, 1, 0);
    // 1st arg : LCD screen's num. of columns = 16
    // 2nd arg : LCD screen's num. of lines = 2
    // 3rd arg : PCF8574 I2C address = 0x27
    //     PCF8574  : slave adress is 0 1 0 0 A2 A1 A0
    //     PCF8574A : slave adress is 0 1 1 1 A2 A1 A0
    // 4th arg : D4-D7 connected to pin P4 (to P7)
    // 5th arg : EN connected to pin P3
    // 6th arg : RW connected to pin P2
    // 7th arg : RS connected to pin P1
    // 8th arg : BackLight connected to P0
    // Config. examples :
    // lcdi2c.init(16, 2, 0x27,4,2,1,0,3);  // 27 DFRobot noBL i2c/lcd both soldered, needs pullup resistors on SDA/SCL
    // lcdi2c.init(16, 2, 0x27,0,4,5,6,7);  // black mjkdz, needs pullup resistors on SDA/SCL 
    // lcdi2c.init(16, 2, 0x20,0,4,5,6,7);  // blue mjkdz, address from 20 to 27, needs pullup resistors on SDA/SCL

    // Select backlight or noBacklight according your i2c/lcd module
    lcdi2c.noBacklight();          // turns backlight off (for DIY module or DFRobot)
    // lcdi2c.backlight();         // turns backlight on (for mjkdz)

    lcdi2c.clear();                // clear screen
    lcdi2c.home();                 // set cursor at (0,0)
    lcdi2c.print("lcdi2c demo");
}

void loop()
{
    lcdi2c.setCursor(0, 1);     // set cursor at line 1, col 0
    lcdi2c.printf("i=%u ", i++);
    toggle(USERLED);        // alternate ON and OFF
    delay(1000);
}

/*
	available functions :
		void lcdi2c.backlight();
		void lcdi2c.noBacklight();
		void lcdi2c.clear();
		void lcdi2c.clearLine(u8);
		void lcdi2c.home();
		void lcdi2c.noAutoscroll();
		void lcdi2c.autoscroll();
		void lcdi2c.rightToLeft();
		void lcdi2c.leftToRight();
		void lcdi2c.scrollDisplayRight();
		void lcdi2c.scrollDisplayLeft();
		void lcdi2c.blink();
		void lcdi2c.noBlink();
		void lcdi2c.cursor();
		void lcdi2c.noCursor();
		void lcdi2c.display();
		void lcdi2c.noDisplay();
		void lcdi2c.setCursor(u8, u8);
		void lcdi2c.write(u8,u8);
		void lcdi2c.printChar(u8);
		void lcdi2c.print(const u8 *);		
		void lcdi2c.println(const u8 *);
		void lcdi2c.printCenter(const u8 *);		
		void lcdi2c.printNumber(s32, u8);
		void lcdi2c.printFloat(float, u8);
		void lcdi2c.printf(char*, ...);
		void lcdi2c.newchar(const u8 *, u8);
*/