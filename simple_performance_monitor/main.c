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
	struct cpu_times_t cpu_times0, cpu_times1;
	FILE *stats = fopen("/proc/stat", "r");
	fscanf(
		stats, "%s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
		&(cpu_times0.name),
		&(cpu_times0.user),
		&(cpu_times0.nice),
		&(cpu_times0.system),
		&(cpu_times0.idle),
		&(cpu_times0.iowait),
		&(cpu_times0.irq),
		&(cpu_times0.softirq),
		&(cpu_times0.steal),
		&(cpu_times0.guest),
		&(cpu_times0.guest_nice)
	);
	fclose(stats);

	sleep(1);

	stats = fopen("/proc/stat", "r");
	fscanf(
		stats, "%s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
		&(cpu_times1.name),
		&(cpu_times1.user),
		&(cpu_times1.nice),
		&(cpu_times1.system),
		&(cpu_times1.idle),
		&(cpu_times1.iowait),
		&(cpu_times1.irq),
		&(cpu_times1.softirq),
		&(cpu_times1.steal),
		&(cpu_times1.guest),
		&(cpu_times1.guest_nice)
	);
	fclose(stats);
	
	struct cpu_times_t cpu_times_diff = 
	{
		"diff",
		cpu_times1.user - cpu_times0.user,
		cpu_times1.nice - cpu_times0.nice,
		cpu_times1.system - cpu_times0.system,
		cpu_times1.idle - cpu_times0.idle,
		cpu_times1.iowait - cpu_times0.iowait,
		cpu_times1.irq - cpu_times0.irq,
		cpu_times1.softirq - cpu_times0.softirq,
		cpu_times1.steal - cpu_times0.steal,
		cpu_times1.guest - cpu_times0.guest,
		cpu_times1.guest_nice - cpu_times0.guest_nice
	};

	unsigned long effective_load = cpu_times_diff.user + cpu_times_diff.nice + cpu_times_diff.system + cpu_times_diff.irq +
		cpu_times_diff.softirq + cpu_times_diff.steal + cpu_times_diff.guest + cpu_times_diff.guest_nice;
	unsigned long idle_load = cpu_times_diff.idle; // cpu_times_diff.iowait is not using
	double load = (double)(effective_load) / (effective_load + idle_load);

	if (is_verbose)
	{
		printf("procs available: %d\n", nproc);
		printf("CPU usage is %0.2f %\n", load * 100);
	}

	return load;
}
