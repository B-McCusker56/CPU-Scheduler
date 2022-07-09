#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "exp.h"


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
    if(argc != 8)
    {
        fprintf(stderr, "ERROR: expected 7 arguments, got %d\n", argc - 1);
        return EXIT_FAILURE;
    }
    int n;
    if(!sscanf(argv[1], "%d", &n) || n < 1 || n > 26)
    {
        fprintf(stderr, "ERROR: expected integer 1-26, got \"%s\"\n", argv[1]);
        return EXIT_FAILURE;
    }
    long seed;
    if(!sscanf(argv[2], "%ld", &seed))
    {
        fprintf(stderr, "ERROR: expected long int, got \"%s\"\n", argv[2]);
        return EXIT_FAILURE;
    }
    double lambda;
    if(!sscanf(argv[3], "%lf", &lambda))
    {
        fprintf(stderr, "ERROR: expected double, got \"%s\"\n", argv[3]);
        return EXIT_FAILURE;
    }
    int threshold;
    if(!sscanf(argv[4], "%d", &threshold))
    {
        fprintf(stderr, "ERROR: expected int, got \"%s\"\n", argv[4]);
        return EXIT_FAILURE;
    }
    int tcs;
    if(!sscanf(argv[5], "%d", &tcs) || tcs & 1)
    {
        fprintf(stderr, "ERROR: expected even int, got \"%s\"\n", argv[5]);
        return EXIT_FAILURE;
    }
    double alpha;
    if(!sscanf(argv[6], "%lf", &alpha))
    {
        fprintf(stderr, "ERROR: expected double, got \"%s\"\n", argv[6]);
        return EXIT_FAILURE;
    }
    int tslice;
    if(!sscanf(argv[7], "%d", &tslice))
    {
        fprintf(stderr, "ERROR: expected int, got \"%s\"\n", argv[7]);
        return EXIT_FAILURE;
    }

    srand48(seed);
    set_exp_params(lambda, threshold);

    struct process processes[26];
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
