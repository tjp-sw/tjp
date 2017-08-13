from shows import *
from plyru import lrucache
from sys import maxint

ART_CAR_DETECTION_DEBUG = True

# TODO utilize a LRU to handle evictions from the ring_to_hello_animation's
# cache for me. With six nodes sending info should have a max size of 6.
# Will be assuming nodes send channel information in the same intervals.

class ArtCarHandler:
    'a class to track art car visits'

    def __init__(self, duration=ART_CAR_HELLO_DURATION, threshold=ART_CAR_AMPLITUDE_THRESHOLD):
        self.duration_threshold = duration
        self.amplitude_threshold = threshold
        self.art_car = -1
        self.art_car_takover = False
        # a dictionary with key of ring_num and value of the current hello animation playing
        self.ring_to_hello_animation = lrucache(6)
        # a dictionary with flipped key value pairs from above
        self.hello_animation_to_ring = lrucache(6)
        # a dictionary holding duration of artcar presence per ring
        self.ring_to_animation_start_time = lrucache(6)
        # list of rings to stop hello animations on
        self.rings_to_stop_hello_animation = []

        self.flip_mock = False
        self.mock = False

    # returns the oldest ring still above threshold amplitude
    def get_oldest_ring(self):
        # NOTE because I know the most rings we will have for
        # now is 6 this computation is not too costly. otherwise
        # I would use a sorted DLL or similar to audio events queue

        oldest_time = maxint
        oldest_ring = -1
        for ring, start_time in self.ring_to_animation_start_time.items():
            if start_time < oldest_time:
                oldest_ring = ring
                oldest_time = start_time

        return oldest_ring


    def get_ring_animation(self, ring_num):
        return self.ring_to_hello_animation(ring_num)

    # Returns ring number of detected art car
    # Also mutates a dictionary of rings as keys and value containing the hello animation being shown
    def handle_amplitude_info(self, ring_num, amplitude):
        if ring_num is None or amplitude is None:
            if self.flip_mock:
                ring_num = 56
                amplitude = 30
            else:
                ring_num = 56
                amplitude = 230

        if ART_CAR_DETECTION_DEBUG:
            print "ring num %i reporting amplitue %i" % (ring_num, amplitude)

        if amplitude > self.amplitude_threshold:
            # check if already tracking ring_num
            # print "greater than %i" % self.amplitude_threshold
            if ring_num in self.ring_to_hello_animation:
                # already detected... check time threshold
                art_car_detected_seconds = time.time() - self.ring_to_animation_start_time[ring_num]

                if ART_CAR_DETECTION_DEBUG:
                    print "ring %i has been tracked for %i" % (ring_num, art_car_detected_seconds)

                if int(art_car_detected_seconds) > int(self.duration_threshold):
                    if self.art_car is NO_ART_CAR:
                        print "tracked for greater than %i" % self.duration_threshold
                        self.art_car = ring_num
                        # HELP set edm animation here or further up in brain's check_art_car_status?
                            # Setting further up
                        print "art car ring in shows " + str(self.art_car)

                        if ART_CAR_DETECTION_DEBUG:
                            print "art car pass ART_CAR_HELLO_DURATION triggering edm animations on structure"
                    if int(art_car_detected_seconds) > int(self.duration_threshold + 10):
                        self.flip_mock = True
            else:
                # give hello animation & update dictionaries
                self.give_suitable_hello_animation(ring_num)
                self.ring_to_animation_start_time[ring_num] = time.time()

                if ART_CAR_DETECTION_DEBUG:
                    print "ring_num %i has met art car hello threshold level. given hello %i" % (ring_num, self.ring_to_hello_animation[ring_num])

            return ring_num
        else:
            # check if was tracking
            if ring_num in self.ring_to_animation_start_time:
                # check if that ring was triggering edm animations
                if self.art_car == ring_num:
                    # turn off edm animations for art car
                    self.art_car = NO_ART_CAR

                    print "turning off art car edm animation triggered by ring %i \
                            for %i seconds and clearing art car tracking caches" % (ring_num, int(time.time() - self.ring_to_animation_start_time[ring_num]))
                    show_parameters[SEVEN_PAL_BEAT_PARAM_START] = 0

                    # clearing art car tracking cache to start from scratch again
                    self.ring_to_animation_start_time.clear()
                    self.hello_animation_to_ring.clear()
                    self.ring_to_hello_animation.clear()

                    return None

                else:
                    # mark ring running hello animation to stop hello animation
                    self.rings_to_stop_hello_animation.append(ring_num)

                    # remove from tracking
                    self.ring_to_animation_start_time.pop(ring_num)
                    self.hello_animation_to_ring.pop(self.ring_to_hello_animation[ring_num])
                    self.ring_to_hello_animation.pop(ring_num)

        return -1

    # TODO add random element for now return the first avaliabe hello animation
    def give_suitable_hello_animation(self, ring_num):
        # check if neighbor
        for i in self.ring_to_hello_animation.keys():
            if abs(ring_num - i) == 1: # direct next door nieghbor

                # copy animation to new ring
                self.ring_to_hello_animation[ring_num] = self.ring_to_hello_animation[i]

                # will remove from hello -> ring dictionary
                hold_hello = self.ring_to_hello_animation[i]
                del self.ring_to_hello_animation[i]

                # mark old 'closest art car ring' to stop hello animation
                self.rings_to_stop_hello_animation.append(i)

                # retain original start hello time
                self.ring_to_animation_start_time[ring_num] = self.ring_to_animation_start_time[i]
                del self.ring_to_animation_start_time[i]

                # transfer animation to ring info
                self.hello_animation_to_ring[hold_hello] = ring_num

                return self.ring_to_hello_animation[ring_num]

        # currently grabbing first avaliable hello animaiton
        # TODO give only designated hello animations
        for i in range(0, NUM_7_COLOR_ANIMATIONS): # using edm animations for hello animations
            if i not in self.hello_animation_to_ring:
                # add to dictionary to keep track / ensure uniqueness {animation:ring_num}
                self.hello_animation_to_ring[i] = ring_num
                self.ring_to_hello_animation[ring_num] = i

                return i

        # if here all hello animations are used.
        # I have a feeling this should never really happen... just printing for now
        print "RAN OUT OF UNIQUE HELLO ART CAR ANIMAITONS... this should be pretty \
        rare or a sign of incorreclty calibrated thresholding for artcar detection"


if False: # for testing... I like Jeff's test format ha.
    ac = ArtCarHandler(ART_CAR_HELLO_DURATION, ART_CAR_AMPLITUDE_THRESHOLD)
    a = 300
    r = 30
    for i in range (0,15):
        ac.handle_amplitude_info(r, a)
        r += 2
        a += 2
        print ac.ring_to_animation_start_time
        print ac.ring_to_animation_start_time.size()
        print ac.get_oldest_ring()
