#!/home/zeduoyu2/anaconda3/bin/python3

import os
import subprocess
import time
import sys
import signal
from queue import Queue, Empty
from threading  import Thread
import re
import time

class vmstat_metric:
    def __init__(self) -> None:
        self.cnt = 0
        self.val = 0
            
    def run_realtime(self, interval=1000, print_info=True) -> None:
        # define and start the parsing threads
        def catch_output():
            prev_val = 0 
            curr_val = 0
            while(True):
                output = subprocess.check_output("cat /proc/vmstat", shell=True)
                output = output.splitlines()
                target_line = output[66]
                target_line = target_line.decode("utf-8") 
                curr_val = int(target_line.split()[-1])  
                '''
                for idx, line in enumerate(output):
                    line = line.decode("utf-8") 
                    if "pgalloc_normal" not in line:
                        continue
                    else:
                        curr_val = int(line.split()[-1])  
                        print(idx)
                        break
                '''

                self.val = curr_val - prev_val
                prev_val = curr_val
                time.sleep(1)

        t = Thread(target=catch_output)
        t.daemon = True
        t.start()
        
        if print_info:
            while(True):
                time.sleep(1)

        
    def get_stat(self) -> int:
        return self.val

if __name__ == "__main__":
    vmstat = vmstat_metric()
    vmstat.run_realtime(print_info=False)
    while(True):
        print(vmstat.get_stat())
        time.sleep(1)
