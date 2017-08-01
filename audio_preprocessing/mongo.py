from pymongo import MongoClient
import ujson

import parser

import sys
sys.path.insert(0, '../')
from audioInfo import AudioFileInfo, AudioEvent


def encode_custom_event(event):
    return {"_type:": "AudioEvent", "time": event.time, "magnitude": event.magnitude, "kind": event.kind}


def decode_custom_event(document):
    return AudioEvent(document["time"], document["magnitude"], document["kind"])


def encode_custom(audioInfo):
    return {"_type": "AudioFileInfo", "name": audioInfo.name, "category": audioInfo.category, "file_index": audioInfo.file_index, "events": ujson.dumps(audioInfo.events)}


def decode_custom(document):
    name = document["name"]
    cat = document["category"]
    fi = document["file_index"]

    events_json = ujson.loads(document["events"])
    events = []
    for event in events_json:
        events.append(decode_custom_event(event))

    return AudioFileInfo(name, cat, fi, events)


def get_collection():
    global files
    client = MongoClient('localhost', 27017)
    db = client.audioInfo
    files = db.audioFiles
    return files


def insert_audio_data(audioInfoList):
    global files
    files = get_collection()

    files.delete_many({})
    for audioInfo in audioInfoList:
        to_mongo(audioInfo)


def to_mongo(audioInfo):
    global files

    ec = encode_custom(audioInfo)
    # print ec
    files.insert({"_id": audioInfo.file_index, "AudioFileInfo": ec})


def grab_audio_info(file_num):
    try:
        audioInfo = decode_custom(files.find_one({"_id": file_num})["AudioFileInfo"])
        return audioInfo
    except:
        print "Invalid audio file number... something went pretty wrong!"


#infoList = parser.parseProcessedAudioData()
#insert_audio_data(infoList)
#grab_audio_info("8092")
