#include "net.h"
#include "imports.h"
#include "../../common/interface.h"
#include "../../common/config.h"
#include "../../ios_net/ios_net_syms.h"

void run_ios_net_patches(void)
{
    // map memory for the custom ios-net code
    // custom ios-net text
    ios_map_shared_info_t map_info;
    map_info.paddr = 0x12432000;
    map_info.vaddr = 0x12432000;
    map_info.size = 0x6000;
    map_info.domain = 7;            // NET
    map_info.type = 3;              // 0 = undefined, 1 = kernel only, 2 = read only, 3 = read/write
    map_info.cached = 0xFFFFFFFF;
    _iosMapSharedUserExecution(&map_info);

    // custom ios-net bss
    map_info.paddr = 0x1288e000;
    map_info.vaddr = 0x1288e000;
    map_info.size = 0x6000;
    map_info.domain = 7;            // NET
    map_info.type = 3;              // 0 = undefined, 1 = kernel only, 2 = read only, 3 = read write
    map_info.cached = 0xFFFFFFFF;
    _iosMapSharedUserExecution(&map_info);

    // Copy configuration to the end of the text section
    Configuration_t *configuration = (Configuration_t *)0x00160000;
    kernel_memcpy((void*)_text_end, configuration, sizeof(*configuration));

    // replace all sendFrame functions
    InterfaceCtx_t** ifaces = (InterfaceCtx_t**) 0x128090bc;
    for (int i = 0; i < 3; i++) {
        if (ifaces[i]->sendFrame) {
            ifaces[i]->sendFrame = (IfaceSendFrameFn) sendFrameHook;
        }
    }

    // hook received usb eth frames
    *(volatile uint32_t*) 0x12308998 = ARM_BL(0x12308998, queuePushHook);

    // hook received wl frames
    *(volatile uint32_t*) 0x123087cc = ARM_BL(0x123087cc, queuePushHook);

    *(volatile uint32_t*) 0x1230130c = ARM_B(0x1230130c, netSuspendHook);
}
