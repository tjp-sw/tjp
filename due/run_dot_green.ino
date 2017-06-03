/*
Simple test routine, it runs a green dot through the strips.
 */

void run_dot_green() {   
  uint16_t pixelOffset = loop_count % NUM_LEDS;

  leds_node(0, pixelOffset-1) = CRGB::Black;
  leds_node[pixelOffset] = CRGB::Green;
  leds_node(pixelOffset+1, LEDS_PER_NODE) = CRGB::Black;
}

