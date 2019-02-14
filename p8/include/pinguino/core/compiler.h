/*	----------------------------------------------------------------------------
	FILE:			compiler.h
	PROJECT:		pinguino
	PURPOSE:		compatibilty helper for SDCC and XC8
	PROGRAMER:		regis blanchot <rblanchot@gmail.com>
	FIRST RELEASE:	26 Jun. 2015
	LAST RELEASE:	26 Jun. 2015
	----------------------------------------------------------------------------
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
	--------------------------------------------------------------------------*/

#ifndef __COMPILER_H
#define __COMPILER_H

    #ifdef __XC8__

        #pragma warning disable 1496 // disable warnings when using va_arg and va_start arithmetic 

        #include <xc.h>

        #ifdef __18F13K50
            #define __18f13k50
        #endif
        #ifdef __18F14K50
            #define __18f14k50
        #endif
        #ifdef __18F2455
            #define __18f2455
        #endif
        #ifdef __18F4455
            #define __18f4455
        #endif
        #ifdef __18F25K22
            #define __18f25k22
        #endif
        #ifdef __18F2550
            #define __18f2550
        #endif
        #ifdef __18F4550
            #define __18f4550
        #endif
        #ifdef __18F25K50
            #define __18f25k50
        #endif
        #ifdef __18F45K50
            #define __18f45k50
        #endif
        #ifdef __18F26J50
            #define __18f26j50
        #endif
        #ifdef __18F46J50
            #define __18f46j50
        #endif
        #ifdef __18F26J53
            #define __18f26j53
        #endif
        #ifdef __18F46J53
            #define __18f46j53
        #endif
        #ifdef __18F27J53
            #define __18f27j53
        #endif
        #ifdef __18F47J53
            #define __18f47j53
        #endif

        #define __asm__(x) asm(x)

        // XC8 bug ? We must declare at least one variable in the section
        // or the linker complains
        #if !defined(__USB__) && !defined(__USBCDC__) && !defined(__USBBULK)
        volatile unsigned char __section("usbram5") __dummy__;
        #endif

    #else

        #include <pic18fregs.h>

    #endif


    #ifdef _16F1459
        #define __16f1459
        #define PROC_NAME      "16F1459"
    #endif
    #ifdef __18f13k50
        #define PROC_NAME      "18F13K50"
    #endif
    #ifdef __18f14k50
        #define PROC_NAME      "18F14K50"
    #endif
    #ifdef __18f2455
        #define PROC_NAME      "18F2455"
    #endif
    #ifdef __18f4455
        #define PROC_NAME      "18F4455"
    #endif
    #ifdef __18f25k22
        #define PROC_NAME      "18F25K22"
    #endif
    #ifdef __18f2550
        #define PROC_NAME      "18F2550"
    #endif
    #ifdef __18f4550
        #define PROC_NAME      "18F4550"
    #endif
    #ifdef __18f25k50
        #define PROC_NAME      "18F25K50"
    #endif
    #ifdef __18f45k50
        #define PROC_NAME      "18F45K50"
    #endif
    #ifdef __18f26j50
        #define PROC_NAME      "18F26J50"
    #endif
    #ifdef __18f46j50
        #define PROC_NAME      "18F46J50"
    #endif
    #ifdef __18f26j53
        #define PROC_NAME      "18F26J53"
    #endif
    #ifdef __18f46j53
        #define PROC_NAME      "18F46J53"
    #endif
    #ifdef __18f27j53
        #define PROC_NAME      "18F27J53"
    #endif
    #ifdef __18f47j53
        #define PROC_NAME      "18F47J53"
    #endif

#endif /* __COMPILER_H */
