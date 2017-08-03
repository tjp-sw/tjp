from pymongo import MongoClient
from pymongo.errors import ServerSelectionTimeoutError
import json
import parser
import parser_new

import sys
sys.path.insert(0, '../')
from audioInfo import AudioFileInfo, AudioEvent


def encode_events(events):
    output = []
    for event in events:
        j = json.dumps(event.__dict__)
        output.append(j)

    return output


def decode_custom_event(document):
    return AudioEvent(document["time"], document["magnitude"], document["kind"], document["category"])


def encode_custom(audioInfo):
    return {"_type": "AudioFileInfo", "name": audioInfo.name, "category": audioInfo.category, "file_index": audioInfo.file_index, "events": encode_events(audioInfo.events)}


def decode_custom(document):
    name = document["name"]
    cat = document["category"]
    fi = document["file_index"]

    #events_json = json.loads(document["events"])
    events_json = document["events"]
    #print events_json
    events = []
    for event in events_json:
        events.append(decode_custom_event(json.loads(event)))

    return AudioFileInfo(name, cat, fi, events)


def get_collection():
    global files
    try:
        client = MongoClient('localhost', 27017)
        db = client.audioInfo
        files = db.audioFiles
    except:
        print 'mongo database:', sys.exc_value
        files = {}
    return files


def insert_audio_data(audioInfoList):
    global files
    files = get_collection()

    res = files.delete_many({})
    print res.deleted_count
    for audioInfo in audioInfoList:
        to_mongo(audioInfo)


def to_mongo(audioInfo):
    global files

    ec = encode_custom(audioInfo)
    # print ec
    files.insert({"_id": audioInfo.file_index, "AudioFileInfo": ec})


def grab_audio_info(colleciton, file_num):
    global files
    try:
        audioInfo = decode_custom(files.find_one({"_id": file_num})["AudioFileInfo"])
        return audioInfo
    except ServerSelectionTimeoutError:
        print 'mongo database:', sys.exc_value
    except TypeError:
        print "Invalid audio file number " + file_num + "... something went pretty wrong!"


#infoList = parser.parseProcessedAudioData()
#infoList = parser_new.parseProcessedAudioData()
#insert_audio_data(infoList)
#ai = grab_audio_info(get_collection(), "8098")
#print ai
#print ai.getNextEvent()
