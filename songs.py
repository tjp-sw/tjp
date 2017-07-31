import random
from datetime import datetime


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
MEDITATIONS = []

SET_THEME = 0


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


def find_meditation(this_time=datetime.now()):
    if this_time.hour < 12:
        return MEDITATIONS[this_time.day * 2]
    return MEDITATIONS[1 + this_time.day * 2]

