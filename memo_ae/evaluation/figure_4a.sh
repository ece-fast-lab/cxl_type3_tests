bash ../../util_scripts/config_all.sh
source ../../util_scripts/env.sh

# config for SPR1
CXL_NODE="2"
NUMA_REMOTE_NODE="1"
CLOSEST_CORE_S="0-31"

# arg1: mem node
# arg2: cores
test_mlc_bw() {
    echo "running mlc, node: ${1}, cores: ${2} ..."
    sudo numactl --membind=$1 ../app/mlc_linux/mlc --peak_injection_bandwidth -k$2 -b104858 > ../results/figure_4a/c${2}_m${1}.txt

    echo "done!"
}


mkdir -p ../results/figure_4a
echo "testing with MLC peak injection bw... "

test_mlc_bw $CXL_NODE $CLOSEST_CORE_S
test_mlc_bw $NUMA_REMOTE_NODE $CLOSEST_CORE_S
