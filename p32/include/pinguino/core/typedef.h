/*	--------------------------------------------------------------------
    FILE:			typedef.h
    PROJECT:		Pinguino
    PURPOSE:		Pinguino types
    PROGRAMER:		Régis Blanchot
    --------------------------------------------------------------------
    CHANGELOG :
    --------------------------------------------------------------------
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
    ------------------------------------------------------------------*/

#ifndef __TYPEDEF_H
#define __TYPEDEF_H

/*  --------------------------------------------------------------------
    pinguino types
    ------------------------------------------------------------------*/

    typedef signed char				s8;
    typedef signed short int		s16;
    typedef signed long	int			s32;
    typedef signed long long 		s64;

    typedef unsigned char			u8;
    typedef unsigned short int		u16;
    typedef unsigned long int		u32;
    typedef unsigned long long 		u64;
    
    typedef union
    {
        u16 w;
        struct
        {
            u8 l8;
            u8 h8;
        };
    } t16;

    typedef union
    {
        u32 w;
        struct
        {
            u8 l;
            u8 h;
            u8 u;
        };
    } t24;

    typedef void (*funcout) (u8);   // type of void funcout(u8)

/*  --------------------------------------------------------------------
    gcc types
    ------------------------------------------------------------------*/

    //#include <types.h>

    typedef unsigned char			byte;
    typedef unsigned char			BYTE;

    typedef unsigned char			BOOL;//bool;					// not compatible with c++
    typedef unsigned char			boolean;	

    typedef signed char 			int8_t;
    typedef short int				int16_t;
    typedef long int				int32_t;
    typedef long long 				int64_t;

    typedef unsigned char 			uint8_t;
    typedef unsigned short int		uint16_t;
    typedef unsigned long int		uint32_t;
    typedef unsigned long long 		uint64_t;

    // 8 bits
    typedef unsigned char			uchar;
    typedef signed char				schar;
    typedef unsigned char			UCHAR;
    typedef signed char				CHAR;

    // 16 bits
    typedef short int				INT;
    typedef unsigned short int		UINT;
/*
 * 2012-04-28 Regis Blanchot - types.h: error: conflicting types
 */ 
//	typedef unsigned short int		uint;
    typedef signed short int		sint;
    typedef unsigned short int		word;
    typedef short int				SHORT;
    typedef unsigned short int		USHORT;
    typedef unsigned short int		WORD;
    typedef unsigned short int		WCHAR;

    // 32 bits
//	typedef unsigned long int		ulong;
    typedef unsigned long int		ULONG;
    typedef signed long int			slong;
    typedef unsigned long int		dword; 
    typedef unsigned long int		DWORD;
    typedef long int				LONG;

#endif	/* __TYPEDEF_H */
