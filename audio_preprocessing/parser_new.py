# Parsing the processed audio info.
# From here will go into Mongo.

#import pprint
import sys
sys.path.insert(0, '../')

from audioInfo import AudioFileInfo, AudioEvent

def parseProcessedAudioData():
    global fileInfos
    fileInfos = []
    events = []
    pack = False
    name = ""
    file_index = ""
    category = ""
    for line in open("audio_preprocessing_output", 'r'):
        info = line.split()
        if(len(info) > 1):
            if(info[0] == "analyzing"):
                if pack:
                    ai = AudioFileInfo(name, file_index, category, events)
                    fileInfos.append(ai)
                    name = ""
                    file_index = ""
                    category = ""
                    events = []

                #print line
                try:
                    name = info[1].split("/")
                    #print name[5]
                    ids = name[5].split("_")
                    #print ids[0] + " " + ids[2]
                    name = name[5]
                    file_index = ids[0]
                    category = ids[2]
                    pack = True
                except IndexError:
                    print str(name) + " isn't an audio file."

            elif(info[0] == "EVENT:"):
                # print line
                time = info[1].split(".")
                event = AudioEvent(str(int(time[0]) * 1000 + int(time[1])), info[3], "amplitude", category)
                # print event
                events.append(event)

    return fileInfos


#parseProcessedAudioData()
#print fileInfos
# grab the pprint library to save your eyes...
#pp = pprint.PrettyPrinter(indent=4)
#pp.pprint(fileInfos)
