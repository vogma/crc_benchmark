#ifndef CRC_H
#define CRC_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    uint32_t crc32_sse(uint32_t crc0, const uint8_t *data, size_t len);
    uint32_t crc32_avx512_vpclmulqdq(uint32_t crc0, const uint8_t *data, size_t len);
    uint32_t avx512_vpclmulqdq_crc32c_v3s2x4(uint32_t crc0, const uint8_t *data, size_t len);

    // crc used by gnulib
    uint32_t crc32_update_no_xor(uint32_t crc, const uint8_t *data, size_t len);

    // fastest tested in fast_crc32
    uint32_t avx512_vpclmulqdq_v3s2x4e(uint32_t crc0, const uint8_t *data, size_t len);

    // aarch64 function
    uint32_t crc32_chrome_scalar(uint32_t crc, const uint8_t *buf, size_t len);
    unsigned long crc32_chromium_scalar_opt(unsigned long crc, const unsigned char *buf, size_t len);
    uint32_t armv8_crc32_pmull_little(uint32_t crc, const unsigned char *buf, size_t len);
    uint32_t armv8_crc32_cloudfare_little(unsigned long crc, const unsigned char *buf, size_t len);
    uint32_t neon_eor3_crc32c_v9s3x2(uint32_t crc0, const uint8_t *data, size_t len);
    uint32_t neon_crc32c_v9s5x5e(uint32_t crc0, const uint8_t *data, size_t len);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* CRC_H */