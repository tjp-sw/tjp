#!/usr/bin/python
import Queue	# thread safe
import select, socket, string, struct, sys, time
from random import randint

mega_to_node_map = {
    1: 2,
    2: 1,
    3: 4,
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
max_palette = 3		# this will vary by time and date at burning man
NUM_COLORS_PER_PALETTE = 3 # how many colors to use out of this palette
NUM_BEAT_EFFECTS = 1
TIME_LIMIT = 30		# number of seconds between animation changes
auto_show_change = False
last_show_change_sec = 0.0

def do_auto(ignored, neglected):
    global auto_show_change, last_show_change_sec
    auto_show_change = not auto_show_change
    last_show_change_sec = time.time()

# an arbitrary initial set of show parameters and colors
show_colors = [2, 1, 4]		# number of items must match due code: NUM_COLORS_PER_PALETTE? max_palette?
show_parameters = [8, 1, 2, 1, 0, len(show_colors), len(show_colors), len(show_colors), len(show_colors), 3, 8, 1, 2, 0, 10, 1, 2, 0, 0, 2]
show_bounds = [			# order must match show_parameters
  # [min, max]
    [0, NUM_ANIMATIONS - 1],	# ANIMATION_INDEX, which animation to play
    [0, 9],			# BACKGROUND_INDEX, which background animation to use
    [0, 9],			# MIDLAYER_INDEX, which mid layer animation to use
    [0, 9],			# SPARKLE_INDEX, which sparkle animation to use
    [0, NUM_BEAT_EFFECTS - 1],	# BEAT_EFFECT_INDEX, which beat effect to use / how to respond to beat with LEDs
    [1, NUM_COLORS_PER_PALETTE - 2],	# NUM_EDM_COLORS_INDEX, how many colors to use out of the edm palette
    [1, NUM_COLORS_PER_PALETTE - 2],	# NUM_BG_COLORS_INDEX
    [1, NUM_COLORS_PER_PALETTE - 2],	# NUM_ML_COLORS_INDEX
    [1, NUM_COLORS_PER_PALETTE - 2],	# NUM_SP_COLORS_INDEX
    [1, 10],			# COLOR_THICKNESS_INDEX, how many pixels should the bands of color be
    [0, 5],			# BLACK_THICKNESS_INDEX, how many black LEDs between color bands
    [0, 3],			# INTRA_RING_MOTION_INDEX, which direction to move lights inside a ring:  0 = none, 1 = CW, 2 = CCW, 3 = split
    [1, 10],			# INTRA_RING_SPEED_INDEX, how fast should intra_ring motion be fixme: still need to decide on units
    [0, 2],			# COLOR_CHANGE_STYLE_INDEX, how should color change during an animation: 0 = none, 1 = cycle thru selected, 2 = cycle thru palette
    [0, 10],			# RING_OFFSET_INDEX, how far one ring pattern is rotated from neighbor -10 -> 10	FIXME allow negative numbers!
    [0, 1],			# INTER_RING_MOTION_INDEX, which color palette to use
    [0, 4],			# INTER_RING_SPEED_INDEX
    [0, 1],			# COLOR_ROTATION, if true, cycle colors through all currently chosen colors
    [0, 1],			# COLOR_RAINBOW_INDEX, if true, spread currently chosen colors around structure like rainbow
    [0, max_palette - 1],	# PALETTE_INDEX, which color palette to use
    ]
NUM_EDM_COLORS_INDEX = 5	# index into show_bounds

def do_show(cmd, param):
    global last_show_change_sec, show_colors, show_parameters
    if param:
        if cmd == 'SetAnimation':
            show_parameters[0] = (ord(param) - ord('0')) % NUM_ANIMATIONS
            last_show_change_sec = time.time()
    do_send(None, struct.pack('>c%uB' % (len(show_parameters) + len(show_colors)), 's', *(show_parameters + show_colors)))
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

def do_random_program_change():
    global show_colors, show_parameters

    # randomly choose a parameter to change
    change_param = randint(0, len(show_parameters) - 1)
    if change_param == NUM_EDM_COLORS_INDEX:
        # randomly change a color
        change_param = randint(0, len(show_colors) - 1)
        show_colors[change_param] = randint(0, 5)
    else:
        # randomly change that parameter
        lower, upper = show_bounds[change_param]
        show_parameters[change_param] = randint(lower, upper)

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
            do_random_program_change()
            last_show_change_sec = time.time()
            do_show(None, None)

    except KeyboardInterrupt:
        running = False

    except:
        print sys.exc_value
        do_disconnect(None, None)	# TODO: be more selective

for s in sources:
    s.close()
