/*
Simple test routine, it pulses with music.
 */

void pulse(){
   
word i;   
   for( i= 0; i<(NUM_LEDS_PER_STRIP * 4); i++)
   {

    leds_node[i].setRGB(0,0,Frequencies_One[3]);
      }
   

}
