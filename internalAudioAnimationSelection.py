from dataBaseInterface import DataBaseInterface
import pprint
import ujson
import global_vars
import select, socket, string, struct, sys, time
from random import randint
from random import sample
import Queue	# thread safe
from audio_event_queue import DoublyLinkedList
from llist import dllist, dllistnode


NUM_7_COLOR_ANIMATIONS = 2
NUM_BASE_ANIMATIONS = 3
NUM_MID_ANIMATIONS = 3
NUM_SPARKLE_ANIMATIONS = 2

NUM_BEAT_EFFECTS = 1
NUM_PARAMETERS = 30
NUM_COLORS_PER_PALETTE = 7

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
        [0, 255],  # sparkle black thickness
        [-1, 1],  # sparkle intra ring motion: -1 CCW, 0 none, 1 CW, 2 alternate, 3 split (down from top)
        [0, 255],  # sparkle intra ring speed
        [-1, 1],  # sparkle inter ring motion: -1 = CCW, 0 = none, 1 = CW
        [0, 255],  # sparkle inter ring speed
        [0, 6], # sparkle max dim
        [0, 255], # sparkle range
        [1, 50], # sparkle spawn frequency
        [0, NUM_7_COLOR_ANIMATIONS - 1],  # which 7 color animation to play, show bound 28
        [0, len(palette) - 1], # which color palette, show bound 29
        [0, NUM_BEAT_EFFECTS - 1],  # which beat effect to use to respond to beat with LEDs, show bound 30
        [0,1], # is_beat boolean, show bound 31
        [0, 100]  # beat proximity - how close you are to beat. so when beat prox >= 95 or <= 5, can smooth beat response
    ]

mega_to_node_map = {
    1: 4,
    2: 2,
    3: 1,
    4: 0,
    5: 5,
    6: 6,
    7: 3,
    }

NUM_ANIMATIONS = 9	# animation programs are numbered 0 through 8
TIME_LIMIT = 5		# number of seconds between animation changes
auto_show = None
last_show_change_sec = 0.0

show_parameters = [0] * NUM_PARAMETERS
show_colors = [[333 for rgb in range(0, 3)] for i in range(0, NUM_COLORS_PER_PALETTE)]	 # invalid values

#event_queue = dllist()
event_queue = DoublyLinkedList()
pp = pprint.PrettyPrinter(indent=4)

NUM_AUDIO_CHANNELS = 7
current_internal_track_per_channel = [0] * NUM_AUDIO_CHANNELS

#audio_event_queue = Queue.Queue()	# create audio event FIFO queue

def constrained_random_parameter(i):
    if show_bounds[i][0] == -1 and show_bounds[i][1] == 1:
        new_parameter = show_bounds[i][randint(0,1)]	# no zero value
    else:
        new_parameter = randint(show_bounds[i][0], show_bounds[i][1])
    # change to unsigned int for passing to due
    if new_parameter < 0:
        new_parameter += 256
    return new_parameter

#TODO make this more intelligent
def constrained_weighted_parameter(i, magnitude):
    if show_bounds[i][0] == -1 and show_bounds[i][1] == 1:
        new_parameter = show_bounds[i][randint(0,1)]	# no zero value
    else:
        old_parameter = show_bounds[i]
        new_parameter = old_parameter + magnitude
    # change to unsigned int for passing to due
    if new_parameter < 0:
        new_parameter += 256
    return new_parameter

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


def constrain_show():
    global show_parameters
    if show_parameters[BACKGROUND_INDEX] == 0 and show_parameters[MIDLAYER_INDEX] == 0 and show_parameters[SPARKLE_INDEX] == 0:
        show_parameters[BACKGROUND_INDEX] = 1	# never black


def interpret_audio_msg(audio_msg):
    channel_map = get_audio_file_info(audio_msg)
    #pp.pprint(channel_map)
    for channel in channel_map.keys():
        if current_internal_track_per_channel[channel] > 0: #channel already had a track on it
            old_audio = current_internal_track_per_channel[channel]
            remove_audio_events_from_queue(old_audio)

        audioInfo = channel_map[channel]
        current_internal_track_per_channel[channel] = audioInfo
        queue_audio_events(audioInfo)

#RJS I don't like how this hard coded... now if the audio contorl message changes this needs to as well.
def get_audio_file_info(audio_msg):
    #current msg format: a0;1;0;0,50,0,0
    info = audio_msg.split(";")
    tracks = info[3].split(",")
    output = {}
    i = 0
    while(i < len(tracks)): #last int value not track selection related
        if int(tracks[i]) > 0:
            output[i] = (DataBaseInterface().grabAudioInfo(tracks[i]))
        i += 1
    return output

# ------------------------------------------------- internal_sound_animations_program() -----------------------------------------------
# show for when the journey playing its internal audio and there the external audio
#is not past threshold amount aka art car not detected
def do_internal_sound_animations(audio_msg, init = False):
    interpret_audio_msg(audio_msg)

    #pulls from event_queue
    drive_internal_animations(init)

def drive_internal_animations(init):
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

    while True:
        next_audio_event_node = event_queue.pop()
        next_audio_event = next_audio_event_node.value
        if next_audio_event.time > time.time(): #valid 'next' event
            magnitude = next_audio_event.magnitude
            show_param = randint(0, 27)
            old_param_value = show_parameters[show_param]
            if next_audio_event.kind == "freqband":
                new_param_value = constrained_weighted_parameter(show_param, magnitude)
                print "Frq event: Set show_param[" + str(show_param) + "] from " + str(old_param_value) + " to " + str(new_param_value)

            elif next_audio_event.kind == "amplitude":
                #TODO make this more intelligent
                new_param_value = constrained_random_parameter(show_param)
                print "Amp event: Set show_param[" + str(show_param) + "] from " + str(old_param_value) + " to " + str(new_param_value)

            break

def remove_audio_events_from_queue(audioInfo):
    node_list = [] #retrieving list of llistnodes first as access to nieghbors is O(1) and
                   #doing deletes resets cache thus retrieval would be back O(n) every time
    for event in audioInfo.events:
        #node_list.append(event_queue.nodeat(event.queue_index))
        #event_queue.remove(event.queue_node)
        try:
            event_queue.remove(event.time)
        except ValueError:
            print "event " + event + " already has been removed from queue"

    #for node in node_list:
    #    event_queue.remove(node)

def queue_audio_events(audioInfo):
    cur_time = time.time()
    for event in audioInfo.events:
        event.time += cur_time
        node = event_queue.add(event)
        #event.queue_node = node



msg = "a0;1;0;1,50,2,0"


do_internal_sound_animations(msg, True)
do_internal_sound_animations(msg)

#valid test!! of adding and modifying time property
#interpret_audio_msg(msg)
#a_info = current_internal_track_per_channel[1]
#b_info = current_internal_track_per_channel[0]

##print a_info
#queue_audio_events(a_info)
##print a_info
#queue_audio_events(b_info)
#print "\n\n"
#print event_queue

#remove_audio_events_from_queue(a_info)
#print "\n\n"
#print event_queue
#print event_queue.size




#print current_internal_track_per_channel
#print "\n\n"
#print ujson.dumps(get_audio_file_info("a0;1;0;0,50,0,0"), indent = 2)
