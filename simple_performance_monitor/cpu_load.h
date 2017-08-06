#ifndef CPU_LOAD_H
#define CPU_LOAD_H

#include <stdbool.h>
#include <sys/time.h>
#include <time.h>

#define CPU_CORE_NAME_LEN 13 // longest name of /proc/stat name field
/* (cat /proc/stat | cut -d " " -f1 | wc -L) => 13 (len("procs_running")) */
#define MOVING_WINDOW_LEN_SEC 2
#define USEC_PER_SEC 1000000

extern const bool is_verbose;

struct cpu_times_t
{
	char name[CPU_CORE_NAME_LEN]; // Name of current cpu core.
	unsigned long user; // Time spent in user mode.
	unsigned long nice; // Time spent in user mode with low priority (nice).
	unsigned long system; // Time spent in system mode.
	unsigned long idle; // Time spent in the idle task.
	unsigned long iowait; // Time waiting for I/O to complete.
	unsigned long irq; // Time servicing interrupts.
	unsigned long softirq; // Time servicing softirqs.
	unsigned long steal; // Stolen time (spent in other OS when running in a virtualized environment).
	unsigned long guest; // Time spent running a virtual CPU for guest OS under the control of the Linux kernel.
	unsigned long guest_nice; // Time spent running a niced guest (virtual CPU for guest OS under the control of the Linux kernel).
};

struct cpu_times_node_t
{
	struct cpu_times_t cpu_times;
	struct cpu_times_node_t *next;
	struct timeval tv;
};

struct cpu_times_list_t
{
	struct cpu_times_node_t *root;
	struct cpu_times_node_t *last;
	size_t size;
};

double cpu_load(struct cpu_times_list_t *cpu_times_list);

#endif // !CPU_LOAD_H
