

#ifndef _ULCER_QUEUE_H_
#define _ULCER_QUEUE_H_

#include "config.h"

typedef struct queue_node_s {
    struct queue_node_s *next;
}queue_node_t;

typedef struct queue_s {
    queue_node_t *tail;
    queue_node_t *head;
}queue_t;

static void __queue_enqueue__(queue_t *queue, queue_node_t *node)
{
    if (queue->tail) {
        queue->tail->next = node;
    }

    if (!queue->head) {
        queue->head = node;
    }

    node->next = NULL;
    queue->tail = node;
}

static queue_node_t *__queue_dequeue__(queue_t *queue)
{
    queue_node_t* node = queue->head;

    if (node) {
        queue->head = node->next;

        if (!queue->head) {
            queue->tail = NULL;
        }
    }

    return node;
}

#define queue_init(queue)                                                     \
    do {                                                                      \
        (queue).head = (queue).tail = NULL;                                   \
    } while (false)

#define queue_enqueue(queue, node)                                            \
    __queue_enqueue__(&(queue), node);

#define queue_dequeue(queue, node)                                            \
    __queue_dequeue__(&(queue), node)

#define queue_is_empty(queue)                                                 \
    ((((queue).head) == NULL) && (((queue).tail) == NULL))

#define queue_peek(queue)                                                     \
    ((queue).head)

#define queue_element(ptr, type, member)                                      \
    ((type)(((unsigned char*)(ptr)) - (unsigned char*)(&(((type)0)->member))))

#endif
