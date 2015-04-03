- [ ] Pinguino 32MX250 Limited memory bug

- [ ] RTCC lib. : to modify to get the same syntax for both 8- and 32-bit version.

8-bit (SDCC):

rtccTime cT;

RTCC.getTime(&cT);

32-bit (GCC) :

rtccTime cT;

cT = RTCC.getTime();

"SDCC" version is easy to implement under GCC but the use of pointers (&cT) is not "user-friendly".

rtccTime -> RTCCTIME ???

- [ ] CTMU lib. : to complete for both 8- and 32-bit version.

- [ ] SPI lib. : to modify to use SPI1, SPI2, SPI3 or SPI4 module for 32-bit.

 (on the same model of Serial or I2C lib.)

- [ ] CDC lib. : to replace call of libcdc.a and get more control over.

CDC.begin(u32 baudrate);
Interrupt management.
CDC.polling();

- [] ST7735 lib. : to add scroll routines
