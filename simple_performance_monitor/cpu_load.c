#include "cpu_load.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#define CPU_CORE_NAME_LEN 13 // longest name of /proc/stat name field
/* (cat /proc/stat | cut -d " " -f1 | wc -L) => 13 (len("procs_running")) */

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

static struct cpu_times_list_t cpu_times_list = { NULL, NULL, 0 };

struct cpu_times_t get_cpu_times();
struct cpu_times_t cpu_times_diff(struct cpu_times_t t0, struct cpu_times_t t1);
struct cpu_times_node_t *push_back_cpu_times_list(struct cpu_times_list_t *cpu_times_list, struct cpu_times_t new_cpu_times);
struct cpu_times_node_t *append_cpu_times(struct cpu_times_list_t *cpu_times_list);
struct cpu_times_t pop_front_cpu_times_list(struct cpu_times_list_t *cpu_times_list);
double cpu_load();
struct timeval timeval_sub(struct timeval a, struct timeval b);


double cpu_load()
{
	struct cpu_times_node_t *node1 = append_cpu_times(&cpu_times_list);
	struct cpu_times_node_t *node0 = cpu_times_list.root;

	struct cpu_times_t diff = cpu_times_diff(node0->cpu_times, node1->cpu_times);

	unsigned long effective_load = diff.user + diff.nice + diff.system + diff.irq +
		diff.softirq + diff.steal + diff.guest + diff.guest_nice;
	unsigned long idle_load = diff.idle; // diff.iowait is not using
	unsigned long full_load = effective_load + idle_load;
	double load = 0.0;
	if (full_load)
	{
		load = (double)(effective_load) / (effective_load + idle_load);
	}
	struct timeval tv_diff = timeval_sub(node1->tv, node0->tv);
	if (tv_diff.tv_sec > MOVING_WINDOW_LEN_SEC)
	{
		pop_front_cpu_times_list(&cpu_times_list);
	}

	if (is_verbose)
	{
		printf("CPU usage is %0.2f %\n", load * 100);
	}

	return load;
}

struct cpu_times_t get_cpu_times()
{
	struct cpu_times_t cpu_times = { 0 };
	FILE *stats = fopen("/proc/stat", "r");
	fscanf(
		stats, "%s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
		&(cpu_times.name),
		&(cpu_times.user),
		&(cpu_times.nice),
		&(cpu_times.system),
		&(cpu_times.idle),
		&(cpu_times.iowait),
		&(cpu_times.irq),
		&(cpu_times.softirq),
		&(cpu_times.steal),
		&(cpu_times.guest),
		&(cpu_times.guest_nice)
	);
	fclose(stats);
	return cpu_times;
}

struct cpu_times_t cpu_times_diff(struct cpu_times_t t0, struct cpu_times_t t1)
{
	struct cpu_times_t diff =
	{
		"diff",
		t1.user - t0.user,
		t1.nice - t0.nice,
		t1.system - t0.system,
		t1.idle - t0.idle,
		t1.iowait - t0.iowait,
		t1.irq - t0.irq,
		t1.softirq - t0.softirq,
		t1.steal - t0.steal,
		t1.guest - t0.guest,
		t1.guest_nice - t0.guest_nice
	};

	return diff;
}

struct cpu_times_node_t *push_back_cpu_times_list(struct cpu_times_list_t *cpu_times_list, struct cpu_times_t new_cpu_times)
{
	struct cpu_times_node_t *new_node = (struct cpu_times_node_t *)malloc(sizeof(struct cpu_times_node_t));
	new_node->cpu_times = new_cpu_times;
	gettimeofday(&(new_node->tv), NULL);
	new_node->next = NULL;

	(cpu_times_list->size)++;

	if (cpu_times_list->root == NULL)
	{
		cpu_times_list->root = new_node;
		cpu_times_list->last = new_node;
		return new_node;
	}

	cpu_times_list->last->next = new_node;
	cpu_times_list->last = cpu_times_list->last->next;
	return new_node;
}

struct cpu_times_node_t *append_cpu_times(struct cpu_times_list_t *cpu_times_list)
{
	return push_back_cpu_times_list(cpu_times_list, get_cpu_times());
}

struct cpu_times_t pop_front_cpu_times_list(struct cpu_times_list_t *cpu_times_list)
{
	struct cpu_times_t result;
	result.name[0] = '\0';

	if (cpu_times_list->size == 0 || cpu_times_list->root == NULL)
	{
		return result;
	}

	(cpu_times_list->size)--;
	result = cpu_times_list->root->cpu_times;

	struct cpu_times_node_t *temp = cpu_times_list->root;

	if (cpu_times_list->root->next)
	{
		cpu_times_list->root = cpu_times_list->root->next;
	}
	else
	{
		cpu_times_list->root = NULL;
		cpu_times_list->last = NULL;
	}

	free((void *)temp);
	temp = NULL;

	return result;
}

struct timeval timeval_sub(struct timeval a, struct timeval b)
{
	struct timeval result = { 0, 0 };
	if (b.tv_sec >= a.tv_sec)
	{
		return result;
	}
	result.tv_sec = a.tv_sec - b.tv_sec;
	result.tv_usec = a.tv_usec - b.tv_usec;
	if (result.tv_usec < 0)
	{
		--result.tv_sec;
		result.tv_usec += USEC_PER_SEC;
	}
	return result;
}
