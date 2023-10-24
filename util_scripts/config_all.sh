#!bin/bash

SETUP_SCRIPT_DIR=$(cd $(dirname $0) && pwd)

# set
bash $SETUP_SCRIPT_DIR/lock_cpu_freq.sh
bash $SETUP_SCRIPT_DIR/check_cpu_freq.sh
bash $SETUP_SCRIPT_DIR/hyperthread_ctrl.sh 0
bash $SETUP_SCRIPT_DIR/numa_balance_ctrl.sh 0

sudo systemctl stop numad
