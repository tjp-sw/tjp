import random
from datetime import datetime, timedelta
from brain import do_send
import songs


# Just random....this signal is coming from the touchpad which is not written yet.
def panel_touched():
    chance = random.randint(0, 400000)
    if chance == 0:
        return True
    return False


def status_update(message):
    print (message)


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

    def send(self):
        empty_msg = True
        if self.command == 3:
            empty_msg = False
        for ch in self.channels:
            if ch > 0:
                empty_msg = False
        if empty_msg is False:
            ctrl_msg = 'a' + ';'.join([str(self.node), str(self.command), str(self.field2),
                                      ','.join(str(ch) for ch in self.channels)]) + ';'
            print(ctrl_msg)
            do_send(self.node, ctrl_msg)

    def play(self, channels, node=0, looping=0):
        self.node = node
        self.command = 1
        self.field2 = looping
        self.channels = channels
        self.send()

    def set_volume(self, channels, fade_speed=2000, node=0):
        self.node = node
        self.command = 2
        self.field2 = fade_speed
        self.channels = channels
        self.send()

    def mute(self, node=0):
        self.node = node
        self.command = 3
        self.send()


class Music:
    def __init__(self):
        self.played_low = -1
        self.played_mid = datetime.min
        self.checked_high = datetime.min
        self.played_high = datetime.min

    def tick(self):
        if self.played_low != datetime.today().weekday():  # Changes at midnight. This probably should be changed
            low = songs.find_low()
            low_msg = ControlMessage()
            low_msg.play([low], looping=1)
            self.played_low = datetime.today().weekday()

        msg = [0] * 4
        if self.played_mid <= (datetime.now() - timedelta(minutes=1)):
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

        tick_msg = ControlMessage()
        tick_msg.play(msg)
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
