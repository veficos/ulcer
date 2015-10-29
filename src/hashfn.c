

#include "hashfn.h"

uint32_t murmur2_hash(unsigned char *data, unsigned long len)
{
    uint32_t  h, k;

    h = 0 ^ len;

    while (len >= 4) {
        k = data[0];
        k |= data[1] << 8;
        k |= data[2] << 16;
        k |= data[3] << 24;

        k *= 0x5BD1E995;
        k ^= k >> 24;
        k *= 0x5BD1E995;

        h *= 0x5BD1E995;
        h ^= k;

        data += 4;
        len -= 4;
    }

    switch (len) {
    case 3:
        h ^= data[2] << 16;
    case 2:
        h ^= data[1] << 8;
    case 1:
        h ^= data[0];
        h *= 0x5BD1E995;
    }

    h ^= h >> 13;
    h *= 0x5BD1E995;
    h ^= h >> 15;

    return h;
}

uint32_t rabin_karp_hash(const unsigned char *data, unsigned long len, uint32_t *pow)
{
    uint32_t     hash = 0;
    unsigned int i = 0, p = 1, sq = HASH_PRIME_RK;

    for (; i < len; i++) {
        hash = hash * HASH_PRIME_RK + (uint32_t)data[i];
    }

    for (i = len; i > 0; i >>= 1) {
        if ((i & 1) != 0) {
            p *= sq;
        }
        sq *= sq;
    }

    if (pow) {
        *pow = p;
    }

    return hash;
}
