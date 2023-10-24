import random

CNT=16
RANGE_LOW=0
RANGE_HIGH=1024
# NTLD
#inst_base = '"vmovntdqa  {0}(%%r11, %%r10), %%zmm{1} \\n" \\'
# NTST
#inst_base = '"vmovntdq %%zmm{1}, {0}(%%r11, %%r10) \\n" \\'
# LD
#inst_base = '"vmovdqa64 {0}(%%r11, %%r10), %%zmm{1} \\n lfence \\n" \\'
#inst_base = '"vmovdqa64 {0}(%%r11, %%r10), %%zmm{1} \\n \\'
# ST + WB
#inst_base = '"vmovdqa64  %%zmm{1},  {0}(%%r11, %%r10) \\n clwb {0}(%%r11, %%r10) \\n" \\'
# ST 
inst_base = '"vmovdqa64  %%zmm{1},  {0}(%%r11, %%r10) \\n" \\'

seen_set = set()

for i in range(CNT):
    curr_offset = random.randrange(RANGE_LOW, RANGE_HIGH)
    while curr_offset in seen_set:
        curr_offset = random.randrange(RANGE_LOW, RANGE_HIGH)
        
    seen_set.add(curr_offset)

    print(inst_base.format(hex(curr_offset << 6), i))
