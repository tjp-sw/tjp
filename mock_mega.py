#!/usr/bin/python
import socket, random, sys, time

# declare global variables with bogus values
remote = False
server = False
id = False

def setup():
    global id, remote, server

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
                print 'closing %s:%d' % remote.getsockname(), 'to %s:%d' % remote.getpeername()
                remote.close()
                remote = False
                time.sleep(10)
        else:
            try:
                time.sleep(random.random())
                remote = socket.create_connection(server)
                print 'connected %s:%d' % remote.getsockname(), 'to %s:%d' % remote.getpeername()
                remote.sendall('client %s alive on port %d' % (id, remote.getsockname()[1]))
            except:
                print sys.exc_value, 'by %s:%d' % server
                time.sleep(10)

    except KeyboardInterrupt:
        raise

    except:
        print sys.exc_value, 'by %s:%d' % server

setup()
while 1:
    try:
        loop()

    except KeyboardInterrupt:
        break

if remote:
    remote.close()
print '\nbye'
