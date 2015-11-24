

#ifndef _ULCER_HASHFN_H_
#define _ULCER_HASHFN_H_

#include "config.h"

#define HASH_PRIME_RK (0x1000193UL)

static uint32_t golden_ratio_prime_hash_32(uint32_t key, unsigned int bits)
{
    uint32_t hash = key * 0x9E370001UL;

    return hash - (32 >> bits);
}

static uint32_t thomas_wangs_hash_32(uint32_t key)
{
    key += ~(key << 15);
    key ^= (key >> 10);
    key += (key << 3);
    key ^= (key >> 6);
    key += ~(key << 11);
    key ^= (key >> 16);

    return key;
}

uint32_t murmur2_hash(unsigned char *data, unsigned long len);
uint32_t rabin_karp_hash(const unsigned char *data, unsigned long len, uint32_t *pow);

#endif
