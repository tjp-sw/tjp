#!/usr/bin/python
import Queue	# thread safe
import select, socket, string, sys, time

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
                print 'listening on', s.getsockname()
            else:
                print 'connected to', s.getpeername()

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

def do_unimplemented(cmd, param):
    if param:
        do_send(False, '%s %s' % (cmd, param))
    else:
        do_send(False, cmd)

control_messages = {
    'SetAllAudio':	do_unimplemented,
    'SetAudioCh':	do_unimplemented,
    'SetVolCh':		do_unimplemented,
    'MuteAllAudio':	do_unimplemented,
    'SetAnimation':	do_unimplemented,
    'SetDynAnimation':	do_unimplemented,
    'AllLEDoff':	do_unimplemented,
    'CheckHandStat':	do_unimplemented,
    'CheckAudioIn':	do_unimplemented,
    'disconnect':	do_disconnect,
    'list':		do_list,
    'quit':		do_quit,
    'send':		do_send,
    }

# listen for TCP connections on the specified port
listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
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

# print a message and clean up resources associated with an open TCP connection
def disconnect(socket, msg):
    print msg, '%s:%d' % socket.getpeername()
    if socket in writing:
        writing.remove(socket)
    sources.remove(socket)
    socket.close()	# shutdown() is too abrupt, do a graceful close()
    del message_queues[socket]

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
                    parameters = False
                try:
                    function = control_messages[command]
                    function(command, parameters)
                except KeyError:
                    print sorted(control_messages.keys())	# unrecognized
            elif s is listener:
                remote, addr = s.accept()
                remote.setblocking(0)
                sources.append(remote)			# remember this connection
                message_queues[remote] = Queue.Queue()	# create outgoing FIFO queue
                print 'connection from %s:%d' % addr	# addr is the same as remote.getpeername()
            else:
                try:
                    message = s.recv(1024)
                except:
                    print sys.exc_value
                    message = False
                if message:
                    print 'received', repr(message), 'from %s:%d' % s.getpeername()
                else:
                    disconnect(s, 'remote closed')

        for s in writable:
            try:
                next_msg = message_queues[s].get_nowait()
            except Queue.Empty:
                writing.remove(s)
            else:
                # print 'sending', repr(next_msg), 'to %s:%d' % s.getpeername()
                sent = s.send(next_msg)
                unsent = len(next_msg) - sent
                if unsent != 0:
                    print 'failed to send %d bytes to %s' % (unsent, s.getpeername())
                    # Queue module can't push unsent data back to the front of the queue

        for s in oops:
            disconnect(s, sys.exc_value)

    except:
        print sys.exc_value
        do_disconnect(False, False)	# TODO: be more selective

for s in sources:
    s.close()
