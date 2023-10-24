#!bin/bash

if [ $1 -eq 2 ];
then
    echo "enable numa balance in tpp mode"
    sudo sh -c "echo 2 > /proc/sys/kernel/numa_balancing"  
fi

if [ $1 -eq 1 ];
then
    echo "enable numa balance"
    sudo sh -c "echo 1 > /proc/sys/kernel/numa_balancing"  
fi

if [ $1 -eq 0 ];
then
    echo "disable numa balance"
    sudo sh -c "echo 0 > /proc/sys/kernel/numa_balancing"  
fi

