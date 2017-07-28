// This file is for completed sparkle animations

//---------------------------------------- SPARKLE GLITTER ---------------------------------------
//  Creates sparkles of glitter randomly all over the structure
// SPARKLE_COLOR_THICKNESS(1:2), SPARKLE_PORTION(16:127), SPARKLE_MIN_DIM(0:2), SPARKLE_MAX_DIM(1:4), SPARKLE_RANGE(40:204)
inline void sparkle_glitter() {
  uint8_t color_thickness = scale_param(SPARKLE_COLOR_THICKNESS, 1, 2);
  uint8_t portion = scale_param(SPARKLE_PORTION, 16, 127);
  uint8_t min_dim = scale_param(SPARKLE_MIN_DIM, 0, 2);
  uint8_t max_dim = scale_param(SPARKLE_MAX_DIM, 1, 4);
  if(max_dim < min_dim) { min_dim = max_dim; }
  uint8_t range = scale_param(SPARKLE_RANGE, 40, 204);

  clear_sparkle_layer();
  
  uint8_t lower_limit = HALF_RING-range;
  uint16_t upper_limit = HALF_RING+range - color_thickness;
  
  //for (uint8_t ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
  for (uint8_t ring = 0; ring < NUM_RINGS; ring++) {
    for (uint16_t pixel = lower_limit; pixel < upper_limit; pixel++) {
      if (random8(portion * color_thickness) == 0) {
        uint8_t dim = random8(min_dim, max_dim + 1);
        uint8_t sparkle_color = random8(2) == 0 ? get_sparkle_color(0, dim) : get_sparkle_color(1, dim);

        for (uint8_t thickness = 0; thickness < color_thickness; thickness++) {
          sparkle_layer[ring][pixel+thickness] = sparkle_color;
        }
      }
    }
  }
}


//--------------------------------------- SPARKLE RAIN -------------------------------------------
//  Puts raindrops randomly at the top of the structure and runs them down both sides of each ring.  
// SPARKLE_COLOR_THICKNESS(1:2), SPARKLE_PORTION(15:80), SPARKLE_MAX_DIM(1:4), SPARKLE_MIN_DIM(0:2), SPARKLE_RANGE(20), SPARKLE_SPAWN_FREQUENCY(20), SPARKLE_INTRA_RING_MOTION(-1:1), SPARKLE_INTRA_RING_SPEED(16:64)
inline void sparkle_rain() {
  uint8_t color_thickness = scale_param(SPARKLE_COLOR_THICKNESS, 1, 2);
  uint8_t portion = scale_param(SPARKLE_PORTION, 15, 80);
  uint8_t min_dim = scale_param(SPARKLE_MIN_DIM, 0, 2);
  uint8_t max_dim = scale_param(SPARKLE_MAX_DIM, 1, 4);
  if(max_dim < min_dim) { min_dim = max_dim; }
  uint8_t range = 20;//scale_param(SPARKLE_RANGE, 20, 20);
  uint8_t spawn_frequency = 20;//scale_param(SPARKLE_SPAWN_FREQUENCY, 20, 20);
  uint8_t intra_speed = 1 << scale_param(SPARKLE_INTRA_RING_SPEED, 4, 6);
  
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
        for (uint16_t pixel = LEDS_PER_RING - 1; pixel > HALF_RING + distance_per_cycle; pixel--)  { sparkle_layer[ring][pixel] = sparkle_layer[ring][pixel-distance_per_cycle]; }
        for (uint8_t pixel = HALF_RING + distance_per_cycle; pixel >= HALF_RING; pixel--)          { sparkle_layer[ring][pixel] = TRANSPARENT; }
        
        // outer half
        for (uint8_t pixel = 0; pixel < HALF_RING-1 - distance_per_cycle; pixel++)             { sparkle_layer[ring][pixel] = sparkle_layer[ring][pixel+distance_per_cycle]; }
        for (uint16_t pixel = HALF_RING-1 - distance_per_cycle; pixel <= HALF_RING-1; pixel++) { sparkle_layer[ring][pixel] = TRANSPARENT; }
      }
      else if(SPARKLE_INTRA_RING_MOTION == UP) {
        // inner half
        for (uint16_t pixel = HALF_RING; pixel < LEDS_PER_RING-1 - distance_per_cycle; pixel++)        { sparkle_layer[ring][pixel] = sparkle_layer[ring][pixel+distance_per_cycle]; }
        for (uint16_t pixel = LEDS_PER_RING-1 - distance_per_cycle; pixel <= LEDS_PER_RING-1; pixel++) { sparkle_layer[ring][pixel] = TRANSPARENT; }

        // outer half
        for (uint16_t pixel = HALF_RING-1; pixel > distance_per_cycle; pixel--) { sparkle_layer[ring][pixel] = sparkle_layer[ring][pixel-distance_per_cycle]; }
        for (int8_t pixel = distance_per_cycle; pixel >= 0; pixel--)            { sparkle_layer[ring][pixel] = TRANSPARENT; }
      }
    }
  }


  // create new raindrops every "frequency" cycles
  if (sparkle_count % spawn_frequency == 0) {
    for (uint8_t ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
      if(SPARKLE_INTRA_RING_MOTION == DOWN) {
        for (uint16_t pixel = HALF_RING - range; pixel < HALF_RING + range; pixel++) {
          if (random16(color_thickness * portion) == 0) {
            spawn_raindrop(ring, pixel, color_thickness, random8(min_dim, max_dim + 1));
          }
        }
      }
      else if(SPARKLE_INTRA_RING_MOTION == UP) {
        for(uint16_t pixel = 0; pixel < range; pixel++) {
          if (random16(color_thickness * portion) == 0) {
            spawn_raindrop(ring, pixel, color_thickness, random8(min_dim, max_dim + 1));
          }
        }
        for(uint16_t pixel = LEDS_PER_RING-1; pixel > LEDS_PER_RING-1 - range; pixel--) {
          if (random16(color_thickness * portion) == 0) {
            spawn_raindrop(ring, pixel, color_thickness, random8(min_dim, max_dim + 1));
          }
        }
      }
    }
  }
}

inline void spawn_raindrop(uint8_t ring, uint16_t pixel, uint8_t color_thickness, uint8_t dim) {
  uint8_t sparkle_color = random8(2) == 0 ? get_sparkle_color(0, dim) : get_sparkle_color(1, dim);
  bool is_inner = pixel >= HALF_RING;
  if(SPARKLE_INTRA_RING_MOTION == UP) { is_inner = !is_inner; }

  for (uint8_t thickness = 0; thickness < color_thickness; thickness++) {
    if (is_inner) { sparkle_layer[ring][pixel - thickness] = sparkle_color; }
    else { sparkle_layer[ring][pixel + thickness] = sparkle_color; }
  }
}


//--------------------------------------- SPARKLE WARP SPEED -------------------------------------------
//  Sends sparkles racing around horizontally from ring to ring
//  When someone is inside it is intended to look like flying through space or that old cheesy 
//  animation when star trek goes to "warp speed"
//
//  To do: not yet plugged into parameters
// SPARKLE_COLOR_THICKNESS(1-6), SPARKLE_PORTION(1-100), SPARKLE_MAX_DIM(0-6), SPARKLE_RANGE(10-100), SPARKLE_SPAWN_FREQUENCY(1-127), SPARKLE_INTRA_RING_MOTION(-1, 1), SPARKLE_INTRA_RING_SPEED(4-64)

#define CYCLES_PER_STEP 15
inline void sparkle_warp_speed() {
  if ((sparkle_count >= 4*CYCLES_PER_STEP) ||
      (sparkle_count < CYCLES_PER_STEP  &&  sparkle_count % 5 == 0) ||
      (sparkle_count < 2*CYCLES_PER_STEP && sparkle_count % 4 == 0) ||
      (sparkle_count < 3*CYCLES_PER_STEP && sparkle_count % 3 == 0) ||
      (sparkle_count < 4*CYCLES_PER_STEP && sparkle_count % 2 == 0))
  {
    move_sparkle_layer_inter_ring(CW);
  }
}








//---------------------------------- SPARKLE 3 CIRCLES ---------------------------
// Highlights the 3 most prominent circles on the torus
// Each of these circles moves around the torus to parallel circles
// 
// fixme: allow speed to change as parameter
// fixme: add 3rd circle - only 2 easiest circles for now

inline void sparkle_three_circles() {}

//---------------------------------- SPARKLE 2 CIRCLES ---------------------------
inline void sparkle_seth_two_circles() {}


/* This code is buggy  
{
  uint8_t ring, pixel;
  uint8_t inter_throttle, intra_throttle;

  uint8_t vertical_color = get_sparkle_color(0, 4);
  uint8_t horizontal_color = get_sparkle_color(1, 6);

  clear_sparkle_layer();
  
  // vertical circle
  for (pixel = 0; pixel < LEDS_PER_RING; pixel++) {
    sparkle_layer[ring-1][pixel] =vertical_color;
  }

  // horizontal circle
  for (ring = 0; ring < 4; ring++) {
    sparkle_layer[ring-1][pixel] = horizontal_color;
  }

  // third circle has a slope of 3
  // work around rings simultaneously from both sides
//  for (ring = 0; ring < 4 / 2; ring++) {
//    for (pixel = 0; pixel < HALF_VISIBLE; pixel += 102) { // fixme: 3 for real structure
 
//      sparkle[(ring + current_coin_bottom) % 72][pixel] = CRGB::Purple;
//      sparkle[3 - (ring + current_coin_bottom) % 72][pixel] = CRGB::Purple;
//    }
//  }

  // move each circle start over one unit according to intra and inter ring speed
  inter_throttle = SPARKLE_INTER_RING_SPEED % 30 + 20;
  if (sparkle_count % inter_throttle == 0) {
    current_ring = (current_ring + SPARKLE_INTER_RING_MOTION) % NUM_RINGS;
  }
  intra_throttle = SPARKLE_INTRA_RING_SPEED % 30 + 20;
  if (sparkle_count % intra_throttle == 0) {
    current_pixel = (current_pixel + SPARKLE_INTRA_RING_MOTION) % LEDS_PER_RING;
  }
}



//---------------------------------- SPARKLE TWINKLE ---------------------------
//
// Chooses random location for stars, with random starting intensities, 
// and then randomly and continuously increases or decreases their intensity over time

inline void sparkle_twinkle() {
  if (sparkle_count % SPARKLE_INTRA_RING_SPEED == 0) {
    for (int ring = 0; ring < NUM_RINGS; ring++) {
        for (int pixel = 0; pixel < LEDS_PER_RING; pixel++) {
          
            // only change intensity if pixel is chosen as star
            if (get_sparkle_color(ring, pixel) != TRANSPARENT) {

// need to update with brian's get dim code
 
                // increase or decrease intensity depending on whether dim is even or odd
                if (dim[ring][pixel] == 6) { dim[ring][pixel] = 5; }
                else if (dim[ring][pixel] == 1) { dim[ring][pixel] = 0; }
                else if (dim[ring][pixel] % 2 == 0) { dim[ring][pixel] += 2; }
                else { dim[ring][pixel] -= 2; }

                // fixme: I can't figure out how to make this 2 color
                // would need to know which color had been dimmed before to put same color back with different dimness.
                // maybe it would look cool to let these colors change, not sure
                // sparkle_color = (get_sparkle[ring][pixel] == ...) ? 0 : 1;
                set_sparkle(ring, pixel, get_sparkle_color(0, dim[ring][pixel]));   
            }

        }
    }
  }
}

*/


