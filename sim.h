#ifndef SIM_H
#define SIM_H

struct burst
{
    int cpu;
    int io;
    // To be modified by scheduling algorithms.
    int cpu_left;
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
    int bursts_done;
};

void gen_processes(struct process* processes, int n, int tau_0);
void print_processes(struct process* processes, int n);

// Expects processes to be sorted by ascending start time.
void sim_fcfs(struct process* processes, int n, int tcs);
void sim_sjf(struct process* processes, int n, int tcs);
void sim_srt(struct process* processes, int n, int tcs);
void sim_rr(struct process* processes, int n, int tcs, int tslice);

#endif
