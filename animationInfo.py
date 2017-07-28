#!/usr/bin/python
#from audioEvent import AudioEvent
from enum import Enum


class AudioFileInfo:
    events = []
    event_index = 0

    def __init__(self, name, index, category):
        self.name = name
        self.category = category
        self.file_index = index

    def __repr__(self):
        return "%s AudioFileInfo '%s' named_category:%s events: [%s]" % (self.file_index, self.name, self.category, self.events)

    def addEvent(self, event):
        self.events.append(event)

    def getNextEvent(self):
        event = self.events[event_index]
        self.event_index += 1
        return event


class FreqBand:
    #Low, Mid, High = range(3)
    low = "low",
    mid = "mid",
    high = "high"


class Degree(Enum):
    MINOR = 0,
    MAJOR = 1


class Kind(Enum):
    Amplitude = "@",
    FreqBand = "!"


class AudioEvent:
    'a class to hold an audio event info'

    def __init__(self, time, degree, magnitude, kind):
        self.time = time
        self.degree = degree
        self.magnitude = magnitude
        self.kind = kind

    #def __repr__(self):
    #    return "%s event @ %s [%s, magnitude:%s]\n" % (self.kind, self.time, self.degree, self.magnitude)
    def __repr__(self):
        return "kind: %s, time: %s, degree: %s, magnitude: %s" % (self.kind, self.time, self.degree, self.magnitude)
