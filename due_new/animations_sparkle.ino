// This file is for completed sparkle animations

//---------------------------------------- SPARKLE GLITTER ---------------------------------------
//  Creates sparkles of glitter randomly all over the structure
// SPARKLE_COLOR_THICKNESS(1:2), SPARKLE_PORTION(16:100), SPARKLE_MAX_DIM(0:6), SPARKLE_RANGE(40:204)
void sparkle_glitter() {
  uint8_t color_thickness = scale_param(SPARKLE_COLOR_THICKNESS, 1, 2);
  uint8_t portion = scale_param(SPARKLE_PORTION, 16, 100);
  uint8_t max_dim = scale_param(SPARKLE_MAX_DIM, 0, 6);
  uint8_t range = scale_param(SPARKLE_RANGE, 40, 204);
  
  clear_sparkle_layer();
  
  uint8_t lower_limit = HALF_RING-range;
  uint16_t upper_limit = HALF_RING+range - color_thickness;
  
  for (uint8_t ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
    for (uint16_t pixel = lower_limit; pixel < upper_limit; pixel++) {
      if (random16(portion * color_thickness) == 0) {
        uint8_t dim = random8(max_dim + 1);
        uint8_t sparkle_color = random8(2) == 0 ? get_sparkle_color(0, dim) : get_sparkle_color(1, dim);
        
        for (uint8_t thickness = 0; thickness < color_thickness; thickness++) {
          set_sparkle(ring, pixel+thickness, sparkle_color);
        }
      }
    }
  }
}


//--------------------------------------- SPARKLE RAIN -------------------------------------------
//  Puts raindrops randomly at the top of the structure and runs them down both sides of each ring.  
// SPARKLE_COLOR_THICKNESS(1:2), SPARKLE_PORTION(25:75), SPARKLE_MAX_DIM(0:6), SPARKLE_RANGE(20), SPARKLE_SPAWN_FREQUENCY(20), SPARKLE_INTRA_RING_MOTION(-1:1), SPARKLE_INTRA_RING_SPEED(16:64)
void sparkle_rain() {
  uint8_t color_thickness = scale_param(SPARKLE_COLOR_THICKNESS, 1, 2);
  uint8_t portion = scale_param(SPARKLE_PORTION, 25, 75);
  uint8_t max_dim = scale_param(SPARKLE_MAX_DIM, 0, 6);
  uint8_t range = 20;//scale_param(SPARKLE_RANGE, 20, 20);
  uint8_t spawn_frequency = 20;//scale_param(SPARKLE_SPAWN_FREQUENCY, 20, 20);
  uint8_t intra_speed = scale_param(SPARKLE_INTRA_RING_SPEED, 16, 64);
  
  uint8_t throttle = 1;
  uint8_t distance_per_cycle = 1;
  if(intra_speed > THROTTLE) {
    distance_per_cycle = intra_speed / THROTTLE;
  }
  else if(intra_speed < THROTTLE) {
    throttle = THROTTLE / intra_speed;
  }

  if(sparkle_count % throttle == 0) {
    // move existing raindrops
    for (uint8_t ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
      if(SPARKLE_INTRA_RING_MOTION == DOWN) {
        // inner half
        for (uint16_t pixel = LEDS_PER_RING - 1; pixel > HALF_RING + distance_per_cycle; pixel--)  { set_sparkle(ring, pixel, get_sparkle(ring, pixel-distance_per_cycle)); }
        for (uint8_t pixel = HALF_RING + distance_per_cycle; pixel >= HALF_RING; pixel--)          { set_sparkle(ring, pixel, TRANSPARENT); }
        
        // outer half
        for (uint8_t pixel = 0; pixel < HALF_RING-1 - distance_per_cycle; pixel++)             { set_sparkle(ring, pixel, get_sparkle(ring, pixel+distance_per_cycle)); }
        for (uint16_t pixel = HALF_RING-1 - distance_per_cycle; pixel <= HALF_RING-1; pixel++) { set_sparkle(ring, pixel, TRANSPARENT); }
      }
      else if(SPARKLE_INTRA_RING_MOTION == UP) {
        // inner half
        for (uint16_t pixel = HALF_RING; pixel < LEDS_PER_RING-1 - distance_per_cycle; pixel++)        { set_sparkle(ring, pixel, get_sparkle(ring, pixel+distance_per_cycle)); }
        for (uint16_t pixel = LEDS_PER_RING-1 - distance_per_cycle; pixel <= LEDS_PER_RING-1; pixel++) { set_sparkle(ring, pixel, TRANSPARENT); }

        // outer half
        for (uint16_t pixel = HALF_RING-1; pixel > distance_per_cycle; pixel--) { set_sparkle(ring, pixel, get_sparkle(ring, pixel-distance_per_cycle)); }
        for (int8_t pixel = distance_per_cycle; pixel >= 0; pixel--)            { set_sparkle(ring, pixel, TRANSPARENT); }
      }
    }
  }


  // create new raindrops every "frequency" cycles
  if (sparkle_count % spawn_frequency == 0) {
    for (uint8_t ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
      if(SPARKLE_INTRA_RING_MOTION == DOWN) {
        for (uint16_t pixel = HALF_RING - range; pixel < HALF_RING + range; pixel++) {
          if (random16(color_thickness * portion) == 0) {
            spawn_raindrop(ring, pixel, color_thickness);
          }
        }
      }
      else if(SPARKLE_INTRA_RING_MOTION == UP) {
        for(uint16_t pixel = 0; pixel < range; pixel++) {
          if (random16(color_thickness * portion) == 0) {
            spawn_raindrop(ring, pixel, color_thickness);
          }
        }
        for(uint16_t pixel = LEDS_PER_RING-1; pixel > LEDS_PER_RING-1 - range; pixel--) {
          if (random16(color_thickness * portion) == 0) {
            spawn_raindrop(ring, pixel, color_thickness);
          }
        }
      }
    }
  }
}

void spawn_raindrop(uint8_t ring, uint16_t pixel, uint8_t color_thickness) {
  uint8_t dim = random8(SPARKLE_MAX_DIM + 1);
  uint8_t sparkle_color = random8(2) == 0 ? get_sparkle_color(0, dim) : get_sparkle_color(1, dim);
  bool is_inner = pixel >= HALF_RING;
  if(SPARKLE_INTRA_RING_MOTION == UP) { is_inner = !is_inner; }

  for (uint8_t thickness = 0; thickness < color_thickness; thickness++) {
    if (is_inner) { set_sparkle(ring, pixel - thickness, sparkle_color); }
    else { set_sparkle(ring, pixel + thickness, sparkle_color); }
  }
}

