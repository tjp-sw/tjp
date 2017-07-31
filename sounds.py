import random, itertools
from datetime import datetime, time, timedelta


# Stores Music library and returns appropriate songs
RED_LOW = [20]
RED_MID = range(1, 60)
RED_HIGH = range(61, 85)
ORANGE_LOW = range(1, 77)
ORANGE_MID = range(1, 77)
ORANGE_HIGH = range(1, 77)
YELLOW_LOW = range(1, 85)
YELLOW_MID = [5]
YELLOW_HIGH = [6]
GREEN_LOW = []
GREEN_MID = [7]
GREEN_HIGH = [8]
BLUE_LOW = range(2500, 2512)
BLUE_MID = range(500, 587)
BLUE_HIGH = range(1500, 1512)
PURPLE_LOW = []
PURPLE_MID = [11]
PURPLE_HIGH = [12]
WHITE_LOW = []
WHITE_MID = [13]
WHITE_HIGH = [14]
MIDS = [RED_MID, ORANGE_MID, YELLOW_MID, GREEN_MID, BLUE_MID, PURPLE_MID, WHITE_MID]
HIGHS = [RED_HIGH, ORANGE_HIGH, YELLOW_HIGH, GREEN_HIGH, BLUE_HIGH, PURPLE_HIGH, WHITE_HIGH]
LOWS = [RED_LOW, ORANGE_LOW, YELLOW_LOW, GREEN_LOW, BLUE_LOW, PURPLE_LOW, WHITE_LOW]
MEDITATIONS_SOUNDS = []

MEDITATIONS_TIMES = ["06:19 28/8/17",  # Monday
                     "19:34 28/8/17",
                     "06:21 29/8/17",  # Tuesday
                     "19:33 29/8/17",
                     "06:22 30/8/17",  # Wednesday
                     "19:31 30/8/17",
                     "06:23 31/8/17",  # Thursday
                     "19:29 31/8/17",
                     "06:23 01/9/17",  # Friday
                     "19:29 01/9/17",
                     "06:25 02/9/17",  # Saturday
                     "19:26 02/9/17",
                     "06:26 03/9/17",  # Sunday
                     "19:24 03/9/17" ]
MEDITATIONS = dict(itertools.izip([datetime.strptime(m, "%H:%M %d/%m/%y") for m in MEDITATIONS_TIMES], MEDITATIONS_SOUNDS))

SET_THEME = 0

def play_meditation(this_time=datetime.now()):
    approx_time= datetime.time(this_time)
    if (approx_time < time(6, 18) or approx_time > time(6, 31)) and \
       (approx_time < time(19, 33) or approx_time > time(19, 38)):
        return None
    for meditation in MEDITATIONS.iterkeys():
        if this_time >= meditation and \
           this_time <= meditation + timedelta(minutes=5):
            return MEDITATIONS[meditation]
    return None

def find_low(theme=datetime.today().weekday()):
    if SET_THEME >= 0:
        theme = SET_THEME
    return random.choice(LOWS[theme])


def find_mid(theme=datetime.today().weekday()):
    if SET_THEME >= 0:
        theme = SET_THEME
    elif datetime.now().hour >= 21:
        all_mids = [song for sublist in MIDS[0:theme] for song in sublist]
        return random.choice(all_mids)
    return random.choice(MIDS[theme])


def find_high(theme=datetime.today().weekday()):
    if SET_THEME >= 0:
        theme = SET_THEME
    elif datetime.now().hour >= 21:
        all_highs = [song for sublist in HIGHS[0:theme] for song in sublist]
        return random.choice(all_highs)
    return random.choice(HIGHS[theme])


