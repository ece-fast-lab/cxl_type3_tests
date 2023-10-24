#!bin/bash

SETUP_SCRIPT_DIR=./

# set
bash $SETUP_SCRIPT_DIR/unlock_cpu_freq.sh
bash $SETUP_SCRIPT_DIR/check_cpu_freq.sh
bash $SETUP_SCRIPT_DIR/hyperthread_ctrl.sh 1
bash $SETUP_SCRIPT_DIR/numa_balance_ctrl.sh 1
