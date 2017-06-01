/*
This routine does the actual writing to the LEDs.  It chops up the leds_node array and redirects
the LED info where it belongs. 
 Works, Don't touch 
 */

void write_leds() {
//temp_saved_time = current_time;    
word i; 
word j;  
   for( i= 0; i<NUM_LEDS_PER_STRIP; i++)
   {
    leds_1[i]  = leds_node[i];
   }

   for( i= 0; i<NUM_LEDS_PER_STRIP; i++)
   {
    leds_2[i]  = leds_node[i+NUM_LEDS_PER_STRIP];
   }

   for( i= 0; i<NUM_LEDS_PER_STRIP; i++)
   {
    leds_3[i]  = leds_node[i+(NUM_LEDS_PER_STRIP*2)];
   }

   for( i= 0; i<NUM_LEDS_PER_STRIP; i++)
   {
    leds_4[i]  = leds_node[i+(NUM_LEDS_PER_STRIP*3)];
   }

temp_current_time = millis();  
temp_saved_time = temp_current_time; 
FastLED.show();
temp_current_time = millis(); 
temp[2] = temp_current_time - temp_saved_time;

}
