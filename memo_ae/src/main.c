/*
 * Developed by FAST Lab @ ECE-UIUC -- 2022-2023
 */
#include "util.h"
#include "test.h"
#include <stdlib.h>
#include <stdio.h>
#include <numa.h>
#include <numaif.h>

int main(int argc, char*argv[]) {
    int ret;
    test_cfg_t* cfg;
    cfg = malloc(sizeof(test_cfg_t));

    ret = parse_arg(argc, argv, cfg);
    if (ret < 0) {
        if (ret == -1) {
            printf("BAD parse_arg\n");
        }
        goto out;
    }

    ret = init_buf(cfg->total_buf_size, cfg->buf_a_numa_node, &(cfg->buf_a));
    if (ret < 0) {
        if (ret == -1) {
            printf("BAD init_buf buf_a, fail to alloc\n");
            goto out;
        } else { // already alloc, needs to free
            printf("BAD init_buf buf_a, alloc strange\n");
            goto out1;
        }
    }

    if (cfg->op == MOV) {
        ret = init_buf(cfg->total_buf_size, cfg->buf_b_numa_node, &(cfg->buf_b));
        if (ret < 0) {
            if (ret == -1) {
                printf("BAD init_buf buf_b, fail to alloc\n");
                goto out1; // free buf_a
            } else { // already alloc, needs to free
                printf("BAD init_buf buf_b, alloc strange\n");
                goto out2; // free buf_b then buf_a
            }
        }
    }

    ret = run_test(cfg);

    ret = get_node(cfg->buf_a, cfg->total_buf_size);
    printf("end, buf_a is on node %d\n", ret);

    if (cfg->op == MOV) {
        ret = get_node(cfg->buf_b, cfg->total_buf_size);
        printf("end, buf_b is on node %d\n", ret);
    }

out2:
    numa_free(cfg->buf_b, cfg->total_buf_size);
out1:
    numa_free(cfg->buf_a, cfg->total_buf_size);
out:
    free(cfg);
    return 0;	
}
