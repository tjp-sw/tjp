
#ifdef DEBUG
// For debugging only, writes all saved variables
void write_to_serial() {
  int num_serial_vals = 0;
  
  #ifdef DEBUG_TIMING
    Serial.print("Timing:");
    num_serial_vals = 6;
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
  Serial.println("Testing LEDs");

  leds_all(0, 10) = CRGB(1,2,3);
  for(int i = 0; i <= 10; i++)
    Serial.print(leds_raw[i] == CRGB(1,2,3) ? "Match1, " : "oops1, ");
  Serial.println();

  leds_all(3*PHYSICAL_LEDS_PER_NODE - 1, 3*PHYSICAL_LEDS_PER_NODE - 11) = CRGB(3,4,5);
  for(int i = 0; i <= 10; i++)
    Serial.print(leds_raw[3*PHYSICAL_LEDS_PER_NODE - 1 - i] == CRGB(3,4,5) ? "Match2, " : "oops2, ");
  Serial.println();


  leds[0](0, 10) = CRGB(6,7,8);
  for(int i = 0; i <= 10; i++)
    Serial.print(leds_raw[1+i] == CRGB(6,7,8) ? "Match3, " : "oops3, ");
  Serial.println();

  leds[0](LEDS_PER_RING-1, LEDS_PER_RING-11) = CRGB(9,10,11);
  for(int i = 0; i <= 10; i++)
    Serial.print(leds_raw[LEDS_PER_RING - i] == CRGB(9,10,11) ? "Match4, " : "oops4, ");
  Serial.println();

  leds[2](LEDS_PER_RING-1, LEDS_PER_RING-11) = CRGB(12,13,14);
  for(int i = 0; i <= 10; i++)
    Serial.print(leds_raw[2*PHYSICAL_LEDS_PER_RING + LEDS_PER_RING - i] == CRGB(12,13,14) ? "Match5, " : "oops5, ");
  Serial.println();

  leds[2](LEDS_PER_RING-1, LEDS_PER_RING-11) = CRGB(15,16,17);
  for(int i = 0; i <= 10; i++)
    Serial.print(leds_raw[LEDS_PER_STRIP - 1 - i] == CRGB(15,16,17) ? "Match6, " : "oops6, ");
  Serial.println();

  leds[3](0, 10) = CRGB(18,19,20);
  for(int i = 0; i <= 10; i++)
    Serial.print(leds_raw[LEDS_PER_STRIP + 1 + i] == CRGB(18,19,20) ? "Match7, " : "oops7, ");
  Serial.println();

  leds[NUM_RINGS-1](0, 10) = CRGB(21,22,23);
  for(int i = 0; i <= 10; i++)
    Serial.print(leds_raw[NUM_NODES*PHYSICAL_LEDS_PER_NODE - LEDS_PER_RING + i] == CRGB(21,22,23) ? "Match8, " : "oops8, ");
  Serial.println();

  leds[NUM_RINGS-1](LEDS_PER_RING-1, LEDS_PER_RING-11) = CRGB(24,25,26);
  for(int i = 0; i <= 10; i++)
    Serial.print(leds_raw[NUM_NODES*PHYSICAL_LEDS_PER_NODE - 1 - i] == CRGB(24,25,26) ? "Match9, " : "oops9, ");
  Serial.println();

  Serial.flush();
}
#endif

void draw_debug_mode() {
  if(loop_count % 4 != 0) return;
  
  uint8_t spacing = node_number <= NUM_NODES ? node_number+2 : 8; // Spacing tells us which node the Due thinks it is
  uint8_t offset = (loop_count/2) % spacing; // Movement

  leds_all = CRGB::Black;

  for(uint8_t i = 0; i < RINGS_PER_NODE; i+=4) {
    for(uint16_t j = 0; j+offset < LEDS_PER_RING; j+=spacing) {
      leds[i][j+offset] = CRGB::Red;
      leds[i+1][j+offset] = CRGB::Green;
      leds[i+2][j+offset] = CRGB::Blue;
      leds[i+3][j+offset] = CRGB(120, 120, 120);
    }
  }
}

