/***********************************************************************
 * PIC32MX270F256B object file
 * Contains Memory Regions definitions
 * 2014-09-10 Régis Blanchot
 * more info :
 * https://sourceware.org/binutils/docs-2.23.1/ld/File-Commands.html#File-Commands
 **********************************************************************/

INPUT("processor.o")
OUTPUT_FORMAT("elf32-littlemips")
OUTPUT_ARCH(mips)
ENTRY(_reset)

/***********************************************************************
 * Base address of interrupt vector table (length = 0x1000)
 * _ebase_address value MUST BE the same as the ORIGIN value of exception_mem
 **********************************************************************/

PROVIDE(_vector_spacing = 0x00000001);
_ebase_address           = 0x9D000000;
_GEN_EXCPT_ADDR          = _ebase_address + 0x180;

/***********************************************************************
 * _RESET_ADDR value MUST BE same as the ORIGIN value of kseg1_boot_mem
 * When the PIC is reset, it goes to the reset address 0xBFC00000,
 * which is the location of the boot flash, and executes the code there:
 * crt0.S which takes care of some initialization tasks,
 * then calls the code for the bootloader,
 * which resides in the KSEG0 program flash (kseg0_program_mem) memory block.
 **********************************************************************/

_RESET_ADDR              = 0x9D003800;

/***********************************************************************
 * Memory Regions
 ***********************************************************************
 * - exception_mem  contains interrupt vector table starting at ebase (cacheable)
 * - kseg1_boot_mem contains reset vector, bootstrap exception handler, debug exception handler (non-cacheable)
 * - kseg0_boot_mem contains C startup module (cacheable)
 * - debug_exec_mem containing debug supporting code for running ICD3/RealICE during a debug session (non-cacheable)
 * - configuration words (non-cacheable)
 **********************************************************************/

MEMORY
{

exception_mem              : ORIGIN = 0x9D000000, LENGTH = 0x1000

/***********************************************************************
 * Reset (0x0), Boot (0x380) and Debug (0x480) vectors
 * kseg1_boot_mem value MUST BE same as the _RESET_ADDR value 
 * in non-cacheable BOOT FLASH
 * vector's length = 0x10 ?
 **********************************************************************/

kseg1_boot_mem             : ORIGIN = 0x9D003800, LENGTH = 0x10

/***********************************************************************
 * Startup code (about 0x200 bytes long)
 * in cacheable BOOT FLASH
 * only 0xBF0 bytes available on PIC32MX2xx
 **********************************************************************/

kseg0_boot_mem             : ORIGIN = 0x9D003810, LENGTH = 0x200

/***********************************************************************
 * User Program code
 * in cacheable PROGRAM FLASH
 * from 0x9D000000 to 0x9D040000 (PIC32MX270F256B = 256 KB flash = 0x40000) 
 * Interrupt Vector Table is placed between 0x9D000000 to 0x9D001000
 * Bootloader is placed between 0x9D001000 and 0x9D003400
 * 0x40000 - 0x3400 = 0x3CC00 bytes free
 **********************************************************************/

kseg0_program_mem    (rx)  : ORIGIN = 0x9D003A10, LENGTH = 0x3C5F0
  
/***********************************************************************
 * Device Configuration Registers (configuration bits)
 **********************************************************************/

config3                    : ORIGIN = 0xBFC00BF0, LENGTH = 0x4
config2                    : ORIGIN = 0xBFC00BF4, LENGTH = 0x4
config1                    : ORIGIN = 0xBFC00BF8, LENGTH = 0x4
config0                    : ORIGIN = 0xBFC00BFC, LENGTH = 0x4
configsfrs                 : ORIGIN = 0xBFC00BF0, LENGTH = 0x10

/***********************************************************************
 * all SFRS
 **********************************************************************/

sfrs                       : ORIGIN = 0xBF800000, LENGTH = 0x100000

/***********************************************************************
 * PIC32MX270F256B has 64 KB RAM, or 0x10000
 **********************************************************************/

kseg1_data_mem       (w!x) : ORIGIN = 0xA0000000, LENGTH = 64K

}
