/******************************************************************************
 * Linker command file for TM4C123GH6PM
 * Toolchain: TI ARM Compiler / CCS
 * MELK OS - Phase 1
 ******************************************************************************/

--retain=g_pfnVectors
--entry_point=Reset_Handler
--stack_size=0x400
--heap_size=0x000

MEMORY
{
	VECTORS (RX) : origin = 0x00000000, length = 0x00000400
    FLASH   (RX) : origin = 0x00000400, length = 0x0003FC00
    SRAM   (RWX) : origin = 0x20000000, length = 0x00008000
}

_estack = 0x20008000;

SECTIONS
{
    .intvecs :
    {
        *(.intvecs)
        *(.intvecs:*)
    } > VECTORS

    .text :
    {
        *(.text)
        *(.text:*)
    } > FLASH

    .const :
    {
        *(.const)
        *(.const:*)
    } > FLASH

    .cinit : > FLASH

    .data :
    {
        *(.data)
    } LOAD = FLASH,
      RUN = SRAM,
      LOAD_START(_sidata),
      RUN_START(_sdata),
      RUN_END(_edata)

    .bss :
    {
        *(.bss)
    } > SRAM,
      RUN_START(_sbss),
      RUN_END(_ebss)

    .stack :
    {
        *(.stack)
    } > SRAM

    .sysmem :
    {
        *(.sysmem)
    } > SRAM
}
