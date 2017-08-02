#include <stdio.h>

#include <stdbool.h>
#include <float.h>
#include <sys/sysinfo.h>

bool is_verbose = true;

double ram_load();

int main()
{
	if (is_verbose) printf("hello from simple_performance_monitor!\n");
	double ram_ld = ram_load();
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