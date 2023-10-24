ITERATION=10000

bash ../../util_scripts/config_all.sh
source ../../util_scripts/env.sh

test_block_lats() {
    echo "[INFO] Test started"

    for ((k=0;k<=$NODE_MAX;k=k+1)); do # node
        for ((j=0;j<4;j=j+1)); do # op
            FOLDER_NAME=block_lats_n${k}
            CURR_RESULT_PATH=../results/$FOLDER_NAME
            mkdir -p $CURR_RESULT_PATH

            #echo "[TEST] $i $j $k......"
            echo "[TEST] op: $j node: $k......"
            LINE=`sudo ../src/cxlMemTest -p $CLOSEST_CORE -t 1 -S 1 -n $k -T 3 -o $j -i $ITERATION -B -F $TSC_FREQ | awk '/Median latency/ {print}'`
            echo $LINE

            LATS=`echo $LINE | awk '{print $(NF-2)}' | grep -Eo '[+-]?[0-9]+([.][0-9]+)?'`
            echo $LATS >> $CURR_RESULT_PATH/block_lats_n${k}.txt
            echo $LATS
        done
    done
}

test_block_lats
