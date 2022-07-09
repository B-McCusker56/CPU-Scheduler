#include <math.h>
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
        processes[i].bursts = malloc(processes[i].num_bursts * sizeof(struct burst));
        for(int j = 0; ; ++i)
        {
            processes[i].bursts[j].cpu = ceil(next_exp());
            if(j >= processes[i].num_bursts - 1)
                break;
            processes[i].bursts[j].io = ceil(next_exp()) * 10;
        }
    }
}
