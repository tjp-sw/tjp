/*
Simple test routine, it pulses with music.
 */

void pulse() {
  /*
  for(int i = 0; i < NUM_LEDS; i++)
  {
    leds[i].setRGB(0, 0, frequencies_one[3]);
  }
  */

  // A faster way
  CRGB temp = CRGB(0, 0, frequencies_one[3]);
  leds_node(0, LEDS_PER_NODE) = temp;
}
