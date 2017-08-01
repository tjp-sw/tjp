#!/usr/bin/python
#from audioEvent import AudioEvent


class AudioFileInfo:


    def __init__(self, name, index, category, events = []):
        self.name = name
        self.category = category
        self.file_index = index
        self.events = events
        self.event_index = 0

    def __repr__(self):
        return "%s AudioFileInfo '%s' named_category:%s events: [%s]" % (self.file_index, self.name, self.category, self.events)

    def addEvent(self, event):
        self.events.append(event)

    def getNextEvent(self):
        event = self.events[self.event_index]
        self.event_index += 1
        return event


class AudioEvent:
    'a class to hold an audio event info'

    def __init__(self, time, magnitude, kind):
        self.time = time
        self.magnitude = magnitude
        self.kind = kind


    def __repr__(self):
        return "kind: %s, time: %s, magnitude: %s" % (self.kind, self.time, self.magnitude)
