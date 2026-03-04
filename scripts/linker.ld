/* linker.ld */
ENTRY(_start)

MEMORY
{
    /* Enforce exactly 16 MB for the ELF sections starting at 0x0 */
    EXEC_RAM (rwx) : ORIGIN = 0x00000000, LENGTH = 16M
}

/* Define segments with specific permissions */
/* FLAGS: 4=Read, 2=Write, 1=Exec */
PHDRS
{
    text PT_LOAD FLAGS(5); /* R + X (4 + 1) */
    data PT_LOAD FLAGS(6); /* R + W (4 + 2) */
}

SECTIONS
{
    /* Start at 0x00000000. */
    . = 0x00000000;

    /* CODE SEGMENT */
    .text : {
        *(.text)        
        *(.text.*)
    } > EXEC_RAM :text

    .rodata : {
        *(.rodata)
        *(.rodata.*)
        *(.srodata)    
        *(.srodata.*)
    } > EXEC_RAM :text

    /* DATA SEGMENT */
    .data : ALIGN(0x1000){
        *(.data)
        *(.data.*)
        *(.sdata)      
        *(.sdata.*)
    } > EXEC_RAM :data

    .bss : ALIGN(0x1000) {
        *(.bss)
        *(.bss.*)
        *(.sbss)     
        *(.sbss.*)
        *(COMMON)
    } > EXEC_RAM :data
}