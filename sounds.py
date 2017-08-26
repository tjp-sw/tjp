import random, itertools
from datetime import datetime, time, timedelta
import shows


# Stores Music library and returns appropriate songs
DAY1_LOW = range(1,8)
DAY1_MID = range(7, 57)
DAY1_HIGH = range(1000,1020)
DAY2_LOW = range(2100,2106)
DAY2_MID = range(100, 160)
DAY2_HIGH = range(1100,1136)
DAY3_LOW = range(2200,2208)
DAY3_MID = range(1200, 1278)
DAY3_HIGH = range(200, 218)
DAY4_LOW = range(2300,2304)
DAY4_MID = range(1300,1377)
DAY4_HIGH = range(300,307)
DAY5_LOW = range(2500, 2513)
DAY5_MID = range(501, 588)
DAY5_HIGH = range(1500, 1513)
DAY6_LOW = range(2600, 2611)
DAY6_MID = range(600, 650)
DAY6_HIGH = range(1600, 1617)
DAY7_LOW = range(2700, 2705)
DAY7_MID = range(1700,1775)
DAY7_HIGH = range(700,715)
MIDS = [DAY1_MID, DAY2_MID, DAY3_MID, DAY4_MID, DAY5_MID, DAY6_MID, DAY7_MID]
HIGHS = [DAY1_HIGH, DAY2_HIGH, DAY3_HIGH, DAY4_HIGH, DAY5_HIGH, DAY6_HIGH, DAY7_HIGH]
LOWS = [DAY1_LOW, DAY2_LOW, DAY3_LOW, DAY4_LOW, DAY5_LOW, DAY6_LOW, DAY7_LOW]
ALL_LOWS = [song for sublist in LOWS for song in sublist]
ALL_MIDS = [song for sublist in MIDS for song in sublist]
ALL_HIGHS = [song for sublist in HIGHS for song in sublist]

MEDITATIONS_SOUNDS = range(4001,4014)

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

SET_THEME = -1

#If it is time to play a meditation will return the number of the correct meditation. Otherwise returns None.
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

def find_low(theme=shows.bm_day_index, now_time=datetime.fromtimestamp(shows.virtual_time)):
    if SET_THEME >= 0:
        theme = SET_THEME
    return random.choice(LOWS[theme])


def find_mid(theme=shows.bm_day_index, now_time=datetime.fromtimestamp(shows.virtual_time)):
    if SET_THEME >= 0:
        theme = SET_THEME
    if time(6, 25) < now_time < time(19, 25):
        return random.choice(MIDS[theme])
    if theme == 7:
        return random.choice(MIDS[theme])
    daily_mids = [song for sublist in MIDS[0:(theme+1)] for song in sublist]
    return random.choice(daily_mids)


def find_high(theme=shows.bm_day_index, now_time=datetime.fromtimestamp(shows.virtual_time)):
    if SET_THEME >= 0:
        theme = SET_THEME
    if time(6, 25) < now_time < time(19, 25):
        return random.choice(HIGHS[theme])
    if theme == 7:
        return random.choice(HIGHS[theme])
    daily_highs = [song for sublist in HIGHS[0:(theme+1)] for song in sublist]
    return random.choice(daily_highs)
