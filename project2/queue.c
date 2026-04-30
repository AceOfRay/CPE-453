#include "queue.h"

#include <stdlib.h>

queue_t *queue_create(void)
{
    queue_t *q = (queue_t *)malloc(sizeof(*q));
    if (!q) {
        return NULL;
    }
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    return q;
}

void queue_clear(queue_t *q, void (*free_fn)(void *))
{
    if (!q) {
        return;
    }
    while (q->head) {
        queue_node_t *n = q->head;
        q->head = n->next;
        if (free_fn) {
            free_fn(n->data);
        }
        free(n);
    }
    q->tail = NULL;
    q->size = 0;
}

void queue_destroy(queue_t **q, void (*free_fn)(void *))
{
    if (!q || !*q) {
        return;
    }
    queue_clear(*q, free_fn);
    free(*q);
    *q = NULL;
}

int queue_push(queue_t *q, void *data)
{
    if (!q) {
        return -1;
    }
    queue_node_t *n = (queue_node_t *)malloc(sizeof(*n));
    if (!n) {
        return -1;
    }
    n->data = data;
    n->next = NULL;
    if (q->tail) {
        q->tail->next = n;
    } else {
        q->head = n;
    }
    q->tail = n;
    q->size++;
    return 0;
}

void *queue_pop(queue_t *q)
{
    if (!q || !q->head) {
        return NULL;
    }
    queue_node_t *n = q->head;
    void *data = n->data;
    q->head = n->next;
    if (!q->head) {
        q->tail = NULL;
    }
    q->size--;
    free(n);
    return data;
}

void *queue_peek(const queue_t *q)
{
    if (!q || !q->head) {
        return NULL;
    }
    return q->head->data;
}

int queue_is_empty(const queue_t *q)
{
    return !q || q->size == 0;
}

size_t queue_size(const queue_t *q)
{
    return q ? q->size : 0;
}
