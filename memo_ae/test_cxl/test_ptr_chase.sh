#!/bin/bash
STEPS=19

bash ../../util_scripts/config_all.sh
source ../../util_scripts/env.sh

for ((j=0;j<=$NODE_MAX;j++)); do
    FOLDER_NAME=chase_${j}_test
    mkdir -p ../results/$FOLDER_NAME

    # Start testing
    echo "[INFO] Test started"
    size=4096
    iter=500
    # max = 2^12 * 2^18 = 2^30 ~ 4GB
    for ((i=0;i<$STEPS;i++)); do
        if [ $((i%3)) -eq 1 ];
        then
            iter=$((iter / 2))
        fi
        echo -n "[TEST] test $i, iteration $iter......"
        LATENCY=`sudo ../src/cxlMemTest -t 1 -m $size -o 0 -T 2 -n $j -i $iter -p $CLOSEST_CORE -F $TSC_FREQ | awk '/chase\/block_lats average/ {print $8}' | tail -n1 | grep -Eo '[+-]?[0-9]+([.][0-9]+)?'`
        echo $LATENCY >> ../results/$FOLDER_NAME/ptr_chase_lat_vs_size.txt
        echo "$LATENCY"
        
        size=$((size*2))
    done
done
