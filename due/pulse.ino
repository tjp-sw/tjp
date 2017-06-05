/*
Simple test routine, it pulses with music.
 */

void pulse() {
  /*
  for(int i = 0; i < NUM_LEDS; i++)
  {
    leds_all[i].setRGB(0, 0, frequencies_one[3]);
  }
  */

  // A faster way
  leds_all = CRGB(0, 0, frequencies_one[3]);

  // This way works too
  //for(int i = 0; i < NUM_RINGS; i++)
  //  leds[i](0, LEDS_PER_RING) = temp;
}
