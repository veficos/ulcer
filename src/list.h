

#ifndef _ULCER_LIST_H_
#define _ULCER_LIST_H_

#include "config.h"

struct list_node_s {
    struct list_node_s *prev;
    struct list_node_s *next;
};

struct list_s {
    struct list_node_s  node;
    struct list_node_s* self;
};

typedef struct list_s list_t;
typedef struct list_node_s list_node_t;
typedef struct list_node_s* list_iter_t;
typedef bool(*list_magic_pt)(list_node_t*);
typedef int(*list_compare_pt)(const list_node_t*, const list_node_t*);

#define list_init(list)                                                       \
    do {                                                                      \
        (list).node.next = &(list).node;                                      \
        (list).node.prev = &(list).node;                                      \
        (list).self = &(list).node;                                           \
    } while (false)

#define list_element(ptr, type, member)                                       \
    ((type)(((unsigned char*)(ptr)) - (unsigned char*)(&(((type)0)->member))))

#define list_is_empty(list)                                                   \
    __list_is_empty__((list).self)

#define list_is_singular(list)                                                \
    __list_is_singular__((list).self)

#define list_replace(o, n)                                                    \
    __list_replace__(&(o), &(n))

#define list_splice(pos, list)                                                \
    __list_splice__(&(pos), (list).self)

#define list_split(newlist, oldlist, cut)                                     \
    __list_split__((newlist).self, (oldlist).self, &(cut))

#define list_push_back_list(list, backlist)                                   \
    __list_splice__((list).self->prev, (backlist).self)

#define list_push_front_list(list, frontlist)                                 \
    __list_splice__((list).self, (frontlist).self)

#define list_push_back(list, node)                                            \
    __list_add__((list).self->prev, (list).self, &(node))
    
#define list_push_front(list, node)                                           \
    __list_add__((list).self, (list).self->next, &(node))

#define list_pop_back(list)                                                   \
    __list_del__((list).self->prev)

#define list_pop_front(list)                                                  \
    __list_del__((list).self->next)

#define list_erase(node)                                                      \
    __list_del__(&(node))

#define list_reverse(list)                                                    \
    __list_reverse__((list).self)

#define list_insertion_sort(list, compare)                                    \
    __list_insertion_sort__((list).self, compare)

#define list_magic(list, magic)                                               \
    __list_magic__((list).self, magic)

#define list_clear(list, magic)                                               \
    __list_clear__((list).self, magic)

#define list_begin(list)     ((list).self->next)

#define list_rbegin(list)    ((list).self->prev)

#define list_end(list)       ((list).self)

#define list_rend(list)      ((list).self)

#define list_next(iter)      ((iter)->next)

#define list_rnext(iter)     ((iter)->prev)

#define list_prev(iter)      ((iter)->prev)

#define list_rprev(iter)     ((iter)->next)

#define list_for_each(list, iter)                                             \
    for ((iter) = (list).self->next;                                          \
         (iter) != (list).self;                                               \
         (iter) = (iter)->next)

#define list_safe_for_each(list, iter, next_iter)                             \
    for ((iter) = (list).self->next, (next_iter) = (iter)->next;              \
         (iter) != (list).self;                                               \
         (iter) = (next_iter), (next_iter) = (iter)->next)

#define list_reverse_for_each(list, iter)                                     \
    for ((iter) = (list).self->prev;                                          \
         (iter) != (list).self;                                               \
         (iter) = (iter)->prev)

#define list_safe_reverse_for_each(list, iter, next_iter)                     \
    for ((iter) = (list).self->prev, (next_iter) = (iter)->prev;              \
         (iter) != (list).self;                                               \
         (iter) = (next_iter), (next_iter) = (iter)->prev)

/* low level interface */
bool __list_is_empty__(list_node_t *head);
bool __list_is_singular__(list_node_t *head);
void __list_add__(list_node_t *prev, list_node_t *next, list_node_t *node);
void __list_del__(list_node_t *node);
void __list_replace__(list_node_t *o, list_node_t *n);
void __list_splice__(list_node_t *pos, list_node_t *head);
void __list_split__(list_node_t *n, list_node_t *o, list_node_t *cut);
void __list_reverse__(list_node_t *head);
void __list_insertion_sort__(list_node_t *head, list_compare_pt compare);
void __list_magic__(list_node_t *head, list_magic_pt magic);
void __list_clear__(list_node_t *head, list_magic_pt magic);

#endif
