OUTPUT_ARCH(arm)

INCLUDE "imports.ld"

SECTIONS {
    . = 0x12432000;

    .text : {
        _text_start = .;
        *(.text*);
        *(.rodata*);
        *(.data*);
    }
    _text_end = .;

    . = 0x1288e000;

    .bss : {
        *(.bss*);
    }

    /DISCARD/ : {
        *(*);
    }
}

ASSERT((0x12432000 + SIZEOF(.text)) < 0x12440000, "ios_net text is too big");
ASSERT((0x1288e000 + SIZEOF(.bss)) < 0x12ec0000, "ios_net bss is too big");
