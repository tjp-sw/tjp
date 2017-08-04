import numpy, string, time
from random import randint
from random import sample
from random import choice
import sys
from dataBaseInterface import DataBaseInterface
from audioInfo import AudioEvent, AudioFileInfo
from audio_event_queue import SortedDLL

# Non-Color Animation Parameter Constants
#
# All (most of) the parameters for a show are stored in an array called show_parameters, so that they
# can be passed around in one go. These constants are the indices into this show_parameters
# array holding that type of parameter value

NUM_7_COLOR_ANIMATIONS = 7
NUM_BASE_ANIMATIONS = 2
NUM_MID_ANIMATIONS = 7
NUM_SPARKLE_ANIMATIONS = 6

NUM_BEAT_EFFECTS = 8
NUM_PARAMETERS = 41
NUM_COLORS_PER_PALETTE = 7

NUM_BASE_TRANSITIONS = 1
NUM_MID_TRANSITIONS = 1
NUM_SPARKLE_TRANSITIONS = 1
NUM_EDM_TRANSITIONS = 1
NUM_TRANSITION_SPEEDS = 6
NUM_PALETTE_CHANGE_STYLES = 5

NUM_MID_ALPHA_MODES = 3
NUM_SPARKLE_ALPHA_MODES = 2

BASE_TIME_LIMIT = 31
BASE_PARAMETER_TIME_LIMIT = 19
MID_TIME_LIMIT = 29
MID_PARAMETER_TIME_LIMIT = 17
SPARKLE_TIME_LIMIT = 37
SPARKLE_PARAMETER_TIME_LIMIT =  13
PALETTE_TIME_LIMIT = 7
# For Lee testing
# PALETTE_TIME_LIMIT = 5

# constants to protect against too frequent param changes during dynamic show
BASE_MAIN_ANIMATION_SWITCH_LAG = 15
BASE_PARAMETER_SWTICH_LAG = 2.9
MID_MAIN_ANIMATION_SWITCH_LAG =  10
MID_PARAMETER_SWTICH_LAG = 1.9
SPARKLE_MAIN_ANIMATION_SWITCH_LAG = 5
SPARKLE_PARAMETER_SWTICH_LAG = 0.9
PALETTE_LAG = 7

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
SEVEN_PAL_BEAT_PARAM_END = 30
ALPHA_PARAM_START = 31
ALPHA_PARAM_END = 32
TRANS_PARAM_START = 33
TRANS_PARAM_END = 40

show_bounds = [  # order must match show_parameters
        # [min, max]
        # show bounds 0 through 7 concern base animations
        [0, NUM_BASE_ANIMATIONS],  # BACKGROUND_INDEX, which background animation to use
        [0, 255], # base color thickness
        [0, 255], # base black thickness
        [-128,127], # base ring offset
        [-1, 2], # base intra ring motion: -1 CCW, 0 none, 1 CW, 2 alternate, 3 split (down from top)
        [0, 255], # base intra ring speed
        [-1,1], # base inter ring motion: -1 = CCW, 0 = none, 1 = CW
        [0,255], # base inter ring speed
        # show bounds 8 through 16 concern mid layer animations
        [0, NUM_MID_ANIMATIONS],  # MIDLAYER_INDEX, which mid layer animation to use
        [1, 3], # mid num colors
        [0, 255],  # mid color thickness
        [0, 255],  # mid black thickness
        [-128,127],  # mid ring offset
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
        [0, 255], # sparkle min dim
        [0, 255], # sparkle max dim
        [0, 255], # sparkle range
        [0, 255], # sparkle spawn frequency, 0 == off entirely (Functions as a boolean when 0|1)
        # show bounds 28 through 28 concern 7-color edm animations
        [0, NUM_7_COLOR_ANIMATIONS],  # which 7 color animation to play, show bound 28
        #show bounds 29 through 32 recently added (maybe need to be renumbered)
        [1, NUM_PALETTE_CHANGE_STYLES], # Palette change style (0 is immediate)
        [0, NUM_BEAT_EFFECTS],  # which beat effect to use to respond to beat with LEDs, show bound 30
        [0, NUM_MID_ALPHA_MODES], # how to blend mid layer with background layer
        [0, NUM_SPARKLE_ALPHA_MODES], # how to blend sparkle layer with mid and background layers
        #show bounds 33 through 40 concern animation transitions
        [1, NUM_BASE_TRANSITIONS], # how to transition the background animation (0 is immediate)
        [1, NUM_TRANSITION_SPEEDS], # how fast to transition the background animation
        [1, NUM_MID_TRANSITIONS], # how to transition the mid animation (0 is immediate)
        [1, NUM_TRANSITION_SPEEDS], # how fast to transition the mid animation
        [1, NUM_SPARKLE_TRANSITIONS], # how to transition the sparkle animation (0 is immediate)
        [1, NUM_TRANSITION_SPEEDS], # how fast to transition the sparkle animation
        [1, NUM_EDM_TRANSITIONS], # how to transition the EDM animation (0 is immediate)
        [1, NUM_TRANSITION_SPEEDS] # how fast to transition the EDM animation
    ]

# Pre-defined color palettes for each chakra color
NUM_BASE_COLORS_PER_FAMILY = 7
NUM_MID_COLORS_PER_FAMILY = 14
NUM_SPARKLE_COLORS_PER_FAMILY = 7

red_dim_colors = [[20, 0, 0], [13, 3, 5], [17, 2, 0], [13, 6, 4], [14, 4, 5], [15, 6, 6], [13, 7, 5]]
orange_dim_colors = [[255, 127, 0], [250, 122, 0], [245, 117, 0], [240, 112, 0], [255, 127, 0], [250, 122, 0], [245, 117, 0], [240, 112, 0]]
yellow_dim_colors = [[255, 255, 0], [250, 250, 0], [245, 245, 0], [240, 240, 0], [255, 255, 0], [250, 250, 0], [245, 245, 0], [240, 240, 0]]
green_dim_colors = [[255, 255, 0], [250, 250, 0], [245, 245, 0], [240, 240, 0], [255, 255, 0], [250, 250, 0], [245, 245, 0], [240, 240, 0]]
blue_dim_colors = [[0, 0, 255], [0, 0, 250], [0, 0, 245], [0, 0, 240], [0, 0, 255], [0, 0, 250], [0, 0, 245], [0, 0, 240]]
purple_dim_colors = [[75, 0, 130], [70, 0, 125], [65, 0, 120], [60, 0, 115], [75, 0, 130], [70, 0, 125], [65, 0, 120], [60, 0, 115]]
white_dim_colors = [[37, 28, 60], [32, 23, 55], [27, 18, 50], [22, 13, 45], [37, 28, 60], [32, 23, 55], [27, 18, 50], [22, 13, 45]]
red_mid_colors = [[232, 8, 16], [232, 0, 32], [224, 32, 0], [224, 24, 8], [224, 16, 16], [224, 8, 24], [224, 0, 32],
[128, 4, 9], [253, 0, 2], [217, 1, 23], [253, 0, 2], [142, 0, 5], [132, 6, 6], [254, 253, 253]]
orange_mid_colors = [[255, 127, 0], [250, 122, 0], [245, 117, 0], [240, 112, 0], [235, 107, 0], [230, 102, 0], [255, 127, 0], [250, 122, 0], [245, 117, 0], [240, 112, 0], [235, 107, 0], [230, 102, 0], [255, 127, 0], [250, 122, 0], [245, 117, 0], [240, 112, 0], [235, 107, 0], [230, 102, 0]]
yellow_mid_colors = [[255, 255, 0], [250, 250, 0], [245, 245, 0], [240, 240, 0], [235, 235, 0], [230, 230, 0], [255, 255, 0], [250, 250, 0], [245, 245, 0], [240, 240, 0], [235, 235, 0], [230, 230, 0], [255, 255, 0], [250, 250, 0], [245, 245, 0], [240, 240, 0], [235, 235, 0], [230, 230, 0]]
green_mid_colors = [[0, 255, 0], [0, 250, 0], [0, 245, 0], [0, 240, 0], [0, 235, 0], [0, 230, 0], [0, 255, 0], [0, 250, 0], [0, 245, 0], [0, 240, 0], [0, 235, 0], [0, 230, 0], [0, 255, 0], [0, 250, 0], [0, 245, 0], [0, 240, 0], [0, 235, 0], [0, 230, 0]]
blue_mid_colors = [[0, 0, 255], [0, 0, 250], [0, 0, 245], [0, 0, 240], [0, 0, 235], [0, 0, 230], [0, 0, 255], [0, 0, 250], [0, 0, 245], [0, 0, 240], [0, 0, 235], [0, 0, 230], [0, 0, 255], [0, 0, 250], [0, 0, 245], [0, 0, 240], [0, 0, 235], [0, 0, 230]]
purple_mid_colors = [[75, 0, 130], [70, 0, 125], [65, 0, 120], [60, 0, 115], [55, 0, 110], [50, 0, 105], [75, 0, 130], [70, 0, 125], [65, 0, 120], [60, 0, 115], [55, 0, 110], [50, 0, 105], [75, 0, 130], [70, 0, 125], [65, 0, 120], [60, 0, 115], [55, 0, 110], [50, 0, 105]]
white_mid_colors = [[37, 28, 60], [32, 23, 55], [27, 18, 50], [22, 13, 45], [27, 18, 50], [22, 13, 45], [37, 28, 60], [32, 23, 55], [27, 18, 50], [22, 13, 45], [27, 18, 50], [22, 13, 45], [37, 28, 60], [32, 23, 55], [27, 18, 50], [22, 13, 45], [27, 18, 50], [22, 13, 45]]
red_bright_colors = [[168, 39, 64], [174, 54, 71], [180, 52, 63], [189, 35, 41], [194, 41, 41], [186, 65, 60], [183, 91, 82]]
orange_bright_colors = [[255, 127, 0], [250, 122, 0], [245, 117, 0], [240, 112, 0], [255, 127, 0], [250, 122, 0], [245, 117, 0], [240, 112, 0]]
yellow_bright_colors = [[255, 255, 0], [250, 250, 0], [245, 245, 0], [240, 240, 0], [255, 255, 0], [250, 250, 0], [245, 245, 0], [240, 240, 0]]
green_bright_colors = [[0, 255, 0], [0, 250, 0], [0, 245, 0], [0, 240, 0], [0, 255, 0], [0, 250, 0], [0, 245, 0], [0, 240, 0]]
blue_bright_colors = [[0, 0, 255], [0, 0, 250], [0, 0, 245], [0, 0, 240], [0, 0, 255], [0, 0, 250], [0, 0, 245], [0, 0, 240]]
purple_bright_colors = [[75, 0, 130], [70, 0, 125], [65, 0, 120], [65, 0, 120], [75, 0, 130], [70, 0, 125], [65, 0, 120], [65, 0, 120]]
white_bright_colors = [[37, 28, 60], [32, 23, 55], [27, 18, 50], [22, 13, 45], [37, 28, 60], [32, 23, 55], [27, 18, 50], [22, 13, 45]]
red_colors = [red_dim_colors, red_mid_colors, red_bright_colors]
orange_colors = [orange_dim_colors, orange_mid_colors, orange_bright_colors]
yellow_colors = [yellow_dim_colors, yellow_mid_colors, yellow_bright_colors]
green_colors = [green_dim_colors, green_mid_colors, green_bright_colors]
blue_colors = [blue_dim_colors, blue_mid_colors, blue_bright_colors]
purple_colors = [purple_dim_colors, purple_mid_colors, purple_bright_colors]
white_colors = [white_dim_colors, white_mid_colors, white_bright_colors]
playa_palette = [red_colors, orange_colors, yellow_colors, green_colors, blue_colors, purple_colors, white_colors]

# Extra color palettes defined by Lee and used in pride and other events
fruit_loop = [[25,0,25], [25,15,0], [180,10,70], [140,60,180], [180,60,60], [255,255,120], [255,100,180]]
icy_bright = [[37,28,60], [70,0,28], [255,108,189], [0,172,238], [44,133,215], [255,255,255], [254,207,24]]
watermelon = [[40,29,35], [5,45,15], [47,140,9], [72,160,5], [148,33,137], [47,192,91], [70,190,91]]
pride = [[255, 0, 0], [255, 127, 0], [255, 255, 0], [0, 255, 0], [0, 0, 255], [75, 0, 130], [148, 0, 211]]
edirp = [[148, 0, 211], [75, 0, 130], [0, 0, 255], [0, 255, 0], [255, 255, 0], [255, 127, 0], [255, 0, 0]]
edm_palettes = [fruit_loop, icy_bright, watermelon, pride, edirp]

#  Time constants
STATIC_START = time.mktime(time.strptime('2017-Aug-27 06:20', '%Y-%b-%d %H:%M'))  # Sunrise Sunday
STATIC_END = time.mktime(time.strptime('2017-Aug-28 06:21', '%Y-%b-%d %H:%M'))  # Sunrise Monday
BURNING_MAN_START = time.mktime(time.strptime('2017-Aug-28 06:21', '%Y-%b-%d %H:%M'))
BURNING_MAN_END   = time.mktime(time.strptime('2017-Sep-4 19:23', '%Y-%b-%d %H:%M'))
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
                time.mktime(time.strptime('2017-Aug-28 19:35', '%Y-%b-%d %H:%M')),
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

#  Art car values
NO_ART_CAR = -1
ART_CAR_HELLO_DURATION = 30
art_car_hello = False
art_car = NO_ART_CAR  # if art car is detected, set to ring number nearest art car



NUM_ANIMATIONS = 9	# animation programs are numbered 0 through 8
TIME_LIMIT = 5		# number of seconds between animation changes
auto_show = None
last_show_change_sec = 0.0

show_parameters = [0] * NUM_PARAMETERS
show_colors = [[33 for rgb in range(0, 3)] for i in range(0, NUM_COLORS_PER_PALETTE)]	# invalid values
bm_day_index = 0

event_queue = SortedDLL()  # create sorted dll to act as the audio event queue (with super duper special powers)
NUM_AUDIO_CHANNELS = 7
current_internal_track_per_channel = [0] * NUM_AUDIO_CHANNELS
internal_audio_show = False  # triggers internal audio animations..
next_audio_event = AudioEvent(-1, -1, "init", "init")
INTERNAL_ANIMATIONS_DEBUG = False
internal_show_init = True

def constrained_random_parameter(i):
    if show_bounds[i][0] == -1 and show_bounds[i][1] == 1:
        new_parameter = show_bounds[i][randint(0,1)]	# no zero value
    else:
        new_parameter = randint(show_bounds[i][0], show_bounds[i][1])
    # change to unsigned int for passing to due
    if new_parameter < 0:
        new_parameter += 256
    return new_parameter

def constrain_show():
    global show_parameters
    if show_parameters[BACKGROUND_INDEX] == 0 and show_parameters[MIDLAYER_INDEX] == 0 and show_parameters[SPARKLE_INDEX] == 0:
        show_parameters[BACKGROUND_INDEX] = 1	# never black


# ------------------------------------------------- get random color ------------------------------------------------
# very much in progress
#  returns one sanctioned color as an *index* into fixed color array, weightedly-randomly chosen
# progresses through different time periods starting at end of sunset meditation; at each
# time period more of previous days' colors are allowed, with proportions rising first for
# previous day's color, then for day before that's color, etc
#
# at the end of the periods, today's chakra color is twice as likely to be chosen from as any of the
# previous days' colors, and choices remain this way until following sunrise meditation


def get_random_color(day, desired_period):
    # percentage color increase or decrease per time interval
    percent_step = 5
    percent_scaling_factor = 10

    # fixme: check num mid, sparkle.
    num_colors_today = day * NUM_BASE_COLORS_PER_FAMILY
    max_value = num_colors_today * percent_scaling_factor
    current_color = day - 1

    # target_other is probably not a multiple of increment, so last step for each color will move probability here
    # target_chakra = target_other * 2: final evolved probability for the day's chakra, twice as large as others

    target_other = max_value / (day + 1)

    increment = percent_step * max_value / 100
    iterations = 0

    probability_array = [0 for j in range(NUM_DAYS)]
    cumulative_array = [0 for j in range(NUM_DAYS)]

    # initialize percentages as scaled percent_step% for previous colors,
    # and rest% for day's color

    for ind in range(1, day):
        probability_array[ind] = increment
        cumulative_array[ind] = cumulative_array[ind - 1] + probability_array[ind]

        # the current day always has the highest probability
        probability_array[day] = max_value - increment * (day - 1)
        cumulative_array[day] = cumulative_array[day - 1] + probability_array[day]

    while (current_color >= 1) and (iterations <= desired_period):
        iterations += 1

        # update current color value until full / it tips at target_other
        if (probability_array[current_color] + increment) < target_other:

            # increase this color percent
            probability_array[current_color] += increment
            probability_array[day] -= increment

        else:

            remainder = target_other - probability_array[current_color]
            probability_array[current_color] = target_other
            probability_array[day] -= remainder

            # move to previous color index
            current_color -= 1

    # now in color probability situation for desired_period

    cumulative_array = [0 for j in range(NUM_DAYS)]
    cumulative_value = 0

    for ind in range(1, day + 1):
        cumulative_value += probability_array[ind]
        cumulative_array[ind] = cumulative_value

    print('probability array', probability_array)
    print('cumulative array', cumulative_array)

    # now use the cumulative probabilily array to generate a weighted random color

    rand = randint(1, max_value)
    ind = 1

    while rand > cumulative_array[ind]:
        ind += 1

    specific_color_index = rand / percent_scaling_factor

    # print('rand is', rand, 'color is', ind, "specific is", specific_color_index)

    return specific_color_index


# ------------------------------------------ choose_new_playa_palette () ----------------------------------------------
# function still in progress - will always return a palette, but maybe not the right one
# I include it early so that others will see that there are two types of palettes:
# - ones for internal sound mode which are generated here, and
# - ones for art car response called edm_palettes which are chosen in choose_random_colors_from_edm_palette()
# also, waiting for Lee to finish color choices

def choose_new_playa_palette():
    global SUNRISE, DAY, SUNSET, NIGHT

    if (show_mode == SUNRISE) or (show_mode == DAY):
        # use only day's chakra colors

        bg_order = sample(range(NUM_BASE_COLORS_PER_FAMILY * bm_day_index, NUM_BASE_COLORS_PER_FAMILY * (bm_day_index + 1)-1), 2)
        mid_order = sample(range(NUM_MID_COLORS_PER_FAMILY * bm_day_index, NUM_MID_COLORS_PER_FAMILY * (bm_day_index + 1)-1), 3)
        sp_order = sample(range(NUM_SPARKLE_COLORS_PER_FAMILY * bm_day_index, NUM_SPARKLE_COLORS_PER_FAMILY * (bm_day_index + 1)-1), 2)

    elif show_mode == SUNSET:
        # use color growth
        # fixme::: has to be updated to use color growth
        bg_order = sample(range(NUM_BASE_COLORS_PER_FAMILY * bm_day_index, NUM_BASE_COLORS_PER_FAMILY * (bm_day_index + 1)-1), 2)
        mid_order = sample(range(NUM_MID_COLORS_PER_FAMILY * bm_day_index, NUM_MID_COLORS_PER_FAMILY * (bm_day_index + 1)-1), 3)
        sp_order = sample(range(NUM_SPARKLE_COLORS_PER_FAMILY * bm_day_index, NUM_SPARKLE_COLORS_PER_FAMILY * (bm_day_index + 1)-1), 2)


    else:  # night time
        # use all previous colors
        # background must always be chosen from day's chakara color

        bg_order = sample(range(NUM_MID_COLORS_PER_FAMILY * bm_day_index, NUM_BASE_COLORS_PER_FAMILY * (bm_day_index + 1) -1), 2)
        mid_order = sample(range(0, NUM_MID_COLORS_PER_FAMILY * (bm_day_index + 1)-1), 3)
        sp_order = sample(range(0, NUM_SPARKLE_COLORS_PER_FAMILY * (bm_day_index + 1)-1), 2)

    show_colors[0] = playa_palette[bm_day_index][0][bg_order[0] % NUM_BASE_COLORS_PER_FAMILY]
    show_colors[1] = playa_palette[bm_day_index][0][bg_order[1] % NUM_BASE_COLORS_PER_FAMILY]
    show_colors[2] = playa_palette[mid_order[0] / NUM_MID_COLORS_PER_FAMILY][1][mid_order[0] % NUM_MID_COLORS_PER_FAMILY]
    show_colors[3] = playa_palette[mid_order[1] / NUM_MID_COLORS_PER_FAMILY][1][mid_order[1] % NUM_MID_COLORS_PER_FAMILY]
    show_colors[4] = playa_palette[mid_order[2] / NUM_MID_COLORS_PER_FAMILY][1][mid_order[2] % NUM_MID_COLORS_PER_FAMILY]
    show_colors[5] = playa_palette[sp_order[0] / NUM_SPARKLE_COLORS_PER_FAMILY][2][sp_order[0] % NUM_SPARKLE_COLORS_PER_FAMILY]
    show_colors[6] = playa_palette[sp_order[1] / NUM_SPARKLE_COLORS_PER_FAMILY][2][sp_order[1] % NUM_SPARKLE_COLORS_PER_FAMILY]

    print "show colors", show_colors


# ----------------------------- choose_random_colors_from_edm_palette() -------------------------------------

def choose_random_colors_from_edm_palette():
    # choose which colors out of the chosen palette to use
    #shuffle the lower 2 colors, mid 3 colors, and upper 2 colors of chosen palette
    bg_order = sample(range(0,2), 2)
    mid_order = sample(range(2,5), 3)
    sp_order = sample(range(5,7), 2)

    current_palette = show_parameters[29]
    show_colors[0] = edm_palettes[current_palette][bg_order[0]]
    show_colors[1] = edm_palettes[current_palette][bg_order[1]]
    show_colors[2] = edm_palettes[current_palette][mid_order[0]]
    show_colors[3] = edm_palettes[current_palette][mid_order[1]]
    show_colors[4] = edm_palettes[current_palette][mid_order[2]]
    show_colors[5] = edm_palettes[current_palette][sp_order[0]]
    show_colors[6] = edm_palettes[current_palette][sp_order[1]]


# ------------------------------------- edm_program() -----------------------------------------------
# show for when the journey is installed at an event with electronic dance music only
# parameters are somewhat randomly chosen

def edm_program(init=False):
    global bg_start_time, bg_parameter_start_time, mid_start_time, mid_parameter_start_time, sparkle_start_time, sparkle_parameter_start_time, palette_start_time
    global show_parameters, show_colors

    if init:
        if show_colors[0] == [33,33,33]:	# invalid values before initialization
            bg_start_time = bg_parameter_start_time = mid_start_time = mid_parameter_start_time = sparkle_start_time = sparkle_parameter_start_time = palette_start_time = time.time()

            # choose random starting values for each of the parameters
            for i in range(0, NUM_PARAMETERS):
                show_parameters[i] = constrained_random_parameter(i)
            constrain_show()
            choose_random_colors_from_edm_palette()
            # For Lee testing: uncomment this to use playa palette instead
            #choose_new_playa_palette()

        print "initial show parameters:"
        print "  base parameters", show_parameters[BASE_PARAM_START:BASE_PARAM_END+1]
        print "  mid parameters", show_parameters[MID_PARAM_START:MID_PARAM_END+1]
        print "  sparkle parameters", show_parameters[SPARKLE_PARAM_START:SPARKLE_PARAM_END+1]
        print "  7 color, palette change, beat", show_parameters[SEVEN_PAL_BEAT_PARAM_START:SEVEN_PAL_BEAT_PARAM_END+1]
        print "  alpha parameters", show_parameters[ALPHA_PARAM_START:ALPHA_PARAM_END+1]
        print "  transition parameters", show_parameters[TRANS_PARAM_START:TRANS_PARAM_END+1]
        print "initial show colors:"
        print "  ", show_colors
        return

    bg_time = bg_parameter_time = mid_time = mid_parameter_time = sparkle_time = sparkle_parameter_time = palette_time = time.time()

    # to avoid hard transitions, change disruptive base animation parameters only when you change background choice
    if bg_time - bg_start_time > BASE_TIME_LIMIT:
        bg_start_time = bg_time

        # change bg show parameters
        for i in range (BACKGROUND_INDEX, BACKGROUND_INDEX + 4):
            show_parameters[i] = constrained_random_parameter(i)
            print "background parameter ", i, "changed to ", show_parameters[i]

    if bg_parameter_time - bg_parameter_start_time > BASE_PARAMETER_TIME_LIMIT:
        bg_parameter_start_time = bg_parameter_time

        # choose which parameter to change
        change_bg = randint(BACKGROUND_INDEX + 4, MIDLAYER_INDEX - 1)
        show_parameters[change_bg] = constrained_random_parameter(change_bg)
        print "background parameter ", change_bg, "changed to ", show_parameters[change_bg]

    # to avoid hard transitions, change disruptive mid animation parameters only when you change mid layer choice
    if mid_time - mid_start_time > MID_TIME_LIMIT:
        mid_start_time = mid_time

        # change mid show parameters
        for i in range (MIDLAYER_INDEX, MIDLAYER_INDEX + 5):
            show_parameters[i] = constrained_random_parameter(i)
            print "mid parameter ", i, "changed to ", show_parameters[i]

    if mid_parameter_time - mid_parameter_start_time > MID_PARAMETER_TIME_LIMIT:
        mid_parameter_start_time = mid_parameter_time

        # choose which parameter to change
        change_mid = randint(MIDLAYER_INDEX + 5, SPARKLE_INDEX - 1)
        show_parameters[change_mid] = constrained_random_parameter(change_mid)
        print "mid parameter ", change_mid, "changed to ", show_parameters[change_mid]

    if sparkle_time - sparkle_start_time > SPARKLE_TIME_LIMIT:
        sparkle_start_time = sparkle_time

        # change sparkle animation
        show_parameters[SPARKLE_INDEX] = constrained_random_parameter(SPARKLE_INDEX)
        print "sparkle choice changed to ", show_parameters[SPARKLE_INDEX]

    # can change sparkle parameters independently of changing sparkle animation, without having hard transitions
    if sparkle_parameter_time - sparkle_parameter_start_time > SPARKLE_PARAMETER_TIME_LIMIT:
        sparkle_parameter_start_time = sparkle_parameter_time

        # choose which parameter to change
        change_sparkle = randint(SPARKLE_INDEX + 1,SPARKLE_INDEX + 10)
        show_parameters[change_sparkle] = constrained_random_parameter(change_sparkle)
        print "sparkle parameter ", change_sparkle, "changed to ", show_parameters[change_sparkle]

    constrain_show()	# keep the lights on

    if palette_time - palette_start_time > PALETTE_TIME_LIMIT:
        palette_start_time = palette_time

        show_parameters[29] = constrained_random_parameter(29)
        choose_random_colors_from_edm_palette()
        # For Lee testing: uncomment this to stick with day 1 colors
        #choose_new_playa_palette()
        print "palette changed ", show_colors



# fixme: ART_CAR mode will be set by Jeff's code
#    if art_car_detected for first ART_CAR_HELLO_DURATION seconds:
#        art_car_hello = true
#        art_car = ring number closest to art car, 0 to 72
#    else if art car sticks around past ART_CAR_HELLO_DURATION seconds:
#        art_car_hello = false
#    else if art car has departed
#        art_car = NO_ART_CAR
#


TEST_CYCLE_MINUTES = 3	# rush through the entire week in this number of minutes
# For Lee testing: uncomment this
#TEST_CYCLE_MINUTES = 15
NUM_DAYS = int((BURNING_MAN_END - BURNING_MAN_START) / 86400 + 0.5)

""" Unused right now. playa_mode is set in Music.tick()
# ------------------------ set_playa_mode() -------------------------------
# returns SUNRISE, DAY, SUNSET, NIGHT

def set_playa_mode(when, mode):
    this_time = datetime.fromtimestamp(when)
    global SUNRISE, DAY, SUNSET, NIGHT, NUM_DAYS, bm_day_index

    bm_day_index = int(when - BURNING_MAN_START) / 86400) % NUM_DAYS

    if (mode == NIGHT) and (when >= sunrise_time[bm_day_index]):
        mode = SUNRISE

    elif (mode == DAY) and (when >= sunset_time[bm_day_index]):
        mode = SUNSET

    # fixme: Kienen's code will set day and night modes at end of meditations
    #   i'll set these next 2 on timer for now, but this won't work for playa bc meditations are not same length

    #    if show_mode == SUNRISE:
    #        play sunrise meditation
    #    if show_mode == SUNSET:
    #        play sunset meditation
    #    if sunrise_meditation_just_ended:
    #        show_mode = DAY
    #    if sunset_meditation_just_ended:
    #        show_mode = NIGHT

    elif (mode == SUNRISE) and (when >= (sunrise_time[bm_day_index] + meditation_sec)):
        mode = DAY

    elif (mode == SUNSET) and (when >= (sunset_time[bm_day_index] + meditation_sec)):
        mode = NIGHT

    else:
        print 'not at burning man'
        #exit()

    return mode
"""


# ------------------------------- playa_program() ----------------------------------
#  This function initializes the playa program. If time is before burning man start, then it sets variables
#  so that a 3 minute test program runs

real_start_time = -1.0
def playa_program(init=False):
    global real_start_time, meditation_sec, time_compression_factor, show_mode
    IDEAL_MEDITATION_MINUTES = 20

    real_time = time.clock()
    if init:  # run test program
        if real_start_time < 0:
            time_compression_factor = float(NUM_DAYS * 60 * 24) / TEST_CYCLE_MINUTES	# 60*24 == minutes per day
            meditation_sec = int(IDEAL_MEDITATION_MINUTES * 60 * time_compression_factor / 233)	# 233 produces about 1/5 of the day with a 3 minute test cycle
            real_start_time = real_time
            edm_program(init)	# good enough for now
            show_parameters[29] = 999	# invalid
            show_mode = SUNRISE
        return

    if real_start_time == BURNING_MAN_START:
        virtual_time = real_time	# this is the live show at Burning Man
        show_mode = SUNRISE
    else:
        if BURNING_MAN_START <= real_time and real_time < BURNING_MAN_END:
            time_compression_factor = 1.0
            #meditation_sec = MEDITATION_MINUTES * 60
            print 'Welcome home!'	# Burning Man has just begun!
            real_start_time = BURNING_MAN_START
            virtual_time = real_time
        else:
            virtual_time = BURNING_MAN_START + (real_time - real_start_time) * time_compression_factor

    # Show mode is set in music.py
    #show_mode = set_playa_mode(virtual_time, show_mode)

    bm_day_index = int((virtual_time - BURNING_MAN_START) / 86400) % NUM_DAYS
    new_palette = bm_day_index % len(edm_palettes)
    if show_parameters[29] != new_palette:
        show_parameters[29] = new_palette
        choose_new_playa_palette()
        print 'palette changed', show_colors

    print 'playa time advanced to', time.ctime(virtual_time), 'on day', bm_day_index, 'in', show_mode



# ------------------------------ internal_sound_animations_program() -----------------------------------------------
# show for when the journey playing its internal audio and the external audio
# is not past threshold amount aka art car not detected

# still very much under development.... fine tuning audio_events processing and
# handling here to avoid too frequent or not frequent enough animation parameter changes
def do_internal_sound_animations(audio_msg, init = False):
    interpret_audio_msg(audio_msg)

    #pulls from event_queue
    drive_internal_animations(init)


def interpret_audio_msg(audio_msg):
    channel_map = get_audio_file_info(audio_msg)
    for channel in channel_map.keys():
        if current_internal_track_per_channel[channel] > 0: # channel already had a track on it
            old_audio = current_internal_track_per_channel[channel]
            remove_audio_events_from_queue(old_audio)

        audioInfo = channel_map[channel]
        current_internal_track_per_channel[channel] = audioInfo
        queue_audio_events(audioInfo)

        if audioInfo is not None:
            if INTERNAL_ANIMATIONS_DEBUG:
                print "setting main animation param due to new audio track's info " + str(audioInfo)
            set_appropriate_layer_main_animation(audioInfo)


# RJS I don't like how this hard coded... if the audio contorl message changes this needs to as well.
def get_audio_file_info(audio_msg):
    # current msg format: a0;1;0;0,50,0,0
    info = audio_msg.split(";")
    tracks = info[3].split(",")

    if INTERNAL_ANIMATIONS_DEBUG:
        print "tracks: " +  str(tracks)

    output = {}
    i = 0
    if tracks[i] is not "" and tracks[i] != '':
        while(i < len(tracks)):
            if int(tracks[i]) > 0:
                output[i] = (DataBaseInterface().grabAudioInfo(tracks[i]))
            i += 1

    return output


# set the appropriate layer's main animation based on audioInfo's predetermined suitiable animations
def set_appropriate_layer_main_animation(audioInfo):
    global bg_start_time, mid_start_time, sparkle_start_time, palette_start_time

    suitable_main_animation = audioInfo.getRandomSuitableAnimation()

    if suitable_main_animation is not None:
        if INTERNAL_ANIMATIONS_DEBUG:
            print "random suitable animation is " + str(suitable_main_animation)

        if audioInfo.category == "LOW" and time.time() - bg_start_time > BASE_MAIN_ANIMATION_SWITCH_LAG:
            show_parameters[BASE_PARAM_START] = suitable_main_animation
            bg_start_time = time.time()
        elif audioInfo.category == "MID" and time.time() - mid_start_time > MID_MAIN_ANIMATION_SWITCH_LAG:
            show_parameters[MID_PARAM_START] = suitable_main_animation
            mid_start_time = time.time()
        elif audioInfo.category == "HIGH" and time.time() - sparkle_start_time > SPARKLE_MAIN_ANIMATION_SWITCH_LAG:
            show_parameters[SPARKLE_PARAM_START] = suitable_main_animation
            sparkle_start_time = time.time()


def drive_internal_animations(init):
    global next_audio_event, event_queue
    global bg_start_time, bg_parameter_start_time, mid_start_time, mid_parameter_start_time, sparkle_start_time, sparkle_parameter_start_time, palette_start_time

    if init:
        if show_colors[0] == [33,33,33]:	# invalid values before initialization
            bg_start_time = bg_parameter_start_time = mid_start_time = mid_parameter_start_time = sparkle_start_time = sparkle_parameter_start_time = palette_start_time = time.time()

            # choose random starting values for each of the parameters
            for i in range(0, NUM_PARAMETERS):
                show_parameters[i] = constrained_random_parameter(i)
            constrain_show()
            choose_new_playa_palette()  # start with day 1 color palette

        print "initial show parameters:"
        print "  base parameters", show_parameters[BASE_PARAM_START:BASE_PARAM_END+1]
        print "  mid parameters", show_parameters[MID_PARAM_START:MID_PARAM_END+1]
        print "  sparkle parameters", show_parameters[SPARKLE_PARAM_START:SPARKLE_PARAM_END+1]
        print "  7 color, palette change, beat", show_parameters[SEVEN_PAL_BEAT_PARAM_START:SEVEN_PAL_BEAT_PARAM_END+1]
        print "  alpha parameters", show_parameters[ALPHA_PARAM_START:ALPHA_PARAM_END+1]
        print "  transition parameters", show_parameters[TRANS_PARAM_START:TRANS_PARAM_END+1]
        print "initial show colors:"
        print "  ", show_colors
        return

    progress_audio_queue()

    if next_audio_event is not None:
        # RJS need to come up with some sort of thresholding for time proximity
        # and animaiton change frequency. Will know better when I have the final
        # audio event lists populated (ideally using Antonio's 'cleaner' method)
        # MAYBE: seperate thread handling polling event_queue and sending animations
        lower_bounds = timeMs() - 1000
        upper_bounds = timeMs() + 1000
        valid_time = next_audio_event.time <= upper_bounds and next_audio_event.time >= lower_bounds

        if INTERNAL_ANIMATIONS_DEBUG:
            print "valid? " + str(valid_time) + " aiming for event time between " + str(lower_bounds) + " - " + str(upper_bounds) + " current event time " + str(next_audio_event.time)

        if valid_time and next_audio_event is not None: #valid 'next' event
            magnitude = next_audio_event.magnitude

            #selecting any base, mind, sparkler layer param - very rudimentary.
            # show_param = randint(0, 27)

            # Selecting a random animaiton paramter to change (exluding the main animations)
            # if the timing threshold is met to avoid too frequent param changes... TODO see how it looks with varoius 'lags'
            show_param = -1;
            if next_audio_event.category == "LOW":
                if time.time() - bg_parameter_start_time > BASE_PARAMETER_SWTICH_LAG:
                    show_param = random.choice(range(BASE_PARAM_START + 1, BASE_PARAM_END))
                    bg_parameter_start_time = time.time()
            elif next_audio_event.category == "MID":
                if time.time() - mid_parameter_start_time > MID_PARAMETER_SWTICH_LAG:
                    show_param = random.choice(range(MID_PARAM_START + 1, MID_PARAM_END))
                    mid_parameter_start_time = time.time()
            elif next_audio_event.category == "HIGH":
                if time.time() - sparkle_parameter_start_time > SPARKLE_PARAM_START:
                    show_param = random.choice(range(SPARKLE_PARAM_START + 1, SPARKLE_PARAM_END))
                    sparkle_parameter_start_time = time.time()

            if show_param > -1:
                old_param_value = show_parameters[show_param]

                if next_audio_event.kind == "freqband":
                    new_param_value = constrained_weighted_parameter(show_param, magnitude)
                    print "Frq event [" + str(next_audio_event) +"]: Set show_param[" + str(show_param) + "] from " + str(old_param_value) + " to " + str(new_param_value)

                elif next_audio_event.kind == "amplitude":
                    new_param_value = constrained_weighted_parameter(show_param, magnitude)
                    print "Amp event: [" + str(next_audio_event) +"]: Set show_param[" + str(show_param) + "] from " + str(old_param_value) + " to " + str(new_param_value)

                # setting the param to its new value
                #show_parameters[show_param] = new_param_value
                do_set_show_parameter(None, str(show_param) + " " + str(new_param_value))

                try:
                    if event_queue.size > 0:
                        event_queue.remove(next_audio_event.time)
                    else:
                        next_audio_event = None
                except AttributeError:
                    print "event_queue is empty ", sys.exc_value

        if time.time() - palette_start_time > PALETTE_LAG:
            bm_day_index = int((time.time() - BURNING_MAN_START) / 86400) % NUM_DAYS
            #get_random_color(bm_day_index, 1) #TODO not sure what this second value should be...
            # OR should I be using
            choose_new_playa_palette()
            palette_start_time = time.time()

        constrain_show()


def progress_audio_queue():
    global event_queue, next_audio_event
    # ensure not looking at events that have already passed
    while True:
        try:
            next_audio_event_node = event_queue.peek()
            next_audio_event = next_audio_event_node.value
            if INTERNAL_ANIMATIONS_DEBUG:
                print "next audio event " + str(next_audio_event)
        except ValueError:
            if INTERNAL_ANIMATIONS_DEBUG:
                print "event_queue is empty", sys.exc_value
            break

        stale = next_audio_event.time <= timeMs() - 1000
        # print "diff event - now = " + str(next_audio_event.time - timeMs())
        if stale:
            print "it's " + str(timeMs()) + " stale event " + str(next_audio_event) + " popping!"
            event_queue.remove(next_audio_event.time)
        else:
            break


#TODO make this more intelligent
def constrained_weighted_parameter(i, magnitude):
    if show_bounds[i][0] == -1 and show_bounds[i][1] == 1:
        new_parameter = show_bounds[i][randint(0,1)]	# no zero value
    else:
        old_parameter = show_parameters[i]
        up_down = randint(0, 1)
        if up_down == 1:
            new_parameter = old_parameter + quantify_magnitude_impact(magnitude)
        else:
            new_parameter = old_parameter - quantify_magnitude_impact(magnitude)

    # change to unsigned int for passing to due
    if new_parameter < 0:
        new_parameter += 256
    return new_parameter


def remove_audio_events_from_queue(audioInfo):
    for event in audioInfo.events:
        try:
            event_queue.remove(event.time)
        except ValueError:
            print "event " + str(event) + " already has been removed from queue"


def queue_audio_events(audioInfo):
    cur_time_ms = timeMs()
    if audioInfo is not None:
        for event in audioInfo.events:
            event.time = long(event.time) + cur_time_ms
            node = event_queue.add(event)
    else:
        print "seems like it was a database miss... this will happen while we don't have all the auido files"


# quantifying the standard deviation aka magnitude of event into show param quantities
def quantify_magnitude_impact(magnitude):
    magnitude = float(magnitude)

    if magnitude > 5.5:
        return randint(26,30)
    elif magnitude > 5.0:
        return randint(21,25)
    elif magnitude > 4.5:
        return randint(16,20)
    elif magnitude > 4.0:
        return randint(11,15)
    elif magnitude > 3.5:
        return randint(6,10)
    else:
        return randint(1,5)


def timeMs():
    return long(round(time.time() * 1000))


def do_set_show_parameter(ignored, parameters):
    try:
        param, value = string.split(parameters, None, 1)
        param = int(param)
        value = int(value)
    except (AttributeError, ValueError):		# no whitespace or non-integer
        print 'Usage: sp parameter_number value'
        return
    if param < 0 or NUM_PARAMETERS <= param:
        print 'parameter number must be from 0 to', NUM_PARAMETERS-1
        return
    if value < show_bounds[param][0] or show_bounds[param][1] < value:
        print 'parameter', param, 'value must be from', show_bounds[param][0], 'to', show_bounds[param][1]
        return
    print 'show_parameters[%u] set to %d' % (param, value)
    if value < 0:
        value += 256
    show_parameters[param] = value
