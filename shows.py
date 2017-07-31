#!/usr/bin/python
import numpy,time
from random import randint
from random import sample
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

BACKGROUND_INDEX = 0
MIDLAYER_INDEX = 8
SPARKLE_INDEX = 17

# Pre-defined color palettes
fruit_loop = [[25,0,25], [25,15,0], [180,10,70], [140,60,180], [180,60,60], [255,255,120], [255,100,180]]
icy_bright = [[37,28,60], [70,0,28], [255,108,189], [0,172,238], [44,133,215], [255,255,255], [254,207,24]]
watermelon = [[40,29,35], [5,45,15], [47,140,9], [72,160,5], [148,33,137], [47,192,91], [70,190,91]]
pride = [[255, 0, 0], [255, 127, 0], [255, 255, 0], [0, 255, 0], [0, 0, 255], [75, 0, 130], [148, 0, 211]]
edirp = [[148, 0, 211], [75, 0, 130], [0, 0, 255], [0, 255, 0], [255, 255, 0], [255, 127, 0], [255, 0, 0]]
palette = [fruit_loop, icy_bright, watermelon, pride, edirp]

show_bounds = [  # order must match show_parameters
        # [min, max]
        # show bounds 0 through 7 concern base animations
        [0, NUM_BASE_ANIMATIONS],  # BACKGROUND_INDEX, which background animation to use
        [0, 255], # base color thickness
        [0, 255], # base black thickness
        [-128,127], # base ring offset
        [-1, 1], # base intra ring motion: -1 CCW, 0 none, 1 CW, 2 alternate, 3 split (down from top)
        [0, 255], # base intra ring speed
        [-1,1], # base inter ring motion: -1 = CCW, 0 = none, 1 = CW
        [0,255], # base inter ring speed
        # show bounds 8 through 16 concern mid layer animations
        [0, NUM_MID_ANIMATIONS],  # MIDLAYER_INDEX, which mid layer animation to use
        [1, 3], # mid num colors
        [0, 255],  # mid color thickness
        [0, 255],  # mid black thickness
        [-128,127],  # mid ring offset
        [-1, 1],  # mid intra ring motion: -1 CCW, 0 none, 1 CW, 2 alternate, 3 split (down from top)
        [0, 255],  # mid intra ring speed
        [-1, 1],  # mid inter ring motion: -1 = CCW, 0 = none, 1 = CW
        [0, 255],  # mid inter ring speed
        # show bounds 17 through 27 concern sparkle animations
        [0, NUM_SPARKLE_ANIMATIONS],  # SPARKLE_INDEX, which sparkle animation to use
        [2, 200],  # sparkle portion
        [0, 255],  # sparkle color thickness
        [-1, 1],  # sparkle intra ring motion: -1 CCW, 0 none, 1 CW, 2 alternate, 3 split (down from top)
        [0, 255],  # sparkle intra ring speed
        [-1, 1],  # sparkle inter ring motion: -1 = CCW, 0 = none, 1 = CW
        [0, 255],  # sparkle inter ring speed
        [0, 255], # sparkle min dim
        [0, 255], # sparkle max dim
        [0, 255], # sparkle range
        [0, 50], # sparkle spawn frequency, 0 == off entirely (Functions as a boolean when 0|1)
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

NUM_ANIMATIONS = 9	# animation programs are numbered 0 through 8
TIME_LIMIT = 5		# number of seconds between animation changes
auto_show = None
last_show_change_sec = 0.0

show_parameters = [0] * NUM_PARAMETERS
show_colors = [[333 for rgb in range(0, 3)] for i in range(0, NUM_COLORS_PER_PALETTE)]	# invalid values

event_queue = SortedDLL() #create sorted dll to act as the audio event queue (with super duper special powers)
NUM_AUDIO_CHANNELS = 7
current_internal_track_per_channel = [0] * NUM_AUDIO_CHANNELS
internal_audio_show = True #triggers internal audio animations..

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

def choose_random_colors_from_palette():
    # choose which colors out of the chosen palette to use
    #shuffle the lower 2 colors, mid 3 colors, and upper 2 colors of chosen palette
    bg_order = sample(range(0,2), 2)
    mid_order = sample(range(2,5), 3)
    sp_order = sample(range(5,7), 2)

    current_palette = show_parameters[29]
    show_colors[0] = palette[current_palette][bg_order[0]]
    show_colors[1] = palette[current_palette][bg_order[1]]
    show_colors[2] = palette[current_palette][mid_order[0]]
    show_colors[3] = palette[current_palette][mid_order[1]]
    show_colors[4] = palette[current_palette][mid_order[2]]
    show_colors[5] = palette[current_palette][sp_order[0]]
    show_colors[6] = palette[current_palette][sp_order[1]]

# ------------------------------------------------- edm_program() -----------------------------------------------
# show for when the journey is installed at an event with electronic dance music only
# parameters are somewhat randomly chosen

def edm_program(init=False):
    global bg_start_time, bg_parameter_start_time, mid_start_time, mid_parameter_start_time, sparkle_start_time, sparkle_parameter_start_time, palette_start_time
    global show_parameters, show_colors

    if init:
        if show_colors[0] == [333,333,333]:	# invalid values before initialization
            bg_start_time = bg_parameter_start_time = mid_start_time = mid_parameter_start_time = sparkle_start_time = sparkle_parameter_start_time = palette_start_time = time.time()

            # choose random starting values for each of the parameters
            for i in range(0, NUM_PARAMETERS):
                show_parameters[i] = constrained_random_parameter(i)
            constrain_show()
            choose_random_colors_from_palette()

        print "initial show parameters ", show_parameters
        print "initial show colors" , show_colors
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
        choose_random_colors_from_palette()
        print "palette changed ", show_colors

# ------------ playa_program() --------------------------------------------------------
# a partially scripted program for Burning Man 2017

TEST_CYCLE_MINUTES = 3	# rush through the entire week in this number of minutes
MEDITATION_MINUTES = 20
BURNING_MAN_START = time.mktime(time.strptime('2017-Aug-28 00:00', '%Y-%b-%d %H:%M'))
BURNING_MAN_END   = time.mktime(time.strptime('2017-Sep-04 00:00', '%Y-%b-%d %H:%M'))
NUM_DAYS = int((BURNING_MAN_END - BURNING_MAN_START) / 86400 + 0.5)

daynight = None
def major_playa_mode(when):
    global daynight

    sunrise_time = [1503839940, 1503926400, 1504012860, 1504099320, 1504185780,
                    1504272240, 1504358700, 1504445160, 1504531620]
    sunset_time  = [1503887940, 1503974220, 1504060500, 1504146840, 1504233120,
                    1504319460, 1504405740, 1504492020, 1504578360]

    meditation = None
    if sunrise_time[0] <= when and when < sunset_time[len(sunset_time)-1] + meditation_sec:
        for meditation_start in sunrise_time:
            meditation_end = meditation_start + meditation_sec
            if meditation_start <= when and when < meditation_end:
                meditation = 'sunrise'
                daynight = 'daytime'
                break
        if meditation == None:
            for meditation_start in sunset_time:
                meditation_end = meditation_start + meditation_sec
                if meditation_start <= when and when < meditation_end:
                    meditation = 'sunset'
                    daynight = 'nighttime'
                    break
    else:
        daynight = None
    if meditation == None and daynight == None:	# pretend that daytime is between 6 AM and 6 PM
        hour = int(time.strftime('%-H', time.localtime(when)))
        if (6 <= hour and hour < 18):
            daynight = 'daytime'
        else:
            daynight = 'nighttime'
    return meditation or daynight	# meditation has precedence

real_start_time = -1.0
def playa_program(init=False):
    global real_start_time, meditation_sec, time_compression_factor

    real_time = time.time()
    if init:
        if real_start_time < 0:
            time_compression_factor = float(NUM_DAYS * 60 * 24) / TEST_CYCLE_MINUTES	# 60*24 == minutes per day
            meditation_sec = int(MEDITATION_MINUTES * 60 * time_compression_factor / 233)	# 233 produces about 1/5 of the day with a 3 minute test cycle
            real_start_time = real_time
            edm_program(init)	# good enough for now
            show_parameters[29] = 999	# invalid
        return

    if real_start_time == BURNING_MAN_START:
        virtual_time = real_time	# this is the live show at Burning Man
    else:
        if BURNING_MAN_START <= real_time and real_time < BURNING_MAN_END:
            time_compression_factor = 1.0
            meditation_sec = MEDITATION_MINUTES * 60
            print 'Welcome home!'	# Burning Man has just begun!
            real_start_time = BURNING_MAN_START
            virtual_time = real_time
        else:
            virtual_time = BURNING_MAN_START + (real_time - real_start_time) * time_compression_factor

    mode = major_playa_mode(virtual_time)

    bm_day_index = int((virtual_time - BURNING_MAN_START) / 86400) % NUM_DAYS
    new_palette = bm_day_index % len(palette)
    if show_parameters[29] != new_palette:
        show_parameters[29] = new_palette
        choose_random_colors_from_palette()
        print 'palette changed', show_colors

    print 'playa time advanced to', time.ctime(virtual_time), 'on day', bm_day_index, 'in', mode

# ------------------------------------------------- internal_sound_animations_program() -----------------------------------------------
# show for when the journey playing its internal audio and the external audio
# is not past threshold amount aka art car not detected

# still very much under development.... fine tuning audio_events processing and
# handling here to avoid too frequent or not frequent enough animation parameter changes
def do_internal_sound_animations(audio_msg, init = False):
    interpret_audio_msg(audio_msg)

    #pulls from event_queue
    drive_internal_animations(init)

def progress_audio_queue():
    global event_queue, next_audio_event
    # ensure not looking at events that have already passed
    while True:
        try:
            next_audio_event_node = event_queue.peek()
            next_audio_event = next_audio_event_node.value
            # print "next audio event " + str(next_audio_event)
        except:
            # print "event_queue is empty"
            break

        stale = next_audio_event.time <= timeMs() - 1000
        # print "diff event - now = " + str(next_audio_event.time - timeMs())
        if stale:
            print "it's " + str(timeMs()) + " stale event " + str(next_audio_event) + " popping!"
            event_queue.remove(next_audio_event.time)
        else:
            break

def drive_internal_animations(init):
    global next_audio_event, event_queue
    if init:
        if show_colors[0] == [333,333,333]:	# invalid values before initialization
            bg_start_time = bg_parameter_start_time = mid_start_time = mid_parameter_start_time = sparkle_start_time = sparkle_parameter_start_time = palette_start_time = time.time()

            # choose random starting values for each of the parameters
            for i in range(0, NUM_PARAMETERS):
                show_parameters[i] = constrained_random_parameter(i)
            constrain_show()
            choose_random_colors_from_palette()

        print "initial show parameters ", show_parameters
        print "initial show colors" , show_colors
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
        #print "valid? " + str(valid_time) + " aiming for event time between " + str(lower_bounds) + " - " + str(upper_bounds) + " current event time " + str(next_audio_event.time)

        if valid_time: #valid 'next' event
            magnitude = next_audio_event.magnitude
            show_param = randint(0, 27)
            old_param_value = show_parameters[show_param]
            if next_audio_event.kind == "freqband":
                new_param_value = constrained_weighted_parameter(show_param, magnitude)
                print "Frq event [" + str(next_audio_event) +"]: Set show_param[" + str(show_param) + "] from " + str(old_param_value) + " to " + str(new_param_value)

            elif next_audio_event.kind == "amplitude":
                #TODO make this actually intelligent
                new_param_value = constrained_random_parameter(show_param)
                print "Amp event: Set show_param[" + str(show_param) + "] from " + str(old_param_value) + " to " + str(new_param_value)

            try:
                if event_queue.size > 0:
                    event_queue.remove(next_audio_event.time)
                else:
                    next_audio_event = None
            except AttributeError:
                print "event_queue is empty"

#TODO make this actually intelligent
def constrained_weighted_parameter(i, magnitude):
    if show_bounds[i][0] == -1 and show_bounds[i][1] == 1:
        new_parameter = show_bounds[i][randint(0,1)]	# no zero value
    else:
        old_parameter = show_parameters[i]
        new_parameter = old_parameter + magnitude
    # change to unsigned int for passing to due
    if new_parameter < 0:
        new_parameter += 256
    return new_parameter


def interpret_audio_msg(audio_msg):
    channel_map = get_audio_file_info(audio_msg)
    for channel in channel_map.keys():
        if current_internal_track_per_channel[channel] > 0: #channel already had a track on it
            old_audio = current_internal_track_per_channel[channel]
            remove_audio_events_from_queue(old_audio)

        audioInfo = channel_map[channel]
        current_internal_track_per_channel[channel] = audioInfo
        queue_audio_events(audioInfo)


# RJS I don't like how this hard coded... if the audio contorl message changes this needs to as well.
def get_audio_file_info(audio_msg):
    # current msg format: a0;1;0;0,50,0,0
    info = audio_msg.split(";")
    tracks = info[3].split(",")
    output = {}
    i = 0
    while(i < len(tracks)):
        if int(tracks[i]) > 0:
            output[i] = (DataBaseInterface().grabAudioInfo(tracks[i]))
        i += 1
    return output


def remove_audio_events_from_queue(audioInfo):
    for event in audioInfo.events:
        try:
            event_queue.remove(event.time)
        except ValueError:
            print "event " + event + " already has been removed from queue"


def queue_audio_events(audioInfo):
    cur_time_ms = timeMs()
    for event in audioInfo.events:
        event.time += cur_time_ms
        node = event_queue.add(event)


def timeMs():
    return int(round(time.time() * 1000))
