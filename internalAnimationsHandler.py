import shows

import sys
from dataBaseInterface import DataBaseInterface
from audioInfo import AudioEvent, AudioFileInfo
from audio_event_queue import SortedDLL

NUM_AUDIO_CHANNELS = 7

class InternalAninamtionsHandler:
    'a class for handling audio messages to drive internal audio animations'

    def __init__(self):
        self.event_queue = SortedDLL()  # create sorted dll to act as the audio event queue (with super duper special powers)
        self.current_internal_track_per_channel = [0] * NUM_AUDIO_CHANNELS
        self.next_audio_event = AudioEvent(-1, -1, "init", "init")
        self.doing_animations = False

    def set_do_animations(self, do):
        self.doing_animations = do

    def interpret_audio_msg(self, audio_msg):
        channel_map = self.get_audio_file_info(audio_msg)
        for channel in channel_map.keys():
            if self.current_internal_track_per_channel[channel] > 0: # channel already had a track on it
                old_audio = self.current_internal_track_per_channel[channel]
                self.remove_audio_events_from_queue(old_audio)

            audioInfo = channel_map[channel]
            self.current_internal_track_per_channel[channel] = audioInfo
            self.queue_audio_events(audioInfo)

            if audioInfo is not None and self.doing_animations:
                if shows.INTERNAL_ANIMATIONS_DEBUG:
                    print "setting main animation param due to new audio track's info " + str(audioInfo.file_index)
                shows.set_appropriate_layer_main_animation(audioInfo)
                #MAYBE shows. ???? CHECK


    # RJS I don't like how this hard coded... if the audio contorl message changes this needs to as well.
    def get_audio_file_info(self, audio_msg):
        # current msg format: a0;1;0;0,50,0,0
        # print 'grabbing audio info from ' + str(audio_msg)
        try:
            info = audio_msg.split(";")
            tracks = info[3].split(",")
        except:
            print "audio msg format is wrong or has changed", sys.exc_value
            return {}

        if shows.INTERNAL_ANIMATIONS_DEBUG:
            print "tracks: " + str(tracks)

        output = {}
        i = 0
        if tracks[i] is not "" and tracks[i] != '':
            while(i < len(tracks)):
                if int(tracks[i]) > 0:
                    output[i] = (DataBaseInterface().grabAudioInfo(tracks[i]))
                i += 1

        return output


    def progress_audio_queue(self):
        # ensure not looking at events that have already passed
        while True:
            try:
                next_audio_event_node = self.event_queue.peek()
                old_event = self.next_audio_event
                self.next_audio_event = next_audio_event_node.value
                if shows.INTERNAL_ANIMATIONS_DEBUG and str(old_event) != str(self.next_audio_event):
                    print "next audio event " + str(self.next_audio_event)
            except ValueError:
                #if INTERNAL_ANIMATIONS_DEBUG:
                    #print "event_queue is empty", sys.exc_value
                break

            stale = self.next_audio_event.exec_time <= shows.timeMs() - 1000

            if stale:
                if shows.DEBUG:
                    print "it's " + str(shows.timeMs()) + " stale event " + str(self.next_audio_event) + " removing"
                if shows.INTERNAL_ANIMATIONS_DEBUG:
                    print "new audio event queue size: " + str(self.audio_event_queue.size)
                self.event_queue.remove(self.next_audio_event)
            else:
                break

        return self.next_audio_event


    def remove_audio_events_from_queue(self, audioInfo):
        for event in audioInfo.events:
            try:
                self.event_queue.remove(event)
            except ValueError:
                print "event " + str(event) + " already has been removed from queue"
            except AttributeError:
                print "error removing event from queue"
        if shows.INTERNAL_ANIMATIONS_DEBUG:
            print "new event queue size after removal: " + str(self.event_queue.size)


    def queue_audio_events(self, audioInfo):
        cur_time_ms = shows.timeMs()
        if audioInfo is not None:
            if shows.INTERNAL_ANIMATIONS_DEBUG:
                print "queueing events for audioInfo: " + str(audioInfo.file_index)
            for event in audioInfo.events:
                # print "event: " + str(event)
                event.exec_time = int(event.time) + cur_time_ms
                # print "NEW e TIME = " + str(event.exec_time) + "\nCURNT TIME = " + str(cur_time_ms)
                node = self.event_queue.add(event)
        else:
            print "seems like it was a database miss... this will happen while we don't have all the auido files"

        if shows.INTERNAL_ANIMATIONS_DEBUG:
            print "new event queue size: " + str(self.event_queue.size)
