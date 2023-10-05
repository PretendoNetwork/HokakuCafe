#include <stdint.h>

enum {
    MODE_ALL,
    MODE_IPV4,
    MODE_TCP,
    MODE_UDP,
    MODE_PRUDP,
};

typedef struct __attribute__((packed)) Configuration {
    uint32_t enabled;
    uint32_t mode;
    uint32_t maxPacketSize;
} Configuration_t;
