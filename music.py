import random, struct
from datetime import datetime, timedelta, time
import sounds, shows
#import time as epoch_time

DEBUG = 0

# Just random....this signal is coming from the touchpad which is not written yet.
def panel_touched():
    chance = random.randint(0, 400000)
    if chance == 0:
        return True
    return False

def manual_meditation(med):
    meditation_num = int(med)
    if meditation_num in range(1, 14):
        Music.meditation = True
        if (meditation_num % 2):  # odd
            shows.set_show_mode(shows.SUNRISE)
        else:
            shows.set_show_mode(shows.SUNSET)
            
        return play([0, 4000+meditation_num])
    return None


def check_meditation(node=0):
    return send_music(node, 4)


# --------------------Receives information from the Tsunami--------------------------------
# Returns True if the tsunami is playing the looping drone, False if it is not.
def status_update(message):
    message = message[1:]
    if message[0] == 'E':
        try:
            this_sound = int(message[1:])
            if DEBUG:
                print "Sound ending: ", this_sound
            if this_sound in sounds.ALL_MIDS:
                Music.no_mid = datetime.now()
            if this_sound in sounds.ALL_HIGHS:
                Music.no_high = datetime.now()
            if this_sound in sounds.MEDITATIONS_SOUNDS:
                print "Meditation Finished"
                Music.meditation = False
                # if datetime.now().time() > time(hour=19):
                #    shows.set_show_mode(shows.NIGHT)
                #else:
                #    shows.set_show_mode(shows.DAY)
                return True
        except ValueError:
            print"Ending sound ValueError " + str(len(message))
            print message[1:]

    elif message[0] == 'N':
        if DEBUG:
            print "Need Drone"
        return True
    elif message[0] == 'P':
        print "Still Playing: " + message[1:]
        try:
            if int(message[1:]) == 3000 and datetime.now().weekday() != 0:
                return True
        except:
            pass
    return False


# -------------------Formats information to send to the Tsunami---------------------------
def send_music(node=0, command=1, field2=0, channels=None):
    empty_msg = True
    if command == 3:
        return 'a' + "0;3;"
    if channels is not None:
        for ch in channels:
            if ch > 0:
                empty_msg = False
                break
    if empty_msg is False:
        ctrl_msg = 'a' + ';'.join([str(node), str(command), str(field2),
                                   ','.join(str(ch) for ch in channels)]) + ';'
        # print 'audio command:', ctrl_msg
        return ctrl_msg
    return None


def check_drone(node=0):
    return send_music(node, 5)


def mute(node=0):
    return send_music(node, 3)


def play(channels, node=0, looping=0):
    return send_music(node, 1, looping, channels)


def set_volume(channels, fade_speed=2000, node=0):
    return send_music(node, 2, fade_speed, channels)


# Returns one command per tick
class Music:
    meditation = False
    no_low = datetime.min
    no_mid = datetime.min
    no_high = datetime.min

    # show_mode = 0

    def __init__(self):
        # self.meditation = False
        self.need_drone = False
        self.played_low = datetime.min
        self.played_mid = datetime.min
        self.checked_high = datetime.min
        self.played_high = datetime.min
        self.check_drone = datetime.max
        self.checked_meditation = datetime.min
        self.low_wait = random.randint(3, 6)
        self.mid_wait = random.randint(5, 25)
        self.high_wait = random.randint(20, 35)


    def mute(self, node=0):
        mute(node)

    def tick(self, silent=False):
        now_time = datetime.now()
        if DEBUG > 1:
            print now_time
        if now_time.day == 27:
            bm_day = 8
        else:
            bm_day = now_time.weekday()

        # Check if mediation has ended
        if Music.meditation:
            if DEBUG > 1:
                print "meditation"
            if self.checked_meditation <= (now_time - timedelta(minutes=1)):
                self.checked_meditation = now_time
                return check_meditation()
            return None

        # Uncomment to silence sound for art cars
        # 8/24/17 RJS actually not valid art car logic anymore as param 29 is used for lots more than art car takeover
        """
        if shows.show_parameters[29] != shows.NO_ART_CAR:
            if DEBUG:
                print "silent"
            return None
        """
        # Meditation Logic
        this_meditation = sounds.play_meditation(now_time)
        if this_meditation is None:
            if DEBUG > 1:
                print "no meditation"
            Music.meditation = False
            if time(6, 25) <= now_time.time() <= time(19, 25):
                if DEBUG > 1:
                    print "in music.py, setting day on", shows.bm_day_index
                    #shows.set_show_mode(shows.DAY)
            else:
                # shows.set_show_mode(shows.NIGHT)
                if DEBUG > 1:
                    print "in music.py, setting night on", shows.bm_day_index
        else:
            if DEBUG > 1:
                print "setting meditation status"
            Music.meditation = True
            if this_meditation % 2:  # odd
                print "in music.py, setting sunrise on", shows.bm_day_index
                #shows.set_show_mode(shows.SUNRISE)
            else:  # even
                print "in music.py, setting sunset on", shows.bm_day_index
                shows.set_show_mode(shows.SUNSET)
            if now_time.weekday() == 1:
                return play([0, this_meditation, 3999])
            return play([0, this_meditation])

        # Drone Logic
        if self.need_drone:
            print "Setting Drone"
            self.need_drone = False
            return "a0;6;" + str(bm_day) + ";"

        # Soundscape Compilation Logic
        if bm_day < 8 :
            msg = [0] * 4
            if self.played_low < now_time - timedelta(minutes=self.low_wait):
                self.low_wait = random.randint(3, 5)
                low = sounds.find_low()
                self.played_low = now_time
                self.check_drone = now_time
                msg[0] = low

            if self.played_mid <= (now_time - timedelta(seconds=self.mid_wait)):
                self.played_mid = now_time
                self.mid_wait = random.randint(5, 25)
                msg[1] = sounds.find_mid()

            if self.played_high <= (now_time - timedelta(seconds=self.high_wait)):
                self.high_wait = random.randint(20, 35)
                self.played_high = now_time
                msg[2] = sounds.find_high()

            """
            elif self.check_drone < now_time - timedelta (minutes= 1):
                self.check_drone = now_time
                if DEBUG > 1:
                    print "check_drone"
                return check_drone()
            if Music.no_mid <= (now_time - timedelta(seconds=self.mid_wait)):
                msg[1] = sounds.find_mid()
                self.mid_wait = random.randint(5, 30)
                Music.no_mid = datetime.max
            if Music.no_high <= (now_time - timedelta(seconds=self.high_wait)):
                    msg[2] = sounds.find_high()
                    self.high_wait = random.randint(30,60)
                    Music.no_high = datetime.max
            """

            if panel_touched():
                msg[3] = sounds.find_high()

            if DEBUG > 1:
                print msg
            return play(msg)
        else:
            return None
