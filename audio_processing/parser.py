from audioInfo import AudioFileInfo, AudioEvent

fileInfos = []
ind = -1

for line in open("output_t2", 'r'):
    info = line.split()
    if(len(info) > 1):
        if(info[0] == "Processing"):
            #print line
            name = info[1].split("/")
            #print name[5]
            ids = name[5].split("_")
            #print ids[0] + " " + ids[2]
            fileInfo = AudioFileInfo(name[5], ids[0], ids[2])
            #print fileInfo
            fileInfos.append(fileInfo)
            ind += 1
        elif(info[0] == "@@" or info[0] == "##" or info[0] == "@" or info[0] == "#"):
            #print line
            fileI = fileInfos[ind]
            event = AudioEvent(info[2], info[7], info[3], info[5])
            #print event
            fileI.addEvent(event)
            #print fileInfo

print fileInfos
