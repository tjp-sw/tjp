#!/usr/bin/python
from random import randint
from brain import do_send

#Class to handle decision makig process for which animation index to select + all the additional paramters accordingly.
#Goal: to have a formula that creates good bounds for a random number generator to make the final call. Need to ultimately 
#"select" a range of options in order to mitigate repeat animations due to particular sound tirggers. 
class AnimationSelection:
	'a class for handling external inputs and making animation selections accordingly'
	animations = ['lowLevel', 'midLevel', 'highlevel']
	dayOfWeek = 1; #1-7
	hourOfDay = 0; #0-23
	frequencies = ['63 Hz', '160 Hz', '400 Hz', '1 kHz', '2.5 kHz', '6.25 kHz', '16 kHz'] #will hold the respective amplitudes at each band of combined audio output via spectrum shield
	dominantFreqBand = randint(0, 6) #index corresponding to the frequencies[] array indices. Will drive the 'leading' animation index selection

	#todo
	def getExernalNonAudioInputs():
		#get external inputs ie temp, time of day, day of week, hand ic input (proximity & touch)
		#temporarily using random values
		dayOfWeek = randint(1,7)
		hourOfDay = randint(0,23)

	#todo: from db or from spectrum shield in 'realtime'. 
	def getAudioFrequencies():
		#get dominant frequency info 
		populateFrequencies(0, 0, 0, 0, 0, 0, 0)

	def populateFrequencies(hz1, hz2, hz3, hz4, hz5, hz6, hz7):
		frequencies[0] = hz1
		frequencies[1] = hz2
		frequencies[2] = hz3
		frequencies[3] = hz4
		frequencies[4] = hz5
		frequencies[5] = hz6
		frequencies[6] = hz7

		dominantFreqBand = frequencies[0]

do_send(0, "dodo")

