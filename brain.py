#!/usr/bin/python
import Queue	# thread safe
import numpy, select, socket, string, struct
from shows import *
from beats import *
import music
from artCarHandler import ArtCarHandler
from internalAnimationsHandler import InternalAninamtionsHandler

artCarHandler = ArtCarHandler(ART_CAR_HELLO_DURATION, ART_CAR_AMPLITUDE_THRESHOLD, ART_CAR_MIN_HELLO_DURATION)

mega_to_node_map = {
    1: 2,
    2: 0,
    3: 1,
    4: 5,
    5: 4,
    6: 3,
    7: 6,
    }

# close all TCP connections and continue to run
def do_disconnect(ignored, neglected):
    global listener, sources  # , writable, oops
    copy_of_sources = [] + sources	 # array elements will be removed from sources itself
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
    #print 'sending', repr(message)
    if socket and socket != 'send':
        list = [socket]
    else:
        list = message_queues
    for s in list:
        message_queues[s].put(message)
        if s not in writing:
            writing.append(s)

def do_auto(ignored, show_name):
    global auto_show, last_show_change_sec

    auto_show = show_name
    if auto_show:
        auto_show(True)
        last_show_change_sec = time.time()


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

    print_parameters()


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

#play mediation manually
def do_meditation (ignored, meditation_num):
    meditation_play = music.manual_meditation(meditation_num)
    if meditation_play:
        do_send(None, encapsulated_audio_msg(meditation_play))

def do_change_palette(ignored, style):
    if style == None:
        choose_new_playa_palette()
    else:
        use_test_palette()


def stop_auto(ignore, neglect):
    global auto_show
    auto_show = None


control_messages = {
#    'SetAllAudio':	    do_unimplemented,
#    'SetAudioCh':	    do_unimplemented,
#    'SetVolCh':		do_unimplemented,
#    'MuteAllAudio':	do_unimplemented,
    'SetAnimation':	    (do_show, None, None),
#    'AllLEDoff':	    (do_simple, 'program', '0'),
#    'CheckHandStat':	do_unimplemented,
#    'CheckAudioIn':	do_unimplemented,
    'cp':		(do_change_palette, None, None),
    'day':		(do_date, None, None),
    'disconnect':	(do_disconnect, None, None),
    'edm':		(do_auto, None, edm_program),
    'led':		(do_simple, 'program', None),
    'list':		(do_list, None, None),
    'node':		(do_simple, None, None),
    'pause':		(do_auto, None, None),
    'playa':		(do_auto, None, playa_program),
    'quit':		(do_quit, None, None),
    'reconnect':	(do_simple, None, None),
    'send':		(do_send, None, None),
    'sp':		(do_set_show_parameter, None, None),
    'time':		(do_time, None, None),
    'tp':		(do_change_palette, None, 'test'),
    'meditation': (do_meditation, None, None),
    'stopAuto': (stop_auto, None, None)
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

# insert a length byte into an audio message
def encapsulated_audio_msg(audio_msg):
    size = len(audio_msg) - 1	# don't count the 'a' at the beginning
    msg = struct.pack('>cB', audio_msg[0], size) + audio_msg[1:]
    return msg


def get_external_amplitude_sum(channel_data):
    amplitude = 0
    # 7 channels of audio data
    for i in range(0, 6):
        try:
            amplitude += abs(ord(channel_data[i]) - ord(channel_data[i+7]))
        except:
            print "channel data format does not have 2 * 7 channels of information " + str(len(channel_data))
            return -1

    return amplitude


def check_art_car_status(ring_num, amplitude):
    global rings_to_hello_animation, auto_show, art_car

    if ring_num is None or amplitude is None:
        print "seems as those the data did not make a plane, ring -> art car detection not possible"
        if not artCarHandler.mock:
            return

    ring_ac_newly_detected = artCarHandler.handle_amplitude_info(ring_num, amplitude)
    print "ring detected " + str(ring_ac_newly_detected)
    print "art car ring " + str(artCarHandler.art_car)
    if ring_ac_newly_detected is None:
        # artcar total structure animation was running now stop
        get_internal_animations_handler().set_do_animations(True)
        print "stopping art car edm takeover animations"
        do_auto(None, playa_program)
        do_show(None, None)

    elif ring_ac_newly_detected == artCarHandler.art_car and ring_ac_newly_detected != -1:
        # return value signaling ART_CAR_HELLO_DURATION exceeded - trigger edm animations
        get_internal_animations_handler().set_do_animations(False)

        # trigger edm animations on whole structure
        do_auto(None, art_car_edm)
        do_show(None, None)

    elif ring_ac_newly_detected >= 0:
        # Utilizing ArtCarHadler to know which is the oldest ring_num.
        # Necessary becuase it sounds like the messaging system will be a broadcast
        # with every node getting the 'hello' ring. (Not just a response to whoever sent the cahnnel data)
        oldest_ring = artCarHandler.get_oldest_ring()

        # check if the oldest is past the min hello duration
        if artCarHandler.get_detected_duration(oldest_ring) > artCarHandler.get_min_hello_duration():
            # send the hello animation broadcast with target ring
            show_parameters[ART_CAR_RING_PARAM] = oldest_ring
            show_parameters[SEVEN_PAL_BEAT_PARAM_START] = artCarHandler.get_ring_animation(oldest_ring)
            do_show(None, None)


    # send hello animation stop message broadcast for target ring
    for ring_num in artCarHandler.rings_to_stop_hello_animation:
        # HELP: HOW TO SEND HELLO ANIMATION AND RING NUM?
        # TODO do_send(?, ?) i.e. do_send(None, oldest_ring)
        pass

do_list(None, None)
print sorted(control_messages.keys())
mega_music = music.Music()
running = True
while running:
    try:
        if len(message_queues) == 0:
            timeout = None	# wait indefinitely when there are no remotes
        else:
            timeout = next_timesync_sec - time.time()
            if auto_show:
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
                    while len(message) > 0:
                        if message[0:1] == 'b':
                            if len(message) >= 55:
                                do_send(None, message)	# relay to all nodes
                                print 'beat message', repr(message), 'from', remote_name[s]
                                message = message[55:]
                            else:
                                print 'beat message expected 55 but has', len(message), 'bytes'
                                break
                        elif message[0:1] == 'c':
                            if len(message) >= 24:
                                node, timestamp, channel_data = struct.unpack_from('>BQ14s', message, 1)
                                timestamp /= 1000.0		# convert from milliseconds
                                #print 'node', node, 'channel data', repr(channel_data), 'at', timestamp

                                amplitude_sum = get_external_amplitude_sum(channel_data)
                                if amplitude_sum > 0: # otherwise error reading channel_data
                                    esitmated_ring_number, mean_intensity = analyze_beat(node, amplitude_sum, timestamp)

                                    show_mode = get_show_mode()
                                    if show_mode == 1 or show_mode == 3: # not during meditaiton
                                        check_art_car_status(esitmated_ring_number, mean_intensity)
                                message = message[24:]
                            else:
                                print 'channel message expected 24 but has', len(message), 'bytes'
                                break
                        elif message[0:1] == 'm':
                            if len(message) >= 3:
                                mega_number = ord(message[1:2])
                                try:
                                    node_number = mega_to_node_map[mega_number]
                                except KeyError:
                                    if mega_number >= 100:	# mock_mega
                                        node_number = mega_number % 6 + 10
                                    else:
                                        node_number = None
                                print 'mega', mega_number, '( node ', repr(node_number), 'switches', repr(ord(message[2:3])), ') is at', remote_name[s]
                                if node_number != None:
                                    remote_name[s] = 'node %u' % node_number
                                    do_send(s, struct.pack('>cB', 'n', node_number))
                                message = message[3:]
                            else:
                                print 'mega message expected 3 but has', len(message), 'bytes'
                                break
                        elif message[0:1] == 's':
                            # print 'tsunami says:', repr(message), 'from', remote_name[s]
                            msg = message[0:2]
                            if message[1:2] == 'N':
                                size = 0
                            else:	# remove the length byte for local processing
                                size = ord(message[2:3])
                                msg += message[3:3+size]
                            if music.status_update(msg):
                                mega_music.need_drone = True
                            message = message[3+size:]
                        else:
                            print 'received unknown', repr(message), 'from', remote_name[s]
                    if len(message) > 0:
                        print 'discarding', len(message), 'bytes of input'
                else:
                    disconnect(s, 'remote closed')

        for s in writable:
            try:
                next_msg = message_queues[s].get_nowait()
            except Queue.Empty:
                writing.remove(s)
            except KeyError:	# this happens occasionally
                pass
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

        #syncing time across nodes
        if time.time() > next_timesync_sec:
            do_time('time', None)

        #audio commands
        audio_msg = mega_music.tick()
        if audio_msg is not None:
            do_send(None, encapsulated_audio_msg(audio_msg))	# always send to all nodes
            print repr(audio_msg)
            # meditation = mega.meditation
            get_internal_animations_handler().interpret_audio_msg(audio_msg)

        if auto_show and time.time() > last_show_change_sec + TIME_LIMIT:
            auto_show()
            last_show_change_sec = time.time()
            do_show(None, None)

    except KeyboardInterrupt:
        running = False

    except:
        raise				# uncomment for debugging
        print sys.exc_value
        do_disconnect(None, None)	# TODO: be more selective


mute_msg = encapsulated_audio_msg(music.mute())
for s in remote_name:
    s.send(mute_msg)

for s in sources:
    s.close()
