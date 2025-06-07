#include <stddef.h>
#include <stdint.h>

#include <arm_neon.h>
#include <arm_acle.h>


uint32_t crc32_chrome_scalar(uint32_t crc, const uint8_t *buf, size_t len) {
    crc = ~crc;

    while (len >= 8) {
        crc = __crc32d(crc, *(uint64_t*)buf);
        len -= 8;
        buf += 8;
    }

    if (len & 4) {
        crc = __crc32w(crc, *(uint32_t*)buf);
        buf += 4;
    }
    if (len & 2) {
        crc = __crc32h(crc, *(uint16_t*)buf);
        buf += 2;
    }
    if (len & 1) {
        crc = __crc32b(crc, *buf);
    }

    return ~crc;
}

