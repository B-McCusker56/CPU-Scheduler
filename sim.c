#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "deque.h"
#include "exp.h"
#include "sim.h"

static char* PROCESS_NAMES = "A\0B\0C\0D\0E\0F\0G\0H\0I\0J\0K\0L\0M\0N\0O\0P\0Q"
                             "\0R\0S\0T\0U\0V\0W\0X\0Y\0Z";

void gen_processes(struct process* processes, int n, int tau_0)
{
    for(int i = 0; i < n; ++i)
    {
        processes[i].name = &PROCESS_NAMES[i << 1];
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

void print_processes(struct process* processes, int n)
{
    for(int i = 0; i < n; ++i)
    {
        printf("Process %s: arrival time %dms; tau %dms; %d CPU bursts:\n",
               processes[i].name, processes[i].arrival, processes[i].tau_0,
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
