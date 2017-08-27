import numpy, string, time
from random import randint
from random import sample
from random import choice
from datetime import timedelta
import sys
from dataBaseInterface import DataBaseInterface
from audioInfo import AudioEvent, AudioFileInfo
from audio_event_queue import SortedDLL
from internalAnimationsHandler import InternalAninamtionsHandler

DEBUG = True
INTERNAL_ANIMATIONS_DEBUG = True

internal_aa_handler = InternalAninamtionsHandler()
# Non-Color Animation Parameter Constants
#
# All (most of) the parameters for a show are stored in an array called show_parameters, so that they
# can be passed around in one go. These constants are the indices into this show_parameters
# array holding that type of parameter value

NUM_7_COLOR_ANIMATIONS = 255
NUM_BASE_ANIMATIONS = 3
NUM_MID_ANIMATIONS = 12
NUM_SPARKLE_ANIMATIONS = 11

NUM_BEAT_EFFECTS = 8
NUM_PARAMETERS = 40
NUM_COLORS_PER_PALETTE = 7

# splitting 7 color animations into two categories
# 1) hello animaitons 2) full sctructure art car edm animations
NUM_7_COLOR_ANIMATIONS_HELLO_START = 128
NUM_7_COLOR_ANIMATIONS_HELLO_END = 133
NUM_7_COLOR_ANIMATIONS_AC_EDM_START = 1
NUM_7_COLOR_ANIMATIONS_AC_EDM_END = 8
NUM_7_COLOR_MEDITATION_ANIMATIONS_START = 192
NUM_7_COLOR_MEDITATION_ANIMATIONS_END = 193  

NUM_BASE_TRANSITIONS = 1
NUM_MID_TRANSITIONS = 1
NUM_SPARKLE_TRANSITIONS = 1
NUM_EDM_TRANSITIONS = 1
NUM_TRANSITION_SPEEDS = 6
NUM_PALETTE_CHANGE_STYLES = 5

BASE_TIME_LIMIT = 31
BASE_PARAMETER_TIME_LIMIT = 19
MID_TIME_LIMIT = 29
MID_PARAMETER_TIME_LIMIT = 17
SPARKLE_TIME_LIMIT = 37
SPARKLE_PARAMETER_TIME_LIMIT = 13
PALETTE_TIME_LIMIT = 7
# For Lee testing
PALETTE_TIME_LIMIT = 5

# constants to protect against too frequent param changes during dynamic show
BASE_MAIN_ANIMATION_SWITCH_LAG = 0  # 12
BASE_PARAMETER_SWTICH_LAG = 0  # 3
MID_MAIN_ANIMATION_SWITCH_LAG = 0  # 8
MID_PARAMETER_SWTICH_LAG = 0  # 2
SPARKLE_MAIN_ANIMATION_SWITCH_LAG = 0  # 6
SPARKLE_PARAMETER_SWTICH_LAG = 0  # 1
PALETTE_LAG = 0  # 7

BACKGROUND_INDEX = 0
MIDLAYER_INDEX = 8
SPARKLE_INDEX = 17

# bounding indices to ease reading of parameter printout
BASE_PARAM_START = 0
BASE_PARAM_END = 7
MID_PARAM_START = 8
MID_PARAM_END = 16
SPARKLE_PARAM_START = 17
SPARKLE_PARAM_END = 27
SEVEN_PAL_BEAT_PARAM_START = 28
ART_CAR_RING_PARAM = 29
SEVEN_PAL_BEAT_PARAM_END = 31
TRANS_PARAM_START = 32
TRANS_PARAM_END = 39

show_bounds = [  # order must match show_parameters
    # [min, max]
    # show bounds 0 through 7 concern base animations
    [0, NUM_BASE_ANIMATIONS],  # BACKGROUND_INDEX, which background animation to use
    [0, 255],  # base color thickness
    [0, 255],  # base black thickness
    [-128, 127],  # base ring offset
    [-1, 2],  # base intra ring motion: -1 CCW, 0 none, 1 CW, 2 alternate, 3 split (down from top)
    [0, 255],  # base intra ring speed
    [-1, 1],  # base inter ring motion: -1 = CCW, 0 = none, 1 = CW
    [0, 255],  # base inter ring speed
    # show bounds 8 through 16 concern mid layer animations
    [0, NUM_MID_ANIMATIONS],  # MIDLAYER_INDEX, which mid layer animation to use
    [1, 3],  # mid num colors
    [0, 255],  # mid color thickness
    [0, 255],  # mid black thickness
    [-128, 127],  # mid ring offset
    [-1, 2],  # mid intra ring motion: -1 CCW, 0 none, 1 CW, 2 alternate, 3 split (down from top)
    [0, 255],  # mid intra ring speed
    [-1, 1],  # mid inter ring motion: -1 = CCW, 0 = none, 1 = CW
    [0, 255],  # mid inter ring speed
    # show bounds 17 through 27 concern sparkle animations
    [0, NUM_SPARKLE_ANIMATIONS],  # SPARKLE_INDEX, which sparkle animation to use
    [2, 200],  # sparkle portion
    [0, 255],  # sparkle color thickness
    [-1, 2],  # sparkle intra ring motion: -1 CCW, 0 none, 1 CW, 2 alternate, 3 split (down from top)
    [0, 255],  # sparkle intra ring speed
    [-1, 1],  # sparkle inter ring motion: -1 = CCW, 0 = none, 1 = CW
    [0, 255],  # sparkle inter ring speed
    [0, 255],  # sparkle min dim
    [0, 255],  # sparkle max dim
    [0, 255],  # sparkle range
    [0, 255],  # sparkle spawn frequency, 0 == off entirely (Functions as a boolean when 0|1)
    # show bounds 28 through 30 concern 7-color edm animations
    [0, NUM_7_COLOR_ANIMATIONS],  # which 7 color animation to play, show bound 28
    [-1, 72],  # Ring number of art car (-1 is no car detected), show bound 29
    [0, NUM_BEAT_EFFECTS],  # which beat effect to use to respond to beat with LEDs, show bound 30

    [1, NUM_PALETTE_CHANGE_STYLES],  # Palette change style (0 is immediate), show bound 31

    # show bounds 32 through 39 concern animation transitions
    [1, NUM_BASE_TRANSITIONS],  # how to transition the background animation (0 is immediate)
    [1, NUM_TRANSITION_SPEEDS],  # how fast to transition the background animation
    [1, NUM_MID_TRANSITIONS],  # how to transition the mid animation (0 is immediate)
    [1, NUM_TRANSITION_SPEEDS],  # how fast to transition the mid animation
    [1, NUM_SPARKLE_TRANSITIONS],  # how to transition the sparkle animation (0 is immediate)
    [1, NUM_TRANSITION_SPEEDS],  # how fast to transition the sparkle animation
    [1, NUM_EDM_TRANSITIONS],  # how to transition the EDM animation (0 is immediate)
    [1, NUM_TRANSITION_SPEEDS]  # how fast to transition the EDM animation
]

# Pre-defined color palettes for each chakra color
NUM_BASE_COLORS_PER_FAMILY = 7
NUM_MID_COLORS_PER_FAMILY = 15
NUM_SPARKLE_COLORS_PER_FAMILY = 7

red_dim_colors = [[20, 0, 0], [13, 3, 5], [17, 2, 0], [13, 6, 4], [14, 4, 5], [15, 6, 6], [13, 7, 5]]
orange_dim_colors = [[14, 6, 5], [14, 3, 1], [13, 7, 4], [11, 7, 2], [11, 8, 4], [20, 0, 0], [13, 4, 0]]
yellow_dim_colors = [[11, 7, 3], [13, 8, 0], [12, 9, 0], [15, 14, 0], [15, 13, 4], [18, 15, 0], [19, 17, 0]]
green_dim_colors = [[5, 13, 0], [5, 10, 3], [3, 15, 1], [3, 19, 05, 14, 6], [3, 10, 4], [10, 15, 0], [7, 17, 0]]
blue_dim_colors = [[6, 0, 22], [0, 8, 21], [8, 12, 18], [8, 3, 18], [7, 6, 20], [1, 0, 26], [0, 15, 13]]
purple_dim_colors = [[4, 0, 10], [5, 0, 10], [5, 0, 9], [6, 0, 8], [7, 0, 8], [8, 0, 8], [8, 0, 7]]
white_dim_colors = [[60, 24, 164], [194, 30, 24], [97, 24, 128], [135, 24, 90], [83, 172, 24], [175, 49, 24],
                    [142, 97, 24]]

red_mid_colors = [[248, 8, 0], [248, 0, 8], [240, 16, 0], [240, 8, 8], [240, 0, 16], [232, 24, 0], [232, 16, 8],
                  [232, 8, 16], [232, 0, 32], [224, 32, 0], [224, 24, 8], [224, 16, 16], [224, 8, 24], [224, 0, 32],
                  [128, 4, 9], [253, 0, 2], [217, 1, 23], [253, 0, 2], [142, 0, 5], [132, 6, 6], [254, 253, 253]]
orange_mid_colors = [[242, 9, 0], [208, 16, 8], [232, 20, 0], [02, 19, 12], [214, 20, 1], [199, 26, 9], [189, 32, 17],
                     [223, 32, 0], [204, 30, 1], [191, 34, 8], [211, 42, 0], [184, 42, 7], [202, 53, 0], [173, 52, 9],
                     [73, 52, 9]]
yellow_mid_colors = [[170, 148, 0], [171, 128, 0], [150, 114, 6], [150, 141, 6], [132, 99, 0], [124, 108, 0],
                     [70, 43, 2], [80, 40, 0], [76, 42, 0], [73, 46, 0], [76, 52, 0], [80, 55, 0], [79, 64, 0],
                     [70, 84, 0], [171, 160, 0]]
green_mid_colors = [[129, 191, 0], [105, 199, 0], [79, 195, 1], [1, 210, 1], [42, 233, 0], [21, 226, 1], [0, 189, 0],
                    [0, 244, 9], [7, 201, 24], [0, 152, 22], [19, 132, 35], [35, 143, 11], [47, 136, 10], [58, 140, 2],
                    [29, 155, 2]]
blue_mid_colors = [[0, 128, 126], [9, 0, 244], [31, 0, 221], [4, 0, 109], [10, 30, 69], [11, 43, 56], [20, 11, 78],
                   [34, 28, 191], [0, 0, 243], [20, 1, 214], [29, 14, 192], [52, 0, 201], [26, 5, 73], [38, 8, 128],
                   [0, 4, 41]]
purple_mid_colors = [[17, 1, 38], [19, 0, 39], [24, 0, 38], [26, 0, 31], [29, 0, 33], [28, 1, 27], [24, 0, 21],
                     [19, 0, 28], [24, 1, 18], [13, 0, 19], [14, 0, 18], [8, 0, 13], [7, 0, 13], [9, 0, 11], [8, 0, 14]]
white_mid_colors = [[191, 63, 63], [170, 63, 39], [162, 68, 35], [155, 70, 28], [144, 81, 28], [142, 84, 20],
                    [142, 124, 28], [117, 162, 58], [93, 174, 60], [60, 148, 103], [54, 126, 119], [61, 61, 191],
                    [76, 66, 181], [67, 34, 161], [90, 46, 148]]

red_bright_colors = [[168, 39, 64], [174, 54, 71], [180, 52, 63], [189, 35, 41], [194, 41, 41], [186, 65, 60],
                     [183, 91, 82]]
orange_bright_colors = [[155, 73, 32], [150, 71, 15], [170, 52, 10], [175, 46, 14], [174, 48, 17], [164, 57, 19],
                        [168, 55, 15]]
yellow_bright_colors = [[142, 115, 33], [144, 103, 14], [142, 104, 22], [142, 89, 22], [144, 112, 14], [144, 119, 15],
                        [141, 141, 25]]
green_bright_colors = [[50, 192, 50], [41, 192, 27], [52, 186, 22], [67, 179, 22], [82, 173, 21], [96, 164, 19],
                       [13, 156, 25]]
blue_bright_colors = [[52, 80, 114], [75, 91, 129], [80, 94, 178], [69, 74, 186], [37, 34, 192], [42, 32, 186],
                      [51, 26, 174]]
purple_bright_colors = [[40, 26, 81], [39, 19, 77], [47, 27, 75], [57, 30, 68], [62, 38, 72], [60, 30, 66],
                        [63, 25, 57]]
white_bright_colors = [[191, 74, 74], [154, 110, 74], [113, 172, 74], [74, 180, 85], [74, 135, 130], [79, 74, 185],
                       [112, 74, 153]]

red_colors = [red_dim_colors, red_mid_colors, red_bright_colors]
orange_colors = [orange_dim_colors, orange_mid_colors, orange_bright_colors]
yellow_colors = [yellow_dim_colors, yellow_mid_colors, yellow_bright_colors]
green_colors = [green_dim_colors, green_mid_colors, green_bright_colors]
blue_colors = [blue_dim_colors, blue_mid_colors, blue_bright_colors]
purple_colors = [purple_dim_colors, purple_mid_colors, purple_bright_colors]
white_colors = [white_dim_colors, white_mid_colors, white_bright_colors]
playa_palette = [red_colors, orange_colors, yellow_colors, green_colors, blue_colors, purple_colors, white_colors]

# Extra color palettes defined by Lee and used in pride and other events
fruit_loop = [[25, 0, 25], [25, 15, 0], [180, 10, 70], [140, 60, 180], [180, 60, 60], [255, 255, 120], [255, 100, 180]]
icy_bright = [[37, 28, 60], [70, 0, 28], [255, 108, 189], [0, 172, 238], [44, 133, 215], [255, 255, 255],
              [254, 207, 24]]
watermelon = [[40, 29, 35], [5, 45, 15], [47, 140, 9], [72, 160, 5], [148, 33, 137], [47, 192, 91], [70, 190, 91]]
pride = [[255, 0, 0], [255, 127, 0], [255, 255, 0], [0, 255, 0], [0, 0, 255], [75, 0, 130], [148, 0, 211]]
edirp = [[148, 0, 211], [75, 0, 130], [0, 0, 255], [0, 255, 0], [255, 255, 0], [255, 127, 0], [255, 0, 0]]
edm_palettes = [fruit_loop, icy_bright, watermelon, pride, edirp]

#  Time constants
STATIC_START = time.mktime(time.strptime('2017-Aug-27 06:20', '%Y-%b-%d %H:%M'))  # Sunrise Sunday
STATIC_END = time.mktime(time.strptime('2017-Aug-28 06:21', '%Y-%b-%d %H:%M'))  # Sunrise Monday
BURNING_MAN_START = time.mktime(time.strptime('2017-Aug-28 06:21', '%Y-%b-%d %H:%M'))
BURNING_MAN_END = time.mktime(time.strptime('2017-Sep-4 19:23', '%Y-%b-%d %H:%M'))
sunrise_time = [time.mktime(time.strptime('2017-Aug-27 06:20', '%Y-%b-%d %H:%M')),
                time.mktime(time.strptime('2017-Aug-28 06:21', '%Y-%b-%d %H:%M')),
                time.mktime(time.strptime('2017-Aug-29 06:22', '%Y-%b-%d %H:%M')),
                time.mktime(time.strptime('2017-Aug-30 06:23', '%Y-%b-%d %H:%M')),
                time.mktime(time.strptime('2017-Aug-31 06:24', '%Y-%b-%d %H:%M')),
                time.mktime(time.strptime('2017-Sep-1 06:25', '%Y-%b-%d %H:%M')),
                time.mktime(time.strptime('2017-Sep-2 06:26', '%Y-%b-%d %H:%M')),
                time.mktime(time.strptime('2017-Sep-3 06:27', '%Y-%b-%d %H:%M')),
                time.mktime(time.strptime('2017-Sep-4 06:28', '%Y-%b-%d %H:%M'))]
sunset_time = [time.mktime(time.strptime('2017-Aug-27 19:36', '%Y-%b-%d %H:%M')),
               time.mktime(time.strptime('2017-Aug-28 19:00', '%Y-%b-%d %H:%M')),
               time.mktime(time.strptime('2017-Aug-29 19:33', '%Y-%b-%d %H:%M')),
               time.mktime(time.strptime('2017-Aug-30 19:31', '%Y-%b-%d %H:%M')),
               time.mktime(time.strptime('2017-Aug-31 19:30', '%Y-%b-%d %H:%M')),
               time.mktime(time.strptime('2017-Sep-1 19:28', '%Y-%b-%d %H:%M')),
               time.mktime(time.strptime('2017-Sep-2 19:27', '%Y-%b-%d %H:%M')),
               time.mktime(time.strptime('2017-Sep-3 19:25', '%Y-%b-%d %H:%M')),
               time.mktime(time.strptime('2017-Sep-4 19:23', '%Y-%b-%d %H:%M'))]

#  Show modes
SUNRISE = 0
DAY = 1
SUNSET = 2
NIGHT = 3
show_mode = SUNRISE
bm_day_index = -1
virtual_time = -1.0
IDEAL_MEDITATION_SECONDS = 20 * 60


def set_show_mode(mode):
    global show_mode
    show_mode = mode


def get_show_mode():
    global show_mode
    return show_mode


#  Art car values
NO_ART_CAR = -1
ART_CAR_HELLO_DURATION = 30  # seconds before entire structure edm art car takeover
ART_CAR_MIN_HELLO_DURATION = 5  # seconds before sending ring hello animation
# art_car_hello = False # not used
ART_CAR_AMPLITUDE_THRESHOLD = 100  # TODO calibrate appropriately... keep track of variation over time would be best but can get messy

# testing_meditation_seconds = 20
color_evolution_timer = time.time()

NUM_ANIMATIONS = 9  # animation programs are numbered 0 through 8
TIME_LIMIT = 5  # number of seconds between animation changes
auto_show = None
last_show_change_sec = 0.0

show_parameters = [0] * NUM_PARAMETERS
show_colors = [[33 for rgb in range(0, 3)] for i in range(0, NUM_COLORS_PER_PALETTE)]  # invalid values


def constrained_random_parameter(i):
    if show_bounds[i][0] == -1 and show_bounds[i][1] == 2:
        new_parameter = show_bounds[i][randint(0, 1)]  # no zero value
    else:
        new_parameter = randint(show_bounds[i][0], show_bounds[i][1])
    # change to unsigned int for passing to due
    if new_parameter < 0:
        new_parameter += 256
    return new_parameter


def constrain_show():
    global show_parameters
    #if show_parameters[BACKGROUND_INDEX] == 0 and show_parameters[MIDLAYER_INDEX] == 0 and show_parameters[
    #    SPARKLE_INDEX] == 0:
    #    show_parameters[BACKGROUND_INDEX] = 1  # never black
    pass


# ------------------------------------------------- print_parameters() --------------------------------------------

def print_parameters():
    global show_mode, bm_day_index, virtual_time

    if show_mode == SUNRISE:
        print_mode = 'sunrise meditation'
    elif show_mode == DAY:
        print_mode = 'daytime'
    elif show_mode == SUNSET:
        print_mode = 'sunset meditation'
    elif show_mode == NIGHT:
        print_mode = 'night'
    elif DEBUG:
        print 'illegal show mode'
    if DEBUG:
        print ''
        print "--> Show parameters -- day", bm_day_index, 'during', print_mode
        print "    base parameters", show_parameters[BASE_PARAM_START:BASE_PARAM_END + 1]
        print "    mid parameters", show_parameters[MID_PARAM_START:MID_PARAM_END + 1]
        print "    sparkle parameters", show_parameters[SPARKLE_PARAM_START:SPARKLE_PARAM_END + 1]
        print "    7 color, ring, beat, palette change", show_parameters[
                                                       SEVEN_PAL_BEAT_PARAM_START:SEVEN_PAL_BEAT_PARAM_END + 1]
        print "    transition parameters", show_parameters[TRANS_PARAM_START:TRANS_PARAM_END + 1]
        print "    colors", show_colors
        print ' '


# -------------------------------------------- mid_lower_bound() -------------------------------------------

def mid_lower_bound(mid_step):

    low = NUM_MID_COLORS_PER_FAMILY * bm_day_index

    time_elapsed = time.time() - color_evolution_timer
    num_steps = IDEAL_MEDITATION_SECONDS / mid_step

    for n in range(1, num_steps):
        if time_elapsed > n * mid_step:  # at least n periods have passed
            low -= 1
            # print "new low ", low
    return low


# -------------------------------------------- sp_lower_bound() -------------------------------------------

def sp_lower_bound(sparkle_step):

    low = NUM_SPARKLE_COLORS_PER_FAMILY * bm_day_index

    time_elapsed = time.time() - color_evolution_timer
    num_steps = IDEAL_MEDITATION_SECONDS / sparkle_step

    for n in range(1, num_steps):
        if time_elapsed > n * sparkle_step:  # at least n periods have passed
            low -= 1
            # print "new low ", low
    return low


# ------------------------------------------ choose_new_playa_palette () ----------------------------------------------

def choose_new_playa_palette():
    global SUNRISE, DAY, SUNSET, NIGHT, virtual_time, show_mode, bm_day_index

    if (show_mode == SUNRISE) or (show_mode == DAY):  # use only day's chakra colors
        bg_order = sample(
            range(NUM_BASE_COLORS_PER_FAMILY * bm_day_index, NUM_BASE_COLORS_PER_FAMILY * (bm_day_index + 1)), 2)
        mid_order = sample(
            range(NUM_MID_COLORS_PER_FAMILY * bm_day_index, NUM_MID_COLORS_PER_FAMILY * (bm_day_index + 1)), 3)
        sp_order = sample(
            range(NUM_SPARKLE_COLORS_PER_FAMILY * bm_day_index, NUM_SPARKLE_COLORS_PER_FAMILY * (bm_day_index + 1)), 2)

    elif show_mode == SUNSET:
        if bm_day_index == 0:
            sp_low = 0
            mid_low = 0
        else:
            sparkle_step_duration = IDEAL_MEDITATION_SECONDS / (NUM_SPARKLE_COLORS_PER_FAMILY * bm_day_index)
            mid_step_duration = IDEAL_MEDITATION_SECONDS / (NUM_MID_COLORS_PER_FAMILY * bm_day_index)
            sp_low = sp_lower_bound(sparkle_step_duration)
            mid_low = mid_lower_bound(mid_step_duration)

        bg_order = sample(
            range(NUM_BASE_COLORS_PER_FAMILY * bm_day_index, NUM_BASE_COLORS_PER_FAMILY * (bm_day_index + 1)), 2)
        mid_order = sample(range(mid_low, NUM_MID_COLORS_PER_FAMILY * (bm_day_index + 1)), 3)
        sp_order = sample(range(sp_low, NUM_SPARKLE_COLORS_PER_FAMILY * (bm_day_index + 1)), 2)

    else:  # night time: use all previous colors; background must always be chosen from day's chakara color
        bg_order = sample(
            range(NUM_BASE_COLORS_PER_FAMILY * bm_day_index, NUM_BASE_COLORS_PER_FAMILY * (bm_day_index + 1) - 1), 2)
        mid_order = sample(range(0, NUM_MID_COLORS_PER_FAMILY * (bm_day_index + 1)), 3)
        sp_order = sample(range(0, NUM_SPARKLE_COLORS_PER_FAMILY * (bm_day_index + 1)), 2)

    show_colors[0] = playa_palette[bm_day_index][0][bg_order[0] % NUM_BASE_COLORS_PER_FAMILY]
    show_colors[1] = playa_palette[bm_day_index][0][bg_order[1] % NUM_BASE_COLORS_PER_FAMILY]
    show_colors[2] = playa_palette[mid_order[0] / NUM_MID_COLORS_PER_FAMILY][1][
        mid_order[0] % NUM_MID_COLORS_PER_FAMILY]
    show_colors[3] = playa_palette[mid_order[1] / NUM_MID_COLORS_PER_FAMILY][1][
        mid_order[1] % NUM_MID_COLORS_PER_FAMILY]
    show_colors[4] = playa_palette[mid_order[2] / NUM_MID_COLORS_PER_FAMILY][1][
        mid_order[2] % NUM_MID_COLORS_PER_FAMILY]
    show_colors[5] = playa_palette[sp_order[0] / NUM_SPARKLE_COLORS_PER_FAMILY][2][
        sp_order[0] % NUM_SPARKLE_COLORS_PER_FAMILY]
    show_colors[6] = playa_palette[sp_order[1] / NUM_SPARKLE_COLORS_PER_FAMILY][2][
        sp_order[1] % NUM_SPARKLE_COLORS_PER_FAMILY]

    if DEBUG:
        print ''
        print '---> Setting new palette on day', bm_day_index, 'in mode', show_mode
        print '     colors chosen from days', bm_day_index, bm_day_index, ':', mid_order[0] / NUM_MID_COLORS_PER_FAMILY, mid_order[1] / NUM_MID_COLORS_PER_FAMILY, mid_order[2] / NUM_MID_COLORS_PER_FAMILY, ':', sp_order[0] / NUM_SPARKLE_COLORS_PER_FAMILY, sp_order[1] / NUM_SPARKLE_COLORS_PER_FAMILY
        print '    ', show_colors

        
def use_test_palette():
    show_colors[0] = [0, 0, 25]
    show_colors[1] = [0, 10, 10]
    show_colors[2] = [0, 0, 175]
    show_colors[3] = [128, 0, 160]
    show_colors[4] = [0, 128, 128]
    show_colors[5] = [128, 128, 255]
    show_colors[6] = [64, 124, 245]



BASE = 0
MID = 1
HIGH = 2
next_base_index = 0
next_mid_index = 2
next_high_index = 5

# ------------------------------------------ update_playa_palette () ----------------------------------------------

def update_playa_palette(sound_day, frequency):
    global bm_day_index, next_base_index, next_mid_index, next_high_index

    if DEBUG:
        print ''
        print '---> Updating palette on day', bm_day_index, 'in mode', show_mode
        print '     current colors:', show_colors
	# print '     next base, mid, high indices:', next_base_index, next_mid_index, next_high_index
        # print '     changing frequency', frequency, 'to day ', sound_day, '...'
                        
    if (frequency == BASE):  # always choose from today's color palette
        new_bg = randint(0, NUM_BASE_COLORS_PER_FAMILY - 1)
        print "     updating base index", next_base_index, "to base color", new_bg
        show_colors[next_base_index] = playa_palette[bm_day_index][0][new_bg]
        if DEBUG:
            changed = next_base_index
        next_base_index = (next_base_index + 1) % 2
        if DEBUG:
            print "     after update, next base index is", next_base_index

    elif (frequency == MID):
        new_mid = randint(0, NUM_MID_COLORS_PER_FAMILY - 1)
        print "     updating mid index", next_mid_index, "to mid color", new_mid
        show_colors[next_mid_index] = playa_palette[sound_day][1][new_mid]
        if DEBUG:
            changed = next_mid_index
        next_mid_index = ((next_mid_index - 2) + 1) % 3 + 2
        if DEBUG:
            print "     after update, next mid index is", next_mid_index

    elif (frequency == HIGH):
        new_high = randint(0, NUM_SPARKLE_COLORS_PER_FAMILY - 1)
        print "     updating high index", next_high_index, "to high color", new_high
        show_colors[next_high_index] = playa_palette[sound_day][2][new_high]
        if DEBUG:
            changed = next_high_index
        next_high_index = ((next_high_index - 5) + 1) % 2 + 5
        if DEBUG:
            print "     after update, next high index is", next_high_index

    elif DEBUG:
        print "frequency out of range"

    if DEBUG:
        print '     changed frequency', frequency, 'in position', changed, 'to day ', sound_day
        print '     new colors:', show_colors
                                    



# ----------------------------- choose_random_colors_from_edm_palette() -------------------------------------

current_edm_palette = None


def choose_random_colors_from_edm_palette():
    global current_edm_palette

    new = randint(0, len(edm_palettes) - 1)
    if DEBUG:
        print 'edm palette changed from', current_edm_palette, 'to', new
    current_edm_palette = new

    # choose which colors out of the chosen palette to use
    # shuffle the lower 2 colors, mid 3 colors, and upper 2 colors of chosen palette

    bg_order = sample(range(0, 2), 2)
    mid_order = sample(range(2, 5), 3)
    sp_order = sample(range(5, 7), 2)

    show_colors[0] = edm_palettes[current_edm_palette][bg_order[0]]
    show_colors[1] = edm_palettes[current_edm_palette][bg_order[1]]
    show_colors[2] = edm_palettes[current_edm_palette][mid_order[0]]
    show_colors[3] = edm_palettes[current_edm_palette][mid_order[1]]
    show_colors[4] = edm_palettes[current_edm_palette][mid_order[2]]
    show_colors[5] = edm_palettes[current_edm_palette][sp_order[0]]
    show_colors[6] = edm_palettes[current_edm_palette][sp_order[1]]


# ------------------------------------- edm_program() -----------------------------------------------
# show for when the journey is installed at an event with electronic dance music only
# parameters are somewhat randomly chosen

def edm_program(init=False):
    global bg_start_time, bg_parameter_start_time, mid_start_time, mid_parameter_start_time, sparkle_start_time, sparkle_parameter_start_time, palette_start_time
    global internal_aa_handler, show_parameters, show_colors

    # just in case anyone wants to use this command... disabling the intenal audio animations
    internal_aa_handler.set_do_animations(False)

    if init:
        if show_colors[0] == [33, 33, 33]:  # invalid values before initialization
            bg_start_time = bg_parameter_start_time = mid_start_time = mid_parameter_start_time = sparkle_start_time = sparkle_parameter_start_time = palette_start_time = time.time()

            # choose random starting values for each of the parameters
            for i in range(0, NUM_PARAMETERS):
                show_parameters[i] = constrained_random_parameter(i)
            constrain_show()
            choose_new_playa_palette()  # start with day 1 color palette

        print_parameters()
        return

    bg_time = bg_parameter_time = mid_time = mid_parameter_time = sparkle_time = sparkle_parameter_time = palette_time = time.time()

    # to avoid hard transitions, change disruptive base animation parameters only when you change background choice
    if bg_time - bg_start_time > BASE_TIME_LIMIT:
        bg_start_time = bg_time

        # change bg show parameters
        for i in range(BACKGROUND_INDEX, BACKGROUND_INDEX + 4):
            show_parameters[i] = constrained_random_parameter(i)
            if DEBUG:
                print "background parameter ", i, "changed to ", show_parameters[i]

    if bg_parameter_time - bg_parameter_start_time > BASE_PARAMETER_TIME_LIMIT:
        bg_parameter_start_time = bg_parameter_time

        # choose which parameter to change
        change_bg = randint(BACKGROUND_INDEX + 4, MIDLAYER_INDEX - 1)
        show_parameters[change_bg] = constrained_random_parameter(change_bg)

        if DEBUG:
            print "background parameter ", change_bg, "changed to ", show_parameters[change_bg]

    # to avoid hard transitions, change disruptive mid animation parameters only when you change mid layer choice
    if mid_time - mid_start_time > MID_TIME_LIMIT:
        mid_start_time = mid_time

        # change mid show parameters
        for i in range(MIDLAYER_INDEX, MIDLAYER_INDEX + 5):
            show_parameters[i] = constrained_random_parameter(i)
            if DEBUG:
                print "mid parameter ", i, "changed to ", show_parameters[i]

    if mid_parameter_time - mid_parameter_start_time > MID_PARAMETER_TIME_LIMIT:
        mid_parameter_start_time = mid_parameter_time

        # choose which parameter to change
        change_mid = randint(MIDLAYER_INDEX + 5, SPARKLE_INDEX - 1)
        show_parameters[change_mid] = constrained_random_parameter(change_mid)
        if DEBUG:
            print "mid parameter ", change_mid, "changed to ", show_parameters[change_mid]

    if sparkle_time - sparkle_start_time > SPARKLE_TIME_LIMIT:
        sparkle_start_time = sparkle_time

        # change sparkle animation
        show_parameters[SPARKLE_INDEX] = constrained_random_parameter(SPARKLE_INDEX)
        if DEBUG:
            print "sparkle choice changed to ", show_parameters[SPARKLE_INDEX]

    # can change sparkle parameters independently of changing sparkle animation, without having hard transitions
    if sparkle_parameter_time - sparkle_parameter_start_time > SPARKLE_PARAMETER_TIME_LIMIT:
        sparkle_parameter_start_time = sparkle_parameter_time

        # choose which parameter to change
        change_sparkle = randint(SPARKLE_INDEX + 1, SPARKLE_INDEX + 10)
        show_parameters[change_sparkle] = constrained_random_parameter(change_sparkle)
        if DEBUG:
            print "sparkle parameter ", change_sparkle, "changed to ", show_parameters[change_sparkle]

    constrain_show()  # keep the lights on

    if palette_time - palette_start_time > PALETTE_TIME_LIMIT:
        palette_start_time = palette_time

        choose_random_colors_from_edm_palette()
        # For Lee testing: uncomment this to stick with day 1 colors
        # choose_new_playa_palette()


# sunrise and sunset mediation animiatons
def meditaiton_animations(ignored=True):

    if show_parameters[SEVEN_PAL_BEAT_PARAM_START] in range(NUM_7_COLOR_MEDITATION_ANIMATIONS_START, NUM_7_COLOR_MEDITATION_ANIMATIONS_END + 1):
        # only running single meditation animation per sunset/rise" 
        if DEBUG:
            print "CONTINUING doing MEDITAITON ANIMATIONS... zen out, okay?" 
    else:
        # select the meditation animation to use
        if DEBUG:
            print "START doing MEDITAITON ANIMATIONS... zen out, okay?"

        show_parameters[SEVEN_PAL_BEAT_PARAM_START] = randint(NUM_7_COLOR_MEDITATION_ANIMATIONS_START,
                                                              NUM_7_COLOR_MEDITATION_ANIMATIONS_END)
        show_parameters[SEVEN_PAL_BEAT_PARAM_END - 1] = constrained_random_parameter(SEVEN_PAL_BEAT_PARAM_END - 1)
        show_parameters[SEVEN_PAL_BEAT_PARAM_END] = randint(NUM_PALETTE_CHANGE_STYLES - 2,
                                                            NUM_PALETTE_CHANGE_STYLES)  # want transitions to be especially gradual
    
    show_parameters[SPARKLE_PARAM_START] = 0 #keeping zero to ensure seeing the med animation
    show_parameters[MID_PARAM_START] = 0 #keeping zero to ensure seeing the med animation
    show_parameters[BASE_PARAM_START] = 0 #keeping zero to ensure seeing the med animation

    choose_new_playa_palette()
    constrain_show()


# choose random 7 color animation values for full structure animations
def art_car_edm(ignored=True):
    if DEBUG:
        print "DOING ART CAR EDM ANIMATIONS!"

    # NOTE art car ring num is param 29... between the beat param "starts" and "ends"... should be fixed... eventually
    show_parameters[SEVEN_PAL_BEAT_PARAM_START] = randint(NUM_7_COLOR_ANIMATIONS_AC_EDM_START,
                                                          NUM_7_COLOR_ANIMATIONS_AC_EDM_END)
    show_parameters[SEVEN_PAL_BEAT_PARAM_END - 1] = constrained_random_parameter(SEVEN_PAL_BEAT_PARAM_END - 1)
    show_parameters[SEVEN_PAL_BEAT_PARAM_END] = constrained_random_parameter(SEVEN_PAL_BEAT_PARAM_END)
    choose_random_colors_from_edm_palette()
    constrain_show()


# TEST_CYCLE_MINUTES = 5  # rush through the entire week in this number of minutes
# For Lee testing: uncomment this
TEST_CYCLE_MINUTES = 15
NUM_DAYS = int((BURNING_MAN_END - BURNING_MAN_START) / 86400 + 0.5)


# ------------------------ set_playa_mode() -------------------------------
# returns SUNRISE, DAY, SUNSET, NIGHT
# this is a hack for testing - I'll fix it and incorporate kienen's stuff soon -- Diane

def set_playa_mode():

    global SUNRISE, DAY, SUNSET, NIGHT, NUM_DAYS, IDEAL_MEDITATION_SECONDS, show_mode, color_evolution_timer, bm_day_index

    # this is also set in playa_program, but not before the start of burning man
    # we need this value to be allowed to be -1 to start static
    #bm_day_index = int((virtual_time - BURNING_MAN_START) / 86400) % NUM_DAYS
    #if DEBUG:
    #    print "******* in set playa mode virtual time is", time.ctime(virtual_time), 'on day', bm_day_index

    # todays_length = sunset_time[bm_day_index] - sunrise_time[bm_day_index]

    if (show_mode == NIGHT) and (virtual_time >= sunrise_time[bm_day_index]):
        show_mode = SUNRISE
        choose_new_playa_palette()
    elif (show_mode == DAY) and (virtual_time >= sunset_time[bm_day_index]):
        show_mode = SUNSET
        color_evolution_timer = time.time()
        choose_new_playa_palette()

"""
    if virtual_time - sunrise_time[bm_day_index + 1] <= IDEAL_MEDITATION_SECONDS:
        show_mode = SUNRISE
        if DEBUG:
            print 'now it is sunrise'
    elif virtual_time - sunset_time[bm_day_index + 1] < 0:
        show_mode = DAY
        if DEBUG:
            print 'now it is daytime'
    elif virtual_time - sunset_time[bm_day_index + 1] <= IDEAL_MEDITATION_SECONDS:
        show_mode = SUNSET
        if DEBUG:
            print 'now it is sunset'
        color_evolution_timer = time.time()
    else:
        show_mode = NIGHT
        if DEBUG:
            print 'now it is night'
    choose_new_playa_palette()


    # fixme: Kienen's code will set day and night show_modes at end of meditations
    #   i'll set these next 2 on timer for now, but this won't work for playa bc meditations are not same length

    #    if show_mode == SUNRISE:
    #        play sunrise meditation
    #    if show_mode == SUNSET:
    #        play sunset meditation
    #    if sunrise_meditation_just_ended:
    #        show_mode = DAY
    #    if sunset_meditation_just_ended:
    #        show_mode = NIGHT

    elif (show_mode == SUNRISE) and (virtual_time >= (sunrise_time[bm_day_index] + IDEAL_MEDITATION_SECONDS)):
        show_mode = DAY

    elif (show_mode == SUNSET) and (virtual_time >= (sunset_time[bm_day_index] + IDEAL_MEDITATION_SECONDS)):
        show_mode = NIGHT

    else:
        print 'not at burning man'
        exit(1)

    choose_new_playa_palette()
    if DEBUG:
        print '  new colors at day', bm_day_index, "show_mode ", show_mode, show_colors
    #return show_mode
"""


# ------------------------------- playa_program() ----------------------------------
#  This function initializes the playa program. If time is before burning man start, then it sets variables
#  so that a 3 minute test program runs

real_start_time = -1.0


time_speed_factor = 1
def playa_program(init=False):
    global real_start_time, testing_meditation_seconds, time_speed_factor, show_mode, virtual_time, bm_day_index

    real_time = time.time()
    if init:  # set up to run test program
        if real_start_time < 0:
            quantify_magnitude_impact 
            time_speed_factor = 1
            #time_speed_factor = float(NUM_DAYS * 60 * 24) / TEST_CYCLE_MINUTES	# 60*24 == minutes per day
            #testing_meditation_seconds = int(IDEAL_MEDITATION_SECONDS * time_speed_factor)  # / 233)	# 233 produces about 1/5 of the day with a 3 minute test cycle
            # real_start_time = real_time
            # real_start_time = BURNING_MAN_START - time.time()
            real_start_time = time.time()
            real_time = time.time()
            do_internal_sound_animations(init)
            show_parameters[SEVEN_PAL_BEAT_PARAM_START] = 0	# no EDM animations
            show_mode = SUNRISE            
        return

    if real_start_time == BURNING_MAN_START:
        virtual_time = real_time	# this is the live show at Burning Man
        show_mode = SUNRISE
        print "shouldn't happen"
    else:
        if BURNING_MAN_START <= real_time and real_time < BURNING_MAN_END:
            time_speed_factor = 1.0
            if DEBUG:
                print 'Welcome home!'	# Burning Man has just begun!
            real_start_time = BURNING_MAN_START
            virtual_time = real_time
        else:
            virtual_time = STATIC_START + (real_time - real_start_time) * time_speed_factor

    if DEBUG:
        print ' '
        print '---> Playa time advanced to', time.ctime(virtual_time)

    if virtual_time < BURNING_MAN_START:
        # fixme: add this code
        print 'static mode : need to add code to call static sound and animation'
        return

    bm_day_index = int((virtual_time - BURNING_MAN_START) / 86400) % NUM_DAYS
    if DEBUG and bm_day_index == 7:  # burning man is over
        print "Burning man has ended"
        exit(1)

    # Show mode is set in music.py
    show_mode_before = show_mode
    set_playa_mode()

    if show_mode == SUNRISE or show_mode == SUNSET:  # sunrise or sunset meditaiton time
        # do meditation animations
        meditaiton_animations()
        return
    elif (show_mode == 1 or show_mode == 3) and (show_mode_before == 0 or show_mode_before == 2):
        # need to stop the meditation animations
        show_parameters[SEVEN_PAL_BEAT_PARAM_START] = 0
        if DEBUG:
            print "stopping meditation animations"

    # do intenral sound animations after time progresses for palette purposes
    do_internal_sound_animations()


def do_date(ignored, when):
    global real_start_time
    try:
        day, hour = string.split(when, None, 1)  # one token separated by whitespace from the next
        day = int(day)
        hour, minute = string.split(hour, ':', 1)
        hour = int(hour)
        minute = int(minute)
    except (AttributeError, ValueError):  # no whitespace or non-integer
        print 'Usage: day number hour:min'
        return
    day = time.strftime('%Y-%b-%d', time.localtime(1503946800 + day * 86400))  # Monday is day 0
    when = time.mktime(time.strptime('%s %u:%u' % (day, hour, minute), '%Y-%b-%d %H:%M'))
    real_start_time = time.time() + (STATIC_START - when) / time_speed_factor



# ------------------------------ internal_sound_animations_program() -----------------------------------------------
# show for when the journey playing its internal audio and the external audio
# is not past threshold amount aka art car not detected or a meditation is happening
def do_internal_sound_animations(init=False):
    global bg_start_time, bg_parameter_start_time, mid_start_time, mid_parameter_start_time, sparkle_start_time, sparkle_parameter_start_time, palette_start_time
    global internal_aa_handler

    # audio_msg = internalAninamtionHandler.get_last_audio_msg()
    if INTERNAL_ANIMATIONS_DEBUG:
        # if audio_msg is not None:
        # print 'performing internal audio show for ' + audio_msg
        print 'performing internal audio show'

    if init:
        if show_colors[0] == [33, 33, 33]:  # invalid values before initialization
            bg_start_time = bg_parameter_start_time = mid_start_time = mid_parameter_start_time = sparkle_start_time = sparkle_parameter_start_time = palette_start_time = time.time()

            # choose random starting values for each of the parameters
            for i in range(0, NUM_PARAMETERS):
                show_parameters[i] = constrained_random_parameter(i)
                if i in range(SEVEN_PAL_BEAT_PARAM_START, SEVEN_PAL_BEAT_PARAM_END + 1):
                    show_parameters[i] = 0  # edm animations... no need for here

            constrain_show()
            choose_new_playa_palette()  # start with day 1 color palette

    internal_aa_handler.set_do_animations(True)

    # if audio_msg is not None:
    #    interpret_audio_msg(audio_msg)

    # uses internal_audio_animation object pulls from event_queue
    drive_internal_animations()


def get_internal_animations_handler():
    global internal_aa_handler
    return internal_aa_handler


# set the appropriate layer's main animation based on audioInfo's predetermined suitiable animations
def set_appropriate_layer_main_animation(audioInfo):
    global bg_start_time, bg_parameter_start_time, mid_start_time, mid_parameter_start_time, sparkle_start_time, sparkle_parameter_start_time, palette_start_time

    set_random_layer_params = True  # can flip to false if it looks bad...

    suitable_main_animation = audioInfo.getRandomSuitableAnimation()
    # TODO grab the param value suggests from the audioInfo

    if suitable_main_animation is not None:
        if INTERNAL_ANIMATIONS_DEBUG:
            print "random suitable animation is " + str(suitable_main_animation) + " " + str(audioInfo.category)

        if str(audioInfo.category) == "LOW":  # and time.time() - bg_start_time >= BASE_MAIN_ANIMATION_SWITCH_LAG:
            if INTERNAL_ANIMATIONS_DEBUG:
                print "setting base main animiation to " + str(suitable_main_animation)

            show_parameters[BASE_PARAM_START] = suitable_main_animation
            bg_start_time = time.time()

            if set_random_layer_params:
                # change bg show parameters
                for i in range(BACKGROUND_INDEX + 1, MIDLAYER_INDEX - 1):
                    show_parameters[i] = constrained_random_parameter(i)
                    if DEBUG:
                        print "background parameter ", i, "changed to ", show_parameters[i]

                bg_parameter_start_time = bg_start_time

        elif str(audioInfo.category) == "MID":  # and time.time() - mid_start_time >= MID_MAIN_ANIMATION_SWITCH_LAG:
            if INTERNAL_ANIMATIONS_DEBUG:
                print "setting mid main animiation to " + str(suitable_main_animation)

            show_parameters[MID_PARAM_START] = suitable_main_animation
            mid_start_time = time.time()

            if set_random_layer_params:
                # change mid show parameters
                for i in range(MIDLAYER_INDEX + 1, SPARKLE_PARAM_START - 1):
                    show_parameters[i] = constrained_random_parameter(i)
                    if DEBUG:
                        print "mid parameter ", i, "changed to ", show_parameters[i]

                mid_parameter_start_time = mid_start_time

        elif str(
                audioInfo.category) == "HIGH":  # and time.time() - sparkle_start_time >= SPARKLE_MAIN_ANIMATION_SWITCH_LAG:
            if INTERNAL_ANIMATIONS_DEBUG:
                print "setting sparkle main animiation to " + str(suitable_main_animation)

            show_parameters[SPARKLE_PARAM_START] = suitable_main_animation
            sparkle_start_time = time.time()

            if set_random_layer_params:
                # choose which parameter to change
                change_sparkle = randint(SPARKLE_INDEX + 1, SPARKLE_INDEX + 10)
                show_parameters[change_sparkle] = constrained_random_parameter(change_sparkle)
                if DEBUG:
                    print "sparkle parameter ", change_sparkle, "changed to ", show_parameters[change_sparkle]

                sparkle_parameter_start_time = sparkle_start_time


# structure very similar to edm show but instead of simply timing, allow for the
# audio events to trigger the changes. EDM program looks 1000x than this v1... so let's see how this looks
def drive_internal_animations():
    global bg_start_time, bg_parameter_start_time, mid_start_time, mid_parameter_start_time, sparkle_start_time, sparkle_parameter_start_time, palette_start_time
    global show_parameters, show_colors, bm_day_index
    global internal_aa_handler

    if bm_day_index < 0:
        show_parameters[SPARKLE_PARAM_START] = 254 #the static animation
        show_parameters[MID_PARAM_START] = 0 #the static animation
        show_parameters[BASE_PARAM_START] = 0 #the static animation
    else:

        bg_time = bg_parameter_time = mid_time = mid_parameter_time = sparkle_time = sparkle_parameter_time = palette_time = time.time()

        next_audio_event = internal_aa_handler.progress_audio_queue()  # next_audio_event is a global but just to be more clear...

        if next_audio_event is not None and internal_aa_handler.doing_animations:
            # MAYBE: seperate thread handling polling event_queue and sending animations

            abso_diff_val = abs(next_audio_event.exec_time - timeMs())
            valid_time = abso_diff_val < 1000  # one sec within the auido event

            if INTERNAL_ANIMATIONS_DEBUG and valid_time:
                print "valid? " + str(valid_time) + " diff: " + str(abso_diff_val)
                print "Event: " + str(next_audio_event)

            if valid_time:  # valid 'next' event
                magnitude = next_audio_event.magnitude

                # to avoid hard transitions, change disruptive base animation parameters only when you change background choice
                if bg_time - bg_start_time > BASE_MAIN_ANIMATION_SWITCH_LAG:
                    bg_start_time = bg_time

                    # change bg show parameters
                    for i in range(BACKGROUND_INDEX + 1, BACKGROUND_INDEX + 4):
                        show_parameters[i] = constrained_weighted_parameter(i, magnitude)
                        if DEBUG:
                            print "background parameter ", i, "changed to ", show_parameters[i]

                if bg_parameter_time - bg_parameter_start_time > BASE_PARAMETER_SWTICH_LAG:
                    bg_parameter_start_time = bg_parameter_time

                    # choose which parameter to change
                    change_bg = randint(BACKGROUND_INDEX + 4, MIDLAYER_INDEX - 1)
                    show_parameters[change_bg] = constrained_weighted_parameter(change_bg, magnitude)
                    if DEBUG:
                        print "background parameter ", change_bg, "changed to ", show_parameters[change_bg]

                # to avoid hard transitions, change disruptive mid animation parameters only when you change mid layer choice
                if mid_time - mid_start_time > MID_MAIN_ANIMATION_SWITCH_LAG:
                    mid_start_time = mid_time

                    # change mid show parameters
                    for i in range(MIDLAYER_INDEX + 1, MIDLAYER_INDEX + 5):
                        show_parameters[i] = constrained_weighted_parameter(i, magnitude)
                        if DEBUG:
                            print "mid parameter ", i, "changed to ", show_parameters[i]

                if mid_parameter_time - mid_parameter_start_time > MID_PARAMETER_SWTICH_LAG:
                    mid_parameter_start_time = mid_parameter_time

                    # choose which parameter to change
                    change_mid = randint(MIDLAYER_INDEX + 5, SPARKLE_INDEX - 1)
                    show_parameters[change_mid] = constrained_weighted_parameter(change_mid, magnitude)
                    if DEBUG:
                        print "mid parameter ", change_mid, "changed to ", show_parameters[change_mid]
                '''
                if sparkle_time - sparkle_start_time > SPARKLE_MAIN_ANIMATION_SWITCH_LAG:
                    sparkle_start_time = sparkle_time

                    # change sparkle animation
                    show_parameters[SPARKLE_INDEX] = constrained_weighted_parameter(SPARKLE_INDEX, magnitude)
                    print "sparkle choice changed to ", show_parameters[SPARKLE_INDEX]
                '''
                # can change sparkle parameters independently of changing sparkle animation, without having hard transitions
                if sparkle_parameter_time - sparkle_parameter_start_time > SPARKLE_PARAMETER_SWTICH_LAG:
                    sparkle_parameter_start_time = sparkle_parameter_time

                    # choose which parameter to change
                    change_sparkle = randint(SPARKLE_INDEX + 1, SPARKLE_INDEX + 10)
                    show_parameters[change_sparkle] = constrained_weighted_parameter(change_sparkle, magnitude)
                    if DEBUG:
                        print "sparkle parameter ", change_sparkle, "changed to ", show_parameters[change_sparkle]

                if palette_time - palette_start_time > PALETTE_TIME_LIMIT:
                    palette_start_time = palette_time

                    choose_new_playa_palette()

                if float(magnitude) > 5.9:
                    show_parameters[SPARKLE_INDEX] = constrained_random_parameter(SPARKLE_INDEX)
                    show_parameters[MIDLAYER_INDEX] = constrained_random_parameter(MIDLAYER_INDEX)
                    show_parameters[BACKGROUND_INDEX] = constrained_random_parameter(BACKGROUND_INDEX)
                    choose_new_playa_palette()

                    if INTERNAL_ANIMATIONS_DEBUG:
                        print "event with magnitude " + str(
                            magnitude) + " triggered a MAJOR animation change event. ALL layer animations switched."

                # remove the 'actioned on' event from the queue
                try:
                    if internal_aa_handler.event_queue.size > 0:
                        if INTERNAL_ANIMATIONS_DEBUG:
                            print "removing actioned event: " + str(next_audio_event) + " size: " + str(
                                internal_aa_handler.event_queue.size)

                        internal_aa_handler.event_queue.remove(next_audio_event)
                    else:
                        internal_aa_handler.next_audio_event = None
                except AttributeError:
                    # print "event_queue is empty ", sys.exc_value
                    pass

        # necessary backup trigger now that new track selections are less frequent
        # to keep animations changing
        check_time_triggered_animations()

    constrain_show()  # keep the lights on


# check if time limits have been reached to trigger an animation param change
# this is necessary now that new track selections are less frequent.
# will only change params, never main layer animations.
# another adaptation from edm_program as it works fairly well
# changing params by an order magnitude of 1 so won't be nearly as drastic as edm_program
def check_time_triggered_animations():
    global bg_start_time, bg_parameter_start_time, mid_start_time, mid_parameter_start_time, sparkle_start_time, sparkle_parameter_start_time, palette_start_time
    global show_parameters, show_colors

    bg_time = bg_parameter_time = mid_time = mid_parameter_time = sparkle_time = sparkle_parameter_time = palette_time = time.time()

    magnitude = 1  # only changing params as if was an audio event of magnitude 1
    # to avoid hard transitions, change disruptive base animation parameters only when you change background choice
    if bg_time - bg_start_time > BASE_TIME_LIMIT:
        bg_start_time = bg_time

        # change bg show parameters
        for i in range(BACKGROUND_INDEX, BACKGROUND_INDEX + 4):
            show_parameters[i] = constrained_weighted_parameter(i, magnitude)
            if DEBUG:
                print "TIMEOUT: background parameter ", i, "changed to ", show_parameters[i]

    if bg_parameter_time - bg_parameter_start_time > BASE_PARAMETER_TIME_LIMIT:
        bg_parameter_start_time = bg_parameter_time

        # choose which parameter to change
        change_bg = randint(BACKGROUND_INDEX + 4, MIDLAYER_INDEX - 1)
        show_parameters[change_bg] = constrained_weighted_parameter(change_bg, magnitude)
        if DEBUG:
            print "TIMEOUT: background parameter ", change_bg, "changed to ", show_parameters[change_bg]

    # to avoid hard transitions, change disruptive mid animation parameters only when you change mid layer choice
    if mid_time - mid_start_time > MID_TIME_LIMIT:
        mid_start_time = mid_time

        # change mid show parameters
        for i in range(MIDLAYER_INDEX, MIDLAYER_INDEX + 5):
            show_parameters[i] = constrained_weighted_parameter(i, magnitude)
            if DEBUG:
                print "TIMEOUT: mid parameter ", i, "changed to ", show_parameters[i]

    if mid_parameter_time - mid_parameter_start_time > MID_PARAMETER_TIME_LIMIT:
        mid_parameter_start_time = mid_parameter_time

        # choose which parameter to change
        change_mid = randint(MIDLAYER_INDEX + 5, SPARKLE_INDEX - magnitude)
        show_parameters[change_mid] = constrained_random_parameter(change_mid)
        if DEBUG:
            print "TIMEOUT: mid parameter ", change_mid, "changed to ", show_parameters[change_mid]

    '''
    if sparkle_time - sparkle_start_time > SPARKLE_TIME_LIMIT:
        sparkle_start_time = sparkle_time

        # change sparkle animation
        show_parameters[SPARKLE_INDEX] = constrained_weighted_parameter(SPARKLE_INDEX, magnitude)
        print "sparkle choice changed to ", show_parameters[SPARKLE_INDEX]
    '''
    # can change sparkle parameters independently of changing sparkle animation, without having hard transitions
    if sparkle_parameter_time - sparkle_parameter_start_time > SPARKLE_PARAMETER_TIME_LIMIT:
        sparkle_parameter_start_time = sparkle_parameter_time

        # choose which parameter to change
        change_sparkle = randint(SPARKLE_INDEX, SPARKLE_INDEX + 10)
        show_parameters[change_sparkle] = constrained_weighted_parameter(change_sparkle, magnitude)
        if DEBUG:
            print "TIMEOUT: sparkle parameter ", change_sparkle, "changed to ", show_parameters[change_sparkle]

    constrain_show()  # keep the lights on

    if palette_time - palette_start_time > PALETTE_TIME_LIMIT:
        palette_start_time = palette_time

        choose_new_playa_palette()


# similar to constrained_random_parameter but changed dependent on magnitude
def constrained_weighted_parameter(i, magnitude):
    if show_bounds[i][0] == -1 and show_bounds[i][1] == 1:
        new_parameter = show_bounds[i][randint(0, 1)]  # no zero value
    else:
        old_parameter = show_parameters[i]
        up_down = randint(0, 2)
        if up_down == 1:
            new_parameter = (old_parameter + quantify_magnitude_impact(magnitude)) % show_bounds[i][1]
        else:
            new_parameter = (old_parameter - quantify_magnitude_impact(magnitude)) % show_bounds[i][1]

    # change to unsigned int for passing to due
    if new_parameter < 0:
        new_parameter += 256
    return new_parameter


# quantifying the standard deviation aka magnitude of event into show param quantities
def quantify_magnitude_impact(magnitude):
    magnitude = float(magnitude)

    if magnitude > 5.5:
        return randint(26, 30)
    elif magnitude > 5.0:
        return randint(21, 25)
    elif magnitude > 4.5:
        return randint(16, 20)
    elif magnitude > 4.0:
        return randint(11, 15)
    elif magnitude > 3.5:
        return randint(6, 10)
    else:
        return randint(1, 5)


def timeMs():
    return int(round(time.time() * 1000))


def do_set_show_parameter(ignored, parameters):
    try:
        param, value = string.split(parameters, None, 1)
        param = int(param)
        value = int(value)
    except (AttributeError, ValueError):  # no whitespace or non-integer
        print 'Usage: sp parameter_number value'
        return
    if param < 0 or NUM_PARAMETERS <= param:
        print 'parameter number must be from 0 to', NUM_PARAMETERS - 1
        return
    if value < show_bounds[param][0] or show_bounds[param][1] < value:
        print 'parameter', param, 'value must be from', show_bounds[param][0], 'to', show_bounds[param][1]
        return
    print 'show_parameters[%u] set to %d' % (param, value)
    if value < 0:
        value += 256
    show_parameters[param] = value
