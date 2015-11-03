

#include "hash_table.h"
#include "alloc.h"

#include <assert.h>

#ifndef HASH_TABLE_INIT_PREALLOC
#define HASH_TABLE_INIT_PREALLOC (8UL)
#endif

#ifndef HASH_TABLE_FORCE_RESIZE_RATIO 
#define HASH_TABLE_FORCE_RESIZE_RATIO 5
#endif

#ifndef LONG_MAX
#define LONG_MAX (long)((~(unsigned long)0)>>1)
#endif

static hash_table_t __hash_table_init__(hash_table_t ht, hlist_node_ops_t *ops);
static bool __hash_table_rehash__(hash_table_t ht, unsigned long step);
static long __hash_table_compute_index__(hash_table_t ht, hlist_node_t *node);
static void __hash_table_reset_bucket__(struct hash_bucket_s *bucket);
static bool __hash_table_bucket_if_neeed_expand__(hash_table_t ht);
static unsigned long __hash_table_compute_bucket_size__(unsigned long size);
static void __hash_table_clear_bucket(hash_table_t ht, struct hash_bucket_s *hb);

hash_table_t hash_table_new(hlist_node_ops_t *ops)
{
    hash_table_t ht = (hash_table_t) mem_alloc(sizeof(struct hash_table_s));
    if (!ht) {
        return NULL;
    }

    if (!__hash_table_init__(ht, ops)) {
        mem_free(ht);
        return NULL;
    }
    return ht;
}

void hash_table_free(hash_table_t ht)
{
    __hash_table_clear_bucket(ht, &ht->hb[0]);
    __hash_table_clear_bucket(ht, &ht->hb[1]);

    mem_free(ht);
}

void hash_table_clear(hash_table_t ht)
{
    __hash_table_clear_bucket(ht, &ht->hb[0]);
    __hash_table_clear_bucket(ht, &ht->hb[1]);

    ht->rehashidx = -1;
    ht->iterators = 0;
}

bool hash_table_insert(hash_table_t ht, hlist_node_t *node)
{
    hlist_node_t *newnode;
    struct hash_bucket_s *hb;
    long index;

    if (hash_table_is_rehashing(ht)) {
        hash_table_rehash(ht);
    }

    if ((index = __hash_table_compute_index__(ht, node)) == -1) {
        return false;
    }

    newnode = ht->ops->dup ? ht->ops->dup(node) : node;
    if (!newnode) {
        return false;
    }

    if (ht->ops->construct) {
        ht->ops->construct(newnode);
    }

    hb = hash_table_is_rehashing(ht) ? &ht->hb[1] : &ht->hb[0];

    hlist_insert(hb->bucket[index], newnode);
    hb->used++;

    return true;
}

bool hash_table_remove(hash_table_t ht, hlist_node_t *node)
{
    unsigned long hash;
    long i;

    if (ht->hb[0].size == 0) {
        return false;
    }

    if (hash_table_is_rehashing(ht)) {
        hash_table_rehash(ht);
    }

    hash = ht->ops->hashfn(node);

    for (i = 0; i < 2; i++) {
        hlist_node_t *hn;
        long index = hash & ht->hb[i].sizemask;

        hlist_for_each(ht->hb[i].bucket[index], hn) {
            if (ht->ops->compare(hn, node) == 0) {
                hlist_remove(hn);
                if (ht->ops->destructor) {
                    ht->ops->destructor(hn);
                }
                return true;
            }
        }

        if (!hash_table_is_rehashing(ht)) {
            break;
        }
    }

    return false;
}

bool hash_table_replace(hash_table_t ht, hlist_node_t *node)
{
    hlist_node_t *newnode;
    struct hash_bucket_s *hb;
    unsigned long hash;
    long index = 0;
    int i;
    
    if (hash_table_is_rehashing(ht)) {
        hash_table_rehash(ht);
    }

    if (!__hash_table_bucket_if_neeed_expand__(ht)) {
        return false;
    }

    newnode = ht->ops->dup ? ht->ops->dup(node) : node;
    if (!newnode) {
        return false;
    }

    if (ht->ops->construct) {
        ht->ops->construct(newnode);
    }

    hash = ht->ops->hashfn(node);

    for (i = 0; i < 2; i++) {
        hlist_node_t *hn;

        index = hash & ht->hb[i].sizemask;
        hlist_for_each(ht->hb[i].bucket[index], hn) {
            if (ht->ops->compare(hn, newnode) == 0) {
                hlist_replace(hn, newnode);

                if (ht->ops->destructor) {
                    ht->ops->destructor(hn);
                }
                return true;
            }
        }

        if (!hash_table_is_rehashing(ht)){
            break;
        }
    }

    hb = hash_table_is_rehashing(ht) ? &ht->hb[1] : &ht->hb[0];
    hlist_insert(hb->bucket[index], newnode);
    hb->used++;
    return true;
}

hlist_node_t *hash_table_search(hash_table_t ht, hlist_node_t *node)
{
    unsigned long hash;
    long i;

    if (ht->hb[0].size == 0) {
        return NULL;
    }

    if (hash_table_is_rehashing(ht)) {
        hash_table_rehash(ht);
    }

    hash = ht->ops->hashfn(node);

    for (i = 0; i < 2; i++) {
        hlist_node_t *hn;
        long index = hash & ht->hb[i].sizemask;
        
        hlist_for_each(ht->hb[i].bucket[index], hn) {
            if (ht->ops->compare(hn, node) == 0) {
                return hn;
            }
        }

        if (!hash_table_is_rehashing(ht)) {
            break;
        }
    }

    return NULL;
}

bool hash_table_expand_bucket(hash_table_t ht, unsigned long size)
{
    struct hash_bucket_s hb;
    unsigned long realsize;
    unsigned long i;
    
    realsize = __hash_table_compute_bucket_size__(size);

    if (hash_table_is_rehashing(ht) || ht->hb[0].used > size) {
        return false;
    }

    hb.bucket = (hlist_t*) mem_alloc(sizeof(hlist_t) * realsize);
    for (i = 0; i < realsize; i++) {
        hlist_init(hb.bucket[i]);
    }

    hb.size     = realsize;
    hb.sizemask = realsize - 1;
    hb.used     = 0;

    if (!ht->hb[0].bucket) {
        ht->hb[0] = hb;
    } else {
        ht->hb[1] = hb;
        ht->rehashidx = 0;
    }

    return true;
}

bool hash_table_rehash(hash_table_t ht)
{
    return __hash_table_rehash__(ht, 1);
}

static hash_table_t __hash_table_init__(hash_table_t ht, hlist_node_ops_t *ops)
{
    if (!ops->hashfn && !ops->compare && !ops->hashfn) {
        return NULL;
    }

    __hash_table_reset_bucket__(&ht->hb[0]);
    __hash_table_reset_bucket__(&ht->hb[1]);

    ht->ops         = ops;
    ht->rehashidx   = -1;
    ht->iterators   = 0;
    ht->enable_rehash = true;

    return ht;
}

static long __hash_table_compute_index__(hash_table_t ht, hlist_node_t *node)
{
    unsigned long h, idx = (unsigned long)~0;
    long i;

    if (!__hash_table_bucket_if_neeed_expand__(ht)) {
        return false;
    }

    h = ht->ops->hashfn(node);

    for (i = 0; i < 2; i++) {
        hlist_node_t *hn;

        idx = h & ht->hb[i].sizemask;
        hlist_for_each(ht->hb[i].bucket[idx], hn) {
            if (ht->ops->compare(hn, node) == 0) {
                return -1;
            }
        }

        if (!hash_table_is_rehashing(ht)) {
            break;
        }
    }

    return (long)idx;
}

static bool __hash_table_rehash__(hash_table_t ht, unsigned long step)
{
    int empty_visits = step * 10;

    if (!hash_table_is_rehashing(ht)) {
        return false;
    }

    while (step-- && ht->hb[0].used != 0) {
        hlist_node_t *hn, *nexthn;

        assert(ht->hb[0].size > (unsigned long)ht->rehashidx);

        while (!ht->hb[0].bucket[ht->rehashidx].first) {
            ht->rehashidx++;
            if (--empty_visits) {
                return true;
            }
        }

        hlist_safe_for_each(ht->hb[0].bucket[ht->rehashidx], hn, nexthn) {
            unsigned int index = ht->ops->hashfn(hn) & ht->hb[1].sizemask;
            hlist_insert(ht->hb[1].bucket[index], hn);
            ht->hb[0].used--;
            ht->hb[1].used++;
        }

        ht->hb[0].bucket[ht->rehashidx].first = NULL;
        ht->rehashidx++;
    }

    if (ht->hb[0].used == 0) {
        mem_free(ht->hb[0].bucket);
        ht->hb[0] = ht->hb[1];
        __hash_table_reset_bucket__(&ht->hb[1]);
        ht->rehashidx = -1;
        return false;
    }

    return true;
}

static void __hash_table_reset_bucket__(struct hash_bucket_s *hb)
{
    hb->bucket = NULL;
    hb->size = 0;
    hb->sizemask = 0;
    hb->used = 0;
}

static bool __hash_table_bucket_if_neeed_expand__(hash_table_t ht)
{
    if (hash_table_is_rehashing(ht)) {
        return true;
    }

    if (ht->hb[0].size == 0) {
        return hash_table_expand_bucket(ht, HASH_TABLE_INIT_PREALLOC);
    }

    if (ht->hb[0].used >= ht->hb[0].size &&
        (ht->enable_rehash ||
            ht->hb[0].used / ht->hb[0].size > HASH_TABLE_FORCE_RESIZE_RATIO)) {
        return hash_table_expand_bucket(ht, ht->hb[0].used * 2);
    }

    return true;
}

static unsigned long __hash_table_compute_bucket_size__(unsigned long size)
{
    unsigned long i = HASH_TABLE_INIT_PREALLOC;

    if (size >= LONG_MAX) {
        return LONG_MAX;
    }

    while (true) {
        if (i >= size) {
            return i;
        }
        i <<= 1;
    }

    return i;
}

static void __hash_table_clear_bucket(hash_table_t ht, struct hash_bucket_s *hb)
{
    unsigned long i;

    if (!hb->bucket) {
        return ;
    }

    for (i = 0; i < hb->size && hb->used > 0; i++) {
        hlist_t head;
        hlist_node_t *hn, *nexthn;

        if ((head = hb->bucket[i]).first == NULL) {
            continue;
        }

        hlist_safe_for_each(head, hn, nexthn) {
            if (ht->ops->destructor) {
                ht->ops->destructor(hn);
            }
            hb->used--;
        }
    }

    mem_free(hb->bucket);

    __hash_table_reset_bucket__(hb);
}

bool __hash_table_iter_next__(hash_table_iter_t *iterator)
{
    while (true) {
        if (!iterator->hn) {
            struct hash_bucket_s *hb = &iterator->ht->hb[iterator->bucket_index];

            iterator->index++;
            if (iterator->index >= (long) hb->size) {
                if (hash_table_is_rehashing(iterator->ht) && iterator->bucket_index == 0) {
                    iterator->index = 0;
                    hb = &iterator->ht->hb[++iterator->bucket_index];
                } else {
                    break;
                }
            }

            iterator->hn = hb->bucket[iterator->index].first;

        } else {
            iterator->hn = iterator->nexthn;
        }

        if (iterator->hn) {
            iterator->nexthn = iterator->hn->next;
            return true;
        }
    }

    iterator->bucket_index = 0;
    iterator->index = -1;
    iterator->hn = NULL;
    return false;
}