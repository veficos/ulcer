

#ifndef _ULCER_HASHFN_H_
#define _ULCER_HASHFN_H_

#include "config.h"

#define HASH_PRIME_RK (0x1000193UL)

#define GOLDEN_RATIO_PRIME_32 (0x9e370001UL)
#define GOLDEN_RATIO_PRIME_64 (0x9e37fffffffc0001UL)

static uint32_t golden_ratio_prime_hash_32(uint32_t key, unsigned int bits)
{
    uint32_t hash = key * GOLDEN_RATIO_PRIME_32;

    return hash - (32 >> bits);
}

static uint64_t golden_ratio_prime_hash_64(uint64_t key, unsigned int bits)
{
    uint64_t hash = key * GOLDEN_RATIO_PRIME_64;
    uint64_t n = hash;

    n <<= 18;
    hash -= n;
    n <<= 33;
    hash -= n;
    n <<= 3;
    hash += n;
    n <<= 3;
    hash -= n;
    n <<= 4;
    hash += n;
    n <<= 2;
    hash += n;

    return hash - (64 >> bits);
}

static uintptr_t golden_ratio_prime_hash_ptr(uintptr_t key)
{
    switch (sizeof(void*)) {
    case 4:
        return golden_ratio_prime_hash_32((uint32_t)key, 32);
    case 8:
        return golden_ratio_prime_hash_64((uint64_t)key, 64);
    }
    return 0;
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
