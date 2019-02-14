/*  --------------------------------------------------------------------
    FILE:        TM16XX.h
    PROJECT:     Pinguino
    PURPOSE:     Library implementation for TM16XX displays
    PROGRAMERS:  Ricardo Batista <rjbatista@gmail.com>
                 Regis blanchot <rblanchot@gmail.com>
    --------------------------------------------------------------------
    CHANGELOG
                 - Ricardo Batista - first release
    30 Dec. 2016 - Régis Blanchot  - fixed for Pinguino 8- and 32-bit
    02 Jan. 2017 - Régis Blanchot  - added new functions
    --------------------------------------------------------------------
    This program is free software: you can redistribute it and/or modify
    it under the terms of the version 3 GNU General Public License as
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    ------------------------------------------------------------------*/

#ifndef TM16XX_H
#define TM16XX_H

#include <typedef.h>

// Commands
#define TM16XX_DATACMD      0x40    // 0b01000000
#define TM16XX_WRITE        0b0000
#define TM16XX_READKEY      0b0010
#define TM16XX_FIXEDADDR    0b0100
#define TM16XX_DISPLAYON    0b1000
#define TM16XX_DISPLAYOFF   0b0000
#define TM16XX_DISPCMD      0x80    // 0b10000000
#define TM16XX_ADDRCMD      0xC0    // 0b11000000

// Colors
#define TM16XX_NONE         0
#define TM16XX_RED          1
#define TM16XX_GREEN        2

// Set the intensity (0-7)
void TM16XX_setIntensity(u8 intensity);

//void TM16XX_init(u8 dataPin, u8 clockPin, u8 strobePin, u8 displays, u8 activateDisplay = true, u8 intensity = 7);
void TM16XX_init(u8 displays, u8 dataPin, u8 clockPin, u8 strobePin);

// Set the display (segments and LEDs) active or off and intensity (range from 0-7). 
//void TM16XX_setupDisplay(u8 active, u8 intensity);

// Set a single display at pos (starting at 0) to a digit (left to right) 
//void TM16XX_setDisplayDigit(u8 digit, u8 pos, u8 dot, const u8 *numberFont);// = NUMBER_FONT);

// Set the display to an error message 
//void TM16XX_setDisplayToError();

// Clear  a single display at pos (starting at 0, left to right)  
//void TM16XX_clearDisplayDigit(u8 pos, u8 dot);

// Set the display to the values (left to right) 
//void TM16XX_setDisplay(u8 *values, unsigned int length);// = 8);

// Clear all or a specific display 
void TM16XX_clearAll();
void TM16XX_clearDisplay(u8);

// Print functions
void TM16XX_printChar(u8 c);
void TM16XX_print(const u8* string);
void TM16XX_printNumber(s32 value, u8 base);
void TM16XX_printFloat(float number, u8 digits);
void TM16XX_printf(const u8 *fmt, ...);

// Set the display to the string (defaults to built in font) 
//void TM16XX_setDisplayToString(const u8* string, const word dots, const u8 pos,	const u8 *font);// = FONT_DEFAULT);

//
//void TM16XX_sendChar(u8 pos, u8 data, u8 dot);// = 0;

//
//void TM16XX_setDisplayToDecNumber(unsigned long number, u8 dots, u8 startingPos, u8 leadingZeros, u8 *numberFont);

// Set the display to a unsigned hexadecimal number (with or without leading zeros)
//void TM16XX_setDisplayToHexNumber(unsigned long number, char dots, char leadingZeros, char *numberFont);

// Set the display to a unsigned decimal number (with or without leading zeros)
//void TM16XX_setDisplayToDecNumber(unsigned long number, char dots, char leadingZeros, char *numberFont);

// Set the display to a signed decimal number (with or without leading zeros)
//void TM16XX_setDisplayToSignedDecNumber(signed long number, char dots, char leadingZeros, char *numberFont);

// Set the display to a unsigned binary number
//void TM16XX_setDisplayToBinNumber(char number, char dots, char *numberFont);

// Set the LED at pos to color (TM1638_COLOR_RED, TM1638_COLOR_GREEN or both)
void TM16XX_setLED(u8 pos, u8 color);
// Set the LEDs. MSB byte for the green LEDs, LSB for the red LEDs
void TM16XX_setLEDs(unsigned int led);

// Returns the pressed buttons as a bit set (left to right)
int TM16XX_getButtons();

void TM16XX_sendCommand(u8 led);
void TM16XX_sendData(u8 add, u8 data);
void TM16XX_send(u8 data);
u8   TM16XX_receive();

/*
    The bits are displayed by mapping bellow
     -- 0 --
    |       |
    5       1
     -- 6 --
    4       2
    |       |
     -- 3 --  .7
*/

// definition for standard hexadecimal numbers
/*
u8 NUMBER_FONT[] = {
  0b00111111, // 0
  0b00000110, // 1
  0b01011011, // 2
  0b01001111, // 3
  0b01100110, // 4
  0b01101101, // 5
  0b01111101, // 6
  0b00000111, // 7
  0b01111111, // 8
  0b01101111, // 9
  0b01110111, // A
  0b01111100, // B
  0b00111001, // C
  0b01011110, // D
  0b01111001, // E
  0b01110001  // F
};

u8 MINUS = 0b01000000;

// definition for error
u8 ERROR_DATA[] = {
  0b01111001, // E
  0b01010000, // r
  0b01010000, // r
  0b01011100, // o
  0b01010000, // r
  0,
  0,
  0
};
*/

// definition for the displayable ASCII u8s
u8 TM16XX_font[] = {
  0b00000000, // (32)  <space>
  0b10000110, // (33)	!
  0b00100010, // (34)	"
  0b01111110, // (35)	#
  0b01101101, // (36)	$
  0b00000000, // (37)	%
  0b00000000, // (38)	&
  0b00000010, // (39)	'
  0b00110000, // (40)	(
  0b00000110, // (41)	)
  0b01100011, // (42)	*
  0b00000000, // (43)	+
  0b00000100, // (44)	,
  0b01000000, // (45)	-
  0b10000000, // (46)	.
  0b01010010, // (47)	/
  0b00111111, // (48)	0
  0b00000110, // (49)	1
  0b01011011, // (50)	2
  0b01001111, // (51)	3
  0b01100110, // (52)	4
  0b01101101, // (53)	5
  0b01111101, // (54)	6
  0b00100111, // (55)	7
  0b01111111, // (56)	8
  0b01101111, // (57)	9
  0b00000000, // (58)	:
  0b00000000, // (59)	;
  0b00000000, // (60)	<
  0b01001000, // (61)	=
  0b00000000, // (62)	>
  0b01010011, // (63)	?
  0b01011111, // (64)	@
  0b01110111, // (65)	A
  0b01111111, // (66)	B
  0b00111001, // (67)	C
  0b00111111, // (68)	D
  0b01111001, // (69)	E
  0b01110001, // (70)	F
  0b00111101, // (71)	G
  0b01110110, // (72)	H
  0b00000110, // (73)	I
  0b00011111, // (74)	J
  0b01101001, // (75)	K
  0b00111000, // (76)	L
  0b00010101, // (77)	M
  0b00110111, // (78)	N
  0b00111111, // (79)	O
  0b01110011, // (80)	P
  0b01100111, // (81)	Q
  0b00110001, // (82)	R
  0b01101101, // (83)	S
  0b01111000, // (84)	T
  0b00111110, // (85)	U
  0b00101010, // (86)	V
  0b00011101, // (87)	W
  0b01110110, // (88)	X
  0b01101110, // (89)	Y
  0b01011011, // (90)	Z
  0b00111001, // (91)	[
  0b01100100, // (92)	\ (this can't be the last u8 on a line, even in comment or it'll concat)
  0b00001111, // (93)	]
  0b00000000, // (94)	^
  0b00001000, // (95)	_
  0b00100000, // (96)	`
  0b01011111, // (97)	a
  0b01111100, // (98)	b
  0b01011000, // (99)	c
  0b01011110, // (100)	d
  0b01111011, // (101)	e
  0b00110001, // (102)	f
  0b01101111, // (103)	g
  0b01110100, // (104)	h
  0b00000100, // (105)	i
  0b00001110, // (106)	j
  0b01110101, // (107)	k
  0b00110000, // (108)	l
  0b01010101, // (109)	m
  0b01010100, // (110)	n
  0b01011100, // (111)	o
  0b01110011, // (112)	p
  0b01100111, // (113)	q
  0b01010000, // (114)	r
  0b01101101, // (115)	s
  0b01111000, // (116)	t
  0b00011100, // (117)	u
  0b00101010, // (118)	v
  0b00011101, // (119)	w
  0b01110110, // (120)	x
  0b01101110, // (121)	y
  0b01000111, // (122)	z
  0b01000110, // (123)	{
  0b00000110, // (124)	|
  0b01110000, // (125)	}
  0b00000001, // (126)	~
};

#endif // TM16XX_H
