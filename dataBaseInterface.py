#!/usr/bin/python
from animationInfo import AnimationInfo

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


DataBaseInterface().populateAnimations()
