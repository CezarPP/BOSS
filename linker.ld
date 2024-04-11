ENTRY(start)

SECTIONS
{
    /* The bootloader loads the kernel here */
    . = 1M; /* 0x100000 */
    .boot :
    {
        KEEP(*(.multiboot_header))
    }

    .text :
    {
        *(.text)
    }
}

