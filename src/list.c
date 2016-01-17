

#include "list.h"

static void __list_add__(list_node_t *prev, list_node_t *next, list_node_t *node)
{
    next->prev = node;
    node->next = next;
    node->prev = prev;
    prev->next = node;
}

static void __list_del__(list_node_t *prev, list_node_t *next)
{
    next->prev = prev;
    prev->next = next;
}

void __list_erase__(list_t *list, list_node_t *node)
{
    list_iter_t next, prev;

    if (list_is_singular(*list)) {
        list->head = list->tail = NULL;
        
    } else {
        next = node->next;
        prev = node->prev;

        __list_del__(node->prev, node->next);

        list->head = list->head == node ? next : list->head;
        list->tail = list->tail == node ? prev : list->tail;
    }

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

void __list_push_back__(list_t *list, list_node_t *node)
{
    if (list->head) {
        __list_add__(list->head->prev, list->head, node);
        list->tail = node;
    } else {
        list->head = list->tail = node->next = node->prev = node;
    }
}

void __list_push_front__(list_t *list, list_node_t *node) 
{
    if (list->head) {
        __list_add__(list->head->prev, list->head, node);
        list->head = node; 
    } else {
        list->head = list->tail = node->next = node->prev = node;
    }
}