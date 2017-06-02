#!/usr/bin/python
import socket, random, struct, sys, time

# declare global variables with bogus values
remote = None
server = None
mega_number = None
loop_start_time_msec = None
unix_epoch_msec = None	# unix time this process started
epoch_msec = None	# process start time as received from brain
network_data = None

def now_sec(when_msec):
    return long((epoch_msec + when_msec) / 1000)

def millis():
    return long(time.time() * 1000.0 - unix_epoch_msec)

def process_commands():
    global network_data

    while len(network_data) > 0:
        print 'mega', mega_number, 'looking at', repr(network_data)
        size = 1
        command = network_data[0:1]
        if command == 'p':
            size += 1	# unsigned 8-bit integer

            if len(network_data) >= size:
                print 'new program %u' % struct.unpack_from('>B', network_data, 1)
            else:
                print 'command', command, 'needs', size, 'bytes but only', len(network_data), 'available'
                break
        elif command == 'r':
            global remote

            print 'mega', mega_number, 'closing %s:%d' % remote.getsockname(), 'to %s:%d' % remote.getpeername()
            remote.close()
            remote = None
            network_data = None
            time.sleep(10)
            break
        elif command == 't':
            global epoch_msec
            size += 8	# unsigned 64-bit integer

            if len(network_data) >= size:
                epoch_msec = struct.unpack_from('>Q', network_data, 1)[0] / 1000 - loop_start_time_msec
                print 'mega', mega_number, 'time set at %u seconds' % now_sec(millis())
            else:
                print 'command', command, 'needs', size, 'bytes but only', len(network_data), 'available'
                break
        else:
            print 'mega', mega_number, 'received unknown command', repr(command)
        network_data = network_data[size:]

def setup():
    global epoch_msec, mega_number, network_data, remote, server, unix_epoch_msec

    unix_epoch_msec = time.time() * 1000.0
    epoch_msec = long(0)

    mega_number = int(100 * random.random() + 100)	# random number from 100 through 199
    server = ('localhost', 3528)
    remote = None
    network_data = None

def loop():
    global loop_start_time_msec, network_data, remote

    loop_start_time_msec = None
    try:
        if remote:
            message = remote.recv(1024)
            if message:
                loop_start_time_msec = millis()
                network_data += message
                process_commands()
            else:
                print 'mega', mega_number, 'closing %s:%d' % remote.getsockname(), 'to %s:%d' % remote.getpeername()
                remote.close()
                remote = None
                network_data = None
                time.sleep(10)
        else:
            try:
                time.sleep(random.random())
                remote = socket.create_connection(server)
                network_data = ''
                print 'mega', mega_number, 'connected %s:%d' % remote.getsockname(), 'to %s:%d' % remote.getpeername()
                remote.sendall(struct.pack('>cB', 'm', mega_number))
            except:
                print 'mega', mega_number, sys.exc_value, 'by %s:%d' % server
                time.sleep(10)

    except KeyboardInterrupt:
        raise

    except:
        print 'mega', mega_number, sys.exc_value, 'by %s:%d' % server
        time.sleep(1)

setup()
while 1:
    try:
        loop()

    except KeyboardInterrupt:
        break

if remote:
    remote.close()
print '\nmega', mega_number, 'over and out'
