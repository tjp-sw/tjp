import random, itertools
from datetime import datetime, time, timedelta
import shows


# Stores Music library and returns appropriate songs
DAY1_LOW = range(1,7)
DAY1_MID = range(7, 15)
DAY1_HIGH = range(1000,1019)
DAY2_LOW = range(2100,2105)
DAY2_MID = range(100, 159)
DAY2_HIGH = range(1100,1135)
DAY3_LOW = range(2200,2207)
DAY3_MID = range(1200, 1277)
DAY3_HIGH = range(200, 217)
DAY4_LOW = [20]
DAY4_MID = range(1300,1325)
DAY4_HIGH = range(300,304)
DAY5_LOW = range(2500, 2512)
DAY5_MID = range(501, 587)
DAY5_HIGH = range(1500, 1512)
DAY6_LOW = [20]
DAY6_MID = [20]
DAY6_HIGH = [20]
DAY7_LOW = [20]
DAY7_MID = [20]
DAY7_HIGH = [20]
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

SET_THEME = 0

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

def find_low(theme=datetime.today().weekday()):
    if SET_THEME >= 0:
        theme = SET_THEME
    return random.choice(LOWS[theme])


def find_mid(theme=datetime.today().weekday()):
    if SET_THEME >= 0:
        theme = SET_THEME
    elif shows.show_mode == shows.NIGHT:
        daily_mids = [song for sublist in MIDS[0:theme] for song in sublist]
        return random.choice(daily_mids)
    return random.choice(MIDS[theme])


def find_high(theme=datetime.today().weekday()):
    if SET_THEME >= 0:
        theme = SET_THEME
    elif shows.show_mode == shows.NIGHT:
        daily_highs = [song for sublist in HIGHS[0:theme] for song in sublist]
        return random.choice(daily_highs)
    return random.choice(HIGHS[theme])
