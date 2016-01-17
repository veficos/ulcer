

#ifndef _ULCER_HLIST_H_
#define _ULCER_HLIST_H_

#include "config.h"

typedef struct hlist_node_s {
    struct hlist_node_s *next;
    struct hlist_node_s **pprev;
} hlist_node_t;

typedef struct hlist_s {
    struct hlist_node_s *first;
} hlist_t;

static bool __hlist_is_empty__(hlist_t *hlist)
{
    return (!hlist->first);
}

static void __hlist_move__(hlist_t *o, hlist_t *n)
{
    n->first = o->first;
    if (n->first) {
        n->first->pprev = &n->first;
    }
    o->first = NULL;
}

static void __hlist_insert__(hlist_t *hlist, hlist_node_t *node)
{
    hlist_node_t *first = hlist->first;

    node->next = first;
    if (first) {
        first->pprev = &node->next;
    }
    hlist->first = node;
    node->pprev = &hlist->first;
}

static void hlist_remove(hlist_node_t *node)
{
    hlist_node_t *next = node->next;
    hlist_node_t **pprev = node->pprev;

    *pprev = next;
    if (next) {
        next->pprev = pprev;
    }
}

static void hlist_insert_front(hlist_node_t *next, hlist_node_t *node)
{
    node->next = next;
    node->pprev = next->pprev;
    next->pprev = &node->next;
    *(node->pprev) = node;
}

static void hlist_insert_back(hlist_node_t *prev, hlist_node_t *node)
{
    node->next = prev->next;
    prev->next = node;
    node->pprev = &prev->next;

    if (node->next) {
        node->next->pprev = &node->next;
    }
}

static void hlist_replace(hlist_node_t *o, hlist_node_t *n)
{
    hlist_insert_front(o, n);
    hlist_remove(o);
}

#define hlist_init(hlist)                                                     \
    do {                                                                      \
        (&hlist)->first = NULL;                                               \
    } while (false)

#define hlist_is_empty(hlist)                                                 \
    __hlist_is_empty__(&(hlist))

#define hlist_insert(hlist, node)                                             \
    __hlist_insert__(&(hlist), (node))

#define hlist_move(o, n)                                                      \
    __hlist_move__(&(o), &(n))

#define hlist_element(ptr, type, member)                                      \
    ((type)(((unsigned char*)(ptr)) - (unsigned char*)(&(((type)0)->member))))

#define hlist_for_each(hlist, it)                                             \
    for ((it) = (&(hlist))->first; (it); (it)=(it)->next)

#define hlist_safe_for_each(hlist, it, nextit)                                \
    for ((it) = (&(hlist))->first, (nextit) = (it)->next;                     \
         (it);                                                                \
         (it) = (nextit),                                                     \
         (nextit) = (nextit) ? (nextit)->next : NULL)

#endif
