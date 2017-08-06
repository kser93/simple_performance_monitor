#include "ram_load.h"
#include "cpu_load.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

const bool is_verbose = true;
//bool foreach_cpu_core = true;

int main()
{
	if (is_verbose) printf("hello from simple_performance_monitor!\n");
	
	double ram_ld = 0.0;
	double cpu_ld = 0.0;

	while (true)
	{
		ram_ld = ram_load();
		cpu_ld = cpu_load();
		
		usleep(USEC_PER_SEC / 10);
	}
	return 0;
}
