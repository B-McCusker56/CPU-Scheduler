/* Simple deque implementation using a doubly linked list. */

#include <stdlib.h>
#include "deque.h"

struct deque_node
{
    struct deque_node* next;
    struct deque_node* prev;
    void* data;
};

struct deque
{
    int size;
    struct deque_node* dummy;
};

struct deque* deque_create()
{
    struct deque* q = malloc(sizeof(struct deque));
    if(!q)
        return NULL;
    q->size = 0;
    q->dummy = malloc(sizeof(struct deque_node));
    if(!q->dummy)
    {
        free(q);
        return NULL;
    }
    q->dummy->next = q->dummy;
    q->dummy->prev = q->dummy;
    return q;
}

void deque_destroy(struct deque* q)
{
    if(!q)
        return;
    struct deque_node* cur = q->dummy->next;
    while(cur != q->dummy)
    {
        struct deque_node* next = cur->next;
        free(cur->data);
        free(cur);
        cur = next;
    }
    free(q->dummy);
    free(q);
}

int deque_push_front(struct deque* q, void* data)
{
    if(!q)
        return 0;
    struct deque_node* node = malloc(sizeof(struct deque_node));
    if(!node)
        return 0;
    node->data = data;
    node->prev = q->dummy;
    node->next = q->dummy->next;
    node->next->prev = node;
    q->dummy->next = node;
    return ++q->size;
}

int deque_push_back(struct deque* q, void* data)
{
    if(!q)
        return 0;
    struct deque_node* node = malloc(sizeof(struct deque_node));
    if(!node)
        return 0;
    node->data = data;
    node->prev = q->dummy->prev;
    node->next = q->dummy;
    q->dummy->prev = node;
    node->prev->next = node;
    return ++q->size;
}

void* deque_pop_front(struct deque* q)
{
    if(!q || !q->size)
        return NULL;
    struct deque_node* front = q->dummy->next;
    void* data = front->data;
    q->dummy->next = front->next;
    front->next->prev = q->dummy;
    free(front);
    --q->size;
    return data;
}

void* deque_pop_back(struct deque* q)
{
    if(!q || !q->size)
        return NULL;
    struct deque_node* back = q->dummy->prev;
    void* data = back->data;
    q->dummy->prev = back->prev;
    back->prev->next = q->dummy;
    free(back);
    --q->size;
    return data;
}

int deque_size(struct deque* q)
{
    return q ? q->size : -1;
}
