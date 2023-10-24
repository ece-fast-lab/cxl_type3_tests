# memo benchmark

## Setup
### Prerequisite
- Must
    - Installing cpupower,turbostat
        - `sudo apt-get install -y linux-tools-$(uname -r)`
    - libnuma installation
        - `sudo apt-get install libnuma-dev`

### Clone & Build
```bash
git clone https://github.com/ece-fast-lab/cxl_type3_tests.git 
cd memo_ae/src
make
```

### Get Turbo stat
```bash
# under memo_ae 
cd ./test_cxl/
bash get_turbostat.sh
```

### Setup `env.sh`
```bash
# Use your favorite editor to open env.sh
vim ../util_scripts/env.sh
```
1. Set `CLOSEST_NODE`: The NUMA node that the CXL device is directly attached to.  
    - From mlc output in `results/mlc.txt`, find the NUMA node that has the lowest latency to CXL memory
    - Command `sudo lspci -vvv` should also show the NUMA node that a CXL device attaches to.
2. Set `CLOSEST_CORE`: This is one of the CPU cores within CLOSEST NODE 
    - Place the first core number of the `CLOSEST_NODE` in `CLOSEST_CORE`. You may find the core range of a NUMA node using this command: `numactl -H`. 
    - For example, if CXL is connected to NUMA node 1, please place the first CPU in `node 1 cpus: XX, XX+1` (XX) in `CLOSEST_CPU`. 
3. Set `TSC_FREQ`:
    - This should be the output from `Test Turbo stat` in `results/turbostat.txt`, it should look like this: 
```
Core	CPU	Avg_MHz	Busy%	Bzy_MHz	TSC_MHz	IPC	IRQ	SMI	POLL	C1	C1E	C3	C6	POLL%	C1%	C1E%	C3%	C6%	CPU%c1	CPU%c3	CPU%c6	CoreTmp	CoreThr	PkgTmp	Pkg%pc2	Pkg%pc3	Pkg%pc6	PkgWatt	RAMWatt	PKG_%	RAM_%
-	-	10	0.57	1753	2000	0.52	985	0	14	15	108	0	945	0.00	0.12	0.40	0.00	98.93	2.23	0.00	97.20	22	0	25	2.14	0.00	76.94	25.52	0.00	0.00	0.00
0	0	4	0.25	1802	2000	1.02	29	0	0	0	0	0	34	0.00	0.00	0.00	0.00	99.76	0.93	0.00	98.83	22	0	25	2.14	0.00	76.95	25.52	0.00	0.00	0.00
```
**Note:**
1. There should be a constant number for all cores, 2000MHz in the example above. Please set `TSC_FREQ` (unit = MHz) to 2000 if the number is 2000.
2. In most system, this should also be the 6th number in the second row of `results/turbostat.txt`

## Notes
* memo is only tested on AVX-enabled machines
* Single-op latency (`-T 0`) has a much higher absolute values than block access latency (`-T 3`).
* Block access is default to issue 16 accesses with randomly hard-coded offsets within a 64KB region.
    - You may play with `generate_random_inst.py` to generate a new set of random offsets.
    - You may change the MACROs in `src/workload.h`, `BLOCK_xN` and `*_xN_RAND_AVX512` to see how the number of parallel issue affects the average latency of each access.
* Setting the `-F` argument is critical for all latency measures. `-F` is not used for all bandwidth tests.
* Latency tests should always put `-t 1` for the thread count argument.

## Known issues
* Random pointer chasing is NOT implemented, i.e. running with `-T 2` with `-r`
    - Testing with pointer chasing should always pin to core with `-p <core>`, which defaults to run a sequential link list chasing with prefetching OFF on `<core>`
* The `-R` and `-o 5` options for read-write ratio are experimental.

## Arguments

| Argument | Brief description | Default | Valid inputs | Note |
| -------- | ----------------- | ------- | ------------ | ---- |
| t | Number of testing threads | 32 | 1 - X | This should be set to 1 for all latency tests |
| f | Prefetching enabled | disabled | -f | When `-p` is not specified, prefetch is NOT toggled. When `-p` is specified, default to prefetching disabled |
| m | Total buffer size in bytes | 2^30 | 32-bit integer | Anything larger than 2^30 should use the `-S` argument |
| S | Total buffer size in GiB | 1 GiB | 1 - total memory size on a NUMA node | / |
| n | Buffer NUMA node | 0 | 0 - (Number of NUMA node - 1) | When `-o 4` is specified, this argument is used as the source buffer of the move operation |
| d | Buffer NUMA node | 0 | 0 - (Number of NUMA node - 1) | When `-o 4` is specified, this argument is used as the destination buffer of the move operation. Otherwise, this argument is ignored. |
| s | Stall Ratio | 0 | 0 - X | This argument will be used in the bandwidth test where each block of accesses is accompanied by `-s` number of stall blocks. A stall block consists of 6 x 16 x 16 x 4 = 6144 `nop` instructions. |
| i | Iteration | 1 | 1 - X | For bandwidth tests, each iteration monitor the number of byte accessed across all thread in 0.5 second. For latency tests, each iteration is a single op / single block of accesses. For the pointer chasing test, each iteration is chasing through all cachelines in the specified buffer size. |
| T | Type of operation | 0 | 0 - 3 | <ul><li>0 = single-op latency</li><li>1 = bandwidth</li><li>2 = pointer chasing</li><li>3 = block-access latency|
| p | Pin to core | -1 (not pin to core) | 0 - number of cores | Pinning to core affects argument `-f` |
| a | Two core bandwidth test (core a) | -1 | 0 - number of cores | When testing with 2 thread bandwidth test, `-a` will specify the first core the thread should pin to. The policy for prefetching is aligned with `-p`.|
| b | Two core bandwidth test (core b) | -1 | 0 - number of cores | (Same with `-a`, this will pin the second thread)|
| g | Bandwidth test access block size | 512 | 16 - (per thread buff size / 64) | The MACROs for bandwidth tests are all 1024 Byte, thus the smallest stepping would be 16 cachlines |
| r | Random bandwidth test | disabled | -r | This argument will trigger the next block of access to increment by a somewhat random fashion. However, accesses within a block remains sequential |
| o | Operation | 0 (Load) | 0 - 4 | <ul><li>0 = Load</li><li>1 = NT-load</li><li>2 = Store</li><li>3 = Nt-store</li><li>4 = movdir64B (only in bandwidth tests)|
| B | Flush before block access latency test | NOT flushed | -B | Used only in `-T 3`, this argument decides whether the 64KB region to be access will be flushed. | 
| C | Number of `nop` blocks before block access latency test | 0 | 0 - X | Used only in `-T 3`, this argument decides how many blocks of `nop` should be issued after the cacheline flushes (if there's any) and before the test begines.  |
| F | TSC frequnecy | 2GHz | X | This value should **always** present for any latency tests. Please refer to the turbostat section for determining this value. | 


## Other profilings
Under `./test_cxl/`
#### Block access (fast) < 5 min
```
bash test_block_access_latency.sh 
```

#### Ptr chasing (fast) < 5 min
```
bash test_ptr_chase.sh 
```

#### Single operation latency (fast) < 5 min
```
bash test_single_op_latency.sh
```

#### `movdir64B` bandwidth (long long) > 15 min
```
bash test_movdir_bw.sh
```

#### Sequential access bandwidth (long long) > 15 min
```
bash test_seq_bw.sh
```

#### Random access bandwidth (long long) > 30 min
```
bash test_rand_bw.sh
```

## Results
All results are under the `results` folder. 


## Acknowledgement  
Some parts of this source code and the methodology are inspired by the marvalous work in this [publication(FAST20-Yang)](https://www.usenix.org/conference/fast20/presentation/yang) and this [reposiroty(OptaneStudy)](https://github.com/NVSL/OptaneStudy/tree/master).

