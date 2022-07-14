#include <limits.h>
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
            processes[i].bursts[j].cpu_left = 0;
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
static int rr(struct process* processes, int n, int tcs, int tslice)
{
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
    do { printf("time %dms: " msg " [Q: ", time, ##__VA_ARGS__); \
        deque_print(q, print_process_name); \
        puts("]"); } \
    while(0)
#define REWIND() \
    do { --time; \
        if(switch_in < tcs) ++switch_in; } \
    while(0)
    for(; p < n || using_cpu || deque_size(q) || !pq_empty(ioq); ++time)
    {
        // CPU-burst completions.
        if(using_cpu && time - cpu_time
                     == using_cpu->bursts[using_cpu->bursts_done].cpu_left)
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
                double key = io + using_cpu->name[0] / 100.0;
                struct pq_pair* item = pq_pair_create(key, using_cpu);
                pq_insert(ioq, item);
            }
            else
                PRINT_EVENT("Process %s terminated", using_cpu->name);
            using_cpu->bursts[using_cpu->bursts_done].cpu_left = 0;
            using_cpu = NULL;
            switch_out = tcs;
        }
        // Time-slice expirations.
        if(using_cpu && time - cpu_time == tslice)
        {
            cpu_time = time;
            using_cpu->bursts[using_cpu->bursts_done].cpu_left -= tslice;
            if(deque_size(q))
            {
                PRINT_EVENT("Time slice expired; process %s preempted with %dms"
                            " remaining", using_cpu->name,
                            using_cpu->bursts[using_cpu->bursts_done].cpu_left);
                deque_push_back(q, using_cpu);
                using_cpu = NULL;
                switch_out = tcs;
            }
            else
                PRINT_EVENT("Time slice expired; no preemption because ready "
                            "queue is empty");
        }
        // Processes starting to use the CPU.
        if(!using_cpu && !switch_out && deque_size(q))
        {
            if(!switch_in)
            {
                using_cpu = deque_pop_front(q);
                cpu_time = time;
                struct burst* b = &using_cpu->bursts[using_cpu->bursts_done];
                if(!b->cpu_left)
                {
                    b->cpu_left = b->cpu;
                    PRINT_EVENT("Process %s started using the CPU for %dms "
                                "burst", using_cpu->name, b->cpu);
                }
                else
                    PRINT_EVENT("Process %s started using the CPU for "
                                "remaining %dms of %dms burst",
                                using_cpu->name, b->cpu_left, b->cpu);
                switch_in = tcs;
            }
            else
                --switch_in;
        }
        // I/O-burst completions.
        if(!pq_empty(ioq) && floor(pq_find_min(ioq)->key) == time)
        {
            struct pq_pair* item = pq_delete_min(ioq);
            struct process* proc = item->data;
            ++proc->bursts_done;
            deque_push_back(q, proc);
            PRINT_EVENT("Process %s completed I/O; added to ready queue",
                        proc->name);
            free(item);
            // Re-simulate one ms, since more processes may leave IO at the same
            // time. Also, the CPU should realize immediately that something new
            // is in the queue.
            REWIND();
            continue;
        }
        // New process arrivals.
        if(p < n && processes[p].arrival == time)
        {
            processes[p].bursts_done = 0;
            deque_push_back(q, &processes[p]);
            PRINT_EVENT("Process %s arrived; added to ready queue",
                        processes[p].name);
            ++p;
            // See reasoning above.
            REWIND();
            continue;
        }
        if(switch_out)
            --switch_out;
    }
#undef PRINT_EVENT
#undef REWIND
    deque_destroy(q);
    pq_destroy(ioq);
    return time + tcs - 1;
}
void sim_fcfs(struct process* processes, int n, int tcs)
{
    puts("time 0ms: Simulator started for FCFS [Q: empty]");
    int time = rr(processes, n, tcs, INT_MAX);
    printf("time %dms: Simulator ended for FCFS [Q: empty]\n", time);
}
void sim_rr(struct process* processes, int n, int tcs, int tslice)
{
    printf("time 0ms: Simulator started for RR with time slice %dms [Q: empty"
           "]\n", tslice);
    int time = rr(processes, n, tcs, tslice);
    printf("time %dms: Simulator ended for RR [Q: empty]\n", time);
}
