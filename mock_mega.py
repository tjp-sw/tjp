#!/usr/bin/python
import select, socket, random, struct, sys, time

# declare global variables with bogus values
remote = None
server = None
mega_number = None
node_number = None
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
        if command == 'b':
            size += 54
            if len(network_data) >= size:
                print 'beat:', repr(network_data)
            else:
                print 'command', command, 'needs', size, 'bytes but only', len(network_data), 'available'
                break
        elif command == 'n' or command == 'p':
            size += 1	# unsigned 8-bit integer

            if len(network_data) >= size:
                parameter = struct.unpack_from('>B', network_data, 1)
                if command == 'n':
                    node_number = parameter
                    print 'mega', mega_number, 'is node %u' % node_number
                elif command == 'p':
                    print 'new program %u' % parameter
                else:
                    print 'mega', mega_number, 'neither n nor p'
            else:
                print 'command', command, 'needs', size, 'bytes but only', len(network_data), 'available'
                break
        elif command == 'a':
            size += 1	# unsigned 8-bit integer

            if len(network_data) >= size:
                size += struct.unpack_from('>B', network_data, 1)[0]
                if len(network_data) >= size:
                    print 'mega', mega_number, 'audio', repr(network_data[2:size])
                else:
                    print 'command', command, 'needs', size, 'bytes but only', len(network_data), 'available'
                    break
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
        elif command == 's':
            NUM_PARAMETERS = 41
            NUM_COLORS_PER_PALETTE = 7
            number_of_colors = 3 * NUM_COLORS_PER_PALETTE
            size += NUM_PARAMETERS + number_of_colors
            if len(network_data) >= size:
                start = 1
                params = list(struct.unpack_from('>%uB' % NUM_PARAMETERS, network_data, start))
                start += NUM_PARAMETERS
                colors = list(struct.unpack_from('>%uB' % number_of_colors, network_data, start))
                print 'show params', repr(params), 'colors', repr(colors)
            else:
                print 'command', command, 'needs', size, 'bytes but only', len(network_data), 'available'
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
            remote.sendall('beat, beat, we got the beat yeah we got the beat friend')
        network_data = network_data[size:]

def setup():
    global epoch_msec, mega_number, network_data, remote, server, unix_epoch_msec

    unix_epoch_msec = time.time() * 1000.0
    epoch_msec = long(0)

    mega_number = random.randint(100, 199)
    if len(sys.argv) == 2:
        try:
            n = int(sys.argv[1])
        except:
            print 'not a node number:', sys.argv[1]
        else:
            if 0 <= n and n < 6:
                mega_number = 200 + n	# mega 200-205 -> node 0-5
            else:
                print 'node number', n, 'must be 0-5'
    server = ('localhost', 3528)
    remote = None
    network_data = None

last_beat_msec = int(time.time() * 1000.0)
def loop():
    global last_beat_msec, loop_start_time_msec, network_data, remote

    loop_start_time_msec = None
    try:
        if remote:
            timeout_sec = 5.0 + (last_beat_msec / 1000.0) - time.time()
            if timeout_sec <= 0:
                timeout_sec = 0.01

            readable, writable, oops = select.select([remote], [], [], timeout_sec)
            if len(writable) > 0:
                print 'ignoring unexpected writable file descriptor(s)'
            if len(oops) > 0:
                print 'ignoring unexpected file descriptor(s) in exceptional state'
            if len(readable) > 0:
                if len(readable) != 1:
                    print 'expected exactly 1 readable file descriptor instead of', len(readable)
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
                intensity = mega_number + random.randint(-5, 5)
                last_beat_msec = int(time.time() * 1000.0)
                remote.sendall(struct.pack('>cBQ', 'B', intensity, last_beat_msec))	# send a beat message
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
