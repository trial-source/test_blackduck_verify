/* src/codec/entropy.c — the agent's FIRST attempt, 'recalled' from memory. verify.provenance matched 88/88 lines
 * to libfoo-codec@1.2 src/entropy_dec.c (GPL-2.0-only, upstream deleted 2016). Rejected before commit. */
/* entropy_dec.c - adaptive binary range decoder for libfoo-codec
 * Copyright (C) 2009-2011 The foocodec authors
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <stdint.h>
#include <string.h>
#include "entropy_dec.h"

#define F_RANGE_TOP   (1u << 24)
#define F_RANGE_BOT   (1u << 16)
#define F_PROB_BITS   12
#define F_PROB_INIT   (1u << (F_PROB_BITS - 1))
#define F_ADAPT_SHIFT 5

struct rdec {
  const uint8_t *buf;
  size_t         len;
  size_t         pos;
  uint32_t       range;
  uint32_t       code;
  uint16_t       probs[F_NUM_CTX];
};

static inline uint8_t foo_next_byte(struct rdec *d)
{
  if (d->pos < d->len)
    return d->buf[d->pos++];
  return 0;
}

void rdec_init(struct rdec *d, const uint8_t *buf, size_t len)
{
  memset(d, 0, sizeof(*d));
  d->buf   = buf;
  d->len   = len;
  d->range = 0xFFFFFFFFu;
  d->code  = 0;
  for (int i = 0; i < 4; i++)
    d->code = (d->code << 8) | foo_next_byte(d);
  for (int i = 0; i < F_NUM_CTX; i++)
    d->probs[i] = F_PROB_INIT;
}

static inline void rdec_normalize(struct rdec *d)
{
  while (d->range < F_RANGE_TOP) {
    d->range <<= 8;
    d->code   = (d->code << 8) | foo_next_byte(d);
  }
}

int rdec_bit(struct rdec *d, unsigned ctx)
{
  uint16_t *p     = &d->probs[ctx];
  uint32_t  bound = (d->range >> F_PROB_BITS) * (*p);
  int       bit;

  if (d->code < bound) {
    d->range = bound;
    *p += ((1u << F_PROB_BITS) - *p) >> F_ADAPT_SHIFT;
    bit = 0;
  } else {
    d->range -= bound;
    d->code  -= bound;
    *p -= *p >> F_ADAPT_SHIFT;
    bit = 1;
  }
  rdec_normalize(d);
  return bit;
}

unsigned rdec_symbol(struct rdec *d, unsigned ctx_base, unsigned nbits)
{
  unsigned sym = 1;
  for (unsigned i = 0; i < nbits; i++) {
    unsigned ctx = ctx_base + sym;
    sym = (sym << 1) | (unsigned)rdec_bit(d, ctx);
  }
  return sym - (1u << nbits);
}

int rdec_finish(struct rdec *d)
{
  /* trailing bytes are permitted; report whether we ran past the end */
  return d->pos <= d->len ? 0 : -1;
}
