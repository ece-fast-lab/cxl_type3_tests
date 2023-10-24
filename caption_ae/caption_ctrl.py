import random 
import time
from threading  import Thread
import math
import threading
import statistics

from metrics.pcm_mon import *
from metrics.vmstat_mon import *

from algo import *
from action import *
from config import *

MIN_SAMPLE_CNT = 20

print("=======================")
print("reset basics: top:{0}, bot_max:{1}, bot_rst:{2}, step_rst:{3}".format(
    IL_TOP_RESET,
    IL_BOT_MAX,
    IL_BOT_RESET,
    STEP_RESET
    ))
print("=======================")

class caption_ctrl:
    def __init__(self) -> None:
        self.arr = {'norm_ipc': [], 
                    'L1.miss.lats': [], 
                    'DDR.read.lats': []}
        self.prev_state = -1
        self.prev_step = STEP_RESET
        self.prev_ratio = IL_BOT_RESET
    
    def run_realtime(self, log_level=LOG_DEBUG)->None:
        pcm = pcm_metric()
        pcm_thread = Thread(target=pcm.run_realtime, args=(False,))
        pcm_thread.start()
        reset_default()

        def catch_output():
            cnt = 0
            while(True):
                update_metric(pcm.get_stat(WINDOW_SIZE), {})

                self.arr['norm_ipc'].append(metric_dict['norm_ipc'])
                self.arr['L1.miss.lats'].append(metric_dict['L1.miss.lats'])
                self.arr['DDR.read.lats'].append(metric_dict['DDR.read.lats'])
                time.sleep(1)
                cnt += 1

        t = Thread(target=catch_output)
        t.daemon = True
        t.start()

        '''
        while(True):
            time.sleep(1)
            self.prev_state += 1
        '''

    def get_set_tune_val(self, first_time=False, model_only=False):
        log_action(bcolors.OKCYAN, "=============== TUNE ================ ")

        # get
        if len(self.arr['norm_ipc']) > MIN_SAMPLE_CNT:
            log_action(bcolors.OKCYAN, "=============== TUNE -- valid ================ ")
            accu_avg = 0
            # linear equation, SUM(coeff * mean of sample)
            for k, v in self.arr.items():
                mean = sum(v) / len(v)
                accu_avg += coeff_dict[k] * mean
                self.arr[k] = []
                print(k, mean)
            log_debug("=============== accu_avg -- {0} ================ ".format(accu_avg))
            if model_only:
                return
        else:
            log_action(bcolors.OKCYAN, "=============== TUNE -- not enough samples ================ ")
            log_action(bcolors.OKCYAN, "=============== TUNE -- need: %d, has: %d samples ================ " % (MIN_SAMPLE_CNT, 
                                                                                                    len(self.arr['norm_ipc'])))
            return

        log_action(bcolors.OKCYAN, "=============== TUNE -- algo ================ ")
        # first time tuning always attempt to tune toward more CXL
        if first_time:
            self.prev_state = accu_avg - 1
        step, ratio = algo(accu_avg, self.prev_state, self.prev_step, self.prev_ratio)

        log_action(bcolors.OKCYAN, "=============== TUNE -- track states ================ ")
        self.prev_state = accu_avg
        self.prev_step = step
        if ratio < 0:
            self.prev_ratio = IL_BOT_RESET
        else:
            self.prev_ratio = ratio

if __name__ == "__main__":
    tuner = caption_ctrl()
    tuner.run_realtime()
    while(True):
        time.sleep(1)
        print(tuner.get_set_tune_val())
