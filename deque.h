#ifndef DEQUE_H
#define DEQUE_H

struct deque
{
    int size;
    struct deque_node* dummy;
};

struct deque_node
{
    struct deque_node* next;
    struct deque_node* prev;
    void* data;
};

struct deque* deque_create();
void deque_destroy(struct deque* q);
int deque_push_front(struct deque* q, void* data);
int deque_push_back(struct deque* q, void* data);
void* deque_pop_front(struct deque* q);
void* deque_pop_back(struct deque* q);
int deque_size(struct deque* q);

#endif
