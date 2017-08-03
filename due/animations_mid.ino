// This file is for completed mid animations

//-------------------------------- SNAKE --------------------------------
// Sends alternating bands of colors rotating around the rings
// Creates repeated snakes with MID_NUM_COLORS colors, with each color repeated COLOR_THICKNESS times, separated by BLACK_THICKNESS black LEDs.
// MID_NUM_COLORS(1:3), MID_COLOR_THICKNESS(2:5), MID_BLACK_THICKNESS(6:50), MID_INTRA_RING_MOTION(-1:1), MID_RING_OFFSET(-period/2:period/2), MID_INTRA_RING_SPEED(8-32)
inline void snake(uint8_t min_ring, uint8_t max_ring) {
  uint8_t color_thickness = scale_param(MID_COLOR_THICKNESS, 2, 5);
  uint8_t black_thickness = scale_param(MID_BLACK_THICKNESS, 6, 50);
  uint8_t intra_speed = 1 << scale_param(MID_INTRA_RING_SPEED, 3, 5);
  uint8_t period = MID_NUM_COLORS * color_thickness + black_thickness;
  int8_t ring_offset = scale_param(MID_RING_OFFSET, -1 * period/2, period/2);
  uint16_t extended_led_count = ((LEDS_PER_RING-1)/period+1)*period;
  int8_t alternating_multiplier = MID_INTRA_RING_MOTION;

  if(node_number*RINGS_PER_NODE > min_ring) { min_ring = node_number*RINGS_PER_NODE; }
  if((node_number+1)*RINGS_PER_NODE < max_ring) { max_ring = (node_number+1)*RINGS_PER_NODE; }
  for (uint8_t ring = min_ring; ring < max_ring; ring++) {
      // if alternating rotation style, alternate direction of every other ring
      if(MID_INTRA_RING_MOTION == ALTERNATE) { alternating_multiplier = ring % 2 == 0 ? -1 : 1; }
      int16_t motion_offset = ring*ring_offset + alternating_multiplier * intra_speed * mid_count / THROTTLE;
      while(motion_offset < 0) { motion_offset += extended_led_count; }
      
      for (uint16_t pixel = 0; pixel < extended_led_count; pixel++) {
          uint16_t idx = (pixel + motion_offset) % extended_led_count;
          if(idx >= LEDS_PER_RING) { continue; }

          uint8_t pattern_idx = pixel % period;

          // I'd recommend this if you want it to allow more than 3 colors
          if(pattern_idx < black_thickness) {
            mid_layer[ring][idx] = TRANSPARENT;
          }
          else {
            for(uint8_t color = 0; color < MID_NUM_COLORS; color++) {
              if(pattern_idx < black_thickness + (color+1)*color_thickness) {
                mid_layer[ring][idx] = get_mid_color(color);
                break;
              }
            }
          }

      /*
          for (int color = 0; color <= MID_NUM_COLORS; color++) {
              if(pattern_idx < black_thickness) {
                  mid_layer[ring][idx] = TRANSPARENT;
              }
              else if(pattern_idx < black_thickness + color_thickness) {
                  mid_layer[ring][idx] = get_mid_color(color);
              }
*/
/* my old index code
        if (pixel % strip_length >= num_colors * color_length) {
            leds[ring][(pixel + multiplier * rotation_direction * loop_count + offset*ring) % VISIBLE_LEDS_PER_RING] = CRGB::Black;
          }

          // 
          else if ((pixel % strip_length >= color*color_length) &&  (pixel % strip_length < (color + 1)*color_length)) {
            leds[ring][(pixel + multiplier * rotation_direction * loop_count + offset*ring) % VISIBLE_LEDS_PER_RING] = get_color(palette_num, show_colors[color]);
          }
        }
*/

/* don't need this since it's handled in the color loop above
      else if(pattern_idx < black_thickness + 2*color_thickness) {
        mid_layer[ring][idx] = get_mid_color(1);
      }
      else {
        mid_layer[ring][idx] = get_mid_color(2);
      }
      */
    }
  }
}


//-------------------------------- FIRE ---------------------------------
// Adapted from: Fire2012 by Mark Kriegsman, July 2012, part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
// Changes palette to a heat scale, and values in mid_layer represent the amount of heat in each pixel
// The palette_type parameter changes how the heat scale is constructed. Setting to 0 will skip this step and just use the actual palette.
// MID_INTRA_RING_MOTION: UP or DOWN; DOWN will spawn fire at the top and heat travels downward
// MID_BLACK_THICKNESS(15) == cooling, MID_COLOR_THICKNESS(130) == sparking chance, MID_NUM_COLORS(150) == minimum spark size
// A cooling wind rolls around the structure: MID_INTER_RING_SPEED(8) == wind speed, MID_RING_OFFSET(96) == wind decay amount in neighboring rings

#define SPARKING_RANGE 10   // How far from bottom sparks will form
#define MAX_WIND_COOLING 70 // Largest extra cooling, at center of the wind
#define MAX_WIND_RANGE 6    // Wind cooling reaches 6 rings in either direction from center
inline void fire(uint8_t palette_type, bool draw_inner_half, uint8_t min_ring, uint8_t max_ring) {
  uint8_t cooling = palette_type == FIRE_PALETTE_DISABLED ? 12 : 8;//scale_param(MID_BLACK_THICKNESS, 15, 15);
  uint8_t sparking_chance = 180;//scale_param(MID_COLOR_THICKNESS, 130, 130);
  uint8_t min_spark_size = 150;//scale_param(MID_NUM_COLORS, 150, 150);
  uint8_t wind_speed = 8;//scale_param(MID_INTER_RING_SPEED, 8, 8);
  uint8_t wind_decay = 96;//scale_param(MID_RING_OFFSET, 96, 96);

  random16_add_entropy(loop_count);
  
  if(palette_type != FIRE_PALETTE_DISABLED) {
    if(palette_type == FIRE_PALETTE_STANDARD) { set_fire_palette(); }
  }

  if(node_number*RINGS_PER_NODE > min_ring) { min_ring = node_number*RINGS_PER_NODE; }
  if((node_number+1)*RINGS_PER_NODE < max_ring) { max_ring = (node_number+1)*RINGS_PER_NODE; }
  
  // Step 1.  Cool down every 4th cell a little, and add an extra cooling wind that moves around the structure
  uint8_t coldest_ring = (mid_count * wind_speed / THROTTLE) % NUM_RINGS;
  for(uint8_t ring = min_ring; ring < max_ring; ring++) {
    uint8_t dist = ring > coldest_ring ? ring - coldest_ring : coldest_ring - ring;
    if(dist > NUM_RINGS/2) { dist = NUM_RINGS - dist; }
    
    uint8_t wind_cooling = 0;
    if(dist <= MAX_WIND_RANGE) {
      wind_cooling = MAX_WIND_COOLING;
      while(dist > 0) {
        wind_cooling = wind_cooling * wind_decay / 127;
        dist--;
      }
    }
    
    for(uint16_t pixel = 0; pixel < HALF_RING; pixel+=4) {
      uint8_t extra_cooling = wind_cooling;
      //if((pixel < 2*SPARKING_RANGE) || (pixel > NUM_LEDS-1 - 2*SPARKING_RANGE))
        //extra_cooling = 255;
      //if((pixel < 4*SPARKING_RANGE) || (pixel > NUM_LEDS-1 - 4*SPARKING_RANGE))
        //extra_cooling = wind_cooling * 3;
      mid_layer[ring][pixel] = qsub8(mid_layer[ring][pixel], random8(0, cooling+extra_cooling));
      if(draw_inner_half) {
        mid_layer[ring][LEDS_PER_RING - 1 - pixel] = qsub8(mid_layer[ring][LEDS_PER_RING - 1 - pixel],  random8(0, cooling+extra_cooling));
      }
    }

    // Cool down cells that otherwise are never cooled and don't inherit heat from below
    if(MID_INTRA_RING_MOTION == UP) {
      mid_layer[ring][LEDS_PER_RING - 2] = qsub8(mid_layer[ring][LEDS_PER_RING-2], random8(0, cooling+wind_cooling));
      mid_layer[ring][1] = qsub8(mid_layer[ring][1], random8(0, cooling+wind_cooling));
    }
    else {
      mid_layer[ring][HALF_RING - 1] = qsub8(mid_layer[ring][HALF_RING - 1], random8(0, cooling+wind_cooling));
      mid_layer[ring][HALF_RING - 2] = qsub8(mid_layer[ring][HALF_RING - 2], random8(0, cooling+wind_cooling));
      mid_layer[ring][HALF_RING] = qsub8(mid_layer[ring][HALF_RING], random8(0, cooling+wind_cooling));
      mid_layer[ring][HALF_RING + 1] = qsub8(mid_layer[ring][HALF_RING + 1], random8(0, cooling+wind_cooling));
    }
  }

  if(MID_INTRA_RING_MOTION == UP) {
    for(uint8_t ring = min_ring; ring < max_ring; ring++) {
      
      // Step 2.  Heat from each cell drifts 'up' and diffuses a little
      for(uint16_t pixel = 1; pixel < HALF_RING - 1; pixel++) {
        // Outer half
        mid_layer[ring][HALF_RING - pixel] = (mid_layer[ring][HALF_RING - pixel - 1] + mid_layer[ring][HALF_RING - pixel - 2]) / 2;
        
        // Inner half
        if(draw_inner_half) { 
          mid_layer[ring][HALF_RING + pixel - 1] = (mid_layer[ring][HALF_RING + pixel] + mid_layer[ring][HALF_RING + pixel + 1]) / 2;
        }
      }
  
    
      // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
      if(random8() < sparking_chance) {
        uint8_t pixel = random8(SPARKING_RANGE);
        mid_layer[ring][pixel] = qadd8(mid_layer[ring][pixel], random8(min_spark_size, 255));
      }
  
      if(draw_inner_half && random8() < sparking_chance) {
        uint16_t pixel = LEDS_PER_RING - 1 - random8(SPARKING_RANGE);
        mid_layer[ring][pixel] = qadd8(mid_layer[ring][pixel], random8(min_spark_size, 255));
      }
    }
  }
  else {
    // Spawn fire at top and heat travels down; heavily repeated code
    for(uint8_t ring = min_ring; ring < max_ring; ring++) {
      
      // Step 2.  Heat from each cell drifts 'down' and diffuses a little
      for(int16_t pixel = 1; pixel < HALF_RING - 1; pixel++) {
        // Outer half
        mid_layer[ring][pixel - 1] = (mid_layer[ring][pixel] + mid_layer[ring][pixel + 1]) / 2;
        
        // Inner half
        if(draw_inner_half) {
          mid_layer[ring][LEDS_PER_RING - pixel] = (mid_layer[ring][LEDS_PER_RING - pixel - 1] + mid_layer[ring][LEDS_PER_RING - pixel - 2]) / 2;
        }
      }


      // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
      if(random8() < sparking_chance) {
        uint16_t pixel = HALF_RING - 1 - random8(SPARKING_RANGE);
        mid_layer[ring][pixel] = qadd8(mid_layer[ring][pixel], random8(min_spark_size, 255));
      }

      if(draw_inner_half && random8() < sparking_chance) {
        uint8_t pixel = HALF_RING + random8(SPARKING_RANGE);
        mid_layer[ring][pixel] = qadd8(mid_layer[ring][pixel], random8(min_spark_size, 255));
      }
    }
  }
}

inline void set_fire_palette() {
  // change mid_palette to heat scale
  CRGB steps[3];
  steps[0] = current_palette[2];
  steps[1] = current_palette[3];
  steps[2] = current_palette[4];

  steps[0].maximizeBrightness();
  steps[1].maximizeBrightness();

  steps[0] %= 64; // Set to 1/4 of max brightness
  steps[1] %= 64;// Set to 1/4 of max brightness

  while(steps[0].r > 5 && steps[0].g > 5 && steps[0].b > 5) { steps[0]--; } // Manually increase saturation
  //while(steps[2].r < 255 && steps[2].g < 255 && steps[3].b < 255) { steps[2]++; }
  steps[2] |= CRGB(40, 40, 40); // Raise each channel to a minimum of 60
  steps[2].maximizeBrightness();
  steps[2] %= 64;// Set to 1/4 of max brightness

  CRGB start, target;

  // Step 1: Black to dim, saturated color0
  target = steps[0];
  for(uint8_t i = 0; i < 90; i++) {
    start = CRGB::Black;
    nblend(start, target, 255 * i / 90);
    mid_palette.entries[i] = start;
  }

  // Step 2: color0 to color1, up value and drop saturation
  target = steps[1];
  for(uint8_t i = 90; i < 160; i++) {
    start = steps[0];
    nblend(start, target, 255 * (i-90) / 70);
    mid_palette.entries[i] = start;
  }

  // Step 3: color1 to color2, up value and drop saturation
  target = steps[2];
  for(uint8_t i = 160; i < 240; i++) {
    start = steps[1];
    nblend(start, target, 255 * (i-160) / 80);
    mid_palette.entries[i] = start;
  }

  // Step 4: blend into fully bright white
  target = CRGB(255, 255, 255);
  for(uint16_t i = 240; i < 256; i++) {
    start = steps[2];
    nblend(start, target, 255 * (i-240) / 16);
    mid_palette.entries[i] = start;
  }
}

inline void cleanup_fire() {
  // change mid_palette from heat scale back to normal palette with dimming and gradients
  create_mid_palette(&mid_palette, current_palette[2], current_palette[3], current_palette[4]);
}


//-------------------------------- SCROLLING DIM ---------------------------------
// Draws single-colored bands that fade in and out and scroll around rings
// Breaking these into separate functions with mostly duplicated code because these functions already run pretty slowly (due to heavy blending with background)
// MID_NUM_COLORS(1:3), MID_COLOR_THICKNESS(1:6), MID_BLACK_THICKNESS(6:18), MID_INTRA_RING_MOTION(-1:1), MID_RING_OFFSET(-period/2:period/2), MID_INTRA_RING_SPEED(8:64)
inline void mid_scrolling_dim1(uint8_t min_ring, uint8_t max_ring) {
  uint8_t color_thickness = scale_param(MID_COLOR_THICKNESS, 1, 6);
  uint8_t black_thickness = scale_param(MID_BLACK_THICKNESS, 6, 18);
  uint8_t intra_speed = 1 << scale_param(MID_INTRA_RING_SPEED, 3, 6);
  uint8_t dim_period = color_thickness + black_thickness + 2*MAX_MID_DIMMING;
  uint8_t full_period = dim_period * MID_NUM_COLORS;
  int8_t ring_offset = scale_param(MID_RING_OFFSET, -1 * dim_period/2, dim_period/2);
  uint16_t extended_led_count = ((LEDS_PER_RING-1)/full_period+1)*full_period;
  int8_t alternating_multiplier = MID_INTRA_RING_MOTION;


  if(node_number*RINGS_PER_NODE > min_ring) { min_ring = node_number*RINGS_PER_NODE; }
  if((node_number+1)*RINGS_PER_NODE < max_ring) { max_ring = (node_number+1)*RINGS_PER_NODE; }
  for(uint8_t ring = min_ring; ring < max_ring; ring++) {
    if(MID_INTRA_RING_MOTION == ALTERNATE) { alternating_multiplier = ring % 2 == 0 ? -1 : 1; }
    int16_t motion_offset = ring*ring_offset + alternating_multiplier * intra_speed * mid_count / THROTTLE;
    while(motion_offset < 0) { motion_offset += extended_led_count; }

    for(uint16_t pixel = 0; pixel < extended_led_count; pixel++) {
      uint16_t idx = (pixel + motion_offset) % extended_led_count;
      if(idx >= LEDS_PER_RING) { continue; }

      // Color is dependent on the the LED position, hard switches from one hue to the next
      uint8_t color_index = (idx / dim_period) % MID_NUM_COLORS;
      uint8_t pattern_idx = pixel % dim_period;

      if(pattern_idx < color_thickness) {
        mid_layer[ring][idx] = get_mid_color(color_index);
      }
      else if(pattern_idx < color_thickness + MAX_MID_DIMMING) {
        uint8_t dim_amount = 1 + (pattern_idx - color_thickness);
        mid_layer[ring][idx] = get_mid_color(color_index, dim_amount);
      }
      else if(pattern_idx < color_thickness + MAX_MID_DIMMING + black_thickness) {
        mid_layer[ring][idx] = TRANSPARENT;
      }
      else {
        uint8_t dim_amount = MAX_MID_DIMMING - (pattern_idx - color_thickness - black_thickness - MAX_MID_DIMMING);
        mid_layer[ring][idx] = get_mid_color(color_index, dim_amount);
      }
    }
  }
}

inline void mid_scrolling_dim2(uint8_t min_ring, uint8_t max_ring) {
  uint8_t color_thickness = scale_param(MID_COLOR_THICKNESS, 1, 6);
  uint8_t black_thickness = scale_param(MID_BLACK_THICKNESS, 6, 18);
  uint8_t intra_speed = 1 << scale_param(MID_INTRA_RING_SPEED, 3, 6);
  uint8_t dim_period = color_thickness + black_thickness + 2*MAX_MID_DIMMING;
  uint8_t full_period = dim_period * MID_NUM_COLORS;
  int8_t ring_offset = scale_param(MID_RING_OFFSET, -1 * dim_period/2, dim_period/2);
  uint16_t extended_led_count = ((LEDS_PER_RING-1)/full_period+1)*full_period;
  int8_t alternating_multiplier = MID_INTRA_RING_MOTION;

  if(node_number*RINGS_PER_NODE > min_ring) { min_ring = node_number*RINGS_PER_NODE; }
  if((node_number+1)*RINGS_PER_NODE < max_ring) { max_ring = (node_number+1)*RINGS_PER_NODE; }
  for(uint8_t ring = min_ring; ring < max_ring; ring++) {
    if(MID_INTRA_RING_MOTION == ALTERNATE) { alternating_multiplier = ring % 2 == 0 ? -1 : 1; }
    int16_t motion_offset = ring*ring_offset + alternating_multiplier * intra_speed * mid_count / THROTTLE;
    while(motion_offset < 0) { motion_offset += extended_led_count; }

    for(uint16_t pixel = 0; pixel < extended_led_count; pixel++) {
      uint16_t idx = (pixel + motion_offset) % extended_led_count;
      if(idx >= LEDS_PER_RING) { continue; }

      // Color is dependent on the the LED position, and is a gradient from one hue to the next
      uint8_t color_index = (idx / dim_period) % MID_NUM_COLORS;
      uint8_t pattern_idx = pixel % dim_period;
      
      uint8_t next_color = (color_index + 1) % MID_NUM_COLORS;
      uint16_t blend_amount = (idx % dim_period) * MID_GRADIENT_SIZE / dim_period;
      
      if(pattern_idx < color_thickness) {
        mid_layer[ring][idx] = get_mid_color(color_index, next_color, blend_amount);
      }
      else if(pattern_idx < color_thickness + MAX_MID_DIMMING) {
        uint8_t dim_amount = 1 + (pattern_idx - color_thickness);
        mid_layer[ring][idx] = get_mid_color(color_index, next_color, blend_amount, dim_amount);
      }
      else if(pattern_idx < color_thickness + MAX_MID_DIMMING + black_thickness) {
        mid_layer[ring][idx] = TRANSPARENT;
      }
      else {
        uint8_t dim_amount = MAX_MID_DIMMING - (pattern_idx - color_thickness - black_thickness - MAX_MID_DIMMING);\
        mid_layer[ring][idx] = get_mid_color(color_index, next_color, blend_amount, dim_amount);
      }
    }
  }
}

inline void mid_scrolling_dim3(uint8_t min_ring, uint8_t max_ring) {
  uint8_t color_thickness = scale_param(MID_COLOR_THICKNESS, 1, 6);
  uint8_t black_thickness = scale_param(MID_BLACK_THICKNESS, 6, 18);
  uint8_t intra_speed = 1 << scale_param(MID_INTRA_RING_SPEED, 3, 6);
  uint8_t dim_period = color_thickness + black_thickness + 2*MAX_MID_DIMMING;
  uint8_t full_period = dim_period * MID_NUM_COLORS;
  int8_t ring_offset = scale_param(MID_RING_OFFSET, -1 * dim_period/2, dim_period/2);
  uint16_t extended_led_count = ((LEDS_PER_RING-1)/full_period+1)*full_period;
  int8_t alternating_multiplier = MID_INTRA_RING_MOTION;


  if(node_number*RINGS_PER_NODE > min_ring) { min_ring = node_number*RINGS_PER_NODE; }
  if((node_number+1)*RINGS_PER_NODE < max_ring) { max_ring = (node_number+1)*RINGS_PER_NODE; }
  for(uint8_t ring = min_ring; ring < max_ring; ring++) {
    if(MID_INTRA_RING_MOTION == ALTERNATE) { alternating_multiplier = ring % 2 == 0 ? -1 : 1; }
    int16_t motion_offset = ring*ring_offset + alternating_multiplier * intra_speed * mid_count / THROTTLE;
    while(motion_offset < 0) { motion_offset += extended_led_count; }

    for(uint16_t pixel = 0; pixel < extended_led_count; pixel++) {
      uint16_t idx = (pixel + motion_offset) % extended_led_count;
      if(idx >= LEDS_PER_RING) { continue; }

      // Each band is a solid hue
      uint8_t color_index = (((pixel + dim_period/2) % extended_led_count) / dim_period) % MID_NUM_COLORS;
      uint8_t pattern_idx = pixel % dim_period;

      if(pattern_idx < color_thickness) {
        mid_layer[ring][idx] = get_mid_color(color_index);
      }
      else if(pattern_idx < color_thickness + MAX_MID_DIMMING) {
        uint8_t dim_amount = 1 + (pattern_idx - color_thickness);
        mid_layer[ring][idx] = get_mid_color(color_index, dim_amount);
      }
      else if(pattern_idx < color_thickness + MAX_MID_DIMMING + black_thickness) {
        mid_layer[ring][idx] = TRANSPARENT;
      }
      else {
        uint8_t dim_amount = MAX_MID_DIMMING - (pattern_idx - color_thickness - black_thickness - MAX_MID_DIMMING);
        mid_layer[ring][idx] = get_mid_color(color_index, dim_amount);
      }
    }
  }
}

inline void mid_scrolling_dim4(uint8_t min_ring, uint8_t max_ring) {
  uint8_t color_thickness = scale_param(MID_COLOR_THICKNESS, 1, 6);
  uint8_t black_thickness = scale_param(MID_BLACK_THICKNESS, 6, 18);
  uint8_t intra_speed = 1 << scale_param(MID_INTRA_RING_SPEED, 3, 6);
  uint8_t dim_period = color_thickness + black_thickness + 2*MAX_MID_DIMMING;
  uint8_t full_period = dim_period * MID_NUM_COLORS;
  int8_t ring_offset = scale_param(MID_RING_OFFSET, -1 * dim_period/2, dim_period/2);
  uint16_t extended_led_count = ((LEDS_PER_RING-1)/full_period+1)*full_period;
  int8_t alternating_multiplier = MID_INTRA_RING_MOTION;


  if(node_number*RINGS_PER_NODE > min_ring) { min_ring = node_number*RINGS_PER_NODE; }
  if((node_number+1)*RINGS_PER_NODE < max_ring) { max_ring = (node_number+1)*RINGS_PER_NODE; }
  for(uint8_t ring = min_ring; ring < max_ring; ring++) {
    if(MID_INTRA_RING_MOTION == ALTERNATE) { alternating_multiplier = ring % 2 == 0 ? -1 : 1; }
    int16_t motion_offset = ring*ring_offset + alternating_multiplier * intra_speed * mid_count / THROTTLE;
    while(motion_offset < 0) { motion_offset += extended_led_count; }

    for(uint16_t pixel = 0; pixel < extended_led_count; pixel++) {
      uint16_t idx = (pixel + motion_offset) % extended_led_count;
      if(idx >= LEDS_PER_RING) { continue; }

      // Each band is half one color and half the next
      uint8_t color_index = (((dim_period + pixel - color_thickness/2) % extended_led_count) / dim_period) % MID_NUM_COLORS;
      uint8_t pattern_idx = pixel % dim_period;

      if(pattern_idx < color_thickness) {
        mid_layer[ring][idx] = get_mid_color(color_index);
      }
      else if(pattern_idx < color_thickness + MAX_MID_DIMMING) {
        uint8_t dim_amount = 1 + (pattern_idx - color_thickness);
        mid_layer[ring][idx] = get_mid_color(color_index, dim_amount);
      }
      else if(pattern_idx < color_thickness + MAX_MID_DIMMING + black_thickness) {
        mid_layer[ring][idx] = TRANSPARENT;
      }
      else {
        uint8_t dim_amount = MAX_MID_DIMMING - (pattern_idx - color_thickness - black_thickness - MAX_MID_DIMMING);
        mid_layer[ring][idx] = get_mid_color(color_index, dim_amount);
      }
    }
  }
}

inline void mid_scrolling_dim5(uint8_t min_ring, uint8_t max_ring) {
  uint8_t color_thickness = scale_param(MID_COLOR_THICKNESS, 1, 6);
  uint8_t black_thickness = scale_param(MID_BLACK_THICKNESS, 6, 18);
  uint8_t intra_speed = 1 << scale_param(MID_INTRA_RING_SPEED, 3, 6);
  uint8_t dim_period = color_thickness + black_thickness + 2*MAX_MID_DIMMING;
  uint8_t full_period = dim_period * MID_NUM_COLORS;
  int8_t ring_offset = scale_param(MID_RING_OFFSET, -1 * dim_period/2, dim_period/2);
  uint16_t extended_led_count = ((LEDS_PER_RING-1)/full_period+1)*full_period;
  int8_t alternating_multiplier = MID_INTRA_RING_MOTION;


  if(node_number*RINGS_PER_NODE > min_ring) { min_ring = node_number*RINGS_PER_NODE; }
  if((node_number+1)*RINGS_PER_NODE < max_ring) { max_ring = (node_number+1)*RINGS_PER_NODE; }
  for(uint8_t ring = min_ring; ring < max_ring; ring++) {
    if(MID_INTRA_RING_MOTION == ALTERNATE) { alternating_multiplier = ring % 2 == 0 ? -1 : 1; }
    int16_t motion_offset = ring*ring_offset + alternating_multiplier * intra_speed * mid_count / THROTTLE;
    while(motion_offset < 0) { motion_offset += extended_led_count; }

    int16_t color_motion_offset = ring*ring_offset + 2 * alternating_multiplier * intra_speed * mid_count / THROTTLE;
    for(uint16_t pixel = 0; pixel < extended_led_count; pixel++) {
      uint16_t idx = (pixel + motion_offset) % extended_led_count;
      if(idx >= LEDS_PER_RING) { continue; }

      uint16_t color_idx = (pixel + color_motion_offset) % extended_led_count;

      // Color is dependent on position; color gradient that moves twice as fast as the dimming pattern
      uint8_t color_index = (color_idx / dim_period) % MID_NUM_COLORS;
      uint8_t next_color = (color_index + 1) % MID_NUM_COLORS;
      uint8_t blend_amount = (color_idx % dim_period) * MID_GRADIENT_SIZE / dim_period;
      uint8_t pattern_idx = pixel % dim_period;

      if(pattern_idx < color_thickness) {
        mid_layer[ring][idx] = get_mid_color(color_index, next_color, blend_amount);
      }
      else if(pattern_idx < color_thickness + MAX_MID_DIMMING) {
        uint8_t dim_amount = 1 + (pattern_idx - color_thickness);
        mid_layer[ring][idx] = get_mid_color(color_index, next_color, blend_amount, dim_amount);
      }
      else if(pattern_idx < color_thickness + MAX_MID_DIMMING + black_thickness) {
        mid_layer[ring][idx] = TRANSPARENT;
      }
      else {
        uint8_t dim_amount = MAX_MID_DIMMING - (pattern_idx - color_thickness - black_thickness - MAX_MID_DIMMING);
        mid_layer[ring][idx] = get_mid_color(color_index,  next_color, blend_amount, dim_amount);
      }
    }
  }
}



//-------------------------------- ARROW ---------------------------------
void init_arrow() {
  //number of transparent pixels between arrow start-end: choose such that integer number of arrows fit in structure
  //ie, (7+5)*6 = 72 = number of rings
  //uint8_t arrow_length = 7;
  uint8_t ring_spacing = 5;
  uint8_t pixel_spacing = 3; //transparent pixels between arrows
  uint16_t narrow = LEDS_PER_RING / pixel_spacing; //408 / 3 = 136
  uint8_t maxbrite = 7; //max value of last argument to get_mid_color: brightness == maxbrite 
  uint8_t maxdim = 12;
  for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
    for(uint8_t a = 0; a < narrow; a++) {
      uint16_t pixel = a * pixel_spacing;
      uint8_t arg = (ring+a*ring_spacing)%(maxbrite+ring_spacing);
      uint8_t c1 = pixel < 2*RINGS_PER_NODE/3 ? 0 : 1;
      uint8_t c2 = pixel <   RINGS_PER_NODE/3 ? 1 : 2;
      uint8_t b  = (pixel%narrow)*maxdim/narrow;
      mid_layer[ring][pixel] = (arg < maxbrite ? get_mid_color(c1, c2, b, arg) : TRANSPARENT);
    }
  }
}

void arrow() {
  uint8_t speed = 5; //only rotate every this mid_count
  //for(uint8_t i = 0; i < 5; i++) {
  if( mid_count % speed == 0 ) {
    move_mid_layer_inter_ring(CW);
  }
}


//-------------------------------- WAVE ---------------------------------
#include <math.h> //for sin
const uint8_t nwave = 12; //per half structure: each half is mirrored
uint16_t start_pixel[nwave] = {20, 30, 40, 50, 60, 80, 100, 110, 120, 130, 140, 180}; 
uint8_t  start_ring [nwave] = { 0,  9,  0,  4,  0, 36,   0,  12,   0,   4,   0,   0};
uint8_t  period     [nwave] = {12,  9,  6,  4, 18, 36,  18,  12,   6,   4,   9,  36}; //N = 6, 8, 12, 18, 4, 2, etc
uint8_t  wspeed     [nwave] = { 5,  4,  3,  2,  6,  8,   6,   5,   3,   2,   4,   8};
//uint8_t  amplitude  [nwave] = {5, 5, 5, 5}; //can put this back if desired

void wave() {

  //this loop structure means that the higher index waves will overwrite the lower
  for( uint8_t i=0; i < nwave*2; i++ ) {
	  //draw wave
	  //for( uint8_t j=start_ring[i]-1; j < start_ring[i]+period[i]; j++ ) { //1 period
    for(uint8_t j = 0; j < NUM_RINGS; j++) {
	    //uint16_t pixel = start_pixel[i%nwave] + amplitude[i%nwave] * sin(TWO_PI * j / period[i%nwave]); //use amplitude array
      uint16_t pixel = start_pixel[i%nwave] + period[i%nwave]/2 * sin(TWO_PI * j / period[i%nwave]); //amplitude=period/2
	    if( i < nwave )
	      pixel = LEDS_PER_RING - pixel; //mirror
      
      if( (j+start_ring[i%nwave]) % (period[i%nwave]*2) >= period[i%nwave] ) { //half structure
        mid_layer[j][pixel] = TRANSPARENT;
        continue;
      }
      uint8_t b = j%(NUM_RINGS/4) * 12 / (NUM_RINGS/4); //blend ranges over every 4th structure
      mid_layer[j][pixel] = get_mid_color(i%nwave < 2*nwave/3 ? 0 : 1, i%nwave < nwave/3 ? 1 : 2, b, 0);
      //Serial.print("r " + String(j) + " p " + String(pixel) + "; ");
	  }
    //Serial.println();
	  //move wave for next loop
	  if( mid_count % wspeed[i%nwave] == 0 )
	    start_ring[i%nwave] = (start_ring[i%nwave] < NUM_RINGS-1 ? ++start_ring[i%nwave] : 0);
  }
  
}


void init_radiation_symbol() {
  for(uint8_t ring = 0; ring < NUM_RINGS; ring++) {
    uint16_t pixel = 0;
    if((ring / 6) % 2 == 0) {
      for(; pixel < 250; pixel++) {
        mid_layer[ring][pixel] = current_palette[2];
      }
    }

    for(pixel = 250; pixel < LEDS_PER_RING-50; pixel++) {
      mid_layer[ring][pixel] = current_palette[2];
    }
  }
}

void radiation_symbol() {
  if(mid_count % 4 == 0) {
    move_mid_layer_inter_ring(CW);
  }
}



void ring_intersecting_periods() {
  static uint8_t ring[6];
  if(mid_count == 0) {
    uint8_t start = 6;//random8(72);
    for(uint8_t i = 0; i < 6; i++) {
      ring[i] = start;
    }
  }

  clear_mid_layer();
  
  ring[0] += 2;
  ring[1]++;
  if(mid_count % 3 == 0) ring[2]+=2;
  if(mid_count % 2 == 0) ring[3]++;
  if(mid_count % 5 == 0) ring[4]+=2;
  if(mid_count % 3 == 0) ring[5]++;

  for(uint8_t i = 0; i < 6; i++) {
    if(ring[i] >= NUM_RINGS) { ring[i] -= NUM_RINGS; }
  }

  if(ring[0] == ring[1] && ring[0] == ring[2] && ring[0] == ring[3] && ring[0] == ring[4] && ring[0] == ring[5]) {
    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      mid_layer[ring[0]][pixel] = WHITE;
    }
  }
  else {
    // 68 pixels per band
    uint16_t pixel;
    for(pixel = 0; pixel < 34; pixel++) {
      mid_layer[ring[5]][pixel] = current_palette[2];
      mid_layer[ring[5]][LEDS_PER_RING-1-pixel] = current_palette[2];
    }
    for(; pixel < 68; pixel++) {
      mid_layer[ring[4]][pixel] = current_palette[3];
      mid_layer[ring[4]][LEDS_PER_RING-1-pixel] = current_palette[3];
    }
    for(; pixel < 102; pixel++) {
      mid_layer[ring[3]][pixel] = current_palette[4];
      mid_layer[ring[3]][LEDS_PER_RING-1-pixel] = current_palette[4];
    }
    for(; pixel < 136; pixel++) {
      mid_layer[ring[2]][pixel] = current_palette[4];
      mid_layer[ring[2]][LEDS_PER_RING-1-pixel] = current_palette[4];
    }
    for(; pixel < 170; pixel++) {
      mid_layer[ring[1]][pixel] = current_palette[3];
      mid_layer[ring[1]][LEDS_PER_RING-1-pixel] = current_palette[3];
    }
    for(; pixel < 204; pixel++) {
      mid_layer[ring[0]][pixel] = current_palette[2];
      mid_layer[ring[0]][LEDS_PER_RING-1-pixel] = current_palette[2];
    }
  }
}

void square_pattern() {
  const uint8_t height = 24;
  const uint8_t width = 6;

  static uint8_t current_height1 = 0;
  static uint8_t current_width1 = 0;
  static uint8_t current_height2 = 0;
  static uint8_t current_width2 = 0;
  
  clear_mid_layer();

  // Draw vertical lines
  for(uint8_t ring = current_width1; ring < NUM_RINGS; ring += width) {
    if(ring > node_number*RINGS_PER_NODE && ring < (node_number+1)*RINGS_PER_NODE) {
      for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        mid_layer[ring][pixel] = current_palette[3];
      }
    }
  }

  for(uint8_t ring = current_width2; ring < NUM_RINGS; ring += width) {
    if(ring > node_number*RINGS_PER_NODE && ring < (node_number+1)*RINGS_PER_NODE) {
      for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        mid_layer[ring][pixel] = current_palette[3];
      }
    }
  }

  // Draw horizontal lines at 0, 24, 48, 72, ... <408
  for(uint8_t ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
    for(uint16_t pixel = current_height1; pixel < LEDS_PER_RING; pixel += height) {
      uint8_t phase = pixel % width;
      if(phase == current_width1 || phase == current_width2) { mid_layer[ring][pixel] = WHITE; }
      else { mid_layer[ring][pixel] = current_palette[2]; }
    }
    for(uint16_t pixel = current_height2; pixel < LEDS_PER_RING; pixel += height) {
      uint8_t phase = pixel % width;
      if(phase == current_width1 || phase == current_width2) { mid_layer[ring][pixel] = WHITE; }
      else { mid_layer[ring][pixel] = current_palette[2]; }
    }
  }

  if(++current_height1 == height) { current_height1 = 0; }
  if(--current_height2 == 255) { current_height2 = height - 1; }
  if(mid_count % (height/width) == 0) { if(++current_width1 == width) { current_width1 = 0; } }
  if(mid_count % (height/width) == 0) { if(--current_width2 == 255) { current_width2 = width; } }
}

