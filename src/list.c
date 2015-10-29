

#include "list.h"

bool __list_is_empty__(list_t *head)
{
    list_node_t *next = head->next;
    return (next == head) && (next == head->prev);
}

bool __list_is_singular__(list_t *head)
{
    return (!__list_is_empty__(head)) && (head->next == head->prev);
}

void __list_add__(list_node_t *prev, list_node_t *next, list_node_t *node)
{
    next->prev = node;
    node->next = next;
    node->prev = prev;
    prev->next = node;
}

static void __list_del_assist__(list_node_t *prev, list_node_t *next)
{
    next->prev = prev;
    prev->next = next;
}

void __list_del__(list_node_t *node)
{
    __list_del_assist__(node->prev, node->next);
    node->prev = NULL;
    node->next = NULL;
}

void __list_replace__(list_node_t *o, list_node_t *n)
{
    n->next = o->next;
    n->next->prev = n;
    n->prev = o->prev;
    n->prev->next = n;
}

static void __list_splice_assist__(list_node_t *prev, list_node_t *next, const list_t *head)
{
    list_node_t *first = head->next;
    list_node_t *last = head->prev;

    first->prev = prev;
    prev->next  = first;

    last->next = next;
    next->prev = last;
}

void __list_splice__(list_node_t *pos, list_t *head)
{
    __list_splice_assist__(pos, pos->next, head);
    head->prev = head;
    head->next = head;
}

void __list_split__(list_t *n, list_t *o, list_node_t *cut)
{
    list_t *first = cut->next;

    n->next = o->next;
    n->next->prev = n;

    n->prev = cut;
    cut->next = n;

    o->next = first;
    first->prev = o;
}

void __list_insertion_sort__(list_t *head, list_compare_pt compare)
{
    list_node_t *node;
    list_node_t *prev;

    if (__list_is_empty__(head) || __list_is_singular__(head)) {
        return ;
    }

    node = head->next;

    for (node = node->next; node != head; node = node->next) {

        prev = node->prev;

        __list_del__(node);

        do {
            if (compare(node, prev) >= 0) {
                break;
            }

            prev = prev->prev;

        } while(prev != head);

        __list_add__(prev, prev->next, node);
    }
}

void __list_reverse__(list_t *head)
{
    list_node_t *current = head, *next = current->next;
    list_node_t *temp;

    do {

        temp = current->next;
        current->next = current->prev;
        current->prev = temp;
        current = next, next = next->next;

    } while (current != head);
}

void __list_magic__(list_t *head, list_magic_pt magic)
{
    list_iter_t iter;

    for (iter = head->next; iter != head; iter = iter->next) {
        if (!magic(iter)) {
            break;
        }
    }
}

void __list_clear__(list_t *head, list_magic_pt magic)
{
    list_node_t *node;

    while (!__list_is_empty__(head)) {

        node = head->next;

        __list_del__(head->next);

        if (magic) {
            if (!magic(node))
                break;
        }
    }
}
