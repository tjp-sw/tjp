// This file is for completed sparkle animations

//---------------------------------------- SPARKLE GLITTER ---------------------------------------
//  Creates sparkles of glitter randomly all over the structure
// SPARKLE_COLOR_THICKNESS(1-6), SPARKLE_PORTION(1-100), SPARKLE_MAX_DIM(0-6), SPARKLE_RANGE(20-127)
void sparkle_glitter() {
  clear_sparkle_layer();
  
  for (uint8_t ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
    for (uint16_t pixel = HALF_RING-1-SPARKLE_RANGE; pixel < HALF_RING+SPARKLE_RANGE - SPARKLE_COLOR_THICKNESS; pixel++) {
      if (random16(SPARKLE_PORTION * SPARKLE_COLOR_THICKNESS) == 0) {
        uint8_t dim = random8(SPARKLE_MAX_DIM + 1);
        uint8_t sparkle_color = random8(2) == 0 ? get_sparkle_color(0, dim) : get_sparkle_color(1, dim);
        
        for (uint8_t thickness = 0; thickness < SPARKLE_COLOR_THICKNESS; thickness++) {
          set_sparkle(ring, pixel+thickness, sparkle_color);
        }
      }
    }
  }
}


//--------------------------------------- SPARKLE RAIN -------------------------------------------
//  Puts raindrops randomly at the top of the structure and runs them down both sides of each ring.  
// SPARKLE_COLOR_THICKNESS(1-6), SPARKLE_PORTION(1-100), SPARKLE_MAX_DIM(0-6), SPARKLE_RANGE(10-100), SPARKLE_SPAWN_FREQUENCY(1-127), SPARKLE_INTRA_RING_MOTION(-1, 1), SPARKLE_INTRA_RING_SPEED(4-64)
void sparkle_rain() {
  uint8_t throttle = 1;
  uint8_t distance_per_cycle = 1;
  if(SPARKLE_INTRA_RING_SPEED > THROTTLE) {
    distance_per_cycle = SPARKLE_INTRA_RING_SPEED / THROTTLE;
  }
  else if(SPARKLE_INTRA_RING_SPEED < THROTTLE) {
    throttle = THROTTLE / SPARKLE_INTRA_RING_SPEED;
  }

  if(sparkle_count % throttle == 0) {
    // move existing raindrops 1 pixel
    for (uint8_t ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
      if(SPARKLE_INTRA_RING_MOTION == 1) {
        // inner half
        for (uint16_t pixel = LEDS_PER_RING - 1; pixel > HALF_RING + distance_per_cycle; pixel--) { set_sparkle(ring, pixel, get_sparkle(ring, pixel-distance_per_cycle)); }
        for (uint16_t pixel = HALF_RING + distance_per_cycle; pixel > HALF_RING; pixel--)         { set_sparkle(ring, pixel, TRANSPARENT); }
        
        // outer half
        for (uint16_t pixel = 0; pixel < HALF_RING-1 - distance_per_cycle; pixel++)           { set_sparkle(ring, pixel, get_sparkle(ring, pixel+distance_per_cycle)); }
        for (uint16_t pixel = HALF_RING-1 - distance_per_cycle; pixel < HALF_RING-1; pixel++) { set_sparkle(ring, pixel, TRANSPARENT); }
      }
      else if(SPARKLE_INTRA_RING_MOTION == 1) {
        // inner half
        for (uint16_t pixel = HALF_RING; pixel < LEDS_PER_RING-1 - distance_per_cycle; pixel++)       { set_sparkle(ring, pixel, get_sparkle(ring, pixel+distance_per_cycle)); }
        for (uint16_t pixel = LEDS_PER_RING-1 - distance_per_cycle; pixel < LEDS_PER_RING-1; pixel++) { set_sparkle(ring, pixel, TRANSPARENT); }
        
        // outer half
        for (uint16_t pixel = HALF_RING-1; pixel > distance_per_cycle; pixel--) { set_sparkle(ring, pixel, get_sparkle(ring, pixel-distance_per_cycle)); }
        for (uint16_t pixel = distance_per_cycle; pixel > 0; pixel--)           { set_sparkle(ring, pixel, TRANSPARENT); }
      }
    }
  }


  // create new raindrops every "frequency" cycles
  if (sparkle_count % SPARKLE_SPAWN_FREQUENCY == 0) {
    for (uint8_t ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
      if(SPARKLE_INTRA_RING_MOTION == 1) {
        for (uint16_t pixel = HALF_RING - SPARKLE_RANGE; pixel < HALF_RING + SPARKLE_RANGE; pixel++) {
          if (random16(SPARKLE_COLOR_THICKNESS * SPARKLE_PORTION) == 0) {
            spawn_raindrop(ring, pixel);
          }
        }
      }
      else if(SPARKLE_INTRA_RING_MOTION == -1) {
        for(uint16_t pixel = 0; pixel < SPARKLE_RANGE; pixel++) {
          if (random16(SPARKLE_COLOR_THICKNESS * SPARKLE_PORTION) == 0) {
            spawn_raindrop(ring, pixel);
          }
        }
        for(uint16_t pixel = LEDS_PER_RING-1; pixel > LEDS_PER_RING-1 - SPARKLE_RANGE; pixel--) {
          if (random16(SPARKLE_COLOR_THICKNESS * SPARKLE_PORTION) == 0) {
            spawn_raindrop(ring, pixel);
          }
        }
      }
    }
  }
}

void spawn_raindrop(uint8_t ring, uint16_t pixel) {
  uint8_t dim = random8(SPARKLE_MAX_DIM + 1);
  uint8_t sparkle_color = random8(2) == 0 ? get_sparkle_color(0, dim) : get_sparkle_color(1, dim);
  bool is_inner = pixel >= HALF_RING;

  for (uint8_t thickness = 0; thickness < SPARKLE_COLOR_THICKNESS; thickness++) {
    if (is_inner) { set_sparkle(ring, pixel + thickness, sparkle_color); }
    else { set_sparkle(ring, pixel - thickness, sparkle_color); }
  }
}

