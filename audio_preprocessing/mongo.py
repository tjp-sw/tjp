from pymongo import MongoClient
import datetime
import pprint
import ujson

import sys
sys.path.insert(0, '../')
from audioInfo import AudioFileInfo, AudioEvent

client = MongoClient('localhost', 27017)

db = client.test

files = db.audioFiles

pprint.pprint(files.find_one())

audioInfo = AudioFileInfo("trees", "9", "high")
event1 = AudioEvent("453", "-1", "FreqBand")
audioInfo.addEvent(event1)
print(audioInfo)

postt = ujson.dumps(audioInfo)

print postt

#NOT WORKING YET.... IN PROGRESS!!! 7/27
post = {"_id": audioInfo.file_index,
        "name": audioInfo.name,
        "category": audioInfo.category,
        "events": audioInfo.events}

files.insert_one(post)

#print(ujson.dumps(audioInfo))
