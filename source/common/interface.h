#include <stdint.h>
#include <assert.h>

typedef struct InterfaceCtx InterfaceCtx_t;

typedef int (*IfaceCreateFn)(int idx);
typedef int (*IfaceSendFrameFn)(InterfaceCtx_t* ctx, void* buf, uint32_t size);

typedef struct __attribute__((packed)) InterfaceCtx {
    InterfaceCtx_t* next;
    char id[5];
    uint8_t unk0[0xf];
    IfaceCreateFn create;
    IfaceSendFrameFn sendFrame;
    uint8_t unk1[0x48];
    int32_t idx;
    const char* full_name;
    uint8_t unk2[0x58];
    uint32_t flags;
    uint8_t unk3[0x14];
} InterfaceCtx_t;

static_assert(sizeof(InterfaceCtx_t) == 0xe0, "InterfaceCtx_t: different size than expected");
