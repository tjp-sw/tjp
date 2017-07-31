// Use scale_param to convert parameters into the range you actually want. The brain will send all ranged parameters from 0:255 (unsigned), or -128:127 (signed).
// Parameters with a predefined number of values (NUM_COLORS, INTRA_RING_MOTION) don't need to be scaled with these functions.
inline uint8_t scale_param(uint8_t param, uint8_t minVal, uint8_t maxVal) {
  //uint8_t range = maxVal - minVal + 1;
  //Serial.println("signed " + String(param) + " from " + String(minVal) + ":" + String(maxVal) + " = " + String(minVal + param * range / 256));
  return minVal + param * (maxVal - minVal + 1) / 256;
}
inline int8_t scale_param(int8_t param, int8_t minVal, int8_t maxVal) {
  uint8_t range = maxVal - minVal + 1;
  //Serial.println("signed " + String(param) + " from " + String(minVal) + ":" + String(maxVal) + " = " + String(minVal + param * range / 256 + range/2));
  return minVal + param * range / 256 + range/2;
}
inline int16_t scale_param_16(int8_t param, int16_t minVal, int16_t maxVal) {
  int16_t range = maxVal - minVal + 1;
  return minVal + param * range / 256 + range/2;
}

// Use these functions to directly access leds[]. This can be used for EDM and meditation animations.
inline CRGB get_led(uint8_t ring, uint16_t pixel) {
  return leds[get_1d_index(ring, pixel)];
}
inline void set_led(uint8_t ring, uint16_t pixel, CRGB color) {
  leds[get_1d_index(ring, pixel)] = color;
}
// Takes a ring 0:11, pixel 0:407, returns 1d offset into leds[]. Not to be used when drawing layers.
#if STRIPS_PER_NODE == 6
inline uint16_t get_1d_index(uint8_t ring, uint16_t pixel) {
  #ifdef DEBUG
    if(ring >= RINGS_PER_NODE) Serial.println("Error in get_1d_index(" + String(ring) + ", " + String(pixel) + "). Ring out of range.");
    if(pixel >= LEDS_PER_RING) Serial.println("Error in get_1d_index(" + String(ring) + ", " + String(pixel) + "). Pixel out of range.");
  #endif
  
  bool backward_strip = ring < RINGS_PER_NODE/3;
  uint8_t strip = (ring % 2) + 2*(ring/4);
  
  uint16_t pixel_offset = LEDS_PER_STRIP * strip;
  if(backward_strip) {
    // pixel offset handles the strip offset
    // Add LEDS_PER_STRIP-1 to jump to the end of the entire strip (i.e. pixel 0)
    // Subtract 420 LEDs for every 2 rings you go
    // Subtract pixel to get to specific LED
    return pixel_offset + LEDS_PER_STRIP - 1 - PHYSICAL_LEDS_PER_RING*(ring/2) - pixel;
  }
  else {
    // pixel offset handles the strip offset
    // Add 1 to jump over the test LED
    // Add 420 LEDs for every 2 rings you go, ignoring the first 4 rings
    // Add pixel to get to specific LED
    return pixel_offset + 1 + PHYSICAL_LEDS_PER_RING*((ring % (RINGS_PER_NODE/3))/2) + pixel;
  }
}
#elif STRIPS_PER_NODE == 4
inline uint16_t get_1d_index(uint8_t ring, uint16_t pixel) {
  #ifdef DEBUG
    if(ring >= RINGS_PER_NODE) Serial.println("Error in get_1d_index(" + String(ring) + ", " + String(pixel) + "). Ring out of range.");
    if(pixel >= LEDS_PER_RING) Serial.println("Error in get_1d_index(" + String(ring) + ", " + String(pixel) + "). Pixel out of range.");
  #endif
  
  bool backward_strip = ring < RINGS_PER_NODE/2;
  uint8_t strip = ring % 2 + (backward_strip ? 0 : 2);
  
  uint16_t pixel_offset = LEDS_PER_STRIP * strip;
  if(backward_strip) {
    return pixel_offset + LEDS_PER_STRIP - 1 - PHYSICAL_LEDS_PER_RING*(ring/2) - pixel;
  }
  else {
    return pixel_offset + 1 + PHYSICAL_LEDS_PER_RING*((ring - RINGS_PER_NODE/2)/2) + pixel;
  }
}
#endif


// Functions to rotate layers, might be better to just call the rotate ones directly since we always know which layer we want to rotate
inline void move_inter_ring(uint8_t layer, int8_t direction, uint8_t speed) {
  #ifdef DEBUG
    if(layer < MID_LAYER || layer > SPARKLE_LAYER) {
      Serial.println("layer out of bounds in move_inter_ring(" + String(layer) + ", " + String(direction) + ", " + String(speed));
    }
  #endif
  
  if(layer == MID_LAYER) {
    for(uint8_t i = 0; i < speed; i++)
      move_mid_layer_inter_ring(direction);
  }
  else {
    for(uint8_t i = 0; i < speed; i++)
      move_sparkle_layer_inter_ring(direction);
  }
}
inline void move_mid_layer_inter_ring(int8_t direction) {
  uint8_t temp[LEDS_PER_RING];
  if(direction == CW) {
    memcpy(temp, mid_layer[0], LEDS_PER_RING);
    
    for (uint8_t ring = 1; ring < NUM_RINGS; ring++) {
      memcpy(mid_layer[ring-1], mid_layer[ring], LEDS_PER_RING);
    }
    
    memcpy(mid_layer[NUM_RINGS-1], temp, LEDS_PER_RING);
  }
  else {
    memcpy(temp, mid_layer[NUM_RINGS-1], LEDS_PER_RING);
    
    for (int8_t ring = NUM_RINGS-1; ring > 0; ring--) {
      memcpy(mid_layer[ring], mid_layer[ring-1], LEDS_PER_RING);
    }
    
    memcpy(mid_layer[0], temp, LEDS_PER_RING);
  }
}
inline void move_sparkle_layer_inter_ring(int8_t direction) {
  uint8_t temp[LEDS_PER_RING];
  if(direction == CW) {
    memcpy(temp, sparkle_layer[0], LEDS_PER_RING);
    
    for (uint8_t ring = 1; ring < NUM_RINGS; ring++) {
      memcpy(sparkle_layer[ring-1], sparkle_layer[ring], LEDS_PER_RING);
    }
    
    memcpy(sparkle_layer[NUM_RINGS-1], temp, LEDS_PER_RING);
  }
  else {
    memcpy(temp, sparkle_layer[NUM_RINGS-1], LEDS_PER_RING);
    
    for (int8_t ring = NUM_RINGS-1; ring > 0; ring--) {
      memcpy(sparkle_layer[ring], sparkle_layer[ring-1], LEDS_PER_RING);
    }
    
    memcpy(sparkle_layer[0], temp, LEDS_PER_RING);
  }
}



// Rotate layers within rings; this will cause wrap-around pattern mismatches if used on a short repeating pattern.
// These should only be used if there is no pattern, or the pattern exactly lines up with 408 pixels.
inline void move_intra_ring(uint8_t layer, uint8_t ring, int8_t direction, uint8_t speed) {
  #ifdef DEBUG
    if(layer < MID_LAYER || layer > SPARKLE_LAYER) {
      Serial.println("layer out of bounds in move_intra_ring(" + String(layer) + ", " + String(ring) + ", " + String(direction) + ", " + String(speed));
    }
    if(speed > 127) { Serial.println("speed out of bounds in move_intra_ring: " + String(speed)); }
  #endif
  
  if(layer == MID_LAYER) {
   move_mid_layer_intra_ring(ring, direction*speed);
  }
  else {
    move_sparkle_layer_intra_ring(ring, direction*speed);
  }
}
inline void move_mid_layer_intra_ring(uint8_t ring, int16_t offset) {
  uint8_t orig[LEDS_PER_RING];
  memcpy(orig, mid_layer[ring], LEDS_PER_RING);
  
  uint16_t target_pixel = (LEDS_PER_RING + offset) % LEDS_PER_RING;
  for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
    mid_layer[ring][pixel] = orig[target_pixel];
    if(++target_pixel == LEDS_PER_RING) { target_pixel = 0; }
  }
}
inline void move_sparkle_layer_intra_ring(uint8_t ring, int16_t offset) {
  uint8_t orig[LEDS_PER_RING];
  memcpy(orig, sparkle_layer[ring], LEDS_PER_RING);
  
  uint16_t target_pixel = (LEDS_PER_RING + offset) % LEDS_PER_RING;
  for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
    sparkle_layer[ring][pixel] = orig[target_pixel];
    if(++target_pixel == LEDS_PER_RING) { target_pixel = 0; }
  }
}

// This one directly accesses the LEDs array, and should only be used for EDM animations. write_pixel_data(), used for layers, will overwrite what this does
inline void move_leds_intra_ring(uint8_t ring, int16_t offset) {
  CRGB orig[LEDS_PER_RING];
  for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
    orig[pixel] = get_led(ring, pixel);
  }
  
  uint16_t target_pixel = (LEDS_PER_RING + offset) % LEDS_PER_RING;
  for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
    set_led(ring, pixel, orig[target_pixel]);
    if(++target_pixel == LEDS_PER_RING) { target_pixel = 0; }
  }
}

// Hopefully slightly faster blend implementation due to inlining and no return val
inline void tjp_nblend(CRGB& existing, const CRGB& overlay, uint8_t amountOfOverlay) {
  existing.red   = blend8( existing.red,   overlay.red,   amountOfOverlay);
  existing.green = blend8( existing.green, overlay.green, amountOfOverlay);
  existing.blue  = blend8( existing.blue,  overlay.blue,  amountOfOverlay);
}

