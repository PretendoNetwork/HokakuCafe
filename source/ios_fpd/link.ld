OUTPUT_ARCH(arm)

INCLUDE "imports.ld"

SECTIONS {
    . = 0xe316c000;

    .text : {
        _text_start = .;
        *(.text*);
        *(.rodata*);
        *(.data*);
    }
    _text_end = .;

    . = 0xe32fd000;

    .bss : {
        *(.bss*);
    }

    /DISCARD/ : {
        *(*);
    }
}

ASSERT((0xe316c000 + SIZEOF(.text)) < 0xe3180000, "ios_net text is too big");
ASSERT((0xe32fd000 + SIZEOF(.bss)) < 0xe4000000, "ios_net bss is too big");
