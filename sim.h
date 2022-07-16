#ifndef SIM_H
#define SIM_H

// When this is defined, output is limited to time 999 ms.
#define LIMIT_LINES

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

struct stats
{
    int bursts;
    // Total times---divide by bursts for average.
    int burst;
    // Can calculate using other stats already found.
    int wait;
    // Rationale for calculation of total turnaround time:
    // Each process either (1) has not arrived, (2) is in IO, or (3) doing
    // something that would count as part of turnaround time. Thus, in the
    // loop, we subtract the current time whenever a process arrives or leaves
    // IO, and add it whenever the process switches out.
    int turnaround;

    int switches;
    int preemptions;

    int time;
};

void gen_processes(struct process* processes, int n, int tau_0);
void print_processes(struct process* processes, int n);

// Each simulation expects processes to be sorted by ascending start time.
struct stats* sim_fcfs(struct process* processes, int n, int tcs);
struct stats* sim_sjf(struct process* processes, int n, int tcs);
struct stats* sim_srt(struct process* processes, int n, int tcs);
struct stats* sim_rr(struct process* processes, int n, int tcs, int tslice);

#endif
