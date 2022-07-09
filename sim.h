#ifndef SIM_H
#define SIM_H

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

void gen_processes(struct process processes[MAX_PROCESSES], int n, int tau_0);

#endif
