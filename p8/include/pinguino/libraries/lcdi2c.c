/*  --------------------------------------------------------------------
    FILE:           lcdi2c.c
    PROJECT:        Pinguino - http://www.pinguino.cc/
    PURPOSE:        Driving a LCD display through i2c PCF8574 I/O expander
    PROGRAMER:      Regis Blanchot <rblanchot@gmail.com>
    --------------------------------------------------------------------
    CHANGELOG:
    29 Jul. 2008    Regis Blanchot - first release
    15 Jun. 2011    Regis Blanchot - added print functions
    25 Nov. 2016    Regis Blanchot - added multi I2C module support
    28 Nov. 2016    Regis Blanchot - replaced all global variables with LCDI2C struct
    13 Mar. 2017    Regis Blanchot - fixed backlight routine
    --------------------------------------------------------------------
    TODO:
    * Manage other I/O expander (cf MCP23S17 / MCP342x / MCP23017 libraries)
    --------------------------------------------------------------------
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
    ------------------------------------------------------------------*/

/*  --------------------------------------------------------------------
    LCD 2x16 (GDM1602A with build-in Samsung KS0066/S6A0069)
    --------------------------------------------------------------------

    01 - VSS (GRND)
    02 - VDD (+5V)
    03 - Vo (R = 1KOhm à la masse)
    04 - RS
    05 - RW (can be connected to GND so that RW = 0 = write)
    06 - EN
    07 a 10 - D0 to D3 connected to GND (4-bit mode).
    11 a 16 - D4 to D7 connected to PCF8574's pins (see below)
    15 - LED+ R330
    16 - LED- GND or Backlight pin in PCF8574
    ------------------------------------------------------------------*/

/*  --------------------------------------------------------------------
    ---------- PCF8574P
    --------------------------------------------------------------------

    +5V     A0 -|o  |- VDD  +5V
    +5V     A1 -|   |- SDA  pull-up 1K8 au +5V
    +5V     A2 -|   |- SCL  pull-up 1K8 au +5V
            P0 -|   |- INT
            P1 -|   |- P7
            P2 -|   |- P6
            P3 -|   |- P5
    GRND   VSS -|   |- P4

    SYMBOL  PIN DESCRIPTION                     NB
    A0      1   address input 0                 adress = 0 1 0 0 A2 A1 A0 0
    A1      2   address input 1                 A0, A1 et A2 relies au +5V
    A2      3   address input 2                 so adress = 01001110 = 0x4E
    P0      4   quasi-bidirectional I/O 0
    P1      5   quasi-bidirectional I/O 1
    P2      6   quasi-bidirectional I/O 2
    P3      7   quasi-bidirectional I/O 3
    VSS     8   supply ground
    P4      9   quasi-bidirectional I/O 4
    P5      10  quasi-bidirectional I/O 5
    P6      11  quasi-bidirectional I/O 6
    P7      12  quasi-bidirectional I/O 7
    INT     13  interrupt output (active LOW)
    SCL     14  serial clock line               Pinguino SCL
    SDA     15  serial data line                Pinguino SDA
    VDD     16  supply voltage
    ------------------------------------------------------------------*/

#ifndef __LCDI2C_C
#define __LCDI2C_C

#include <typedef.h>
#include <const.h>
//#include <macro.h>              // interrupt(), noInterrupt(), ...
#include <lcdi2c.h>
#include <stdarg.h>
#ifndef __PIC32MX__
#include <delayms.c>
//#include <delayus.c>
#else
#include <delay.c>
#endif
#include <i2c.c>

// Printf
#ifdef LCDI2CPRINTF
    #include <printFormated.c>
#endif

// PrintFloat
#if defined(LCDI2CPRINTFLOAT)
    #include <printFloat.c>
#endif

// PrintNumber
#if defined(LCDI2CPRINTNUMBER) || defined(LCDI2CPRINTFLOAT)
    #include <printNumber.c>
#endif

/*  --------------------------------------------------------------------
    Défintion des caractères spéciaux
    --------------------------------------------------------------------
    â, à, ç, é, î, ô, ù, ê, è, ë, ï, û, €
    usage :
    1/ réservation de l'emplacement 0 (max. 7) pour la lettre "é" :
       lcdi2c_newchar(car3, 0);
    2/ écriture du nouveau caractère sur le LCD : lcdi2c_write(0);
    ------------------------------------------------------------------*/
#if 0
    const u8 car0[8]={
        0b00000100,      //â
        0b00001010,
        0b00001110,
        0b00000001,
        0b00001111,
        0b00010001,
        0b00001111,
        0b00000000
    };
    const u8 car1[8]={
        0b00000100,     //à
        0b00000010,
        0b00001110,
        0b00000001,
        0b00001111,
        0b00010001,
        0b00001111,
        0b00000000
    };
    const u8 car2[8]={
        0b00001110,     //ç
        0b00010000,
        0b00010000,
        0b00010001,
        0b00001110,
        0b00000100,
        0b00001100,
        0b00000000
    };
    const u8 car3[8]={
        0b00000100,     //é
        0b00001000,
        0b00001110,
        0b00010001,
        0b00011111,
        0b00010000,
        0b00001110,
        0b00000000
    };
    const u8 car4[8]={
        0b00000100,     //è
        0b00000010,
        0b00001110,
        0b00010001,
        0b00011111,
        0b00010000,
        0b00001110,
        0b00000000
    };
    const u8 car5[8]={
        0b00000100,     //ê
        0b00001010,
        0b00001110,
        0b00010001,
        0b00011111,
        0b00010000,
        0b00001110,
        0b00000000
    };
    const u8 car6[8]={
        0b00001010,     //ë
        0b00000000,
        0b00001110,
        0b00010001,
        0b00011111,
        0b00010000,
        0b00001110,
        0b00000000
    };
    const u8 car7[8]={
        0b00000111,     //€
        0b00001000,
        0b00011110,
        0b00001000,
        0b00011110,
        0b00001000,
        0b00000111,
        0b00000000
    };
    const u8 car8[8]={
        0b00000100,     //î
        0b00001010,
        0b00001100,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00001110,
        0b00000000
    };
    const u8 car9[8]={
        0b00001010,     //ï
        0b00000000,
        0b00001100,
        0b00000100,
        0b00000100,
        0b00000100,
        0b00001110,
        0b00000000
    };
    const u8 car10[8]={
        0b00000100,     //ô
        0b00001010,
        0b00001110,
        0b00010001,
        0b00010001,
        0b00010001,
        0b00001110,
        0b00000000
    };
    const u8 car11[8]={
        0b00000100,     //ù
        0b00000010,
        0b00010001,
        0b00010001,
        0b00010001,
        0b00010011,
        0b00001101,
        0b00000000
    };
    const u8 car12[8]={
        0b00000100,     //û
        0b00001010,
        0b00010001,
        0b00010001,
        0b00010001,
        0b00010011,
        0b00001101,
        0b00000000
    };
#endif

/*  --------------------------------------------------------------------
    global variables
    ------------------------------------------------------------------*/

    volatile LCDI2C_t LCDI2C[NUMOFI2C];
    volatile u8 gI2C_module;

/*  --------------------------------------------------------------------
    Send upper 4 bits of a byte to the PCF8574
    --------------------------------------------------------------------
    Send a half-byte (value) to pin the PCF8574
    @param value = 4 bits to send
    @param mode = LCD Command (LCD_CMD) or Data (LCD_DATA) mode
    ------------------------------------------------------------------*/

static void lcdi2c_send4(u8 module, u8 value, u8 mode)
{
    //u8 status = isInterrupts();

    BitWrite(LCDI2C[module].data, LCDI2C[module].pin.d4, value & Bit(0));
    BitWrite(LCDI2C[module].data, LCDI2C[module].pin.d5, value & Bit(1));
    BitWrite(LCDI2C[module].data, LCDI2C[module].pin.d6, value & Bit(2));
    BitWrite(LCDI2C[module].data, LCDI2C[module].pin.d7, value & Bit(3));
    
    // LCD is in "write mode" when RW is tied to ground : RW = 0
    BitWrite(LCDI2C[module].data, LCDI2C[module].pin.rw, 0);
    BitWrite(LCDI2C[module].data, LCDI2C[module].pin.rs, mode);
    
    /// ---------- LCD Enable Cycle

    //if (status) noInterrupts();    
    
    I2C_start(module);                            // send start condition

    I2C_write(module, LCDI2C[module].address);

    BitSet(LCDI2C[module].data, LCDI2C[module].pin.en);
    I2C_write(module, LCDI2C[module].data);
    // E Pulse Width > 300ns

    BitClear(LCDI2C[module].data, LCDI2C[module].pin.en);
    I2C_write(module, LCDI2C[module].data);
    // E Enable Cycle > (300 + 200) = 500ns

    I2C_stop(module);                             // send stop confition

    //if (status) interrupts();    
}

/*  --------------------------------------------------------------------
    Ecriture d'un value dans le LCD en mode 4 bits
    --------------------------------------------------------------------
    Les données sont écrites en envoyant séquentiellement :
    1/ les quatre bits de poids fort
    2/ les quatre bits de poids faible
    NB : les poids sont stockes dans les quatre bits de poids fort
    qui correspondent aux pins D4 a D7 du LCD ou du PCF8574
    @param value = value a envoyer au LCD
    @param mode = LCD Command (LCD_CMD) or Data (LCD_DATA) mode
    ------------------------------------------------------------------*/

static void lcdi2c_send8(u8 module, u8 value, u8 mode)
{
    //I2C_start(module);                              // send start condition
    lcdi2c_send4(module, value >> 4, mode);         // send upper 4 bits
    lcdi2c_send4(module, value & 0x0F, mode);       // send lower 4 bits
    //I2C_stop(module);                               // send stop confition

    // Wait for instruction excution time (more than 46us)
    #ifdef __XC8__
    Delayms(5);    //Delayus(46);
    #endif
}

/*  --------------------------------------------------------------------
    backlight
    NB : PCF8574 is logical inverted
    ------------------------------------------------------------------*/

#if defined(LCDI2CBACKLIGHT)
void lcdi2c_backlight(u8 module)
{
    //LCDI2C[module].backlight = 1;
    //LCDI2C[module].polarity  = polarity;
    BitWrite(LCDI2C[module].data, LCDI2C[module].pin.bl, LCDI2C[module].polarity);
}
#endif

#if defined (LCDI2CNOBACKLIGHT)
void lcdi2c_noBacklight(u8 module)
{
    //LCDI2C[module].backlight = 0;
    //LCDI2C[module].polarity  = polarity;
    BitWrite(LCDI2C[module].data, LCDI2C[module].pin.bl, 1-LCDI2C[module].polarity);
}
#endif

/*  --------------------------------------------------------------------
    Positionne le curseur sur le LCD
    from (0,0) to (15,1)
    ------------------------------------------------------------------*/

#if defined(LCDI2CSETCURSOR)
void lcdi2c_setCursor(u8 module, u8 col, u8 line)
{
    const u8 row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };

    if (col > LCDI2C[module].width)
        col = LCDI2C[module].width;  // - 1;           // we count rows starting w/0

    if (line > LCDI2C[module].height)
        line = LCDI2C[module].height;// - 1;           // we count rows starting w/0

    lcdi2c_send8(module, LCD_SETDDRAMADDR | (col + row_offsets[line]), LCD_CMD);
}
#endif

/*  --------------------------------------------------------------------
    Efface une ligne
    ------------------------------------------------------------------*/

#if defined(LCDI2CCLEARLINE)
void lcdi2c_clearLine(u8 module, u8 line)
{
    u8 i;

    lcdi2c_setCursor(module, 0, line);
    for (i = 0; i <= LCDI2C[module].width; i++)
        lcdi2c_printChar(module, SPACE);                // display spaces
}
#endif

/*  --------------------------------------------------------------------
    Affiche un caractere ASCII a la position courante du curseur
    c = code ASCII du caractere
    ------------------------------------------------------------------*/

void lcdi2c_printChar(u8 module, u8 c)
{
    if (c < 32) c = 32;                     // replace ESC char with space
    lcdi2c_send8(module, c, LCD_DATA);
}
#endif

void lcdi2c_putChar(u8 c)
{
    if (c < 32) c = 32;                     // replace ESC char with space
    lcdi2c_send8(gI2C_module, c, LCD_DATA);
}

/*  --------------------------------------------------------------------
    print
    ------------------------------------------------------------------*/

#if defined(LCDI2CPRINT)       || defined(LCDI2CPRINTLN)    || \
    defined(LCDI2CPRINTNUMBER) || defined(LCDI2CPRINTFLOAT) || \
    defined(LCDI2CPRINTCENTER)

void lcdi2c_print(u8 module, const u8 *string)
{
    gI2C_module = module;
    while (*string)
        lcdi2c_putChar(*string++);
}

#endif

/*  --------------------------------------------------------------------
    println : useless on a LCD
    ------------------------------------------------------------------*/

#if 0
#if defined(LCDI2CPRINTLN)
void lcdi2c_println(u8 module, const u8 *string)
{
    lcdi2c_print(string);
    lcdi2c_print((u8*)"\n\r");
}
#endif
#endif

/*  --------------------------------------------------------------------
    printCenter : centers text in line
    ------------------------------------------------------------------*/

#if defined(LCDI2CPRINTCENTER)
void lcdi2c_printCenter(u8 module, const u8 *string)
{
    u8 len=0, nbspace;
    const u8 *p = string;

    while (*p++) len++;
    nbspace = (LCDI2C[module].width + 1 - len) / 2;
    
    // write spaces before
    while(nbspace--)
        lcdi2c_send8(module, (u8)SPACE, LCD_DATA);

    // write string
    lcdi2c_print(module, string);
}
#endif

/*  --------------------------------------------------------------------
    printNumber
    ------------------------------------------------------------------*/

#if defined(LCDI2CPRINTNUMBER) || defined(LCDI2CPRINTFLOAT)
void lcdi2c_printNumber(u8 module, s32 value, u8 base)
{
    gI2C_module = module;
    printNumber(lcdi2c_putChar, value, base);
}
#endif

/*  --------------------------------------------------------------------
    printFloat
    ------------------------------------------------------------------*/

#if defined(LCDI2CPRINTFLOAT)
void lcdi2c_printFloat(u8 module, float number, u8 digits)
{
    gI2C_module = module;
    printFloat(lcdi2c_putChar, number, digits);
}
#endif

/*  --------------------------------------------------------------------
    printf
    ------------------------------------------------------------------*/

#if defined(LCDI2CPRINTF)

void lcdi2c_printf(u8 module, char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    gI2C_module = module;
    pprintf(lcdi2c_putChar, fmt, args);
    va_end(args);
}

void lcdi2c1_printf(char* fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    gI2C_module = I2C1;
    pprintf(lcdi2c_putChar, fmt, args);
    //lcdi2c_printf(I2C1, fmt, args)
    va_end(args);
}

void lcdi2c2_printf(char* fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    gI2C_module = I2C2;
    pprintf(lcdi2c_putChar, fmt, args);
    //lcdi2c_printf(I2C2, fmt, args)
    va_end(args);
}
#endif

/*  --------------------------------------------------------------------
    Définit un caractère personnalisé de 8x8 points.
    --------------------------------------------------------------------
    d'après Nabil Al-HOSSRI <http://nalhossri.free.fr>
    Le LCD utilisé admet au maximum 8 caractères spéciaux.
    char_code : code du caractère à définir (0 <= char_code <= 7)
    lcdi2c_newchar(car3, 0); Définit le caractère 'é' à l'adresse 0 de la mémoire CG RAM
    lcdi2c_newchar(car8, 1); Définit le caractère 'è' à l'adresse 1.
    ------------------------------------------------------------------*/

#if defined(LCDI2CNEWCHAR)
void lcdi2c_newchar(u8 module, const u8 *c, u8 char_code)
{
    u8 i, a;

    // les caractères sont logés dans le CGRAM du LCD à partir de l'adresse 0x40.
    a = (char_code << 3) | LCD_ADRESS_CGRAM;
    for (i=0; i<8; i++)
    {
        lcdi2c_send8(module, a, LCD_CMD);
        lcdi2c_send8(module, c[i], LCD_DATA);
        a++;
    };
}
#endif

/*  --------------------------------------------------------------------
    Définition de 8 nouveaux caractères
    ------------------------------------------------------------------*/
/*
void lcdi2c_newpattern()
{
    lcdi2c_newchar(car0,  ACIRC);		// â
    lcdi2c_newchar(car1,  AGRAVE);		// à

    lcdi2c_newchar(car2,  CCEDIL);		// ç

    lcdi2c_newchar(car3,  EACUTE);		// é
    lcdi2c_newchar(car4,  EGRAVE);		// è
    lcdi2c_newchar(car5,  ECIRC);		// ê
    lcdi2c_newchar(car6,  ETREMA);		// ë
    lcdi2c_newchar(car7,  EURO);		// €

    //lcdi2c_newchar(car8,  ICIRC);		// î
    //lcdi2c_newchar(car9, ITREMA);		// ï

    //lcdi2c_newchar(car10,  OCIRC);	// ô

    //lcdi2c_newchar(car11,  UGRAVE);	// ù
    //lcdi2c_newchar(car12, UCIRC);		// û
}
*/
/*  --------------------------------------------------------------------
    Initialisation du LCD
    --------------------------------------------------------------------
    This function must be called before any other function.
    No need to wait between 2 commands because i2c bus is quite slow.
    cf. Microchip AN587 Interfacing PICmicro® MCUs to an LCD Module
    --------------------------------------------------------------------
    PCF8574  7-bit slave adress format is [0 1 0 0 A2 A1 A0]
    PCF8574A 7-bit slave adress format is [0 1 1 1 A2 A1 A0]
    --------------------------------------------------------------------
    usage ex. : lcdi2c.init(I2C1, 16, 2, 0x27, 4, 2, 1, 0, 3);
    ------------------------------------------------------------------*/

void lcdi2c_init(u8 module, u8 numcol, u8 numline, u8 i2c_address, u8 rs, u8 rw, u8 en, u8 d4, u8 d5, u8 d6, u8 d7, u8 bl, u8 polarity)
{
    u8 cmd8bits = 0x03, cmd4bits = 0x02;
    
    LCDI2C[module].width   = numcol - 1;
    LCDI2C[module].height  = numline - 1;
    LCDI2C[module].address = (i2c_address << 1) & 0xFE; // Write mode = bit 0 to 0
    LCDI2C[module].data    = 0;

    LCDI2C[module].pin.rs  = rs;
    LCDI2C[module].pin.rw  = rw;
    LCDI2C[module].pin.en  = en;
    LCDI2C[module].pin.d4  = d4;
    LCDI2C[module].pin.d5  = d5;
    LCDI2C[module].pin.d6  = d6;
    LCDI2C[module].pin.d7  = d7;
    LCDI2C[module].pin.bl  = bl;

    LCDI2C[module].polarity  = polarity;

    gI2C_module = module;

    /*
    if (LCDI2C[module].pin.d4 != 0)
    {
        cmd8bits = 0x30; // MSB of LCD_SYSTEM_SET_8BITS
        cmd4bits = 0x20; // MSB of LCD_SYSTEM_SET_4BITS
    }
    */
    
    I2C_init(module, I2C_MASTER_MODE, I2C_100KHZ);
    //I2C_init(module, I2C_MASTER_MODE, I2C_400KHZ);
    //I2C_init(module, I2C_MASTER_MODE, I2C_1MHZ);

    Delayms(15);                                // Wait more than 15 ms after VDD rises to 4.5V

    //I2C_start(module);                          // send start condition
    lcdi2c_send4(module, cmd8bits, LCD_CMD);    // 0x30 - Mode 8 bits
    Delayms(5);                                 // Wait for more than 4.1 ms
    lcdi2c_send4(module, cmd8bits, LCD_CMD);    // 0x30 - Mode 8 bits
    Delayms(5);                                 // Wait for more than 4.1 ms
    //Delayus(100);                             // Wait more than 100 μs
    lcdi2c_send4(module, cmd8bits, LCD_CMD);    // 0x30 - Mode 8 bits
    //Delayus(100);                             // Wait more than 100 μs
    lcdi2c_send4(module, cmd4bits, LCD_CMD);    // 0x20 - Mode 4 bits
    //I2C_stop(module);                           // send stop condition
    lcdi2c_send8(module, LCD_SYSTEM_SET_4BITS, LCD_CMD);// 0x28 - Mode 4 bits - 2 Lignes - 5x8
    //Delayus(4);                               // Wait more than 40 ns
    lcdi2c_send8(module, LCD_DISPLAY_ON, LCD_CMD);// 0x0C - Display ON + Cursor OFF + Blinking OFF
    //Delayus(4);                               // Wait more than 40 ns
    lcdi2c_send8(module, LCD_DISPLAY_CLEAR, LCD_CMD);   // 0x01 - Efface l'affichage + init. DDRAM
    Delayms(2);                                 // Execution time > 1.64ms
    lcdi2c_send8(module, LCD_ENTRY_MODE_SET, LCD_CMD);// 0x06 - Increment + Display not shifted (Déplacement automatique du curseur)
    //Delayus(4);                               // Wait more than 40 ns
    #if 0
    lcdi2c_newpattern(module);                  // Set new characters
    #endif
}

#endif
