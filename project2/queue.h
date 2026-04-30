#ifndef QUEUE_H
#define QUEUE_H

#include <stddef.h>

typedef struct queue_node {
    void *data;
    struct queue_node *next;
} queue_node_t;

typedef struct queue {
    queue_node_t *head;
    queue_node_t *tail;
    size_t size;
} queue_t;

queue_t *queue_create(void);
void queue_destroy(queue_t **q, void (*free_fn)(void *));
void queue_clear(queue_t *q, void (*free_fn)(void *));

int queue_push(queue_t *q, void *data);
void *queue_pop(queue_t *q);
void *queue_peek(const queue_t *q);

int queue_is_empty(const queue_t *q);
size_t queue_size(const queue_t *q);

#endif
