#!/usr/bin/python
from audioInfo import AudioFileInfo, AudioEvent
import sys
sys.path.insert(0, 'audio_preprocessing/')
import mongo


RETURN_MOCK = False
RETURN_MOCK_NEW_PROCESSING = True

# Class for interfacing with the audio database on the pi.
class DataBaseInterface:
	'a class for interfacing with the database'

	def grabAudioInfo(self, file_num):
		'grab audio file characteristics from db'

		if not RETURN_MOCK:
			# print "grabbing audioInfo for TRACK: " + file_num

			files = mongo.get_collection()
			audioInfo = mongo.grab_audio_info(files, file_num)
			# print str(audioInfo)

			return audioInfo
		else:
			# MOCKED return vals
			if RETURN_MOCK_NEW_PROCESSING:
				if int(file_num) == 1:
					audioInfo = AudioFileInfo("rain", 1, "mid")

					e2 = AudioEvent(407, 3.9, "amplitude", "mid")
					e4 = AudioEvent(7000, 4.6, "amplitude", "mid")

					audioInfo.addEvent(e2)
					audioInfo.addEvent(e4)
				elif int(file_num) == 2:
					audioInfo = AudioFileInfo("thunder", 2, "low")

					e2 = AudioEvent(450, 3.2, "amplitude", "low")
					e4 = AudioEvent(2450, 5.4, "amplitude", "low")

					audioInfo.addEvent(e2)
					audioInfo.addEvent(e4)
				else:
					audioInfo = AudioFileInfo("wind", 9, "mid")

					e2 = AudioEvent(1450, 4.1, "amplitude", "mid")
					e4 = AudioEvent(2450, 6.0, "amplitude", "mid")

					audioInfo.addEvent(e2)
					audioInfo.addEvent(e3)
			else:
				if int(file_num) == 1:
					audioInfo = AudioFileInfo("rain", 1, "mid")

					e1 = AudioEvent(4, -3, "freqband", "mid")
					e2 = AudioEvent(47, 900, "amplitude", "mid")
					e3 = AudioEvent(640, 1, "freqband", "mid")
					e4 = AudioEvent(700, -1300, "amplitude", "mid")

					audioInfo.addEvent(e1)
					audioInfo.addEvent(e2)
					audioInfo.addEvent(e3)
					audioInfo.addEvent(e4)
				elif int(file_num) == 2:
					audioInfo = AudioFileInfo("thunder", 2, "low")

					e2 = AudioEvent(45, 2200, "amplitude", "low")
					e1 = AudioEvent(60, 2, "freqband", "low")
					e3 = AudioEvent(240, -3, "freqband", "low")
					e4 = AudioEvent(245, -800, "amplitude", "low")

					audioInfo.addEvent(e1)
					audioInfo.addEvent(e2)
					audioInfo.addEvent(e3)
					audioInfo.addEvent(e4)
				else:
					audioInfo = AudioFileInfo("wind", 9, "mid")
					#    def __init__(self, time, degree, magnitude, kind):
					e1 = AudioEvent(1, -6, "freqband", "mid")
					e2 = AudioEvent(145, 800, "amplitude", "mid")
					e3 = AudioEvent(240, 3, "freqband", "mid")
					e4 = AudioEvent(245, -1600, "amplitude", "mid")

					audioInfo.addEvent(e1)
					audioInfo.addEvent(e2)
					audioInfo.addEvent(e3)
					audioInfo.addEvent(e4)

			return audioInfo

#dbinter = DataBaseInterface()
#dbinter.grabAudioInfo("8")
