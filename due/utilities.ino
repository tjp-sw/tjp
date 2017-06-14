void overlay() {

  // copy mid layer on top of existing background layer
//  for  (int ring = 0; ring < RINGS_PER_NODE; ring++) {
//    for (int pixel = 0; pixel < LEDS_PER_RING; pixel++) {
//      if (mid_is_set[ring][pixel]) {
//        leds[ring][pixel] = mid[ring][pixel];
//      }
//    }
//  }

  
  // copy sparkle layer on top of existing led layer
  for  (int ring = 0; ring < RINGS_PER_NODE; ring++) {
    for (int pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      if (sparkle_is_set[ring][pixel]) {
        leds[ring][pixel] = sparkle[ring][pixel];
      }
    }
  }
}


//  palette_num from 0 to 2 indicates which of the 3 palettes we're going to use for this animation
//  color_index from 0 to 6 indicates which of the 7 colors in that palette to use
CRGB get_color(int palette_num, int color_index) {

  switch (palette_num)  {   
  case 0:
    return icy_bright[color_index];

  case 1:
    return watermelon[color_index];

  case 2:
    return fruit_loop[color_index];
  }
}


#ifdef DEBUG
// For debugging only, writes all saved variables
void write_to_serial() {
  int num_serial_vals = 0;
  
  #ifdef DEBUG_TIMING
    Serial.print("Timing:");
    num_serial_vals = 4;
  #endif
  
  for(int i = 0; i < num_serial_vals; i++) {
    Serial.print("\t" + String(serial_val[i]));
  }

  if(num_serial_vals > 0)
  {
    Serial.println();
    Serial.flush(); // Guarantees buffer will write if future code hangs
  }
}

// Used to verify multiple views of LEDs all reference the raw leds_raw[][] array correctly.
void testLEDs() {
  /*
    CRGB leds_raw[NUM_RINGS][LEDS_PER_RING];
    CRGBSet leds_all(*leds, NUM_LEDS);
    CRGBSet leds_node(leds_raw[NODE_ID * RINGS_PER_NODE], LEDS_PER_NODE);
    CRGBSet* leds;
 */

  Serial.println("Testing LEDs");

  leds[NUM_RINGS-1](0, 10) = CRGB(1,2,3);
  for(int i = 0; i <= 10; i++)
    Serial.print(leds_raw[NUM_RINGS-1][i] == CRGB(1,2,3) ? "Match1, " : "oops1, ");
  Serial.println();

  leds_all(LEDS_PER_RING, LEDS_PER_RING+10) = CRGB(4,5,6);
  for(int i = 0; i <= 10; i++)
    Serial.print(leds_raw[1][i] == CRGB(4,5,6) ? "Match2, " : "oops2, ");    
  Serial.println();

  leds_node[2](0, 10) = CRGB(7,8,9);
  for(int i = 0; i <= 10; i++)
    Serial.print(leds_raw[NODE_ID*RINGS_PER_NODE+2][i] == CRGB(7,8,9) ? "Match3, " : "oops3, ");
  Serial.println();

  leds_node_all(3*LEDS_PER_RING, 3*LEDS_PER_RING+10) = CRGB(2,3,4);
  for(int i = 0; i <= 10; i++)
    Serial.print(leds_raw[NODE_ID*RINGS_PER_NODE+3][i] == CRGB(2,3,4) ? "Match4, " : "oops4, ");
  Serial.println();

  Serial.flush();
}
#endif
