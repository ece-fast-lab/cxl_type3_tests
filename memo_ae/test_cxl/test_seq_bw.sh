#!/bin/bash
NUM_THREADS=32
STEP=2
ITERATION=3
OP_MAX=3

bash ../../util_scripts/config_all.sh
source ../../util_scripts/env.sh

for ((j=0;j<=$NODE_MAX;j++)); do
    FOLDER_NAME=seq_bw_${j}_test
    CURR_RESULT_PATH=../results/$FOLDER_NAME
    mkdir -p $CURR_RESULT_PATH

    for ((k=0;k<=$OP_MAX;k++)); do

        for ((i=0;i<=$NUM_THREADS;i=i+$STEP)); do
            if [ $i == 0 ];then
                continue
            fi

            echo "[TEST] node: $j, op: $k, num_thread: $i......"
            THROUGHPUT=`sudo ../src/cxlMemTest -p $CLOSEST_CORE -f -t $i -S 6 -n $j -d $j -T 1 -o $k -i $ITERATION | awk '/get_bw/ {print}'` 
            BW=`echo $THROUGHPUT | awk '{print $(NF-1)}'`
            echo $BW >> $CURR_RESULT_PATH/seq_bw_$k.txt
            echo $THROUGHPUT
        done
    done
done
