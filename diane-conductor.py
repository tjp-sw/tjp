from array import *
import matplotlib.pyplot as plt
from random import randint
import math

import time
import numpy as np
from matplotlib import pyplot as plt
from matplotlib import animation

time.time()

# ----------------------------------------------------------------------------------------
# Testing / Debug Constants
#
# if testing is true, program will run independent of hardcoded real time of installation,
# and times for things to function will be dramatically reduced
# ----------------------------------------------------------------------------------------

TESTING_FULL_PROGRAM = True
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
COLORS_PER_DAY = 20
NUM_DAYS = 3
NUM_COLOR_FAMILIES = 7  # is this used? should usually be same as num_days, but just in case....
SUNRISE_MEDITATION_MINUTES = 20  # minutes
SUNSET_MEDITATION_MINUTES = 20  # minutes
CYCLE_PAUSE = MINUTES_OF_TESTING / 10  # seconds -- how long to pause before running conductor loop again -- might need to change to .1

# structure constants
NUM_NODES = 6
RINGS_PER_NODE = 12  # in pairs
RINGS_PER_STRIP = 3  # LED strips purchased each wrap around one ring; these are connected in triples
LIGHTS_PER_RING = 408  # numbered clockwise from inside (7:00) to outside (5:00) fixme check this
TOTAL_RINGS = NUM_NODES * RINGS_PER_NODE
STRIPS_PER_NODE = RINGS_PER_NODE / RINGS_PER_STRIP
LIGHTS_PER_STRIP = LIGHTS_PER_RING * RINGS_PER_STRIP

# pre-chosen rgb values of pleasing colors from each chakra in linear order: r, o, y, g, b, p, w
# will probably pull this out of the database
COLOR_ARRAY = [
    [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3],
    [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3],
    [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3],
    [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3],
    [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3],
    [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3],
    [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3],
    [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3],
    [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3],
    [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3],
    [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3],
    [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3],
    [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3], [1, 2, 3]
]


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

    # current_time = time.time()

    # plot_journey()

    today = TEST_THIS_DAY  # will be set to correct current day below if not testing

    if TESTING_SUNRISE:
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

    current_time = time.time()
    today = int((current_time - beginning_of_time) / seconds_per_day) + 1

    while (today <= NUM_DAYS):

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


# *************** attempt at writing cellular automata *******************

def cellular():
    new_color = [Color(12, 120, 200), Color(120, 200, 12), Color(200, 12, 120)]

    # create LED array with random entries
    led_array = [[0 for row in range(0, 72)] for col in range(0, 408)]

    for row in range(0, 71):
        led_array[12][row] = 1

    for time_step in range(1, 1000):
        for ring in range(0, 71):
            for led in range(0, 407):
                center_color = led_array[led][ring]
                if led_array[led][(ring + 1) % 72] == (center_color + 1) % 3 or \
                                led_array[led][(ring - 1) % 72] == (center_color + 1) % 3 or \
                                led_array[(led + 1) % 408][ring] == (center_color + 1) % 3 or \
                                led_array[(led - 1) % 408][ring] == (center_color + 1) % 3:
                    led_array[led][ring] = (led_array[led][ring] + 1) % 3
                    #        time.sleep(1)
        print time_step
        plot_journey(led_array)


# ------------------------------------------------- get_node() -----------------------------------------------
# given an integer (0 to 35) representing a choice of ring, return which node is in charge of that ring
def get_node(ring_index):
    return (ring_index / RINGS_PER_NODE) % 6


# ---------------------------------------------- get_ring_in_node() -----------------------------------------------
#  given an integer (0 to 35) representing a choice of ring
# return which ring it is relative to its node (0 to 5)

def get_ring_in_node(ring_index):
    return ring_index % RINGS_PER_NODE


# --------------------------------------------- get_strip_in_node() -----------------------------------------------
# given an integer (0 to 35) representing a choice of ring,
# return which strip it is relative to its node (0 to 5)
# fixme I don't think this is right. plus, maybe 6 should be number of nodes instead

def get_strip_in_node(ring_index, strips_per_node):
    return (ring_index / 6) % strips_per_node


# ------------------------------------------------- get random color ------------------------------------------------
# returns one sanctioned color as an *index* into fixed color array, weightedly-randomly chosen
# progresses through different time periods starting at end of sunset meditation; at each
# time period more of previous days' colors are allowed, with proportions rising first for
# previous day's color, then for day before that's color, etc
#
# at the end of the periods, today's chakra color is twice as likely to be chosen from as any of the
# previous days' colors, and choices remain this way until following sunrise meditation
#
# note that this code does not actually use percentages, but a multiple of percentages
# where the multiple is num_colors_today * percent_scaling_factor

# fixme: does this depend on number of days being run?
# fixme: should i have percent_* constants moved to top, for easy changing?
# fixme: they might have changed their mind about how they want to choose the colors, so
#   this code might not be relevant anyway

def get_random_color(day, desired_period):
    # percentage color increase or decrease per time interval
    percent_step = 5

    # i don't know if this makes a difference, but it seemed like it would be better to have a
    # range of size 10 for each color rather than size 1, for the random int function to work better.
    # i'm not sure if that's correct or not, but it doesn't hurt

    percent_scaling_factor = 10

    num_colors_today = day * COLORS_PER_DAY
    max_value = num_colors_today * percent_scaling_factor
    current_color = day - 1

    # target_other is probably not a multiple of increment, so last step for each color will move probability here
    # target_chakra = target_other * 2: final evolved probability for the day's chakra, twice as large as others

    target_other = max_value / (day + 1)

    increment = percent_step * max_value / 100
    iterations = 0

    # probability arrays - these are used by get_random_color() - should work for up to 14 days
    # i think if you need more than 14, you can just make these arrays and it will still work
    # For the 2017 BM installation, 7 of these array slots will be used and the rest will be empty / irrelevant

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

    if TESTING_FULL_PROGRAM:
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


# puts rgb values of desired colors [0 to 255] in linear order r, o, y, g, b, p, w into an array

# ------------------------------------- 2d plot of led animation for use during testing --------------------------------
# Works sort of, but unfortunately there are too many pixels to really see any detail once it's created

def plot_journey(led_array):
    # plt.axes()
    # circle = plt.Circle((0, 0), radius=0.75, fc='y')

    for angle in range(0, 71):
        x_line = math.sin(5 * angle)
        y_line = math.cos(5 * angle)

        for scalar in range(10, 110):
            if led_array[scalar - 10][angle] == 0:
                col = 'b'
            elif led_array[scalar - 10][angle] == 1:
                col = 'r'
            elif led_array[scalar - 10][angle] == 2:
                col = 'g'
            circle = plt.Circle((x_line * scalar, y_line * scalar), .5, fc=col)
            plt.gca().add_patch(circle)

    plt.axis('scaled')
    plt.show()


# ------------------------------------------------- scale() -----------------------------------------------

#  this function will scale RGB values up and down to change brightness,
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


# ---------------------------------------------- ring_pixel_2_node_pixel -----------------------------------------------
# convert ring_index (0 to 35), led_index (0 to 407)
# to node, strip, and strip pixel to use coords that nodes use

def ring_pixel_2_node_strip_pixel(ring_index, led_index):
    node = get_node(ring_index)
    strip = ring_index % 3
    strip_pixel = led_index * strip
    return node, strip, strip_pixel


def sunrise_meditation(today):
    print("sunrise meditation")
    cellular()


def day_program(today):
    print("daytime")


def sunset_meditation(today):
    print("sunset meditation")


def bass_component():
    print "bass component"
    # one program which brings in different undulating effects tied to bass noises


def mid_range():
    print "midrange"
    # one program which responds to midrange noises with rhythmy things


# single program which responds to treble noises with thin line racy things and sparkles
def treble():
    print "treble"


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
