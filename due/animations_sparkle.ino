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
  uint8_t spawn_frequency = SPARKLE_SPAWN_FREQUENCY <= 1 ? SPARKLE_SPAWN_FREQUENCY : 20;//scale_param(SPARKLE_SPAWN_FREQUENCY, 20, 20);
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
  if(SPARKLE_SPAWN_FREQUENCY == 0) { return; }
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

#define WARP_SPEED_STEP1 40
#define WARP_SPEED_STEP2 70
#define WARP_SPEED_STEP3 95
#define WARP_SPEED_STEP4 120
#define WARP_SPEED_STEP5 140
#define WARP_SPEED_STEP6 160

inline void sparkle_warp_speed() {
  if (sparkle_count < WARP_SPEED_STEP1)
  {
    if(sparkle_count % 6 == 0) { move_sparkle_layer_inter_ring(CW); }
  }
  else if(sparkle_count < WARP_SPEED_STEP2) {
    if(sparkle_count % 4 == 0) { move_sparkle_layer_inter_ring(CW); }
  }
  else if(sparkle_count < WARP_SPEED_STEP3) {
    if(sparkle_count % 3 == 0) { move_sparkle_layer_inter_ring(CW); }
  }
  else if(sparkle_count < WARP_SPEED_STEP5) {
    if(sparkle_count % 2 == 0) { move_sparkle_layer_inter_ring(CW); }
  }
  else if(sparkle_count < WARP_SPEED_STEP6) {
    move_sparkle_layer_inter_ring(CW);
  }
  else {
    move_sparkle_layer_inter_ring(CW);
    move_sparkle_layer_inter_ring(CW);
  }
  

  // Extend length of sparkles
  if(sparkle_count == WARP_SPEED_STEP1 || sparkle_count == WARP_SPEED_STEP2 || sparkle_count == WARP_SPEED_STEP3 || sparkle_count == WARP_SPEED_STEP4) {
    uint8_t temp[LEDS_PER_RING];
    memcpy(temp, sparkle_layer[0], LEDS_PER_RING);
      
    for(uint8_t ring = 1; ring < NUM_RINGS-1; ring++) {
      for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        if(sparkle_layer[ring][pixel] != TRANSPARENT) {
          sparkle_layer[ring-1][pixel] = sparkle_layer[ring][pixel];
        }
      }
    }

    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      if(temp[pixel] != TRANSPARENT) {
        sparkle_layer[NUM_RINGS-1][pixel] = temp[pixel];
      }
    }
  }
}


//---------------------------------- SPARKLE 3 CIRCLES ---------------------------
// Highlights the 3 most prominent circles on the torus
// Each of these circles moves around the torus to parallel circles
// 

inline void sparkle_three_circles() {

  uint8_t ring, inter_throttle, intra_throttle, coin_throttle;
  uint8_t slope = LEDS_PER_RING / NUM_RINGS;  // slope in ring-pixel plane of coin circle
  int pixel;
  
  uint8_t vertical_color = get_sparkle_color(0, 4);
  uint8_t horizontal_color = get_sparkle_color(1, 6);

  clear_sparkle_layer();

  // vertical circle
  for (pixel = 0; pixel < LEDS_PER_RING; pixel++) {
    sparkle_layer[current_ring][pixel] = vertical_color;
  }

  // horizontal circle
  for (ring = 0; ring < NUM_RINGS; ring++) {
    sparkle_layer[ring][current_pixel] = horizontal_color;
  }

  // diagonal circle
  for (ring = 0; ring < NUM_RINGS; ring++) {
      sparkle_layer[(ring + current_coin) % NUM_RINGS][ring * slope] = horizontal_color;
  }

  // move each circle start over one unit according to intra and inter ring speed
  inter_throttle = SPARKLE_INTER_RING_SPEED % 30 + 10;
  if (sparkle_count % inter_throttle == 0) {
    current_ring = (current_ring + SPARKLE_INTER_RING_MOTION) % NUM_RINGS;
  }
  intra_throttle = SPARKLE_INTRA_RING_SPEED % 10 + 10;
  if (sparkle_count % intra_throttle == 0) {
    current_pixel = (current_pixel + SPARKLE_INTRA_RING_MOTION) % LEDS_PER_RING;
  }

  // there's no parameter for this speed
  coin_throttle = (inter_throttle + intra_throttle) / 2;
  if (sparkle_count % coin_throttle == 0) {
    current_coin = (current_coin + 1) % LEDS_PER_RING;
  } 
}


//---------------------------------- SPARKLE 2 COINS ---------------------------
// Highlights the 3 most prominent circles on the torus
// Each of these circles moves around the torus to parallel circles
// 

inline void sparkle_two_coins() {
  
  uint8_t ring, coin_1_throttle, coin_2_throttle;
  uint8_t slope = LEDS_PER_RING / NUM_RINGS;  // slope in ring-pixel plane of coin circle

  uint8_t coin_1_color = get_sparkle_color(0, 6);
  uint8_t coin_2_color = get_sparkle_color(1, 6);

  clear_sparkle_layer();
  
  for (ring = 0; ring < NUM_RINGS; ring++) {
      sparkle_layer[(ring + current_coin) % NUM_RINGS][ring * slope] = coin_1_color;
  }

  // current_ring is a global variable I'm re-using to save space. its meaning is base of coin 2
  for (ring = 0; ring < NUM_RINGS; ring++) {
    sparkle_layer[(ring + current_ring) % NUM_RINGS][LEDS_PER_RING- ring * slope] = coin_2_color;
  }

  // move each circle start over one unit according to intra and inter ring speed
  coin_1_throttle = SPARKLE_INTER_RING_SPEED % 30 + 10;
  if (sparkle_count % coin_1_throttle == 0) {
    current_coin = (current_coin + SPARKLE_INTER_RING_MOTION) % NUM_RINGS;
  }
  
  coin_2_throttle = SPARKLE_INTRA_RING_SPEED % 10 + 10;
  if (sparkle_count % coin_2_throttle == 0) {
    current_ring = (current_ring + SPARKLE_INTRA_RING_MOTION) % LEDS_PER_RING;
  }

}


//---------------------------------- SPARKLE TWINKLE ---------------------------
//
// Chooses random location for stars, with random starting intensities, 
// and then randomly and continuously increases or decreases their intensity over time
#define TWINKLE_STEP_SIZE 8 // This value must be even
inline void sparkle_twinkle() {
  uint8_t dim, raw_color;

  // fixme: could/should this be tied to parameter?
  if ((sparkle_count % 1) == 0) {
    for (int ring = 0; ring < RINGS_PER_NODE; ring++) {
      for (int pixel = 0; pixel < LEDS_PER_RING; pixel++) {
          
        // only change intensity if pixel is chosen as star
        if (sparkle_layer[ring][pixel] != TRANSPARENT) {

          dim = get_sparkle_dim_value(ring, pixel);
          raw_color = get_sparkle_raw_color(ring, pixel);

          // cycle about 1/4 the brightness values each pass
          // fixme: could this be tied to one of the parameters, rather than just using 4?
          if (random16(4) == 0) {

            // increase intensity if brightness is even, decrease if odd
            if (dim % 2 == 0) {
              // Increasing dim
              if(dim > MAX_SPARKLE_DIMMING - TWINKLE_STEP_SIZE) { dim = MAX_SPARKLE_DIMMING-1; }
              else { dim += TWINKLE_STEP_SIZE; }
            }
            else {
              // Decreasing dim
              if (dim <= TWINKLE_STEP_SIZE) { dim = 0; }
              else { dim -= TWINKLE_STEP_SIZE; }
            }

            sparkle_layer[ring][pixel] = get_sparkle_color(raw_color, dim);
          }
        }
      }
    }
  }
}



// ----------------- Variable spin -----------------
// Each ring rotates blocks of pixels around rings at different rates.
// Goal is to find cool periods that result in several unique alignments forming
// (Goal was not met but hey it's a thing...)
#define VARIABLE_SPIN_STEP_SIZE 5
void variable_spin() {
  // 6 spin rates, centered at the middle of each node, so node edges line up with each other.
  // First half of node spins one way, second half the other way. Reverse direction on odd numbered nodes so node edges line up.
  const uint8_t spinRates[12] = { 2, 3, 4, 5, 6, 7 };
  static int16_t centerPoints[12] = { HALF_RING, HALF_RING, HALF_RING, HALF_RING, HALF_RING, HALF_RING, HALF_RING, HALF_RING, HALF_RING, HALF_RING, HALF_RING, HALF_RING };
  for(uint8_t i = 0; i < 6; i++)
  {
      centerPoints[i] -= spinRates[i];
      if(centerPoints[i] < 0) { centerPoints[i] += LEDS_PER_RING; }

      centerPoints[11-i] += spinRates[i];
      if(centerPoints[11-i] > LEDS_PER_RING-1) { centerPoints[11-i] -= LEDS_PER_RING; }
  }

  clear_sparkle_layer();
  
  for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
    uint8_t cur_color = get_sparkle_color(ring % 2, 70);
    uint16_t centerPoint = centerPoints[ring];

    sparkle_layer[ring][centerPoint] = cur_color;
    
    if(centerPoint == 0) {
      sparkle_layer[ring][LEDS_PER_RING - 1] = cur_color;
      sparkle_layer[ring][LEDS_PER_RING - 2] = cur_color;
      //sparkle_layer[ring][LEDS_PER_RING - 3] = cur_color;
    }
    else if(centerPoint == 1) {
      sparkle_layer[ring][0] = cur_color;
      sparkle_layer[ring][LEDS_PER_RING - 1] = cur_color;
      //sparkle_layer[ring][LEDS_PER_RING - 2] = cur_color;
    }
    else if(centerPoint == 2) {
      sparkle_layer[ring][1] = cur_color;
      sparkle_layer[ring][0] = cur_color;
      //sparkle_layer[ring][LEDS_PER_RING - 1] = cur_color;
    }
    else {
      sparkle_layer[ring][centerPoint-1] = cur_color;
      sparkle_layer[ring][centerPoint-2] = cur_color;
      //sparkle_layer[ring][centerPoint-3] = cur_color;
    }

    if(centerPoint == LEDS_PER_RING - 1) {
      sparkle_layer[ring][0] = cur_color;
      sparkle_layer[ring][1] = cur_color;
      //sparkle_layer[ring][2] = cur_color;
    }
    else if(centerPoint == LEDS_PER_RING - 2) {
      sparkle_layer[ring][LEDS_PER_RING-1] = cur_color;
      sparkle_layer[ring][0] = cur_color;
      //sparkle_layer[ring][1] = cur_color;
    }
    else if(centerPoint == LEDS_PER_RING - 3) {
      sparkle_layer[ring][LEDS_PER_RING-2] = cur_color;
      sparkle_layer[ring][LEDS_PER_RING-1] = cur_color;
      //sparkle_layer[ring][0] = cur_color;
    }
    else {
      sparkle_layer[ring][centerPoint+1] = cur_color;
      sparkle_layer[ring][centerPoint+2] = cur_color;
      //sparkle_layer[ring][centerPoint+3] = cur_color;
    }
  }
}



