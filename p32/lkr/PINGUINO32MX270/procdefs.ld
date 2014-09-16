/***********************************************************************
 * PIC32MX270F256B object file
 * Contains Memory Regions definitions
 * 2014-09-10 Régis Blanchot
 **********************************************************************/

INPUT("processor.o")

/***********************************************************************
 * For interrupt vector handling
 * base address of interrupt vector table
 * placed at the end of flash memory
 **********************************************************************/
 
PROVIDE(_vector_spacing = 0x00000001);

_ebase_address = 0x9D03F000;

/***********************************************************************
 * Memory Address Equates
 * _RESET_ADDR      -- Reset Vector
 * _BEV_EXCPT_ADDR  -- Boot exception Vector
 * _DBG_EXCPT_ADDR  -- In-circuit Debugging Exception Vector
 * _DBG_CODE_ADDR   -- In-circuit Debug Executive address
 * _DBG_CODE_SIZE   -- In-circuit Debug Executive size
 * _GEN_EXCPT_ADDR  -- General Exception Vector
 **********************************************************************/

/***********************************************************************
 * match reset address to ORIGIN of kseg1_boot_mem
 **********************************************************************/
 
_RESET_ADDR              = 0xBFC00000;

/***********************************************************************
 * map boot exception vector to kseg1_boot_mem
 **********************************************************************/

_BEV_EXCPT_ADDR          = 0xBFC00000 + 0x380;

/***********************************************************************
 * map _DBG_EXCPT_ADDR to kseg1_boot_mem
 * we don't use debug supporting code
 * _DBG_EXCPT_ADDR          = ;
 * _DBG_CODE_ADDR           = ;
 * _DBG_CODE_SIZE           = ;
 **********************************************************************/

/***********************************************************************
 * 
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
 ***********************************************************************
 * Cacheable (KSEG0) and non-cacheable (KSEG1)
 * KSEG0 PROGRAM FLASH [0x9D000000:0x9D03FFFF]
 * KSEG1 PROGRAM FLASH [0xBD000000:0xBD03FFFF]
 * KSEG0    BOOT FLASH [0x9FC00000:0x9FC00BEF]
 * KSEG1    BOOT FLASH [0xBFC00000:0xBFC00BEF]
 * KSEG1           RAM [0xA0000000:0xA000FFFF]
 **********************************************************************/

MEMORY
{

/***********************************************************************
 * interrupt vector table in PROGRAM FLASH starting at ebase
 **********************************************************************/

exception_mem              : ORIGIN = 0x9D03F000, LENGTH = 0x1000

/***********************************************************************
 * Startup code in BOOT FLASH
 * only 0xBEF bytes available on PIC32MX2xx
 * 0xBEF - 0x390 (Reset & Boot vectors) = 0x860
 **********************************************************************/

kseg0_boot_mem             : ORIGIN = 0x9FC00390, LENGTH = 0x860

/***********************************************************************
 * Reset vector (0x0) and Boot exception vector (0x380) in BOOT FLASH
 * vector's length = 0x10 ?
 **********************************************************************/

kseg1_boot_mem             : ORIGIN = 0xBFC00000, LENGTH = 0x390

/***********************************************************************
 * Bootloader is in PROGRAM FLASH
 * Bootloader length < 0x5000 bytes
 * PIC32MX250F128B = 256 KB flash = 0x40000
 **********************************************************************/

kseg0_program_mem    (rx)  : ORIGIN = 0x9D005000, LENGTH = 0x3EB00
  
/***********************************************************************
 * we don't use debug supporting code
 * debug_exec_mem             : ORIGIN = 0x9FC00490, LENGTH = 0x760
 **********************************************************************/

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
