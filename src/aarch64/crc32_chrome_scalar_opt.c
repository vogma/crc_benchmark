#include <stddef.h>
#include <stdint.h>

#include <arm_neon.h>
#include <arm_acle.h>
#include <stdatomic.h>

/*
   Constants empirically determined to maximize speed. These values are from
   measurements on a Cortex-A57. Your mileage may vary.
 */
#define Z_BATCH 3990             /* number of words in a batch */
#define Z_BATCH_ZEROS 0xa10d3d0c /* computed from Z_BATCH = 3990 */
#define Z_BATCH_MIN 800          /* fewest words in a final batch */
typedef unsigned z_crc_t;

#define Z_U8 unsigned
typedef Z_U8 z_word_t;

#define z_off64_t long

#define POLY 0xedb88320 /* p(x) reflected, with x^32 implied */

z_crc_t x2n_table[32];

/* Definition of once functionality. */
typedef struct once_s once_t;

/* Structure for once(), which must be initialized with ONCE_INIT. */
struct once_s
{
    atomic_flag begun;
    atomic_int done;
};
#define ONCE_INIT {ATOMIC_FLAG_INIT, 0}

/*
  Run the provided init() function exactly once, even if multiple threads
  invoke once() at the same time. The state must be a once_t initialized with
  ONCE_INIT.
 */
void once(once_t *state, void (*init)(void))
{
    if (!atomic_load(&state->done))
    {
        if (atomic_flag_test_and_set(&state->begun))
            while (!atomic_load(&state->done))
                ;
        else
        {
            init();
            atomic_store(&state->done, 1);
        }
    }
}

/*
  Return a(x) multiplied by b(x) modulo p(x), where p(x) is the CRC polynomial,
  reflected. For speed, this requires that a not be zero.
 */
z_crc_t multmodp(z_crc_t a, z_crc_t b)
{
    z_crc_t m, p;

    m = (z_crc_t)1 << 31;
    p = 0;
    for (;;)
    {
        if (a & m)
        {
            p ^= b;
            if ((a & (m - 1)) == 0)
                break;
        }
        m >>= 1;
        b = b & 1 ? (b >> 1) ^ POLY : b >> 1;
    }
    return p;
}

void make_crc_table(void)
{
    unsigned n;
    z_crc_t p;

    /* initialize the x^2^n mod p(x) table */
    p = (z_crc_t)1 << 30; /* x^1 */
    x2n_table[0] = p;
    for (n = 1; n < 32; n++)
        x2n_table[n] = p = multmodp(p, p);
}

once_t made = ONCE_INIT;
/*
  Return x^(n * 2^k) modulo p(x). Requires that x2n_table[] has been
  initialized.
 */
z_crc_t x2nmodp(z_off64_t n, unsigned k)
{
    z_crc_t p;

    p = (z_crc_t)1 << 31; /* x^0 == 1 */
    while (n)
    {
        if (n & 1)
            p = multmodp(x2n_table[k & 31], p);
        n >>= 1;
        k++;
    }
    return p;
}

unsigned long crc32_chromium_scalar_opt(unsigned long crc, const unsigned char *buf,
                                        size_t len)
{
    z_crc_t val;
    z_word_t crc1, crc2;
    const z_word_t *word;
    z_word_t val0, val1, val2;
    size_t last, last2, i;
    size_t num;

    /* Return initial CRC, if requested. */
    if (buf == NULL)
        return 0;

    once(&made, make_crc_table);

    /* Pre-condition the CRC */
    crc = (~crc) & 0xffffffff;

    /* Compute the CRC up to a word boundary. */
    while (len && ((size_t)buf & 7) != 0)
    {
        len--;
        val = *buf++;
        __asm__ volatile("crc32b %w0, %w0, %w1" : "+r"(crc) : "r"(val));
    }

    /* Prepare to compute the CRC on full 64-bit words word[0..num-1]. */
    word = (z_word_t const *)buf;
    num = len >> 3;
    len &= 7;

    /* Do three interleaved CRCs to realize the throughput of one crc32x
       instruction per cycle. Each CRC is calculated on Z_BATCH words. The
       three CRCs are combined into a single CRC after each set of batches. */
    while (num >= 3 * Z_BATCH)
    {
        crc1 = 0;
        crc2 = 0;
        for (i = 0; i < Z_BATCH; i++)
        {
            val0 = word[i];
            val1 = word[i + Z_BATCH];
            val2 = word[i + 2 * Z_BATCH];
            __asm__ volatile("crc32x %w0, %w0, %x1" : "+r"(crc) : "r"(val0));
            __asm__ volatile("crc32x %w0, %w0, %x1" : "+r"(crc1) : "r"(val1));
            __asm__ volatile("crc32x %w0, %w0, %x1" : "+r"(crc2) : "r"(val2));
        }
        word += 3 * Z_BATCH;
        num -= 3 * Z_BATCH;
        crc = multmodp(Z_BATCH_ZEROS, crc) ^ crc1;
        crc = multmodp(Z_BATCH_ZEROS, crc) ^ crc2;
    }

    /* Do one last smaller batch with the remaining words, if there are enough
       to pay for the combination of CRCs. */
    last = num / 3;
    if (last >= Z_BATCH_MIN)
    {
        last2 = last << 1;
        crc1 = 0;
        crc2 = 0;
        for (i = 0; i < last; i++)
        {
            val0 = word[i];
            val1 = word[i + last];
            val2 = word[i + last2];
            __asm__ volatile("crc32x %w0, %w0, %x1" : "+r"(crc) : "r"(val0));
            __asm__ volatile("crc32x %w0, %w0, %x1" : "+r"(crc1) : "r"(val1));
            __asm__ volatile("crc32x %w0, %w0, %x1" : "+r"(crc2) : "r"(val2));
        }
        word += 3 * last;
        num -= 3 * last;
        val = x2nmodp(last, 6);
        crc = multmodp(val, crc) ^ crc1;
        crc = multmodp(val, crc) ^ crc2;
    }

    /* Compute the CRC on any remaining words. */
    for (i = 0; i < num; i++)
    {
        val0 = word[i];
        __asm__ volatile("crc32x %w0, %w0, %x1" : "+r"(crc) : "r"(val0));
    }
    word += num;

    /* Complete the CRC on any remaining bytes. */
    buf = (const unsigned char *)word;
    while (len)
    {
        len--;
        val = *buf++;
        __asm__ volatile("crc32b %w0, %w0, %w1" : "+r"(crc) : "r"(val));
    }

    /* Return the CRC, post-conditioned. */
    return crc ^ 0xffffffff;
}