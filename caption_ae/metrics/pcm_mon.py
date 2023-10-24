#!/home/zeduoyu2/anaconda3/bin/python3

import os
import subprocess
import time
import sys
import signal
from queue import Queue, Empty
from threading  import Thread
import re

PCM_PATH = "/home/yans3/AE_root/pcm/build/bin/"

class pcm_metric:
    def __init__(self) -> None:

        self.pcmLat_cmd = ["sudo", PCM_PATH + "pcm-latency"]
        self.pcmBw_cmd = ["sudo",  PCM_PATH + "pcm-memory"]
        self.pcmAll_cmd = ["sudo", PCM_PATH + "pcm", "-nc", "-ns"]

        self.stats = {
                "pcm_l1miss":[], 
                "pcm_ddrReadLat":[], 
                "pcm_norm_ipc":[],
                "pcm_ipc":[]}

        def signal_handler(sig, frame):
            os.killpg(os.getpgid(self.p_latency.pid), signal.SIGINT)
            os.killpg(os.getpgid(self.p_all.pid), signal.SIGINT)
            print('[INFO] You pressed Ctrl+C!')
            sys.exit(0)

        #signal.signal(signal.SIGINT, signal_handler)
        print('[INFO] Press Ctrl+C to exit')

    def get_stat(self, window_size:int=5):
        res = {}
        # Take mean of last N samples
        for k, v in self.stats.items():
            length = len(v)
            if length >= 1:
                last_n = v[-min(length,window_size):]
                res[k] = sum(last_n) / len(last_n)
        return res

    def run_realtime(self, print_info=True) -> None:

        def signal_handler(sig, frame):
            os.killpg(os.getpgid(self.p_latency.pid), signal.SIGINT)
            os.killpg(os.getpgid(self.p_all.pid), signal.SIGINT)
            print('[INFO] You pressed Ctrl+C!')
            sys.exit(0)

        #signal.signal(signal.SIGINT, signal_handler)
        print('[INFO] Press Ctrl+C to exit')

        # Start the PCM processes
        print("[COMMAND]", self.pcmLat_cmd)
        self.p_latency = subprocess.Popen(self.pcmLat_cmd, stdout=subprocess.PIPE, text=True, preexec_fn=os.setsid)
        time.sleep(5)

        print("[COMMAND]", self.pcmAll_cmd)
        self.p_all = subprocess.Popen(self.pcmAll_cmd, stdout=subprocess.PIPE, text=True, preexec_fn=os.setsid)


        # define and start the recording threads
        def enqueue_output(stdout, queue):
            for line in stdout:
                queue.put(line)
            stdout.close()

        q_latency = Queue()
        q_all = Queue()

        t_latency = Thread(target=enqueue_output, args=(self.p_latency.stdout, q_latency))
        t_all = Thread(target=enqueue_output, args=(self.p_all.stdout, q_all))

        t_latency.daemon = True
        t_all.daemon = True

        t_latency.start()
        t_all.start()


        def catch_output_all_cal(q:Queue):
            lines = ""
            result_arr = []
            while(True):
                try: 
                    line = q.get_nowait()
                except Empty:
                    time.sleep(1)  # tune to 0.5 just in case
                else:
                    lines += line
                    if line == "---------------------------------------------------------------------------------------------------------------\n":
                        norm_ipc_cal = 0
                        ipc_cal = 0
                        valid_cnt = 0

                        for norm_ipc, ipc in result_arr:
                            # XXX, hack -- norm_ipc > 0.1 kind of indicates the core is running something
                            if True or norm_ipc > 0.1:
                                norm_ipc_cal += norm_ipc
                                ipc_cal += ipc
                                valid_cnt += 1

                        if valid_cnt > 0:
                            norm_ipc_cal = norm_ipc_cal / valid_cnt
                            ipc_cal = ipc_cal / valid_cnt
                            print('valid_cnt:', valid_cnt)

                        result_arr = []
                        self.stats["pcm_norm_ipc"].append(norm_ipc_cal)
                        self.stats["pcm_ipc"].append(ipc_cal)

                    else:
                        line_arr = line.split()
                        if len(line_arr) < 5:
                            continue
                        elif line_arr[0].isdigit():
                            norm_ipc = float(line_arr[2])
                            ipc = float(line_arr[3])
                            result_arr.append((norm_ipc, ipc))
                            '''
                            if int(line_arr[0]) > 7: 
                                result_arr.append((norm_ipc, ipc))
                            '''

        # define and start the parsing threads
        def catch_output_all(q:Queue):
            lines = ""
            pattern_norm_ipc = re.compile(r"Instructions per nominal CPU cycle: ([0-9]+\.[0-9]+)")
            pattern_ipc = re.compile(r" PHYSICAL CORE IPC                 : ([0-9]+\.[0-9]+)")
            while(True):
                try: 
                    line = q.get_nowait()
                except Empty:
                    time.sleep(1)  # tune to 0.5 just in case
                else:
                    lines += line
                    if line == "---------------------------------------------------------------------------------------------------------------\n":
                        matches = pattern_norm_ipc.findall(lines)
                        if matches: 
                            if print_info: print("norm IPC: " + matches[0])
                            self.stats["pcm_norm_ipc"].append(float(matches[0]))

                        matches = pattern_ipc.findall(lines)
                        if matches: 
                            if print_info: print("IPC: " + matches[0])
                            self.stats["pcm_ipc"].append(float(matches[0]))
                        lines = ""

        def catch_output_latency(q:Queue):
            lines = ""
            while(True):
                try: 
                    line = q.get_nowait()
                except Empty:
                    time.sleep(1)  # tune to 0.5 just in case
                else: # got line
                    lines += line
                    if line == "-----------------------------------------------------------------------------\n":
                        # q_out.put(lines)
                        pattern = re.compile(r"L1 Cache Miss Latency\(ns\) \[Adding 5 clocks for L1 Miss\]\n+Socket0: ([0-9]+\.[0-9]+)")
                        matches = pattern.findall(lines)
                        if matches: 
                            if print_info: print("[RESULT] L1 Miss Latency:" + matches[0])
                            self.stats["pcm_l1miss"].append(float(matches[0]))

                        pattern = re.compile(r"DDR read Latency\(ns\)\nSocket0: ([0-9]+\.[0-9]+)\s*")
                        matches = pattern.findall(lines)
                        if matches:
                            if print_info: print("[RESULT] DDR Read Latency:" + matches[0])
                            self.stats["pcm_ddrReadLat"].append(float(matches[0]))

                        lines = ""

        t_catch_latency = Thread(target=catch_output_latency, args=[q_latency])
        t_catch_all = Thread(target=catch_output_all, args=[q_all])

        t_catch_latency.daemon = True
        t_catch_all.daemon = True

        t_catch_latency.start()
        t_catch_all.start()

        if print_info:
            while(True):
                time.sleep(1) # fixed BUG: used to be 'pass', which cause the utilization to be 100%


if __name__ == "__main__":
    pcm = pcm_metric()
    pcm.run_realtime(print_info=False)
    while(True):
        print(pcm.get_stat())
        time.sleep(1)
