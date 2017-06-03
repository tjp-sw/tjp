/*
Simple test routine, it runs a red dot through the strips.
 */

void run_dot() {   
  uint16_t pixelOffset = loop_count % NUM_LEDS;

/*    
  for(int i = 0; i < NUM_LEDS; i++)
  {
    if (pixelOffset == i)
      leds_node[i] = CRGB::Red;
    else
      leds_node[i] = CRGB::Black;
  }
*/

  // faster
  leds_node_all(0, pixelOffset-1) = CRGB::Black;
  leds_node_all[pixelOffset] = CRGB::Red;
  leds_node_all(pixelOffset+1, LEDS_PER_NODE) = CRGB::Black;
}

