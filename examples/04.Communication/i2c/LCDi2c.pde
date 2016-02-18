/*  ----------------------------------------------------------------------------
    FILE:           lcdi2c.pde
    PROJECT:        pinguino
    PURPOSE:        driving lcd display through i2c pcf8574 i/o expander
    PROGRAMER:      regis blanchot <rblanchot@gmail.com>
    FIRST RELEASE:  06 apr. 2011
    LAST RELEASE:   12 jun. 2012
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
    ---------- PCF8574P
    ----------------------------------------------------------------------------

    +5V		A0	-|o |-		VDD		+5V
    +5V		A1	-|	 |-		SDA		pull-up 1K8 au +5V
    +5V		A2	-|	 |-		SCL 	pull-up 1K8 au +5V
    LCD_BL	P0	-|	 |-		INT
    LCD_RS	P1	-|	 |-		P7		LCD_D7
    LCD_RW	P2	-|	 |-		P6		LCD_D6
    LCD_EN	P3	-|	 |-		P5		LCD_D5
    GRND	VSS		-|	 |-		P4		LCD_D4

    SYMBOL  PIN DESCRIPTION					          NB
    A0		   1		address input 0				
    A1		   2		address input 1				        A0, A1 et A2 connected to +5V
    A2		   3		address input 2				
    P0		   4		quasi-bidirectional I/O 0    LCD_BL
    P1		   5		quasi-bidirectional I/O 1	  LCD_RS
    P2		   6		quasi-bidirectional I/O 2	  LCD_RW
    P3		   7		quasi-bidirectional I/O 3	  LCD_EN
    VSS	   8		supply ground
    P4		   9		quasi-bidirectional I/O 4	  LCD_D4
    P5		   10	  quasi-bidirectional I/O 5	  LCD_D5
    P6		   11	  quasi-bidirectional I/O 6	  LCD_D6
    P7		   12	  quasi-bidirectional I/O 7	  LCD_D7
    INT	   13	  interrupt output (active LOW)
    SCL	   14	  serial clock line			       To Pinguino SCL
    SDA	   15	  serial data line			       To Pinguino SDA
    VDD	   16	  supply voltage

    Pinguino    x550    x6j50
    SDA         DO      D5
    SDL         D1      D4

    ----------------------------------------------------------------------------
    ---------- LCD 2x16 (GDM1602A with build-in Samsung KS0066/S6A0069)
    ----------------------------------------------------------------------------

    01 - VSS (GND)
    02 - VDD (+5V)
    03 - Vo (R = 1K to GND)
    04 - RS (P1)
    05 - RW (P2)
    06 - EN (P3)
    07 a 10 - D0 to D3 connected to GND.
    11 a 16 - D4 to D7 connected to PCF8574
    15 - LED+ 380 Ohm to +5V
    16 - LED- to GND
    
    --------------------------------------------------------------------------*/

void setup()
{
    u8 cpu;
    
    pinMode(USERLED, OUTPUT);

    // PCF8574  : slave adress is 0 1 0 0 A2 A1 A0
    // PCF8574A : slave adress is 0 1 1 1 A2 A1 A0
    lcdi2c.init(16, 2, 0x27);           // display is 2x16, ic2 address is 0100111 (see above)
    lcdi2c.backlight();                 // turns backlight on
    lcdi2c.clear();                     // clear screen
    lcdi2c.printCenter("PINGUINO INFO.");
    cpu = System.getCpuFrequency() / MHZ;
    lcdi2c.setCursor(0, 1);	    // set cursor at line 1, col 0
    lcdi2c.print("CPU=");
    lcdi2c.printNumber(cpu, DEC);
    lcdi2c.print("MHz");
    delay(2500);
}

void loop()
{

    u16 fps;                       // frame per second
    u32 timeEnd = millis() + 1000; // 1000 ms = 1 sec

    for (fps = 1; millis() < timeEnd; fps++);

    lcdi2c.setCursor(0, 1);	    // set cursor at line 1, col 0
    //lcdi2c.printf("MIPS=%u", fps);
    lcdi2c.print("MIPS=");
    lcdi2c.printNumber(fps, DEC);

    delay(500);
    toggle(USERLED);
}
