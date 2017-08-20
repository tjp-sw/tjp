from pymongo import MongoClient
try:
    from pymongo.errors import ServerSelectionTimeoutError
    from pymongo.errors import DuplicateKeyError
except:
    pass
import json
import parser
import parser_new

import sys
sys.path.insert(0, '../')
from audioInfo import AudioFileInfo, AudioEvent

MONGO_DEBUG = True

def encode_events(events):
    output = []
    for event in events:
        j = json.dumps(event.__dict__)
        output.append(j)

    return output


def decode_custom_event(document):
    return AudioEvent(document["time"], document["magnitude"], document["kind"], document["category"])


def encode_custom(audioInfo):
    #return {"_type": "AudioFileInfo", "name": audioInfo.name, "category": audioInfo.category, "file_index": audioInfo.file_index, "animations:": audioInfo.animations, "events": encode_events(audioInfo.events)}
    return {"_type": "AudioFileInfo", "name": audioInfo.name, "category": audioInfo.category, "file_index": audioInfo.file_index, "events": encode_events(audioInfo.events)}


def decode_custom(document):
    name = document["name"]
    cat = document["category"]
    fi = document["file_index"]
    #anis = document["animations"]

    events_json = document["events"]
    # print events_json
    events = []
    for event in events_json:
        events.append(decode_custom_event(json.loads(event)))

    #return AudioFileInfo(name, cat, fi, events, anis)
    return AudioFileInfo(name, fi, cat, events)


def get_collection():
    global files
    try:
        client = MongoClient('localhost', 27017)
        db = client.audioInfo
        files = db.audioFiles
    except:
        if MONGO_DEBUG:
            print 'mongo database:', sys.exc_value
        files = {}
    return files


def insert_audio_data(audioInfoList):
    global files
    files = get_collection()

    res = files.delete_many({})
    if MONGO_DEBUG:
        print res.deleted_count
    for audioInfo in audioInfoList:
        to_mongo(audioInfo)


def to_mongo(audioInfo):
    global files

    # hack

    ec = encode_custom(audioInfo)
    # print ec
    try:
        files.insert({"_id": audioInfo.file_index, "AudioFileInfo": ec})
    except:
        if MONGO_DEBUG:
            print "duplicate key... audio files aren't uniquely named: ", sys.exc_value

def grab_audio_info(colleciton, file_num):
    global files
    try:
        audioInfo = decode_custom(files.find_one({"_id": file_num})["AudioFileInfo"])
        return audioInfo
    except TypeError:
        if MONGO_DEBUG:
            print "Invalid audio file number " + file_num + "... something went pretty wrong!"
    except:
        if MONGO_DEBUG:
            print 'mongo database:', sys.exc_value


#infoList = parser.parseProcessedAudioData()
populate_mongo = False
if populate_mongo:
    infoList = parser_new.parseProcessedAudioData()
    insert_audio_data(infoList)
