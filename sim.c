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
static void print_process_name(void* i)
{
    printf("%s", ((struct process*) i)->name);
}

// Constants for the state of the CPU.
enum state
{
    WAITING,
    RUNNING,
    // Intermediate state to remove the process from the ready queue and load
    // into the CPU before beginning a context switch.
    LOADING_PROCESS,
    SWITCHING_IN,
    SWITCHING_OUT,
    // A special state which has the same meaning as SWITCHING_OUT, except that
    // the process is moved back to the ready queue instead of into IO.
    PREEMPTED,
};
static int rr(struct process* processes, int n, int tcs, int tslice)
{
    int time = processes[0].arrival;
    // Current CPU state.
    enum state state = WAITING;
    // Current process using the CPU.
    // Only valid when the CPU is in the RUNNING or SWITCHING_IN states.
    struct process* running;
    // Next process to arrive.
    int p = 0;
    // Ready queue.
    struct deque* q = deque_create();
    // Heap for holding processes in IO.
    struct pq* ioq = pq_create();
    // Time at which the CPU started running the last process.
    int cpu_time;
    // Time at which the last context switch began.
    int switch_time;

#define PRINT_EVENT(msg, ...) \
    do { printf("time %dms: " msg " [Q: ", time, ##__VA_ARGS__); \
        deque_print(q, print_process_name); \
        puts("]"); } \
    while(0)

    // Run until (1) all processes have arrived, (2) the CPU is done working,
    // (3) the ready queue is empty, and (4) all processes have finished IO.
    while(p < n || state != WAITING || deque_size(q) || !pq_empty(ioq))
    {
        switch(state)
        {
        case RUNNING:; // required empty statement for a declaration next line
            struct burst* current_burst = &running->bursts[running->bursts_done];
            // Check for CPU-burst completion.
            if(time - cpu_time == current_burst->cpu_left)
            {
                current_burst->cpu_left = 0;
                switch_time = time;
                if(running->bursts_done < running->num_bursts - 1)
                {
                    int to_go = running->num_bursts - running->bursts_done - 1;
                    PRINT_EVENT("Process %s completed a CPU burst; %d burst%s"
                                " to go", running->name, to_go,
                                to_go == 1 ? "" : "s");
                    PRINT_EVENT("Process %s switching out of CPU; will block on"
                                " I/O until time %dms", running->name,
                                time + (tcs >> 1) + current_burst->io);
                }
                else
                    PRINT_EVENT("Process %s terminated", running->name);
                state = SWITCHING_OUT;
            }
            // Time-slice expirations.
            else if(time - cpu_time == tslice)
            {
                current_burst->cpu_left -= tslice;
                cpu_time = time;
                if(deque_size(q))
                {
                    switch_time = time;
                    PRINT_EVENT("Time slice expired; process %s preempted with"
                                " %dms remaining", running->name,
                                current_burst->cpu_left);
                    state = PREEMPTED;
                }
                else
                    PRINT_EVENT("Time slice expired; no preemption because"
                                " ready queue is empty");
            }
            break;
        case WAITING:
            // Processes starting to use the CPU.
            if(deque_size(q))
            {
                switch_time = time;
                state = LOADING_PROCESS;
            }
            break;
        case PREEMPTED:
            if(time - switch_time == tcs >> 1)
            {
                deque_push_back(q, running);
                state = WAITING;
                continue;
            }
            break;
        case SWITCHING_OUT:
            if(time - switch_time == tcs >> 1)
            {
                // Make sure process has not terminated.
                if(running->bursts_done < running->num_bursts - 1)
                {
                    int io = running->bursts[running->bursts_done].io;
                    // Add the process's name as a decimal in the key, so it
                    // sorts by name secondarily.
                    double key = time + io + running->name[0] / 100.0;
                    struct pq_pair* item = pq_pair_create(key, running);
                    pq_insert(ioq, item);
                }
                // Re-simulate one ms whenever the state becomes WAITING, since
                // the CPU can recognize instantly whether a process has become
                // available.
                state = WAITING;
                continue;
            }
            break;
        case LOADING_PROCESS:
            running = deque_pop_front(q);
            state = SWITCHING_IN;
            // fall through
        case SWITCHING_IN:
            if(time - switch_time == tcs >> 1)
            {
                cpu_time = time;
                struct burst* b = &running->bursts[running->bursts_done];
                if(!b->cpu_left)
                {
                    b->cpu_left = b->cpu;
                    PRINT_EVENT("Process %s started using the CPU for %dms "
                                "burst", running->name, b->cpu);
                }
                else
                    PRINT_EVENT("Process %s started using the CPU for "
                                "remaining %dms of %dms burst",
                                running->name, b->cpu_left, b->cpu);
                state = RUNNING;
            }
        }

        // IO-burst completions.
        if(!pq_empty(ioq) && floor(pq_find_min(ioq)->key) == time)
        {
            struct pq_pair* item = pq_delete_min(ioq);
            struct process* proc = item->data;
            free(item);
            ++proc->bursts_done;
            deque_push_back(q, proc);
            PRINT_EVENT("Process %s completed I/O; added to ready queue",
                        proc->name);
            // Re-simulate one ms, since more processes may leave IO at the same
            // time. Also, the CPU should realize immediately that something new
            // is in the queue, as above.
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
            continue;
        }

        ++time;
    }
#undef PRINT_EVENT
    return time;
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

// Although this and the previous algorithm have many similarities, there
// were enough differences to justify splitting them into two functions.
void sim_sjf(struct process* processes, int n, int tcs)
{
    puts("time 0ms: Simulator started for SJF [Q: empty]");
    int time = processes[0].arrival;
    // Current CPU state.
    enum state state = WAITING;
    // Current process using the CPU.
    // Only valid when the CPU is in the RUNNING or SWITCHING_IN states.
    struct process* running;
    // Next process to arrive.
    int p = 0;
    // Ready queue.
    struct pq* q = pq_create();
    // Heap for holding processes in IO.
    struct pq* ioq = pq_create();
    // Time at which the CPU started running the last process.
    int cpu_time;
    // Time at which the last context switch began.
    int switch_time;

#define PRINT_EVENT(msg, ...) \
    do { printf("time %dms: " msg " [Q: ", time, ##__VA_ARGS__); \
        pq_print(q, print_process_name); \
        puts("]"); } \
    while(0)

    // Run until (1) all processes have arrived, (2) the CPU is done working,
    // (3) the ready queue is empty, and (4) all processes have finished IO.
    while(p < n || state != WAITING || !pq_empty(q) || !pq_empty(ioq))
    {
        switch(state)
        {
        case RUNNING:; // required empty statement for a declaration next line
            struct burst* current_burst = &running->bursts[running->bursts_done];
            // Check for CPU-burst completion.
            if(time - cpu_time == current_burst->cpu_left)
            {
                current_burst->cpu_left = 0;
                switch_time = time;
                if(running->bursts_done < running->num_bursts - 1)
                {
                    int to_go = running->num_bursts - running->bursts_done - 1;
                    PRINT_EVENT("Process %s (tau %dms) completed a CPU burst; "
                                "%d burst%s to go", running->name, running->tau,
                                to_go, to_go == 1 ? "" : "s");
                    int new_tau = next_tau(current_burst->cpu, running->tau);
                    PRINT_EVENT("Recalculated tau for process %s: old tau %dms;"
                                " new tau %dms", running->name, running->tau,
                                new_tau);
                    running->tau = new_tau;
                    PRINT_EVENT("Process %s switching out of CPU; will block on"
                                " I/O until time %dms", running->name,
                                time + (tcs >> 1) + current_burst->io);
                }
                else
                    PRINT_EVENT("Process %s terminated", running->name);
                state = SWITCHING_OUT;
            }
            break;
        case WAITING:
            // Processes starting to use the CPU.
            if(!pq_empty(q))
            {
                switch_time = time;
                state = LOADING_PROCESS;
            }
            break;
        case SWITCHING_OUT:
            if(time - switch_time == tcs >> 1)
            {
                // Make sure process has not terminated.
                if(running->bursts_done < running->num_bursts - 1)
                {
                    int io = running->bursts[running->bursts_done].io;
                    // Add the process's name as a decimal in the key, so it
                    // sorts by name secondarily.
                    double key = time + io + running->name[0] / 100.0;
                    struct pq_pair* item = pq_pair_create(key, running);
                    pq_insert(ioq, item);
                }
                // Re-simulate one ms whenever the state becomes WAITING, since
                // the CPU can recognize instantly whether a process has become
                // available.
                state = WAITING;
                continue;
            }
            break;
        case LOADING_PROCESS:; // see case RUNNING above
            struct pq_pair* next = pq_delete_min(q);
            running = next->data;
            free(next);
            state = SWITCHING_IN;
            // fall through
        case SWITCHING_IN:
            if(time - switch_time == tcs >> 1)
            {
                cpu_time = time;
                struct burst* b = &running->bursts[running->bursts_done];
                b->cpu_left = b->cpu;
                PRINT_EVENT("Process %s (tau %dms) started using the CPU for"
                            " %dms burst", running->name, running->tau, b->cpu);
                state = RUNNING;
            }
        }

        // IO-burst completions.
        if(!pq_empty(ioq) && floor(pq_find_min(ioq)->key) == time)
        {
            struct pq_pair* item = pq_delete_min(ioq);
            struct process* proc = item->data;
            free(item);
            ++proc->bursts_done;
            struct pq_pair* proc_pair =
                pq_pair_create(proc->tau + proc->name[0] / 100.0, proc);
            pq_insert(q, proc_pair);
            PRINT_EVENT("Process %s (tau %dms) completed I/O; added to ready"
                        " queue", proc->name, proc->tau);
            // Re-simulate one ms, since more processes may leave IO at the same
            // time. Also, the CPU should realize immediately that something new
            // is in the queue, as above.
            continue;
        }
        // New process arrivals.
        if(p < n && processes[p].arrival == time)
        {
            struct process* proc = &processes[p];
            proc->bursts_done = 0;
            proc->tau = proc->tau_0;
            struct pq_pair* proc_pair =
                pq_pair_create(proc->tau + proc->name[0] / 100.0, proc);
            pq_insert(q, proc_pair);
            PRINT_EVENT("Process %s (tau %dms) arrived; added to ready queue",
                        proc->name, proc->tau);
            ++p;
            // See reasoning above.
            continue;
        }

        ++time;
    }
#undef PRINT_EVENT
    pq_destroy(q);
    pq_destroy(ioq);
    printf("time %dms: Simulator ended for SJF [Q: empty]\n", time);
}
