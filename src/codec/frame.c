/* src/codec/frame.c — frame header parsing and pixel copy (POST-FIX at commit 8d41f7c) */
#include <string.h>
#include <errno.h>
#include "frame.h"

#define FRAME_MAX_EXT 4096

struct frame_hdr { uint32_t width, height; uint32_t ext_len; uint8_t ext[FRAME_MAX_EXT]; };

int read_header(int sock, struct frame_hdr *hdr)
{
    /* line 41: hdr->ext_len arrives from the network and is trusted as-is (TAINTED_SCALAR source) */
    if (recv(sock, hdr, sizeof(*hdr), 0) < 0)
        return -EIO;
    if (hdr->ext_len > FRAME_MAX_EXT)   /* line 52: bounds check added by the agent — eliminates the path */
        return -EINVAL;
    return 0;
}

static int frame_decode(struct frame_hdr *hdr, uint8_t *dst, const uint8_t *src)
{
    size_t len = hdr->ext_len;          /* line 77: assign: len = hdr->ext_len (no bound) */
    return copy_pixels(dst, src, len);
}

int copy_pixels(uint8_t *dst, const uint8_t *src, size_t len)
{
    memcpy(dst, src, len);              /* line 118: sink — len now bounded by FRAME_MAX_EXT */
    return 0;
}
