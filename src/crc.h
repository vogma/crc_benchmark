#ifndef CRC_H
#define CRC_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t crc32_sse(uint32_t crc0, const uint8_t *data, size_t len);
uint32_t crc32_avx512_vpclmulqdq(uint32_t crc0, const uint8_t *data, size_t len);


#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* CRC_H */