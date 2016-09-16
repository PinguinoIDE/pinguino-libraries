TODO
==================

- [x] updating SD lib. to support all Pinguino 32MX2x0 boards.
- [x] updating SPI lib. to use SPI1, SPI2, SPI3 or SPI4 module for 32-bit.
- [x] modifying RTCC lib. to have the same syntax for both 8- and 32-bit version.
      either RTCC.getTime(&cT); or cT = RTCC.getTime();
- [x] renaming rtccTime type to RTCCTIME
- [ ] renaming C++ name.other to C name_other

WORK IN PROGRES
==================

- [ ] completing CTMU lib. for both 8- and 32-bit version.
- [x] replacing libcdc.a with a new CDC lib. to get more control over.

    - [x] CDC.begin(u32 baudrate);
    - [x] CDC Interrupt management (CDC.polling())
    - [x] non-blocking CDC when USB cable is unplugged
    
- [ ] adding scroll routines to ST7735 lib. 
- [x] Generating Bitnap fonts from TTF fonts to use with all LCD/TFT display libraries
      Tip : http://arduinoexplained.blogspot.nl/2012/05/generating-your-own-lcd-raster-fonts.html
      
DONE
==================

- [x] Pinguino 32MX250 Limited memory bug
      Fixed in the new bootloader
- [x] Added XC8 support
- [x] Added PIC16F1459 support

