/*
Simple test routine, it pulses with music.
 */

void pulse() {
  
  for(int i = 0; i < NUM_LEDS; i++)
  {
    leds_all[i].setRGB(0, 0, frequencies_one[3]);
  }
  
/*
  // A faster way
//  for(uint8_t i = 0; i < RINGS_PER_NODE; i++)
//    leds[i] = CRGB(0, 0, frequencies_one[i]);

  // This way works too
  for(int i = 0; i < NUM_RINGS; i++)
    leds[i](0, LEDS_PER_RING) = temp;
*/
}


/*
Simple test routine, it runs a red dot through the strips.
 */

void run_dot() {
  uint16_t pixelOffset = loop_count % LEDS_PER_NODE;
  if(pixelOffset > 0) leds_node_all(0, pixelOffset-1) = CRGB::Black;
  leds_node_all[pixelOffset] = CRGB::Red;
  if(pixelOffset < LEDS_PER_NODE - 1) leds_node_all(pixelOffset+1, LEDS_PER_NODE) = CRGB::Black;
}

/*
Simple test routine, it runs a green dot through the strips.
 */

void run_dot_green() {   
  uint16_t pixelOffset = loop_count % NUM_LEDS;
  // One dot for whole structure, instead of one dot per node
  leds_all = CRGB::Black;
  leds_all[pixelOffset] = CRGB::Green;
}

