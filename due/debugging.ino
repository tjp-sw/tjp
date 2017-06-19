
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

// To help with plugging in strips and troubleshooting.
// Scrolls blocks of pixels around each ring. Rings will be colored RGBWRGBW...
// Number of lit pixels signals which strand it is for the current node
// Number of empty pixels singals which node number it is
void draw_debug_mode() {
  uint8_t ringOffset = node_number < NUM_NODES ? node_number*RINGS_PER_NODE : 0;
  uint8_t unlitPixels = node_number < NUM_NODES ? node_number + 1 : 8;

  for(uint8_t ring = ringOffset; ring < ringOffset + RINGS_PER_NODE; ring++)
  {
    // Clear pixels
    leds[ring] = CRGB::Black;

    // Pick color based on ring
    uint8_t temp = ring % 4;
    CRGB ringColor = temp == 0 ? CRGB::Red : temp == 1 ? CRGB::Green : temp == 2 ? CRGB::Blue : CRGB(120, 120, 120);

    // Determine strand number and number of pixels to light up
    temp = (ring - ringOffset) / 3; // Strand #
    uint8_t litPixels = 1 + temp;

    // Determine period of repeating pattern and get an extended LED count that is a multiple of the period
    uint8_t period = litPixels + unlitPixels;
    uint16_t extendedLEDCount = ((LEDS_PER_RING-1)/period+1)*period;

    // Loop over extended LED count, but don't write the ones that are outside the range of the physical LEDs
    for(uint16_t curPixel = 0; curPixel < extendedLEDCount; curPixel++) {
      uint16_t idx = (curPixel + loop_count/10) % extendedLEDCount; // Add scrolling based off loop_count
      if(idx >= LEDS_PER_RING) continue;

      if(curPixel % period < litPixels) { // This line references curPixel since we are talking about the pattern
        leds[ring][idx] = ringColor; // This line references idx since we are talking about where the pattern gets written
      }
    }
  }
}

