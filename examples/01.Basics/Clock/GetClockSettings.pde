/*-----------------------------------------------------
  Author: --<Josep M. Mirats Tur>
  Project: GetClockSettings
  Date: 2017-07-26 12:47:05
  Description: Get Clock Settings for PIC32-PINGUINO-OTG Board
-----------------------------------------------------*/

// Define variables
u8 myFNOSC;
u8 myPLLIDIV;
u8 myPLLODIV;
u8 myPLLMULT;
u8 myPBDIV;
u8 test=0;

void setup()
{
  //Specific digital I/O pins configuration
  pinMode(USERLED, OUTPUT);

  myPLLIDIV = DEVCFG2bits.FPLLIDIV;
  myPLLODIV = OSCCONbits.PLLODIV;
  myPLLMULT = OSCCONbits.PLLMULT;
  myPBDIV   = OSCCONbits.PBDIV;
  myFNOSC   = DEVCFG1bits.FNOSC;
}

//Infinitely publish data each byte followed by a green led blinking
void loop()
{
  CDC.printf("test %u\r\n", test++);

  CDC.printf("FNOSC=%u ", myFNOSC);
  CDC.printf("PLLI=%u ",  myPLLIDIV);
  CDC.printf("PLLO=%u ",  myPLLODIV);
  CDC.printf("PLLM=%u ",  myPLLMULT);
  CDC.printf("PB=%u\r\n", myPBDIV);

  toggle(USERLED);
  delay(1000);
}
