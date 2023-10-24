/*
 * Developed by FAST Lab @ ECE-UIUC -- 2022-2023
 */
#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>
#include <stdbool.h>

#define DEBUG 1
#define debug_print(fmt, ...) \
    do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
            __LINE__, __func__, __VA_ARGS__); } while (0)


/* text color */
#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

typedef struct chase_struct chase_t;

struct chase_struct {
    // 64-bit addr, 64 * 64 = 512 bit per cacheline
    chase_t* ptr_arr[8]; 
};

typedef enum test_op {
    READ,
    READ_NT,
    WRITE,
    WRITE_NT,
    MOV,
    MIXED  /* mix read and write */
} test_op_t;

typedef enum test_type {
    LATS_CLFLUSH,
    BW,
    LATS_CHASE, 
    BLOCK_LATS
} test_type_t;

typedef struct test_cfg {
    // overall
    uint64_t num_thread;
    uint64_t total_buf_size;
    int buf_a_numa_node;
    int buf_b_numa_node;
    char* buf_a;
    char* buf_b;
    bool prefetch_en;
    int bw_granu; // number of cache line (n * 64B)
    double tsc_freq; // GHz

    // thread 
    int thread_idx;
    int core_a;
    int core_b;
    char* start_addr_a;
    char* start_addr_b;
    uint64_t per_thread_size; // num byte per thread
    int op_iter;
    test_type_t type;
    test_op_t op;
    int starting_core;
    bool random;
    int stall_ratio;
    int read_ratio; /* computed by (read / write) */
    bool flush_block;
    int num_clear_pipe;

    // monitoring
    volatile uint64_t curr_op_cnt; 

    // thread sync 	
    volatile int halt;

} test_cfg_t;

int parse_arg(int argc, char*argv[], test_cfg_t* cfg);

int get_node(void* p, uint64_t size);

int init_buf(uint64_t size, int node, char** alloc_ptr);

uint64_t read_MSR(int cpu);

void write_MSR(int cpu, uint64_t val);

void disable_prefetch(int cpu);

void enable_prefetch(int cpu);

uint64_t xorshf96(uint64_t* x);

void flush_all_cache();

#endif // UTIL_H
