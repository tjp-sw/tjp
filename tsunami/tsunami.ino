#include <Tsunami.h>            // Include the Tsunami library header
#include <stdlib.h>

#define SETALLAUDIO 1
#define SETAUDIOCH 2
#define SETVOLCH 3
#define MUTEALLAUDIO 4

Tsunami tsunami;                // Our Tsunami object
boolean new_ctrl_msg = false;  // whether a new message exists to process
int channels[18] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int node = 0;

struct control_message {
  int command;        //Command to perform
  int channels[18];   //Array of tracks to change
  int gain;           //Optional volume
  int fade_speed;     //speed to change volume
};

int input_digit = 0;
int input_num [ ] = {0,0,0,0} ;

control_message ctrl_msg = {0,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},0,0};

// ***Magically figure out what node this is. 
// ***Probably should be slightly less magical in the final version
void get_node() {
  node = 0;
}

void setup() {
  // initialize serial:
  Serial.begin(9600);

  // We should wait for the Tsunami to finish reset before trying to send
  // commands.
  delay(1000);

  // Tsunami startup at 57600
  tsunami.start();
  delay(10);
  
  // Send a stop-all command and reset the sample-rate offset, in case we have
  //  reset while the Tsunami was already playing.
  tsunami.stopAllTracks();
  tsunami.samplerateOffset(0, 0);
  tsunami.masterGain(0, 0);              // Reset the master gain to 0dB  

  // Enable track reporting from the Tsunami
  tsunami.setReporting(true);
  
  // Allow time for the Tsunami to respond with the version string and
  //  number of tracks.
  delay(100); 

  get_node();

  Serial.println("Ready");
}

//convert control message into commands for Tsunami
/*
  Prototype input receives messages from serial instead of brain.
  Assume this will be rewritten to handle control messages from the brain.
  Requires a 4 digit integer
  0 = command
  123 = track #
*/
void handle_ctrl_msg () {
  ctrl_msg.command = input_num[0];
  ctrl_msg.channels[0]= input_num[1]*100 + input_num[2]*10 + input_num[3];
    // clear the string:
    input_digit= 0;
    input_num [0]= 0 ;
    input_num [1]= 0 ;
    input_num [2]= 0 ;
    input_num [3]= 0 ;
}


void do_command () {
  switch (ctrl_msg.command) {
    //Change the music playing

    case SETAUDIO :
      if (DEBUG)
          Serial.println("SetAudio");
      for (int ch = 0; ch < 18; ch++) {
        if (ctrl_msg.channels[ch]) {
          if (channels[ch]){
            tsunami.trackFade(channels[ch], -70, 1000, true);
          }
          channels[ch]= ctrl_msg.channels[ch];
          ch_loop[ch]= ctrl_msg.bool_loop;
          tsunami.trackGain(channels[ch], ch_gain[ch]);
          tsunami.trackLoad(channels[ch], 0, true);
          if (DEBUG) {
            Serial.print("Gain ");
            Serial.println(ch_gain[ch]);
          }
          Serial.print("Now Playing ");
          Serial.println(channels[ch]);

        }
      }
      tsunami.resumeAllInSync();
      break;

    //I think this command is really just a subset of the previous command
    case SETAUDIOCH :
      Serial.println("case SetAudioCH");
      break;

    //change gain 
    //The range for gain is -70 to +10. A value of 0 (no gain) plays the track at the nominal value in the wav file. 
    case SETVOLCH :
      Serial.println("case SetVolCH");
      for (int channel = 0; channel < 18; channel++) {
        if (ctrl_msg.channels[channel]) {
          tsunami.trackFade(channels[channel], ctrl_msg.channels[channel], ctrl_msg.fade_speed, false);
        }
      }
      break;

    //Instantly mute all audio on node.
    case MUTEALLAUDIO :
      Serial.println("case MuteAllAudio");
      tsunami.stopAllTracks();
      break;
  } 
}

void loop() {
  if (new_ctrl_msg) {
    handle_ctrl_msg();
    do_command();

    new_ctrl_msg = false;
  }
  
  //tsunami.trackPlayPoly(6, 0, true);     // Start Track 6
  //tsunami.trackFade(6, -70, 2000, true);  // Fade Track 6 to 0dB over 2 sec  
  tsunami.update();
  for (int channel = 0; channel < 18; channel++) {
    if(!(tsunami.isTrackPlaying(channels[channel]))) {
      channels[channel]= 0;
    }
  }
}


/*
  Prototype input receives messages from serial instead of brain.
  Requires a 4 digit integer
  0 = command
  123 = track #
*/
void serialEvent() {  
  while (Serial.available()) {
      // get the new byte:
     char inChar = (char)Serial.read();

     if (inChar == '\n') { 
      new_ctrl_msg = true;
      break; 
     } else {
      input_num[input_digit]= (int)(inChar - '0');
      input_digit++;
     }
  }
}


