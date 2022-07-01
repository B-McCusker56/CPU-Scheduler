/* Priority-queue implementation using a pairing heap. All keys are doubles. */

#ifndef PQ_H
#define PQ_H

#define pq pq_node*

struct pq_pair
{
    double key;
    void* data;
    // Should be NULL or undefined until set by pq methods.
    struct pq_node* node;
};
struct pq_pair* pq_pair_create(double key, void* data);

struct pq* pq_create();
void pq_destroy(struct pq* h);
struct pq_pair* pq_find_min(struct pq* h);
int pq_insert(struct pq* h, struct pq_pair* x);
struct pq_pair* pq_delete_min(struct pq* h);
int pq_meld(struct pq* h1, struct pq* h2);
int pq_decrease_key(struct pq* h, struct pq_pair* x, double new_key);
int pq_delete(struct pq* h, struct pq_pair* x);
int pq_empty(struct pq* h);

#endif
