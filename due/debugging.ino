
// To help with plugging in strips and troubleshooting. This does not use layers or palettes.
// Scrolls blocks of pixels around each ring. Rings will be colored RGBWRGBW...
// Number of lit pixels signals which strand it is for the current node
// Number of empty pixels singals which node number it is
inline void draw_debug_mode() {  
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
    unlitPixels = 12;
    throttle = 16;
  }

  // Clear all LEDs
  leds_all = CRGB::Black;

  
  for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++)
  {
    // Pick color based on ring
    uint8_t temp = (ring + ringOffset) % 4;
    CRGB ringColor = temp == 0 ? CRGB::Red : temp == 1 ? CRGB::Green : temp == 2 ? CRGB::Blue : CRGB(90, 90, 90);

    
    // Determine strand number and number of pixels to light up
    uint8_t strand;
    #if STRIPS_PER_NODE == 4
      if(ring < RINGS_PER_NODE/2) {
        strand = ring % 2;
      }
      else {
        strand = 2 + (ring % 2);
      }
    #elif STRIPS_PER_NODE == 6
      strand = (ring % 2) + 2*(ring/4);
    #endif
    
    uint8_t litPixels = 1 + strand;


    // Determine period of the repeating pattern and get an extended LED count that is a multiple of the period
    uint8_t period = litPixels + unlitPixels;
    uint16_t extended_led_count = ((LEDS_PER_RING-1)/period+1)*period;

    // Loop over extended LED count, but don't write the ones that are outside the range of the physical LEDs
    for(uint16_t curPixel = 0; curPixel < extended_led_count; curPixel++) {
      uint16_t idx = (curPixel + loop_count/throttle) % extended_led_count; // Add scrolling based off loop_count
      if(idx >= LEDS_PER_RING) continue;

      if(curPixel % period < litPixels) { // This line references curPixel since we are talking about the pattern
        set_led(ring, idx, ringColor); // This line references idx since we are talking about where the pattern gets written
      }
    }
  }
}

#define TEST_STRANDS_DELAY_INC 500
inline void test_strands() {
  static uint16_t delay_factor = 0;
  for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
    if(ring == (loop_count % RINGS_PER_NODE)) {
      if(ring == 0) {
        delay_factor++;
        #ifdef DEBUG
          Serial.println("delay(" + String(TEST_STRANDS_DELAY_INC * delay_factor) + ")");
        #endif
      }
      for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        set_led(ring, pixel, CRGB::White);//CRGB(128,128,128);
      }
    }
    else {
      for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        set_led(ring, pixel, CRGB::Black);
      }
    }
  }
  
  LEDS.show();
  delay(TEST_STRANDS_DELAY_INC * delay_factor);
}

inline void test_strands_2strands_only() {
  static uint16_t delay_factor = 0;

  uint8_t col_index  = (loop_count/2) % 4; 
  CRGB color = col_index == 0 ? CRGB::White : col_index == 1 ? CRGB::Red : col_index == 2 ? CRGB::Blue : CRGB::Green;
  if(loop_count % 2 == 0) {
    if(delay_factor == 0 || delay_factor >= 60) { delay_factor = 1; }
    else { delay_factor*=2; }
    #ifdef DEBUG
      Serial.println("delay(" + String(TEST_STRANDS_DELAY_INC * delay_factor) + ")");
    #endif
    for(uint16_t pixel = 0; pixel < PHYSICAL_LEDS_PER_RING; pixel++) {
      set_led(6, pixel, color);//CRGB(128,128,128);
      set_led(7, pixel, CRGB::Black);//CRGB(128,128,128);
    }
  }
  else {
    for(uint16_t pixel = 0; pixel < PHYSICAL_LEDS_PER_RING; pixel++) {
      set_led(6, pixel, CRGB::Black);
      set_led(7, pixel, color);
    }
  }

  unsigned long max = millis() + 1000*delay_factor;
  while(millis() < max) {
    LEDS.show();
  }
}

