/*
Simple test routine, it pulses with music.
 */

void pulse() {  
  for(int i= 0; i<NUM_LEDS; i++)
  {
    leds[i].setRGB(0,0,Frequencies_One[3]);
  }
}
