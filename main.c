#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "exp.h"
#include "sim.h"

#define NUM_ARGS 7
#define MAX_PROCESSES 26

static int cmpprocess(const void* _p1, const void* _p2)
{
    const struct process* p1 = _p1;
    const struct process* p2 = _p2;
    int arrival = p1->arrival - p2->arrival;
    return arrival ? arrival : p1->name[0] - p2->name[0];
}

int main(int argc, char** argv)
{
#define S_(x) #x
#define S(x) S_(x)
    if(argc != NUM_ARGS + 1)
    {
        fprintf(stderr, "ERROR: expected " S(NUM_ARGS) " arguments, got %d\n",
                argc - 1);
        return EXIT_FAILURE;
    }
#define READ(type, name, arg, str, error, cond) \
    type name; \
    if(!sscanf(argv[arg], str, &name) || cond) \
    { \
        fprintf(stderr, "ERROR: expected " error ", got \"%s\"\n", argv[arg]); \
        return EXIT_FAILURE; \
    }
    READ(int, n, 1, "%d", "integer 1-" S(MAX_PROCESSES),
         n < 1 || n > MAX_PROCESSES);
    READ(long, seed, 2, "%ld", "long int", 0);
    READ(double, lambda, 3, "%lf", "double", 0);
    READ(int, threshold, 4, "%d", "int", 0);
    READ(int, tcs, 5, "%d", "even int", tcs & 1);
    READ(double, alpha, 6, "%lf", "double", 0);
    READ(int, tslice, 7, "%d", "int", 0);
#undef S
#undef S_
#undef READ

    srand48(seed);
    set_exp_params(lambda, threshold);

    struct process processes[MAX_PROCESSES];
    gen_processes(processes, n, ceil(1 / lambda));
    print_processes(processes, n);

    qsort(processes, n, sizeof(struct process), cmpprocess);
    print_processes(processes, n);

    for(int i = 0; i < n; ++i)
        free(processes[i].bursts);
}
