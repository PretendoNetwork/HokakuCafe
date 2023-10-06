#include <stdio.h>
#include <string.h>

#include "ios.h"
#include "fsa.h"
#include "utils.h"
#include "../../common/interface.h"
#include "../../common/config.h"

#define ETHERTYPE_IPV4 0x0800
#define PROTO_TCP 0x06
#define PROTO_UDP 0x11

#define LINKTYPE_ETHERNET 1

typedef struct pcap_hdr_s {
    uint32_t magic_number;   /* magic number */
    uint16_t version_major;  /* major version number */
    uint16_t version_minor;  /* minor version number */
    int32_t  thiszone;       /* GMT to local correction */
    uint32_t sigfigs;        /* accuracy of timestamps */
    uint32_t snaplen;        /* max length of captured packets, in octets */
    uint32_t network;        /* data link type */
} pcap_hdr_t;

typedef struct pcaprec_hdr_s {
    uint32_t ts_sec;         /* timestamp seconds */
    uint32_t ts_usec;        /* timestamp microseconds */
    uint32_t incl_len;       /* number of octets of packet saved in file */
    uint32_t orig_len;       /* actual length of packet */
} pcaprec_hdr_t;

extern Configuration_t _configuration;

int semaphore = -1;
int fileHandle = -1;
void* writeBuffer = NULL;
int fsaHandle = -1;

int matchData(uint8_t *data)
{
    if (_configuration.mode == MODE_ALL) {
        return 1;
    }

    uint16_t ether_type = (data[12] << 8) | data[13];
    if (ether_type == ETHERTYPE_IPV4) {
        if (_configuration.mode == MODE_IPV4) {
            return 1;
        }

        if (data[23] == PROTO_TCP) {
            if (_configuration.mode == MODE_TCP) {
                return 1;
            }
        } else if (data[23] == PROTO_UDP) {
            if (_configuration.mode == MODE_UDP) {
                return 1;
            }

            if (_configuration.mode == MODE_PRUDP &&
                // PRUDPv1
                ((data[42] == 0xea && data[43] == 0xd0) ||
                // PRUDPv0 (Nintendo)
                (data[42] == 0xaf && data[43] == 0xa1))) {
                return 1;
            }
        }
    }

    return 0;
}

void writeData(void* data, uint32_t size)
{
    if (semaphore < 0) {
        semaphore = IOS_CreateSemaphore(1, 1);
        if (semaphore < 0) {
            printf("Failed to create semaphore\n");
        }
    }

    IOS_WaitSemaphore(semaphore, 0);

    if (fsaHandle < 0) {
        fsaHandle = IOS_Open("/dev/fsa", 0);
        if (fsaHandle < 0) {
            printf("Failed to open FSA\n");
            IOS_SignalSemaphore(semaphore);
            return;
        }
    }

    if (!writeBuffer) {
        writeBuffer = IOS_AllocAligned(0xcaff, _configuration.maxPacketSize + sizeof(pcaprec_hdr_t), 0x40);
        if (!writeBuffer) {
            printf("Failed to allocate write buffer\n");
            IOS_SignalSemaphore(semaphore);
            return;
        }
    }

    if (fileHandle < 0) {
        int res = FSA_Mount(fsaHandle, "/dev/sdcard01", "/vol/storage_framedump", 2, NULL, 0);
        if (res >= 0) {
            FSA_MakeDir(fsaHandle, "/vol/storage_framedump/HokakuCafe", 0x600);

            CalendarTime_t ctime;
            IOS_GetAbsTimeCalendar(&ctime);

            char name[512];
            snprintf(name, sizeof(name), "/vol/storage_framedump/HokakuCafe/%04ld-%02ld-%02ld_%02ld-%02ld-%02ld.pcap",
                ctime.year, ctime.month, ctime.day, ctime.hour, ctime.minute, ctime.second);

            res = FSA_OpenFile(fsaHandle, name, "w", &fileHandle);
            if (res < 0) {
                printf("Failed to open file\n");
                FSA_Unmount(fsaHandle, "/vol/storage_framedump", 2);
                fileHandle = -1;
                IOS_SignalSemaphore(semaphore);
                return;
            }

            pcap_hdr_t hdr = { 0 };
            hdr.magic_number = 0xa1b2c3d4;
            hdr.version_major = 2;
            hdr.version_minor = 4;
            hdr.snaplen = _configuration.maxPacketSize;
            hdr.network = LINKTYPE_ETHERNET;
            memcpy(writeBuffer, &hdr, sizeof(hdr));
            
            FSA_WriteFile(fsaHandle, writeBuffer, 1, sizeof(hdr), fileHandle, 0);
            FSA_FlushFile(fsaHandle, fileHandle);
        }
        else {
            printf("Failed to mount sdcard\n");
            IOS_SignalSemaphore(semaphore);
            return;
        }
    }

    uint64_t time;
    IOS_GetAbsTime64(&time);

    pcaprec_hdr_t rec_hdr = { 0 };
    rec_hdr.ts_sec = time / 1000000;
    rec_hdr.ts_usec = time - (rec_hdr.ts_sec * 1000000);
    rec_hdr.orig_len = size;
    rec_hdr.incl_len = (size >= _configuration.maxPacketSize) ? _configuration.maxPacketSize : size;

    memcpy(writeBuffer, &rec_hdr, sizeof(rec_hdr));
    memcpy((uint8_t*) writeBuffer + sizeof(rec_hdr), data, rec_hdr.incl_len);

    if (FSA_WriteFile(fsaHandle, writeBuffer, 1, sizeof(rec_hdr) + rec_hdr.incl_len, fileHandle, 0) < 0) {
        FSA_CloseFile(fsaHandle, fileHandle);
        fileHandle = -1;
    } else {
        FSA_FlushFile(fsaHandle, fileHandle);
    }

    IOS_SignalSemaphore(semaphore);
}

void netSuspendHook(void)
{
    IOS_Free(IOS_HEAP_SHARED, writeBuffer);
    writeBuffer = NULL;

    if (fsaHandle < 0) {
        return;
    }

    FSA_CloseFile(fsaHandle, fileHandle);
    IOS_Close(fsaHandle);

    fileHandle = -1;
    fsaHandle = -1;
}

typedef struct {
    uint32_t free;
    uint8_t* data;
    uint32_t max_data_size;
    uint32_t unk0;
    uint32_t packet_size;
    uint32_t unk1;
    InterfaceCtx_t* iface;
} Packet_t;

void queuePush(void* queue, Packet_t* packet);

void queuePushHook(void* queue, Packet_t* packet)
{
    uint8_t* data = packet->data + 2;
    // packet_size doesn't include the size of the frame header
    uint32_t size = packet->packet_size + 0xe;

    if (matchData(data)) {
        writeData(data, size);
    }

    queuePush(queue, packet);
}

int __UsbEthSendFrame(InterfaceCtx_t* ctx, uint8_t* data, uint32_t size);
int __WlSendFrame(InterfaceCtx_t* ctx, uint8_t* data, uint32_t size);

int sendFrameHook(InterfaceCtx_t* ctx, uint8_t* data, uint32_t size)
{
    // Only write packets which match to the pcap file
    if (matchData(data)) {
        writeData(data, size);
    }

    if (ctx->id[0] == 'U' && ctx->id[1] == 'S' && ctx->id[2] == 'B') {
        return __UsbEthSendFrame(ctx, data, size);
    }
    else if (ctx->id[0] == 'w' && ctx->id[1] == 'l') {
        return __WlSendFrame(ctx, data, size);
    }

    printf("sendFrameHook: unknown interface %s\n", ctx->id);
    return 0;
}
