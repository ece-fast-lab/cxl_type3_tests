from action import *
from config import *

# Input (tracked by the caller)
#   curr state
#   prev state
#   prev stepping
#   bot ratio 
# Output (returned to the caller)
#   curr stepping
#   new bot ratio 

# In this implementation, the top is fixed to some ratio
#   The bot is tunned in some range of value

def algo(dynamic_state, static_state, prev_step, bot_ratio):
    diff = dynamic_state - static_state
    abs_diff = abs(diff)
    log_debug("PRE: diff:{0}, prev_step:{1}, bot_ratio:{2}".format(diff, prev_step, bot_ratio))
    log_debug("PRE: dyn:{0}, stc:{1}".format(dynamic_state, static_state))

    # ================================== pre-condition
    if dynamic_state > IDLE_THRESHOLD:
        log_action(bcolors.OKGREEN, "Pass -- idle")
        return MIN_STEP, bot_ratio
    elif abs_diff < TUNE_TRESHOLD:
        log_action(bcolors.OKGREEN, "Pass -- stable")
        set_ratio(IL_TOP_RESET, bot_ratio)
        if prev_step < 0:
            return -MIN_STEP , bot_ratio
        else:
            return MIN_STEP , bot_ratio
    elif abs_diff > RESET_THRESHOLD:
        log_action(bcolors.WARNING, "Reset")
        reset_default() 
        return STEP_RESET, -1 

    # ================================== step
    curr_step = prev_step
    log_action(bcolors.OKBLUE, "Tune")
    if diff > 0: # gets better
        log_action(bcolors.OKGREEN, "better")
        curr_step = prev_step
    else: # gets worse
        curr_step = -prev_step / 2 # apply reversed half step
        log_action(bcolors.OKCYAN, "worse, pre-bound step = " + str(curr_step))

    # ================================== bound step
    if curr_step < MIN_STEP and curr_step > -MIN_STEP:
        if curr_step < 0:
            curr_step = -MIN_STEP
        else:
            curr_step = MIN_STEP
    curr_step = int(curr_step)
    log_debug("post-bound step = " + str(curr_step))

    ## ================================== bound ratio
    bot_ratio += curr_step 
    if bot_ratio <= 1: # cap at ddr:cxl = 10:1
        log_action(bcolors.WARNING, "lower bound: bot{0}, step{1}".format(bot_ratio, curr_step))
        bot_ratio = 1
    elif bot_ratio >= (IL_BOT_MAX):
        log_action(bcolors.WARNING, "upper bound: bot{0}, step{1}".format(bot_ratio, curr_step))
        bot_ratio = IL_BOT_MAX

    # ================================== set
    bot_ratio = int(bot_ratio)
    set_ratio(IL_TOP_RESET, bot_ratio)

    log_debug("POST: curr_step:{0}, prev_step:{1}, bot_ratio:{2}".format(curr_step, prev_step, bot_ratio))
    log_debug("POST: dynamic_state:{0}, static_state:{1}, bot_ratio:{2}".format(dynamic_state, static_state, bot_ratio))
    return curr_step, bot_ratio

# Linear function
def calculate_state():
    ret = 0
    for i, (k, v) in enumerate(coeff_dict.items()):
        # Log/square applied in the translation
        mul = v * metric_dict[k]
        ret += mul
        #print(mul, v, metric_dict[k])
    return ret
