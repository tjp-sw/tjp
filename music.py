import random, struct
from datetime import datetime, timedelta
import sounds  # , shows


# Just random....this signal is coming from the touchpad which is not written yet.
def panel_touched():
    chance = random.randint(0, 400000)
    if chance == 0:
        return True
    return False

def manual_meditation(med):
    meditation_num = int(med)
    if meditation_num in range(1,14):
        Music.meditation = True
        return play([0, 4000+meditation_num])
    return None

def status_update(message):
    message = message[1:]
    if message[0] == 'E':
        print "Sound ending: " + message[1:]
        Music.meditation = True
        if int(message[1:]) > 4000:
            print "Meditation Finished"
            Music.meditation = False
    elif message[0] == 'N':
        print "Need Drone"
        return True
    elif message[0] == 'P':
        print "Still Playing: " + message[1:]
    return False


def send_music(node=0, command=1, field2=0, channels=None):
    empty_msg = True
    if command == 3:
        return struct.pack('>cB', 'a', 4) + "0;3;" 
    if channels is not None:
        for ch in channels:
            if ch > 0:
                empty_msg = False
                break
    if empty_msg is False:
        ctrl_msg = 'a' + ';'.join([str(node), str(command), str(field2),
                                  ','.join(str(ch) for ch in channels)]) + ';'
        # print 'audio command:', ctrl_msg
        # hope the length is less than 256
        return struct.pack('>cB', ctrl_msg[0:1], len(ctrl_msg[1:])) + ctrl_msg[1:]
    return None


def check_meditation(node=0):
    return send_music(node, 4)

def check_drone(node=0):
    return send_music(node, 5)


def mute(node=0):
    return send_music(node, 3)


def play(channels, node=0, looping=0):
    return send_music(node, 1, looping, channels)


def set_volume(channels, fade_speed=2000, node=0):
    return send_music(node, 2, fade_speed, channels)

#TODO: set shows.show_mode
# Can only return one command per tick
class Music:
    meditation = False
    #show_mode = 0 

    def __init__(self):
        # self.meditation = False
        self.played_low = datetime.min
        self.played_mid = datetime.min
        self.checked_high = datetime.min
        self.played_high = datetime.min
        self.drone = datetime.min
        self.drone_count = 0  # Todo: change drone?
        self.checked_meditation = datetime.min

    def tick(self, silent=False):
        now_time = datetime.now()
        if Music.meditation:
            if self.checked_meditation <= (now_time - timedelta(minutes=1)):
                self.checked_meditation = now_time
                return check_meditation()
            return None
        if silent:
            return None

        # Todo: Play meditations manually
        this_meditation = sounds.play_meditation()
        if this_meditation is not None:
            self.meditation = True
            return play([0, this_meditation])

        msg = [0] * 4  # Todo: Loop channel 0 on the mega
        if self.played_low != datetime.today().weekday():  # Todo: change drone after meditation
            low = sounds.find_low()
            self.played_low = datetime.today().weekday()
            self.drone = now_time
            msg[0] = low
        if self.drone < now_time - timedelta (minutes= 1):
            self.drone = now_time
            return check_drone()
        if self.played_mid <= (now_time - timedelta(seconds=30)):
            msg[1] = sounds.find_mid()
            self.played_mid = now_time

        if self.checked_high <= (now_time - timedelta(seconds=30)):
            play_chance = random.randint(0, 4)
            if play_chance == 0 or \
               self.played_high <= (now_time - timedelta(minutes=2)):
                msg[2] = sounds.find_high()
                self.played_high = now_time
            self.checked_high = now_time

        if panel_touched():
            msg[3] = sounds.find_high()

        return play(msg)

