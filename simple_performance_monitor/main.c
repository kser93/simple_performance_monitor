#include <stdio.h>

#include <stdbool.h>
#include <float.h>

#include <sys/sysinfo.h>

#include <unistd.h>

#define CPU_CORE_NAME_LEN 13

bool is_verbose = true;
//bool foreach_cpu_core = true;

double ram_load();
double cpu_load();

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

struct cpu_times_t get_cpu_times();

int main()
{
	if (is_verbose) printf("hello from simple_performance_monitor!\n");
	double ram_ld = ram_load();
	double cpu_ld = cpu_load();
	return 0;
}

double ram_load()
{
	struct sysinfo info;
	int code = sysinfo(&info);
	if (code < 0)
		return -DBL_MAX;
	double load = (double)(info.totalram - info.freeram) / info.totalram;
	if (is_verbose)
	{
		printf("free RAM is %lu bytes\n", info.freeram);
		printf("total RAM is %lu bytes\n", info.totalram);
		printf("RAM usage is %0.2f %\n", load * 100);
	}
	return load;
}

double cpu_load()
{
	int nproc = get_nprocs();

	struct cpu_times_t t0 = get_cpu_times();
	sleep(1);
	struct cpu_times_t t1 = get_cpu_times();
	
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

	unsigned long effective_load = diff.user + diff.nice + diff.system + diff.irq +
		diff.softirq + diff.steal + diff.guest + diff.guest_nice;
	unsigned long idle_load = diff.idle; // diff.iowait is not using
	double load = (double)(effective_load) / (effective_load + idle_load);

	if (is_verbose)
	{
		printf("procs available: %d\n", nproc);
		printf("CPU usage is %0.2f %\n", load * 100);
	}

	return load;
}

struct cpu_times_t get_cpu_times()
{
	struct cpu_times_t cpu_times;
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