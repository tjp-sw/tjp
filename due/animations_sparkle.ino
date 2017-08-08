// This file is for completed sparkle animations


//---------------------------------------- SPARKLE GLITTER ---------------------------------------
//  Creates sparkles of glitter randomly all over the structure
// SPARKLE_COLOR_THICKNESS(1:2), SPARKLE_PORTION(16:127), SPARKLE_MIN_DIM(0:2), SPARKLE_MAX_DIM(1:4), SPARKLE_RANGE(40:204)
inline void sparkle_glitter(uint8_t num_colors) {
  uint8_t color_thickness = scale_param(SPARKLE_COLOR_THICKNESS, 1, 2);
  uint8_t portion = scale_param(SPARKLE_PORTION, 16, 127);
  uint8_t min_dim = scale_param(SPARKLE_MIN_DIM, 0, 2);
  uint8_t max_dim = scale_param(SPARKLE_MAX_DIM, 10, 126);
  uint8_t sparkle_color;

  if(max_dim < min_dim) { min_dim = max_dim; }
  uint8_t range = scale_param(SPARKLE_RANGE, 40, 204);

  clear_sparkle_layer();
  
  uint8_t lower_limit = HALF_RING-range;
  uint16_t upper_limit = HALF_RING + range - color_thickness;
  
  //for (uint8_t ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
  for (uint8_t ring = 0; ring < NUM_RINGS; ring++) {
    for (uint16_t pixel = lower_limit; pixel < upper_limit; pixel++) {
      if (random8(portion * color_thickness) == 0) {
        uint8_t dim = random8(min_dim, max_dim + 1);
        if (num_colors == 1) {
          sparkle_color = get_sparkle_color(0, dim);
        }
        else {
          sparkle_color = random8(2) == 0 ? get_sparkle_color(0, dim) : get_sparkle_color(1, dim);
        }

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
  inter_throttle = SPARKLE_INTER_RING_SPEED % 10 + 10;
  if (sparkle_count % inter_throttle == 0) {
    current_ring = (current_ring + SPARKLE_INTER_RING_MOTION) % NUM_RINGS;
  }
  intra_throttle = SPARKLE_INTRA_RING_SPEED % 5 + 10;
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
  coin_1_throttle = SPARKLE_INTER_RING_SPEED % 20 + 10;
  if (sparkle_count % coin_1_throttle == 0) {
    current_coin = (current_coin + SPARKLE_INTER_RING_MOTION) % NUM_RINGS;
  }
  
  coin_2_throttle = SPARKLE_INTRA_RING_SPEED % 5 + 10;
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
              if(dim > 126 - TWINKLE_STEP_SIZE) { dim = 126-1; }
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


//---------------------------- SPARKLE 3 CIRCLE TRAILS ---------------------------
// Highlights the 3 most prominent circles on the torus
// Each of these circles moves around the torus to parallel circles, leaving trails of dimmer circles
// 

inline void sparkle_circle_trails() {

  uint8_t ring, inter_throttle, intra_throttle, coin_throttle, copy;
  uint8_t trail, dim_step = 10; //fixme: tie this to a parameter
  uint8_t slope = LEDS_PER_RING / NUM_RINGS;  // slope in ring-pixel plane of coin circle
  int pixel;
  uint8_t trail_length = 5;
 
  uint8_t vertical_color = random8(2);
  uint8_t horizontal_color = random8(2);
  uint8_t coin_color = random8(2);
  uint8_t num_copies = scale_param(SPARKLE_PORTION, 1, 6);
  
  clear_sparkle_layer();

  // vertical circle
  for (pixel = 0; pixel < LEDS_PER_RING; pixel++) {
    //sparkle_layer[current_ring][pixel] = get_sparkle_color(0, 6);
    for (trail = 0; trail < trail_length; trail += 1) {
      for (copy = 0; copy < num_copies; copy++) {
        sparkle_layer[(current_ring + trail + copy * NUM_RINGS / num_copies) % NUM_RINGS][pixel] = get_sparkle_color(vertical_color, 126 - trail * dim_step);
      }
    }
  }

  // horizontal circle
  for (ring = 0; ring < NUM_RINGS; ring++) {
    for (trail = 0; trail < trail_length * 2; trail += 1) {
      for (copy = 0; copy < num_copies; copy++) {
        sparkle_layer[ring][(current_pixel - trail - copy * LEDS_PER_RING / num_copies) % LEDS_PER_RING] = get_sparkle_color(horizontal_color, 126 - trail * dim_step);
      }
    }
  }

  // diagonal circle
  for (ring = 0; ring < NUM_RINGS; ring++) {
    for (trail = 0; trail < trail_length; trail += 1) {
      for (copy = 0; copy < num_copies; copy++) {
        sparkle_layer[(ring + current_coin + trail + copy * NUM_RINGS / num_copies) % NUM_RINGS][ring * slope] = get_sparkle_color(coin_color, 126 - trail * dim_step);
      }
    }
  }

  // fixme: ack! why am i geting all these colors??? kinda cool but kinda fucked up
  // opposite diagonal circle
//  for (ring = 0; ring < NUM_RINGS; ring++) {
//    for (trail = 0; trail < trail_length; trail += 1) {
//      for (copy = 0; copy < num_copies; copy++) {
//        sparkle_layer[(ring - current_coin - trail - copy * NUM_RINGS / num_copies) % NUM_RINGS][ring * slope] = get_sparkle_color(coin_color, 126 - trail * dim_step);
//      }
//    }
//  }
  
  // move each circle start over one unit according to intra and inter ring speed
  inter_throttle = SPARKLE_INTER_RING_SPEED % 30 + 10;
//  if (sparkle_count % inter_throttle == 0) {
    current_ring = (current_ring + SPARKLE_INTER_RING_MOTION) % NUM_RINGS;
//  }
  intra_throttle = SPARKLE_INTRA_RING_SPEED % 10 + 10;
//  if (sparkle_count % intra_throttle == 0) {
    current_pixel = (current_pixel + SPARKLE_INTRA_RING_MOTION) % LEDS_PER_RING;
//  }

  // there's no parameter for this speed
  coin_throttle = (inter_throttle + intra_throttle) / 2;
//  if (sparkle_count % coin_throttle == 0) {
    current_coin = (current_coin + 1) % LEDS_PER_RING;
//  } 
}




//---------------------------------- SPARKLE TORUS_KNOT ---------------------------
// This code assumes there would be 504 = 72 * 7 pixels all the way around one ring, if pixels went under the floor

inline void sparkle_torus_knot() {
  int num_extended_pixels = 72 * 7;
  uint8_t knot_width = scale_param(SPARKLE_COLOR_THICKNESS, 1, 10);
  uint8_t num_stripes_index = scale_param(SPARKLE_PORTION, 0, 9);
  uint8_t intra_throttle = scale_param(SPARKLE_INTRA_RING_SPEED, 10, 20);
  uint8_t inter_throttle = scale_param(SPARKLE_INTER_RING_SPEED, 10, 20);
  int pixel;

  uint8_t num_stripes_options[] = {2, 3, 4, 6, 8, 9, 12, 18, 24, 36};  // # stripes of the knot passing through any one ring must be divisor of 72
  uint8_t num_stripes = num_stripes_options[num_stripes_index];  
  int segment_length = num_extended_pixels / num_stripes;  // split one ring's pixels into num_stripes separate segments

  // for aesthetic reasons, rotate no more than 1/8th of a ring from one ring to next, so max_pixels_rotated <= num_extended_pixels / 8 = 63
  // need n such that n * segment_length <= 63
  // in order to rotate n segments worth (n x segment_length pixels) over the 72 rings with above conditions, must 
  // rotate num_pixels_rotated = n x (7/segment_length) from one ring to the next
  // must make num_pixels_rotated <= 63; this works out to needing n <= 9 * segment_length
  uint8_t max_n = 9 * segment_length;
  uint8_t num_segments_to_rotate = random16(max_n) + 1;
  float pixels_rotated_in_decimal = num_segments_to_rotate * 7.0 / segment_length;

  for (uint8_t ring = 0; ring < NUM_RINGS; ring++) {
    uint8_t inter_rotation = (uint8_t) (((float) ring) * pixels_rotated_in_decimal);
    for (uint8_t stripe = 0; stripe < num_stripes; stripe++) {
      for (pixel = 0; pixel < knot_width; pixel++) {
          sparkle_layer[(ring + sparkle_count) % NUM_RINGS][stripe * segment_length + pixel + inter_rotation] = get_sparkle_color(0,6);
      }
//      for (pixel = knot_width; pixel < segment_length; pixel++) {
//          sparkle_layer[(ring + sparkle_count) % NUM_RINGS][stripe * segment_length + pixel + inter_rotation] = TRANSPARENT;
//      }
    }
  }
  if (sparkle_count % inter_throttle == 0) {
    current_ring = (current_ring + SPARKLE_INTER_RING_MOTION) % NUM_RINGS;
  }
  if (sparkle_count % intra_throttle == 0) {
    current_pixel = (current_pixel + SPARKLE_INTRA_RING_MOTION) % LEDS_PER_RING;
  }
  

}

//---------------------------------- SPARKLE TORUS_LINK ---------------------------
// This code assumes there would be 504 = 72 * 7 pixels all the way around one ring, if pixels went under the floor

inline void sparkle_torus_link() {
  int num_extended_pixels = 72 * 7;
  uint8_t knot_1_width = scale_param(SPARKLE_COLOR_THICKNESS, 5, 10);
  uint8_t knot_2_width = scale_param(SPARKLE_RANGE, 1, 7);
  int pixel;

  uint8_t num_stripes_options[] = {2, 3, 4, 6, 8, 9, 12, 18, 24, 36};  // # stripes of the knot passing through any one ring must be divisor of 72
  uint8_t num_1_stripes_index = scale_param(SPARKLE_PORTION, 0, 9);
  uint8_t num_1_stripes = num_stripes_options[num_1_stripes_index];  
  int segment_1_length = num_extended_pixels / num_1_stripes;  // split one ring's pixels into num_stripes separate segments
  
  uint8_t num_2_stripes_index = 9 - num_1_stripes_index; // not awesome but we have no more relevant sparkle parameters
  uint8_t num_2_stripes = num_stripes_options[num_2_stripes_index];  
  int segment_2_length = num_extended_pixels / num_2_stripes;  // split one ring's pixels into num_stripes separate segments

  // see torus knot comments for reasoning
  uint8_t max_1_n = 9 * segment_1_length;
  uint8_t num_segments_to_rotate_1 = random16(max_1_n) + 1;
  uint8_t max_2_n = 9 * segment_2_length;
  uint8_t num_segments_to_rotate_2 = random16(max_2_n) + 1;
  
  float pixels_1_rotated_in_decimal = num_segments_to_rotate_1 * 7.0 / segment_1_length;
  float pixels_2_rotated_in_decimal = num_segments_to_rotate_2 * 7.0 / segment_2_length;

  
  Serial.println("sparkle count " + String(sparkle_count));
  Serial.println("num stripes " + String(num_1_stripes)  + " and "+  String(num_2_stripes));
  Serial.println("num segments to rotate " + String(num_segments_to_rotate_1)  + " and "+  String(num_segments_to_rotate_2));
  Serial.println("widths " + String(knot_1_width) + " and "+ String(knot_2_width));

  clear_sparkle_layer();
  
  for (uint8_t ring = 0; ring < NUM_RINGS; ring++) {
    uint8_t inter_1_rotation = (uint8_t) (((float) ring) * pixels_1_rotated_in_decimal);
    uint8_t inter_2_rotation = (uint8_t) (((float) ring) * pixels_2_rotated_in_decimal);
    for (uint8_t stripe = 0; stripe < num_1_stripes; stripe++) {
      for (pixel = 0; pixel < knot_1_width; pixel++) {
        // fixme: maybe do something with dimming values 
        sparkle_layer[(ring + sparkle_count) % NUM_RINGS][stripe * segment_1_length + pixel + inter_1_rotation] = get_sparkle_color(0,6);
      }
//      for (pixel = knot_1_width; pixel < segment_1_length; pixel++) {
//        //sparkle_layer[(ring + sparkle_count) % NUM_RINGS][stripe * segment_1_length + pixel + inter_1_rotation] = TRANSPARENT;
//      }
    }
    for (uint8_t stripe = 0; stripe < num_2_stripes; stripe++) {
      for (pixel = 0; pixel < knot_2_width; pixel++) {
        sparkle_layer[(ring + sparkle_count) % NUM_RINGS][stripe * segment_2_length + pixel - inter_2_rotation] = get_sparkle_color(1,6);
      }
//      for (pixel = knot_2_width; pixel < segment_2_length; pixel++) {
//        sparkle_layer[(ring + sparkle_count) % NUM_RINGS][stripe * segment_2_length + pixel - inter_2_rotation] = TRANSPARENT;
//      }
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



