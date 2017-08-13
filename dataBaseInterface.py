#!/usr/bin/python
from audioInfo import AudioFileInfo, AudioEvent
import sys
sys.path.insert(0, 'audio_preprocessing/')
import mongo


RETURN_MOCK = False
RETURN_MOCK_NEW_PROCESSING = True

INTERNAL_ANIMATIONS_DB_INTER_DEBUG = False

# Class for interfacing with the audio database on the pi.
class DataBaseInterface:
	'a class for interfacing with the database'

	def grabAudioInfo(self, file_num):
		'grab audio file characteristics from db'

		if INTERNAL_ANIMATIONS_DB_INTER_DEBUG:
			print "grabbing audioInfo for TRACK: " + file_num

		if not RETURN_MOCK:
			files = mongo.get_collection()
			audioInfo = mongo.grab_audio_info(files, file_num)

			if INTERNAL_ANIMATIONS_DB_INTER_DEBUG:
				print "grab result: " + str(audioInfo)

			return audioInfo
		else:
			# MOCKED return vals
			if RETURN_MOCK_NEW_PROCESSING:
				if int(file_num) == 1:
					audioInfo = None
					audioInfo = AudioFileInfo("rain", 1, "MID")

					e2 = AudioEvent(407, 3.9, "amplitude", "MID")
					e4 = AudioEvent(7000, 4.6, "amplitude", "MID")

					audioInfo.addEvent(e2)
					audioInfo.addEvent(e4)
				elif int(file_num) == 2:
					audioInfo = None
					audioInfo = AudioFileInfo("thunder", 2, "LOW")

					e2 = AudioEvent(450, 3.2, "amplitude", "LOW")
					e4 = AudioEvent(2450, 5.4, "amplitude", "LOW")

					audioInfo.addEvent(e2)
					audioInfo.addEvent(e4)
				else:
					audioInfo = None
					audioInfo = AudioFileInfo("wind", 9, "MID")

					e2 = AudioEvent(1450, 4.1, "amplitude", "MID")
					e4 = AudioEvent(2450, 6.0, "amplitude", "MID")
					e1 = AudioEvent(8450, 4.1, "amplitude", "MID")
					e3 = AudioEvent(17450, 6.0, "amplitude", "MID")

					audioInfo.addEvent(e1)
					audioInfo.addEvent(e2)
					audioInfo.addEvent(e3)
					audioInfo.addEvent(e4)
			else:
				if int(file_num) == 1:
					audioInfo = AudioFileInfo("rain", 1, "MID")

					e1 = AudioEvent(4, -3, "freqband", "MID")
					e2 = AudioEvent(47, 900, "amplitude", "MID")
					e3 = AudioEvent(640, 1, "freqband", "MID")
					e4 = AudioEvent(700, -1300, "amplitude", "MID")

					audioInfo.addEvent(e1)
					audioInfo.addEvent(e2)
					audioInfo.addEvent(e3)
					audioInfo.addEvent(e4)
				elif int(file_num) == 2:
					audioInfo = AudioFileInfo("thunder", 2, "LOW")

					e2 = AudioEvent(45, 2200, "amplitude", "LOW")
					e1 = AudioEvent(60, 2, "freqband", "LOW")
					e3 = AudioEvent(240, -3, "freqband", "LOW")
					e4 = AudioEvent(245, -800, "amplitude", "LOW")

					audioInfo.addEvent(e1)
					audioInfo.addEvent(e2)
					audioInfo.addEvent(e3)
					audioInfo.addEvent(e4)
				else:
					audioInfo = AudioFileInfo("wind", 9, "MID")
					#    def __init__(self, time, degree, magnitude, kind):
					e1 = AudioEvent(1, -6, "freqband", "MID")
					e2 = AudioEvent(145, 800, "amplitude", "MID")
					e3 = AudioEvent(240, 3, "freqband", "MID")
					e4 = AudioEvent(245, -1600, "amplitude", "MID")

					audioInfo.addEvent(e1)
					audioInfo.addEvent(e2)
					audioInfo.addEvent(e3)
					audioInfo.addEvent(e4)

			return audioInfo

#dbinter = DataBaseInterface()
#dbinter.grabAudioInfo("8")
