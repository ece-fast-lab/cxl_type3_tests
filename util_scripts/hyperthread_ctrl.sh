#!bin/bash

if [ $1 -eq 1 ];
then
    echo "enable hyperthreading"
    sudo sh -c "echo on > /sys/devices/system/cpu/smt/control"
fi

if [ $1 -eq 0 ];
then
    echo "disable hyperthreading"
    sudo sh -c "echo off > /sys/devices/system/cpu/smt/control"
fi

