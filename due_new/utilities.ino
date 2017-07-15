// Converts 0:255 parameters into the minVal:maxVal expected range
uint8_t scale_param(uint8_t param, uint8_t minVal, uint8_t maxVal) {
  uint8_t range = maxVal - minVal + 1;
  //Serial.println("signed " + String(param) + " from " + String(minVal) + ":" + String(maxVal) + " = " + String(minVal + param * range / 256));
  return minVal + param * range / 256;
}

// Converts -128:127 parameters into the minVal:maxVal expected range
int8_t scale_param(int8_t param, int8_t minVal, int8_t maxVal) {
  uint8_t range = maxVal - minVal + 1;
  //Serial.println("signed " + String(param) + " from " + String(minVal) + ":" + String(maxVal) + " = " + String(minVal + param * range / 256 + range/2));
  return minVal + param * range / 256 + range/2;
}


// Takes a ring 0:11, pixel 0:407, returns 1d offset into leds[]. Not to be used when drawing layers.
uint16_t get_1d_index(uint8_t ring, uint16_t pixel) {
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


// Rotates all rings, directly accessing the leds[] array; USES >1200 BYTES OF RAM :o!!!
void rotate_leds(uint8_t flat_offset, uint8_t increment_per_ring, boolean clockwise) {
  CRGB orig[LEDS_PER_RING];
  for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
    memcpy(orig, &leds[get_1d_index(ring, 0)], LEDS_PER_RING);

    uint16_t offset = increment_per_ring * (node_number*RINGS_PER_NODE + ring) + flat_offset;
    offset %= LEDS_PER_RING;
    if(clockwise) { offset = LEDS_PER_RING - offset; }
    
    uint16_t target_pixel = offset;
    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      leds[get_1d_index(ring, target_pixel)] = orig[pixel];
      if(++target_pixel == LEDS_PER_RING) { target_pixel = 0; }
    }
  }
}


