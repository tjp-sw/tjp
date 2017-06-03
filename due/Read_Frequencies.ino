//  Converts the spectrum analyzer analog input into 7 digital chunchs per 
//  Stereo channel.  0 is quitest, 255 loudest.


void Read_Frequencies(){
  //Read sound frequencies for each band and channel.
  //  Each channel has 7 frequencie "buckets"  0 - 6.
  //  var Frequencies_One[0] is channel 1 lowest frequency
  //  var Frequencies_Two[6] is channel 2 highest frequency
  
  for (freq_amp = 0; freq_amp<7; freq_amp++)
  {
  //  Frequencies_One[freq_amp] = analogRead(DC_One);
    delay(1);
    Frequencies_One[freq_amp] = analogRead(DC_One);
    Frequencies_Two[freq_amp] = analogRead(DC_Two);
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

    
    digitalWrite(STROBE, HIGH);
//   delay(10);
    digitalWrite(STROBE, LOW);
 //  delay(10);
  }
}
