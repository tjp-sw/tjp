/*
Simple test routine, it runs a green dot through the strips.
 */

void run_dot_green() {
  if(loopCount >= NUM_LEDS)
    loopCount = 0;
    
  for(int i = 0; i<NUM_LEDS; i++)
  {
    if (loopCount == i)
      leds[i] = CRGB::Green;
    else
      leds[i] = CRGB::Black;
  }
}
