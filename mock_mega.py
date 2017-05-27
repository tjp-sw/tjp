#!/usr/bin/python
import socket, random, sys, time

MAX_PARAMS = 2		# allow up to 2 command parameters

# declare global variables with bogus values
remote = False
server = False
id = False
loop_start_time_msec = False
unix_epoch_msec = False	# unix time this process started
epoch_msec = False	# process start time as received from brain

def now_sec(when_msec):
    return long((epoch_msec + when_msec) / 1000)

def atol(string):
    reference_value = long(string)
    value = long(0)
    for digit in string:
        value *= 10
        value += int(digit)
    if value != reference_value:
        print 'node', id, 'atol got', value, 'instead of', reference_value
    return reference_value

def millis():
    return long(time.time() * 1000.0 - unix_epoch_msec)

def do_command(message):
    # parse out a command and an array of up to MAX_PARAMS parameters
    parameters = []
    parameter_count = 0
    separator = message.find(' ')
    if separator >= 0:
        command = message[:separator]
        remaining = message[separator + 1:]
        limit = MAX_PARAMS - 1
        while parameter_count < limit:
            separator = remaining.find(' ')
            if separator < 0:
                break
            parameters.append(remaining[:separator])
            parameter_count += 1
            remaining = remaining[separator + 1:]
        parameters.append(remaining)
        parameter_count += 1
    else:					# no whitespace
        command = message
    print 'node', id, "command parsed as '" + command + "'", repr(parameters)

    if command == 'time' and parameter_count == 1:
        global epoch_msec

        epoch_msec = atol(parameters[0]) / 1000 - loop_start_time_msec
        print 'node', id, 'time set at %u seconds' % now_sec(millis())
    else:
        print 'node', id, 'received', command, repr(parameters)

def setup():
    global id, remote, server, unix_epoch_msec

    unix_epoch_msec = time.time() * 1000.0
    epoch_msec = long(0)

    id = '%04.0f' % (10000 * random.random())	# 4-digit (with leading zeroes) random number
    server = ('localhost', 3528)
    remote = False

def loop():
    global loop_start_time_msec, remote

    loop_start_time_msec = long(-1)		# not valid
    try:
        if remote:
            message = remote.recv(1024)
            if message and message != 'reconnect':
                loop_start_time_msec = millis()
                do_command(message)
                remote.sendall('acknowledged: ' + message + (' at %u' % now_sec(loop_start_time_msec)))
            else:
                print 'node', id, 'closing %s:%d' % remote.getsockname(), 'to %s:%d' % remote.getpeername()
                remote.close()
                remote = False
                time.sleep(10)
        else:
            try:
                time.sleep(random.random())
                remote = socket.create_connection(server)
                print 'node', id, 'connected %s:%d' % remote.getsockname(), 'to %s:%d' % remote.getpeername()
                remote.sendall('I am node %s at %u seconds' % (id, now_sec(millis())))
            except:
                print 'node', id, sys.exc_value, 'by %s:%d' % server
                time.sleep(10)

    except KeyboardInterrupt:
        raise

    except:
        print 'node', id, sys.exc_value, 'by %s:%d' % server

setup()
while 1:
    try:
        loop()

    except KeyboardInterrupt:
        break

if remote:
    remote.close()
print '\nnode', id, 'over and out'
