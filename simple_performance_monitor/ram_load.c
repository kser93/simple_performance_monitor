#include "ram_load.h"

#include <float.h>
#include <stdbool.h>
#include <stdio.h>

#include <sys/sysinfo.h>

double ram_load()
{
	struct sysinfo info;
	int code = sysinfo(&info);
	if (code < 0)
	{
		return -DBL_MAX;
	}
	double load = (double)(info.totalram - info.freeram) / info.totalram;
	if (is_verbose)
	{
		printf("free RAM is %lu bytes\n", info.freeram);
		printf("total RAM is %lu bytes\n", info.totalram);
		printf("RAM usage is %0.2f %\n", load * 100);
	}
	return load;
}