#!/home/zeduoyu2/anaconda3/bin/python3

import os
import subprocess
import time
import sys
import signal
from queue import Queue, Empty
from threading  import Thread

class pmu_metric:
    def __init__(self, nodes=["Backend_Bound.Memory_Bound.DRAM_Bound.MEM_Latency", "Backend_Bound.Memory_Bound.L1_Bound",
         "Backend_Bound.Memory_Bound.DRAM_Bound.MEM_Bandwidth"], output_path="result/pmu_tools") -> None:

        self.node_list = nodes
        self.node_names = ','.join(nodes)
        self.output_path = output_path
        self.numaRatio_cmd = "sudo sysctl -w vm.numa_tier_interleave={ratio}"
        self.numaRatioTop_cmd = "sudo sysctl -w vm.numa_tier_interleave_top={top}"
        self.numaRatioBot_cmd = "sudo sysctl -w vm.numa_tier_interleave_bot={bottom}"
        self.toplev_cmd = ["sudo", "/home/yans3/pmu-tools/toplev", "-x,", "-o", "{filename}", "--no-desc", "-I", "1000", "-v", "--nodes", 
                           "!" + self.node_names]
        self.toplev_realtime_cmd = ["sudo", "/home/yans3/pmu-tools/toplev", "--no-desc", "-I", "1000", "-v", 
                                    "--nodes", "!" + self.node_names]
        # self.stats = {"L1_Bound":[], "BW_Bound":[], "Lat_Bound":[]}
        self.stats = {}
        for node in self.node_list:
            self.stats[node] = []
        
        if not os.path.exists(self.output_path):
            os.mkdir(self.output_path)
            

    def set_ratio(self, top:int, bot:int) -> None:

        print("[INFO] Configuring NUMA interleave ratio to %d:%d (DRAM:CXL)"%(top, bot))
        cmd = self.numaRatioTop_cmd.format(top=top)
        print("[COMMAND]", cmd)
        p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stdin=subprocess.PIPE, shell=True, text=True)
        ret = p.wait()
        if ret != 0:
            sys.exit(ret)
        out = p.communicate()[0]
        print("[RETURN] Output:", out)

        cmd = self.numaRatioBot_cmd.format(bottom=bot)
        print("[COMMAND]", cmd)
        p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stdin=subprocess.PIPE, shell=True, text=True)
        ret = p.wait()
        if ret != 0:
            sys.exit(ret)
        out = p.communicate()[0]
        print("[RETURN] Output:", out)



    def start_recording(self, filename:str = "pmu_result.csv") -> None:

        # define the SIGINT handler
        def signal_handler(sig, frame):
            os.killpg(os.getpgid(self.p_toplev.pid), signal.SIGINT)
            print('[INFO] You pressed Ctrl+C!')
            sys.exit(0)

        signal.signal(signal.SIGINT, signal_handler)
        print('[INFO] Press Ctrl+C to exit')

        assert filename.endswith('.csv')
        file_path = os.path.join(self.output_path, filename)
        cmd = self.toplev_cmd
        cmd[4] = file_path
        print("[COMMAND]", cmd)
        self.p_toplev = subprocess.Popen(cmd, text=True, preexec_fn=os.setsid)

    
    def stop_recording(self) -> None:
        # self.fp.close()
        os.killpg(os.getpgid(self.p_toplev.pid), signal.SIGINT)
        print("[INFO] PMU monitoring stopped.")


    def run_realtime(self, interval=1000, print_info=True) -> None:

        # define the SIGINT handler
        def signal_handler(sig, frame):
            os.killpg(os.getpgid(self.p_toplev.pid), signal.SIGINT)
            print('[INFO] You pressed Ctrl+C!')
            sys.exit(0)

        #signal.signal(signal.SIGINT, signal_handler)
        print('[INFO] Press Ctrl+C to exit')

        cmd = self.toplev_realtime_cmd
        cmd[4] = str(interval)
        print("[COMMAND]", cmd)
        self.p_toplev = subprocess.Popen(cmd, text=True, stderr=subprocess.PIPE, preexec_fn=os.setsid)

        # define and start the recording threads
        def enqueue_output(stdout, queue):
            for line in stdout:
                queue.put(line)
            stdout.close()

        q_toplev = Queue()
        t_toplev = Thread(target=enqueue_output, args=(self.p_toplev.stderr, q_toplev))
        t_toplev.daemon = True
        t_toplev.start()
        
        # define and start the parsing threads
        def catch_output(q:Queue):
            while(True):
                try: 
                    line = q.get_nowait() # or q.get(timeout=.1)
                except Empty:
                    time.sleep(interval / 1000)  # tune to 0.5 just in case
                else: # got line
                    if line.isspace(): continue
                    line = line.split()
                    try:
                        float(line[0])
                    except ValueError:
                        continue

                    node = line[2]
                    val = line[5]

                    if print_info:
                        print("[RESULT] {node_name:<60} {value:<4} %".format(node_name=node, value=val))

                    self.stats[node].append(val)

        t_catch_latency = Thread(target=catch_output, args=[q_toplev])
        t_catch_latency.daemon = True
        t_catch_latency.start()
        
        if print_info:
            while(True):
                time.sleep(1)

        
    def get_stat(self, window_size:int=5) -> dict:

        res = {}
        
        # length = len(self.stats["L1_Bound"])
        # if length >= 1:
        #     series = np.array(self.stats["L1_Bound"][-min(length,window_size):]).astype('float')
        #     res["L1_Bound"] = series.mean()

        # length = len(self.stats["BW_Bound"])
        # if length >= 1:
        #     series = np.array(self.stats["BW_Bound"][-min(length,window_size):]).astype('float')
        #     res["BW_Bound"] = series.mean()

        # length = len(self.stats["Lat_Bound"])
        # if length >= 1:
        #     series = np.array(self.stats["Lat_Bound"][-min(length,window_size):]).astype('float')
        #     res["Lat_Bound"] = series.mean()

        for node in self.node_list:
            length = len(self.stats[node])
            if length >= 1:
                series = np.array(self.stats[node][-min(length,window_size):]).astype('float')
                res[node] = series.mean()

        return res


if __name__ == "__main__":

    node_names = ["Backend_Bound.Memory_Bound.DRAM_Bound.MEM_Latency", "Backend_Bound.Memory_Bound.L1_Bound",
            "Backend_Bound.Memory_Bound.DRAM_Bound.MEM_Bandwidth", "Backend_Bound.Memory_Bound",
            "Backend_Bound.Memory_Bound.L2_Bound", "Backend_Bound.Memory_Bound.L3_Bound"]
    
    pmu = pmu_metric(node_names)
    # pmu.set_ratio(10,20)
    # pmu.start_recording("dlrm.csv")
    pmu.run_realtime(print_info=False)
    while(True):
        print(pmu.get_stat())
        # print(pmu.stats)
        time.sleep(1)
