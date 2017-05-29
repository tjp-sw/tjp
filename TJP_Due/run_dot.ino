/*
Simple test routine, it runs a red dot through the strips.
 */

void run_dot() {   
  if(loopCount >= NUM_LEDS)
    loopCount = 0;
    
  for(int i= 0; i<NUM_LEDS; i++)
  {
    if (loopCount == i)
      leds[i] = CRGB::Red;
    else
      leds[i] = CRGB::Black;
   } 
}

