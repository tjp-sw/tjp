
#ifdef DEBUG_TIMING
// For debugging only, writes all saved variables
void write_timing_output() {
  int num_serial_vals = 0;
  
  #ifdef DEBUG_TIMING
    Serial.print("Timing:");
    num_serial_vals = 9;
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
#endif

// To help with plugging in strips and troubleshooting. This does not use layers or palettes.
// Scrolls blocks of pixels around each ring. Rings will be colored RGBWRGBW...
// Number of lit pixels signals which strand it is for the current node
// Number of empty pixels singals which node number it is
void draw_debug_mode() {  
  uint8_t ringOffset, unlitPixels, throttle;
  
  if(node_number < NUM_NODES) {
    // Node number has been assigned
    ringOffset = node_number*RINGS_PER_NODE;
    unlitPixels = 1 + node_number;
    throttle = 8;
  }
  else {
    // Node number has NOT been assigned
    ringOffset = 0;
    unlitPixels = 8;
    throttle = 16;
  }

  // Clear all LEDs
  leds_all = CRGB::Black;

  
  for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++)
  {
    // Pick color based on ring
    uint8_t temp = (ring + ringOffset) % 4;
    CRGB ringColor = temp == 0 ? CRGB::Red : temp == 1 ? CRGB::Green : temp == 2 ? CRGB::Blue : CRGB::White;

    
    // Determine strand number and number of pixels to light up
    uint8_t strand;
    if(ring < RINGS_PER_NODE/2) {
      strand = ring % 2;
    }
    else {
      strand = 2 + (ring % 2);
    }
    
    uint8_t litPixels = 1 + strand;


    // Determine period of the repeating pattern and get an extended LED count that is a multiple of the period
    uint8_t period = litPixels + unlitPixels;
    uint16_t extended_led_count = ((LEDS_PER_RING-1)/period+1)*period;

    // Loop over extended LED count, but don't write the ones that are outside the range of the physical LEDs
    for(uint16_t curPixel = 0; curPixel < extended_led_count; curPixel++) {
      uint16_t idx = (curPixel + loop_count/throttle) % extended_led_count; // Add scrolling based off loop_count
      if(idx >= LEDS_PER_RING) continue;

      if(curPixel % period < litPixels) { // This line references curPixel since we are talking about the pattern
        leds[get_1d_index(ring, idx)] = ringColor; // This line references idx since we are talking about where the pattern gets written
        //Serial.println(String(ring) + ", " + String(idx));
      }
    }
  }
}

void test_strands() {
  const uint16_t delay_inc = 500;
  static uint16_t delay_factor = 0;
  for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
    if(ring == (loop_count % RINGS_PER_NODE)) {
      if(ring == 0) {
        delay_factor++;
        Serial.println("delay(" + String(delay_inc * delay_factor) + ")");
      }
      for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        leds[get_1d_index(ring, pixel)] = CRGB::White;//CRGB(128,128,128);
      }
    }
    else {
      for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        leds[get_1d_index(ring, pixel)] = CRGB::Black;
      }
    }
  }
  
  LEDS.show();
  delay(delay_inc * delay_factor);
}

