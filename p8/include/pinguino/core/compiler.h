/*	----------------------------------------------------------------------------
	FILE:			compiler.h
	PROJECT:		pinguino
	PURPOSE:		compatibilty helper for SDC and XC8
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
        #if !defined(__USB__) && !defined(__USBCDC) && !defined(__USBBULK)
        volatile unsigned char __section("usbram5") __dummy__;
        #endif

    #else

        /*
        #if SDCC < 320
            #error "*******************************************"
            #error "*          Outdated SDCC version          *"
            #error "* try to update to version 3.2.0 or newer *"
            #error "*******************************************"
        #endif
        */
        
        #include <pic18fregs.h>

    #endif

#endif /* __COMPILER_H */
