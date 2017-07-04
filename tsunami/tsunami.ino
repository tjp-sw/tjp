#include <Tsunami.h>            // Include the Tsunami library header
#include <stdlib.h>


#define SETAUDIO 1
#define SETVOL 2
#define MUTEALLAUDIO 3
#define DEBUG 1

Tsunami tsunami;                // Our Tsunami object
boolean new_ctrl_msg = false;  // whether a new message exists to process
int channels[18] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
bool channel_loop[ 18 ] = { false };
int node = 0;

struct control_message {
  int node;           //nodes that will perform the command
  int command;        //Command to perform
  int channels[18];   //Array of tracks to change
  int gain[18];           //Optional volume
  int fade_speed;     //speed to change volume
  bool bool_loop;          //does the audio loop?
};

int input_field = 0;
int which_field = 0;
String input_string = "";

control_message ctrl_msg = {0,0,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},0,0};

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
}

void do_command () {
  switch (ctrl_msg.command) {
    //Change the music playing
    case SETAUDIO :
      if (DEBUG)
          Serial.println("case SetAudio");
      for (int ch = 0; ch < 18; ch++) {
        if (ctrl_msg.channels[ch]) {
          //tsunami.trackPlayPoly(ctrl_msg.channels[ch], 0, true);
          tsunami.samplerateOffset(0, 0);        // Reset sample rate offset to 0
          tsunami.masterGain(0, 0);              // Reset the master gain to 0dB          
          tsunami.trackLoad(ctrl_msg.channels[ch], 0, true);
          channels[ch]= ctrl_msg.channels[ch];
          if (DEBUG) {
            Serial.print("Now Playing ");
            Serial.println(ctrl_msg.channels[ch]);
          }
        }
      }
      tsunami.resumeAllInSync();
      break;

    //change gain 
    //The range for gain is -70 to +10. A value of 0 (no gain) plays the track at the base volume of the wav file. 
    case SETVOL :
      if (DEBUG)
        Serial.println("case SetVol");
      for (int channel = 0; channel < 18; channel++) {
        if (ctrl_msg.channels[channel]) {
          tsunami.trackFade(channels[channel], ctrl_msg.channels[channel], ctrl_msg.fade_speed, false);
        }
      }
      break;

    //Instantly mute all audio on node.
    case MUTEALLAUDIO :
      
      tsunami.stopAllTracks();
      break;
  } 
}

void loop() {
  if (new_ctrl_msg) {
//    handle_ctrl_msg();
    do_command();

    new_ctrl_msg = false;
  }
  
  //tsunami.trackPlayPoly(6, 0, true);     // Start Track 6
  //tsunami.trackFade(6, -70, 2000, true);  // Fade Track 6 to 0dB over 2 sec  
  tsunami.update();
  for (int channel = 0; channel < 18; channel++) {
    /*if (channels[channel] > 0) {
      Serial.print ("channel ");
      Serial.print (channel);
      Serial.print (">");
      Serial.println (channels[channel]);
    }*/
    if(!(tsunami.isTrackPlaying(channels[channel]))) {
      channels[channel]= 0;
      /*if (DEBUG>1) {
        Serial.print("Ended ");
        Serial.println(channels[channel]);
      }*/
    } else {
     if (DEBUG>1) {
        Serial.print("Still playing ");
        Serial.println(channels[channel]);
      } 
    }
  }
}

int msg_position = 0;
String ch_string = "";
void serialEvent() {  
  while (Serial.available()) {
      // get the new byte:
     char inChar = (char)Serial.read();

     if (inChar == '\n') { 
        if (DEBUG)
          Serial.println("New Message");
        which_field=0;
        input_string= "";
        new_ctrl_msg = true;
        break; 
     } else if (inChar == ';') {
        if (DEBUG>1) {
          Serial.print("input ");
          Serial.println(input_string);
        } 
        switch (which_field) {
          case 0 :
            ctrl_msg.node= atoi(input_string.c_str());
            if (DEBUG) {
              Serial.print("Node ");
              Serial.println(ctrl_msg.node);
            }
           break;
            
          case 1 :
            ctrl_msg.command= atoi(input_string.c_str());
            if (DEBUG) {
              Serial.print("Command ");
              Serial.println(ctrl_msg.command);
            }
            break;
            
          case 2 :
            switch (ctrl_msg.command) {
              case SETAUDIO :
                ctrl_msg.bool_loop = (input_string != '0');
                break;
              case SETVOL :
                ctrl_msg.fade_speed = atoi(input_string.c_str());
                break;
            }
            break;
            
          case 3 :
           for (int ch = 0; ch < 18; ch++) {
              do  {
                ch_string+= input_string[msg_position];
                msg_position++;
              } while ((msg_position < strlen(input_string.c_str())) && (input_string[msg_position] != ','));
              switch (ctrl_msg.command) {
                case SETAUDIO :
                  ctrl_msg.channels[ch] = atoi(ch_string.c_str());
                  ch_string= "";
                  if (DEBUG) {
                    Serial.print("channel ");
                    Serial.print(ch);
                    Serial.print(">");
                    Serial.println(ctrl_msg.channels[ch]);
                  }
                  break;
                case SETVOL :
                  ctrl_msg.gain[ch] = atoi(input_string.c_str());
                  if (DEBUG) {
                    Serial.print("volume ");
                    Serial.print(ch);
                    Serial.print("-");
                    Serial.println(ctrl_msg.gain[ch]);
                  }
                  break;
              }
              ch_string = "";
              msg_position++;
              if (msg_position >= strlen(input_string.c_str())) {  
                break;
              }
            }
            break;
        }
        input_string= "";
        which_field++;
     } else {
      input_string+= inChar;
     }   
  }
  
}


