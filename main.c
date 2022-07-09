#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "exp.h"

#define NUM_ARGS 7
#define MAX_PROCESSES 26

struct burst
{
    int cpu;
    int io;
};
struct process
{
    int arrival;
    int tau_0;
    int num_bursts;
    struct burst* bursts;
    // To be modified by scheduling algorithms.
    int tau;
};

int main(int argc, char** argv)
{
#define S_(x) #x
#define S(x) S_(x)
    if(argc != NUM_ARGS + 1)
    {
        fprintf(stderr, "ERROR: expected " S(NUM_ARGS) " arguments, got %d\n", argc - 1);
        return EXIT_FAILURE;
    }
#define READ(type, name, arg, str, error, cond) \
    type name; \
    if(!sscanf(argv[arg], str, &name) || cond) \
    { \
        fprintf(stderr, "ERROR: expected " error ", got \"%s\"\n", argv[arg]); \
        return EXIT_FAILURE; \
    }
    READ(int, n, 1, "%d", "integer 1-" S(MAX_PROCESSES), n < 1 || n > MAX_PROCESSES);
    READ(long, seed, 2, "%ld", "long int", 0);
    READ(double, lambda, 3, "%lf", "double", 0);
    READ(int, threshold, 4, "%d", "int", 0);
    READ(int, tcs, 5, "%d", "even int", tcs & 1);
    READ(double, alpha, 6, "%lf", "double", 0);
    READ(int, tslice, 7, "%d", "int", 0);
#undef S
#undef S_
#undef ERR

    srand48(seed);
    set_exp_params(lambda, threshold);

    struct process processes[MAX_PROCESSES];
    for(int i = 0; i < n; ++i)
    {
        processes[i].arrival = floor(next_exp());
        processes[i].tau_0 = ceil(1 / lambda);
        processes[i].num_bursts = ceil(drand48() * 100);
        processes[i].bursts = malloc(num_bursts * sizeof(struct burst));
        for(int j = 0; ; ++i)
        {
            processes[i].bursts[j].cpu = ceil(next_exp());
            if(j >= processes[i].num_bursts - 1)
                break;
            processes[i].bursts[j].io = ceil(next_exp()) * 10;
        }
    }
}
