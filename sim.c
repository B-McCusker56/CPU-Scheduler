#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "exp.h"
#include "sim.h"

void gen_processes(struct process processes[MAX_PROCESSES], int n, int tau_0)
{
    for(int i = 0; i < n; ++i)
    {
        processes[i].arrival = floor(next_exp());
        processes[i].tau_0 = tau_0;
        processes[i].num_bursts = ceil(drand48() * 100);
        processes[i].bursts =
            malloc(processes[i].num_bursts * sizeof(struct burst));
        for(int j = 0; ; ++j)
        {
            processes[i].bursts[j].cpu = ceil(next_exp());
            if(j >= processes[i].num_bursts - 1)
                break;
            processes[i].bursts[j].io = ceil(next_exp()) * 10;
        }
    }
}

void print_processes(struct process processes[MAX_PROCESSES], int n)
{
    for(int i = 0; i < n; ++i)
    {
        printf("Process %c: arrival time %dms; tau %dms; %d CPU bursts:\n",
               i + 65, processes[i].arrival, processes[i].tau_0,
               processes[i].num_bursts);
        for(int j = 0; ; ++j)
        {
            printf("--> CPU burst %dms", processes[i].bursts[j].cpu);
            if(j >= processes[i].num_bursts - 1)
                break;
            printf(" --> I/O burst %dms\n", processes[i].bursts[j].io);
        }
        putchar('\n');
    }
}
