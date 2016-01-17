

#ifndef _ULCER_HASHTABLE_H_
#define _ULCER_HASHTABLE_H_

#include "config.h"
#include "hlist.h"

typedef struct hlist_node_ops_s {
    void (*construct)(hlist_node_t *node);
    void (*destructor)(hlist_node_t *node);
    unsigned long (*hashfn)(const hlist_node_t *hode);
    int (*compare)(const hlist_node_t *lhs, const hlist_node_t *rhs);
    hlist_node_t* (*dup)(const hlist_node_t *node);
}hlist_node_ops_t;

struct hash_bucket_s {
    hlist_t *bucket;
    unsigned long size;
    unsigned long sizemask;
    unsigned long used;
};

struct hash_table_s {
    struct hash_bucket_s hb[2];
    hlist_node_ops_t *ops;
    long rehashidx;
    long iterators;
    bool enable_rehash;
};

typedef struct hash_table_s* hash_table_t;

#define hash_table_is_rehashing(ht)                                           \
    ((ht)->rehashidx != -1)

#define hash_table_disable_rehash(ht, off)                                    \
    ((ht)->enable_rehash = (off))

#define hash_table_size(ht)                                                   \
    ((ht)->hb[0].used+(ht)->hb[1].used)

hash_table_t hash_table_new(hlist_node_ops_t *ops);
void hash_table_free(hash_table_t ht);
void hash_table_clear(hash_table_t ht);
bool hash_table_insert(hash_table_t ht, hlist_node_t *node);
bool hash_table_remove(hash_table_t ht, hlist_node_t *node);
bool hash_table_replace(hash_table_t ht, hlist_node_t *node);
hlist_node_t *hash_table_search(hash_table_t ht, hlist_node_t *node);

/* low level interface */
bool hash_table_expand_bucket(hash_table_t ht, unsigned long size);
bool hash_table_rehash(hash_table_t ht);

/* iterator */
typedef struct hash_table_iter_s {
    hash_table_t ht;
    hlist_node_t *hn;
    hlist_node_t *nexthn;
    int bucket_index;
    long index;
}hash_table_iter_t;

static hash_table_iter_t hash_table_iter_new(hash_table_t ht)
{
    hash_table_iter_t iter;
    iter.ht     = ht;
    iter.hn     = NULL;
    iter.nexthn = NULL;
    iter.bucket_index  = 0;
    iter.index  = -1;
    ht->iterators++;
    return iter;
}

#define hash_table_iter_free(iter)                                            \
    do {                                                                      \
        (iter).ht->iterators--;                                               \
        (iter).ht = NULL;                                                     \
    } while(false)

#define hash_table_iter_equals(lhs, rhs)                                      \
    (((lhs).ht   == (rhs).ht) &&                                              \
    ((lhs).hn     == (rhs).hn) &&                                             \
    ((lhs).nexthn == (rhs).nexthn) &&                                         \
    ((lhs).table  == (rhs).table) &&                                          \
    ((lhs).index  == (rhs).index))

#define hash_table_iter_element(iter, type, member)                           \
    (!(iter).hn ? NULL : hlist_element((iter).hn, type, member))

#define hash_table_iter_next(iter)                                            \
    __hash_table_iter_next__(&(iter))

#define hash_table_for_each(hash, iter)                                       \
    for (iter = hash_table_iter_new(hash); \
            hash_table_iter_next(iter);)

bool __hash_table_iter_next__(hash_table_iter_t *iterator);

#endif