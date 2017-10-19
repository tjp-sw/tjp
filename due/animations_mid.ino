// This file is for completed mid animations

//-------------------------------- SNAKE --------------------------------
// Sends alternating bands of colors rotating around the rings
// Creates repeated snakes with MID_NUM_COLORS colors, with each color repeated COLOR_THICKNESS times, separated by BLACK_THICKNESS black LEDs.
// MID_NUM_COLORS(1:3), MID_COLOR_THICKNESS(2:5), MID_BLACK_THICKNESS(6:50), MID_INTRA_RING_MOTION(-1:1), MID_RING_OFFSET(-period/2:period/2), MID_INTRA_RING_SPEED(8-32)
// missing: INTER_RING_MOTION, INTER_RING_SPEED
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
// MID_INTRA_RING_MOTION: UP or DOWN; DOWN will spawn fire at the top and heat travels downwardl; ALTERNATE will do both directions
// MID_INTER_RING_SPEED(8) == wind speed, MID_RING_OFFSET(96) == wind decay amount in neighboring rings
// not using: everything else
#define SPARKING_RANGE 20   // How far from bottom sparks will form

#if NODE_NUMBER == 2
#define MAX_WIND_COOLING 12 // Largest extra cooling, at center of the wind
#else
#define MAX_WIND_COOLING 11 // Largest extra cooling, at center of the wind
#endif

#define MAX_WIND_RANGE 6    // Wind cooling reaches 6 rings in either direction from center
inline void fire(uint8_t palette_type, bool draw_inner_half, uint8_t min_ring, uint8_t max_ring) {
  fire(palette_type, draw_inner_half, min_ring, max_ring, 0);
}

inline void fire(uint8_t palette_type, bool draw_inner_half, uint8_t min_ring, uint8_t max_ring, uint8_t extra_cooling) {
  
  uint8_t cooling = palette_type == FIRE_PALETTE_DISABLED ? 13 : 3;//scale_param(MID_BLACK_THICKNESS, 15, 15);
  //if(MID_INTRA_RING_MOTION == ALTERNATE) { cooling += 6; }
  uint8_t sparking_chance = 220;//scale_param(MID_COLOR_THICKNESS, 130, 130);
  uint8_t min_spark_size = 170;//scale_param(MID_NUM_COLORS, 150, 150);
  uint8_t wind_speed = 5;//scale_param(MID_INTER_RING_SPEED, 8, 8);
  uint8_t wind_decay = 128;//scale_param(MID_RING_OFFSET, 96, 96);

  //draw_inner_half = true;
  random16_add_entropy(loop_count);
  
  if(palette_type != FIRE_PALETTE_DISABLED) {
    if(palette_type == FIRE_PALETTE_STANDARD) { set_fire_palette(); }
  }

  if(node_number*RINGS_PER_NODE > min_ring) { min_ring = node_number*RINGS_PER_NODE; }
  if((node_number+1)*RINGS_PER_NODE < max_ring) { max_ring = (node_number+1)*RINGS_PER_NODE; }
  
  // Step 1.  Cool down every 4th cell a little, and add an extra cooling wind that moves around the structure
  
  
  // Other direction
  //uint8_t coldest_ring = (mid_count * wind_speed / THROTTLE) % NUM_RINGS;

  // New direction
  uint8_t coldest_ring = NUM_RINGS - ((mid_count * wind_speed / THROTTLE) % NUM_RINGS);
  
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
    
    for(uint16_t pixel = 0; pixel < HALF_RING; pixel++) {
      mid_layer[ring][LEDS_PER_RING - 1 - pixel] = qsub8(mid_layer[ring][LEDS_PER_RING - 1 - pixel],  random8(0, cooling+wind_cooling+extra_cooling));
      if(draw_inner_half) {
        mid_layer[ring][pixel] = qsub8(mid_layer[ring][pixel], random8(0, cooling+wind_cooling+extra_cooling));
      }

      #if NODE_NUMBER == 2
        pixel ++;
      #endif
    }

    // Cool down cells that otherwise are never cooled and don't inherit heat from below
    if(MID_INTRA_RING_MOTION != DOWN) {
      mid_layer[ring][LEDS_PER_RING - 2] = qsub8(mid_layer[ring][LEDS_PER_RING-2], random8(0, cooling+wind_cooling+extra_cooling));
      mid_layer[ring][1] = qsub8(mid_layer[ring][1], random8(0, cooling+wind_cooling));
    }
    
    if(MID_INTRA_RING_MOTION != UP) {
      mid_layer[ring][HALF_RING - 1] = qsub8(mid_layer[ring][HALF_RING - 1], random8(0, cooling+wind_cooling+extra_cooling));
      mid_layer[ring][HALF_RING - 2] = qsub8(mid_layer[ring][HALF_RING - 2], random8(0, cooling+wind_cooling+extra_cooling));
      mid_layer[ring][HALF_RING] = qsub8(mid_layer[ring][HALF_RING], random8(0, cooling+wind_cooling+extra_cooling));
      mid_layer[ring][HALF_RING + 1] = qsub8(mid_layer[ring][HALF_RING + 1], random8(0, cooling+wind_cooling+extra_cooling));
    }
  }



  for(uint8_t ring = min_ring; ring < max_ring; ring++) {
    
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    if(MID_INTRA_RING_MOTION != DOWN) {
      uint16_t lower_limit = (MID_INTRA_RING_MOTION == ALTERNATE  || MID_INTRA_RING_MOTION == NONE) ? HALF_RING/2 : 1;
    
      for(uint16_t pixel = lower_limit; pixel < HALF_RING - 1; pixel++) {
        // Outer half
        mid_layer[ring][HALF_RING + pixel - 1] = (mid_layer[ring][HALF_RING + pixel] + mid_layer[ring][HALF_RING + pixel + 1]) / 2;
        
        // Inner half
        if(draw_inner_half) { 
          mid_layer[ring][HALF_RING - pixel] = (mid_layer[ring][HALF_RING - pixel - 1] + mid_layer[ring][HALF_RING - pixel - 2]) / 2;
        }
      }

      // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
      if(MID_INTRA_RING_MOTION == UP || MID_INTRA_RING_MOTION == ALTERNATE) {
        
        if(draw_inner_half && random8() < sparking_chance) {
          uint8_t pixel = random8(SPARKING_RANGE);
          mid_layer[ring][pixel] = qadd8(mid_layer[ring][pixel], random8(min_spark_size, 255));
        }
    
        if(random8() < sparking_chance) {
          uint16_t pixel = LEDS_PER_RING - 1 - random8(SPARKING_RANGE);
          mid_layer[ring][pixel] = qadd8(mid_layer[ring][pixel], random8(min_spark_size, 255));
        }
      }
    }

    if(MID_INTRA_RING_MOTION != UP) {
      // Step 2.  Heat from each cell drifts 'down' and diffuses a little
      uint16_t lower_limit = (MID_INTRA_RING_MOTION == ALTERNATE  || MID_INTRA_RING_MOTION == NONE) ? HALF_RING/2 : 1;
      
      for(uint16_t pixel = lower_limit; pixel < HALF_RING - 1; pixel++) {
        // Outer half
        mid_layer[ring][LEDS_PER_RING - pixel] = (mid_layer[ring][LEDS_PER_RING - pixel - 1] + mid_layer[ring][LEDS_PER_RING - pixel - 2]) / 2;
        
        // Inner half
        if(draw_inner_half) {
          mid_layer[ring][pixel - 1] = (mid_layer[ring][pixel] + mid_layer[ring][pixel + 1]) / 2;
        }
      }
      
      
      // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
      if(MID_INTRA_RING_MOTION == DOWN || MID_INTRA_RING_MOTION == ALTERNATE) {
        if(draw_inner_half && random8() < sparking_chance) {
          uint16_t pixel = HALF_RING - 1 - random8(SPARKING_RANGE);
          mid_layer[ring][pixel] = qadd8(mid_layer[ring][pixel], random8(min_spark_size, 255));
        }
        
        if(random8() < sparking_chance) {
          uint8_t pixel = HALF_RING + random8(SPARKING_RANGE);
          mid_layer[ring][pixel] = qadd8(mid_layer[ring][pixel], random8(min_spark_size, 255));
        }
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

  while(steps[0].r > 25 && steps[0].g > 25 && steps[0].b > 25) { steps[0]--; } // Manually increase saturation
  //while(steps[2].r < 255 && steps[2].g < 255 && steps[3].b < 255) { steps[2]++; }
  steps[2] |= CRGB(40, 40, 40); // Raise each channel to a minimum of 60
  steps[2].maximizeBrightness();
  steps[2] %= 64;// Set to 1/4 of max brightness

  CRGB start, target;

  // Step 1: Black to dim, saturated color0
  target = steps[0];
  for(uint8_t i = 0; i < 90; i++) {
    start = CRGB::Black;
    tjp_nblend(start, target, 255 * i / 90);
    mid_palette.entries[i] = start;
  }

  // Step 2: color0 to color1, up value and drop saturation
  target = steps[1];
  for(uint8_t i = 90; i < 160; i++) {
    start = steps[0];
    tjp_nblend(start, target, 255 * (i-90) / 70);
    mid_palette.entries[i] = start;
  }

  // Step 3: color1 to color2, up value and drop saturation
  target = steps[2];
  for(uint8_t i = 160; i < 240; i++) {
    start = steps[1];
    tjp_nblend(start, target, 255 * (i-160) / 80);
    mid_palette.entries[i] = start;
  }

  // Step 4: blend into (almost) fully bright white
  target = CRGB(255, 240, 230);
  tjp_nblend(target, steps[2], 100); // ~60% white
  for(uint16_t i = 240; i < 256; i++) {
    start = steps[2];
    tjp_nblend(start, target, 255 * (i-240) / 16);
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
// MID_NUM_COLORS(1:3), MID_COLOR_THICKNESS(1:6), MID_BLACK_THICKNESS(6:24), MID_INTRA_RING_MOTION(-1:1), MID_RING_OFFSET(-period/2:period/2), MID_INTRA_RING_SPEED(8:64)
// missing: INTER_RING_MOTION, INTER_RING_SPEED

inline void mid_scrolling_dim2(uint8_t min_ring, uint8_t max_ring) {
  uint8_t color_thickness = scale_param(MID_COLOR_THICKNESS, 1, 6);
  uint8_t black_thickness = scale_param(MID_BLACK_THICKNESS, 6, 24);
  uint8_t intra_speed = 1 << scale_param(MID_INTRA_RING_SPEED, 3, 5);
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
  //Serial.println("extended count = " + String(extended_led_count));
  //Serial.println("motion offset = " + String(motion_offset));
    while(motion_offset < 0) { motion_offset += extended_led_count; }
  //Serial.println("end");
    for(uint16_t pixel = 0; pixel < extended_led_count; pixel++) {
      //Serial.println(pixel);
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
        uint8_t dim_amount = MAX_MID_DIMMING - (pattern_idx - color_thickness - black_thickness - MAX_MID_DIMMING);
        mid_layer[ring][idx] = get_mid_color(color_index, next_color, blend_amount, dim_amount);
      }
    }
  }
}

inline void mid_scrolling_dim3(uint8_t min_ring, uint8_t max_ring) {
  uint8_t color_thickness = scale_param(MID_COLOR_THICKNESS, 1, 6);
  uint8_t black_thickness = scale_param(MID_BLACK_THICKNESS, 6, 24);
  uint8_t intra_speed = 1 << scale_param(MID_INTRA_RING_SPEED, 3, 5);
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

    //Serial.println(extended_led_count);
    for(uint16_t pixel = 0; pixel < extended_led_count; pixel++) {
      //Serial.println(pixel);
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
  uint8_t black_thickness = scale_param(MID_BLACK_THICKNESS, 6, 24);
  uint8_t intra_speed = 1 << scale_param(MID_INTRA_RING_SPEED, 3, 5);
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

    //Serial.println(extended_led_count);
    for(uint16_t pixel = 0; pixel < extended_led_count; pixel++) {
      //Serial.println(pixel);
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
  uint8_t black_thickness = scale_param(MID_BLACK_THICKNESS, 6, 24);
  uint8_t intra_speed = 1 << scale_param(MID_INTRA_RING_SPEED, 3, 5);
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

    int16_t color_motion_offset = -3 * alternating_multiplier * intra_speed * mid_count / THROTTLE;
    while(color_motion_offset < 0) { color_motion_offset += extended_led_count; }
    //Serial.println(extended_led_count);
    for(uint16_t pixel = 0; pixel < extended_led_count; pixel++) {
      //Serial.println(pixel);
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
// MID_NUM_COLORS(1:3), MID_COLOR_THICKNESS(1:6), MID_BLACK_THICKNESS(6:18), INTER_RING_MOTION, INTER_RING_SPEED
// missing: MID_RING_OFFSET, MID_INTRA_RING_SPEED, MID_INTRA_RING_MOTION
void init_arrow() {
  random16_set_seed(0);
  
  //number of transparent pixels between arrow start-end: choose such that integer number of arrows fit in structure
  //ie, (arrow_length + horizontal_spacing) should be a multiple of number of rings
  uint8_t horizontal_spacing = 6;//scale_param(MID_COLOR_THICKNESS, 4, 6);
  uint8_t vertical_spacing = scale_param(MID_BLACK_THICKNESS, 2, 6); //transparent pixels between arrows
  
  clear_mid_layer();
  
  for(uint8_t ring = 0; ring < NUM_RINGS; ring++) {
    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel += vertical_spacing) {
      uint8_t comet_num = pixel / vertical_spacing;
      uint8_t dimming = (ring + comet_num*horizontal_spacing) % (NUM_MID_DIMMING_LEVELS+horizontal_spacing);

      if(dimming < NUM_MID_DIMMING_LEVELS) {
        uint8_t arrow_color;
        if(pixel < LEDS_PER_RING/3) {
          uint8_t blend_amount = MID_GRADIENT_SIZE * pixel / (LEDS_PER_RING/3);
          arrow_color = MID_NUM_COLORS == 1 ? get_mid_color(0, dimming) : get_mid_color(0, 1, blend_amount, dimming);
        }
        else if(pixel < LEDS_PER_RING*2/3) {
          if(MID_NUM_COLORS == 1) { arrow_color = get_mid_color(0, dimming); }
          else {
            uint8_t second_color = MID_NUM_COLORS == 2 ? 0 : 2;
            uint8_t blend_amount = MID_GRADIENT_SIZE * (pixel - LEDS_PER_RING/3) / (LEDS_PER_RING/3);
            arrow_color = get_mid_color(1, second_color, blend_amount, dimming);
          }
        }
        else {
          if(MID_NUM_COLORS == 1) { arrow_color = get_mid_color(0, dimming); }
          else if(MID_NUM_COLORS == 2) { arrow_color = mid_layer[ring][pixel - LEDS_PER_RING*2/3]; }
          else {
            uint8_t blend_amount = MID_GRADIENT_SIZE * (pixel - LEDS_PER_RING*2/3) / (LEDS_PER_RING/3);
            arrow_color = get_mid_color(2, 0, blend_amount, dimming);
          }
        }

        mid_layer[ring][pixel] = arrow_color;
      }
    }
  }
}

void arrow() {
  uint8_t throttle = 5 - scale_param(MID_INTER_RING_SPEED, 0, 4); //only rotate every this mid_count
  if(throttle == 0) {
    move_mid_layer_inter_ring(MID_INTER_RING_MOTION == CW ? CW : CCW);
    move_mid_layer_inter_ring(MID_INTER_RING_MOTION == CW ? CW : CCW);
  }
  else if( mid_count % throttle == 0 ) {
    move_mid_layer_inter_ring(MID_INTER_RING_MOTION == CW ? CW : CCW);
  }
}



//-------------------------------- RADIATION SYMBOL ---------------------------------
// MID_NUM_COLORS(1:3), MID_COLOR_THICKNESS(1:6), MID_BLACK_THICKNESS(6:18), INTER_RING_MOTION, INTER_RING_SPEED
// not using: MID_INTRA_RING_SPEED, MID_INTRA_RING_MOTION, MID_RING_OFFSET
void radiation_symbol() {
  uint8_t inter_speed = 1 << scale_param(MID_INTER_RING_SPEED, 3, 6);
  int8_t alternating_multiplier = MID_INTER_RING_MOTION <= 0 ? CW : CCW;

  int16_t motion_offset = alternating_multiplier * inter_speed * mid_count / THROTTLE;
  while(motion_offset < 0) { motion_offset += NUM_RINGS; }
  
  for(uint8_t orig_ring = 0; orig_ring < NUM_RINGS; orig_ring++) {
    uint8_t ring = (orig_ring + motion_offset) % NUM_RINGS;

    uint8_t color_index = get_mid_color((orig_ring / (NUM_RINGS/3)) % MID_NUM_COLORS);
    if((orig_ring / 12) % 2 == 0) {
      for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        mid_layer[ring][pixel] = color_index;
      }
    }
    else {
      for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        mid_layer[ring][pixel] = TRANSPARENT;
      }
    }
  }
}


//-------------------------------- SQUARE PATTERN ---------------------------------
// MID_NUM_COLORS(1:3), MID_COLOR_THICKNESS, MID_BLACK_THICKNESS, INTER_RING_SPEED(1/4:1), MID_INTRA_RING_SPEED(1/4:1), MID_RING_OFFSET(-height/2:height/2)
// not using: INTER_RING_MOTION, MID_INTRA_RING_MOTION
void square_pattern() {
  static uint8_t current_height1 = 0;
  static uint8_t current_width1 = 0;
  static uint8_t current_height2 = 0;
  static uint8_t current_width2 = 0;

  uint8_t height = scale_param(MID_BLACK_THICKNESS, 18, 36);
  if(height % 2 == 1) { height--; }
  uint8_t width = scale_param(MID_BLACK_THICKNESS, 9, 18);
  if(width % 2 == 1) { width--; }
  
  uint8_t vert_color_thickness = scale_param(MID_COLOR_THICKNESS, 1, 2);
  uint8_t hor_color_thickness = scale_param(MID_COLOR_THICKNESS, 1, height/4);
  
  uint8_t horizontal_throttle = 4 - scale_param(MID_INTRA_RING_SPEED, 1, 3);
  uint8_t vertical_throttle = 4 - scale_param(MID_INTER_RING_SPEED, 1, 3);
  
  int8_t ring_offset = scale_param(MID_RING_OFFSET, -1 * height/8, height/8);
  
  uint8_t vertical_color = get_mid_color(0);
  uint8_t horizontal_color = get_mid_color(MID_NUM_COLORS-1);
  
  clear_mid_layer();

  // Draw vertical lines
  for(uint8_t ring = current_width1; ring < NUM_RINGS; ring += width) {
    if(ring >= node_number*RINGS_PER_NODE && ring < (node_number+1)*RINGS_PER_NODE) {
      for(uint8_t i = 0; i < vert_color_thickness; i++) {
        uint8_t actual_ring = (ring+i) % NUM_RINGS;
        for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
          mid_layer[actual_ring][pixel] = vertical_color;
        }
      }
    }
  }

  for(uint8_t ring = current_width2; ring < NUM_RINGS; ring += width) {
    if(ring >= node_number*RINGS_PER_NODE && ring < (node_number+1)*RINGS_PER_NODE) {
      for(uint8_t i = 0; i < vert_color_thickness; i++) {
        uint8_t actual_ring = (ring+i) % NUM_RINGS;
        for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
          mid_layer[actual_ring][pixel] = vertical_color;
        }
      }
    }
  }


  // Draw horizontal lines at 0, 24, 48, 72, ... <408
  for(uint8_t ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
    int16_t this_ring_current_height1 = current_height1 + ring*ring_offset;
    while(this_ring_current_height1 < 0) { this_ring_current_height1 += height; }
    this_ring_current_height1 %= height;
    
    int16_t this_ring_current_height2 = current_height2 + ring*ring_offset;
    while(this_ring_current_height2 < 0) { this_ring_current_height2 += height; }
    this_ring_current_height2 %= height;


    for(uint8_t pixel_offset = 0; pixel_offset < hor_color_thickness; pixel_offset++) {
      for(uint16_t pixel = this_ring_current_height1; pixel < LEDS_PER_RING; pixel += height) {
        uint16_t actual_pixel = (pixel + pixel_offset) % LEDS_PER_RING;
        
        uint8_t phase = ring % width;
        uint8_t col = horizontal_color;
        for(uint8_t ring_offset = 0; ring_offset < vert_color_thickness; ring_offset++) {
          uint8_t actual_cur_width1 = (current_width1 + ring_offset) % NUM_RINGS;
          uint8_t actual_cur_width2 = (current_width2 + ring_offset) % NUM_RINGS;
          if(phase == actual_cur_width1 || phase == actual_cur_width2) { col = MID_LAYER_SPARKLE; }
        }
        mid_layer[ring][actual_pixel] = col;
      }
      
      for(uint16_t pixel = this_ring_current_height2; pixel < LEDS_PER_RING; pixel += height) {
        uint16_t actual_pixel = (pixel + pixel_offset) % LEDS_PER_RING;
        
        uint8_t phase = ring % width;
        uint8_t col = horizontal_color;
        for(uint8_t ring_offset = 0; ring_offset < vert_color_thickness; ring_offset++) {
          uint8_t actual_cur_width1 = (current_width1 + ring_offset) % NUM_RINGS;
          uint8_t actual_cur_width2 = (current_width2 + ring_offset) % NUM_RINGS;
          if(phase == actual_cur_width1 || phase == actual_cur_width2) { col = MID_LAYER_SPARKLE; }
        }
        mid_layer[ring][actual_pixel] = col;
      }
    }
  }

  if(mid_count % horizontal_throttle == 0) {
    if(++current_height1 == height) { current_height1 = 0; }
    if(--current_height2 == 255) { current_height2 = height - 1; }
  }

  if(mid_count % vertical_throttle == 0) {
    if((mid_count/vertical_throttle) % (height/width) == 0) {
      if(++current_width1 == width) { current_width1 = 0; }
      if(--current_width2 == 255) { current_width2 = width-1; }
    }
  }
}

void square_pattern2() {
  static bool moving_height = true;
  static uint8_t vert_col_index1 = 0;
  static uint8_t vert_col_index2 = 1 % MID_NUM_COLORS;
  static uint8_t hor_col_index1 = 2 % MID_NUM_COLORS;
  static uint8_t hor_col_index2 = 0;
  
  uint8_t vertical_color1 = vert_col_index1 == MID_NUM_COLORS ? BLACK : get_mid_color(vert_col_index1);
  uint8_t vertical_color2 = vert_col_index2 == MID_NUM_COLORS ? BLACK : get_mid_color(vert_col_index2);
  uint8_t horizontal_color1 = hor_col_index1 == MID_NUM_COLORS ? BLACK : get_mid_color(hor_col_index1);
  uint8_t horizontal_color2 = hor_col_index2 == MID_NUM_COLORS ? BLACK : get_mid_color(hor_col_index2);
  
  static uint8_t current_height1 = 0;
  static uint8_t current_width1 = 0;
  static uint8_t current_height2 = 0;
  static uint8_t current_width2 = 0;

  uint8_t height = scale_param(MID_BLACK_THICKNESS, 18, 36);
  if(height % 2 == 1) { height--; }
  uint8_t width = scale_param(MID_BLACK_THICKNESS, 9, 18);
  if(width % 2 == 1) { width--; }
  
  uint8_t vert_color_thickness = scale_param(MID_COLOR_THICKNESS, 1, 2);
  uint8_t hor_color_thickness = scale_param(MID_COLOR_THICKNESS, 1, height/4);
  
  uint8_t horizontal_throttle = 3 - scale_param(MID_INTRA_RING_SPEED, 1, 2);
  uint8_t vertical_throttle = 3 - scale_param(MID_INTER_RING_SPEED, 1, 2);
  
  int8_t ring_offset = scale_param(MID_RING_OFFSET, -1 * height/8, height/8);
  
  clear_mid_layer();

  // Draw vertical lines
  for(uint8_t ring = current_width1; ring < NUM_RINGS; ring += width) {
    if(ring >= node_number*RINGS_PER_NODE && ring < (node_number+1)*RINGS_PER_NODE) {
      for(uint8_t i = 0; i < vert_color_thickness; i++) {
        uint8_t actual_ring = (ring+i) % NUM_RINGS;
        for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
          mid_layer[actual_ring][pixel] = vertical_color1;
        }
      }
    }
  }

  for(uint8_t ring = current_width2; ring < NUM_RINGS; ring += width) {
    if(ring >= node_number*RINGS_PER_NODE && ring < (node_number+1)*RINGS_PER_NODE) {
      for(uint8_t i = 0; i < vert_color_thickness; i++) {
        uint8_t actual_ring = (ring+i) % NUM_RINGS;
        for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
          mid_layer[actual_ring][pixel] = vertical_color2;
        }
      }
    }
  }


  // Draw horizontal lines at 0, 24, 48, 72, ... <408
  for(uint8_t ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
    int16_t this_ring_current_height1 = current_height1 + ring*ring_offset;
    while(this_ring_current_height1 < 0) { this_ring_current_height1 += height; }
    this_ring_current_height1 %= height;
    
    int16_t this_ring_current_height2 = current_height2 + ring*ring_offset;
    while(this_ring_current_height2 < 0) { this_ring_current_height2 += height; }
    this_ring_current_height2 %= height;


    for(uint8_t pixel_offset = 0; pixel_offset < hor_color_thickness; pixel_offset++) {
      for(uint16_t pixel = this_ring_current_height1; pixel < LEDS_PER_RING; pixel += height) {
        uint16_t actual_pixel = (pixel + pixel_offset) % LEDS_PER_RING;
        
        uint8_t phase = ring % width;
        uint8_t col = horizontal_color1;
        for(uint8_t ring_offset = 0; ring_offset < vert_color_thickness; ring_offset++) {
          uint8_t actual_cur_width1 = (current_width1 + ring_offset) % NUM_RINGS;
          uint8_t actual_cur_width2 = (current_width2 + ring_offset) % NUM_RINGS;
          if(phase == actual_cur_width1 || phase == actual_cur_width2) { col = MID_LAYER_SPARKLE; }
        }
        mid_layer[ring][actual_pixel] = col;
      }
      
      for(uint16_t pixel = this_ring_current_height2; pixel < LEDS_PER_RING; pixel += height) {
        uint16_t actual_pixel = (pixel + pixel_offset) % LEDS_PER_RING;
        
        uint8_t phase = ring % width;
        uint8_t col = horizontal_color2;
        for(uint8_t ring_offset = 0; ring_offset < vert_color_thickness; ring_offset++) {
          uint8_t actual_cur_width1 = (current_width1 + ring_offset) % NUM_RINGS;
          uint8_t actual_cur_width2 = (current_width2 + ring_offset) % NUM_RINGS;
          if(phase == actual_cur_width1 || phase == actual_cur_width2) { col = MID_LAYER_SPARKLE; }
        }
        mid_layer[ring][actual_pixel] = col;
      }
    }
  }

  if(moving_height) {
    if(mid_count % horizontal_throttle == 0) {
      if(++current_height1 == height) { current_height1 = 0; }
      if(--current_height2 == 255) { current_height2 = height - 1; }
      if(current_height1 == current_height2) {
        moving_height = false;
        hor_col_index1 = random8(MID_NUM_COLORS + 1);
        vert_col_index2 = random8(MID_NUM_COLORS + 1);
        if(hor_col_index2 == MID_NUM_COLORS && vert_col_index2 == MID_NUM_COLORS) {
          hor_col_index1 = random8(MID_NUM_COLORS);
        }
      }
    }
  }
  else {
    if(mid_count % vertical_throttle == 0) {
      if((mid_count/vertical_throttle) % (height/width) == 0) {
        if(++current_width1 == width) { current_width1 = 0; }
        if(--current_width2 == 255) { current_width2 = width-1; }
        if(current_width1 == current_width2) {
          moving_height = true;
          vert_col_index1 = random8(MID_NUM_COLORS + 1);
          hor_col_index2 = random8(MID_NUM_COLORS + 1);
          if(hor_col_index2 == MID_NUM_COLORS && vert_col_index2 == MID_NUM_COLORS) {
            hor_col_index2 = random8(MID_NUM_COLORS);
          }
        }
      }
    }
  }
}



//------------------------------------------- UNDER DEVELOPMENT --------------------------------------------
//--------------------- Only move above this line when code has been thoroughly tested ---------------------
//--------------------- Only include in allowable animations when moved above this line --------------------
//----------------------------------------------------------------------------------------------------------



//-------------------------------- WAVE ---------------------------------
#include <math.h> //for sin
//used: MID_COLOR_THICKNESS (1:2), MID_NUM_COLORS(1:3)
//? to use: MID_BLACK_THICKNESS, INTER_RING_SPEED(1/4:1), MID_RING_OFFSET(-height/2:height/2), INTER_RING_MOTION,
// not using: MID_INTRA_RING_MOTION, MID_INTRA_RING_SPEED(1/4:1)
const uint8_t nwave = 12; //per half structure: each half is mirrored
uint16_t wave_start_pixel[nwave] = {20, 40, 55, 75, 95, 95, 115, 135, 150, 165, 185, LEDS_PER_RING/2}; //204
uint8_t  wave_start_ring [nwave] = { 0,  9,  0, 12,  0,  6,   0,   9,   0,   6,   0, 36};
uint8_t  wave_period     [nwave] = {12,  9,  6, 12, 18,  6,  12,   9,   6,   6,  18, 36}; //N = 6, 8, 12, 18, 4, 2, etc (leave int for % arg)
uint8_t  wave_speed     [nwave] = { 5,  4,  3,  2,  5,  3,   5,   3,   2,   2,   3,  6};
//uint8_t  amplitude  [nwave] = {5, 5, 5, 5}; //can put this back if desired

void wave() {
  dim_mid_layer(6); //arg is number dim units per cycle: large values dim faster
  //this loop structure means that the higher index waves will overwrite the lower
  uint8_t thickness = 3;//scale_param(MID_COLOR_THICKNESS, 1, 3); 
  for( uint8_t i=0; i < nwave*2; i++ ) { //CHANGE BACK TO nwave*2
    for(uint8_t j = 0; j < NUM_RINGS; j++) {
      if( (j+wave_start_ring[i%nwave]) % (wave_period[i%nwave]*2) >= wave_period[i%nwave] ) { //half structure
        continue;
      }
      //uint16_t pixel = wave_start_pixel[i%nwave] + amplitude[i%nwave] * sin(TWO_PI * j / wave_period[i%nwave]); //use amplitude array
      uint16_t pixel = round(float(wave_start_pixel[i%nwave]) + max(4.,float(wave_period[i%nwave]/2)) * sin(TWO_PI * j / wave_period[i%nwave])); //amplitude=period/2
      if( i < nwave )
        pixel = LEDS_PER_RING - pixel; //mirror
      
      uint8_t b = j%(NUM_RINGS/4) * (MID_GRADIENT_SIZE-1) / (NUM_RINGS/4); //blend ranges over every 4th structure
      uint8_t c1 = getWaveC1(i), c2 = getWaveC2(i);
      for( uint8_t k = 1; k < thickness+1; k++ )
        mid_layer[j][pixel+k] = get_mid_color(c1, c2, b, 0);
      //Serial.print("r " + String(j) + " p " + String(pixel) + "; ");
    }
    //Serial.println();
    //move wave for next loop
    if( mid_count % wave_speed[i%nwave] == 0 )
      wave_start_ring[i%nwave] = (wave_start_ring[i%nwave] < NUM_RINGS-1 ? wave_start_ring[i%nwave]+1 : 0);
  }
} //end wave()

uint8_t getWaveC1(uint8_t i) {
  if( MID_NUM_COLORS <= 1 )
    return 0;
  else if( MID_NUM_COLORS == 2 )
    return i%2;
  else
    return i%3;
}
uint8_t getWaveC2(uint8_t i) {
  if( MID_NUM_COLORS <= 1 )
    return 0;
  else if( MID_NUM_COLORS == 2 )
    return (i+1)%2;
  else
    return (i+1)%3;
}


//-------------------------------- ROTATING RINGS ---------------------------------
// needs love
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


// -------------------- A dynamic arrow-like animation --------------------------
void dim_mid_layer(uint8_t amount) {
  for(uint8_t ring = 0; ring < NUM_RINGS; ring++) {
    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      if(get_mid_dim_value(ring, pixel) > MAX_MID_DIMMING - amount) { mid_layer[ring][pixel] = TRANSPARENT; }
      else { mid_layer[ring][pixel] += amount;}
    }
  }
}

// NUM_COLORS, COLOR_THICKNESS, BLACK_THICKNESS, RING_OFFSET, INTER_RING_MOTION, INTER_RING_SPEED, INTRA_RING_MOTION, INTRA_RING_SPEED, RING_OFFSET
void arrow_new() {
  uint8_t color_thickness = 1;//scale_param(MID_COLOR_THICKNESS, 1, 5);
  //uint8_t horizontal_spacing = 5;//scale_param(MID_BLACK_THICKNESS, 3, 8);
  uint8_t vertical_spacing = 3;//scale_param(MID_BLACK_THICKNESS, 2, 7) + color_thickness;
  uint8_t inter_speed = 1 << scale_param(MID_INTER_RING_SPEED, 3, 6);
  //uint8_t intra_speed = 1 << scale_param(MID_INTRA_RING_SPEED, 3, 6);
  uint8_t ring_offset = 0;//scale_param(MID_RING_OFFSET, 1, 10);
  
  dim_mid_layer(1);

  for(uint8_t ring = node_number * RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
    uint8_t first_pixel = ring*ring_offset + inter_speed*mid_count/THROTTLE;
    for(uint16_t raw_pixel = first_pixel; raw_pixel < LEDS_PER_RING; raw_pixel += vertical_spacing) {
      for(uint8_t pixel_offset = 0; pixel_offset < color_thickness; pixel_offset++) {
        uint16_t pixel = raw_pixel + pixel_offset;
        mid_layer[ring][pixel] = WHITE;
      }
    }
  }
}
