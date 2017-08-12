#!/usr/bin/python
#from audioEvent import AudioEvent
from sys import maxint
from random import randint


class AudioFileInfo:
    def __init__(self, name, index, category, events = [], animations = []):
        self.name = name
        self.category = category
        self.file_index = index
        self.events = events
        self.event_index = 0
        self.animations = animations


    def __repr__(self):
        return "%s AudioFileInfo '%s' named_category:%s animations: [%s] events: [%s]" % (self.file_index, self.name, self.category, self.animations, self.events)


    def __str__(self):
        return "%s AudioFileInfo '%s' named_category:%s animations: [%s] events: [%s]" % (self.file_index, self.name, self.category, self.animations, self.events)


    def addAnimation(self, animation):
        self.animations.append(animation)


    def getRandomSuitableAnimation(self):
        if len(self.animations) > 0:
            return self.animations[randint(0, len(self.animations))]
        #return None

        #HACK UNTIL CAN GET REAL VALUES POPULATED
        if self.category == "LOW":
            return randint(1,3) # number of base animations
        if self.category == "MID":
            return randint(1,13) # number of mid animations

        #high
        return randint(1,10) # number of sparkle animations


    def addEvent(self, event):
        self.events.append(event)


    def getNextEvent(self):
        if self.event_index < len(self.events):
            event = self.events[self.event_index]
            self.event_index += 1
            return event
        return None


class AudioEvent:
    'a class to hold an audio event info'


    def __init__(self, time, magnitude, kind, category, exec_time=0):
        self.time = time
        self.magnitude = magnitude
        self.kind = kind
        self.category = category
        self.exec_time = exec_time


    def __dict__(self):
        return {"time": self.time, "magnitude": self.magnitude, "kind": self.kind, "category": self.category}


    def __eq__(self, other):
        return self.__dict__ == other.__dict__


    def __str__(self):
        return "kind: %s, time: %s, magnitude: %s, category: %s, exec_time: %s" % (self.kind, self.time, self.magnitude, self.category, self.exec_time)


    def __repr__(self):
        return "kind: %s, time: %s, magnitude: %s, category: %s, exec_time: %s" % (self.kind, self.time, self.magnitude, self.category, self.exec_time)
