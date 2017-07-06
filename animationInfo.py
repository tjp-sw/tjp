#!/usr/bin/python
from enum import Enum

#Classes for encapsulating animation info post retrieval from DB.
#The attributes that will be stored in DB are slightly arbitrary but need to be populated...
#I feel this will be the basis for selecting appropriate animations based on frequency response.
#Certainly more inputs + randomization is necessary for the final animation output. 

class FreqBand(Enum):
   LOW = 0
   MID = 1
   HIGH = 2

#Frequency bands corresp
class AnimationInfo:
   'a class to hold animation characteristics'

   def __init__(self, index, bestFreqBands, name):
      self.index = index
      self.bestFreqBands = bestFreqBands
      self.name = name

   def __repr__(self):
      return "Animation '%s' [index:%s, bestFreqBands:%s>]" % (self.name, self.index, self.bestFreqBands)

   def displayInfo(self):
      print "AnimatoinIndex : ", self.index,  ", BestFreqBands: ", self.bestFreqBands

   def addBand(self, band):
      self.bestFreqBands.add(band)

   def containsBand(self, FreqBand):
      return FreqBand in bestFreqBands

class Animations:
   'A class to contian lists of particular animations for various audio inputs. Ideally should be populated from the Database as the information is non-volitle'
   totalNumOfAnimations = 0;
   lowFreqAnimation = [];
   midFreqAnimation = [];
   highFreqAnimations = [];

   def displayCount(self):
     print "Total number of animations %d" % AnimationInfo.totalNumOfAnimations

   def addAnimation(animation_info):
      if(FreqBand.LOW in animation_info.bestFreqBands):
         lowFreqAnimation.append(animation_info);
      if(FreqBand.MID in animation_info.bestFreqBands):
         midFreqAnimation.append(animation_info);
      if(FreqBand.HIGH in animation_info.bestFreqBands):
         highFreqAnimation.append(animation_info);

      totalNumOfAnimations += 1

