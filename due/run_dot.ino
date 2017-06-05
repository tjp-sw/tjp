/*
Simple test routine, it runs a red dot through the strips.
 */

void run_dot() {   
  uint16_t pixelOffset = loop_count % NUM_LEDS;

/*    
  for(int i = 0; i < RINGS_PER_NODE; i++)
  {
    for(int j = 0; j < LEDS_PER_RING; j++)
    {
      if (pixelOffset == i * LEDS_PER_RING + j)
        leds_node[i][j] = CRGB::Red;
      else
        leds_node[i][j] = CRGB::Black;
    }
  }
*/

  // faster
  leds_node_all(0, pixelOffset-1) = CRGB::Black;
  leds_node_all[pixelOffset] = CRGB::Red;
  leds_node_all(pixelOffset+1, LEDS_PER_NODE) = CRGB::Black;
}

