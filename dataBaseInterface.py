#!/usr/bin/python
from audioInfo import AudioFileInfo, AudioEvent
from animationInfo import AnimationInfo
from pymongo import MongoClient
import pprint

#Class for interfacing with the animations database on the pi.
class DataBaseInterface:
	'a class for interfacing with the database'
	animationList = []

	def populateAnimations(self):
		animationList = self.grabAllAnimations()
		for animation in animationList:
			print animation

	#todo
	#grabs all animation entries in db
	def grabAllAnimations(self):
		result = []

		#loop through entries getting attributes
			#add new animationInfo object to result list

		#MOCKED RETURN
		animationInfo_1 = AnimationInfo(0, {0,1,2}, "animation1")
		animationInfo_2 = AnimationInfo(1, {2,3}, "animation2")
		animationInfo_3 = AnimationInfo(2, {5,6,7}, "animation3")
		animationInfo_4 = AnimationInfo(3, {1,2,3}, "animation4")
		animationInfo_5 = AnimationInfo(4, {3,4,5}, "animation5")
		mockEntryList = [animationInfo_1, animationInfo_2, animationInfo_3, animationInfo_4, animationInfo_5]

		return mockEntryList

	def grabAudioInfo(self, file_num):
		'grab audio file characteristics from db'

		#print "grabbing audioInfo for TRACK: " + file_num
		client = MongoClient('localhost', 27017)
		db = client.audio
		audioFile = db.audioFiles

		#todo... syntax?
		audioFile.find()

		#MOCKED return vals
		if int(file_num) == 1:
			audioInfo = AudioFileInfo("rain", 1, "mid")
			#    def __init__(self, time, degree, magnitude, kind):
			e1 = AudioEvent(4, -3, "freqband")
			e2 = AudioEvent(47, 900, "amplitude")
			e3 = AudioEvent(640, 1, "freqband")
			e4 = AudioEvent(700, -1300, "amplitude")

			audioInfo.addEvent(e1)
			audioInfo.addEvent(e2)
			audioInfo.addEvent(e3)
			audioInfo.addEvent(e4)
		elif int(file_num) == 2:
			audioInfo = AudioFileInfo("thunder", 2, "low")
			#    def __init__(self, time, degree, magnitude, kind):
			e2 = AudioEvent(45, 2200, "amplitude")
			e1 = AudioEvent(60, 2, "freqband")
			e3 = AudioEvent(240, -3, "freqband")
			e4 = AudioEvent(245, -800, "amplitude")

			audioInfo.addEvent(e1)
			audioInfo.addEvent(e2)
			audioInfo.addEvent(e3)
			audioInfo.addEvent(e4)
		else:
			audioInfo = AudioFileInfo("wind", 9, "mid")
			#    def __init__(self, time, degree, magnitude, kind):
			e1 = AudioEvent(1, -6, "freqband")
			e2 = AudioEvent(145, 800, "amplitude")
			e3 = AudioEvent(240, 3, "freqband")
			e4 = AudioEvent(245, -1600, "amplitude")

			audioInfo.addEvent(e1)
			audioInfo.addEvent(e2)
			audioInfo.addEvent(e3)
			audioInfo.addEvent(e4)

		return audioInfo


#DataBaseInterface().populateAnimations()
