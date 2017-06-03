//  Converts the spectrum analyzer analog input into 7 digital chunchs per 
//  Stereo channel.  0 is quitest, 255 loudest.


void read_frequencies(){
  //Read sound frequencies for each band and channel.
  //  Each channel has 7 frequency "buckets"  0 - 6.
  //  var frequencies_one[0] is channel 1 lowest frequency
  //  var frequencies_two[6] is channel 2 highest frequency
  
  for(int freq_amp = 0; freq_amp<NUM_CHANNELS; freq_amp++)
  {
    frequencies_one[freq_amp] = analogRead(SS_PIN_DC_ONE);
    //delayMicroseconds(100);
    frequencies_two[freq_amp] = analogRead(SS_PIN_DC_TWO);
    //delayMicroseconds(100);


    // Calibrate intensity - Channel 1  --------------------------------//
    if (frequencies_one[freq_amp] >= 100){
     frequencies_one[freq_amp] = frequencies_one[freq_amp] - 100;
    } else {
     frequencies_one[freq_amp] = 0;
    }
    
    //frequencies_one[freq_amp].  Tweak it till you like it.
    frequencies_one[freq_amp] = frequencies_one[freq_amp]/2;
    //frequencies_one[freq_amp] = frequencies_one[freq_amp]*2;
    //frequencies_one[freq_amp] = frequencies_one[freq_amp]/1;
    
    
    // Calibrate intensity - Channel 2  --------------------------------//  
    if (frequencies_two[freq_amp] >= 100){
      frequencies_two[freq_amp] = frequencies_two[freq_amp] - 100;
    } else {
     frequencies_two[freq_amp] = 0;
    }

    //frequencies_two[freq_amp] = frequencies_two[freq_amp]/4;
    frequencies_two[freq_amp] = frequencies_two[freq_amp]/2;
    //frequencies_two[freq_amp] = frequencies_two[freq_amp]*8;

    frequencies_avg[freq_amp] = frequencies_one[freq_amp]/2 + frequencies_two[freq_amp]/2;
    
    // Acknowledging the read?
    digitalWrite(SS_PIN_STROBE, HIGH);
    //delay(10);
    digitalWrite(SS_PIN_STROBE, LOW);
    //delay(10);

    #ifdef DEBUG_TIMING
      unsigned long now = millis();
      serial_val[0] = now - current_time;
      last_debug_time = now;
    #endif
    
    #ifdef DEBUG_SPECTRUM
      for(int i = 0; i < NUM_CHANNELS; i++)
      {
        serial_val[i] = frequencies_one[i];
        serial_val[NUM_CHANNELS+i] = frequencies_two[i];
      }
    #endif
  }
}

