#!/bin/bash
NUM_THREADS=32
STEP=2
ITERATION=3
OP_MAX=3

bash ../../util_scripts/config_all.sh
source ../../util_scripts/env.sh

FOLDER_NAME=movdir_bw_test
CURR_RESULT_PATH=../results/$FOLDER_NAME
mkdir -p $CURR_RESULT_PATH

for ((src=0;src<=$NODE_MAX;src++)); do
    for ((dst=0;dst<=$NODE_MAX;dst++)); do
        for ((i=0;i<=$NUM_THREADS;i=i+$STEP)); do
            if [ $i == 0 ];then
                continue
            fi
            echo "[TEST] src: $src dst: $dst, num_thread: $i......"
            THROUGHPUT=`sudo ../src/cxlMemTest -p $CLOSEST_CORE -f -t $i -S 6 -n $src -d $dst -T 1 -o 4 -i $ITERATION | awk '/get_bw/ {print}'` 
            BW=`echo $THROUGHPUT | awk '{print $(NF-1)}'`
            echo $BW >> $CURR_RESULT_PATH/s${src}_d${dst}.txt
            echo $THROUGHPUT
        done
    done
done
