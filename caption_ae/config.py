class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

LOG_NONE = 0
LOG_ACTION = LOG_NONE + 1
LOG_DEBUG = LOG_ACTION + 1
LOG_METRIC = LOG_DEBUG + 1

DO_LOG = LOG_DEBUG
ACTION_ENABLE = True
#ACTION_ENABLE = False

IL_TOP_RESET = 10
IL_BOT_MAX = IL_TOP_RESET * 2
IL_BOT_RESET = 1
STEP_RESET = 3
MIN_STEP = 1


WINDOW_SIZE=5

# Config #1
RESET_THRESHOLD = 500
TUNE_TRESHOLD = 0.2
IDLE_THRESHOLD = 70000
ALLOC_THRESHOLD = 150000
ALLOC_DROP_THRESHOLD = 50000

scale_dict = { }

metric_dict = {
    'norm_ipc': 0,
    'L1.miss.lats': 0,
    'DDR.read.lats': 0,
}

# Model from R-studio
#norm_ipc    99.55281   11.24329   8.854 2.35e-08 ***
#l1_lat      -0.04686    0.01539  -3.045  0.00639 ** 
#ddr_lat     -0.48751    0.14351  -3.397  0.00286 ** 
coeff_dict = {
    'norm_ipc': 99.55281,
    'L1.miss.lats': -0.04686,
    'DDR.read.lats': -0.48751,
}

pmu_translateion = { }

pcm_translateion = {
    'norm_ipc': 'pcm_norm_ipc',
    'L1.miss.lats': 'pcm_l1miss',
    'DDR.read.lats': 'pcm_ddrReadLat',
}
