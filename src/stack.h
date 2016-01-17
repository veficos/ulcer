

#ifndef _ULCER_STACK_H_
#define _ULCER_STACK_H_

#include "config.h"

typedef struct stack_node_s {
    struct stack_node_s *next;
}stack_node_t;

typedef struct stack_t {
    stack_node_t *top;
}stack_t;

static void __stack_push__(stack_t *stack, stack_node_t *node)
{
    node->next = stack->top;
    stack->top = node;
}

static stack_node_t *__stack_pop__(stack_t *stack)
{
    stack_node_t *node = stack->top;

    if (node) {
        stack->top = node->next;
        node->next = NULL;
    }

    return node;
}

#define stack_init(stack)                                                     \
    do {                                                                      \
        (stack).top = NULL;                                                   \
    } while (false)

#define stack_push(stack, node)                                               \
    __stack_push__(&(stack), &(node))

#define stack_pop(stack)                                                      \
    __stack_pop__(&(stack))

#define stack_element(ptr, type, member)                                      \
    ((type)(((unsigned char*)(ptr)) - (unsigned char*)(&(((type)0)->member))))

#define stack_is_empty(stack)                                                 \
    ((stack).top == NULL)

#define stack_peek(stack)                                                     \
    ((stack).top)

#endif
