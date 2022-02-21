#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include "ios.h"
#include "fsa.h"
#include "utils.h"

static int writeFileToSD(const char* path, void* data, uint32_t size)
{
    int fsaFd = IOS_Open("/dev/fsa", 0);
    if (fsaFd < 0) {
        return fsaFd;
    }

    int res = FSA_Mount(fsaFd, "/dev/sdcard01", "/vol/storage_tokendump", 2, NULL, 0);
    if (res >= 0) {
        FSA_MakeDir(fsaFd, "/vol/storage_tokendump/HokakuCafe", 0x600);

        int fileHandle;
        res = FSA_OpenFile(fsaFd, path, "w", &fileHandle);
        if (res >= 0) {
            FSA_WriteFile(fsaFd, data, 1, size, fileHandle, 0);
            FSA_CloseFile(fsaFd, fileHandle);
        }
        else {
            printf("Failed to create file 0x%x\n", res);
        }

        FSA_Unmount(fsaFd, "/vol/storage_tokendump", 2);
    }

    IOS_Close(fsaFd);

    return res;
}

typedef struct {
    void* unalignedBeforeBuffer;
    uint32_t unalignedBeforeSize;
    void* alignedBuffer;
    uint32_t alignedSize;
    void* unalignedAfterBuffer;
    uint32_t unalignedAfterSize;
} ManagedBuffer_t;

int acquireNexServiceToken(uint8_t param_1, ManagedBuffer_t* buffer, uint32_t gameId, uint8_t parentalControls);

int acquireNexServiceTokenHook(uint8_t param_1, ManagedBuffer_t* buffer, uint32_t gameId, uint8_t parentalControls)
{
    int result = acquireNexServiceToken(param_1, buffer, gameId, parentalControls);
    if (result == 0) {
        uint32_t size = buffer->unalignedBeforeSize + buffer->alignedSize + buffer->unalignedAfterSize;
        void* data = IOS_AllocAligned(IOS_HEAP_SHARED, size, 0x40);
        if (!data) {
            return result;
        }

        uint8_t* writePtr = (uint8_t*) data;
        memcpy(writePtr, buffer->unalignedBeforeBuffer, buffer->unalignedBeforeSize);
        writePtr += buffer->unalignedBeforeSize;

        memcpy(writePtr, buffer->alignedBuffer, buffer->alignedSize);
        writePtr += buffer->alignedSize;

        memcpy(writePtr, buffer->unalignedAfterBuffer, buffer->unalignedAfterSize);
        writePtr += buffer->unalignedAfterSize;

        char name[512];
        snprintf(name, sizeof(name), "/vol/storage_tokendump/HokakuCafe/nexServiceToken-%08lx.bin", gameId);

        writeFileToSD(name, data, size);

        IOS_Free(IOS_HEAP_SHARED, data);
    }

    return result;
}
