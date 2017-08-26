#!/usr/bin/python
#from audioEvent import AudioEvent
from sys import maxint
from random import randint

# Stores Music library and returns appropriate songs
# SUPER HACKY... SHOUDL REALLY IMPORT sounds.py BUT TIRED AND SLOPPY RIGHT NOW
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


    def getAudioDay(self):
        out = -1
        if(int(self.file_index) in DAY1_LOW or int(self.file_index) in DAY1_MID or int(self.file_index) in DAY1_HIGH):
            out = 0
        elif(int(self.file_index) in DAY2_LOW or int(self.file_index) in DAY2_MID or int(self.file_index) in DAY2_HIGH):
            out =  1
        elif(int(self.file_index) in DAY3_LOW or int(self.file_index) in DAY3_MID or int(self.file_index) in DAY3_HIGH):
            out =  2
        elif(int(self.file_index) in DAY4_LOW or int(self.file_index) in DAY4_MID or int(self.file_index) in DAY4_HIGH):
            out =  3
        elif(int(self.file_index) in DAY5_LOW or int(self.file_index) in DAY5_MID or int(self.file_index) in DAY5_HIGH):
            out =  4
        elif(int(self.file_index) in DAY6_LOW or int(self.file_index) in DAY6_MID or int(self.file_index) in DAY6_HIGH):
            out =  5
        elif(int(self.file_index) in DAY7_LOW or int(self.file_index) in DAY7_MID or int(self.file_index) in DAY7_HIGH):
            out =  6
        print "audio day: " + str(out)
        return out

    def getNumericalCategory(self):
        out = -1
        if(self.category == "LOW"):
            out =  0
        elif(self.category == "MID"):
            out =  1
        elif(self.category == "HIGH"):
            out =  2
        print "audio cat num: " + str(out)
        return out

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

