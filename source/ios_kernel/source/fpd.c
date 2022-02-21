#include "net.h"
#include "imports.h"
#include "../../ios_fpd/ios_fpd_syms.h"

void run_ios_fpd_patches(void)
{
    // map memory for the custom ios code
    // custom text
    ios_map_shared_info_t map_info;
    map_info.paddr = 0xe316c000 - 0xe3000000 + 0x13640000;
    map_info.vaddr = 0xe316c000;
    map_info.size = 0x6000;
    map_info.domain = 12;            // FPD
    map_info.type = 3;              // 0 = undefined, 1 = kernel only, 2 = read only, 3 = read/write
    map_info.cached = 0xFFFFFFFF;
    _iosMapSharedUserExecution(&map_info);

    // custom bss
    map_info.paddr = 0xe32fd000 - 0xe31af000 + 0x137ef000;
    map_info.vaddr = 0xe32fd000;
    map_info.size = 0x6000;
    map_info.domain = 12;            // FPD
    map_info.type = 3;              // 0 = undefined, 1 = kernel only, 2 = read only, 3 = read write
    map_info.cached = 0xFFFFFFFF;
    _iosMapSharedUserExecution(&map_info);

    // nex token acquire hook
    *(volatile uint32_t*) (0xe30b3b28 - 0xe3000000 + 0x13640000) = ARM_BL(0xe30b3b28, acquireNexServiceTokenHook);
}
