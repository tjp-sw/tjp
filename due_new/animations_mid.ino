// This file is for completed mid animations

//-------------------------------- SNAKE --------------------------------
// Sends alternating bands of colors rotating around the rings
// Creates repeated snakes with MID_NUM_COLORS colors, with each color repeated COLOR_THICKNESS times, separated by BLACK_THICKNESS black LEDs.
// MID_NUM_COLORS(1:3), MID_COLOR_THICKNESS(2:5), MID_BLACK_THICKNESS(5:20), MID_INTRA_RING_MOTION(-1:1), MID_RING_OFFSET(-period/2:period/2), MID_INTRA_RING_SPEED(8-32)
void snake(uint8_t ring_mode) {
  uint8_t color_thickness = scale_param(MID_COLOR_THICKNESS, 2, 5);
  uint8_t black_thickness = scale_param(MID_BLACK_THICKNESS, 5, 50);
  uint8_t intra_speed = scale_param(MID_INTRA_RING_SPEED, 8, 32);
  uint8_t period = MID_NUM_COLORS * color_thickness + black_thickness;
  int8_t ring_offset = scale_param(MID_RING_OFFSET, -1 * period/2, period/2);
  uint16_t extended_led_count = ((LEDS_PER_RING-1)/period+1)*period;
  uint8_t alternating_multiplier = 1;

  uint8_t ring_start = node_number*RINGS_PER_NODE + (ring_mode == ODD_RINGS ? 1 : 0);
  uint8_t ring_inc = ring_mode == ALL_RINGS ? 1 : 2;
  for (uint8_t ring = ring_start; ring < (node_number+1)*RINGS_PER_NODE; ring += ring_inc) {
      // if alternating rotation style, alternate direction of every other ring
      alternating_multiplier = ((MID_INTRA_RING_MOTION == ALTERNATE) && ((ring % 2) == 0)) ? -1 : 1;
    
      for (uint16_t pixel = 0; pixel < extended_led_count; pixel++) {
          uint16_t idx = (pixel + ring_offset*ring + alternating_multiplier * MID_INTRA_RING_MOTION * intra_speed * mid_count / THROTTLE) % extended_led_count;
          if(idx >= LEDS_PER_RING) { continue; }

          uint8_t pattern_idx = pixel % period;

          for (int color = 0; color <= MID_NUM_COLORS; color++) {
              if(pattern_idx < black_thickness) {
                  mid_layer[ring][idx] = TRANSPARENT;
              }
              else if(pattern_idx < black_thickness + color_thickness) {
                  mid_layer[ring][idx] = get_mid_color(color);
              }

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
}


//-------------------------------- SNAKE --------------------------------
// Sends alternating bands of colors rotating around the rings
// Creates repeated snakes with MID_NUM_COLORS colors, with each color repeated COLOR_THICKNESS times, separated by BLACK_THICKNESS black LEDs.
// MID_NUM_COLORS(1:3), MID_COLOR_THICKNESS(2:5), MID_BLACK_THICKNESS(5:20), MID_INTRA_RING_MOTION(-1:1), MID_RING_OFFSET(-period/2:period/2), MID_INTRA_RING_SPEED(8-32)
void old_snake(uint8_t ring_mode) {
  uint8_t color_thickness = scale_param(MID_COLOR_THICKNESS, 2, 5);
  uint8_t black_thickness = scale_param(MID_BLACK_THICKNESS, 5, 50);
  uint8_t intra_speed = scale_param(MID_INTRA_RING_SPEED, 8, 32);
  uint8_t period = MID_NUM_COLORS * color_thickness + black_thickness;
  int8_t ring_offset = scale_param(MID_RING_OFFSET, -1 * period/2, period/2);
  uint16_t extended_led_count = ((LEDS_PER_RING-1)/period+1)*period;

  uint8_t ring_start = node_number*RINGS_PER_NODE + (ring_mode == ODD_RINGS ? 1 : 0);
  uint8_t ring_inc = ring_mode == ALL_RINGS ? 1 : 2;
  for (uint8_t ring = ring_start; ring < (node_number+1)*RINGS_PER_NODE; ring += ring_inc) {
    for (uint16_t pixel = 0; pixel < extended_led_count; pixel++) {
      uint16_t idx = (pixel + ring_offset*ring + MID_INTRA_RING_MOTION * intra_speed * mid_count / THROTTLE) % extended_led_count;
      if(idx >= LEDS_PER_RING) { continue; }

      uint8_t pattern_idx = pixel % period;
      if(pattern_idx < black_thickness) {
        mid_layer[ring][idx] = TRANSPARENT;
      }
      else if(pattern_idx < black_thickness + color_thickness) {
        mid_layer[ring][idx] = get_mid_color(0);
      }
      else if(pattern_idx < black_thickness + 2*color_thickness) {
        mid_layer[ring][idx] = get_mid_color(1);
      }
      else {
        mid_layer[ring][idx] = get_mid_color(2);
      }
    }
  }
}


//-------------------------------- FIRE ---------------------------------
// Adapted from: Fire2012 by Mark Kriegsman, July 2012, part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
// Changes palette to a heat scale, and values in mid_layer represent the amount of heat in each pixel
// The palette_type parameter changes how the heat scale is constructed. Setting to 0 will skip this step and just use the actual palette.
// MID_BLACK_THICKNESS(15) == cooling, MID_COLOR_THICKNESS(130) == sparking chance, MID_NUM_COLORS(150) == minimum spark size
// A cooling wind rolls around the structure: MID_INTER_RING_SPEED(8) == wind speed, MID_RING_OFFSET(96) == wind decay amount in neighboring rings
void set_fire_palette() {
  // change mid_palette to heat scale
  CRGB steps[3];
  steps[0] = current_palette[2];
  steps[1] = current_palette[3];
  steps[2] = current_palette[4];

  steps[0].maximizeBrightness();
  steps[1].maximizeBrightness();

  steps[0] %= 75; // Set to 75/255 % of max brightness
  steps[1] %= 150;// Set to 150/255 % of max brightness

  while(steps[0].r > 5 && steps[0].g > 5 && steps[0].b > 5) { steps[0]--; } // Manually increase saturation
  //while(steps[2].r < 255 && steps[2].g < 255 && steps[3].b < 255) { steps[2]++; }
  steps[2] |= CRGB(40, 40, 40); // Raise each channel to a minimum of 60
  steps[2].maximizeBrightness();

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
  for(uint8_t i = 160; i < 225; i++) {
    start = steps[1];
    nblend(start, target, 255 * (i-160) / 65);
    mid_palette.entries[i] = start;
  }

  // Step 4: blend into fully bright white
  target = CRGB(255, 255, 255);
  for(uint16_t i = 225; i < 256; i++) {
    start = steps[2];
    nblend(start, target, 255 * (i-225) / 31);
    mid_palette.entries[i] = start;
  }
}

void cleanup_fire() {
  // change mid_palette from heat scale back to normal palette with dimming and gradients
  create_mid_palette(&mid_palette, current_palette[2], current_palette[3], current_palette[4]);
}

#define SPARKING_RANGE 20   // How far from bottom sparks will form
#define MAX_WIND_COOLING 60 // Largest extra cooling, at center of the wind
#define MAX_WIND_RANGE 6    // Wind cooling reaches 6 rings in either direction from center
void fire(uint8_t ring_mode, uint8_t palette_type) {
  uint8_t cooling = 15;//scale_param(MID_BLACK_THICKNESS, 15, 15);
  uint8_t sparking_chance = 130;//scale_param(MID_COLOR_THICKNESS, 130, 130);
  uint8_t min_spark_size = 150;//scale_param(MID_NUM_COLORS, 150, 150);
  uint8_t wind_speed = 8;//scale_param(MID_INTER_RING_SPEED, 8, 8);
  uint8_t wind_decay = 96;//scale_param(MID_RING_OFFSET, 96, 96);
  
  if(palette_type != FIRE_PALETTE_DISABLED) {
    if(palette_type == FIRE_PALETTE_STANDARD) { set_fire_palette(); }
  }

  uint8_t ring_start = node_number*RINGS_PER_NODE + (ring_mode == ODD_RINGS ? 1 : 0);
  uint8_t ring_inc = ring_mode == ALL_RINGS ? 1 : 2;
  
  // Step 1.  Cool down every 4th cell a little, and add an extra cooling wind that moves around the structure
  uint8_t coldest_ring = (mid_count * wind_speed / THROTTLE) % NUM_RINGS;
  for(uint8_t ring = ring_start; ring < (node_number+1)*RINGS_PER_NODE; ring += ring_inc) {
    uint8_t dist = ring > coldest_ring ? ring - coldest_ring : coldest_ring - ring;

    uint8_t wind_cooling = 0;
    if(dist <= MAX_WIND_RANGE) {
      wind_cooling = MAX_WIND_COOLING;
      while(dist-- > 0) {
        wind_cooling = wind_cooling * wind_decay / 127;
      }
    }
    
    
    for(uint16_t pixel = 0; pixel < HALF_RING; pixel+=4) {
      uint8_t extra_cooling = wind_cooling;
      //if((pixel < 2*SPARKING_RANGE) || (pixel > NUM_LEDS-1 - 2*SPARKING_RANGE))
        //extra_cooling = 255;
      //if((pixel < 4*SPARKING_RANGE) || (pixel > NUM_LEDS-1 - 4*SPARKING_RANGE))
        //extra_cooling = wind_cooling * 3;
      mid_layer[ring][pixel] = qsub8(mid_layer[ring][pixel],  random(0, cooling+extra_cooling));
      mid_layer[ring][HALF_RING + pixel] = qsub8(mid_layer[ring][HALF_RING + pixel],  random(0, cooling+extra_cooling));
    }
  }

  for(uint8_t ring = ring_start; ring < (node_number+1)*RINGS_PER_NODE; ring += ring_inc) {
    
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for(uint16_t pixel = 1; pixel < HALF_RING - 1; pixel++) {
      // Inner half
      //mid_layer[ring][HALF_RING - pixel] = (mid_layer[ring][HALF_RING - pixel] + mid_layer[ring][HALF_RING - pixel - 1] + mid_layer[ring][HALF_RING - pixel - 2]) / 2;
      mid_layer[ring][HALF_RING - pixel] = (mid_layer[ring][HALF_RING - pixel - 1] + mid_layer[ring][HALF_RING - pixel - 2]) / 2;
      
      // Outer half
      //mid_layer[ring][HALF_RING + pixel - 1] = (mid_layer[ring][HALF_RING + pixel - 1] + mid_layer[ring][HALF_RING + pixel] + mid_layer[ring][HALF_RING + pixel + 1]) / 2;
      mid_layer[ring][HALF_RING + pixel - 1] = (mid_layer[ring][HALF_RING + pixel] + mid_layer[ring][HALF_RING + pixel + 1]) / 2;
    }

  
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if(random(256) < sparking_chance) {
      uint8_t pixel = random8(SPARKING_RANGE);
      mid_layer[ring][pixel] = qadd8(mid_layer[ring][pixel], random(min_spark_size, 255));
    }

    if(random(256) < sparking_chance) {
      uint16_t pixel = LEDS_PER_RING - 1 - random(SPARKING_RANGE);
      mid_layer[ring][pixel] = qadd8(mid_layer[ring][pixel], random(min_spark_size, 255));
    }
  }

  for(uint8_t ring = ring_start; ring < (node_number+1)*RINGS_PER_NODE; ring += ring_inc) {
    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      if(mid_layer[ring][pixel] < 9) { mid_layer[ring][pixel] = TRANSPARENT; }
    }
  }
}


//-------------------------------- SCROLLING DIM ---------------------------------
// Draws single-colored bands that fade in and out and scroll around rings
// MID_NUM_COLORS(1:3), MID_COLOR_THICKNESS(1:6), MID_BLACK_THICKNESS(2:15), MID_INTRA_RING_MOTION(-1:1), MID_RING_OFFSET(-period/2:period/2), MID_INTRA_RING_SPEED(4:64)
void mid_scrolling_dim(uint8_t color_mode) {
  uint8_t color_thickness = scale_param(MID_COLOR_THICKNESS, 1, 6);
  uint8_t black_thickness = scale_param(MID_BLACK_THICKNESS, 2, 15);
  uint8_t intra_speed = scale_param(MID_INTRA_RING_SPEED, 4, 64);
  uint8_t dim_period = color_thickness + black_thickness + 2*MAX_DIMMING;
  uint8_t full_period = dim_period * MID_NUM_COLORS;
  int8_t ring_offset = scale_param(MID_RING_OFFSET, -1 * dim_period/2, dim_period/2);
  uint16_t extended_led_count = ((LEDS_PER_RING-1)/full_period+1)*full_period;

  for(uint8_t ring = node_number * RINGS_PER_NODE; ring < (node_number + 1) * RINGS_PER_NODE; ring++) {
    for(uint16_t pixel = 0; pixel < extended_led_count; pixel++) {
      uint16_t idx = (pixel + ring*ring_offset + MID_INTRA_RING_MOTION * intra_speed * mid_count / THROTTLE) % extended_led_count;
      if(idx >= LEDS_PER_RING) { continue; }

      uint8_t color_index;
      if(color_mode == COLOR_BY_LOCATION)     { color_index = idx / dim_period; }
      else if(color_mode == COLOR_BY_PATTERN) { color_index = ((pixel + dim_period/2) % extended_led_count) / dim_period; }
      else                                    { color_index = ((dim_period + pixel - color_thickness/2) % extended_led_count) / dim_period; }

      color_index %= MID_NUM_COLORS;

      uint8_t pattern_idx = pixel % dim_period;
      //if(idx == 0) Serial.println(pattern_idx);
      if(pattern_idx < color_thickness) {
        mid_layer[ring][idx] = get_mid_color(color_index);
      }
      else if(pattern_idx < color_thickness + MAX_DIMMING) {
        uint8_t dim_amount = 1 + (pattern_idx - color_thickness);
        mid_layer[ring][idx] = get_mid_color(color_index, dim_amount);
      }
      else if(pattern_idx < color_thickness + MAX_DIMMING + black_thickness) {
        mid_layer[ring][idx] = BLACK;
      }
      else {
        uint8_t dim_amount = MAX_DIMMING - (pattern_idx - color_thickness - black_thickness - MAX_DIMMING);
        mid_layer[ring][idx] = get_mid_color(color_index, dim_amount);
      }
    }
  }
}

