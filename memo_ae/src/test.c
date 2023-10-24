/*
 * Developed by FAST Lab @ ECE-UIUC -- 2022-2023
 */
#define _GNU_SOURCE

#include "test.h"
#include "util.h"
#include "workload.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sched.h>
#include <errno.h>
#include <time.h>

#define WAIT_SEC_US 	5000000

#define US_TO_S         1000000

#define SET_VAL         15

#define PAGE_SIZE       4096

//#define CHECK_NT_ST

//#define DUMP_ZMM

// =================================================
//              zmm test functions
// =================================================
/*
 * These functions were used for dumping the data in
 * the avx zmm registers. In zmm0, zmm1, and zmm2, there
 * are a few bytes stayed constant despite storing to it.
 * Hence the weird conditions in `check_buff`
 * Maybe because of this: https://stackoverflow.com/questions/41819514/why-do-sse-instructions-preserve-the-upper-128-bit-of-the-ymm-registers
 */
void set_data_buf(char* data_buf, uint64_t size) {
    fprintf(stdout, "[set_data_buf] \n");
    for (uint64_t i = 0; i < size; i++) {
        data_buf[i] = i; 
    }
}

void clear_buff(char* buff, uint64_t size) {
    fprintf(stdout, "[clear_buff]\n");
    for (uint64_t i = 0; i < size; i++) {
        buff[i] = 0;
    }
}

void check_buff(char* buff, uint64_t size) {
    fprintf(stdout, "[check_buff]\n");
    uint64_t error_cnt = 0;
    uint64_t correct_cnt = 0;

    char* truth_buf;
    posix_memalign((void**)(&truth_buf), PAGE_SIZE, PAGE_SIZE);
    dump_zmm(truth_buf, 1024);
    int mod;
    for (uint64_t i = 0; i < size; i++) {
        mod = i % 1024;
        if (mod <= 0xF) continue;
        if (mod >= 0x40 && mod <= 0x4F) continue;
        if (mod >= 0x80 && mod <= 0x8F) continue;
        if (mod >= 0x140 && mod <= 0x14F) continue;
        if (buff[i] != truth_buf[mod]) {
            fprintf(stdout, "[check_buff] buff[%lx] != truth, found %x, expect %x\n", i, (unsigned char)buff[i], (unsigned char)truth_buf[mod]);
            error_cnt++;
        } else {
            correct_cnt++;
        }
        if (error_cnt > 100) {
            fprintf(stdout, "[check_buff] before exit with 100 error, correct_cnt %ld\n", correct_cnt);
            return;
        }
    }
    free(truth_buf);
    if (error_cnt == 0) {
        fprintf(stdout, "[check_buff] all correct! correct_cnt: %ld\n", correct_cnt);
    }
}


// =================================================
//              benchmark wrapping functions
// =================================================

static volatile int keepRunning = 1;
void stop_threads(test_cfg_t* cfg_arr) {
    int num_thread;
    num_thread = cfg_arr[0].num_thread;
    fprintf(stdout, "[stop_threads]\n");

    for (int i = 0; i < num_thread; i++) {
        cfg_arr[i].halt = 1;
    }
}

void intHandler(int dummy) {
    fprintf(stdout, "Ctrl-c detected, %d\n", dummy);
    keepRunning = 0;
}

/** 
 * get_bw
 *   @brief Read the curr_op_cnt from each thread and calculate the sum every *delay* microsecond.
 *   @param cfg_arr array of config.
 *	 @param	iter 	how many times we probe the bandwidth.
 *	 @param delay 	interval between probes in unit of us.
 *   @return none
 */
void get_bw(test_cfg_t* cfg_arr, int iter, int delay) {
    int num_thread;
    num_thread = cfg_arr[0].num_thread;
    uint64_t prev_cnt, curr_cnt;
    prev_cnt = 0;

    for (int j = 0; j < iter; j++) {
        curr_cnt = 0;
        for (int i = 0; i < num_thread; i++) {
            curr_cnt += cfg_arr[i].curr_op_cnt;
        }
        // fprintf(stdout, GRN "[get_bw] " RESET "%.1f MB/sec\n", ((double)(curr_cnt - prev_cnt) / (double)((1 << 20) * (delay / 1000000))));
        /* for easier processing */
        fprintf(stdout, "[get_bw] %.1f MB/sec\n", ((double)(curr_cnt - prev_cnt) / (double)((1 << 20) * (delay / US_TO_S))));
        prev_cnt = curr_cnt;
        if (keepRunning == 0) break;

        usleep(delay);
    }
}

/*
 * wrapping:
 *  main
 * 		run_test
 *			thread_wrapper
 *				lats/bw_wrapper
 *					operation
 */

// spawn thread 
int run_test(test_cfg_t* cfg) {
    pthread_t* thread_arr;
    test_cfg_t* cfg_arr;
    test_cfg_t* curr_cfg;
    int ret, num_thread; 

    // just in case
    signal(SIGINT, intHandler);

    // alloc
    num_thread = cfg->num_thread;
    thread_arr = malloc(num_thread * sizeof(pthread_t));
    cfg_arr = malloc(num_thread * sizeof(test_cfg_t));
    memset(cfg_arr, 0, num_thread * sizeof(test_cfg_t));


    // clear buff
#ifdef CHECK_NT_ST
    clear_buff(cfg->buf_a, cfg->total_buf_size);
    if (cfg->op == MOV) {
        clear_buff(cfg->buf_b, cfg->total_buf_size);
    }
#endif
    
    // launch thread
    for (int i = 0; i < num_thread; i++) {
        curr_cfg = &(cfg_arr[i]);
        memcpy(curr_cfg, cfg, sizeof(test_cfg_t));

        curr_cfg->thread_idx = i;
        curr_cfg->halt = 0;
        curr_cfg->curr_op_cnt = 0;

        curr_cfg->start_addr_a = &(curr_cfg->buf_a[i * curr_cfg->per_thread_size]);
        if (cfg->op == MOV) {
            curr_cfg->start_addr_b = &(curr_cfg->buf_b[i * curr_cfg->per_thread_size]);
        }
        ret = pthread_create(&thread_arr[i], NULL, thread_wrapper, (void*)curr_cfg);
    }

    // monitor threads
    switch(cfg->type) {
        case LATS_CLFLUSH:
            // do nothing, latency is monitored within a single thread
            break;
        case BW:
            get_bw(cfg_arr, cfg->op_iter, WAIT_SEC_US); 
            break;
        case LATS_CHASE:
            // do nothing, latency is monitored within a single thread
            break;
        case BLOCK_LATS:
            // do nothing, latency is monitored within a single thread
            break; 
        default:
            fprintf(stderr, "unknown type, thread idx: %d\n", cfg->thread_idx);
    }
    
    if (cfg->type == BW) {
        stop_threads(cfg_arr);
    }

    // join threads
    for (int i = 0; i < num_thread; i++) {
        ret = pthread_join(thread_arr[i], NULL);
    }

    free(cfg_arr);
    free(thread_arr);
    return ret;
}

void print_lats(test_cfg_t* cfg, uint64_t min, uint64_t max, uint64_t sum, uint64_t num_chase_block) {
    uint64_t avg_cycle = sum / cfg->op_iter;
    printf(GRN "[RESULT]" RESET " Max latency: %.1f, Min latency: %.1f\n", 1.0*max/cfg->tsc_freq, 1.0*min/cfg->tsc_freq);
    printf(GRN "[RESULT]" RESET " Max cycle  : %lu,  Min cycle  : %lu, Avg cycle: %lu\n", max, min, avg_cycle);
    printf(GRN "[RESULT]" RESET " Thread %d average latency among %d iterations: %.1fns (assume %fGHz)\n", cfg->thread_idx, \
            cfg->op_iter, 1.0 / cfg->tsc_freq * avg_cycle, cfg->tsc_freq);
    if (num_chase_block != 0) {
        printf(RED "[RESULT]" RESET "chase/block_lats average cycle among %d iterations: %.1fcycles\n", cfg->op_iter, 1.0 * avg_cycle / num_chase_block);
        printf(RED "[RESULT]" RESET " chase/block_lats average latency among %d iterations: %.1fns (assume %fGHz)\n", cfg->op_iter, 1.0 / cfg->tsc_freq * avg_cycle / num_chase_block, cfg->tsc_freq);
    }
}

int comp(const void* elem1, const void* elem2){
    uint64_t f = *((uint64_t*)elem1);
    uint64_t s = *((uint64_t*)elem2);
    if (f > s) return  1;
    if (f < s) return -1;
    return 0;
}

void print_lats_median(test_cfg_t* cfg, uint64_t* result) {
    qsort(result, cfg->op_iter, sizeof(*result), comp);
    uint64_t median = result[cfg->op_iter / 2];
    printf(RED "[RESULT]" RESET " Median latency among %d iterations: %.1fns (assume %fGHz)\n", cfg->op_iter, 1.0 / cfg->tsc_freq * median, cfg->tsc_freq);
}

void init_ptr_buf_random(test_cfg_t* cfg) {
    // FIXME -- this is NOT implemented
    printf(YEL "[INFO]" RESET " Random pointer chasing is NOT implemented. Building the ptr array in order\n");
    chase_t* curr_ptr;
    chase_t* next_ptr;
    uint64_t num_chase_block;

    curr_ptr = (chase_t*)cfg->start_addr_a;
    num_chase_block = cfg->total_buf_size / 64;

    for (uint64_t i = 0; i < num_chase_block - 1; i++) {
        next_ptr = &(curr_ptr[1]);
        curr_ptr->ptr_arr[0] = next_ptr;
        curr_ptr = next_ptr;
    }
    curr_ptr->ptr_arr[0] = (chase_t*)cfg->start_addr_a;
}

uint64_t init_ptr_buf(test_cfg_t* cfg) {

    chase_t* curr_ptr;
    chase_t* next_ptr;
    uint64_t num_chase_block;

    printf(YEL "[INFO]" RESET " building pointer chasing link list, block size: %ld bytes ... \n", sizeof(chase_t));

    if (cfg->random) {
        init_ptr_buf_random(cfg);

    } else {
        curr_ptr = (chase_t*)cfg->start_addr_a;
        num_chase_block = cfg->total_buf_size / sizeof(chase_t);

        for (uint64_t i = 0; i < num_chase_block - 1; i++) {
            //for (uint64_t i = 0; i < 5; i++) {
            next_ptr = &(curr_ptr[1]);
            curr_ptr->ptr_arr[0] = next_ptr;
            curr_ptr = next_ptr;
        }
        curr_ptr->ptr_arr[0] = (chase_t*)cfg->start_addr_a;
    }

    printf(YEL "[INFO]" RESET " num blocks: %lu \n", num_chase_block);
    printf(YEL "[INFO]" RESET " Chase confirm: start_addr: 0x%lx, first chase addr: 0x%lx\n",
            (uint64_t)cfg->start_addr_a,
            (uint64_t)(&((chase_t*)(cfg->start_addr_a))->ptr_arr[0]));

    curr_ptr = (chase_t*)cfg->start_addr_a;
    printf(YEL "[INFO]" RESET " Chase confirm: next_addr: 0x%lx, second chase addr: 0x%lx\n",
            (uint64_t)(&curr_ptr[1]),
            (uint64_t)(((chase_t*)(cfg->start_addr_a))->ptr_arr[0]));

    return num_chase_block;
}

void set_prefetching(int starting_core, bool prefetch_en, int core_num) {
    if (starting_core >= 0) {
        if (prefetch_en) {
            enable_prefetch(core_num);
        } else {
            disable_prefetch(core_num);
        }
    }
}

void restore_prefetching(int starting_core, bool prefetch_en, int core_num) {
    if (starting_core >= 0) {
        // restore to enable prefetching
        if (!prefetch_en) {
            enable_prefetch(core_num);
        }
    }
}


void lats_chase_wrapper(test_cfg_t* cfg) {
    uint64_t result, latency_sum = 0;
    uint64_t min, max;
    uint64_t num_chase_block; 
    int core_num = cfg->thread_idx + cfg->starting_core;

    if (cfg->start_addr_a == NULL) {
        printf(RED "[ERROR]" RESET " init_ptr_buf, found null buf addr\n");
        return;
    }

    num_chase_block = init_ptr_buf(cfg);
    
    set_prefetching(cfg->starting_core, cfg->prefetch_en, core_num);

    cfg->op_iter += 1; // for warm up
    for (int i = 0; i < cfg->op_iter; i++) {
        switch (cfg->op) {
            default:
                result = op_ptr_chase(cfg->start_addr_a, num_chase_block);
                break;
        }
        if (i >= 1) {
            latency_sum += result;
            printf("result = %lu\n", result);
        } else {
            printf("warmup = %lu\n", result);
        }
        if (i == 1) {
            min = result;
            max = result;
        } else {
            if (min < result) min = result;
            if (max > result) max = result;
        }
    }

    restore_prefetching(cfg->starting_core, cfg->prefetch_en, core_num);

    cfg->op_iter -= 1; // remove warm up for average
    print_lats(cfg, min, max, latency_sum, num_chase_block);
}

/*
 * This function tests multi-operation latency.
 * The scheme here goes as:  
 *  flush cacheline <optional, parameter -B>
 *      mfence
 *  issue many nop <optional, parameter -C>
 *  mark time1
 *      issue X ops
 *  mark time2 
 *
 * In most cases, the latency goes down as more ops
 *  are issued in parallel. 
 */
void block_lats_wrapper(test_cfg_t* cfg) {
    uint64_t result, latency_sum = 0;
    uint64_t min, max;
    int offset; /* measure the latency op_iter times and take average */
    int core_num = cfg->thread_idx + cfg->starting_core;

    uint64_t* result_buff;
    result_buff = malloc(sizeof(uint64_t) * cfg->op_iter);

    set_prefetching(cfg->starting_core, cfg->prefetch_en, core_num);

    flush_all_cache();
    for (int i = 0; i < cfg->op_iter; i++) {
        offset = rand() % cfg->total_buf_size & ~(0xFFFF);
        switch (cfg->op) {
            case READ:
                result = op_ld_block_lat(cfg->start_addr_a + offset, cfg->flush_block, cfg->num_clear_pipe);
                break;
            case READ_NT:
                result = op_ntld_block_lat(cfg->start_addr_a + offset, cfg->flush_block, cfg->num_clear_pipe);
                break;
            case WRITE:
                result = op_stwb_block_lat(cfg->start_addr_a + offset, cfg->flush_block, cfg->num_clear_pipe);
                break;
            case WRITE_NT:
                result = op_ntst_block_lat(cfg->start_addr_a + offset, cfg->flush_block, cfg->num_clear_pipe);
                break;
            default:
                printf(RED "[ERROR]" RESET "bad cfg->op\n");
                goto out;
        }
        if (i == 0) {
            min = result;
            max = result;
        } else {
            min = (result < min) ? result : min;
            max = (result > max) ? result : max;
        }
        latency_sum += result;
        result_buff[i] = result / BLOCK_xN;
    }
    print_lats(cfg, min, max, latency_sum, BLOCK_xN);
    print_lats_median(cfg, result_buff);

out:
    restore_prefetching(cfg->starting_core, cfg->prefetch_en, core_num);
    free(result_buff);
}

/*
 * This function tests single operation latency.
 * The scheme here goes as:  
 *  flush cacheline
 *  issue many nop
 *  mark time1
 *      issue 1 op
 *  mark time2 
 *
 * In most cases, the latency here is very high,
 *  and the actual interpretation of this latency
 *  may vary.
 */
void lats_clflush_wrapper(test_cfg_t* cfg) {

    uint64_t result, latency_sum = 0;
    uint64_t min, max;
    int offset; /* measure the latency op_iter times and take average */
    int core_num = cfg->thread_idx + cfg->starting_core;

    uint64_t* result_buff;
    result_buff = malloc(sizeof(uint64_t) * cfg->op_iter);

    set_prefetching(cfg->starting_core, cfg->prefetch_en, core_num);
    
    flush_all_cache();

    switch (cfg->op)
    {
        case READ:
            for (int i = 0; i < cfg->op_iter; i++){
                offset = rand() % cfg->total_buf_size;
                result = op_ld_64B_lat(cfg->start_addr_a + offset);
                latency_sum += result;
                if (i == 0){
                    min = result;
                    max = result;
                }else{
                    min = (result < min) ? result : min;
                    max = (result > max) ? result : max;
                }
                result_buff[i] = result;
            }
            break;

        case READ_NT:
            for (int i = 0; i < cfg->op_iter; i++){
                offset = rand() % cfg->total_buf_size;
                result = op_ntld_64B_lat(cfg->start_addr_a + offset);
                latency_sum += result;
                if (i == 0){
                    min = result;
                    max = result;
                }else{
                    min = (result < min) ? result : min;
                    max = (result > max) ? result : max;
                }
                result_buff[i] = result;
            }
            break;

        case WRITE:
            for (int i = 0; i < cfg->op_iter; i++){
                offset = rand() % cfg->total_buf_size;
                result = op_st_cl_flush_64B_lat(cfg->start_addr_a + offset);
                latency_sum += result;
                if (i == 0){
                    min = result;
                    max = result;
                }else{
                    min = (result < min) ? result : min;
                    max = (result > max) ? result : max;
                }
                result_buff[i] = result;
            }
            break;

        case WRITE_NT:
            for (int i = 0; i < cfg->op_iter; i++){
                offset = rand() % cfg->total_buf_size;
                result = op_ntst_64B_lat(cfg->start_addr_a + offset);
                latency_sum += result;
                if (i == 0){
                    min = result;
                    max = result;
                }else{
                    min = (result < min) ? result : min;
                    max = (result > max) ? result : max;
                }
                result_buff[i] = result;
            }
            break;

        default:
            break;
    }
    print_lats(cfg, min, max, latency_sum, 0);
    print_lats_median(cfg, result_buff);
    
    restore_prefetching(cfg->starting_core, cfg->prefetch_en, core_num);

    free(result_buff);
    return;
}

void bw_wrapper(test_cfg_t* cfg) {
    const uint64_t fixed_step = cfg->bw_granu << 6;
    // random steps will be aligned by fixed steps
    const uint64_t align_mask = (~(fixed_step - 1));
    // random steps will be multiple of fix steps
    const uint64_t step_bound_mask = ~(align_mask << 6);

    cfg->curr_op_cnt = 0;
    char* src = cfg->start_addr_a;
    char* dst = cfg->start_addr_b;
    uint64_t rand_offset = rand();
    uint64_t curr_step = fixed_step;
    uint64_t counter = 0;
    int stall_cnt;
    int core_num = cfg->thread_idx + cfg->starting_core;
    int rw_ratio = cfg->read_ratio;  // rw_ratio can be 1, 2, 3.
    int mixed_switch = 0;

    set_prefetching(cfg->starting_core, cfg->prefetch_en, core_num);

    if (cfg->op == MOV) {
        printf("src: 0x%lx, dst: 0x%lx\n", (uint64_t)src, (uint64_t)dst);
    }

    /* sanity check */
    if (cfg->op == MIXED && rw_ratio == 2 && fixed_step != 384) {
        printf("[WARNING] You are using a BW_granu other than 384 for R2W1 mixed bandwidth test. Data will be skewed.\n");
    }

#ifdef DUMP_ZMM
    char* data_buf;

    // set all zmm
    posix_memalign((void**)(&data_buf), PAGE_SIZE, PAGE_SIZE);
    set_data_buf(data_buf, PAGE_SIZE);
    set_all_zmm(data_buf);
    dump_zmm(NULL, 0);
    free(data_buf);
#endif


    while (1) {
        if(counter + fixed_step > cfg->per_thread_size){
            //fprintf(stdout, "reach end, reset\n");
            counter = 0;
            src = cfg->start_addr_a;
            dst = cfg->start_addr_b;
        }
        switch(cfg->op) {
            case WRITE:
                op_st(src, fixed_step);
                break;

            case WRITE_NT:
                op_ntst(src, fixed_step);
                break;

            case READ:
                op_ld(src, fixed_step);
                break;

            case READ_NT:
                op_ntld(src, fixed_step);
                break;

            case MOV:
                op_movdir64B(src, dst, fixed_step);
                break;

            case MIXED:
                // op_mixed(src, fixed_step, rw_ratio);
                if (mixed_switch == rw_ratio){
                    op_st(src, fixed_step);
                    mixed_switch = 0;
                } else {
                    op_ld(src, fixed_step);
                    mixed_switch += 1;
                }
                break;

            default:
                fprintf(stderr, "unknown op, thread idx: %d\n", cfg->thread_idx);
                goto out;
        }
        // ==================================== 
        //              Stepping, rand/seq
        // ==================================== 
        // increment number of byte operated on
        cfg->curr_op_cnt += fixed_step;

        /* update the address of interest */
        if (cfg->random) {
            curr_step = (curr_step ^ xorshf96(&rand_offset)) & align_mask;
            curr_step &= step_bound_mask;
            //fprintf(stdout, "curr_step: 0x%lx\n", curr_step);
        } else {
            curr_step = fixed_step;
        }
        counter += curr_step;
        src += curr_step;
        dst += curr_step;

        // ==================================== 
        //              Stalling
        // ==================================== 
        // create artificial stalling if desired
        stall_cnt = 0;
        while (stall_cnt < cfg->stall_ratio) {
            op_stall();
            stall_cnt++; 
        }

        if (cfg->halt) {
            fprintf(stdout, "thread idx: %d end\n", cfg->thread_idx);
#ifdef CHECK_NT_ST
            if (cfg->op == WRITE_NT) {
                check_buff(cfg->buf_a, cfg->total_buf_size);
            }
#endif
            break;
        }
    }
out:
    restore_prefetching(cfg->starting_core, cfg->prefetch_en, core_num);
}

// taken from https://stackoverflow.com/questions/1407786/how-to-set-cpu-affinity-of-a-particular-pthread
int stick_this_thread_to_core(int core_id) {
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    if (core_id < 0 || core_id >= num_cores)
        return EINVAL;

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);

    pthread_t current_thread = pthread_self();
    return pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
}

// dispatch to different workload wrappers
void* thread_wrapper(void* arg) {
    test_cfg_t* cfg;
    cfg = (test_cfg_t*)arg;
    fprintf(stdout, "thread %d created.\n", cfg->thread_idx);

    int res;
    if (cfg->core_a >= 0 && cfg->core_b >= 0) {
        if (cfg->thread_idx == 0) {
            cfg->starting_core = cfg->core_a; // to make prefetch pin to correct core

        } else if (cfg->thread_idx == 1) {
            cfg->starting_core = cfg->core_b; // to make prefetch pin to correct core

        } else {
            printf(RED "[ERROR]" RESET "more than 2 thread in testing pinning to core a, b\n");
            return NULL;
        }
    }

    if (cfg->starting_core >= 0) {
        res = stick_this_thread_to_core(cfg->thread_idx + cfg->starting_core);
    } else {
        printf(YEL "[INFO]" RESET " core pinning is not specified. Prefetching options will be ignored\n");
    }

    if(res != 0){
        printf(RED "[ERROR]" RESET " Thread affinity set failure.\n");
        return NULL;
    }

    switch(cfg->type) {
        case LATS_CLFLUSH:
            lats_clflush_wrapper(cfg);
            break;
        case BW:
            bw_wrapper(cfg);	
            break;
        case LATS_CHASE:
            lats_chase_wrapper(cfg);
            break;
        case BLOCK_LATS:
            block_lats_wrapper(cfg);
            break;
        default:
            fprintf(stderr, "unkown type, thread idx: %d\n", cfg->thread_idx);
    }
    return NULL;
}

