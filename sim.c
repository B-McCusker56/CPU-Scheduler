#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "deque.h"
#include "exp.h"
#include "pq.h"
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
        printf("Process %s: arrival time %dms; tau %dms; %d CPU burst%s:\n",
               processes[i].name, processes[i].arrival, processes[i].tau_0,
               processes[i].num_bursts,
               processes[i].num_bursts == 1 ? "" : "s");
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

// Helper function to pass to deque_print().
void print_process_name(void* i)
{
    printf("%s", ((struct process*) i)->name);
}
void sim_fcfs(struct process* processes, int n, int tcs)
{
    puts("time 0ms: Simulator started for FCFS [Q: empty]");
    int time = processes[0].arrival;
    struct process* using_cpu = NULL;
    struct deque* q = deque_create();
    struct pq* ioq = pq_create();
    int cpu_time;
    tcs >>= 1;
    int switch_in = tcs;
    int switch_out = 0;
    int p = 0;
#define PRINT_EVENT(msg, ...) \
    do { printf("time %dms: " msg " [Q: ", time, __VA_ARGS__); \
        deque_print(q, print_process_name); \
        puts("]"); } \
    while(0)
#define REWIND() \
    do { --time; \
        if(switch_in < tcs) ++switch_in; if(switch_out) ++switch_out; } \
    while(0)
    for(; p < n || using_cpu || deque_size(q) || !pq_empty(ioq); ++time)
    {
        // CPU burst completions.
        if(using_cpu && time - cpu_time
                     == using_cpu->bursts[using_cpu->bursts_done].cpu)
        {
            if(using_cpu->bursts_done < using_cpu->num_bursts - 1)
            {
                int to_go = using_cpu->num_bursts - using_cpu->bursts_done - 1;
                PRINT_EVENT("Process %s completed a CPU burst; %d burst%s to "
                            "go", using_cpu->name, to_go,
                            to_go == 1 ? "" : "s");
                int io = time + tcs
                       + using_cpu->bursts[using_cpu->bursts_done].io;
                PRINT_EVENT("Process %s switching out of CPU; will block on "
                            "I/O until time %dms", using_cpu->name, io);
                struct pq_pair* item = pq_pair_create(io, using_cpu);
                pq_insert(ioq, item);
            }
            else
                PRINT_EVENT("Process %s terminated", using_cpu->name);
            using_cpu = NULL;
            switch_out = tcs;
        }
        // Processes starting to use the CPU.
        if(!using_cpu && !switch_out && deque_size(q))
        {
            if(!switch_in)
            {
                using_cpu = deque_pop_front(q);
                cpu_time = time;
                PRINT_EVENT("Process %s started using the CPU for %dms burst",
                            using_cpu->name,
                            using_cpu->bursts[using_cpu->bursts_done].cpu);
                switch_in = tcs;
            }
            else
                --switch_in;
        }
        if(switch_out)
            --switch_out;
        // I/O burst completions.
        if(!pq_empty(ioq) && pq_find_min(ioq)->key == time)
        {
            struct pq_pair* item = pq_delete_min(ioq);
            struct process* proc = item->data;
            ++proc->bursts_done;
            deque_push_back(q, proc);
            PRINT_EVENT("Process %s completed I/O; added to ready queue",
                        proc->name);
            free(item);
            REWIND();
        }
        // New process arrivals.
        if(p < n && processes[p].arrival == time)
        {
            processes[p].bursts_done = 0;
            deque_push_back(q, &processes[p]);
            PRINT_EVENT("Process %s arrived; added to ready queue",
                        processes[p].name);
            ++p;
            // Re-simulate one ms, since more processes may arrive at the same
            // time. Also, the CPU should realize immediately that something new
            // is in the queue.
            REWIND();
        }
    }
#undef PRINT_EVENT
#undef REWIND
    deque_destroy(q);
    pq_destroy(ioq);
    printf("time %dms: Simulator ended for FCFS [Q: empty]\n", time + tcs - 1);
}
