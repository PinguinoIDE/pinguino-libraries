/*  --------------------------------------------------------------------
    FILE:           audio.c
    PROJECT:        Pinguino
    PURPOSE:        Functions to play sounds
    PROGRAMER:      Régis Blanchot <rblanchot@gmail.com>
    --------------------------------------------------------------------
    CHANGELOG:
    *  5 Jan. 2010  Régis Blanchot - first release
    * 15 Feb. 2015  Régis Blanchot - added Sine Wave Generator
    * 16 Feb. 2015  Régis Blanchot - added Audio.init(SAMPLERATE)
    * 17 Feb. 2015  Régis Blanchot - added Direct Digital Synthesis
    * 17 Feb. 2015  Régis Blanchot - renamed library to audio.c
    * 17 Feb. 2015  Régis Blanchot - added Audio.staccato() and Audio.legato()
    * 24 Feb. 2015  Régis Blanchot - Pinguino 32 version
    TODO:
    * DTMF (dual-tone multi-frequency)
    * Wav decoder
    READINGS :
    * http://www.romanblack.com/one_sec.htm#BDA 
    * http://www.electricdruid.net/index.php?page=info.dds
    --------------------------------------------------------------------
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    ------------------------------------------------------------------*/

#ifndef __AUDIO_C
    #define __AUDIO_C
    #define __AUDIO__

    #include <typedef.h>
    #include <pin.h>            // CCPx pin definitions
    #include <audio.h>
    #ifndef __PIC32MX__
    #include <delayms.c>
    #include <digitalp.c>
    #include <oscillator.c>     // System_getPeripheralFrequency
    #else
    #include <p32xxxx.h>        // PIC32 registers
    #include <delay.c>
    #include <digitalw.c>
    #include <system.c>         // getPeripheralClock
    #include <interrupt.c>      // interrupts routines
    #endif
    
    // PWM mode
    #define PWMMODE         0b00001100

    // PWM registers pointers
    #ifndef __PIC32MX__
    volatile u8 *pCCPxCON;               // CCPxCON
    volatile u8 *pCCPRxL;                // CCPRxL
    #else
    volatile u16* pOCxCON;
    volatile u16* pOCxR;
    volatile u16* pOCxRS;
    #endif

    // Global variables
    volatile u16 gPeriodPlus1;  // u32 ?
    volatile u16 gPhase = 0;    // 16-bit accumulator
    volatile u16 gFreq1Inc = 0;
    volatile u16 gFreq2Inc = 0;
             u16 gSampleRate;  // u32 ?
             u8  gStaccato = true;

    // Waveform table
    const u8 sine64[64] = {
        50,54,59,64,68,73,77,81,85,88,91,93,95,97,98,99,99,99,98,97,95,
        93,91,88,85,81,77,73,68,64,59,54,50,45,40,35,31,26,22,18,14,11,
        8,6,4,2,1,0,0,0,1,2,4,6,8,11,14,18,22,26,31,35,40,45 };

    // Prototypes
    void Audio_init(u16 samplerate);
    void Audio_tone(u8 pin, u16 freq, u16 duration);
    void Audio_DTMF(u8 pin, u16 freq1, u16 freq2, u16 duration);
    void Audio_noTone(u8 pin);

/*  --------------------------------------------------------------------
    Audio_init
    --------------------------------------------------------------------
    @descr :    Generate a sample PWM period signal
                Enable TIMER2 interrupt
    @param :    Samplerate frequency in Hertz
    @note  :    This function computes the best values for the 
                TIMER2 prescaler and PR2 register depending on
                current Peripheral Frequency.
    @return:    none
    @usage :    Audio.init(CDQUALITY);
    ------------------------------------------------------------------*/

    void Audio_init(u16 samplerate)
    {
        u8 prescaler;

        gSampleRate = samplerate;

        // TIMER2 period (PR2+1) calculation
        // Timer2 clock input is the peripheral clock (FOSC/4). 
        #ifndef __PIC32MX__
        gPeriodPlus1 = System_getPeripheralFrequency() / samplerate;
        #else
        gPeriodPlus1 = GetPeripheralClock() / samplerate;
        #endif

        // stops interrupt
        noInterrupts();

        // configures Timer2 interrut
        #ifndef __16F1459
        IPR1bits.TMR2IP = 1;         // interrupt has high priority
        #endif
        PIR1bits.TMR2IF = 0;         // reset interrupt flag

        // Timer2 prescaler calculation
        // PR2 max value is 255, so PR2+1 max value is 256
        // only 3 possible prescaler value : 1, 4 or 16
        // so gPeriodPlus1 can not be > to 16 * 256 = 4096
        // and frequency can not be < 2929Hz (12MHZ/4096)
        
        if (gPeriodPlus1 <= 4096)          // check if it's not too high
        {
            if (gPeriodPlus1 <= 256)       // no needs of any prescaler
            {
                prescaler = 0;             // prescaler is 1, Timer2 On
            }
            else if (gPeriodPlus1 <= 1024) // needs prescaler 1:4
            {
                gPeriodPlus1 = gPeriodPlus1 >> 2;// divided by 4
                prescaler = 1;            // prescaler is 4, Timer2 On
            }
            else                          // needs prescaler 1:6
            {
                gPeriodPlus1 = gPeriodPlus1 >> 4;// divided by 16
                prescaler = 2;           // prescaler is 16, Timer2 On
            }
        }

        TMR2  = 0;
        PR2   = gPeriodPlus1 - 1;
        #ifdef __XC8__
        T2CON = prescaler | _T2CON_TMR2ON_MASK;
        #else
        T2CON = prescaler | _TMR2ON;
        #endif
        
        // (re-)starts interrupt
        interrupts();
}

/*  --------------------------------------------------------------------
    staccato
    --------------------------------------------------------------------
    Separates note from the note that may follow by silence
    @param:             none
    ------------------------------------------------------------------*/

    #define Audio_staccato() { gStaccato = true; }
    
/*  --------------------------------------------------------------------
    legato
    --------------------------------------------------------------------
    Plays note with the shortest silence between notes
    @param:             none
    ------------------------------------------------------------------*/

/*  --------------------------------------------------------------------
    Audio_tone
    --------------------------------------------------------------------
    Play sound with a certain frequency for a certain duration
    @param pin:         pin number where buzzer or loudspeaker is connected
    @param freq:        note frequency
    @param duration:    Duration in ms
    @return:            none
    @usage:             Audio.tone(PWM4, 440, 100); // LA 440Hz for 100 ms
    
    Note : When the output compare module is enabled, the I/O pin direction is
    controlled by the compare module. The compare module returns the I/O pin
    control back to the appropriate pin LAT and TRIS control bits when it is
    disabled.
    ------------------------------------------------------------------*/

    void Audio_tone(u8 pin, u16 freq, u16 duration)
    {
        switch (pin)
        {
            ///*********************************************************
            #if defined(PIC32_PINGUINO_220)
            ///*********************************************************

            case  2: pOCxCON = (u16*)&OC3CON; pOCxR = (u16*)&OC3R; pOCxRS = (u16*)&OC3RS; break;
            case  3: pOCxCON = (u16*)&OC4CON; pOCxR = (u16*)&OC4R; pOCxRS = (u16*)&OC4RS; break;
            case 11: pOCxCON = (u16*)&OC2CON; pOCxR = (u16*)&OC2R; pOCxRS = (u16*)&OC2RS; break;
            case 12: pOCxCON = (u16*)&OC5CON; pOCxR = (u16*)&OC5R; pOCxRS = (u16*)&OC5RS; break;
            case 13: pOCxCON = (u16*)&OC1CON; pOCxR = (u16*)&OC1R; pOCxRS = (u16*)&OC1RS; break;

            ///*********************************************************
            #elif defined(PINGUINO32MX220) || \
                  defined(PINGUINO32MX250) || \
                  defined(PINGUINO32MX270)
            ///*********************************************************

            case  1: pOCxCON = (u16*)&OC3CON; pOCxR = (u16*)&OC3R; pOCxRS = (u16*)&OC3RS; break;
            case  2: pOCxCON = (u16*)&OC4CON; pOCxR = (u16*)&OC4R; pOCxRS = (u16*)&OC4RS; break;
            case  6: pOCxCON = (u16*)&OC2CON; pOCxR = (u16*)&OC2R; pOCxRS = (u16*)&OC2RS; break;
            case  7: pOCxCON = (u16*)&OC5CON; pOCxR = (u16*)&OC5R; pOCxRS = (u16*)&OC5RS; break;
            case  8: pOCxCON = (u16*)&OC1CON; pOCxR = (u16*)&OC1R; pOCxRS = (u16*)&OC1RS; break;

            ///*********************************************************
            #elif defined(PIC32_PINGUINO_MICRO)
            ///*********************************************************

            case 10: pOCxCON = (u16*)&OC2CON; pOCxR = (u16*)&OC2R; pOCxRS = (u16*)&OC2RS; break;
            case 11: pOCxCON = (u16*)&OC3CON; pOCxR = (u16*)&OC3R; pOCxRS = (u16*)&OC3RS; break;
            case 12: pOCxCON = (u16*)&OC4CON; pOCxR = (u16*)&OC4R; pOCxRS = (u16*)&OC4RS; break;
            case 13: pOCxCON = (u16*)&OC5CON; pOCxR = (u16*)&OC5R; pOCxRS = (u16*)&OC5RS; break;

            ///*********************************************************
            #elif defined(PIC32_PINGUINO)     || \
                  defined(PIC32_PINGUINO_OTG)
            ///*********************************************************

            case  2: pOCxCON = (u16*)&OC1CON; pOCxR = (u16*)&OC1R; pOCxRS = (u16*)&OC1RS; break;
            case  1: pOCxCON = (u16*)&OC4CON; pOCxR = (u16*)&OC4R; pOCxRS = (u16*)&OC4RS; break;
            case  0: pOCxCON = (u16*)&OC3CON; pOCxR = (u16*)&OC3R; pOCxRS = (u16*)&OC3RS; break;

            ///*********************************************************
            #elif defined(EMPEROR460) || \
                  defined(EMPEROR795)
            ///*********************************************************

            case  0:
            case 72: pOCxCON = (u16*)&OC1CON; pOCxR = (u16*)&OC1R; pOCxRS = (u16*)&OC1RS; break;
            case  1:
            case 69: pOCxCON = (u16*)&OC2CON; pOCxR = (u16*)&OC2R; pOCxRS = (u16*)&OC2RS; break;
            case  2:
            case 68: pOCxCON = (u16*)&OC3CON; pOCxR = (u16*)&OC3R; pOCxRS = (u16*)&OC3RS; break;
            case  3:
            case 67: pOCxCON = (u16*)&OC4CON; pOCxR = (u16*)&OC4R; pOCxRS = (u16*)&OC4RS; break;
            case  4:
            case 66: pOCxCON = (u16*)&OC5CON; pOCxR = (u16*)&OC5R; pOCxRS = (u16*)&OC5RS; break;

            ///*********************************************************
            #elif defined(UBW32_460) || \
                  defined(UBW32_795) || \
                  defined(PIC32_PINGUINO_T795)
            ///*********************************************************

            case  0:
            case 24:
            case 40: pOCxCON = (u16*)&OC1CON; pOCxR = (u16*)&OC1R; pOCxRS = (u16*)&OC1RS; break;
            case  1:
            case  9:
            case 43: pOCxCON = (u16*)&OC2CON; pOCxR = (u16*)&OC2R; pOCxRS = (u16*)&OC2RS; break;
            case  2:
            case  8:
            case 44: pOCxCON = (u16*)&OC3CON; pOCxR = (u16*)&OC3R; pOCxRS = (u16*)&OC3RS; break;
            case  3:
            case  7:
            case 45: pOCxCON = (u16*)&OC4CON; pOCxR = (u16*)&OC4R; pOCxRS = (u16*)&OC4RS; break;
            case  4:
            case 25:
            case 60: pOCxCON = (u16*)&OC5CON; pOCxR = (u16*)&OC5R; pOCxRS = (u16*)&OC5RS; break;

            ///*********************************************************
            #elif defined(__16F1459)
            ///*********************************************************
            
            case PWM1 : pCCPxCON = &PWM1CON;  pCCPRxL = &PWM1DCL;  break;
            case PWM2 : pCCPxCON = &PWM2CON;  pCCPRxL = &PWM2DCL;  break;

            ///*********************************************************
            #elif defined(__18f26j53) || defined(__18f46j53) || \
                  defined(__18f27j53) || defined(__18f47j53)
            ///*********************************************************

            case PWM1 : pCCPxCON = &CCP4CON;  pCCPRxL = &CCPR4L;  break;
            case PWM2 : pCCPxCON = &CCP5CON;  pCCPRxL = &CCPR5L;  break;
            case PWM3 : pCCPxCON = &CCP6CON;  pCCPRxL = &CCPR6L;  break;
            case PWM4 : pCCPxCON = &CCP7CON;  pCCPRxL = &CCPR7L;  break;
            case PWM5 : pCCPxCON = &CCP8CON;  pCCPRxL = &CCPR8L;  break;
            case PWM6 : pCCPxCON = &CCP9CON;  pCCPRxL = &CCPR9L;  break;
            case PWM7 : pCCPxCON = &CCP10CON; pCCPRxL = &CCPR10L; break;

            ///*********************************************************
            #else
            ///*********************************************************

            case PWM1 : pCCPxCON = &CCP1CON;  pCCPRxL = &CCPR1L;  break;
            case PWM2 : pCCPxCON = &CCP2CON;  pCCPRxL = &CCPR2L;  break;

            #endif
        }

        // 16-bit accumulator's increment value.
        // the accumulator will go back to zero after (gSampleRate/freq) ticks
        gPhase = 0;
        gFreq1Inc = 65536 * freq / gSampleRate;

        #ifndef __PIC32MX__
        *pCCPxCON = PWMMODE;
        pinmode(pin, OUTPUT);   // PWM pin as OUTPUT
        PIE1bits.TMR2IE = 1;    // enable interrupt
        #else
        // PWMx On, PWMx pin set automatically as OUTPUT 
        *pOCxCON = PWMMODE;
        #endif

        Delayms(duration);
        
        if (gStaccato)
            #ifndef __PIC32MX__
            *pCCPxCON = 0;      // staccato
            #else
            *pOCxCON = 0;      // staccato (PWMx Off)
            #endif
    }

/*  --------------------------------------------------------------------
    Audio_dualTone (DTMF - dual-tone multi-frequency)
    --------------------------------------------------------------------
    Play sound with a certain frequency for a certain duration
    @param pin:         pin number where buzzer or loudspeaker is connected
    @param freq1:       1st frequency
    @param freq2:       2nd frequency
    @param duration:    Duration in ms
    
    Note : When the output compare module is enabled, the I/O pin direction is
    controlled by the compare module. The compare module returns the I/O pin
    control back to the appropriate pin LAT and TRIS control bits when it is
    disabled.
    ------------------------------------------------------------------*/

    #ifdef AUDIODTMF

    void Audio_dualTone(u8 pin, u32 freq1, u32 freq2, u32 duration)
    {
        switch (pin)
        {
            ///*********************************************************
            #if defined(PIC32_PINGUINO_220)
            ///*********************************************************
            
            case  2: pOCxCON = (u16*)&OC3CON; pOCxR = (u16*)&OC3R; pOCxRS = (u16*)&OC3RS; break;
            case  3: pOCxCON = (u16*)&OC4CON; pOCxR = (u16*)&OC4R; pOCxRS = (u16*)&OC4RS; break;
            case 11: pOCxCON = (u16*)&OC2CON; pOCxR = (u16*)&OC2R; pOCxRS = (u16*)&OC2RS; break;
            case 12: pOCxCON = (u16*)&OC5CON; pOCxR = (u16*)&OC5R; pOCxRS = (u16*)&OC5RS; break;
            case 13: pOCxCON = (u16*)&OC1CON; pOCxR = (u16*)&OC1R; pOCxRS = (u16*)&OC1RS; break;

            ///*********************************************************
            #elif defined(PINGUINO32MX220) || \
                  defined(PINGUINO32MX250) || \
                  defined(PINGUINO32MX270)
            ///*********************************************************
        
            case  1: pOCxCON = (u16*)&OC3CON; pOCxR = (u16*)&OC3R; pOCxRS = (u16*)&OC3RS; break;
            case  2: pOCxCON = (u16*)&OC4CON; pOCxR = (u16*)&OC4R; pOCxRS = (u16*)&OC4RS; break;
            case  6: pOCxCON = (u16*)&OC2CON; pOCxR = (u16*)&OC2R; pOCxRS = (u16*)&OC2RS; break;
            case  7: pOCxCON = (u16*)&OC5CON; pOCxR = (u16*)&OC5R; pOCxRS = (u16*)&OC5RS; break;
            case  8: pOCxCON = (u16*)&OC1CON; pOCxR = (u16*)&OC1R; pOCxRS = (u16*)&OC1RS; break;
    
            ///*********************************************************
            #elif defined(PIC32_PINGUINO_MICRO)
            ///*********************************************************

            case 10: pOCxCON = (u16*)&OC2CON; pOCxR = (u16*)&OC2R; pOCxRS = (u16*)&OC2RS; break;
            case 11: pOCxCON = (u16*)&OC3CON; pOCxR = (u16*)&OC3R; pOCxRS = (u16*)&OC3RS; break;
            case 12: pOCxCON = (u16*)&OC4CON; pOCxR = (u16*)&OC4R; pOCxRS = (u16*)&OC4RS; break;
            case 13: pOCxCON = (u16*)&OC5CON; pOCxR = (u16*)&OC5R; pOCxRS = (u16*)&OC5RS; break;

            ///*********************************************************
            #elif defined(PIC32_PINGUINO) || \
                  defined(PIC32_PINGUINO_OTG)
            ///*********************************************************

            case  2: pOCxCON = (u16*)&OC1CON; pOCxR = (u16*)&OC1R; pOCxRS = (u16*)&OC1RS; break;
            case  1: pOCxCON = (u16*)&OC4CON; pOCxR = (u16*)&OC4R; pOCxRS = (u16*)&OC4RS; break;
            case  0: pOCxCON = (u16*)&OC3CON; pOCxR = (u16*)&OC3R; pOCxRS = (u16*)&OC3RS; break;

            ///*********************************************************
            #elif defined(EMPEROR460) || \
                  defined(EMPEROR795)
            ///*********************************************************

            case  0:
            case 72: pOCxCON = (u16*)&OC1CON; pOCxR = (u16*)&OC1R; pOCxRS = (u16*)&OC1RS; break;
            case  1:
            case 69: pOCxCON = (u16*)&OC2CON; pOCxR = (u16*)&OC2R; pOCxRS = (u16*)&OC2RS; break;
            case  2:
            case 68: pOCxCON = (u16*)&OC3CON; pOCxR = (u16*)&OC3R; pOCxRS = (u16*)&OC3RS; break;
            case  3:
            case 67: pOCxCON = (u16*)&OC4CON; pOCxR = (u16*)&OC4R; pOCxRS = (u16*)&OC4RS; break;
            case  4:
            case 66: pOCxCON = (u16*)&OC5CON; pOCxR = (u16*)&OC5R; pOCxRS = (u16*)&OC5RS; break;

            ///*********************************************************
            #elif defined(UBW32_460) || \
                  defined(UBW32_795) || \
                  defined(PIC32_PINGUINO_T795)
            ///*********************************************************

            case  0:
            case 24:
            case 40: pOCxCON = (u16*)&OC1CON; pOCxR = (u16*)&OC1R; pOCxRS = (u16*)&OC1RS; break;
            case  1:
            case  9:
            case 43: pOCxCON = (u16*)&OC2CON; pOCxR = (u16*)&OC2R; pOCxRS = (u16*)&OC2RS; break;
            case  2:
            case  8:
            case 44: pOCxCON = (u16*)&OC3CON; pOCxR = (u16*)&OC3R; pOCxRS = (u16*)&OC3RS; break;
            case  3:
            case  7:
            case 45: pOCxCON = (u16*)&OC4CON; pOCxR = (u16*)&OC4R; pOCxRS = (u16*)&OC4RS; break;
            case  4:
            case 25:
            case 60: pOCxCON = (u16*)&OC5CON; pOCxR = (u16*)&OC5R; pOCxRS = (u16*)&OC5RS; break;

            ///*********************************************************
            #elif defined(__16F1459)
            ///*********************************************************
            
            case PWM1 : pCCPxCON = &PWM1CON;  pCCPRxL = &PWM1DCL;  break;
            case PWM2 : pCCPxCON = &PWM2CON;  pCCPRxL = &PWM2DCL;  break;

            ///*********************************************************
            #elif defined(__18f26j53) || defined(__18f46j53) || \
                  defined(__18f27j53) || defined(__18f47j53)
            ///*********************************************************

            case PWM1 : pCCPxCON = &CCP4CON;  pCCPRxL = &CCPR4L;  break;
            case PWM2 : pCCPxCON = &CCP5CON;  pCCPRxL = &CCPR5L;  break;
            case PWM3 : pCCPxCON = &CCP6CON;  pCCPRxL = &CCPR6L;  break;
            case PWM4 : pCCPxCON = &CCP7CON;  pCCPRxL = &CCPR7L;  break;
            case PWM5 : pCCPxCON = &CCP8CON;  pCCPRxL = &CCPR8L;  break;
            case PWM6 : pCCPxCON = &CCP9CON;  pCCPRxL = &CCPR9L;  break;
            case PWM7 : pCCPxCON = &CCP10CON; pCCPRxL = &CCPR10L; break;

            ///*********************************************************
            #else
            ///*********************************************************

            case PWM1 : pCCPxCON = &CCP1CON;  pCCPRxL = &CCPR1L;  break;
            case PWM2 : pCCPxCON = &CCP2CON;  pCCPRxL = &CCPR2L;  break;

            #endif
        }

        // 16-bit accumulator's increment value.
        // the accumulator will go back to zero after (gSampleRate/freq) ticks
        gPhase = 0;
        // 16-bit accumulator's increment value = 65536 / (gSampleRate/freq)
        gFreq1Inc = 65536 * freq1 / gSampleRate;
        gFreq2Inc = 65536 * freq2 / gSampleRate;

        #ifndef __PIC32MX__
        *pCCPxCON = PWMMODE;
        pinmode(pin, OUTPUT);   // PWM pin as OUTPUT
        PIE1bits.TMR2IE = 1;    // enable interrupt
        #else
        // PWM On, PWM pin as OUTPUT
        //IntEnable(_TIMER_2_IRQ);  // Enable TIMER2 interrupt
        *pOCxCON = PWMMODE;
	#endif

        Delayms(duration);
        
        if (gStaccato)
            #ifndef __PIC32MX__
            *pCCPxCON = 0;      // staccato
            #else
            *pOCxCON = 0;      // staccato
	    #endif
    }

    #endif // AUDIODTMF

/*  --------------------------------------------------------------------
    Audio_noTone
    --------------------------------------------------------------------
    @param pin:         pin number where loudspeaker is connected
    
    Note : When the output compare module is enabled, the I/O pin direction is
    controlled by the compare module. The compare module returns the I/O pin
    control back to the appropriate pin LAT and TRIS control bits when it is
    disabled.
    ------------------------------------------------------------------*/

    void Audio_noTone(u8 pin)
    {
        // We don't stop TIMER2 interrupt here
        // because user can use more than one PWM at a time
        /*
	#ifndef __PIC32MX__
        pinmode(pin, INPUT);    // PWM pin as INPUT
        PIE1bits.TMR2IE = 0;    // disable interrupt 
        #else
        IntDisable(_TIMER_2_IRQ); // Disable TIMER2 interrupt
        #endif
        */
        TMR2  = 0;

        switch (pin)            // PWM mode disable
        {
            ///*********************************************************
            #if defined(PIC32_PINGUINO_220)
            ///*********************************************************
            
            case 2:  OC3CON = 0; break;
            case 3:  OC4CON = 0; break;
            case 11: OC2CON = 0; break;
            case 12: OC5CON = 0; break;
            case 13: OC1CON = 0; break;

            ///*********************************************************
            #elif defined(PINGUINO32MX220) || \
                  defined(PINGUINO32MX250) || \
                  defined(PINGUINO32MX270)
            ///*********************************************************
        
            case 1:  OC3CON = 0; break;
            case 2:  OC4CON = 0; break;
            case 6:  OC2CON = 0; break;
            case 7:  OC5CON = 0; break;
            case 8:  OC1CON = 0; break;
    
            ///*********************************************************
            #elif defined(PIC32_PINGUINO_MICRO)
            ///*********************************************************

            case 10: OC2CON = 0; break;
            case 11: OC3CON = 0; break;
            case 12: OC4CON = 0; break;
            case 13: OC5CON = 0; break;

            ///*********************************************************
            #elif defined(PIC32_PINGUINO) || \
                  defined(PIC32_PINGUINO_OTG)
            ///*********************************************************

            case 2:  OC1CON = 0; break;
            case 1:  OC4CON = 0; break;
            case 0:  OC3CON = 0; break;

            ///*********************************************************
            #elif defined(EMPEROR460) || \
                  defined(EMPEROR795)
            ///*********************************************************

            case  0:
            case 72: OC1CON = 0; break;
            case  1:
            case 69: OC2CON = 0; break;
            case  2:
            case 68: OC3CON = 0; break;
            case  3:
            case 67: OC4CON = 0; break;
            case  4:
            case 66: OC5CON = 0; break;

            ///*********************************************************
            #elif defined(UBW32_460) || \
                  defined(UBW32_795) || \
                  defined(PIC32_PINGUINO_T795)
            ///*********************************************************

            case  0:
            case 24:
            case 40: OC1CON = 0; break;
            case  1:
            case  9:
            case 43: OC2CON = 0; break;
            case  2:
            case  8:
            case 44: OC3CON = 0; break;
            case  3:
            case  7:
            case 45: OC4CON = 0; break;
            case  4:
            case 25:
            case 60: OC5CON = 0; break;

            ///*********************************************************
            #elif defined(__16F1459)
            ///*********************************************************
            
            case PWM1 : PWM1CON = 0; break;
            case PWM2 : PWM2CON = 0; break;

            ///*********************************************************
            #elif defined(__18f26j53) || defined(__18f46j53) || \
                  defined(__18f27j53) || defined(__18f47j53)
            ///*********************************************************

            case PWM1 : CCP4CON  = 0; break;
            case PWM2 : CCP5CON  = 0; break;
            case PWM3 : CCP6CON  = 0; break;
            case PWM4 : CCP7CON  = 0; break;
            case PWM5 : CCP8CON  = 0; break;
            case PWM6 : CCP9CON  = 0; break;
            case PWM7 : CCP10CON = 0; break;

            ///*********************************************************
            #else
            ///*********************************************************

            case PWM1 : CCP1CON  = 0; break;
            case PWM2 : CCP2CON  = 0; break;

            #endif
        }
    }

/*  --------------------------------------------------------------------
    Audio_staccato
    --------------------------------------------------------------------
    Separates note from the note that may follow by silence
    @param:             none
    ------------------------------------------------------------------*/

    #define Audio_staccato() { gStaccato = true; }
    
/*  --------------------------------------------------------------------
    Audio_legato
    --------------------------------------------------------------------
    Plays note with the shortest silence between notes
    @param:             none
    ------------------------------------------------------------------*/

    #define Audio_legato()  { gStaccato = false; }

/*  --------------------------------------------------------------------
    Interrupt Service Routine
    --------------------------------------------------------------------
    In PWM mode, the OCxR register is a read-only slave duty cycle
    register and OCxRS is a buffer register that is written by the user
    to update the PWM duty cycle.

    On every timer to period register match event (end of PWM period),
    the duty cycle register, OCxR, is loaded with the contents of OCxRS.

    The TyIF interrupt flag is asserted at each PWM period boundary.

    The new PWM value must be computed in less than the PWM cycle gPeriodPlus1.
    If sample rate = CDQUALITY    -> Tpwm =  22 us (1/44100 sec.)
    If sample rate = TAPEQUALITY  -> Tpwm =  45 us
    If sample rate = RADIOQUALITY -> Tpwm =  90 us
    If sample rate = TELQUALITY   -> Tpwm = 113 us

    To generate a single clean tone we load the duty cycle register
    according to a pure sinusoid
    ------------------------------------------------------------------*/

void pwm_interrupt()
{
    u16 duty;
    
    if (PIR1bits.TMR2IF)
    {
        // Clear interrupt flag
        PIR1bits.TMR2IF = 0;

        // Increment the 16-bit phase accumulator
        gPhase += gFreq1Inc;
        
        // The signal level must be offset so that the zero level
        // generates a PWM output with a 50% duty cycle
        //*pCCPRxL = sine64[ gPhase & 0x3F ];// * gPeriodPlus1 / 100;
        //duty = sine64[ gPhase & 0x3F ] * gPeriodPlus1 / 100;
        duty = sine64[ gPhase & 0x3F ] * 100;
        //duty = duty * 4;

        // Load the duty cycle register according to the sine table
        //PWM_setDutyCycle(gPin, duty);
        *pCCPRxL   = (duty >> 2) & 0xFF;          // 8 MSB
        #if defined(__16F1459)
        *pCCPxCON |= ((u8)duty & 0x03) << 6;      // 2 LSB in <7:6>
        #else
        *pCCPxCON |= ((u8)duty & 0x03) << 4;      // 2 LSB in <5:4>
        #endif
    }
}

#endif // __AUDIO_C
