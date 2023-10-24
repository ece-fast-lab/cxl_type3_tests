/*
 * Developed by FAST Lab @ ECE-UIUC -- 2022-2023
 *  Some part of this file follows the methodology of FAST-20 Yang's resporitory
 *  @ https://github.com/NVSL/OptaneStudy/tree/master
 */
#include "workload.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <util.h>
#include <math.h>
#include <string.h>

#define MIN_GRANULARITY     512

// change me to use different size of AVX
//#define SIZENTLD_MACRO SIZENTLD_512_AVX512
#define SIZENTLD_MACRO SIZENTLD_1024_AVX512
#define SIZELD_MACRO   SIZELD_1024_AVX512
#define SIZEST_MACRO   SIZEST_1024_AVX512
#define SIZEMOV_MACRO  SIZEMOV_1024
//#define SIZEST_MACRO   SIZEST_WB_1024_AVX512


/**
 * op_ntld
 *   @brief Load the given size data from the memory with non-temporal hint.
 *   @param addr  the load start address
 *   @param size   		the size of the memory we want to access (in byte)
 *   @return none
 */
void op_ntld(char* addr, long size){
    /* by default we perform load in 512 byte granularity */
    /* sanity check */
    if(size < MIN_GRANULARITY){
        fprintf(stderr, RED "[ERROR]" RESET "op_ntld(): buffer size is smaller than %d byte.", MIN_GRANULARITY);
        exit(1);
    }
    /* round down to MIN_GRANULARITY */
    size = size - (size % MIN_GRANULARITY);

    asm volatile(
            "mov %[addr], %%r9 \n"
            "xor %%r10, %%r10 \n"
            "LOOP_NTLD: \n"
            SIZENTLD_MACRO	
            "cmp %[size], %%r10 \n"
            "jl LOOP_NTLD \n"
            : /* output */
            :[size]"r"(size), [addr]"r"(addr) /* input */
            :"%r9", "%r10" /* clobbered register */
            );
}

/**
 * op_ld
 *   @brief Load the given size data from the memory with non-temporal hint.
 *   @param addr  the load start address
 *   @param size   		the size of the memory we want to access (in byte)
 *   @return none
 */
void op_ld(char* addr, long size){
    /* by default we perform load in 512 byte granularity */
    /* sanity check */
    if(size < MIN_GRANULARITY){
        fprintf(stderr, RED "[ERROR]" RESET "op_ld(): buffer size is smaller than %d byte.", MIN_GRANULARITY);
        exit(1);
    }
    /* round down to MIN_GRANULARITY*/
    size = size - (size % MIN_GRANULARITY);

    asm volatile(
            "mov %[addr], %%r9 \n"
            "xor %%r10, %%r10 \n"
            "LOOP_LD: \n"
            SIZELD_MACRO	
            "cmp %[size], %%r10 \n"
            "jl LOOP_LD \n"
            : /* output */
            :[size]"r"(size), [addr]"r"(addr) /* input */
            :"%r9", "%r10", ZMM_0_15 /* clobbered register */
            );
}

/**
 * op_ntst
 *   @brief Store the given size data to the memory with non-temporal hint.
 *   @param addr the store start address
 *   @param size	   the size of the memory we want to store (in byte)
 *   @return none
 */
void op_ntst(char* addr, long size){
    /* by default we perform load in 512 byte granularity */
    /* sanity check */
    if(size < MIN_GRANULARITY){
        fprintf(stderr, RED "[ERROR]" RESET "op_ntst(): buffer size is smaller than %d byte.", MIN_GRANULARITY);
        exit(1);
    }
    /* round down to MIN_GRANULARITY*/
    size = size - (size % MIN_GRANULARITY);

    asm volatile(
            "mov %[addr], %%r9 \n"
            "xor %%r10, %%r10 \n"
            "LOOP_NTST: \n"
            SIZENTST_1024_AVX512
            "cmp %[size], %%r10 \n"
            "jl LOOP_NTST \n"
            "sfence \n"
            : /* output */
            :[size]"r"(size), [addr]"r"(addr) /* input */
            :"%r9", "%r10", ZMM_0_15 /* clobbered register */
            );
}

/**
 * op_st
 *   @brief Store the given size data to the memory with non-temporal hint.
 *   @param addr the store start address
 *   @param size	   the size of the memory we want to store (in byte)
 *   @return none
 */
void op_st(char* addr, long size){
    /* by default we perform load in 512 byte granularity */
    /* sanity check */
    if(size < MIN_GRANULARITY){
        fprintf(stderr, RED "[ERROR]" RESET "op_st(): buffer size is smaller than %d byte.", MIN_GRANULARITY);
        exit(1);
    }
    /* round down to MIN_GRANULARITY*/
    size = size - (size % MIN_GRANULARITY);

    asm volatile(
            "mov %[addr], %%r9 \n"
            "xor %%r10, %%r10 \n"
            "LOOP_ST: \n"
            SIZEST_MACRO
            "cmp %[size], %%r10 \n"
            "jl LOOP_ST \n"
            : /* output */
            :[size]"r"(size), [addr]"r"(addr) /* input */
            :REGISTERS, "%r10" /* clobbered register */
            );
}

/**
 * op_mixed
 *   @brief Store the given size data to the memory with non-temporal hint.
 *   @param addr the store start address
 *   @param size the total size of the memory we want to operate (in byte)
 * 	 @param ratio the read to write ratio of memory accesses
 *   @return none
 */
void op_mixed(char* addr, long size, int ratio){
    /* by default we perform load in 512 byte granularity */
    /* sanity check */
    if(size < 384){
        fprintf(stderr, RED "[ERROR]" RESET "op_mix(): buffer size is smaller than 384 byte.");
        exit(1);
    }
    /* round down to 512 */
    // size = size - (size % 512);

    switch (ratio)
    {
        case 1:  // 1R:1W
            asm volatile(
                    "mov %[addr], %%r9 \n"
                    "xor %%r10, %%r10 \n"
                    "LOOP_MIXED1: \n"
                    SIZE_R1W1_512
                    "cmp %[size], %%r10 \n"
                    "jl LOOP_MIXED1 \n"
                    : /* output */
                    :[size]"r"(size), [addr]"r"(addr) /* input */
                    :REGISTERS, "%r10" /* clobbered register */
                    );
            break;

        case 2:  // 2R:1W
            asm volatile(
                    "mov %[addr], %%r9 \n"
                    "xor %%r10, %%r10 \n"
                    "LOOP_MIXED2: \n"
                    // SIZE_R2W1_384
                    SIZE_R2W1_576
                    "cmp %[size], %%r10 \n"
                    "jl LOOP_MIXED2 \n"
                    : /* output */
                    :[size]"r"(size), [addr]"r"(addr) /* input */
                    :REGISTERS, "%r10" /* clobbered register */
                    );
            break;
        case 3: // 3R:1W
            asm volatile(
                    "mov %[addr], %%r9 \n"
                    "xor %%r10, %%r10 \n"
                    "LOOP_MIXED3: \n"
                    SIZE_R3W1_512
                    "cmp %[size], %%r10 \n"
                    "jl LOOP_MIXED3 \n"
                    : /* output */
                    :[size]"r"(size), [addr]"r"(addr) /* input */
                    :REGISTERS, "%r10" /* clobbered register */
                    );
            break;

        default:
            fprintf(stderr, RED "[ERROR]" RESET "op_mix(): Invalid RW ratio.");
            exit(1);
            break;
    }


}

/**
 * op_stall 
 *   @brief stall the core by issuing nop
 */

void op_stall() {
    asm volatile(
            CLEAR_PIPELINE
            CLEAR_PIPELINE
            CLEAR_PIPELINE
            CLEAR_PIPELINE
            :
            :
            :
            );
}

/**
 * op_movdir64B
 *   @brief Store the given size data to the memory with non-temporal hint.

 *   @param addr the store start address
 *   @param size	   the size of the memory we want to store (in byte)
 *   @return none
 */
void op_movdir64B(char* src_addr, char* dst_addr, long size) {
    /* by default we perform load in 512 byte granularity */
    /* sanity check */
    if(size < MIN_GRANULARITY){
        fprintf(stderr, RED "[ERROR]" RESET "op_st(): buffer size is smaller than %d byte.", MIN_GRANULARITY);
        exit(1);
    }
    /* round down to MIN_GRANULARITY */
    size = size - (size % MIN_GRANULARITY);
    asm volatile(
            "mov %[src_addr], %%r9 \n"
            "mov %[dst_addr], %%r12 \n"
            "xor %%r10, %%r10 \n"
            "LOOP_MOV: \n"
            SIZEMOV_MACRO
            "cmp %[size], %%r10 \n"
            "jl LOOP_MOV \n"
            "sfence \n"
            : /* output */
            :[size]"r"(size), [src_addr]"r"(src_addr), [dst_addr]"r"(dst_addr)/* input */
            :REGISTERS, "%r10", "%r11", "%r12" /* clobbered register */
            );
}

/**
 * op_ntld_32B_lat
 *   @brief measure the latency of loading 32 bytes with non-temporal hint
 *   @param addr the memory address from where we load the 32 bytes
 *   @return the time elapsed during load process. In unit of CPU cycles.
 */
uint64_t op_ntld_32B_lat(char* addr){
    uint64_t t_start = 0, t_end = 0;

    /* make sure the addr is 32 byte aligned */
    addr = (char*)((uint64_t)addr & (~0x1F));

    asm volatile(
            "mov %[addr], %%rsi\n"
            "mfence\n"
            FLUSH_CACHE_LINE
            TIMING_BEGIN
            "vmovntdqa 0*32(%%rsi), %%ymm0 \n"
            TIMING_END
            :[t_start] "=r" (t_start), [t_end] "=r" (t_end)
            :[addr] "r" (addr)
            :REGISTERS
            );

    return (t_end - t_start);
}

/**
 * op_ntld_64B_lat
 *   @brief measure the latency of loading 64 bytes with non-temporal hint
 *   @param addr the memory address from where we load the 64 bytes
 *   @return the time elapsed during load process. In unit of CPU cycles.
 */
uint64_t op_ntld_64B_lat(char* addr){
    uint64_t t_start = 0, t_end = 0;

    /* make sure address is 64byte aligned (what will happen if not?) */
    addr = (char*)((uint64_t)addr & (~0x3F));

    asm volatile(
            "mov %[addr], %%rsi\n"
            "mfence\n"
            FLUSH_CACHE_LINE
            TIMING_BEGIN
            "vmovntdqa 0*32(%%rsi), %%ymm0 \n"
            "vmovntdqa 1*32(%%rsi), %%ymm1 \n"
            TIMING_END
            :[t_start] "=r" (t_start), [t_end] "=r" (t_end)
            :[addr] "r" (addr)
            :REGISTERS
            );

    return (t_end - t_start);
}


/**
 * op_ntst_64B_lat
 *   @brief measure the latency of storing 64 bytes with non-temporal hint
 *   @param addr the memory address from where we store the 64 bytes
 *   @return the time elapsed during store process. In unit of CPU cycles.
 */
uint64_t op_ntst_64B_lat(char* addr){
    uint64_t t_start = 0, t_end = 0;

    /* make sure address is 64byte aligned (what will happen if not?) */
    addr = (char*)((uint64_t)addr & (~0x3F));

    asm volatile(
            "mov %[addr], %%rsi\n"
            "mfence\n"
            FLUSH_CACHE_LINE
            CLEAR_PIPELINE
            TIMING_BEGIN
            "vmovntpd %%ymm0, 0*32(%%rsi) \n"
            "vmovntpd %%ymm1, 1*32(%%rsi) \n"

            TIMING_END
            :[t_start] "=r" (t_start), [t_end] "=r" (t_end)
            :[addr] "r" (addr)
            :REGISTERS
            );

    return (t_end - t_start);
}

/**
 * op_ld_64B_lat
 *   @brief measure the latency of loading 64 bytes without non-temporal hint
 *   @param addr the memory address from where we load the 64 bytes
 *   @return the time elapsed during load process. In unit of CPU cycles.
 */
uint64_t op_ld_64B_lat(char* addr){
    uint64_t t_start = 0, t_end = 0;

    /* make sure address is 64byte aligned (what will happen if not?) */
    addr = (char*)((uint64_t)addr & (~0x3F));

    asm volatile(
            "mov %[addr], %%rsi\n"
            "mfence\n"
            FLUSH_CACHE_LINE
            CLEAR_PIPELINE
            TIMING_BEGIN
            "vmovdqa 0*32(%%rsi), %%ymm0 \n"
            "vmovdqa 1*32(%%rsi), %%ymm1 \n"
            TIMING_END
            :[t_start] "=r" (t_start), [t_end] "=r" (t_end)
            :[addr] "r" (addr)
            :REGISTERS
            );

    return (t_end - t_start);
}

/**
 * op_st_64B_lat
 *   @brief measure the latency of storing 64 bytes without non-temporal hint
 *   @param addr the memory address from where we store the 64 bytes
 *   @return the time elapsed during store process. In unit of CPU cycles.
 */
uint64_t op_st_64B_lat(char* addr){
    uint64_t t_start = 0, t_end = 0;

    /* make sure address is 64byte aligned (what will happen if not?) */
    addr = (char*)((uint64_t)addr & (~0x3F));

    asm volatile(
            "mov %[addr], %%rsi\n"
            "mfence\n"
            FLUSH_CACHE_LINE
            CLEAR_PIPELINE
            TIMING_BEGIN
            "vmovdqa %%ymm0, 0*32(%%rsi) \n"
            "vmovdqa %%ymm0, 1*32(%%rsi) \n"
            //"sfence \n"
            TIMING_END
            :[t_start] "=r" (t_start), [t_end] "=r" (t_end)
            :[addr] "r" (addr)
            :REGISTERS
            );

    return (t_end - t_start);
}

/**
 * op_st_cl_flush_64B_lat
 *   @brief measure the latency of storing 64 bytes & flushing the cacheline, without non-temporal hint
 *   @param addr the memory address from where we store the 64 bytes
 *   @return the time elapsed during store process. In unit of CPU cycles.
 */
uint64_t op_st_cl_flush_64B_lat(char* addr){
    uint64_t t_start = 0, t_end = 0;

    /* make sure address is 64byte aligned (what will happen if not?) */
    addr = (char*)((uint64_t)addr & (~0x3F));

    asm volatile(
            "mov %[addr], %%rsi\n"
            "mfence\n"
            CLEAR_PIPELINE
            TIMING_BEGIN
            "vmovdqa %%ymm0, 0*32(%%rsi) \n"
            "vmovdqa %%ymm0, 1*32(%%rsi) \n"
            "clwb 0*32(%%rsi) \n"
            TIMING_END
            :[t_start] "=r" (t_start), [t_end] "=r" (t_end)
            :[addr] "r" (addr)
            :REGISTERS
            );
    return (t_end - t_start);
}

/**
 * op_st_32B_lat
 *   @brief measure the latency of storing 32 bytes without non-temporal hint
 *   @param addr the memory address from where we store the 32 bytes
 *   @return the time elapsed during store process. In unit of CPU cycles.
 */
uint64_t op_st_32B_lat(char* addr){
    uint64_t t_start = 0, t_end = 0;

    /* make sure address is 32 byte aligned (what will happen if not?) */
    addr = (char*)((uint64_t)addr & (~0x1F));

    asm volatile(
            "mov %[addr], %%rsi\n"
            "mfence\n"
            FLUSH_CACHE_LINE
            TIMING_BEGIN
            "vmovdqa %%ymm0, 0*32(%%rsi) \n"
            TIMING_END
            :[t_start] "=r" (t_start), [t_end] "=r" (t_end)
            :[addr] "r" (addr)
            :REGISTERS
            );

    return (t_end - t_start);
}

uint64_t op_ptr_chase(char* addr, uint64_t num_chase_block) {
    uint64_t t_start = 0, t_end = 0;
    asm volatile(
            "mov %[addr], %%r11 \n"
            "xor %%r10, %%r10 \n"
            TIMING_BEGIN

            "LOOP_CHASE: \n"
            "mov (%%r11), %%r11 \n"
            "inc %%r10 \n"
            "cmp %[num_chase_block], %%r10 \n"
            "jl LOOP_CHASE \n"

            TIMING_END
            :[t_start] "=r" (t_start), [t_end] "=r" (t_end)
            :[addr] "r" (addr), [num_chase_block] "r" (num_chase_block)
            :REGISTERS, "%r10", "%r11"
            );
    return (t_end - t_start);
}

uint64_t op_stwb_block_lat(char* addr, bool flush_block, long num_clear_pipe) {
    uint64_t t_start = 0, t_end = 0;
    //assume 64KB buff
    asm volatile(
            "mov %[addr], %%r11 \n"
            "xor %%r10, %%r10 \n"

            "cmp $0x0, %[flush_block] \n"
            "je LOOP_BLOCK_STWB_FLUSH_DONE \n"
            "LOOP_BLOCK_STWB_FLUSH: \n"
                "clflush (%%r11, %%r10) \n"
                "add $0x40, %%r10 \n"
                "cmp $0x10000, %%r10 \n"
                "jl LOOP_BLOCK_STWB_FLUSH \n"
            "xor %%r10, %%r10 \n"
            "mfence \n"

            "LOOP_BLOCK_STWB_FLUSH_DONE: \n"

                "cmp %[num_clear_pipe], %%r10 \n"
                "je LOOP_BLOCK_STWB_START \n"
                CLEAR_PIPELINE_x16
                "add $0x1, %%r10 \n"
                "jmp LOOP_BLOCK_STWB_FLUSH_DONE \n"

            "LOOP_BLOCK_STWB_START: \n"
            "xor %%r10, %%r10 \n"

            // Test 
            TIMING_BEGIN
            STWB_xN_RAND_AVX512 
            TIMING_END

            :[t_start] "=r" (t_start), [t_end] "=r" (t_end)
            :[addr] "r" (addr), [flush_block] "r" (flush_block), [num_clear_pipe] "r" (num_clear_pipe)
            :REGISTERS, "%r10", "%r11", ZMM_0_15
    );

    return (t_end - t_start);
}

uint64_t op_ld_block_lat(char* addr, bool flush_block, long num_clear_pipe) {
    uint64_t t_start = 0, t_end = 0;
    asm volatile(
            "mov %[addr], %%r11 \n"
            "xor %%r10, %%r10 \n"

            // flush data
            "cmp $0x0, %[flush_block] \n"
            "je LOOP_BLOCK_LD_FLUSH_DONE \n"
            "LOOP_BLOCK_LD_FLUSH: \n"
                "clflush (%%r11, %%r10) \n"
                "add $0x40, %%r10 \n"
                "cmp $0x10000, %%r10 \n"
                "jl LOOP_BLOCK_LD_FLUSH \n"
            "xor %%r10, %%r10 \n"
            "mfence \n"

            "LOOP_BLOCK_LD_FLUSH_DONE: \n"

                "cmp %[num_clear_pipe], %%r10 \n"
                "je LOOP_BLOCK_LD_START \n"
                CLEAR_PIPELINE_x16
                "add $0x1, %%r10 \n"
                "jmp LOOP_BLOCK_LD_FLUSH_DONE \n"

            "LOOP_BLOCK_LD_START: \n"
            "xor %%r10, %%r10 \n"

            // Test 
            TIMING_BEGIN
            LD_xN_RAND_AVX512
            TIMING_END

            :[t_start] "=r" (t_start), [t_end] "=r" (t_end)
            :[addr] "r" (addr), [flush_block] "r" (flush_block), [num_clear_pipe] "r" (num_clear_pipe)
            :REGISTERS, "%r10", "%r11", ZMM_0_15
    );
    return (t_end - t_start);
}

uint64_t op_ntld_block_lat(char* addr, bool flush_block, long num_clear_pipe) {
    uint64_t t_start = 0, t_end = 0;
    asm volatile(
            "mov %[addr], %%r11 \n"
            "xor %%r10, %%r10 \n"

            // flush data
            "cmp $0x0, %[flush_block] \n"
            "je LOOP_BLOCK_NTLD_FLUSH_DONE \n"
            "LOOP_BLOCK_NTLD_FLUSH: \n"
                "clflush (%%r11, %%r10) \n"
                "add $0x40, %%r10 \n"
                "cmp $0x10000, %%r10 \n"
                "jl LOOP_BLOCK_NTLD_FLUSH \n"
            "xor %%r10, %%r10 \n"
            "mfence \n"

            "LOOP_BLOCK_NTLD_FLUSH_DONE: \n"

                "cmp %[num_clear_pipe], %%r10 \n"
                "je LOOP_BLOCK_NTLD_START \n"
                CLEAR_PIPELINE_x16
                "add $0x1, %%r10 \n"
                "jmp LOOP_BLOCK_NTLD_FLUSH_DONE \n"

            "LOOP_BLOCK_NTLD_START: \n"
            "xor %%r10, %%r10 \n"

            // Test 
            TIMING_BEGIN
            NTLD_xN_RAND_AVX512
            TIMING_END

            :[t_start] "=r" (t_start), [t_end] "=r" (t_end)
            :[addr] "r" (addr), [flush_block] "r" (flush_block), [num_clear_pipe] "r" (num_clear_pipe)
            :REGISTERS, "%r10", "%r11", ZMM_0_15
    );
    return (t_end - t_start);
}

uint64_t op_ntst_block_lat(char* addr, bool flush_block, long num_clear_pipe) {
    uint64_t t_start = 0, t_end = 0;
    asm volatile(
            "mov %[addr], %%r11 \n"
            "xor %%r10, %%r10 \n"

            // flush data
            "cmp $0x0, %[flush_block] \n"
            "je LOOP_BLOCK_NTST_FLUSH_DONE \n"
            "LOOP_BLOCK_NTST_FLUSH: \n"
                "clflush (%%r11, %%r10) \n"
                "add $0x40, %%r10 \n"
                "cmp $0x10000, %%r10 \n"
                "jl LOOP_BLOCK_NTST_FLUSH \n"
            "xor %%r10, %%r10 \n"
            "mfence \n"

            "LOOP_BLOCK_NTST_FLUSH_DONE: \n"

                "cmp %[num_clear_pipe], %%r10 \n"
                "je LOOP_BLOCK_NTST_START \n"
                CLEAR_PIPELINE_x16
                "add $0x1, %%r10 \n"
                "jmp LOOP_BLOCK_NTST_FLUSH_DONE \n"

            "LOOP_BLOCK_NTST_START: \n"
            "xor %%r10, %%r10 \n"

            // Test 
            TIMING_BEGIN
            NTST_xN_RAND_AVX512
            "sfence \n"
            TIMING_END

            :[t_start] "=r" (t_start), [t_end] "=r" (t_end)
            :[addr] "r" (addr), [flush_block] "r" (flush_block), [num_clear_pipe] "r" (num_clear_pipe)
            :REGISTERS, "%r10", "%r11", ZMM_0_15
    );
    return (t_end - t_start);
}

void set_all_zmm(char* addr) {
    asm volatile(
        "mov %[addr], %%r9 \n"
        "xor %%r10, %%r10 \n"
        SIZELD_MACRO	
        "mfence\n"
        : /* output */
        :[addr]"r"(addr) /* input */
        :"%r9", "%r10", REGISTERS, ZMM_0_15 /* clobbered register */
    );
}

void dump_zmm(char* dst, uint64_t size) {
    char* data_buf;
    posix_memalign((void**)(&data_buf), 4096, 4096);
    for (int i = 0; i < 4096; i++) {
        data_buf[i] = 0;
    }
    asm volatile(
        "mov %[addr], %%r9 \n"
        "xor %%r10, %%r10 \n"
        SIZEST_MACRO
        "mfence\n"
        : /* output */
        :[addr]"r"(data_buf) /* input */
        :"%r9", "%r10", REGISTERS, ZMM_0_15 /* clobbered register */
    );
    for (int i = 0; i < 1024; i++) {
        if (i % 64 == 0) {
            printf("zmm%d ", i / 64);
        }
        printf("%x", (unsigned char)data_buf[i]);
        if (i % 64 == 63) {
            printf("\n");
        }
    }
    if (dst != NULL) {
        uint64_t copy_size = size > 1024 ? 1024 : size;
        memcpy(data_buf, dst, copy_size);
    }
    free(data_buf);
}
