import random
from datetime import date, time, datetime, timedelta


# sends a control message to the mega
class ControlMessage:
    def __init__(self, node=0, command=0, field2=0, channels=None):
        self.node = node
        self.command = command
        self.field2 = field2
        self.channels = channels

    def send(self):
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
        self.channels = [0] * 7
        self.low_is_playing = False
        self.played_mid = datetime.min
        self.played_high = datetime.min

        self.theme = theme
        self.available_lows = [self.LOWS[theme]]
        # self.available_mids = []
        # self.available_highs = []
        # self.msg = ControlMessage()

    def find_mid(self, this_theme=None):
        if this_theme is None:
            this_theme = self.theme

        if datetime.now().hour >= 21:
            all_mids = [song for sublist in self.MIDS for song in sublist]
            return random.choice(all_mids)
        return random.choice(self.MIDS[this_theme])

    def find_high(self, this_theme= None):
        if this_theme is None:
            this_theme = self.theme

        if datetime.now().hour >= 21:
            all_highs = [song for sublist in self.HIGHS for song in sublist]
            return random.choice(all_highs)
        return random.choice(self.MIDS[this_theme])





m= Music()
ctrl= ControlMessage()

while True:

print(datetime.now().hour)
print (m.find_high())
#ctrl.play([m.find_high()])
#ControlMessage([3])

"""  
class Theme:
    def __init__(self, chakra):
        self.low= Songs

m = Music()
 
t= Tsunami()
print("yo")
     
def send_ctrl_msg(node, command, field2, channels):
    ctrl_msg = ';'.join([str(node), str(command), str(field2), ','.join(str (ch) for ch in channels)])
    ctrl_msg += ';'
    print(ctrl_msg)

def play_music(node, looping, channels):
    send_ctrl_msg(node, 1, looping, channels)

def set_volume(node, fade_speed, channels):
    node
    ctrl_msg = ';'.join([str(node), '2', str(fade_speed), ','.join(str (ch) for ch in channels)])
    ctrl_msg += ';'
    print(ctrl_msg)


#returns a random relevant song
def find_available(this_theme, channel, is_night= 0):
    return 17

def setup():

    play_music(0, 0, [find_available(theme, 1), find_available(theme, 2)])

#send_music(0, 1,0,[6,7])
random.seed()
#print (random.randint(0,23))
#print(date.today().weekday())

#Set theme
theme_list= ['RED', 'ORANGE', 'YELLOW', 'GREEN', 'BLUE', 'PURPLE', 'WHITE']
RED= 0
ORANGE= 1
YELLOW= 2
GREEN= 3
BLUE= 4
PURPLE= 5
WHITE= 6

if (low_is_playing is False):
    play_music(0, 1, [find_available(theme, 0)])
    low_is_playing= True
#print (datetime.now() - timedelta(seconds=30))


if (played_mid <= (datetime.now() - timedelta(seconds=30))):
    play_music(0, 0, [find_available(theme, 1)])
    played_mid= datetime.now()

if (played_high <= (datetime.now() - timedelta(minutes=1))):
    play_chance= random.randint(0, 4)
    if (play_chance == 0):
        play_music(0, 0, [find_available(theme, 1)])
    played_high= datetime.now()


    if(played_mid):
        print(played_mid)
except NameError:
    print (datetime.now())
"""
