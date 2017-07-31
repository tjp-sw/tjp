#include <Tsunami.h>            // Include the Tsunami library header
#include <stdlib.h>


#define SETAUDIO 1
#define SETVOL 2
#define MUTEALLAUDIO 3

#ifdef DEBUG
  #define DEBUG_LEVEL 1
#else
  #define DEBUG_LEVEL 2
#endif

Tsunami tsunami;                // Our Tsunami object
//variables tracking currently playing song data
boolean new_ctrl_msg = false;
int channels[ 18 ] = { 0 };
bool ch_loop[ 18 ] = { false };
int ch_gain[ 18 ] = { 0 };


//variables for control message input
unsigned int msg_position = 0;

int which_field = 0;
String ch_string = "";
String input_string = "";

struct control_message {
  int node;           //nodes that will perform the command
  int command;        //Command to perform
  int channels[18];   //Array of tracks to change
  int gain[18];       //Optional volume
  int fade_speed;     //speed to change volume
  bool bool_loop;     //does the audio loop?
};
#define EMPTY_CTRL_MSG {0,0,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},0,0}

control_message ctrl_msg = EMPTY_CTRL_MSG;

void setup_tsunami () {
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
  Serial.println("Tsunami Ready");
}


void handle_command(const char command[]) {
    String cmd_str (command);
    //remote.write(cmd_str.c_str());
    if (DEBUG_LEVEL>1)
        Serial.println(cmd_str);

    for( unsigned int letter = 0; letter < cmd_str.length(); letter = letter + 1 ) {
        char inChar = cmd_str[letter];

         if (inChar == ';') {
            if (DEBUG_LEVEL>1) {
              Serial.print("input_string=");
              Serial.println(input_string);
            }
            switch (which_field) {
              case 0 :
                ctrl_msg.node= atoi(input_string.c_str());
                if (DEBUG_LEVEL) {
                  Serial.print("Node ");
                  Serial.println(ctrl_msg.node);
                }
               break;

              case 1 :
                ctrl_msg.command= atoi(input_string.c_str());
                if (DEBUG_LEVEL) {
                  Serial.print("Command ");
                  Serial.println(ctrl_msg.command);
                }
                break;

              case 2 :
                switch (ctrl_msg.command) {
                  case SETAUDIO :
                    ctrl_msg.bool_loop = (input_string.charAt(0) != '0');
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
                      if (DEBUG_LEVEL) {
                        Serial.print("channel ");
                        Serial.print(ch);
                        Serial.print(">");
                        Serial.println(ctrl_msg.channels[ch]);
                      }
                      break;
                    case SETVOL :
                      ctrl_msg.gain[ch] = atoi(ch_string.c_str());
                      ch_string= "";
                      if (DEBUG_LEVEL) {
                        Serial.print("volume ");
                        Serial.print(ch);
                        Serial.print(">");
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
  msg_position = 0;
  which_field = 0;
  ch_string = "";
  input_string= "";
  if ((ctrl_msg.node == node_number) || (ctrl_msg.node == 0)){
    new_ctrl_msg = true;
    if (DEBUG_LEVEL>1)
      Serial.println("New Message");
      do_command ();
  } else {
    ctrl_msg = {0,0,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},0,0};
  }
}


void do_command () {
  if (DEBUG_LEVEL) {
      Serial.println("---Control Message---");
      Serial.print("Node ");
      Serial.println(ctrl_msg.node);
      Serial.print("Command ");
      Serial.println(ctrl_msg.command);
  }
  for (int x=0; x<18; x++) {
    if (ctrl_msg.channels[x]) {
      Serial.print("channels[");
      Serial.print(x);
      Serial.print("] ");
      Serial.println(ctrl_msg.channels[x]);
    }
  }
  for (int x=0; x<18; x++) {
    if (ctrl_msg.gain[x]) {
      Serial.print("gain[");
      Serial.print(x);
      Serial.print("] ");
      Serial.println(ctrl_msg.gain[x]);
    }
  }
  if (ctrl_msg.fade_speed){
    Serial.print("fade_speed ");
    Serial.println(ctrl_msg.fade_speed);
  }
  if (ctrl_msg.command == SETAUDIO){
    Serial.print("Looping ");
    Serial.println(ctrl_msg.bool_loop);
  }
  Serial.println("--------------------");

  switch (ctrl_msg.command) {
    //Change the music playing
    case SETAUDIO :
      if (DEBUG_LEVEL)
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
          if (DEBUG_LEVEL) {
            Serial.print("Gain ");
            Serial.println(ch_gain[ch]);
          }
          Serial.print("Now Playing ");
          Serial.println(channels[ch]);
        }
      }
      tsunami.samplerateOffset(0, 0);        // Reset sample rate offset to 0
      tsunami.masterGain(0, 0);              // Reset the master gain to 0dB
      tsunami.resumeAllInSync();
      break;

    //change gain
    //The range for gain is -70 to +10. A value of 0 (no gain) plays the track at the base volume of the wav file.
    case SETVOL :
      if (DEBUG_LEVEL)
        Serial.println("SetVol");
      for (int ch = 0; ch < 18; ch++) {
        if (ctrl_msg.gain[ch]) {
            ch_gain[ch]= ctrl_msg.gain[ch];
          if (ch_gain[ch] == -70) {
            tsunami.trackFade(channels[ch], ch_gain[ch], ctrl_msg.fade_speed, true);
            channels[ch]= 0;
            ch_loop[ch]= false;
          } else {
            tsunami.trackFade(channels[ch], ch_gain[ch], ctrl_msg.fade_speed, false);
          }
          if (DEBUG_LEVEL) {
            Serial.print("Fading Channel ");
            Serial.print(ch);
            Serial.print(">");
            Serial.println(ch_gain[ch]);
          }
          delay(10);
        }
      }
      break;

    //Instantly mute all audio on node.
    case MUTEALLAUDIO :
      Serial.println("MuteAllAudio");
      tsunami.stopAllTracks();
      memset(ch_loop,0,sizeof(ch_loop));
      memset(channels,0,sizeof(channels));
      memset(ch_gain,0,sizeof(ch_gain));
      break;
  }
  ctrl_msg = EMPTY_CTRL_MSG;
}

void do_tsunami() {
  delay(10);
  tsunami.update();

  for (int ch = 0; ch < 18; ch++) {
    if(!(tsunami.isTrackPlaying(channels[ch]))) {
      if (ch_loop[ch]){
        Serial.print("Replaying ");
        Serial.println(channels[ch]);
        tsunami.trackGain(channels[ch], ch_gain[ch]);
        tsunami.trackLoad(channels[ch], 0, true);
        String msg = "sR" + String (channels[ch]);
        remote.write(msg.c_str());
      } else {
        if (channels[ch]) {
          Serial.print("Ending ");
          Serial.println(channels[ch]);
        String msg = "sE" + String (channels[ch]);
        remote.write(msg.c_str());
        }
        channels[ch]= 0;
        ch_gain[ch]=0;
      }
    } else {
     if (DEBUG_LEVEL>2) {
        Serial.print("Still playing ");
        Serial.println(channels[ch]);
      }
    }
  }
  tsunami.samplerateOffset(0, 0);        // Reset sample rate offset to 0
  tsunami.masterGain(0, 0);              // Reset the master gain to 0dB
  tsunami.resumeAllInSync();
}

void serialEvent() {
  while (Serial.available()) {
      // get the new byte:
     char inChar = (char)Serial.read();

     if (inChar == '\n') {
        msg_position = 0;
        which_field=0;
        ch_string = "";
        input_string= "";
        if ((ctrl_msg.node == node_number) || (ctrl_msg.node == 0)){
          new_ctrl_msg = true;
          if (DEBUG_LEVEL>1)
            Serial.println("New Message");
            do_command ();
        } else {
          ctrl_msg = EMPTY_CTRL_MSG;
        }
        break;
     } else if (inChar == ';') {
        if (DEBUG_LEVEL>1) {
          Serial.print("input_string=");
          Serial.println(input_string);
        }
        switch (which_field) {
          case 0 :
            ctrl_msg.node= atoi(input_string.c_str());
            if (DEBUG_LEVEL) {
              Serial.print("Node ");
              Serial.println(ctrl_msg.node);
            }
           break;

          case 1 :
            ctrl_msg.command= atoi(input_string.c_str());
            if (DEBUG_LEVEL) {
              Serial.print("Command ");
              Serial.println(ctrl_msg.command);
            }
            break;

          case 2 :
            switch (ctrl_msg.command) {
              case SETAUDIO :
                ctrl_msg.bool_loop = (input_string.charAt(0) != '0');
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
                  if (DEBUG_LEVEL) {
                    Serial.print("channel ");
                    Serial.print(ch);
                    Serial.print(">");
                    Serial.println(ctrl_msg.channels[ch]);
                  }
                  break;
                case SETVOL :
                  ctrl_msg.gain[ch] = atoi(ch_string.c_str());
                  ch_string= "";
                  if (DEBUG_LEVEL) {
                    Serial.print("volume ");
                    Serial.print(ch);
                    Serial.print(">");
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

