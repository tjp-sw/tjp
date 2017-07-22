import random
from datetime import datetime, timedelta
import songs


# Just random....this signal is coming from the touchpad which is not written yet.
def panel_touched():
    chance = random.randint(0, 400000)
    if chance == 0:
        return True
    return False


def status_update(message):
    print (message)

"""
# sends a control message to the mega
class ControlMessage:
    def __init__(self, node=0, command=1, field2=0, channels=None):
        self.node = node
        self.command = command
        self.field2 = field2
        if channels is not None:
            self.channels = channels
        else:
            self.channels = [0] * 7
"""


def send_music(node=0, command=1, field2=0, channels=None):
    empty_msg = True
    if command == 3:
        empty_msg = False
    if channels is not None:
        for ch in channels:
            if ch > 0:
                empty_msg = False
    if empty_msg is False:
        ctrl_msg = 'a' + ';'.join([str(node), str(command), str(field2),
                                  ','.join(str(ch) for ch in channels)]) + ';'
        print(ctrl_msg)
        return (node, ctrl_msg)
    return (None, None)


def play(channels, node=0, looping=0):
    return send_music(node, 1, looping, channels)


def set_volume(channels, fade_speed=2000, node=0):
    return send_music(node, 2, fade_speed, channels)


def mute(node=0):
    return send_music(node, 3)

#Can only return one command per tick....need to update looping
class Music:
    def __init__(self):
        self.played_low = -1
        self.played_mid = datetime.min
        self.checked_high = datetime.min
        self.played_high = datetime.min

    def tick(self):
        if self.played_low != datetime.today().weekday():  # Changes at midnight. This probably should be changed
            low = songs.find_low()
            self.played_low = datetime.today().weekday()
            return play([low], looping=1)

        msg = [0] * 4
        if self.played_mid <= (datetime.now() - timedelta(seconds=30)):
            msg[1] = songs.find_mid()
            self.played_mid = datetime.now()

        if self.checked_high <= (datetime.now() - timedelta(minutes=1)):
            play_chance = random.randint(0, 4)
            if play_chance == 0 or \
               self.played_high <= (datetime.now() - timedelta(minutes=5)):
                msg[2] = songs.find_high()
                self.played_high = datetime.now()
            self.checked_high = datetime.now()

        if panel_touched():
            msg[3] = songs.find_high()

        return play(msg)

"""
music = Music()

while True:
    music.tick()

  

#Set theme
theme_list= ['RED', 'ORANGE', 'YELLOW', 'GREEN', 'BLUE', 'PURPLE', 'WHITE']
RED= 0
ORANGE= 1
YELLOW= 2
GREEN= 3
BLUE= 4
PURPLE= 5
WHITE= 6

"""
