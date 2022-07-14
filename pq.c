/* Priority-queue implementation using a pairing heap. All keys are doubles. */

#include <stdio.h>
#include <stdlib.h>
#include "pq.h"

struct pq_pair* pq_pair_create(double key, void* data)
{
    struct pq_pair* p = malloc(sizeof(struct pq_pair));
    p->key = key;
    p->data = data;
    return p;
}

struct pq_node
{
    int children;
    struct pq_pair* data;
    struct pq_node* parent;
    struct pq_node* left;
    struct pq_node* right;
};

struct pq_node** pq_create()
{
    struct pq_node** res = malloc(sizeof(struct pq_node*));
    *res = NULL;
    return res;
}

void destroy_tree(struct pq_node* h)
{
    if(!h)
        return;
    destroy_tree(h->left);
    destroy_tree(h->right);
    free(h->data->data);
    free(h->data);
    free(h);
}
void pq_destroy(struct pq_node** h)
{
    if(!h)
        return;
    destroy_tree(*h);
    free(h);
}

static void link(struct pq_node** h1, struct pq_node* h2)
{
    if(!*h1)
    {
        *h1 = h2;
        return;
    }
    if(h2->data->key < (*h1)->data->key)
    {
        struct pq_node* tmp = *h1;
        *h1 = h2;
        h2 = tmp;
    }
    if((*h1)->left)
    {
        (*h1)->left->parent = h2;
        h2->right = (*h1)->left;
    }
    (*h1)->left = h2;
    h2->parent = (*h1);
    ++(*h1)->children;
}

struct pq_pair* pq_find_min(struct pq_node** h)
{
    return h ? (*h)->data : NULL;
}

int pq_insert(struct pq_node** h, struct pq_pair* x)
{
    if(!h)
        return -1;
    struct pq_node* h2 = malloc(sizeof(struct pq_node));
    h2->children = 0;
    h2->data = x;
    x->node = h2;
    h2->parent = h2->left = h2->right = NULL;
    link(h, h2);
    return 0;
}

struct pq_pair* pq_delete_min(struct pq_node** h)
{
    if(!h || !*h)
        return NULL;
    struct pq_pair* res = (*h)->data;
    res->node = NULL;
    struct pq_node** forest = malloc((*h)->children * sizeof(struct pq_node*));
    int length = 0;
    for(struct pq_node* cur = (*h)->left; cur; cur = cur->right)
    {
        cur->parent = cur->parent->right = NULL;
        forest[length++] = cur;
    }
    free(*h);
    for(int i = 1; i < length; i += 2)
        link(forest + i - 1, forest[i]);
    for(int i = length - 1 - !(length & 1); i >= 2; i -= 2)
        link(forest + i - 2, forest[i]);
    *h = length ? forest[0] : NULL;
    free(forest);
    return res;
}

int pq_meld(struct pq_node** h1, struct pq_node** h2)
{
    if(!h1 || !h2)
        return -1;
    struct pq_node* other = *h2;
    free(h2);
    if(other)
        link(h1, other);
    return 0;
}

static void cut_tree(struct pq_node* h2)
{
    if(h2 == h2->parent->left)
    {
        h2->parent->left = h2->right;
        --h2->parent->children;
    }
    else
        h2->parent->right = h2->right;
    if(h2->right)
        h2->right->parent = h2->parent;
    h2->parent = h2->right = NULL;
}

int pq_decrease_key(struct pq_node** h, struct pq_pair* x, double new_key)
{
    if(!h || !x || !x->node)
        return -1;
    if(new_key > x->key)
        return -2;
    x->key = new_key;
    if(x->node != *h)
    {
        cut_tree(x->node);
        link(h, x->node);
    }
    return 0;
}

int pq_delete(struct pq_node** h, struct pq_pair* x)
{
    if(!h || !x || !x->node)
        return -1;
    if(x->node == *h)
        pq_delete_min(h);
    else
    {
        cut_tree(x->node);
        struct pq_node* h2 = x->node;
        pq_delete_min(&h2);
        link(h, h2);
    }
    return 0;
}

int pq_empty(struct pq_node** h)
{
    return !*h;
}

void pq_print(struct pq* q, void (*print)(void*))
{
    if(pq_empty(q))
    {
        printf("empty");
        return;
    }
    struct pq* npq = pq_create();
    while (!pq_empty(q))
    {
        struct pq_pair* cur = pq_delete_min(q);
        print(cur->data);
        pq_insert(npq, cur);
        if(pq_empty(q))
            break;
        putchar(' ');
    }
    *q = *npq;
    free(npq);
}
