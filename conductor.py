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
TIME_LIMIT = 10  # number of seconds that can pass before a parameter is changed

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

NUM_ANIMATIONS = 4
NUM_BEAT_EFFECTS = 1
NUM_PARAMETERS = 9
NUM_COLORS_PER_PALETTE = 7

ANIMATION_INDEX = 0  # which animation to play
BEAT_EFFECT_INDEX = 1  # how to respond to beat
PALETTE_INDEX = 2  # which color palette to use
NUM_COLORS_INDEX = 3  # how many colors to use out of this palette
COLOR_THICKNESS_INDEX = 4  # how many consecutive lit LEDs in a row
BLACK_THICKNESS_INDEX = 5  # how many dark LEDs between lit ones
INTRA_RING_MOTION_INDEX = 6  # 0 = none, 1 = CW, 2 = CCW, 3 = split
INTRA_RING_SPEED_INDEX = 7  # fixme: still need to decide on units
COLOR_CHANGE_STYLE_INDEX = 8  # 0 = none, 1 = cycle thru selected, 2 = cycle thru palette

lower = [0] * NUM_PARAMETERS
upper = [0] * NUM_PARAMETERS
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


# ----------------- def of color class --------------------------------------------------------
class Color:
        def __init__(self, r, g, b):  # RGB color values 0..255
            self.red = r
            self.blue = b
            self.green = g

        # attempt to resolve undesired color shift that occurs when intensity increases
        # to the point that one base color maxes out before the others
        # here, native intensity of the color is normalized to 1, and the value returned is
        # the highest multiple of rgb that won't have that effect

        def max_intensity_multiplier(self):

            brightest = max(self.red, self.green, self.blue)
            max_multiplier = 255.0 / brightest

            return max_multiplier


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
def edm_program():

    print "sanctioned colors", SANCTIONED_COLORS


    start_time = time.time()

    # randomly initialize parameters to animations
    # later these will be based more closely on other inputs
    # Jeff fixme: show_parameters[] and show_colors[] are the two parameters to be passed to Due

    max_palette = 3  # this will vary by time and date at burning man

    show_parameters = [0] * NUM_PARAMETERS
    show_colors = [0] * NUM_COLORS_PER_PALETTE

    set_parameter_bounds()

    # choose random starting values for each of the parameters
    for i in range(0, NUM_PARAMETERS - 1):
        show_parameters[i] = randint(lower[i], upper[i])

    # choose which colors out of the chosen palette to use
    # show_parameters[NUM_COLORS_INDEX] returns how many colors to use in the current animation
    show_colors = sample(range(1, 7), show_parameters[NUM_COLORS_INDEX])

    # fixme: Jeff: here's where parameters -> due is called
    send_due_parameters()

    temporary_counter = 0
    current_time = time.time()
    while True:  # run forever until show is taken down

        print "current params", show_parameters
        print "current colors", show_colors

        if (current_time - start_time) > TIME_LIMIT:
            start_time = current_time

            # randomly choose a parameter to change
            change_param = randint(0, NUM_PARAMETERS - 1)
            print "changing parameter", change_param

            # now randomly change that parameter
            new = randint(lower[change_param], upper[change_param])
            show_parameters[change_param] = new
            print "new show parameter", show_parameters[change_param]

            if change_param == NUM_COLORS_INDEX:
                # choose which colors out of the chosen palette to use
                show_colors = sample(range(0, 5), show_parameters[NUM_COLORS_INDEX])

        # fixme: Jeff: here's where parameters -> due is called
        send_due_parameters()

        time.sleep(3)
        current_time = time.time()
        temporary_counter += 1


#  fixme: Jeff, here is a place for the code to send the parameters to the due
#  parameters are in 2 arrays; show_parameters[NUM_PARAMETERS] and SHOW_COLORS[NUM_COLORS_PER_PALETTE]
def send_due_parameters():



# ------------------------------------------------- scale() -----------------------------------------------

# this function will scale RGB values up and down to change brightness,
# but will stop scaling once one value hits 255
# this will change self's rgb values. please don't pass it the original desired color array!
# mult = 1 corresponds to no scaling at all
# usage:    for i in range(1, 10):
#               mult = i / 10.0
#               scaled_color = scale(led_color, mult)

def scale(color, mult):

    max_ = color.max_intensity_multiplier()

    if mult < max_:  # this scaling won't distort color
        scale_factor = mult
    else:  # then this scaling would push one of the color values past 255; just scale as far as you can
        scale_factor = max_

    r = int(round(color.red * scale_factor))
    g = int(round(color.green * scale_factor))
    b = int(round(color.blue * scale_factor))

    new_color = Color(r, g, b)

    return new_color


#------------------------------- set parameter bounds -------------------------------
# sets upper and lower bounds for each of the animation parameters
# only needs to be set once
# fixme: I don't seem to understand passing parameters in python well enough to change these 2 lists
# fixme: global vars is the only way I know how to make it work.

def set_parameter_bounds():

    max_palette = 3

    # which animation to play
    lower[ANIMATION_INDEX] = 0
    upper[ANIMATION_INDEX] = NUM_ANIMATIONS - 1

    # which color palette to use
    lower[PALETTE_INDEX] = 0
    upper[PALETTE_INDEX] = max_palette - 1  # this will vary by time and date at burning man

    # how many colors to use in the animation
    lower[NUM_COLORS_INDEX] = 1
    upper[NUM_COLORS_INDEX] = NUM_COLORS_PER_PALETTE + 1  # the last color indicates rainbow around structure

    # which beat effect to use / how to respond to beat with LEDs
    lower[BEAT_EFFECT_INDEX] = 0
    upper[BEAT_EFFECT_INDEX] = NUM_BEAT_EFFECTS - 1

    # how many pixels should the bands of color be
    lower[COLOR_THICKNESS_INDEX] = 1
    upper[COLOR_THICKNESS_INDEX] = 10

    # how many black LEDs between color bands
    lower[BLACK_THICKNESS_INDEX] = 0
    upper[BLACK_THICKNESS_INDEX] = 5

    # which direction to move lights inside a ring:  0 = none, 1 = CW, 2 = CCW, 3 = split
    lower[INTRA_RING_MOTION_INDEX] = 0
    upper[INTRA_RING_MOTION_INDEX] = 3

    # how fast should intra_ring motion be fixme: still need to decide on units
    lower[INTRA_RING_SPEED_INDEX] = 1
    upper[INTRA_RING_SPEED_INDEX] = 10

    # how should color change during an animation: 0 = none, 1 = cycle thru selected, 2 = cycle thru palette
    lower[COLOR_CHANGE_STYLE_INDEX] = 0
    upper[COLOR_CHANGE_STYLE_INDEX] = 2


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
