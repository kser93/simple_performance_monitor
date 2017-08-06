#ifndef CPU_LOAD_H
#define CPU_LOAD_H

#include <stdbool.h>
#include <sys/time.h>
#include <time.h>

#define MOVING_WINDOW_LEN_SEC 2
#define USEC_PER_SEC 1000000

extern const bool is_verbose;

double cpu_load();

#endif // !CPU_LOAD_H
