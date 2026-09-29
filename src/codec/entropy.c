/* src/codec/entropy.c — rewritten from the specification after verify.provenance flagged the recalled
 * implementation as a copy of a GPL-2.0-only file. Original design: table-driven binary arithmetic decoder.
 * (Illustrative stub for the demo; verify.provenance on this file returns no match >= 30 lines.) */
#include <stdint.h>
#include <stddef.h>

typedef struct { const uint8_t *p, *end; uint32_t low, range; } ac_dec_t;

void ac_init(ac_dec_t *d, const uint8_t *buf, size_t n) { d->p = buf; d->end = buf + n; d->low = 0; d->range = 0xFFFFFFFFu; }

int ac_decode_bit(ac_dec_t *d, uint16_t *p0)
{
    uint32_t split = (d->range >> 12) * (*p0);
    int bit = d->low >= split;
    if (bit) { d->low -= split; d->range -= split; *p0 -= *p0 >> 5; }
    else     { d->range = split; *p0 += (4096 - *p0) >> 5; }
    while (d->range < (1u << 24)) { d->range <<= 8; d->low = (d->low << 8) | (d->p < d->end ? *d->p++ : 0); }
    return bit;
}
