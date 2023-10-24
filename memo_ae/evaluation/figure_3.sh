ITERATION=10000

bash ../../util_scripts/config_all.sh
source ../../util_scripts/env.sh

test_block_lats() {
    echo "[INFO] Test started"
    echo "CLOSEST_NODE: $CLOSEST_NODE"
    echo "CLOSEST_CORE: $CLOSEST_CORE"
    echo "TSC_FREQ: $TSC_FREQ"
    FOLDER_NAME="figure_3_memo"

    for ((k=0;k<=$NODE_MAX;k=k+1)); do # node
        for ((j=0;j<4;j=j+1)); do # op
            CURR_RESULT_PATH=../results/$FOLDER_NAME
            mkdir -p $CURR_RESULT_PATH

            #echo "[TEST] $i $j $k......"
            echo "[TEST] op: $j node: $k, core: $CLOSEST_CORE......"
            LINE=`sudo ../src/cxlMemTest -p $CLOSEST_CORE -t 1 -S 1 -n $k -T 3 -o $j -i $ITERATION -B -F $TSC_FREQ | awk '/Median latency/ {print}'`
            echo $LINE

            LATS=`echo $LINE | awk '{print $(NF-2)}' | grep -Eo '[+-]?[0-9]+([.][0-9]+)?'`
            echo $LATS >> $CURR_RESULT_PATH/block_lats_n${k}.txt
            echo $LATS
        done
    done
}

test_mlc_lats() {
    echo "running mlc ..."
    sudo ../app/mlc_linux/mlc --latency_matrix > ../results/figure_3_mlc/mlc.txt

    echo "done!"
}

mkdir -p ../results/figure_3_mlc
echo "testing with MLC ... "
test_mlc_lats

echo "testing with MEMO ... "
test_block_lats
