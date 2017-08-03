# USED FOR OLD AUDIO PROCESSING METHOD
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
    for line in open("sample_processing", 'r'):
        info = line.split()
        if(len(info) > 1):
            if(info[0] == "Processing"):
                if pack:
                    ai = AudioFileInfo(name, file_index, category, events)
                    fileInfos.append(ai)
                    name = ""
                    file_index = ""
                    category = ""
                    events = []

                #print line
                name = info[1].split("/")
                #print name[5]
                ids = name[5].split("_")
                #print ids[0] + " " + ids[2]
                name = name[5]
                file_index = ids[0]
                category = ids[2]
                pack = True

            elif(info[0] == "@@" or info[0] == "##" or info[0] == "@" or info[0] == "#"):
                #print line
                event = AudioEvent(info[2], info[3], info[5], category)
                #print event
                events.append(event)
                #print fileInfo

    return fileInfos


#parseProcessedAudioData()
#print fileInfos
#print len(fileInfos)
# grab the pprint library to save your eyes...
#pp = pprint.PrettyPrinter(indent=4)
#pp.pprint(fileInfos)
