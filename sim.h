#ifndef SIM_H
#define SIM_H

struct burst
{
    int cpu;
    int io;
};
struct process
{
    char* name;
    int arrival;
    int tau_0;
    int num_bursts;
    struct burst* bursts;
    // To be modified by scheduling algorithms.
    int tau;
};

void gen_processes(struct process* processes, int n, int tau_0);
void print_processes(struct process* processes, int n);

#endif
