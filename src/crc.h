#ifndef CRC_H
#define CRC_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t crc32_sse(uint32_t crc0, const uint8_t *data, size_t len);
uint32_t crc32_avx512_vpclmulqdq(uint32_t crc0, const uint8_t *data, size_t len);
uint32_t avx512_vpclmulqdq_crc32c_v3s2x4(uint32_t crc0, const uint8_t *data, size_t len);

// crc used by gnulib
uint32_t crc32_update_no_xor(uint32_t crc, const uint8_t *data, size_t len);

// fastest tested in fast_crc32
uint32_t avx512_vpclmulqdq_v3s2x4e(uint32_t crc0, const uint8_t *data, size_t len);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* CRC_H */