/*
Simple test routine, it runs a green dot through the strips.
 */

void run_dot_green(){
   
word i;   
   for( i= 0; i<(NUM_LEDS_PER_STRIP * 4); i++)
   {
    if (cntr_test == i){
      leds_node[i] = CRGB::Green;
    }
    else
    leds_node[i] = CRGB::Black;
   }
   if (cntr_test >= (NUM_LEDS_PER_STRIP * 4)) {
      cntr_test = 0; } else
      {
        cntr_test = cntr_test + 1;
      }
   

}
