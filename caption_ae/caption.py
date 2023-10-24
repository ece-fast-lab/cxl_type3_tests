import os
import time
import sys
from caption_ctrl import *
import argparse
import subprocess
import threading
import time

MAX_TUNE_ITER = 7

parser = argparse.ArgumentParser()
parser.add_argument("-x", "--step", help="stepping mode (10:x+=step). Algorithm is not applied, but the output of the estimator will be printed.", action='store_true')
parser.add_argument("-s", "--sh-path", help="shell script that houses the program to be tuned. You may embed several program in this shell script")
parser.add_argument("-n", "--no-tune", help="disable tuning. This will simply run the passed in shell script", action='store_true')
parser.add_argument("-t", "--batch_txt", help="txt script that houses several shell scripts to be exectued. Tuning will happen whenevr a script ended its execution. You may set 'tune_mask' to enable tunning when one of the program ends")
args = parser.parse_args()

#print(args.echo)
def exec_cmd_and_wait(sh_path):
    print(sh_path)
    subprocess.call(["sudo", "bash", sh_path]) 

def exec_cmd_and_wait_arg(sh_path):
    sh_path_arr = sh_path.split()
    sh_path_arr.insert(0, "bash") 
    sh_path_arr.insert(0, "sudo") 
    print(sh_path_arr)
    subprocess.call(sh_path_arr) 

def sync_tune():
    # start monitor
    tuner = caption_ctrl()
    tuner.run_realtime()
    time.sleep(5)

    tune_iter = 0
    first_time = True

    stepping_ratio = 1

    try:
        while True:
            print("sync tuning iteration: %d" % (tune_iter))
                
            # run exec
            exec_cmd_and_wait(args.sh_path)

            # tune
            if args.no_tune:
                print(" ============ no tune ============== ")
            if args.step is not None:
                print(" ============ no tune, stepping only ============== ")
                tuner.get_set_tune_val(first_time, model_only=True)
                stepping_ratio += 2
                set_ratio(10, stepping_ratio)
            else:
                tuner.get_set_tune_val(first_time)

            first_time = False
            time.sleep(5)
            tune_iter += 1

            if tune_iter >= MAX_TUNE_ITER:
                break
    except KeyboardInterrupt:
        print("ended with ctrl-c")

def async_tune():
    # start monitor
    tuner = caption_ctrl()
    tuner.run_realtime()
    time.sleep(5)

    tune_iter = 0
    first_time = True

    # read path 
    # (t, sh_path)
    thread_arr = []
    tune_mask = []
    with open(args.batch_txt) as f:
        for line in f.readlines():
            line = line.strip()
            print(line)
            t = threading.Thread(target=exec_cmd_and_wait_arg, args=(line,))
            thread_arr.append((t, line))

            # FIXME, set to the desired mask
            tune_mask.append(True)
            # for example: tune whenever "roms" ended 
            #tune_mask.append("roms" in line)

    for t, _ in thread_arr:
        t.start()

    print("tune_mask: ", tune_mask)

    try:
        while True:
            print("async tuning iteration: %d" % (tune_iter))
            
            found_end = False
            # This will find the targeted ending thread
            while found_end is False:

                for idx, (t, sh_path) in enumerate(thread_arr):
                    if not t.is_alive():
                        print("ended %s" % sh_path)

                        # restart
                        new_t = threading.Thread(target=exec_cmd_and_wait_arg, args=(sh_path,))
                        thread_arr[idx] = (new_t, sh_path)
                        new_t.start()

                        # stop
                        if tune_mask[idx]:
                            print("tune!")
                            found_end = True
                            break

                    # sleep, avoid spinning 
                    time.sleep(1)

            # tune
            if args.no_tune:
                print(" ============ no tune ============== ")
            else:
                tuner.get_set_tune_val(first_time)

            first_time = False
            time.sleep(5)
            tune_iter += 1

            if tune_iter >= MAX_TUNE_ITER:
                break

    except KeyboardInterrupt:
        print("ctrl-c pressed")
        for t, _ in thread_arr:
            t.stop()
        print("ended with ctrl-c")

if __name__ == "__main__":
    if args.batch_txt is not None:
        async_tune()
    else:
        sync_tune()

