import random
from datetime import datetime


# Stores Music library and returns appropriate songs
RED_MID = [1]
RED_HIGH = [2]
ORANGE_MID = [1,8,12]
ORANGE_HIGH = [7,11]
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
LOWS = [15, 4, 17]


def find_low(theme=datetime.today().weekday()):
    return LOWS[theme]


def find_mid(theme=datetime.today().weekday()):
    if datetime.now().hour >= 21:
        all_mids = [song for sublist in MIDS for song in sublist]
        return random.choice(all_mids)
    return random.choice(MIDS[theme])


def find_high(theme=datetime.today().weekday()):
    if datetime.now().hour >= 21:
        all_highs = [song for sublist in HIGHS for song in sublist]
        return random.choice(all_highs)
    return random.choice(HIGHS[theme])
