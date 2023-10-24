#ifndef TEST_H
#define TEST_H
#include "util.h"
#include <pthread.h>

// spawn thread 
int run_test(test_cfg_t* cfg);

// dispatch to different workload wrappers
void* thread_wrapper(void* arg);

void get_bw(test_cfg_t* cfg_arr, int iter, int delay);

#endif // TEST_H
