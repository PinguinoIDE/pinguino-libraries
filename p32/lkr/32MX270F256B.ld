/***********************************************************************
 * PIC32MX270F256B object file
 * Contains Memory Regions definitions
 * 2014-09-10 Régis Blanchot
 * more info :
 * https://sourceware.org/binutils/docs-2.23.1/ld/File-Commands.html#File-Commands
 **********************************************************************/

/*
INPUT("processor.o")
OUTPUT_FORMAT("elf32-littlemips")
OUTPUT_ARCH(mips)
ENTRY(_reset)
*/
PROVIDE(_vector_spacing = 0x00000001);

/* not PIC32 specific, so define them here */
BMXDUPBA = 0xBF882030;
BMXDKPBA = 0xBF882010;
BMXDUDBA = 0xBF882020;

/***********************************************************************
 * Base address of interrupt vector table (length = 0x1000)
 * _ebase_address value MUST BE the same as the ORIGIN value of exception_mem
 * _RESET_ADDR value MUST BE same as the ORIGIN value of kseg1_boot_mem
 * When the PIC is reset, it goes to the reset address 0xBFC00000,
 * which is the location of the boot flash, and executes the code there:
 * crt0.S which takes care of some initialization tasks,
 * then calls the code for the bootloader,
 * which resides in the KSEG0 program flash (kseg0_program_mem) memory block.
 **********************************************************************/

_ebase_address           = 0x9D000000;
_RESET_ADDR              = 0x9D004000;

/***********************************************************************
 * A Bootstrap exception is an exception which occurs while bootstrap code
 * is running (StatusBEV=1).
 * All Bootstrap exceptions are vectored to 0xBFC00380.
 * At this location, the Compiler places a branch instruction targeting
 * the function named ‘_bootstrap_exception_handler()’. See crt0.S
 **********************************************************************/

/* _BEV_EXCPT_ADDR          = _RESET_ADDR + 0x380; */

/***********************************************************************
 * A general exception is any non-interrupt exception that occurs during
 * program execution outside of bootstrap code (StatusBEV=0).
 * General exceptions are vectored to offset 0x180 from EBase.
 * At this location, the Compiler places a branch instruction targeting
 * the function named ‘_general_exception_context()’. See crt0.S
 * The provided implementation of this function saves context,
 * calls an application handler function, restores context and performs
 * a return from the exception instruction.
 **********************************************************************/

_GEN_EXCPT_ADDR          = _ebase_address + 0x180;

/***********************************************************************
 * Memory Regions
 ***********************************************************************
 * - exception_mem  contains interrupt vector table starting at ebase (cacheable)
 * - kseg1_boot_mem contains reset vector, bootstrap exception handler, debug exception handler (non-cacheable)
 * - kseg0_boot_mem contains C startup module (cacheable)
 * - debug_exec_mem containing debug supporting code for running ICD3/RealICE during a debug session (non-cacheable)
 * - configuration words (non-cacheable)
 * - length = 0x200 + 64 vectors * 32 bytes = 0xA00 < 0xBF0
**********************************************************************/

MEMORY
{

/**********************************************************************
 * exception_mem  contains INTERRUPT VECTOR TABLE starting at ebase (cacheable)
 * exception_mem value MUST BE same as value of _ebase_address
 * exception_mem must align with 4K address boundary
 * SPACE SHARED BETWEEN THE BOOTLOADER AND THE PINGUINO APPLICATION
 * 0x200 + 64 vectors * 32 bytes = 0xA00
 **********************************************************************/

exception_mem              : ORIGIN = 0x9D000000, LENGTH = 0x1000

/**********************************************************************
 * kseg1_boot_mem (non-cacheable) contains
 * - Reset vector (0x000)
 * - Bootstrap exception handler (0x380)
 * - Debug exception handler (0x480)
 **********************************************************************/

kseg1_boot_mem             : ORIGIN = 0x9D004000, LENGTH = 0x10

/**********************************************************************
 * kseg0_boot_mem (cacheable) contains C startup module (cacheable)
 * Startup code is 0x1B0 bytes long
 **********************************************************************/

kseg0_boot_mem             : ORIGIN = 0x9D004010, LENGTH = 0x200

/***********************************************************************
 * User Program code
 * in cacheable PROGRAM FLASH
 * from 0x9D000000 to 0x9D040000 (PIC32MX270F256B = 256 KB flash = 0x40000) 
 * Bootloader Interrupt Vector Table is placed between 0x9D000000 to 0x9D001000
 * Bootloader Code is placed between 0x9D001000 and 0x9D004000
 * 0x40000 − 0x4000 = 0x3C000 bytes free
 **********************************************************************/

kseg0_program_mem    (rx)  : ORIGIN = 0x9D005000, LENGTH = 0x3B000
  
/***********************************************************************
 * Device Configuration Registers (configuration bits)
 **********************************************************************/

configsfrs                 : ORIGIN = 0xBFC00BF0, LENGTH = 0x10
config3                    : ORIGIN = 0xBFC00BF0, LENGTH = 0x4
config2                    : ORIGIN = 0xBFC00BF4, LENGTH = 0x4
config1                    : ORIGIN = 0xBFC00BF8, LENGTH = 0x4
config0                    : ORIGIN = 0xBFC00BFC, LENGTH = 0x4

/***********************************************************************
 * all SFRS
 **********************************************************************/

sfrs                       : ORIGIN = 0xBF800000, LENGTH = 0x100000

/***********************************************************************
 * PIC32MX270F256B has 64 KB RAM, or 0x10000
 **********************************************************************/

boot_software_key          : ORIGIN = 0xA0000000, LENGTH = 0x4
kseg1_data_mem       (w!x) : ORIGIN = 0xA0000004, LENGTH = 0xFFFC

}

SECTIONS
{
  /*********************************************************************
   *** Config. Sections
   ********************************************************************/

  .config_BFC00BF0 :
  {
    KEEP(*(.config_BFC00BF0))
  } > config3

  .config_BFC00BF4 :
  {
    KEEP(*(.config_BFC00BF4))
  } > config2
  
  .config_BFC00BF8 :
  {
    KEEP(*(.config_BFC00BF8))
  } > config1
  
  .config_BFC00BFC :
  {
    KEEP(*(.config_BFC00BFC))
  } > config0
}
