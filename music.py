import random
from datetime import datetime, timedelta
import sounds


# Just random....this signal is coming from the touchpad which is not written yet.
def panel_touched():
    chance = random.randint(0, 400000)
    if chance == 0:
        return True
    return False


def status_update(message):
    message = message[1:]
    print (message)


def send_music(node=0, command=1, field2=0, channels=None):
    empty_msg = True
    if command == 3:
        # empty_msg = False
        return 'a' + '0' + ';' + '3' + ';'
        # channels= []
    if channels is not None:
        for ch in channels:
            if ch > 0:
                empty_msg = False
    if empty_msg is False:
        ctrl_msg = 'a' + ';'.join([str(node), str(command), str(field2),
                                  ','.join(str(ch) for ch in channels)]) + ';'
        # print 'audio command:', ctrl_msg
        return ctrl_msg
    return None


def play(channels, node=0, looping=0):
    return send_music(node, 1, looping, channels)


def set_volume(channels, fade_speed=2000, node=0):
    return send_music(node, 2, fade_speed, channels)


def mute(node=0):
    return send_music(node, 3)


# Can only return one command per tick....need to update looping
class Music:
    def __init__(self):
        self.meditation = False
        self.played_low = datetime.min
        self.played_mid = datetime.min
        self.checked_high = datetime.min
        self.played_high = datetime.min
        self.drone = datetime.min
        self.drone_count = 0  # Todo: change drone?

    def tick(self):
        if self.meditation:
            return None
        # Todo: Turn off meditation
        # Todo: Play meditations manually
        meditation = sounds.play_meditation()
        if meditation is not None:
            self.meditation = True
            return play([0, meditation])

        msg = [0] * 4  # Todo: Loop channel 0 on the mega
        if self.played_low != datetime.today().weekday():  # Todo: change drone after meditation
            low = sounds.find_low()
            self.played_low = datetime.today().weekday()
            msg[0] = low

        if self.played_mid <= (datetime.now() - timedelta(seconds=2)):
            msg[1] = sounds.find_mid()
            self.played_mid = datetime.now()

        if self.checked_high <= (datetime.now() - timedelta(seconds=30)):
            play_chance = random.randint(0, 4)
            if play_chance == 0 or \
               self.played_high <= (datetime.now() - timedelta(minutes=2)):
                msg[2] = sounds.find_high()
                self.played_high = datetime.now()
            self.checked_high = datetime.now()

        if panel_touched():
            msg[3] = sounds.find_high()

        return play(msg)

