#!/usr/bin/python
import socket, random, sys, time

# declare global variables with bogus values
remote = False
server = False
id = False
unix_epoch = False	# unix time this process started

def millis():
    return long((time.time() - unix_epoch) * 1000.0)

def setup():
    global id, remote, server, unix_epoch

    unix_epoch = time.time()

    id = '%04.0f' % (10000 * random.random())	# 4-digit (with leading zeroes) random number
    server = ('localhost', 3528)
    remote = False

def loop():
    global remote

    try:
        if remote:
            message = remote.recv(1024)
            if message and message != 'reconnect':
                print 'client received', repr(message)
                remote.sendall('acknowledged: ' + message)
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
                remote.sendall('I am node %s at %d seconds' % (id, millis() / 1000))
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
