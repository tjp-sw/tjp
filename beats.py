#!/usr/bin/python
import math, numpy, sys, time

# thanks to http://mathcentral.uregina.ca/QQ/database/QQ.09.07/h/martin4.html
XYZ = math.sqrt(3.0) / 2.0	# need a better 3-letter name for this constant
microphone_coordinates = [
    # (x, y) positive values upward and to the right
    ( 0.0,  1.0),	# top
    ( XYZ,  0.5),	# upper right  /\
    ( XYZ, -0.5),	# lower right |  |
    ( 0.0, -1.0),	# bottom       \/
    (-XYZ, -0.5),	# lower left
    (-XYZ,  0.5),	# upper left
    ]

# 1. detect presence and direction of a loud beat
# 2. send beat predictions to the nodes
# 3. return estimated ring number and its amplitude
beat_history = []

TOTAL_RINGS = 72 # could import conductor.py but refraining... seems like a pretty constant number

def analyze_beat(node, intensity, timestamp):
    global beat_history
    NODE = 0
    INTENSITY = 1
    TIMESTAMP = 2

    # prune old data
    # beat history timestamps are roughly oldest first
    keeper = time.time() - 30	# 30 seconds ago
    for i in range(0, len(beat_history)):
        if beat_history[i][TIMESTAMP] > keeper:
            if i > 0:
                beat_history = beat_history[i:]
                # print 'removed', i, 'entries from beat_history leaving', len(beat_history)
            break

    beat_history += [(node, intensity, timestamp)]	# a list of 3-tuples

    # thanks to Ben at https://stackoverflow.com/questions/1400213/3d-least-squares-plane
    tmp_A = []
    tmp_b = []
    for i in range(0, len(beat_history)):
        x, y = microphone_coordinates[beat_history[i][NODE] % 6]	# keep mock node numbers in bounds
        tmp_A.append([x, y, 1])
        tmp_b.append(beat_history[i][INTENSITY])
    b = numpy.matrix(tmp_b).T
    A = numpy.matrix(tmp_A)
    try:
        fit = (A.T * A).I * A.T * b
    except:
        # print sys.exc_value	# usually "Singular matrix" when the data doesn't define a plane
        return None, None
    else:
        # print 'solution:', 'z = %f x + %f y + %f' % (fit[0], fit[1], fit[2])
        # errors = b - A * fit
        # print 'residual:', numpy.linalg.norm(errors)
        # print 'errors:'
        # print errors

        # Compute the angle in the x/y plane of the steepest slope up
        # the z axis of the plane we just found.  This is the
        # direction of the loudest sound source (presumably an art
        # car).  0 is along the y axis increasing and radians increase
        # going clockwise as seen from above (pi/2 radians is along
        # the x axis increasing).
        if fit[0] == 0:					# no division by zero, please
            if fit[1] > 0:
                direction_radians = 0.0			# along y axis positive
                # print "up"
            else:
                direction_radians = math.pi		# along y axis negative
                # print "down"
        else:
            FULL_CIRCLE = math.pi * 2.0
            direction_radians = FULL_CIRCLE + math.pi * 0.5 - math.atan(fit[1] / fit[0])
            if fit[0] < 0:				# toward negative side of x axis
                direction_radians += math.pi
            if direction_radians >= FULL_CIRCLE:
                direction_radians -= FULL_CIRCLE	# normalize

        amplitude = fit[0]*fit[0] + fit[1]*fit[1]
        esitmated_ring_number = int(math.degrees(direction_radians) / (360 / TOTAL_RINGS))
        # print 'art car direction = %.2f degrees, ring num = %d, amplitude = %.1f' % (math.degrees(direction_radians), esitmated_ring_number, amplitude)
        
        # print 'estimated ring number = %i' % esitmated_ring_number

        return esitmated_ring_number, amplitude

if False:	# debug art car direction computation
    fake_time = int(time.time()) - 5
    while fake_time < time.time():
        beat_history += [(0, 50, fake_time)]
#        beat_history += [(1, 50, fake_time)]
#        beat_history += [(2, 50, fake_time)]
#        beat_history += [(3, 50, fake_time)]
        beat_history += [(4, 50, fake_time)]
        beat_history += [(5, 70, fake_time)]
        fake_time += 1
    analyze_beat(*beat_history.pop())
    sys.exit(0)
