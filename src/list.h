

#ifndef _ULCER_LIST_H_
#define _ULCER_LIST_H_

#include "config.h"

struct list_s {
    struct list_s *prev;
    struct list_s *next;
};

typedef struct list_s list_t;
typedef struct list_s list_node_t;
typedef struct list_s* list_iter_t;
typedef bool(*list_magic_pt)(list_node_t*);
typedef int (*list_compare_pt)(const list_node_t*, const list_node_t*);

#define list_init(list)                                                       \
    do {                                                                      \
        (list).next = &(list);                                                \
        (list).prev = &(list);                                                \
    } while (false)

#define list_element(ptr, type, member)                                       \
    ((type)(((unsigned char*)(ptr)) - (unsigned char*)(&(((type)0)->member))))

#define list_is_empty(list)                                                   \
    __list_is_empty__((list).next->prev)

#define list_is_singular(list)                                                \
    __list_is_singular__((list).next->prev)

#define list_replace(o, n)                                                    \
    __list_replace__(&(o), &(n))

#define list_splice(pos, list)                                                \
    __list_splice__((pos), (list).next->prev)

#define list_split(newlist, oldlist, cut)                                     \
    __list_split__((newlist).next->prev, (oldlist).next->prev, &(cut))

#define list_push_back_list(list, backlist)                                   \
    __list_splice__((list).prev, (backlist).next->prev)

#define list_push_front_list(list, frontlist)                                 \
    __list_splice__((list).next->prev, (frontlist).next->prev)

#define list_push_back(list, node)                                            \
    __list_add__((list).prev, (list).next->prev, &(node))

#define list_push_front(list, node)                                           \
    __list_add__((list).next->prev, (list).next, &(node))

#define list_pop_back(list)                                                   \
    __list_del__((list).prev)                                                 \

#define list_pop_front(list)                                                  \
    __list_del__((list).next)

#define list_erase(node)                                                      \
    __list_del__(&(node))

#define list_reverse(list)                                                    \
    __list_reverse__((list).next->prev)

#define list_insertion_sort(list, compare)                                    \
    __list_insertion_sort__((list).next->prev, compare)

#define list_magic(list, magic)                                               \
    __list_magic__((list).next->prev, magic)

#define list_clear(list, magic)                                               \
    __list_clear__((list).next->prev, magic)

#define list_begin(list)     ((list).next)

#define list_rbegin(list)    ((list).prev)

#define list_end(list)       ((list).next->prev)

#define list_rend(list)      ((list).next->prev)

#define list_next(iter)      ((iter)->next)

#define list_rnext(iter)     ((iter)->prev)

#define list_prev(iter)      ((iter)->prev)

#define list_rprev(iter)     ((iter)->next)

#define list_for_each(list, iter)                                             \
    for ((iter) = (list).next;                                                \
         (iter) != (list).next->prev;                                         \
         (iter) = (iter)->next)

#define list_safe_for_each(list, iter, next_iter)                             \
    for ((iter) = (list).next, (next_iter) = (iter)->next;                    \
         (iter) != (list).next->prev;                                         \
         (iter) = (next_iter), (next_iter) = (iter)->next)

#define list_reverse_for_each(list, iter)                                     \
    for ((iter) = (list).prev;                                                \
         (iter) != (list).next->prev;                                         \
         (iter) = (iter)->prev)

#define list_safe_reverse_for_each(list, iter, next_iter)                     \
    for ((iter) = (list).prev, (next_iter) = (iter)->prev;                    \
         (iter) != (list).next->prev;                                         \
         (iter) = (next_iter), (next_iter) = (iter)->prev)

/* low level interface */
bool __list_is_empty__(list_t *head);
bool __list_is_singular__(list_t *head);
void __list_add__(list_node_t *prev, list_node_t *next, list_node_t *node);
void __list_del__(list_node_t *node);
void __list_replace__(list_node_t *o, list_node_t *n);
void __list_splice__(list_node_t *pos, list_t *head);
void __list_split__(list_t *n, list_t *o, list_t *cut);
void __list_reverse__(list_t *head);
void __list_insertion_sort__(list_t *head, list_compare_pt compare);
void __list_magic__(list_t *head, list_magic_pt magic);
void __list_clear__(list_t *head, list_magic_pt magic);

#endif
