#!/home/zeduoyu2/anaconda3/bin/python3

import os
import subprocess
import time
import sys
import signal
from queue import Queue, Empty
from threading  import Thread

class slab_metric:
    def __init__(self, window_size=5) -> None:
        self.slab_realtime_cmd = ["sudo", "python3", "/home/yans3/bcc/tools/slabratetop.py", "-C"]
        self.stats = {'alloc_sum':[]}
        self.cnt = 0
        self.moving_sum = 0
        self.window_size = window_size
        for i in range(window_size):
            self.stats['alloc_sum'].append(0)
            
    def run_realtime(self, interval=1000, print_info=True) -> None:

        # define the SIGINT handler
        def signal_handler(sig, frame):
            os.killpg(os.getpgid(self.p_slab.pid), signal.SIGINT)
            print('[INFO] You pressed Ctrl+C!')
            sys.exit(0)

        #signal.signal(signal.SIGINT, signal_handler)
        print('[INFO] Press Ctrl+C to exit')

        cmd = self.slab_realtime_cmd
        print("[COMMAND]", cmd)
        self.p_slab = subprocess.Popen(cmd, text=True, stdout=subprocess.PIPE, preexec_fn=os.setsid)

        # define and start the recording threads
        def enqueue_output(stdout, queue):
            for line in stdout:
                queue.put(line)
            stdout.close()

        q_slab = Queue()
        t_slab = Thread(target=enqueue_output, args=(self.p_slab.stdout, q_slab))
        t_slab.daemon = True
        t_slab.start()
        
        # define and start the parsing threads
        def catch_output(q:Queue):
            curr_sum = 0
            while(True):
                try: 
                    line = q.get_nowait() # or q.get(timeout=.1)
                except Empty:
                    time.sleep(interval / 1000)  # tune to 0.5 just in case
                else: # got line
                    if line.isspace(): continue
                    line = line.split()
                    try:
                        float(line[-1])
                    except ValueError:
                        continue
                    
                    if "loadavg" in line[1]:
                        self.moving_sum += curr_sum
                        self.cnt += 1
                        if self.cnt >= self.window_size:
                            self.moving_sum -= self.stats['alloc_sum'][self.cnt % self.window_size] 

                        if print_info:
                            print('mov', self.moving_sum, 'curr', curr_sum)
                            print(self.stats['alloc_sum'])

                        self.stats['alloc_sum'][self.cnt % self.window_size] = curr_sum
                        curr_sum = 0
                    else:
                        curr_sum += int(line[1])

                    if print_info:
                        print(line)


        t_catch_latency = Thread(target=catch_output, args=[q_slab])
        t_catch_latency.daemon = True
        t_catch_latency.start()
        
        if print_info:
            while(True):
                time.sleep(1)

        
    def get_stat(self) -> int:
        return self.moving_sum / self.window_size

if __name__ == "__main__":
    slab = slab_metric(1)
    slab.run_realtime(print_info=True)
    while(True):
        print(slab.get_stat())
        time.sleep(1)
