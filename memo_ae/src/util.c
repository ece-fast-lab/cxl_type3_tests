/*
 * Developed by FAST Lab @ ECE-UIUC -- 2022-2023
 */
#include "util.h"
#include <stdio.h>
#include <numa.h>
#include <numaif.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#define MAX_NUM_THREAD 		128
#define MAX_BUF_GB 			16
#define MAX_NUMA_NODE 		10
#define MAX_SKIP_BYTE 		1024
#define PREFETCH_REG_ADDR   0x1A4
#define MAX_CORE_NUM		63
#define FLUSH_SIZE          (512 * (1 << 20)) // MB
#define TSC_FREQ_GHZ	    2.0

char* help_str = " Usage: \n" \
                  "-t  	Number of threads.\n" \
                  "-f   enable prefetching (when -p is not specified, prefetch is NOT toggled.) (when -p is specified, default to prefetching disabled)\n" \
                  "-m   buffer size, in byte (!!! only with 32-bit int).\n" \
                  "-S  	buffer size, in GB.\n" \
                  "-n  	NUMA node, if op {0,1,2,3}; SRC node if op {4}.\n" \
                  "-d  	NUMA node, DST node, only used if op {4}.\n" \
                  "-s  	stall ratio -- 5 for 1:5 ratio in op:stall in bandwidth tests.\n" \
                  "-i  	number of iteration. For BW it means how many times we probe all threads.\n" \
                  "-T  	0 stands for Latency, 1 stands for Bandwidth, 2 stands for pointer tracing, 3 stands for block access latency.\n" \
                  "-p  	Pin to cores starting at core X, default -- do not pin to core (let the Linux scheduler decides).\n" \
                  "-a/b pin to core a and core b. -a and -b must be used at the same time\n" \
                  "-g   Bandwidth granularity -- batch size per each workload call, in the unit of 64B.\n" \
                  "-r   random access in bandwidth tests (sequential by default) \n" \
                  "-o   0 - Read, 1 - Read Non-temporal, 2 - Write, 3 - Write Non-temporal, 4 - movdir64B, 5 - Read/Write Mixed.\n" \
                  "-R   If chose operation as Read/Write Mixed, this argument is used to specify the read ratio. Example: 20:80.\n" \
                  "-B   flush 64KB of data block during block latency test (default to 0)\n" \
                  "-C   number of clear pipeline block in block latency tests (default to 0)\n" \
                  "-F   TSC_Freq, used for calculating cycle --> ns. Unit = MHz; default to (2000MHz). Please check with turbostat";

void set_default_cfg(test_cfg_t* cfg) {
    cfg->op = READ;		
    cfg->type = BW;
    cfg->num_thread = 32;
    cfg->total_buf_size = (1 << 30);
    cfg->buf_a_numa_node = 0; // src
    cfg->buf_b_numa_node = 0; // dst
    cfg->op_iter = 1;
    cfg->per_thread_size = cfg->total_buf_size / cfg->num_thread;
    cfg->starting_core = -1;
    cfg->random = false;
    cfg->prefetch_en = false;
    cfg->stall_ratio = 0;
    cfg->bw_granu = 512;
    cfg->core_a = -1;
    cfg->core_b = -1;
    cfg->read_ratio = 1;
    cfg->flush_block = 0;
    cfg->num_clear_pipe = 0;
    cfg->tsc_freq = TSC_FREQ_GHZ;
}

void print_cfg(test_cfg_t* cfg) {

    fprintf (stdout, "==========================\n");
    fprintf (stdout, "num_thread:     %lu\n", cfg->num_thread);
    fprintf (stdout, "total_buf_size: %lu\n", cfg->total_buf_size);
    fprintf (stdout, "buf_a_numa_node:%d\n", cfg->buf_a_numa_node);
    fprintf (stdout, "buf_b_numa_node:%d\n", cfg->buf_b_numa_node);
    fprintf (stdout, "per_thread_size:%ld\n", cfg->per_thread_size);
    fprintf (stdout, "op_iter:        %d\n", cfg->op_iter);
    fprintf (stdout, "type:           %d\n", cfg->type);
    fprintf (stdout, "op:             %d\n", cfg->op);
    fprintf (stdout, "starting_core:  %d\n", cfg->starting_core);
    fprintf (stdout, "random:         %d\n", cfg->random);
    fprintf (stdout, "stall_ratio:    %d\n", cfg->stall_ratio);
    fprintf (stdout, "bw_granu:       %d\n", cfg->bw_granu);
    fprintf (stdout, "core_a:         %d\n", cfg->core_a);
    fprintf (stdout, "core_b:         %d\n", cfg->core_b);
    fprintf (stdout, "flush_block:    %d\n", cfg->flush_block);
    fprintf (stdout, "num_clear_pipe: %d\n", cfg->num_clear_pipe);
    fprintf (stdout, "tsc_freq (GHz): %f\n", cfg->tsc_freq);
    fprintf (stdout, "==========================\n");
}



int parse_arg(int argc, char*argv[], test_cfg_t* cfg) {
    int opt;	
    int num;
    int read;
    int write;
    set_default_cfg(cfg);

    // TODO, parse arg for operation / type
    while ((opt = getopt(argc, argv, "F:C:p:a:b:t:m:S:n:d:s:i:g:T:o:R:rhfB")) != -1) {
        switch (opt) {
            case 'F':
                num = atoi(optarg);
                cfg->tsc_freq = (double)num / (double)(1000.0);
                break;
            case 'C':
                num = atoi(optarg);
                cfg->num_clear_pipe = num;
                break;
            case 'B':
                cfg->flush_block = 1;
                break;
            case 'a':
                num = atoi(optarg);
                if (num < 0 || num > MAX_CORE_NUM) {
                    fprintf (stderr, "Can't start a from core: %d\n", num);
                    return -1;
                }
                cfg->core_a = num;
                break;
            case 'b':
                num = atoi(optarg);
                if (num < 0 || num > MAX_CORE_NUM) {
                    fprintf (stderr, "Can't start b from core: %d\n", num);
                    return -1;
                }
                cfg->core_b = num;
                break;
            case 'p':
                num = atoi(optarg);
                if (num > MAX_CORE_NUM || num < 0) {
                    fprintf (stderr, "Can't start from core: %d\n", num);
                    return -1;
                }
                cfg->starting_core = num;
                break;
            case 't':
                num = atoi(optarg);
                if (num > MAX_NUM_THREAD) {
                    fprintf (stderr, "Can't have more than %d threads, %d\n", MAX_NUM_THREAD, num);
                    return -1;
                } else {
                    cfg->num_thread = num;
                }
                break;
            case 'm':
                num = atoi(optarg);
                cfg->total_buf_size = num;
                break;

            case 'S':
                num = atoi(optarg);
                if (num > MAX_BUF_GB) {
                    fprintf (stderr, "Can't have more than %d GB buf, %d\n", MAX_BUF_GB, num);
                    return -1;
                } else {
                    cfg->total_buf_size = ((uint64_t)num << 30);
                }
                break;

            case 'n':
                num = atoi(optarg);
                if (num < 0 || num > MAX_NUMA_NODE) {
                    fprintf (stderr, "NUMA node out of range (0, %d): %d\n", MAX_NUMA_NODE, num);
                    return -1;
                } else {
                    cfg->buf_a_numa_node = num;
                }
                break;

            case 'd':
                num = atoi(optarg);
                if (num < 0 || num > MAX_NUMA_NODE) {
                    fprintf (stderr, "NUMA node out of range (0, %d): %d\n", MAX_NUMA_NODE, num);
                    return -1;
                } else {
                    cfg->buf_b_numa_node = num;
                }
                break;

            case 's':
                num = atoi(optarg);
                if (num < 0) {
                    fprintf (stderr, "stall ratio must be greater than 0, found: %d\n", num);
                    return -1;
                } else {
                    cfg->stall_ratio = num;
                }
                break;

            case 'i':
                num = atoi(optarg);
                if (num < 0) {
                    fprintf (stderr, "iteration count must be positive: %d\n", num);
                    return -1;
                } else {
                    cfg->op_iter = num;
                }
                break;

            case 'T':
                num = atoi(optarg);
                if(num < 0 || num > 3){
                    fprintf(stderr, "type must be 0(latency clflush), 1(bandwidth), 2(pointer chasing), 3(block latency).\n");
                    return -1;
                } else {
                    cfg->type = num;
                }
                break;

            case 'o':
                num = atoi(optarg);
                if(num < 0 || num > 5){
                    fprintf(stderr, "operation must be 0(read), 1(read non-temporal), 2(write), 3(write non-temporal), 4(movdir64B) or 5(mix RW).\n");
                    return -1;
                } else {
                    cfg->op = num;
                }
                break;

            case 'R':
                sscanf(optarg, "%d:%d", &read, &write);
                if (read <= 0 || write <= 0) {
                    fprintf(stderr, "Read/Write ratio cannot be negative numbers!\n");
                } else {
                    cfg->read_ratio = read / write;
                }
                break;

            case 'g':
                num = atoi(optarg);
                cfg->bw_granu = num;
                break;

            case 'r':
                cfg->random = true;
                break;

            case 'f':
                cfg->prefetch_en = true;
                break;

            case 'h':
                fprintf (stdout, "%s\n", help_str);
                return -2;
                break;

            case '?':
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                return -1;
                break;

            default:
                fprintf (stderr, "default, %c, abort\n", optopt);
                return -1;
                abort();
        }
    }

    if (cfg->core_a * cfg->core_b < 0) {
        fprintf (stderr, "found core_a: %d, core_b: %d, please set them accordingly\n", cfg->core_a, cfg->core_b);
        return -1;
    }

    cfg->per_thread_size = cfg->total_buf_size / cfg->num_thread;
    uint64_t calculated_buf_size = cfg->per_thread_size * cfg->num_thread;
    printf("cal: %lu vs total: %lu\n", calculated_buf_size, cfg->total_buf_size); 

    if (calculated_buf_size != cfg->total_buf_size) {
        // reset per thread size to 2^12 byte aligned (avoid AVX run out of addresss)
        cfg->per_thread_size &= 0xFFFFFFFFFFFFF000;
    }

    // optind is for the extra arguments
    // which are not parsed
    for(; optind < argc; optind++){     
        printf("extra arguments: %s\n", argv[optind]); 
    }

    print_cfg(cfg);
    return 0;
}

// This function returns the NUMA node that a pointer address resides on.
int get_node(void *p, uint64_t size)
{
    int* status;
    void** page_arr;
    unsigned long page_size;
    unsigned long page_cnt;
    int ret;
    char* start_addr;

    page_size = (unsigned long)getpagesize();
    page_cnt = (size / page_size);
    status = malloc(page_cnt * sizeof(int));
    page_arr = malloc(page_cnt * sizeof(char*));
    start_addr = (char*)p;

    fprintf(stdout, "[get_node] buf: %lx, page_size: %ld, page_cnt: %ld\n", (uint64_t)(p), page_size, page_cnt);

    for (unsigned long i = 0; i < page_cnt; i++) {
        page_arr[i] = start_addr;
        if (i < page_cnt) {
            start_addr = &(start_addr[page_size]);
        }
    }


    ret = move_pages(0, page_cnt, page_arr, NULL, status, 0); 
    if (ret != 0) {
        fprintf(stderr, "Problem in %s line %d calling move_pages(), ret = %d\n", __FILE__,__LINE__, ret);
        printf("%s\n", strerror(errno));
    }

    ret = status[0];
    for (uint64_t i = 0; i < page_cnt; i++) {
        if (ret != status[i]) {
            fprintf(stderr, "found page: %lu on node: %d, different from node: %d\n", i, status[i], ret);
            ret = status[i];
            break;
        }
    }

    if (ret == status[0]) {
        fprintf(stdout, "all pages: %lx, %lx ... are on node: %d\n", (uint64_t)(page_arr[0]), (uint64_t)(page_arr[1]), ret);
    }

    free(page_arr);
    free(status);
    return ret;
}

int init_buf(uint64_t size, int node, char** alloc_ptr) {
    char *ptr;
    int ret;
    unsigned long page_size;
    uint64_t page_cnt;
    uint64_t idx;

    if ((ptr = (char *)numa_alloc_onnode(size, node)) == NULL) {
        fprintf(stderr,"Problem in %s line %d allocating memory\n",__FILE__,__LINE__);
        return -1;
    }
    printf("[INFO] done alloc. Next, touch all pages\n");
    // alloc is only ready when accessed
    page_size = (unsigned long)getpagesize();
    page_cnt = (size / page_size);
    idx = 0;
    for (uint64_t i = 0; i < page_cnt; i++) {
        ptr[idx] = 0;	
        idx += page_size;
    }
    printf("[INFO] done touching pages. Next, validate on node X\n");
    ret = get_node(ptr, size);
    if (ret != node) {
        printf("ptr is on node %d, but expect node %d\n", ret, node);
        return -2;
    }
    printf("ptr is on node %d\n", ret);
    printf("allocated: %luMB\n", (size >> 20));

    *alloc_ptr = ptr;
    
    return 0;
}

uint64_t read_MSR(int cpu){
    int fd;
    uint64_t data;
    char msr_file_name[64];

    sprintf(msr_file_name, "/dev/cpu/%d/msr", cpu);
    fd = open(msr_file_name, O_RDONLY);

    if (fd < 0) {
        if (errno == ENXIO) {
            fprintf(stderr, "rdmsr: No CPU %d\n", cpu);
            exit(2);
        } else if (errno == EIO) {
            fprintf(stderr, "rdmsr: CPU %d doesn't support MSRs\n",
                    cpu);
            exit(3);
        } else {
            perror("rdmsr: open");
            exit(127);
        }
    }

    if (pread(fd, &data, sizeof data, PREFETCH_REG_ADDR) != sizeof data) {
        if (errno == EIO) {
            fprintf(stderr, "rdmsr: CPU %d cannot read ", cpu);
            exit(4);
        } else {
            perror("rdmsr: pread");
            exit(127);
        }
    }

    close(fd);

    return data;
}

void write_MSR(int cpu, uint64_t val){
    int fd;
    char msr_file_name[64];

    sprintf(msr_file_name, "/dev/cpu/%d/msr", cpu);
    fd = open(msr_file_name, O_WRONLY);

    if (fd < 0) {
        if (errno == ENXIO) {
            fprintf(stderr, "rdmsr: No CPU %d\n", cpu);
            exit(2);
        } else if (errno == EIO) {
            fprintf(stderr, "rdmsr: CPU %d doesn't support MSRs\n",
                    cpu);
            exit(3);
        } else {
            perror("rdmsr: open");
            exit(127);
        }
    }

    if (pwrite(fd, &val, sizeof(val), PREFETCH_REG_ADDR) != sizeof(val)){
        if (errno == EIO) {
            fprintf(stderr,
                    "wrmsr: CPU %d cannot set MSR ", cpu);
            exit(4);
        } else {
            perror("wrmsr: pwrite");
            exit(127);
        }
    }

    close(fd);

    return;
}

void disable_prefetch(int cpu){
    uint64_t val;
    val = read_MSR(cpu);
    write_MSR(cpu, val | 0xF);
    val = read_MSR(cpu);
    printf(YEL "[INFO]" RESET " CPU %d prefetch disabled. Now at 0x1A4: %lx\n", cpu, val);
}

void enable_prefetch(int cpu){
    uint64_t val;
    val = read_MSR(cpu);
    write_MSR(cpu, val & 0xFFFFFFFFFFFFFFF0);
    printf(YEL "[INFO]" RESET " CPU %d prefetch enabled.\n", cpu);
}

// taken from https://stackoverflow.com/questions/1046714/what-is-a-good-random-number-generator-for-a-game
static uint64_t y=362436069, z=521288629;
uint64_t xorshf96(uint64_t* xx) {          //period 2^96-1
    uint64_t t;
    uint64_t x = *xx;
    x ^= x << 16;
    x ^= x >> 5;
    x ^= x << 1;

    t = x;
    x = y;
    y = z;
    z = t ^ x ^ y;
    *xx = x;

    return z;
}

// can't use WBINVD
// https://stackoverflow.com/questions/1756825/how-can-i-do-a-cpu-cache-flush-in-x86-windows
// alloc large bue and read/write
void flush_all_cache() {
    char* buf;
    printf(YEL "[INFO]" RESET " Flushing cache, with %d MB access ... \n", FLUSH_SIZE >> 20);

    buf = malloc(FLUSH_SIZE);
    for (int j = 0; j < 2; j++) {
        for (int i = 0; i < FLUSH_SIZE; i++) {
            buf[i] = i + 1; // make sure this is not optimized
        }
    }
    free(buf);
    printf(YEL "[INFO]" RESET " Cache flush done ... \n");
}
