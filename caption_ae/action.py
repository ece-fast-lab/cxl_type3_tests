from config import *
import subprocess

def log_action(color, log):
    if DO_LOG >= LOG_ACTION:
        print("[ACTION] === " + color + log + bcolors.ENDC + " ===")
    pass
def log_metric(tag, log):
    log = str(log)
    log = tag + " " + log
    if DO_LOG >= LOG_METRIC:
        print("[METRIC] === " + bcolors.UNDERLINE + log + bcolors.ENDC + " ===")
        pass
    pass
def log_debug(log):
    log = str(log)
    if DO_LOG >= LOG_DEBUG:
        print("[DEBUG] === " + bcolors.BOLD + log + bcolors.ENDC + " ===")
    pass

def update_metric(pcm_dict, pmu_dict):
    #if len(pcm_dict) == 0 or len(pmu_dict) == 0:
    #    return
    for i, (v, k) in enumerate(pcm_translateion.items()):
        if k not in pcm_dict:
            continue
        val = pcm_dict[k]
        if 'log' in v:
            val = math.log10(val)        
        if "2)2" in v:
            val = val * val
        metric_dict[v] = val
        log_metric(v, val)

    for i, (v, k) in enumerate(pmu_translateion.items()):
        if k not in pmu_dict:
            continue
        val = pmu_dict[k]
        if 'log' in v:
            val = math.log10(val)        
        if "2)2" in v:
            val = val * val
        metric_dict[v] = pmu_dict[k]
        log_metric(v, val)

def set_ratio(top, bot):
    if ACTION_ENABLE:
        subprocess.run(['sudo','sysctl','-w','vm.numa_tier_interleave_top='+str(top)])
        subprocess.run(['sudo','sysctl','-w','vm.numa_tier_interleave_bot='+str(bot)])

def reset_default():
    set_ratio(IL_TOP_RESET, IL_BOT_RESET)
