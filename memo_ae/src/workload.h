/*
 * Developed by FAST Lab @ ECE-UIUC -- 2022-2023
 *  Some part of this file follows the methodology of FAST-20 Yang's resporitory
 *  @ https://github.com/NVSL/OptaneStudy/tree/master
 */
#ifndef WORKLOAD_H
#define WORKLOAD_H

#include <stdint.h>
#include <stdbool.h>

/*
#define BLOCK_xN 1
#define LD_xN_RAND_AVX512   LD_x1_RAND_AVX512 
#define STWB_xN_RAND_AVX512 STWB_x1_RAND_AVX512 
#define NTLD_xN_RAND_AVX512 NTLD_x1_RAND_AVX512 
#define NTST_xN_RAND_AVX512 NTST_x1_RAND_AVX512 
*/

/*
#define BLOCK_xN 8
#define LD_xN_RAND_AVX512   LD_x8_RAND_AVX512 
#define STWB_xN_RAND_AVX512 STWB_x8_RAND_AVX512 
#define NTLD_xN_RAND_AVX512 NTLD_x8_RAND_AVX512 
#define NTST_xN_RAND_AVX512 NTST_x8_RAND_AVX512 
*/

#define BLOCK_xN 16
#define LD_xN_RAND_AVX512   LD_x16_RAND_AVX512 
#define STWB_xN_RAND_AVX512 STWB_x16_RAND_AVX512 
#define NTLD_xN_RAND_AVX512 NTLD_x16_RAND_AVX512 
#define NTST_xN_RAND_AVX512 NTST_x16_RAND_AVX512 
//#define LD_xN_RAND_AVX512   LD_LFENCE_x16_RAND_AVX512 
//#define STWB_xN_RAND_AVX512 STWB_SFENCE_x16_RAND_AVX512 
//#define NTLD_xN_RAND_AVX512 NTLD_LFENCE_x16_RAND_AVX512 
//#define NTST_xN_RAND_AVX512 NTST_SFENCE_x16_RAND_AVX512 

/*
#define BLOCK_xN 32
#define LD_xN_RAND_AVX512   LD_x32_RAND_AVX512 
#define STWB_xN_RAND_AVX512 STWB_x32_RAND_AVX512 
#define NTLD_xN_RAND_AVX512 NTLD_x32_RAND_AVX512 
#define NTST_xN_RAND_AVX512 NTST_x32_RAND_AVX512 
*/

void op_ntld(char* addr, long size);

void op_ld(char* addr, long size);

void op_ntst(char* addr, long size);

void op_st(char* addr, long size);

void op_stall();

void op_movdir64B(char* src_addr, char* dst_addr, long size);

void op_mixed(char* addr, long size, int ratio);

uint64_t op_ntld_32B_lat(char* addr);

uint64_t op_ntld_64B_lat(char* addr);

uint64_t op_ntst_64B_lat(char* addr);

uint64_t op_ld_64B_lat(char* addr);

uint64_t op_st_64B_lat(char* addr);

uint64_t op_st_cl_flush_64B_lat(char* addr);

uint64_t op_st_32B_lat(char* addr);

uint64_t op_ptr_chase(char* addr, uint64_t num_chase_block);

uint64_t op_ld_block_lat(char* addr, bool flush_block, long num_clear_pipe);

uint64_t op_ntld_block_lat(char* addr, bool flush_block, long num_clear_pipe);

uint64_t op_stwb_block_lat(char* addr, bool flush_block, long num_clear_pipe);

uint64_t op_ntst_block_lat(char* addr, bool flush_block, long num_clear_pipe);

void set_all_zmm(char* addr);

void dump_zmm(char* dst, uint64_t size);

/* Assembly to perform non-temporal load */
#define SIZENTLD_64_AVX512	\
				"vmovntdqa 0x0(%%r9, %%r10), %%zmm0 \n" \
				"add $0x40, %%r10 \n"

#define SIZENTLD_128_AVX512	\
				"vmovntdqa  0x0(%%r9, %%r10), %%zmm0 \n" \
				"vmovntdqa  0x40(%%r9, %%r10), %%zmm1 \n" \
				"add $0x80, %%r10 \n"

#define SIZENTLD_256_AVX512	\
				"vmovntdqa  0x0(%%r9, %%r10), %%zmm0 \n" \
				"vmovntdqa  0x40(%%r9, %%r10), %%zmm1 \n" \
				"vmovntdqa  0x80(%%r9, %%r10), %%zmm2 \n" \
				"vmovntdqa  0xc0(%%r9, %%r10), %%zmm3 \n" \
				"add $0x100, %%r10 \n"

#define SIZENTLD_512_AVX512	\
				"vmovntdqa  0x0(%%r9, %%r10), %%zmm0 \n" \
				"vmovntdqa  0x40(%%r9, %%r10), %%zmm1 \n" \
				"vmovntdqa  0x80(%%r9, %%r10), %%zmm2 \n" \
				"vmovntdqa  0xc0(%%r9, %%r10), %%zmm3 \n" \
				"vmovntdqa  0x100(%%r9, %%r10), %%zmm4 \n" \
				"vmovntdqa  0x140(%%r9, %%r10), %%zmm5 \n" \
				"vmovntdqa  0x180(%%r9, %%r10), %%zmm6 \n" \
				"vmovntdqa  0x1c0(%%r9, %%r10), %%zmm7 \n" \
				"add $0x200, %%r10 \n"

#define SIZENTLD_1024_AVX512	\
				"vmovntdqa  0x0(%%r9, %%r10), %%zmm0 \n" \
				"vmovntdqa  0x40(%%r9, %%r10), %%zmm1 \n" \
				"vmovntdqa  0x80(%%r9, %%r10), %%zmm2 \n" \
				"vmovntdqa  0xc0(%%r9, %%r10), %%zmm3 \n" \
				"vmovntdqa  0x100(%%r9, %%r10), %%zmm4 \n" \
				"vmovntdqa  0x140(%%r9, %%r10), %%zmm5 \n" \
				"vmovntdqa  0x180(%%r9, %%r10), %%zmm6 \n" \
				"vmovntdqa  0x1c0(%%r9, %%r10), %%zmm7 \n" \
				"vmovntdqa  0x200(%%r9, %%r10), %%zmm8 \n" \
				"vmovntdqa  0x240(%%r9, %%r10), %%zmm9 \n" \
				"vmovntdqa  0x280(%%r9, %%r10), %%zmm10 \n" \
				"vmovntdqa  0x2c0(%%r9, %%r10), %%zmm11 \n" \
				"vmovntdqa  0x300(%%r9, %%r10), %%zmm12 \n" \
				"vmovntdqa  0x340(%%r9, %%r10), %%zmm13 \n" \
				"vmovntdqa  0x380(%%r9, %%r10), %%zmm14 \n" \
				"vmovntdqa  0x3c0(%%r9, %%r10), %%zmm15 \n" \
				"add $0x400, %%r10 \n"

#define NTLD_x1_RAND_AVX512 \
                "vmovntdqa  0xd6c0(%%r11, %%r10), %%zmm0 \n"

#define NTLD_x8_RAND_AVX512 \
                "vmovntdqa  0xd6c0(%%r11, %%r10), %%zmm0 \n" \
                "vmovntdqa  0xb680(%%r11, %%r10), %%zmm1 \n" \
                "vmovntdqa  0x7040(%%r11, %%r10), %%zmm2 \n" \
                "vmovntdqa  0x36c0(%%r11, %%r10), %%zmm3 \n" \
                "vmovntdqa  0x3b80(%%r11, %%r10), %%zmm4 \n" \
                "vmovntdqa  0x9340(%%r11, %%r10), %%zmm5 \n" \
                "vmovntdqa  0x9ec0(%%r11, %%r10), %%zmm6 \n" \
                "vmovntdqa  0x6e80(%%r11, %%r10), %%zmm7 \n"

#define NTLD_x16_RAND_AVX512 \
                "vmovntdqa  0xc840(%%r11, %%r10), %%zmm0 \n" \
                "vmovntdqa  0xf180(%%r11, %%r10), %%zmm1 \n" \
                "vmovntdqa  0xce40(%%r11, %%r10), %%zmm2 \n" \
                "vmovntdqa  0x300(%%r11, %%r10), %%zmm3 \n" \
                "vmovntdqa  0x6d40(%%r11, %%r10), %%zmm4 \n" \
                "vmovntdqa  0xa440(%%r11, %%r10), %%zmm5 \n" \
                "vmovntdqa  0xa9c0(%%r11, %%r10), %%zmm6 \n" \
                "vmovntdqa  0xe980(%%r11, %%r10), %%zmm7 \n" \
                "vmovntdqa  0xc940(%%r11, %%r10), %%zmm8 \n" \
                "vmovntdqa  0x8200(%%r11, %%r10), %%zmm9 \n" \
                "vmovntdqa  0xbac0(%%r11, %%r10), %%zmm10 \n" \
                "vmovntdqa  0x8940(%%r11, %%r10), %%zmm11 \n" \
                "vmovntdqa  0xe700(%%r11, %%r10), %%zmm12 \n" \
                "vmovntdqa  0xe100(%%r11, %%r10), %%zmm13 \n" \
                "vmovntdqa  0x8f40(%%r11, %%r10), %%zmm14 \n" \
                "vmovntdqa  0xf2c0(%%r11, %%r10), %%zmm15 \n"

#define NTLD_x32_RAND_AVX512 \
                "vmovntdqa  0x3d80(%%r11, %%r10), %%zmm0 \n" \
                "vmovntdqa  0x1780(%%r11, %%r10), %%zmm1 \n" \
                "vmovntdqa  0x4700(%%r11, %%r10), %%zmm2 \n" \
                "vmovntdqa  0xb980(%%r11, %%r10), %%zmm3 \n" \
                "vmovntdqa  0xaa00(%%r11, %%r10), %%zmm4 \n" \
                "vmovntdqa  0xad00(%%r11, %%r10), %%zmm5 \n" \
                "vmovntdqa  0x9a40(%%r11, %%r10), %%zmm6 \n" \
                "vmovntdqa  0x5300(%%r11, %%r10), %%zmm7 \n" \
                "vmovntdqa  0x7d40(%%r11, %%r10), %%zmm8 \n" \
                "vmovntdqa  0xf480(%%r11, %%r10), %%zmm9 \n" \
                "vmovntdqa  0x9480(%%r11, %%r10), %%zmm10 \n" \
                "vmovntdqa  0xbd80(%%r11, %%r10), %%zmm11 \n" \
                "vmovntdqa  0x3fc0(%%r11, %%r10), %%zmm12 \n" \
                "vmovntdqa  0xcdc0(%%r11, %%r10), %%zmm13 \n" \
                "vmovntdqa  0x480(%%r11, %%r10), %%zmm14 \n" \
                "vmovntdqa  0xb400(%%r11, %%r10), %%zmm15 \n" \
                "vmovntdqa  0xb500(%%r11, %%r10), %%zmm16 \n" \
                "vmovntdqa  0x49c0(%%r11, %%r10), %%zmm17 \n" \
                "vmovntdqa  0x3380(%%r11, %%r10), %%zmm18 \n" \
                "vmovntdqa  0x36c0(%%r11, %%r10), %%zmm19 \n" \
                "vmovntdqa  0x14c0(%%r11, %%r10), %%zmm20 \n" \
                "vmovntdqa  0xcc80(%%r11, %%r10), %%zmm21 \n" \
                "vmovntdqa  0xb600(%%r11, %%r10), %%zmm22 \n" \
                "vmovntdqa  0x6840(%%r11, %%r10), %%zmm23 \n" \
                "vmovntdqa  0x6c80(%%r11, %%r10), %%zmm24 \n" \
                "vmovntdqa  0x2c0(%%r11, %%r10), %%zmm25 \n" \
                "vmovntdqa  0x62c0(%%r11, %%r10), %%zmm26 \n" \
                "vmovntdqa  0x79c0(%%r11, %%r10), %%zmm27 \n" \
                "vmovntdqa  0xfe40(%%r11, %%r10), %%zmm28 \n" \
                "vmovntdqa  0xc200(%%r11, %%r10), %%zmm29 \n" \
                "vmovntdqa  0x58c0(%%r11, %%r10), %%zmm30 \n" \
                "vmovntdqa  0x9b40(%%r11, %%r10), %%zmm31 \n"

/* Assembly to perform non-temporal store */
#define SIZENTST_64_AVX512		\
				"vmovntdq  %%zmm0,  0x0(%%r9, %%r10) \n" \
				"add $0x40, %%r10 \n"

#define SIZENTST_128_AVX512		\
				"vmovntdq  %%zmm0,  0x0(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x40(%%r9, %%r10) \n" \
				"add $0x80, %%r10 \n"

#define SIZENTST_256_AVX512		\
				"vmovntdq  %%zmm0,  0x0(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x40(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x80(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0xc0(%%r9, %%r10) \n" \
				"add $0x100, %%r10 \n"

#define SIZENTST_512_AVX512		\
				"vmovntdq  %%zmm0,  0x0(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x40(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x80(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0xc0(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x100(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x140(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x180(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm0,  0x1c0(%%r9, %%r10) \n" \
				"add $0x200, %%r10 \n"

#define SIZENTST_1024_AVX512	\
				"vmovntdq  %%zmm0,  0x0(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm1,  0x40(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm2,  0x80(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm3,  0xc0(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm4,  0x100(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm5,  0x140(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm6,  0x180(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm7,  0x1c0(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm8,  0x200(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm9,  0x240(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm10,  0x280(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm11,  0x2c0(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm12,  0x300(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm13,  0x340(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm14,  0x380(%%r9, %%r10) \n" \
				"vmovntdq  %%zmm15,  0x3c0(%%r9, %%r10) \n" \
				"add $0x400, %%r10 \n"

#define NTST_x1_RAND_AVX512 \
                "vmovntdq %%zmm0, 0x9680(%%r11, %%r10) \n"

#define NTST_x8_RAND_AVX512 \
                "vmovntdq %%zmm0, 0x9680(%%r11, %%r10) \n" \
                "vmovntdq %%zmm1, 0x15c0(%%r11, %%r10) \n" \
                "vmovntdq %%zmm2, 0x4a80(%%r11, %%r10) \n" \
                "vmovntdq %%zmm3, 0xb800(%%r11, %%r10) \n" \
                "vmovntdq %%zmm4, 0x9700(%%r11, %%r10) \n" \
                "vmovntdq %%zmm5, 0x2000(%%r11, %%r10) \n" \
                "vmovntdq %%zmm6, 0x8d40(%%r11, %%r10) \n" \
                "vmovntdq %%zmm7, 0xb640(%%r11, %%r10) \n"

#define NTST_x16_RAND_AVX512 \
                "vmovntdq %%zmm0, 0x3680(%%r11, %%r10) \n" \
                "vmovntdq %%zmm1, 0x4140(%%r11, %%r10) \n" \
                "vmovntdq %%zmm2, 0x2cc0(%%r11, %%r10) \n" \
                "vmovntdq %%zmm3, 0x28c0(%%r11, %%r10) \n" \
                "vmovntdq %%zmm4, 0x8440(%%r11, %%r10) \n" \
                "vmovntdq %%zmm5, 0xec40(%%r11, %%r10) \n" \
                "vmovntdq %%zmm6, 0x1080(%%r11, %%r10) \n" \
                "vmovntdq %%zmm7, 0x6e00(%%r11, %%r10) \n" \
                "vmovntdq %%zmm8, 0x3300(%%r11, %%r10) \n" \
                "vmovntdq %%zmm9, 0xef80(%%r11, %%r10) \n" \
                "vmovntdq %%zmm10, 0xb900(%%r11, %%r10) \n" \
                "vmovntdq %%zmm11, 0x2280(%%r11, %%r10) \n" \
                "vmovntdq %%zmm12, 0x85c0(%%r11, %%r10) \n" \
                "vmovntdq %%zmm13, 0x240(%%r11, %%r10) \n" \
                "vmovntdq %%zmm14, 0x40c0(%%r11, %%r10) \n" \
                "vmovntdq %%zmm15, 0x3100(%%r11, %%r10) \n"

#define NTST_x32_RAND_AVX512 \
                "vmovntdq %%zmm0, 0x4240(%%r11, %%r10) \n" \
                "vmovntdq %%zmm1, 0x6400(%%r11, %%r10) \n" \
                "vmovntdq %%zmm2, 0xe4c0(%%r11, %%r10) \n" \
                "vmovntdq %%zmm3, 0xf200(%%r11, %%r10) \n" \
                "vmovntdq %%zmm4, 0xc400(%%r11, %%r10) \n" \
                "vmovntdq %%zmm5, 0x9e80(%%r11, %%r10) \n" \
                "vmovntdq %%zmm6, 0xaf80(%%r11, %%r10) \n" \
                "vmovntdq %%zmm7, 0xb380(%%r11, %%r10) \n" \
                "vmovntdq %%zmm8, 0xc7c0(%%r11, %%r10) \n" \
                "vmovntdq %%zmm9, 0x65c0(%%r11, %%r10) \n" \
                "vmovntdq %%zmm10, 0x5b40(%%r11, %%r10) \n" \
                "vmovntdq %%zmm11, 0x8640(%%r11, %%r10) \n" \
                "vmovntdq %%zmm12, 0x67c0(%%r11, %%r10) \n" \
                "vmovntdq %%zmm13, 0xaa80(%%r11, %%r10) \n" \
                "vmovntdq %%zmm14, 0x7640(%%r11, %%r10) \n" \
                "vmovntdq %%zmm15, 0x6d40(%%r11, %%r10) \n" \
                "vmovntdq %%zmm16, 0x1400(%%r11, %%r10) \n" \
                "vmovntdq %%zmm17, 0x3fc0(%%r11, %%r10) \n" \
                "vmovntdq %%zmm18, 0x6640(%%r11, %%r10) \n" \
                "vmovntdq %%zmm19, 0x1f40(%%r11, %%r10) \n" \
                "vmovntdq %%zmm20, 0x3a00(%%r11, %%r10) \n" \
                "vmovntdq %%zmm21, 0x1080(%%r11, %%r10) \n" \
                "vmovntdq %%zmm22, 0x9c0(%%r11, %%r10) \n" \
                "vmovntdq %%zmm23, 0xf80(%%r11, %%r10) \n" \
                "vmovntdq %%zmm24, 0xcb00(%%r11, %%r10) \n" \
                "vmovntdq %%zmm25, 0x7e80(%%r11, %%r10) \n" \
                "vmovntdq %%zmm26, 0x99c0(%%r11, %%r10) \n" \
                "vmovntdq %%zmm27, 0x680(%%r11, %%r10) \n" \
                "vmovntdq %%zmm28, 0x12c0(%%r11, %%r10) \n" \
                "vmovntdq %%zmm29, 0x2880(%%r11, %%r10) \n" \
                "vmovntdq %%zmm30, 0xd140(%%r11, %%r10) \n" \
                "vmovntdq %%zmm31, 0xf400(%%r11, %%r10) \n"

/* temporal load */
#define SIZELD_1024_AVX512	\
				"vmovdqa64  0x0(%%r9, %%r10), %%zmm0 \n" \
				"vmovdqa64  0x40(%%r9, %%r10), %%zmm1 \n" \
				"vmovdqa64  0x80(%%r9, %%r10), %%zmm2 \n" \
				"vmovdqa64  0xc0(%%r9, %%r10), %%zmm3 \n" \
				"vmovdqa64  0x100(%%r9, %%r10), %%zmm4 \n" \
				"vmovdqa64  0x140(%%r9, %%r10), %%zmm5 \n" \
				"vmovdqa64  0x180(%%r9, %%r10), %%zmm6 \n" \
				"vmovdqa64  0x1c0(%%r9, %%r10), %%zmm7 \n" \
				"vmovdqa64  0x200(%%r9, %%r10), %%zmm8 \n" \
				"vmovdqa64  0x240(%%r9, %%r10), %%zmm9 \n" \
				"vmovdqa64  0x280(%%r9, %%r10), %%zmm10 \n" \
				"vmovdqa64  0x2c0(%%r9, %%r10), %%zmm11 \n" \
				"vmovdqa64  0x300(%%r9, %%r10), %%zmm12 \n" \
				"vmovdqa64  0x340(%%r9, %%r10), %%zmm13 \n" \
				"vmovdqa64  0x380(%%r9, %%r10), %%zmm14 \n" \
				"vmovdqa64  0x3c0(%%r9, %%r10), %%zmm15 \n" \
				"add $0x400, %%r10 \n"

#define LD_x1_RAND_AVX512 \
                "vmovdqa64 0x4140(%%r11, %%r10), %%zmm0 \n"

#define LD_x8_RAND_AVX512 \
                "vmovdqa64 0x4140(%%r11, %%r10), %%zmm0 \n" \
                "vmovdqa64 0xf340(%%r11, %%r10), %%zmm1 \n" \
                "vmovdqa64 0x2640(%%r11, %%r10), %%zmm2 \n" \
                "vmovdqa64 0x1000(%%r11, %%r10), %%zmm3 \n" \
                "vmovdqa64 0xda40(%%r11, %%r10), %%zmm4 \n" \
                "vmovdqa64 0x5200(%%r11, %%r10), %%zmm5 \n" \
                "vmovdqa64 0x180(%%r11, %%r10), %%zmm6 \n" \
                "vmovdqa64 0xf3c0(%%r11, %%r10), %%zmm7 \n"

#define LD_LFENCE_x16_RAND_AVX512 \
                "vmovdqa64 0x2a80(%%r11, %%r10), %%zmm0 \n lfence \n" \
                "vmovdqa64 0x680(%%r11, %%r10), %%zmm1 \n lfence \n" \
                "vmovdqa64 0x8500(%%r11, %%r10), %%zmm2 \n lfence \n" \
                "vmovdqa64 0x8980(%%r11, %%r10), %%zmm3 \n lfence \n" \
                "vmovdqa64 0x6d40(%%r11, %%r10), %%zmm4 \n lfence \n" \
                "vmovdqa64 0xf7c0(%%r11, %%r10), %%zmm5 \n lfence \n" \
                "vmovdqa64 0x4640(%%r11, %%r10), %%zmm6 \n lfence \n" \
                "vmovdqa64 0x1480(%%r11, %%r10), %%zmm7 \n lfence \n" \
                "vmovdqa64 0x2f00(%%r11, %%r10), %%zmm8 \n lfence \n" \
                "vmovdqa64 0x15c0(%%r11, %%r10), %%zmm9 \n lfence \n" \
                "vmovdqa64 0xf100(%%r11, %%r10), %%zmm10 \n lfence \n" \
                "vmovdqa64 0x66c0(%%r11, %%r10), %%zmm11 \n lfence \n" \
                "vmovdqa64 0xe240(%%r11, %%r10), %%zmm12 \n lfence \n" \
                "vmovdqa64 0xf480(%%r11, %%r10), %%zmm13 \n lfence \n" \
                "vmovdqa64 0x84c0(%%r11, %%r10), %%zmm14 \n lfence \n" \
                "vmovdqa64 0xe480(%%r11, %%r10), %%zmm15 \n lfence \n"

#define LD_x16_RAND_AVX512 \
                "vmovdqa64 0xc300(%%r11, %%r10), %%zmm0 \n" \
                "vmovdqa64 0xda00(%%r11, %%r10), %%zmm1 \n" \
                "vmovdqa64 0x1980(%%r11, %%r10), %%zmm2 \n" \
                "vmovdqa64 0xddc0(%%r11, %%r10), %%zmm3 \n" \
                "vmovdqa64 0xaa00(%%r11, %%r10), %%zmm4 \n" \
                "vmovdqa64 0x5540(%%r11, %%r10), %%zmm5 \n" \
                "vmovdqa64 0x6740(%%r11, %%r10), %%zmm6 \n" \
                "vmovdqa64 0x5a80(%%r11, %%r10), %%zmm7 \n" \
                "vmovdqa64 0xa680(%%r11, %%r10), %%zmm8 \n" \
                "vmovdqa64 0xdb00(%%r11, %%r10), %%zmm9 \n" \
                "vmovdqa64 0x3340(%%r11, %%r10), %%zmm10 \n" \
                "vmovdqa64 0x7e40(%%r11, %%r10), %%zmm11 \n" \
                "vmovdqa64 0x3600(%%r11, %%r10), %%zmm12 \n" \
                "vmovdqa64 0x5080(%%r11, %%r10), %%zmm13 \n" \
                "vmovdqa64 0x6e00(%%r11, %%r10), %%zmm14 \n" \
                "vmovdqa64 0x1540(%%r11, %%r10), %%zmm15 \n"

#define LD_x32_RAND_AVX512 \
                "vmovdqa64 0x7b40(%%r11, %%r10), %%zmm0 \n" \
                "vmovdqa64 0x7640(%%r11, %%r10), %%zmm1 \n" \
                "vmovdqa64 0xdf00(%%r11, %%r10), %%zmm2 \n" \
                "vmovdqa64 0xdb40(%%r11, %%r10), %%zmm3 \n" \
                "vmovdqa64 0xb6c0(%%r11, %%r10), %%zmm4 \n" \
                "vmovdqa64 0x6980(%%r11, %%r10), %%zmm5 \n" \
                "vmovdqa64 0xf280(%%r11, %%r10), %%zmm6 \n" \
                "vmovdqa64 0x3dc0(%%r11, %%r10), %%zmm7 \n" \
                "vmovdqa64 0x6d80(%%r11, %%r10), %%zmm8 \n" \
                "vmovdqa64 0xf580(%%r11, %%r10), %%zmm9 \n" \
                "vmovdqa64 0xf300(%%r11, %%r10), %%zmm10 \n" \
                "vmovdqa64 0x3140(%%r11, %%r10), %%zmm11 \n" \
                "vmovdqa64 0x8980(%%r11, %%r10), %%zmm12 \n" \
                "vmovdqa64 0xecc0(%%r11, %%r10), %%zmm13 \n" \
                "vmovdqa64 0xc5c0(%%r11, %%r10), %%zmm14 \n" \
                "vmovdqa64 0x1e40(%%r11, %%r10), %%zmm15 \n" \
                "vmovdqa64 0xf3c0(%%r11, %%r10), %%zmm16 \n" \
                "vmovdqa64 0xe800(%%r11, %%r10), %%zmm17 \n" \
                "vmovdqa64 0x2200(%%r11, %%r10), %%zmm18 \n" \
                "vmovdqa64 0x66c0(%%r11, %%r10), %%zmm19 \n" \
                "vmovdqa64 0xc00(%%r11, %%r10), %%zmm20 \n" \
                "vmovdqa64 0x2bc0(%%r11, %%r10), %%zmm21 \n" \
                "vmovdqa64 0x6a80(%%r11, %%r10), %%zmm22 \n" \
                "vmovdqa64 0x94c0(%%r11, %%r10), %%zmm23 \n" \
                "vmovdqa64 0xbec0(%%r11, %%r10), %%zmm24 \n" \
                "vmovdqa64 0xcdc0(%%r11, %%r10), %%zmm25 \n" \
                "vmovdqa64 0xf80(%%r11, %%r10), %%zmm26 \n" \
                "vmovdqa64 0xc000(%%r11, %%r10), %%zmm27 \n" \
                "vmovdqa64 0x4340(%%r11, %%r10), %%zmm28 \n" \
                "vmovdqa64 0x4640(%%r11, %%r10), %%zmm29 \n" \
                "vmovdqa64 0xcc0(%%r11, %%r10), %%zmm30 \n" \
                "vmovdqa64 0x6b40(%%r11, %%r10), %%zmm31 \n"

#define STWB_x1_RAND_AVX512 \
                "vmovdqa64  %%zmm0,  0xe80(%%r11, %%r10) \n clwb 0xe80(%%r11, %%r10) \n"

#define STWB_x8_RAND_AVX512 \
                "vmovdqa64  %%zmm0,  0xe80(%%r11, %%r10) \n clwb 0xe80(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm1,  0xe4c0(%%r11, %%r10) \n clwb 0xe4c0(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm2,  0x4780(%%r11, %%r10) \n clwb 0x4780(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm3,  0xc240(%%r11, %%r10) \n clwb 0xc240(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm4,  0x2e00(%%r11, %%r10) \n clwb 0x2e00(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm5,  0xf4c0(%%r11, %%r10) \n clwb 0xf4c0(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm6,  0xe5c0(%%r11, %%r10) \n clwb 0xe5c0(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm7,  0x7040(%%r11, %%r10) \n clwb 0x7040(%%r11, %%r10) \n"

//#define STWB_SFENCE_x16_RAND_AVX512

#define STWB_x16_RAND_AVX512 \
                "vmovdqa64  %%zmm0,  0x28c0(%%r11, %%r10) \n clwb 0x28c0(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm1,  0xc880(%%r11, %%r10) \n clwb 0xc880(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm2,  0x3cc0(%%r11, %%r10) \n clwb 0x3cc0(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm3,  0xdd40(%%r11, %%r10) \n clwb 0xdd40(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm4,  0x6bc0(%%r11, %%r10) \n clwb 0x6bc0(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm5,  0xe600(%%r11, %%r10) \n clwb 0xe600(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm6,  0x1c0(%%r11, %%r10) \n clwb 0x1c0(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm7,  0xf540(%%r11, %%r10) \n clwb 0xf540(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm8,  0x11c0(%%r11, %%r10) \n clwb 0x11c0(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm9,  0xb000(%%r11, %%r10) \n clwb 0xb000(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm10,  0x3f80(%%r11, %%r10) \n clwb 0x3f80(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm11,  0x5c40(%%r11, %%r10) \n clwb 0x5c40(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm12,  0xed00(%%r11, %%r10) \n clwb 0xed00(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm13,  0xd600(%%r11, %%r10) \n clwb 0xd600(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm14,  0x4c80(%%r11, %%r10) \n clwb 0x4c80(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm15,  0xb280(%%r11, %%r10) \n clwb 0xb280(%%r11, %%r10) \n"

/* temporal store */
#define STWB_x32_RAND_AVX512 \
                "vmovdqa64  %%zmm0,  0x9c0(%%r11, %%r10) \n clwb 0x9c0(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm1,  0x3b40(%%r11, %%r10) \n clwb 0x3b40(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm2,  0xe540(%%r11, %%r10) \n clwb 0xe540(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm3,  0xe180(%%r11, %%r10) \n clwb 0xe180(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm4,  0x2b80(%%r11, %%r10) \n clwb 0x2b80(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm5,  0xa380(%%r11, %%r10) \n clwb 0xa380(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm6,  0x9ac0(%%r11, %%r10) \n clwb 0x9ac0(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm7,  0xd500(%%r11, %%r10) \n clwb 0xd500(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm8,  0x51c0(%%r11, %%r10) \n clwb 0x51c0(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm9,  0x99c0(%%r11, %%r10) \n clwb 0x99c0(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm10,  0xacc0(%%r11, %%r10) \n clwb 0xacc0(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm11,  0x4900(%%r11, %%r10) \n clwb 0x4900(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm12,  0x3540(%%r11, %%r10) \n clwb 0x3540(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm13,  0x8ac0(%%r11, %%r10) \n clwb 0x8ac0(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm14,  0x2580(%%r11, %%r10) \n clwb 0x2580(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm15,  0xc5c0(%%r11, %%r10) \n clwb 0xc5c0(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm16,  0xfd40(%%r11, %%r10) \n clwb 0xfd40(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm17,  0xac40(%%r11, %%r10) \n clwb 0xac40(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm18,  0x1240(%%r11, %%r10) \n clwb 0x1240(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm19,  0xa00(%%r11, %%r10) \n clwb 0xa00(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm20,  0x53c0(%%r11, %%r10) \n clwb 0x53c0(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm21,  0xcd00(%%r11, %%r10) \n clwb 0xcd00(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm22,  0xbac0(%%r11, %%r10) \n clwb 0xbac0(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm23,  0x2500(%%r11, %%r10) \n clwb 0x2500(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm24,  0xd300(%%r11, %%r10) \n clwb 0xd300(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm25,  0xba40(%%r11, %%r10) \n clwb 0xba40(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm26,  0xf500(%%r11, %%r10) \n clwb 0xf500(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm27,  0x2080(%%r11, %%r10) \n clwb 0x2080(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm28,  0xf2c0(%%r11, %%r10) \n clwb 0xf2c0(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm29,  0xa980(%%r11, %%r10) \n clwb 0xa980(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm30,  0x8880(%%r11, %%r10) \n clwb 0x8880(%%r11, %%r10) \n" \
                "vmovdqa64  %%zmm31,  0x54c0(%%r11, %%r10) \n clwb 0x54c0(%%r11, %%r10) \n"

#define SIZESTWB_1024_AVX512	\
				"vmovdqa64  %%zmm0,  0x0(%%r9, %%r10) \n" \
                "clwb  0x0(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x40(%%r9, %%r10) \n" \
                "clwb  0x40(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x80(%%r9, %%r10) \n" \
                "clwb  0x80(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0xc0(%%r9, %%r10) \n" \
                "clwb  0xc0(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x100(%%r9, %%r10) \n" \
                "clwb  0x100(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x140(%%r9, %%r10) \n" \
                "clwb  0x140(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x180(%%r9, %%r10) \n" \
                "clwb  0x180(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x1c0(%%r9, %%r10) \n" \
                "clwb  0x1c0(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x200(%%r9, %%r10) \n" \
                "clwb  0x200(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x240(%%r9, %%r10) \n" \
                "clwb  0x240(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x280(%%r9, %%r10) \n" \
                "clwb  0x280(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x2c0(%%r9, %%r10) \n" \
                "clwb  0x2c0(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x300(%%r9, %%r10) \n" \
                "clwb  0x300(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x340(%%r9, %%r10) \n" \
                "clwb  0x340(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x380(%%r9, %%r10) \n" \
                "clwb  0x380(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x3c0(%%r9, %%r10) \n" \
                "clwb  0x3c0(%%r9, %%r10) \n" \
				"add $0x400, %%r10 \n"

#define SIZEST_1024_AVX512	\
				"vmovdqa64  %%zmm0,  0x0(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm1,  0x40(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm2,  0x80(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm3,  0xc0(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm4,  0x100(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm5,  0x140(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm6,  0x180(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm7,  0x1c0(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm8,  0x200(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm9,  0x240(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm10,  0x280(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm11,  0x2c0(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm12,  0x300(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm13,  0x340(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm14,  0x380(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm15,  0x3c0(%%r9, %%r10) \n" \
				"add $0x400, %%r10 \n"

/* perform movdir64B */
#define SIZEMOV_1024 \
                "movdir64b 0x0(%%r9, %%r10), %%r12 \n" \
                "add $0x40, %%r12 \n" \
                "movdir64b 0x40(%%r9, %%r10), %%r12\n" \
                "add $0x40, %%r12 \n" \
                "movdir64b 0x80(%%r9, %%r10), %%r12\n" \
                "add $0x40, %%r12 \n" \
                "movdir64b 0xc0(%%r9, %%r10), %%r12\n" \
                "add $0x40, %%r12 \n" \
                "movdir64b 0x100(%%r9, %%r10), %%r12\n" \
                "add $0x40, %%r12 \n" \
                "movdir64b 0x140(%%r9, %%r10), %%r12\n" \
                "add $0x40, %%r12 \n" \
                "movdir64b 0x180(%%r9, %%r10), %%r12\n" \
                "add $0x40, %%r12 \n" \
                "movdir64b 0x1c0(%%r9, %%r10), %%r12\n" \
                "add $0x40, %%r12 \n" \
                "movdir64b 0x200(%%r9, %%r10), %%r12\n" \
                "add $0x40, %%r12 \n" \
                "movdir64b 0x240(%%r9, %%r10), %%r12\n" \
                "add $0x40, %%r12 \n" \
                "movdir64b 0x280(%%r9, %%r10), %%r12\n" \
                "add $0x40, %%r12 \n" \
                "movdir64b 0x2c0(%%r9, %%r10), %%r12\n" \
                "add $0x40, %%r12 \n" \
                "movdir64b 0x300(%%r9, %%r10), %%r12\n" \
                "add $0x40, %%r12 \n" \
                "movdir64b 0x340(%%r9, %%r10), %%r12\n" \
                "add $0x40, %%r12 \n" \
                "movdir64b 0x380(%%r9, %%r10), %%r12\n" \
                "add $0x40, %%r12 \n" \
                "movdir64b 0x3c0(%%r9, %%r10), %%r12\n" \
                "add $0x40, %%r12 \n" \
				"add $0x400, %%r10 \n" \

/* Mixed read and write */
/* try using the same dest reg. Assign some value to zmm0 for storing. */
#define SIZE_R1W1_512 \
				"vmovdqa64  0x0(%%r9, %%r10), %%zmm0 \n" \
				"vmovdqa64  0x40(%%r9, %%r10), %%zmm1 \n" \
				"vmovdqa64  0x80(%%r9, %%r10), %%zmm2 \n" \
				"vmovdqa64  0xc0(%%r9, %%r10), %%zmm3 \n" \
				"" \
				"vmovq		%1, %%xmm0 \n" \
				"vmovdqa64  %%zmm0,  0x0(%%r9, %%r10) \n" \
				"clwb  0x0(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x40(%%r9, %%r10) \n" \
				"clwb  0x40(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x80(%%r9, %%r10) \n" \
				"clwb  0x80(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0xc0(%%r9, %%r10) \n" \
				"clwb  0xc0(%%r9, %%r10) \n" \
				"add $0x200, %%r10 \n" \


#define SIZE_R2W1_576 \
				"vmovdqa64  0x0(%%r9, %%r10), %%zmm0 \n" \
				"vmovdqa64  0x40(%%r9, %%r10), %%zmm0 \n" \
				"vmovdqa64  0x80(%%r9, %%r10), %%zmm0 \n" \
				"vmovdqa64  0xc0(%%r9, %%r10), %%zmm0 \n" \
				"vmovdqa64  0x100(%%r9, %%r10), %%zmm0 \n" \
				"vmovdqa64  0x140(%%r9, %%r10), %%zmm0 \n" \
				"vmovq	%1, %%xmm1 \n" \
				"vmovdqa64  %%zmm1,  0x0(%%r9, %%r10) \n" \
				"clwb  0x0(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm1,  0x40(%%r9, %%r10) \n" \
				"clwb  0x40(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm1,  0x80(%%r9, %%r10) \n" \
				"clwb  0x80(%%r9, %%r10) \n" \
				"add $0x180, %%r10 \n" \


#define SIZE_R2W1_384 \
				"vmovdqa64  0x0(%%r9, %%r10), %%zmm0 \n" \
				"vmovdqa64  0x40(%%r9, %%r10), %%zmm1 \n" \
				"vmovdqa64  0x80(%%r9, %%r10), %%zmm2 \n" \
				"vmovdqa64  0xc0(%%r9, %%r10), %%zmm3 \n" \
				"vmovq	%1, %%xmm0 \n" \
				"vmovdqa64  %%zmm0,  0x0(%%r9, %%r10) \n" \
				"clwb  0x0(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x40(%%r9, %%r10) \n" \
				"clwb  0x40(%%r9, %%r10) \n" \
				"add $0x180, %%r10 \n" \


#define SIZE_R3W1_512 \
				"vmovdqa64  0x0(%%r9, %%r10), %%zmm0 \n" \
				"vmovdqa64  0x40(%%r9, %%r10), %%zmm1 \n" \
				"vmovdqa64  0x80(%%r9, %%r10), %%zmm2 \n" \
				"vmovdqa64  0xc0(%%r9, %%r10), %%zmm3 \n" \
				"vmovdqa64  0x100(%%r9, %%r10), %%zmm4 \n" \
				"vmovdqa64  0x140(%%r9, %%r10), %%zmm5 \n" \
				"" \
				"vmovq		%1, %%xmm0 \n" \
				"vmovdqa64  %%zmm0,  0x0(%%r9, %%r10) \n" \
				"clwb  0x0(%%r9, %%r10) \n" \
				"vmovdqa64  %%zmm0,  0x40(%%r9, %%r10) \n" \
				"clwb  0x40(%%r9, %%r10) \n" \
				"add $0x200, %%r10 \n" \

/* snippets for latency measuring */

/* Assembly instructions utilize the following registers:
 * rsi: memory address
 * rax, rdx, rcx, r8d and r9d: timing
 * rdx: populating cache-lines
 * ymm0: streaming instructions
 */
#define REGISTERS "rsi", "rax", "rdx", "rcx", "r8", "r9", "ymm0"

#define REGISTERS_AND_ZMM "rsi", "rax", "rdx", "rcx", "r8", "r9", \

#define ZMM_0_15 "zmm0", "zmm1", "zmm2", "zmm3", "zmm4", "zmm5", "zmm6", "zmm7", "zmm8", "zmm9", "zmm10", "zmm11", "zmm12", "zmm13", "zmm14", "zmm15"


/* rdtscp: reads current timestamp to EDX:EAX and also sets ECX
 * 		   higher 32-bits of RAX, RDX and RCX are cleared
 *
 * r9d = old EDX
 * r8d = old EAX
 * Here is what we do to compute t_start and t_end:
 * - RDX holds t_end
 * - RAX holds t_start
 */

/** Douglas: read this blog for more info about timing 
 * 	http://sites.utexas.edu/jdm4372/2018/07/23/comments-on-timing-short-code-sections-on-intel-processors/
*/
#define TIMING_BEGIN      	"rdtscp \n" \
							"lfence \n" \
							"mov %%edx, %%r9d \n" \
							"mov %%eax, %%r8d \n"

#define TIMING_END      	"mfence \n" \
							"rdtscp \n" \
							"lfence \n" \
							"shl $32, %%rdx \n" \
							"or  %%rax, %%rdx \n" \
							"mov %%r9d, %%eax \n" \
							"shl $32, %%rax \n" \
							"or  %%r8, %%rax \n" \
							"mov %%rax, %[t_start] \n" \
							"mov %%rdx, %[t_end] \n"

#define FLUSH_64K_BLOCK \
                            "LOOP_64K_BLOCK_FLUSH: \n" \
                                "clflush (%%r11, %%r10) \n" \
                                "add $0x40, %%r10 \n" \
                                "cmp $0x10000, %%r10 \n" \
                                "jl LOOP_64K_BLOCK_FLUSH\n" \
                            "xor %%r10, %%r10 \n" \
							"mfence \n"


#define FLUSH_CACHE_LINE    "clflush 0*32(%%rsi) \n" \
							"clflush 2*32(%%rsi) \n" \
							"clflush 4*32(%%rsi) \n" \
							"clflush 6*32(%%rsi) \n" \
							"mfence \n"


#define CLEAR_PIPELINE      "nop \nnop \nnop \nnop \nnop \nnop \n" \
							"nop \nnop \nnop \nnop \nnop \nnop \n" \
							"nop \nnop \nnop \nnop \nnop \nnop \n" \
							"nop \nnop \nnop \nnop \nnop \nnop \n" \
							"nop \nnop \nnop \nnop \nnop \nnop \n" \
							"nop \nnop \nnop \nnop \nnop \nnop \n" \
							"nop \nnop \nnop \nnop \nnop \nnop \n" \
							"nop \nnop \nnop \nnop \nnop \nnop \n" \
							"nop \nnop \nnop \nnop \nnop \nnop \n" \
							"nop \nnop \nnop \nnop \nnop \nnop \n" \
							"nop \nnop \nnop \nnop \nnop \nnop \n" \
							"nop \nnop \nnop \nnop \nnop \nnop \n" \
							"nop \nnop \nnop \nnop \nnop \nnop \n" \
							"nop \nnop \nnop \nnop \nnop \nnop \n" \
							"nop \nnop \nnop \nnop \nnop \nnop \n" \
							"nop \nnop \nnop \nnop \nnop \nnop \n" 

#define CLEAR_PIPELINE_x16  CLEAR_PIPELINE \
            CLEAR_PIPELINE \
            CLEAR_PIPELINE \
            CLEAR_PIPELINE \
            CLEAR_PIPELINE \
            CLEAR_PIPELINE \
            CLEAR_PIPELINE \
            CLEAR_PIPELINE \
            CLEAR_PIPELINE \
            CLEAR_PIPELINE \
            CLEAR_PIPELINE \
            CLEAR_PIPELINE \
            CLEAR_PIPELINE \
            CLEAR_PIPELINE \
            CLEAR_PIPELINE \
            CLEAR_PIPELINE

#endif // WORKLOAD_H
