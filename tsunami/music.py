import random
from datetime import datetime, timedelta


# Just random....not sure where this signal is coming from
def panel_touched():
    chance = random.randint(0, 300000)
    if chance == 0:
        return True
    return False


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

    def add_channel(self, channel, value):
        self.channels[channel] = value

    def send(self):
        empty_msg = True
        if self.command == 3:
            empty_msg = False
        for ch in self.channels:
            if ch > 0:
                empty_msg = False
        if empty_msg is False:
            ctrl_msg = ';'.join([str(self.node), str(self.command), str(self.field2),
                                 ','.join(str(ch) for ch in self.channels)])
            ctrl_msg += ';'
            print(ctrl_msg)

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


# Stores Music library and returns appropriate songs
class Songs:
    RED_MID = [1]
    RED_HIGH = [2]
    ORANGE_MID = [3]
    ORANGE_HIGH = [4]
    YELLOW_MID = [5]
    YELLOW_HIGH = [6]
    GREEN_MID = [7]
    GREEN_HIGH = [8]
    BLUE_MID = [9]
    BLUE_HIGH = [10]
    PURPLE_MID = [11]
    PURPLE_HIGH = [12]
    WHITE_MID = [13]
    WHITE_HIGH = [14]
    MIDS = [RED_MID, ORANGE_MID, YELLOW_MID, GREEN_MID, BLUE_MID, PURPLE_MID, WHITE_MID]
    HIGHS = [RED_HIGH, ORANGE_HIGH, YELLOW_HIGH, GREEN_HIGH, BLUE_HIGH, PURPLE_HIGH, WHITE_HIGH]
    LOWS = [15, 16, 17]

    def __init__(self, theme=datetime.today().weekday()):
        self.theme = theme
        self.available_lows = [self.LOWS[theme]]

    def find_low(self, this_theme=None):
        if this_theme is None:
            this_theme = self.theme
        return self.LOWS[this_theme]

    def find_mid(self, this_theme=None):
        if this_theme is None:
            this_theme = self.theme

        if datetime.now().hour >= 21:
            all_mids = [song for sublist in self.MIDS for song in sublist]
            return random.choice(all_mids)
        return random.choice(self.MIDS[this_theme])

    def find_high(self, this_theme=None):
        if this_theme is None:
            this_theme = self.theme

        if datetime.now().hour >= 21:
            all_highs = [song for sublist in self.HIGHS for song in sublist]
            return random.choice(all_highs)
        return random.choice(self.HIGHS[this_theme])


class Music:
    songs = Songs()

    def __init__(self):
        self.played_low = -1
        self.played_mid = datetime.min
        self.checked_high = datetime.min
        self.played_high = datetime.min

    def tick(self):
        if self.played_low != datetime.today().weekday():  # Changes at midnight. This probably should be fixed
            low = self.songs.find_low()
            low_msg = ControlMessage()
            low_msg.play([low], looping=1)
            self.played_low = datetime.today().weekday()

        tick_msg = ControlMessage()
        if self.played_mid <= (datetime.now() - timedelta(minutes=1)):
            tick_msg.add_channel(1, self.songs.find_mid())
            self.played_mid = datetime.now()

        if self.checked_high <= (datetime.now() - timedelta(minutes=1)):
            play_chance = random.randint(0, 4)
            if play_chance == 0:
                tick_msg.add_channel(2, self.songs.find_high())
                self.played_high = datetime.now()
            self.checked_high = datetime.now()

        if panel_touched():
            tick_msg.add_channel(3, self.songs.find_high())

        tick_msg.send()

music = Music()

while True:
    music.tick()

"""  

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
