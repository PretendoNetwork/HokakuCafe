#pragma once

#include <stdint.h>

typedef struct {
    void* vaddr;
    uint32_t len;
    uint32_t paddr;
} IOSVec_t;

#define IOS_HEAP_LOCAL  0xcafe
#define IOS_HEAP_SHARED 0xcaff

void* IOS_Alloc(uint32_t heap, uint32_t size);

void* IOS_AllocAligned(uint32_t heap, uint32_t size, uint32_t alignment);

void IOS_Free(uint32_t heap, void* ptr);

int IOS_Open(const char* device, int mode);

int IOS_Close(int fd);

int IOS_Ioctl(int fd, uint32_t request, void* input_buffer, uint32_t len_in, void* output_buffer, uint32_t len_out);

int IOS_Ioctlv(int fd, uint32_t request, uint32_t num_in, uint32_t num_out, IOSVec_t* vectors);
