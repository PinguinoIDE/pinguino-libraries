/*  --------------------------------------------------------------------
    FILE:          printFormated.c
    PROJECT:       Pinguino - http://www.pinguino.cc/
    PURPOSE:       Alternative printf and sprintf functions
    PROGRAMERS:	   Regis Blanchot <rblanchot@gmail.com>
                   Mark Harper <markfh@f2s.com>
    --------------------------------------------------------------------
    TODO : thousands separator
    --------------------------------------------------------------------
    CHANGELOG
    10 Nov. 2010 - Régis Blanchot - first release
    08 Feb. 2016 - Régis Blanchot - excluded float support (%f) for the 16F1459
    28 Nov. 2016 - Régis Blanchot - updated to have the same file for P8 and P32
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

#ifndef __PRINTF_C
#define __PRINTF_C

#include <stdarg.h>             // variable args support
#include <typedef.h>            // u8, u16, u32, funcout, ...
#include <const.h>              // BIN, DEC, HEX, ...

#define PRINTF_BUF_LEN  12      // should be enough for 32 bits
#define PAD_RIGHT       1
#define PAD_ZERO        2
#define SIGNED          1
#define UNSIGNED        0
#define UPPERCASE       'A'
#define LOWERCASE       'a'

funcout pputchar;               // void pputchar(u8)

/*  --------------------------------------------------------------------
    pprintc = pinguino print char
    ------------------------------------------------------------------*/

void pprintc(u8 **str, u8 c)
{
    if (str)
    {
        **str = c;
        ++(*str);
    }
    else
    {
        pputchar(c);
    }
}

/*  --------------------------------------------------------------------
    pprints = pinguino print string
    --------------------------------------------------------------------
    out     : pointer on output string or function (if null)
    string  : pointer on string to output
    width   : number of Zeros or Spaces
    pad     : PAD_RIGHT or PAD_ZERO
    return  : string's length
    ------------------------------------------------------------------*/

u8 pprints(u8 **out, const u8 *string, u8 width, u8 pad)
{
    u8 pc = 0;
    u8 padchar = ' ';
    u8 len = 0;
    const u8 *ptr;

    // string length calculation
    for (ptr = string; *ptr; ++ptr)
        ++len;

    if (width > 0)
    {
        if (len >= width)
        {
            len = width;
            width = 0;
        }
        else
            width -= len;
            
        if (pad & PAD_ZERO) padchar = '0';
    }
    
    if (!(pad & PAD_RIGHT))
    {
        for ( ; width > 0; --width)
        {
            pprintc(out, padchar);
            ++pc;
        }
    }
    
    for ( ; *string && len--; ++string)
    {
        pprintc(out, *string);
        ++pc;
    }
    
    for ( ; width > 0; --width)
    {
        pprintc(out, padchar);
        ++pc;
    }

    return pc;
}

/*  --------------------------------------------------------------------
    pprinti = pinguino print 32-bit signed or unsigned integer
    --------------------------------------------------------------------
    i          : 32-bit number to convert into string
    base       : 1 byte, 2 binary, 8 octal, 10 decimal, 16 hexadecimal
    sign       : 0 unsigned, 1 signed
    width      : number of Zeros or Spaces
    pad        : PAD_RIGHT or PAD_ZERO
    lettercase : LOWERCASE or UPPERCASE
    return     : string's length
    ------------------------------------------------------------------*/

u8 pprinti(u8 **out, u32 i, u8 islong, u8 base, u8 sign, u8 width, u8 pad, u8 separator, u8 lettercase)
{
    u8 buffer[PRINTF_BUF_LEN];
    u8 *string;
    u8 neg = 0, pc = 0;
    u32 t, uns32 = i;

    if (i == 0)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
        return pprints(out, buffer, width, pad);
    }

    // Do we have a negative decimal number ?
    if  ( (sign) && (base == 10) )          // decimal signed number ?
    {
        if ( (islong) && ((s32)i < 0) )     // negative 32-bit ?
        {
            neg = 1;
            uns32 = - (s32)i;
        }
        if ( (!islong) && ((s16)i < 0) )    // negative 16-bit ?
        {
            neg = 1;
            uns32 = - (s16)i;
        }
    }
    
    /* Old P32 sequence
    if ( (sign) && (base == 10) && ( (s32)i < 0 ) )
    {
        neg = 1;
        uns32 = - (s32)i;
    }
    */

    // we start at the end
    string = buffer + PRINTF_BUF_LEN - 1;
    *string = '\0';

    while (uns32)
    {
        t = uns32 % base;
        if ( t >= 10 )
            t += lettercase - '0' - 10;
        *--string = t + '0';
        uns32 /= base;
    }

    if (neg)
    {
        if (width && (pad & PAD_ZERO))
        {
            pprintc(out, '-');
            ++pc;
            --width;
        }
        else
        {
            *--string = '-';
        }
    }

    return pc + pprints(out, string, width, pad);
}

#if !defined(__16F1459) && !defined(__18f13k50) && !defined(__18f14k50)
/*  --------------------------------------------------------------------
    pprintfl = pinguino print float
    --------------------------------------------------------------------
    The IEEE 754 standard specifies a binary32 (float) as having:
        Sign bit: 1 bit
        Exponent width: 8 bits
        Significand precision: 24 bits (23 explicitly stored)
    --------------------------------------------------------------------
    out        : pointer to output function
    value      : floating point value
    width      : number of Zeros or Spaces
    pad        : PAD_RIGHT or PAD_ZERO
    separator  : thousands separator (1=ON, 0=OFF)
    precision  : number of digits after comma (from 0 to 6)
    return     : string's length
    ------------------------------------------------------------------*/

#ifndef __PIC32MX__
u8 pprintfl(u8 **out, float value, u8 width, u8 pad, u8 separator, u8 precision)
#else
u8 pprintfl(u8 **out, double value, u8 width, u8 pad, u8 separator, u8 precision)
#endif
{
    u8 i, toPrint;
    u32 int_part;
    float frac_part, rounding;

    u8 buffer[PRINTF_BUF_LEN], *string = buffer;
    u8 tmp[PRINTF_BUF_LEN], *s = tmp;
    u8 count = 0, m = 0;
    u8 length = PRINTF_BUF_LEN - 1;
    
    // Handle negative numbers
    // -----------------------------------------------------------------
    
    if (value < 0.0)
    {
        if (width && (pad & PAD_ZERO))
        {
            pprintc(out, '-');
            ++count;
            --width;
        }
        else
        {
            *string++ = '-';
            length--;
        }
        value = -value;
    }

    // Round correctly so that print(1.999, 2) prints as "2.00"
    // -----------------------------------------------------------------
    /*
    rounding = 0.5;
    for (i=0; i<precision; ++i)
        rounding /= 10.0;
    value += rounding;
    */
    // Extract the integer part of the number and print it  
    // -----------------------------------------------------------------
    
    int_part  = (u32)value;
    frac_part = value - (float)int_part;

    // the string is more easily written backwards
    while (int_part)
    {
        *s++ = int_part % 10 + '0';// decimal base
        int_part /= 10;
        m++;                    // string's length counter
        length--;               // space remaining
    }
    // now the string is written in the right direction
    while (m--)
    {
        *string++ = *--s;
        /*----- TODO separator -------------------------------------
        if ( separator && (m % 3 == 0) )
        {
            pprintc(out, ' ');
            ++count;
            --width;
        }
        ----------------------------------------------------------*/
    }

    // Add fractional part to string
    // -----------------------------------------------------------------
    
    if (precision > 6)
        precision = 6;

    // check if we still have enough space
    if (precision > length)
        precision = length;
 
    // otherwise, number has no fractional part
    if (precision > 0)
    {
        // add the decimal point to string
        *string++ = '.';

        while (precision-- > 0)
        //for (m = 0; m < precision; m++)
        {
            // Extract digits from the frac_part one at a time
            // multiplies frac_part by 10 by adding 8 times and 2 times; 
            //frac_part = (frac_part << 3) + (frac_part << 1);
            frac_part *= 10.0;
            toPrint = (unsigned int)frac_part;
            // print binary-coded decimal (BCD)
            // converts leading digits to number character
            //*string++ = (frac_part >> 24) + '0';
            *string++ =  toPrint + '0';
            // strips off leading digits
            //frac_part &= 0xFFFFFF;
            frac_part -= (float)toPrint;
        }
    }

    /*
    union
    {
        float f;
        u32 l;
    } helper;
    /
    
    u32 helper;
    u32 int_part  = 0;
    u32 frac_part = 0;

    u32 mantissa;   // bits <0:22>
    s8  exponent;    // bits <23:30>
    u8  sign;        // bit  <31>

    u8 buffer[PRINTF_BUF_LEN], *string = buffer;
    u8 tmp[PRINTF_BUF_LEN], *s = tmp;
    u8 count = 0, m, t;
    u8 length = PRINTF_BUF_LEN - 1;
   
    /* 
    #ifndef __PIC32MX__
    helper.f = value;
    #else
    helper.f = (float)value;
    #endif
    /
    
    // takes last 23 bits and adds the implicit 1
    //mantissa = (helper.l & 0x7FFFFF) | 0x800000;
    helper = *((u32*)&value);
    mantissa = (helper & 0x7FFFFF) | 0x800000;
    
    // shifts the 23 bits of mantissa out,
    // takes the next 8 bits
    // then subtracts 127 to get an exponent value in the range −126 .. +127
    //exponent = ((helper.l >> 23) & 0xFF) - 127;
    helper = helper >> 23;
    exponent = (helper & 0xFF) -127 ;
    
    // sign is 31st bit
    //sign = helper.l >> 31;
    helper = helper >> 8;
    sign = (helper & 0x01);

    // add negative sign if applicable
    // (1 for negative, 0 for positive)
    //if (helper.l < 0)
    if (sign)
    {
        if (width && (pad & PAD_ZERO))
        {
            pprintc(out, '-');
            ++count;
            --width;
        }
        else
        {
            *string++ = '-';
            length--;
        }
    }

    /*
    if ( (exponent >= 31) || (exponent < -23) )
    {
        buffer[0] = 'i';
        buffer[1] = 'n';
        buffer[2] = 'f';
        buffer[3] = '\0';
        return pprints(out, buffer, width, pad);
    }
    /
    
    if (exponent >= 38)
    {
        buffer[0] = '+';
        buffer[1] = 'i';
        buffer[2] = 'n';
        buffer[3] = 'f';
        buffer[4] = '\0';
        return pprints(out, buffer, width, pad);
    }

    else if (exponent < -38)
    {
        buffer[0] = '-';
        buffer[1] = 'i';
        buffer[2] = 'n';
        buffer[3] = 'f';
        buffer[4] = '\0';
        return pprints(out, buffer, width, pad);
        /*
        int_part  = 0;
        frac_part = 0;
        /
    }

    else if (exponent >= 23)
    {
        int_part = mantissa << (exponent - 23);
    }

    else if (exponent >= 0) 
    {
        int_part  = mantissa >> (23 - exponent);
        frac_part = (mantissa << (exponent + 1)) & 0xFFFFFF; // mfh
    }

    else // if (exponent < 0)
        frac_part = (mantissa & 0xFFFFFF) >> -(exponent + 1);

    // add integer part to string
    if (int_part == 0)
    {
        *string++ = '0';
        length--;
    }

    else
    {
        m = 0;
        // the string is more easily written backwards
        while (int_part)
        {
            t = int_part % 10;      // decimal base
            *s++ = t + '0';
            int_part /= 10;
            m++;                    // string's length counter
            length--;
        }
        // now the string is written in the right direction
        while (m--)
        {
            *string++ = *--s;
            /*----- TODO separator -------------------------------------
            if ( separator && (m % 3 == 0) )
            {
                pprintc(out, ' ');
                ++count;
                --width;
            }
            ----------------------------------------------------------/
        }
    }
    
    // add fractional part to string
    if (precision > 6)
        precision = 6;

    // check if we have enough space
    if (precision > length)
        precision = length;
 
    // otherwise, number has no fractional part
    if (precision >= 1)
    {
        // add the decimal point to string
        *string++ = '.';

        // print binary-coded decimal (BCD)
        for (m = 0; m < precision; m++)
        {
            // multiplies frac_part by 10 by adding 8 times and 2 times; 
            frac_part = (frac_part << 3) + (frac_part << 1); 
            // converts leading digits to number character
            *string++ = (frac_part >> 24) + '0';
            // strips off leading digits
            frac_part &= 0xFFFFFF;
        }
    }
    */

    // end of string
    *string++ = '\0';

    return count + pprints(out, buffer, width, pad);
}

#endif // !defined(16F1459)

/*  --------------------------------------------------------------------
    pprint = pinguino print
    --------------------------------------------------------------------
    out     : pointer on output string or function (if null)
    format  : pointer on string with % tags
    args    : list of variable arguments
    ------------------------------------------------------------------*/

u8 pprint(u8 **out, const u8 *format, va_list args)
{
    u8 pc = 0;
    u8 width, pad, islong;
    u8 precision = 2;               // default value is 2 digits fractional part
    u8 separator = 0;               // no thousands separator
    u8 scr[2];
    u32 val;

    for (; *format != 0; ++format)
    {
        val = 0;
        #ifndef __PIC32MX__
        islong = 0;                 // default is 16-bit
        #else
        islong = 1;                 // default is 32-bit
        #endif
    
        if (*format == '%')
        {
            width = pad = 0;        // default is left justify, no zero padded
            ++format;               // get the next format identifier

            // end of line
            if (*format == '\0')
                break;

            // error
            if (*format == '%')
                goto abort;

            // right justify
            if (*format == '-')
            {
                ++format;
                pad = PAD_RIGHT;
            }

            // field is padded with 0's instead of blanks
            if (*format == '0')
            {
                ++format;
                pad |= PAD_ZERO;
            }
            
            // how many digits ?
            for ( ; *format >= '0' && *format <= '9'; ++format)
            {
                width *= 10;
                width += *format - '0';
            }

            /*---- TODO thousands separator ----------------------------
            if (*format == '\'')
            {
                separator = 1;
                ++format;
            }
            ----------------------------------------------------------*/

            /*--------------------------------------------------------*/
            #if !defined(__16F1459) && !defined(__18f13k50) && !defined(__18f14k50)

            // float precision
            if (*format == '.')
            {
                ++format;
                precision = 0;
                // get number of digits after decimal point
                for ( ; *format >= '0' && *format <= '9'; ++format)
                {
                    precision *= 10;
                    precision += *format - '0';
                }
            }
            
            // float
            if (*format == 'f')
            {
                #ifndef __PIC32MX__
                pc += pprintfl(out, va_arg(args, float), width, pad, separator, precision);
                #else
                pc += pprintfl(out, va_arg(args, double), width, pad, separator, precision);
                #endif
                continue;
            }
            
            #endif // !defined(__16F1459)
            /*--------------------------------------------------------*/

            // string
            if (*format == 's')
            {
                //RB20150131
                //u8 *s = va_arg(args, u8*);
                //pc += pprints(out, s?s:"(null)", width, pad);
                const u8 *s = va_arg(args, char*);
                if (s)
                    pc += pprints(out, s, width, pad);
                else
                    pc += pprints(out, (const u8 *)"(null)", width, pad);
                continue;
            }
            
            // long support
            if (*format == 'l')
            {
                ++format;
                islong = 1;
            }

            // decimal (10) unsigned (0) integer
            if (*format == 'u')
            {
                // NB : P8 int is u16
                val = (islong) ? va_arg(args, u32) : va_arg(args, u16);
                pc += pprinti(out, val, islong, DEC, UNSIGNED, width, pad, separator, LOWERCASE);
                continue;
            }

            // decimal (10) signed (1) integer
            if (*format == 'd' || *format == 'i')
            {
                // NB : P8 int is u16
                val = (islong) ? va_arg(args, u32) : va_arg(args, u16);
                pc += pprinti(out, val, islong, DEC, SIGNED, width, pad, separator, LOWERCASE);
                continue;
            }

            // unsigned (0) lower (LOWERCASE) hexa (16) or pointer
            if (*format == 'x' || *format == 'p')
            {
                // NB : P8 int is u16
                val = (islong) ? va_arg(args, u32) : va_arg(args, u16);
                pc += pprinti(out, val, islong, HEX, UNSIGNED, width, pad, separator, LOWERCASE);
                continue;
            }

            // unsigned (0) upper (UPPERCASE) hexa (16) or pointer
            if (*format == 'X' || *format == 'P')
            {
                // NB : P8 int is u16
                val = (islong) ? va_arg(args, u32) : va_arg(args, u16);
                pc += pprinti(out, val, islong, HEX, UNSIGNED, width, pad, separator, UPPERCASE);
                continue;
            }

            // binary
            if (*format == 'b')
            {
                // NB : P8 int is u16
                val = (islong) ? va_arg(args, u32) : va_arg(args, u16);
                pc += pprinti(out, val, islong, BIN, UNSIGNED, width, pad, separator, LOWERCASE);
                continue;
            }

            /*--------------------------------------------------------*/
            #if !defined(__16F1459) && !defined(__18f13k50) && !defined(__18f14k50)

            // octal
            if (*format == 'o')
            {
                // NB : P8 int is u16
                val = (islong) ? va_arg(args, u32) : va_arg(args, u16);
                pc += pprinti(out, val, islong, OCT, UNSIGNED, width, pad, separator, LOWERCASE);
                continue;
            }
            
            #endif
            /*--------------------------------------------------------*/
            
            // ASCII
            if (*format == 'c')
            {
                #ifndef __PIC32MX__
                scr[0] = va_arg(args, u16);
                #else
                scr[0] = (u8)va_arg(args, u32);
                #endif
                scr[1] = '\0';
                pc += pprints(out, scr, width, pad);
                continue;
            }

        }
        else
        {
            abort:
            pprintc(out, *format);
            ++pc;
        }
    }
    if (out) **out = '\0';

    return pc;
}

/*  --------------------------------------------------------------------
    pprintf = pinguino print formatted
    --------------------------------------------------------------------
    func    : pointer on output function
    format  : pointer on string with % tags
    args    : list of variable arguments
    return  : string's length
    ------------------------------------------------------------------*/

//int pprintf(funcout func, const char *format, ...)
u8 pprintf(funcout func, const u8 *format, va_list args)
{
    pputchar = func;
    return pprint(0, format, args);
}

/*  --------------------------------------------------------------------
    psprintf2 = pinguino print formatted data to string
    --------------------------------------------------------------------
    out     : pointer on output string or function (if null)
    format  : pointer on string with % tags
    args    : list of variable arguments
    return  : string's length
    Note    : this function is called from CDC.printf only
    ------------------------------------------------------------------*/

u8 psprintf2(u8 *out, const u8 *format, va_list args)
{
    return pprint(&out, format, args);
}

/*  --------------------------------------------------------------------
    psprintf = pinguino print formatted data to string
    --------------------------------------------------------------------
    out     : pointer on output string or function (if null)
    format  : pointer on string with % tags
    args    : list of variable arguments
    return  : string's length
    Note    : Pinguino Sprintf function
    ------------------------------------------------------------------*/

u8 psprintf(u8 *out, const u8 *format, ...)
{
    u8 r;
    va_list args;

    va_start(args, format);
    r = pprint(&out, format, args);
    va_end(args);// RB20140113
    return r;
}

#endif /* __PRINTF_C */
