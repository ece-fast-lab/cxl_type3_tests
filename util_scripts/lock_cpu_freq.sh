sudo cpupower --cpu all frequency-set --freq 2100MHz
sudo sh -c 'echo 0 > /sys/devices/system/cpu/cpufreq/boost'

