#!/usr/bin/python
import Queue	# thread safe
import select, socket, string, struct, sys, time

mega_to_node_map = {
    5: 2,
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

# send the same message to all TCP connections
def do_send(ignored, message):
    global message_queues, writing
    # print 'sending', repr(message)
    for s in message_queues:
        message_queues[s].put(message)
        if s not in writing:
            writing.append(s)

# almost the same as do_send() above, but send the very latest
# timestamp to each remote or just the specified remote
def do_time(socket, neglected):
    global message_queues, writing
    if socket == 'time':
        list = message_queues
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
    'SetAnimation':	(do_simple, 'program', None),
#    'SetDynAnimation':	do_unimplemented,
    'AllLEDoff':	(do_simple, 'program', '0'),
#    'CheckHandStat':	do_unimplemented,
#    'CheckAudioIn':	do_unimplemented,
    'disconnect':	(do_disconnect, None, None),
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
        # could generate writing list here from nonempty message_queues
        readable, writable, oops = select.select(sources, writing, sources)

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
                    if message[0:1] == 'm':
                        mega_number = ord(message[1:2])
                        try:
                            node_number = mega_to_node_map[mega_number]
                        except KeyError:
                            node_number = None
                        print 'mega', mega_number, '( node ', repr(node_number), ') is at', remote_name[s]
                        if node_number:
                            do_simple('node', chr(ord('0') + node_number))
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

    except KeyboardInterrupt:
        running = False

    except:
        print sys.exc_value
        do_disconnect(None, None)	# TODO: be more selective

for s in sources:
    s.close()
