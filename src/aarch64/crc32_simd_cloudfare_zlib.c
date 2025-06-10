// https://github.com/cloudflare/zlib/blob/gcc.amd64/crc32_simd.c

#include <stddef.h>
#include <stdint.h>
#include <arm_acle.h>
#include <arm_neon.h>

// #define __crc32b __builtin_arm_crc32b
// #define __crc32d __builtin_arm_crc32d
// #define __crc32w __builtin_arm_crc32w
// #define __crc32cw __builtin_arm_crc32cw

#if defined(__aarch64__)
#define TARGET_ARMV8_WITH_CRC __attribute__((target("+crc")))
#endif // defined(__aarch64__)

TARGET_ARMV8_WITH_CRC uint32_t armv8_crc32_cloudfare_little(unsigned long crc, const unsigned char *buf, size_t len)
{
    uint32_t c = (uint32_t)~crc;

    while (len && ((uintptr_t)buf & 7))
    {
        c = __crc32b(c, *buf++);
        --len;
    }

    const uint64_t *buf8 = (const uint64_t *)buf;

    while (len >= 64)
    {
        c = __crc32d(c, *buf8++);
        c = __crc32d(c, *buf8++);
        c = __crc32d(c, *buf8++);
        c = __crc32d(c, *buf8++);

        c = __crc32d(c, *buf8++);
        c = __crc32d(c, *buf8++);
        c = __crc32d(c, *buf8++);
        c = __crc32d(c, *buf8++);
        len -= 64;
    }

    while (len >= 8)
    {
        c = __crc32d(c, *buf8++);
        len -= 8;
    }

    buf = (const unsigned char *)buf8;

    while (len--)
    {
        c = __crc32b(c, *buf++);
    }

    return ~c;
}