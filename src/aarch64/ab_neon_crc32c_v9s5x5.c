/* Generated by https://github.com/corsix/fast-crc32/ using: */
/* ./generate -i neon -p crc32c -a v9s5x5e */
/* MIT licensed */

#include <stddef.h>
#include <stdint.h>
#include <arm_acle.h>
#include <arm_neon.h>

#if defined(_MSC_VER)
#define CRC_AINLINE static __forceinline
#define CRC_ALIGN(n) __declspec(align(n))
#else
#define CRC_AINLINE static __inline __attribute__((always_inline))
#define CRC_ALIGN(n) __attribute__((aligned(n)))
#endif
#define CRC_EXPORT extern

CRC_AINLINE uint64x2_t clmul_lo_e(uint64x2_t a, uint64x2_t b, uint64x2_t c)
{
  uint64x2_t r;
  __asm("pmull %0.1q, %2.1d, %3.1d\neor %0.16b, %0.16b, %1.16b\n" : "=w"(r), "+w"(c) : "w"(a), "w"(b));
  return r;
}

CRC_AINLINE uint64x2_t clmul_hi_e(uint64x2_t a, uint64x2_t b, uint64x2_t c)
{
  uint64x2_t r;
  __asm("pmull2 %0.1q, %2.2d, %3.2d\neor %0.16b, %0.16b, %1.16b\n" : "=w"(r), "+w"(c) : "w"(a), "w"(b));
  return r;
}

CRC_AINLINE uint64x2_t clmul_scalar(uint32_t a, uint32_t b)
{
  uint64x2_t r;
  __asm("pmull %0.1q, %1.1d, %2.1d\n" : "=w"(r) : "w"(vmovq_n_u64(a)), "w"(vmovq_n_u64(b)));
  return r;
}

static uint32_t xnmodp(uint64_t n) /* x^n mod P, in log(n) time */
{
  uint64_t stack = ~(uint64_t)1;
  uint32_t acc, low;
  for (; n > 191; n = (n >> 1) - 16)
  {
    stack = (stack << 1) + (n & 1);
  }
  stack = ~stack;
  acc = ((uint32_t)0x80000000) >> (n & 31);
  for (n >>= 5; n; --n)
  {
    acc = __crc32cw(acc, 0);
  }
  while ((low = stack & 1), stack >>= 1)
  {
    poly8x8_t x = vreinterpret_p8_u64(vmov_n_u64(acc));
    uint64_t y = vgetq_lane_u64(vreinterpretq_u64_p16(vmull_p8(x, x)), 0);
    acc = __crc32cd(0, y << low);
  }
  return acc;
}

CRC_AINLINE uint64x2_t crc_shift(uint32_t crc, size_t nbytes)
{
  return clmul_scalar(crc, xnmodp(nbytes * 8 - 33));
}

uint32_t neon_crc32c_v9s5x5e(uint32_t crc0, const uint8_t *data, size_t len)
{
  const char *buf = (const char *)data;

  crc0 = ~crc0;
  for (; len && ((uintptr_t)buf & 7); --len)
  {
    crc0 = __crc32cb(crc0, *buf++);
  }
  if (((uintptr_t)buf & 8) && len >= 8)
  {
    crc0 = __crc32cd(crc0, *(const uint64_t *)buf);
    buf += 8;
    len -= 8;
  }
  if (len >= 352)
  {
    const char *end = buf + len;
    size_t blk = (len - 8) / 344;
    size_t klen = blk * 40;
    const char *buf2 = buf + 0;
    const char *limit = buf + blk * 144 + klen - 80;
    uint32_t crc1 = 0;
    uint32_t crc2 = 0;
    uint32_t crc3 = 0;
    uint32_t crc4 = 0;
    uint64x2_t vc0;
    uint64x2_t vc1;
    uint64x2_t vc2;
    uint64x2_t vc3;
    uint64_t vc;
    /* First vector chunk. */
    uint64x2_t x0 = vld1q_u64((const uint64_t *)buf2), y0;
    uint64x2_t x1 = vld1q_u64((const uint64_t *)(buf2 + 16)), y1;
    uint64x2_t x2 = vld1q_u64((const uint64_t *)(buf2 + 32)), y2;
    uint64x2_t x3 = vld1q_u64((const uint64_t *)(buf2 + 48)), y3;
    uint64x2_t x4 = vld1q_u64((const uint64_t *)(buf2 + 64)), y4;
    uint64x2_t x5 = vld1q_u64((const uint64_t *)(buf2 + 80)), y5;
    uint64x2_t x6 = vld1q_u64((const uint64_t *)(buf2 + 96)), y6;
    uint64x2_t x7 = vld1q_u64((const uint64_t *)(buf2 + 112)), y7;
    uint64x2_t x8 = vld1q_u64((const uint64_t *)(buf2 + 128)), y8;
    uint64x2_t k;
    {
      static const uint64_t CRC_ALIGN(16) k_[] = {0x7e908048, 0xc96cfdc0};
      k = vld1q_u64(k_);
    }
    x0 = veorq_u64((uint64x2_t){crc0, 0}, x0);
    crc0 = 0;
    buf2 += 144;
    buf += blk * 144;
    /* Main loop. */
    while (buf <= limit)
    {
      y0 = clmul_lo_e(x0, k, vld1q_u64((const uint64_t *)buf2)), x0 = clmul_hi_e(x0, k, y0);
      y1 = clmul_lo_e(x1, k, vld1q_u64((const uint64_t *)(buf2 + 16))), x1 = clmul_hi_e(x1, k, y1);
      y2 = clmul_lo_e(x2, k, vld1q_u64((const uint64_t *)(buf2 + 32))), x2 = clmul_hi_e(x2, k, y2);
      y3 = clmul_lo_e(x3, k, vld1q_u64((const uint64_t *)(buf2 + 48))), x3 = clmul_hi_e(x3, k, y3);
      y4 = clmul_lo_e(x4, k, vld1q_u64((const uint64_t *)(buf2 + 64))), x4 = clmul_hi_e(x4, k, y4);
      y5 = clmul_lo_e(x5, k, vld1q_u64((const uint64_t *)(buf2 + 80))), x5 = clmul_hi_e(x5, k, y5);
      y6 = clmul_lo_e(x6, k, vld1q_u64((const uint64_t *)(buf2 + 96))), x6 = clmul_hi_e(x6, k, y6);
      y7 = clmul_lo_e(x7, k, vld1q_u64((const uint64_t *)(buf2 + 112))), x7 = clmul_hi_e(x7, k, y7);
      y8 = clmul_lo_e(x8, k, vld1q_u64((const uint64_t *)(buf2 + 128))), x8 = clmul_hi_e(x8, k, y8);
      crc0 = __crc32cd(crc0, *(const uint64_t *)buf);
      crc1 = __crc32cd(crc1, *(const uint64_t *)(buf + klen));
      crc2 = __crc32cd(crc2, *(const uint64_t *)(buf + klen * 2));
      crc3 = __crc32cd(crc3, *(const uint64_t *)(buf + klen * 3));
      crc4 = __crc32cd(crc4, *(const uint64_t *)(buf + klen * 4));
      crc0 = __crc32cd(crc0, *(const uint64_t *)(buf + 8));
      crc1 = __crc32cd(crc1, *(const uint64_t *)(buf + klen + 8));
      crc2 = __crc32cd(crc2, *(const uint64_t *)(buf + klen * 2 + 8));
      crc3 = __crc32cd(crc3, *(const uint64_t *)(buf + klen * 3 + 8));
      crc4 = __crc32cd(crc4, *(const uint64_t *)(buf + klen * 4 + 8));
      crc0 = __crc32cd(crc0, *(const uint64_t *)(buf + 16));
      crc1 = __crc32cd(crc1, *(const uint64_t *)(buf + klen + 16));
      crc2 = __crc32cd(crc2, *(const uint64_t *)(buf + klen * 2 + 16));
      crc3 = __crc32cd(crc3, *(const uint64_t *)(buf + klen * 3 + 16));
      crc4 = __crc32cd(crc4, *(const uint64_t *)(buf + klen * 4 + 16));
      crc0 = __crc32cd(crc0, *(const uint64_t *)(buf + 24));
      crc1 = __crc32cd(crc1, *(const uint64_t *)(buf + klen + 24));
      crc2 = __crc32cd(crc2, *(const uint64_t *)(buf + klen * 2 + 24));
      crc3 = __crc32cd(crc3, *(const uint64_t *)(buf + klen * 3 + 24));
      crc4 = __crc32cd(crc4, *(const uint64_t *)(buf + klen * 4 + 24));
      crc0 = __crc32cd(crc0, *(const uint64_t *)(buf + 32));
      crc1 = __crc32cd(crc1, *(const uint64_t *)(buf + klen + 32));
      crc2 = __crc32cd(crc2, *(const uint64_t *)(buf + klen * 2 + 32));
      crc3 = __crc32cd(crc3, *(const uint64_t *)(buf + klen * 3 + 32));
      crc4 = __crc32cd(crc4, *(const uint64_t *)(buf + klen * 4 + 32));
      buf += 40;
      buf2 += 144;
    }
    /* Reduce x0 ... x8 to just x0. */
    {
      static const uint64_t CRC_ALIGN(16) k_[] = {0xf20c0dfe, 0x493c7d27};
      k = vld1q_u64(k_);
    }
    y0 = clmul_lo_e(x0, k, x1), x0 = clmul_hi_e(x0, k, y0);
    x1 = x2, x2 = x3, x3 = x4, x4 = x5, x5 = x6, x6 = x7, x7 = x8;
    y0 = clmul_lo_e(x0, k, x1), x0 = clmul_hi_e(x0, k, y0);
    y2 = clmul_lo_e(x2, k, x3), x2 = clmul_hi_e(x2, k, y2);
    y4 = clmul_lo_e(x4, k, x5), x4 = clmul_hi_e(x4, k, y4);
    y6 = clmul_lo_e(x6, k, x7), x6 = clmul_hi_e(x6, k, y6);
    {
      static const uint64_t CRC_ALIGN(16) k_[] = {0x3da6d0cb, 0xba4fc28e};
      k = vld1q_u64(k_);
    }
    y0 = clmul_lo_e(x0, k, x2), x0 = clmul_hi_e(x0, k, y0);
    y4 = clmul_lo_e(x4, k, x6), x4 = clmul_hi_e(x4, k, y4);
    {
      static const uint64_t CRC_ALIGN(16) k_[] = {0x740eef02, 0x9e4addf8};
      k = vld1q_u64(k_);
    }
    y0 = clmul_lo_e(x0, k, x4), x0 = clmul_hi_e(x0, k, y0);
    /* Final scalar chunk. */
    crc0 = __crc32cd(crc0, *(const uint64_t *)buf);
    crc1 = __crc32cd(crc1, *(const uint64_t *)(buf + klen));
    crc2 = __crc32cd(crc2, *(const uint64_t *)(buf + klen * 2));
    crc3 = __crc32cd(crc3, *(const uint64_t *)(buf + klen * 3));
    crc4 = __crc32cd(crc4, *(const uint64_t *)(buf + klen * 4));
    crc0 = __crc32cd(crc0, *(const uint64_t *)(buf + 8));
    crc1 = __crc32cd(crc1, *(const uint64_t *)(buf + klen + 8));
    crc2 = __crc32cd(crc2, *(const uint64_t *)(buf + klen * 2 + 8));
    crc3 = __crc32cd(crc3, *(const uint64_t *)(buf + klen * 3 + 8));
    crc4 = __crc32cd(crc4, *(const uint64_t *)(buf + klen * 4 + 8));
    crc0 = __crc32cd(crc0, *(const uint64_t *)(buf + 16));
    crc1 = __crc32cd(crc1, *(const uint64_t *)(buf + klen + 16));
    crc2 = __crc32cd(crc2, *(const uint64_t *)(buf + klen * 2 + 16));
    crc3 = __crc32cd(crc3, *(const uint64_t *)(buf + klen * 3 + 16));
    crc4 = __crc32cd(crc4, *(const uint64_t *)(buf + klen * 4 + 16));
    crc0 = __crc32cd(crc0, *(const uint64_t *)(buf + 24));
    crc1 = __crc32cd(crc1, *(const uint64_t *)(buf + klen + 24));
    crc2 = __crc32cd(crc2, *(const uint64_t *)(buf + klen * 2 + 24));
    crc3 = __crc32cd(crc3, *(const uint64_t *)(buf + klen * 3 + 24));
    crc4 = __crc32cd(crc4, *(const uint64_t *)(buf + klen * 4 + 24));
    crc0 = __crc32cd(crc0, *(const uint64_t *)(buf + 32));
    crc1 = __crc32cd(crc1, *(const uint64_t *)(buf + klen + 32));
    crc2 = __crc32cd(crc2, *(const uint64_t *)(buf + klen * 2 + 32));
    crc3 = __crc32cd(crc3, *(const uint64_t *)(buf + klen * 3 + 32));
    crc4 = __crc32cd(crc4, *(const uint64_t *)(buf + klen * 4 + 32));
    buf += 40;
    vc0 = crc_shift(crc0, klen * 4 + 8);
    vc1 = crc_shift(crc1, klen * 3 + 8);
    vc2 = crc_shift(crc2, klen * 2 + 8);
    vc3 = crc_shift(crc3, klen + 8);
    vc = vgetq_lane_u64(veorq_u64(veorq_u64(vc0, vc1), veorq_u64(vc2, vc3)), 0);
    /* Reduce 128 bits to 32 bits, and multiply by x^32. */
    vc ^= vgetq_lane_u64(crc_shift(__crc32cd(__crc32cd(0, vgetq_lane_u64(x0, 0)), vgetq_lane_u64(x0, 1)), klen * 5 + 8), 0);
    /* Final 8 bytes. */
    buf += klen * 4;
    crc0 = crc4;
    crc0 = __crc32cd(crc0, *(const uint64_t *)buf ^ vc), buf += 8;
    len = end - buf;
  }
  for (; len >= 8; buf += 8, len -= 8)
  {
    crc0 = __crc32cd(crc0, *(const uint64_t *)buf);
  }
  for (; len; --len)
  {
    crc0 = __crc32cb(crc0, *buf++);
  }
  return ~crc0;
}
