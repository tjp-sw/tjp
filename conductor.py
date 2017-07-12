from array import *
import matplotlib.pyplot as plt
from random import randint
from random import random
from random import sample
import math

import time
import numpy as np
from matplotlib import pyplot as plt
from matplotlib import animation
time.time()
import threading
# import not_yet_used

# ----------------------------------------------------------------------------------------
# Testing / Debug Constants
#
# if testing is true, program will run independent of hardcoded real time of installation,
# and times for things to function will be dramatically reduced
# ----------------------------------------------------------------------------------------

EDM = True
TESTING_FULL_PROGRAM = False
MINUTES_OF_TESTING = 10
TEST_THIS_DAY = 6  # which day's single program you want to test (sunrise, daytime, etc)
TESTING_SUNRISE = False
TESTING_DAYTIME = False
TESTING_SUNSET = False
TESTING_NIGHTTIME = False
# should also adjust NUM_DAYS below for testing
# should also adjust CYCLE_PAUSE for the real show


# ----------------------------------------------------------------------------------------
# Installation Dependent Constants
# 
# Might change for a different installation of the journey
# Creates lot of global variables which I know isn't safe, but I chose this option so that everything
# that would need to change for a different installation is in one place.
#
# Will also need to change hardcoded sunrise and sunset times, defined in main()
# ----------------------------------------------------------------------------------------

# temporal constants
COLORS_PER_FAMILY = 9
NUM_DAYS = 3  # will eventually be 7
NUM_COLOR_FAMILIES = 7  # is this used? should usually be same as num_days, but just in case....
SUNRISE_MEDITATION_MINUTES = 20  # minutes
SUNSET_MEDITATION_MINUTES = 20  # minutes
CYCLE_PAUSE = MINUTES_OF_TESTING / 10  # seconds to pause before re-running conductor loop  -- maybe change to .1
MID_TIME_LIMIT = 60
BASE_TIME_LIMIT = 97
SPARKLE_TIME_LIMIT = 43
SPARMKE_PARAMETER_TIME_LIMIT = 29
PALETTE_TIME_LIMIT = 10  # number of seconds that can pass before a parameter is changed

# structure constants
NUM_NODES = 6
RINGS_PER_NODE = 12  # in pairs
RINGS_PER_STRIP = 3  # LED strips purchased each wrap around one ring; these are connected in triples
LIGHTS_PER_RING = 408  # numbered clockwise from inside (7:00) to outside (5:00) fixme check this
TOTAL_RINGS = NUM_NODES * RINGS_PER_NODE
STRIPS_PER_NODE = RINGS_PER_NODE / RINGS_PER_STRIP
LIGHTS_PER_STRIP = LIGHTS_PER_RING * RINGS_PER_STRIP


# Non-Color Animation Parameter Constants
#
# All (most of) the parameters for a show are stored in an array called show_parameters, so that they
# can be passed around in one go. These constants are the indices into this show_parameters
# array holding that type of parameter value

NUM_7_COLOR_ANIMATIONS = 2
NUM_BASE_ANIMATIONS = 2
NUM_MID_ANIMATIONS = 3
NUM_SPARKLE_ANIMATIONS = 3

NUM_BEAT_EFFECTS = 1
NUM_PARAMETERS = 30
NUM_COLORS_PER_PALETTE = 7
NUM_COLOR_PALETTES = 3

BASE_TIME_LIMIT = 87
MID_TIME_LIMIT = 61
SPARKLE_TIME_LIMIT = 43
SPARKLE_PARAMETER_TIME_LIMIT =  17
PALETTE_TIME_LIMIT = 7

#ANIMATION_INDEX = 0  # which animation to play
#BEAT_EFFECT_INDEX = 1  # how to respond to beat
#PALETTE_INDEX = 2  # which color palette to use
#NUM_COLORS_INDEX = 3  # how many colors to use out of this palette
#COLOR_THICKNESS_INDEX = 4  # how many consecutive lit LEDs in a row
#BLACK_THICKNESS_INDEX = 5  # how many dark LEDs between lit ones
#INTRA_RING_MOTION_INDEX = 6  # 0 = none, 1 = CW, 2 = CCW, 3 = split
#INTRA_RING_SPEED_INDEX = 7  # fixme: still need to decide on units
#COLOR_CHANGE_STYLE_INDEX = 8  # 0 = none, 1 = cycle thru selected, 2 = cycle thru palette
#RING_OFFSET_INDEX = 9  # # LEDs a ring's pattern is rotated from it's neighbors' pattern, -10 to 10

# show_bounds = [[0 for i in range(0, 2)] for j in range(0,NUM_PARAMETERS)]
# lower = [0] * NUM_PARAMETERS
# upper = [0] * NUM_PARAMETERS
show_bounds = [  # order must match show_parameters
        # [min, max]
        # show bounds 0 through 7 concern base animations
        [0, NUM_BASE_ANIMATIONS - 1],  # BACKGROUND_INDEX, which background animation to use
        [0, 255], # base color thickness
        [0, 255], # base black thickness
        [-1, 3], # base intra ring motion: -1 CCW, 0 none, 1 CW, 2 alternate, 3 split (down from top)
        [0, 255], # base intra ring speed
        [-1,1], # base inter ring motion: -1 = CCW, 0 = none, 1 = CW
        [0,255], # base inter ring speed
        [-10,10], # base ring offset
        # show bounds 8 through 16 concern mid layer animations
        [0, NUM_MID_ANIMATIONS - 1],  # MIDLAYER_INDEX, which mid layer animation to use
        [1, 3], # mid num colors
        [0, 255],  # mid color thickness
        [0, 255],  # mid black thickness
        [-1, 3],  # mid intra ring motion: -1 CCW, 0 none, 1 CW, 2 alternate, 3 split (down from top)
        [0, 255],  # mid intra ring speed
        [-1, 1],  # mid inter ring motion: -1 = CCW, 0 = none, 1 = CW
        [0, 255],  # mid inter ring speed
        [-10, 10],  # mid ring offset
        # show bounds 17 through 27 concern sparkle animations
        [0, NUM_SPARKLE_ANIMATIONS - 1],  # SPARKLE_INDEX, which sparkle animation to use
        [2, 200],  # sparkle portion
        [0, 255],  # sparkle color thickness
        [0, 255],  # sparkle black thickness
        [-1, 3],  # sparkle intra ring motion: -1 CCW, 0 none, 1 CW, 2 alternate, 3 split (down from top)
        [0, 255],  # sparkle intra ring speed
        [-1, 1],  # sparkle inter ring motion: -1 = CCW, 0 = none, 1 = CW
        [0, 255],  # sparkle inter ring speed
        [0, 6], # sparkle max dim
        [0, 255], # sparkle range
        [1, 50], # sparkle spawn frequency
        [0, NUM_7_COLOR_ANIMATIONS - 1],  # which 7 color animation to play, show bound 28
        [0, NUM_COLOR_PALETTES - 1], # which color palette, show bound 29
        [0, NUM_BEAT_EFFECTS - 1],  # which beat effect to use to respond to beat with LEDs, show bound 30
        [0,1], # is_beat boolean, show bound 31
        [0,100] # beat proximity - how close you are to beat. so when beat prox >= 95 or <= 5, can smooth beat response
    ]

# Pre-defined color palettes
fruit_loop = [[25,0,25], [25,15,0], [180,10,70], [140,60,180], [180,60,60], [255,255,120], [255,100,180]]
icy_bright = [[37,28,60], [70,0,28], [255,108,189], [0,172,238], [44,133,215], [255,255,255], [254,207,24]]
watermelon = [[40,29,35], [5,45,15], [47,140,9], [72,160,5], [148,33,137], [47,192,91], [70,190,91]]
palette = [fruit_loop, icy_bright, watermelon]


#------------------------------- set parameter bounds -------------------------------
# sets upper and lower bounds for each of the animation parameters
# only needs to be set once
# fixme: I don't seem to understand passing parameters in python well enough to change these 2 lists
# fixme: global vars is the only way I know how to make it work.

def set_parameter_bounds():
    print "set parameterr bounds"



# ----------------------------------------------------------------------------------------
# Functions in here
#
# get_node(ring_index)
# get_ring_in_node(ring_index)
# get_strip_in_node(ring_index)
# get_random_color(day, desired_period)  [period is number of 5 minute segments past sunset med]
# plot_journey()  [would be cool if paper or screen were 10 times bigger]
# scale(color, mult)  [where mult = 1 means no scaling]
# ring_pixel_2_node_strip_pixel(ring_index, led_index)  [which ring and which led on that ring
# sunrise_meditation()
# day_program()
# sunset_meditation()
# night_program()
# ----------------------------------------------------------------------------------------



# ##########################################################################################################
# ###########################################   Main Program   #############################################
# ##########################################################################################################

def main():

    # ---------- initializing and checking or testing vs real thing ----------

    today = TEST_THIS_DAY  # will be set to correct current day below if not testing

    if EDM:
        edm_program()

    elif TESTING_SUNRISE:
        sunrise_meditation(today)

    elif TESTING_DAYTIME:
        day_program(today)  # stick with day's chakra color

    elif TESTING_SUNSET:
        sunset_meditation(today)

    elif TESTING_NIGHTTIME:
        night_program(today)

    elif TESTING_FULL_PROGRAM:  # test NUM_DAYS full days' cycle, compressed to MINUTES_OF_TESTING minutes
        testing_seconds_per_day = MINUTES_OF_TESTING * 60 / NUM_DAYS
        sunrise_to_sunset = MINUTES_OF_TESTING * 60 * .3 / NUM_DAYS  # 30% in daylight; rest at night

        sunrise_meditation_duration = testing_seconds_per_day * .1  # 10% of the day in this meditation
        sunset_meditation_duration = testing_seconds_per_day * .1  # 10% of the day in this meditation

        # create sunrise and sunset times for shorter days, starting now
        # both are indexed starting at 1, not 0
        sunrise_time = [0] * (NUM_DAYS + 2)
        sunset_time = [0] * (NUM_DAYS + 2)

        current_time = time.time()
        beginning_of_time = current_time

        for i in range(1, NUM_DAYS + 2):  # NUM_DAYS + 2 because indexing from 1 & loop doesn't execute at last in range
            sunrise_time[i] = beginning_of_time + (i - 1) * testing_seconds_per_day
            sunset_time[i] = sunrise_time[i] + sunrise_to_sunset

        conductor(sunrise_time, sunset_time, sunrise_meditation_duration, sunset_meditation_duration)

    # ---------------- this code is for the real thing -----------------------

    else: 
        sunrise_meditation_duration = SUNRISE_MEDITATION_MINUTES * 60  # 20 minutes, expressed in seconds
        sunset_meditation_duration = SUNSET_MEDITATION_MINUTES * 60  # 20 minutes, expressed in seconds

        # epoch times for sunrise and sunset from sunday 8/27/17 through following monday
        # sunday is day 0, and sunday sunrise is the beginning of time
        sunrise_time = [0, 1503839940, 1503926400, 1504012860, 1504099320, 1504185780,
                                   1504272240, 1504358700, 1504445160, 1504531620]
        sunset_time = [0, 1503887940, 1503974220, 1504060500, 1504146840, 1504233120,
                                  1504319460, 1504405740, 1504492020, 1504578360]

        conductor(sunrise_time, sunset_time, sunrise_meditation_duration, sunset_meditation_duration)


# ----------------------------------------------- conductor() ----------------------------------------------

def conductor(sunrise_time, sunset_time, sunrise_meditation_duration, sunset_meditation_duration):

    seconds_per_day = sunrise_time[2] - sunrise_time[1]  # recall indexing starts at 1
    end_of_time = sunrise_time[NUM_DAYS + 1]
    beginning_of_time = sunrise_time[1]
    # sunrise and sunset are correct here

    # fixme: start led thread and sound thread


    current_time = time.time()
    today = int((current_time - beginning_of_time) / seconds_per_day) + 1

    while today <= NUM_DAYS:

        # determine which of the day's 4 time intervals we are currently in
        if sunrise_time[today] <= current_time < (sunrise_time[today] + sunrise_meditation_duration):
            sunrise_meditation(today)

        elif (sunrise_time[today] + sunrise_meditation_duration) <= current_time < sunset_time[today]:
            day_program(today)  # stick with day's chakra color

        elif sunset_time[today] <= current_time < (sunset_time[today] + sunset_meditation_duration):
            sunset_meditation(today)

        elif (sunset_time[today] + sunset_meditation_duration) <= current_time < sunrise_time[today + 1]:
            night_program(today)

        else:
            print 'not at burning man'
            exit()

        time.sleep(CYCLE_PAUSE)

        current_time = time.time()
        # again, indexing starting at 1
        if current_time >= sunrise_time[today + 1]:
            today += 1


# ------------------------------------------------- edm_program() -----------------------------------------------
# show for when the journey is installed at an event with electronic dance music only
# parameters are somewhat randomly chosen; this will change at bm when params will be determined by sound
def edm_program():

    bg_start_time = time.time()
    mid_start_time = time.time()
    sparkle_start_time = time.time()
    sparkle_parameter_start_time = time.time()
    palette_start_time = time.time()

    show_parameters = [0] * NUM_PARAMETERS
    show_colors = [[0 for rgb in range(0, 2)] for i in range(0, NUM_COLORS_PER_PALETTE)]
    bg_colors = [[0 for i in range(0, 2)] for rgb in range(0, 2)]
    mid_colors = [[0 for i in range(0, 3)] for rgb in range(0, 2)]
    mid_colors = [[0 for i in range(0, 2)] for rgb in range(0, 2)]

    set_parameter_bounds()

    # choose random starting values for each of the parameters
    for i in range(0, NUM_PARAMETERS - 1):
        new_parameter = randint(show_bounds[i][0], show_bounds[i][1])
        # change to unsigned int for passing to due
        if new_parameter < 0:
            show_parameters[i] = 256 + new_parameter
        else:
            show_parameters[i] = new_parameter

    print "initial show parameters ", show_parameters

    # choose which colors out of the chosen palette to use
    #shuffle the lower 2 colors, mid 3 colors, and upper 2 colors of chosen palette
    bg_order = sample(range(0,2), 2)
    mid_order = sample(range(2,5), 3)
    sp_order = sample(range(5,7), 2)

    # palette[show_parameters[29]] is currently chosen palette
    show_colors[0] = palette[show_parameters[29]][bg_order[0]]
    show_colors[1] = palette[show_parameters[29]][bg_order[1]]
    show_colors[2] = palette[show_parameters[29]][mid_order[0]]
    show_colors[3] = palette[show_parameters[29]][mid_order[1]]
    show_colors[4] = palette[show_parameters[29]][mid_order[2]]
    show_colors[5] = palette[show_parameters[29]][sp_order[0]]
    show_colors[6] = palette[show_parameters[29]][sp_order[1]]
    print "initial show colors" , show_colors

    # fixme: Jeff: here's where parameters -> due is called
    send_due_parameters()

    while True:  # run forever until show is taken down

        current_time = time.time()
        bg_time = time.time()
        mid_time = time.time()
        sparkle_time = time.time()
        sparkle_parameter_time = time.time()
        palette_time = time.time()

        # to avoid hard transitions, change all base animation parameters only when you change background choice
        if bg_time - bg_start_time > BASE_TIME_LIMIT:
            bg_start_time = bg_time

            # change bg show parameters
            for i in range (0, 7):
                new_parameter = randint(show_bounds[i][0], show_bounds[i][1])

                # convert to unsigned int for passing to due
                if new_parameter < 0:
                    show_parameters[i] = 256 + new_parameter
                else:
                    show_parameters[i] = new_parameter
                print "background parameter ", i, "changed to ", show_parameters[i]

        # to avoid hard transitions, change all mid animation parameters only when you change mid layer choice
        if mid_time - mid_start_time > MID_TIME_LIMIT:
            mid_start_time = mid_time

            # change mid show parameters
            for i in range (8, 17):
                new_parameter = randint(show_bounds[i][0], show_bounds[i][1])

                # convert to unsigned int for passing to due
                if new_parameter < 0:
                    show_parameters[i] = 256 + new_parameter
                else:
                    show_parameters[i] = new_parameter
                print "mid parameter ", i, "changed to ", show_parameters[i]

        if sparkle_time - sparkle_start_time > SPARKLE_TIME_LIMIT:
            sparkle_start_time = sparkle_time

            # change sparkle animation
            show_parameters[17] = randint(show_bounds[17][0], show_bounds[17][1])
            print "sparkle choice changed to ", show_parameters[17]

        # can change sparkle parameters independently of changing sparkle animation, without having hard transitions
        if sparkle_parameter_time - sparkle_parameter_start_time > SPARKLE_PARAMETER_TIME_LIMIT:
            sparkle_parameter_start_time = sparkle_parameter_time

            # choose which parameter to change; sparkle params are 18-27
            change_sparkle = randint(18,28)
            new_parameter = randint(show_bounds[change_sparkle][0], show_bounds[change_sparkle][1])

            # change to unsigned int for passing to due
            if new_parameter < 0:
                show_parameters[change_sparkle] = 256 + new_parameter
            else:
                show_parameters[change_sparkle] = new_parameter
            print "sparkle parameter ", change_sparkle, "changed to ", show_parameters[change_sparkle]

        if palette_time - palette_start_time > PALETTE_TIME_LIMIT:
            palette_start_time = palette_time

            show_parameters[29] = randint(show_bounds[29][0], show_bounds[29][1])
            # choose which colors out of the chosen palette to use
            # shuffle the lower 2 colors, mid 3 colors, and upper 2 colors of chosen palette
            bg_order = sample(range(0, 2), 2)
            mid_order = sample(range(2, 5), 3)
            sp_order = sample(range(5, 7), 2)

            # palette[show_parameters[29]] is currently chosen palette
            show_colors[0] = palette[show_parameters[29]][bg_order[0]]
            show_colors[1] = palette[show_parameters[29]][bg_order[1]]
            show_colors[2] = palette[show_parameters[29]][mid_order[0]]
            show_colors[3] = palette[show_parameters[29]][mid_order[1]]
            show_colors[4] = palette[show_parameters[29]][mid_order[2]]
            show_colors[5] = palette[show_parameters[29]][sp_order[0]]
            show_colors[6] = palette[show_parameters[29]][sp_order[1]]

            print "palette changed ", show_colors

        # fixme: Jeff: here's where parameters -> due is called
        send_due_parameters()

        #time.sleep(3)
        current_time = time.time()


#  fixme: Jeff, here is a place for the code to send the parameters to the due
#  parameters are in 2 arrays; show_parameters[NUM_PARAMETERS] and SHOW_COLORS[NUM_COLORS_PER_PALETTE]
def send_due_parameters():
  return(1);



def sunrise_meditation(today):
    print("sunrise meditation")


def day_program(today):
    print("daytime")


def sunset_meditation(today):
    print("sunset meditation")



# ----------------------------------------------- night_program() ----------------------------------------------
# fixme: doesn't work at all yet. just working on this

def night_program(today):
    print('night time')

    # start evolving color palate backwards
    time_period = 0
    t_start = time.time()
    night_time = True

#    while night_time:

        # after sunrise meditation ends, continually check whether it's time to update color probabilities
        # will create new cumulative_array for each time period, will not change after final steady state reached

#        length_time_period = 60 * 15  # minutes
        # if TESTING_FULL_PROGRAM:
        #    length_time_period = 3  # to have changes in color percentages happen more quickly

 #       t_now = time.time()
  #      if (t_now - t_start) < length_time_period:
   #         t_start = t_now
    #        time_period += 1  # color ratios evolve after each time_period has elapsed

        # get weighted-random index into master COLOR_ARRAY
        # random_index = get_random_color(today, time_period)

        # night_time = (t_now < sunrise_time[today + 1])




# -------------------------------------- Call main() ----------------------------------------------------

main()
