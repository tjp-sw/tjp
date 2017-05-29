//  Converts the spectrum analyzer analog input into 7 digital chunchs per 
//  Stereo channel.  0 is quitest, 255 loudest.


void Read_Frequencies(){
  //Read sound frequencies for each band and channel.
  //  Each channel has 7 frequency "buckets"  0 - 6.
  //  var Frequencies_One[0] is channel 1 lowest frequency
  //  var Frequencies_Two[6] is channel 2 highest frequency
  
  for (int freq_amp = 0; freq_amp<7; freq_amp++)
  {
  //  Frequencies_One[freq_amp] = analogRead(DC1);
    delay(1);
    Frequencies_One[freq_amp] = analogRead(SS_PIN_DC1);
    Frequencies_Two[freq_amp] = analogRead(SS_PIN_DC2);
    if (Frequencies_One[freq_amp] >= 100){
     Frequencies_One[freq_amp] = Frequencies_One[freq_amp] - 100;
    } else {
     Frequencies_One[freq_amp] = 0;
    }
 //  dave  the next line is where you claibrate intensity.  you are adjusting the raw analalog input represented by the var 
 //  Frequencies_One[freq_amp].  Tweak it till you like it.
    Frequencies_One[freq_amp] = Frequencies_One[freq_amp]/2;
 //  Frequencies_One[freq_amp] = Frequencies_One[freq_amp]*2;
//   Frequencies_One[freq_amp] = Frequencies_One[freq_amp]/1;
    if (Frequencies_Two[freq_amp] >= 100){
      Frequencies_Two[freq_amp] = Frequencies_Two[freq_amp] - 100;
    } else {
     Frequencies_Two[freq_amp] = 0;
    }
//    Frequencies_Two[freq_amp] = Frequencies_Two[freq_amp]/4;
    Frequencies_Two[freq_amp] = Frequencies_Two[freq_amp]/2;
  //  Frequencies_Two[freq_amp] = Frequencies_Two[freq_amp]*8;

    
    digitalWrite(SS_PIN_STROBE, HIGH);
//   delay(10);
    digitalWrite(SS_PIN_STROBE, LOW);
 //  delay(10);

    #ifdef DEBUG_TIMING
      serialVal[0] = millis() - current_time;
    #endif
    #ifdef DEBUG_SPECTRUM
      serialVal[0] = Frequencies_One[0];
      serialVal[1] = Frequencies_One[1];
      serialVal[2] = Frequencies_One[2]; 
      serialVal[3] = Frequencies_One[3]; 
      serialVal[4] = Frequencies_One[4];
      serialVal[5] = Frequencies_Two[0];
      serialVal[6] = Frequencies_Two[1];
      serialVal[7] = Frequencies_Two[2]; 
      serialVal[8] = Frequencies_Two[3]; 
      serialVal[9] = Frequencies_Two[4];
    #endif
  }
}
