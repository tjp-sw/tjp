
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
    CRGBSet leds_node(leds_raw[node_number * RINGS_PER_NODE], LEDS_PER_NODE);
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
    Serial.print(leds_raw[node_number*RINGS_PER_NODE+2][i] == CRGB(7,8,9) ? "Match3, " : "oops3, ");
  Serial.println();

  leds_node_all(3*LEDS_PER_RING, 3*LEDS_PER_RING+10) = CRGB(2,3,4);
  for(int i = 0; i <= 10; i++)
    Serial.print(leds_raw[node_number*RINGS_PER_NODE+3][i] == CRGB(2,3,4) ? "Match4, " : "oops4, ");
  Serial.println();

  Serial.flush();
}
#endif

void draw_debug_mode() {
  uint8_t spacing = node_number+1; // Spacing tells us which node the Due thinks it is
  uint8_t offset = loop_count % spacing; // Movement

  leds_node_all = CRGB::Black;
  for(uint8_t i = 0; i < RINGS_PER_NODE; i+=4) {
    for(uint16_t j = 0; j + offset < VISIBLE_LEDS_PER_RING; j+=spacing) {
      leds_node[i][j+offset] = CRGB::Red;
      leds_node[i+1][j+offset] = CRGB::Green;
      leds_node[i+2][j+offset] = CRGB::Blue;
      leds_node[i+3][j+offset] = CRGB(120, 120, 120);
    }
  }
}

