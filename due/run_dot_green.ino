/*
Simple test routine, it runs a green dot through the strips.
 */

void run_dot_green() {   
  uint16_t pixelOffset = loop_count % NUM_LEDS;

  // One dot for whole structure, instead of one dot per node
  leds_all(0, pixelOffset-1) = CRGB::Black;
  leds_all[pixelOffset] = CRGB::Green;
  leds_all(pixelOffset+1, LEDS_PER_NODE) = CRGB::Black;
}

