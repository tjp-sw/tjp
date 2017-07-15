#!/usr/bin/python
import Queue	# thread safe
import select, socket, string, struct, sys, time
from random import randint
from random import sample

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
NUM_COLOR_PALETTES = 4

BASE_TIME_LIMIT = 87
MID_TIME_LIMIT = 61
SPARKLE_TIME_LIMIT = 43
SPARKLE_PARAMETER_TIME_LIMIT =  17
PALETTE_TIME_LIMIT = 7

# show_bounds = [[0 for i in range(0, 3)] for j in range(0,NUM_PARAMETERS)]
# lower = [0] * NUM_PARAMETERS
# upper = [0] * NUM_PARAMETERS
show_bounds = [  # order must match show_parameters
        # [min, max]
        # show bounds 0 through 7 concern base animations
        [1, NUM_BASE_ANIMATIONS],  # BACKGROUND_INDEX, which background animation to use
        [0, 255], # base color thickness
        [0, 255], # base black thickness
        [-1, 1], # base intra ring motion: -1 CCW, 0 none, 1 CW, 2 alternate, 3 split (down from top)
        [0, 255], # base intra ring speed
        [-1,1], # base inter ring motion: -1 = CCW, 0 = none, 1 = CW
        [0,255], # base inter ring speed
        [-128,127], # base ring offset
        # show bounds 8 through 16 concern mid layer animations
        [0, NUM_MID_ANIMATIONS],  # MIDLAYER_INDEX, which mid layer animation to use
        [1, 3], # mid num colors
        [0, 255],  # mid color thickness
        [0, 255],  # mid black thickness
        [-1, 1],  # mid intra ring motion: -1 CCW, 0 none, 1 CW, 2 alternate, 3 split (down from top)
        [0, 255],  # mid intra ring speed
        [-1, 1],  # mid inter ring motion: -1 = CCW, 0 = none, 1 = CW
        [0, 255],  # mid inter ring speed
        [-128,127],  # mid ring offset
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
        [0, NUM_COLOR_PALETTES - 1], # which color palette, show bound 29
        [0, NUM_BEAT_EFFECTS - 1],  # which beat effect to use to respond to beat with LEDs, show bound 30
        [0,1], # is_beat boolean, show bound 31
        [0,100] # beat proximity - how close you are to beat. so when beat prox >= 95 or <= 5, can smooth beat response
    ]

# Pre-defined color palettes
fruit_loop = [[25,0,25], [25,15,0], [180,10,70], [140,60,180], [180,60,60], [255,255,120], [255,100,180]]
icy_bright = [[37,28,60], [70,0,28], [255,108,189], [0,172,238], [44,133,215], [255,255,255], [254,207,24]]
watermelon = [[40,29,35], [5,45,15], [47,140,9], [72,160,5], [148,33,137], [47,192,91], [70,190,91]]
pride = [[255, 0, 0], [255, 127, 0], [255, 255, 0], [0, 255, 0], [0, 0, 255], [75, 0, 130], [148, 0, 211]]
edirp = [[148, 0, 211], [75, 0, 130], [0, 0, 255], [0, 255, 0], [255, 255, 0], [255, 127, 0], [255, 0, 0]]
palette = [fruit_loop, icy_bright, watermelon, pride, edirp]

mega_to_node_map = {
    1: 4,
    2: 2,
    3: 1,
    4: 0,
    5: 5,
    6: 6,
    7: 3,
    }

# close all TCP connections and continue to run
def do_disconnect(ignored, neglected):
    global listener, sources #, writable, oops
    copy_of_sources = [] + sources	# array elements will be removed from sources itself
    for s in copy_of_sources:
        if s is not sys.stdin and s is not listener:
            disconnect(s, 'closing')
    # writable = []
    # oops = []

def do_list(ignored, neglected):
    global listener, sources
    for s in sources:
        if s is not sys.stdin:
            if s is listener:
                print 'listening on %s:%d' % s.getsockname()
            else:
                print 'connected to', remote_name[s]

def do_quit(ignored, neglected):
    global running
    running = False

# send a message to one remote or all
def do_send(socket, message):
    global message_queues, writing
    # print 'sending', repr(message)
    if socket and socket != 'send':
        list = [socket]
    else:
        list = message_queues
    for s in list:
        message_queues[s].put(message)
        if s not in writing:
            writing.append(s)

NUM_ANIMATIONS = 9	# animation programs are numbered 0 through 8
TIME_LIMIT = 5		# number of seconds between animation changes
auto_show_change = False
last_show_change_sec = 0.0

def do_auto(ignored, neglected):
    global auto_show_change, last_show_change_sec
    auto_show_change = not auto_show_change
    last_show_change_sec = time.time()

# ------------------------------------------------- edm_program() -----------------------------------------------
# show for when the journey is installed at an event with electronic dance music only
# parameters are somewhat randomly chosen; this will change at bm when params will be determined by sound
# def edm_program():

bg_start_time = time.time()
mid_start_time = time.time()
sparkle_start_time = time.time()
sparkle_parameter_start_time = time.time()
palette_start_time = time.time()

show_parameters = [0] * NUM_PARAMETERS
show_colors = [[0 for rgb in range(0, 3)] for i in range(0, NUM_COLORS_PER_PALETTE)]

# choose random starting values for each of the parameters
for i in range(0, NUM_PARAMETERS):
    new_parameter = randint(show_bounds[i][0], show_bounds[i][1])
    if show_bounds[i][0] == -1 and show_bounds[i][1] == 1 and new_parameter == 0:
        new_parameter = 1;		# FIX: no zero value for now
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
# send_due_parameters()

def edm_program():
    global bg_start_time, mid_start_time, sparkle_start_time, sparkle_parameter_start_time, palette_start_time
    global show_parameters, show_colors
#    while True:  # run forever until show is taken down

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
        for i in range (0, 8):
            new_parameter = randint(show_bounds[i][0], show_bounds[i][1])
            if show_bounds[i][0] == -1 and show_bounds[i][1] == 1 and new_parameter == 0:
                new_parameter = 1;		# FIX: no zero value for now

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
            if show_bounds[i][0] == -1 and show_bounds[i][1] == 1 and new_parameter == 0:
                new_parameter = 1;		# FIX: no zero value for now

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
#    send_due_parameters()

    #time.sleep(3)
    current_time = time.time()

def do_show(cmd, param):
    global last_show_change_sec, show_colors, show_parameters
    if param:
        if cmd == 'SetAnimation':
            show_parameters[0] = (ord(param) - ord('0')) % NUM_ANIMATIONS
            last_show_change_sec = time.time()
    show_colors_list = []
    for i in range(0, NUM_COLORS_PER_PALETTE):
        show_colors_list += show_colors[i]
    do_send(None, struct.pack('>c%uB' % (len(show_parameters) + len(show_colors_list)), 's', *(show_parameters + show_colors_list)))
    print 'show:', repr(show_parameters), repr(show_colors)

next_timesync_sec = 0.0

# almost the same as do_send() above, but send the very latest
# timestamp to each remote or just the specified remote
def do_time(socket, neglected):
    global message_queues, next_timesync_sec, writing
    if socket == 'time':
        list = message_queues
        next_timesync_sec = time.time() + 31	# half a minute in the future
    else:
        list = [socket]
    for s in list:
        # send microseconds, which is all the precision we have
        # network byte order, a character and 64-bit unsigned integer
        message_queues[s].put(struct.pack('>cQ', 't', int(time.time() * 1000000.0)))
        if s not in writing:
            writing.append(s)

# a command and optional 8-bit unsigned integer
def do_simple(cmd, param):
    if param:
        do_send(None, struct.pack('>cB', cmd[0:1], ord(param) - ord('0')))
    else:
        do_send(None, cmd[0:1])

control_messages = {
#    'SetAllAudio':	do_unimplemented,
#    'SetAudioCh':	do_unimplemented,
#    'SetVolCh':		do_unimplemented,
#    'MuteAllAudio':	do_unimplemented,
    'SetAnimation':	(do_show, None, None),
#    'SetDynAnimation':	do_unimplemented,
#    'AllLEDoff':	(do_simple, 'program', '0'),
#    'CheckHandStat':	do_unimplemented,
#    'CheckAudioIn':	do_unimplemented,
    'auto':		(do_auto, None, None),
    'disconnect':	(do_disconnect, None, None),
    'led':		(do_simple, 'program', None),
    'list':		(do_list, None, None),
    'node':		(do_simple, None, None),
    'quit':		(do_quit, None, None),
    'reconnect':	(do_simple, None, None),
    'send':		(do_send, None, None),
    'time':		(do_time, None, None),
    }

# listen for TCP connections on the specified port
listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)	# disregard TIME_WAIT state
listener.setblocking(0)		# don't wait for anything
while listener:
    try:
        listener.bind(('', 3528))	# any local IPv4 address, port 3528
        break
    except:
        print sys.exc_value
        time.sleep(10)
listener.listen(6)		# maximum connection backlog

sources = [sys.stdin, listener]	# read from these
writing = []		# write to these sockets
message_queues = {}	# queues of outgoing messages
remote_name = {}	# socket remote name because getpeername() can fail

# print a message and clean up resources associated with an open TCP connection
def disconnect(socket, msg):
    print msg, remote_name[socket]
    if socket in writing:
        writing.remove(socket)
    sources.remove(socket)
    socket.close()	# shutdown() is too abrupt, do a graceful close()
    del message_queues[socket]
    del remote_name[socket]

do_list(None, None)
print sorted(control_messages.keys())
running = True
while running:
    try:
        if len(message_queues) == 0:
            timeout = None	# wait indefinitely when there are no remotes
        else:
            timeout = next_timesync_sec - time.time()
            if auto_show_change:
                show_timeout = last_show_change_sec + TIME_LIMIT - time.time()
                if show_timeout < timeout:
                    timeout = show_timeout
            if timeout <= 0:
                timeout = 0.01
        # could generate writing list here from nonempty message_queues
        readable, writable, oops = select.select(sources, writing, sources, timeout)

        for s in readable:
            if s is sys.stdin:
                message = s.readline().strip()
                try:
                    command, parameters = string.split(message, None, 1)	# one word separated by whitespace from the parameter(s)
                except ValueError:				# no whitespace
                    command = message
                    parameters = None
                try:
                    action = control_messages[command]
                    function = action[0]
                    if action[1]:
                        command = action[1]
                    if action[2]:
                        parameters = action[2]
                    function(command, parameters)
                except KeyError:
                    print sorted(control_messages.keys())	# unrecognized
            elif s is listener:
                remote, addr = s.accept()
                remote.setblocking(0)
                sources.append(remote)			# remember this connection
                message_queues[remote] = Queue.Queue()	# create outgoing FIFO queue
                remote_name[remote] = '%s:%d' % addr	# addr is the same as remote.getpeername()
                print 'connection from', remote_name[remote]
                do_time(remote, None);			# synchronize time immediately
            else:
                try:
                    message = s.recv(1024)
                except:
                    print sys.exc_value
                    message = None
                if message:
                    if message[0:1] == 'b':
                        if len(message) == 55:
                            do_send(None, message)	# relay to all nodes
                            print 'beat', repr(message), 'from', remote_name[s]
                    elif message[0:1] == 'm':
                        mega_number = ord(message[1:2])
                        try:
                            node_number = mega_to_node_map[mega_number]
                        except KeyError:
                            if mega_number >= 100:	# mock_mega
                                node_number = mega_number % 6 + 10
                            else:
                                node_number = None
                        print 'mega', mega_number, '( node ', repr(node_number), ') is at', remote_name[s]
                        if node_number != None:
                            do_send(s, struct.pack('>cB', 'n', node_number))
                    else:
                        print 'received', repr(message), 'from', remote_name[s]
                else:
                    disconnect(s, 'remote closed')

        for s in writable:
            try:
                next_msg = message_queues[s].get_nowait()
            except Queue.Empty:
                writing.remove(s)
            else:
                # print 'sending', repr(next_msg), 'to', remote_name[s]
                try:
                    sent = s.send(next_msg)
                except socket.error as err:
                    disconnect(s, err.strerror)
                else:
                    unsent = len(next_msg) - sent
                    if unsent != 0:
                        print 'failed to send %d bytes to %s' % (unsent, remote_name[s])
                        # Queue module can't push unsent data back to the front of the queue

        for s in oops:
            disconnect(s, sys.exc_value)

        if time.time() > next_timesync_sec:
            do_time('time', None)

        if auto_show_change and time.time() > last_show_change_sec + TIME_LIMIT:
            edm_program()
            last_show_change_sec = time.time()
            do_show(None, None)

    except KeyboardInterrupt:
        running = False

    except:
        print sys.exc_value
        do_disconnect(None, None)	# TODO: be more selective

for s in sources:
    s.close()
