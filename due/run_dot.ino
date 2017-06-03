/*
Simple test routine, it runs a red dot through the strips.
 */

void run_dot() {   
  uint16_t pixelOffset = loop_count % NUM_LEDS;

/*    
  for(int i = 0; i < NUM_LEDS; i++)
  {
    if (pixelOffset == i)
      leds[i] = CRGB::Red;
    else
      leds[i] = CRGB::Black;
  }
*/
unsigned long now = millis();
  // faster
  leds_node(0, pixelOffset-1) = CRGB::Black;
  leds_node[pixelOffset] = CRGB::Red;
  leds_node(pixelOffset+1, LEDS_PER_NODE) = CRGB::Black;
}

