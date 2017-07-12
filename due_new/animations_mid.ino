// This file is for completed mid animations

//-------------------------------- SNAKE --------------------------------
// Sends alternating bands of colors rotating around the rings
// Creates repeated snakes with NUM_COLORS colors, with each color repeated COLOR_THICKNESS times, separated by BLACK_THICKNESS black LEDs.
// MID_NUM_COLORS(1-3), MID_COLOR_THICKNESS(1-20), MID_BLACK_THICKNESS(0-60), MID_INTRA_RING_MOTION(-1, 1), MID_RING_OFFSET(0-20), MID_INTRA_RING_SPEED(8-127)
void snake(uint8_t ring_mode) {
  uint8_t period = MID_NUM_COLORS * MID_COLOR_THICKNESS + MID_BLACK_THICKNESS;
  uint16_t extended_led_count = ((LEDS_PER_RING-1)/period+1)*period;

  uint8_t ring_start = node_number*RINGS_PER_NODE + (ring_mode == ODD_RINGS ? 1 : 0);
  uint8_t ring_inc = ring_mode == ALL_RINGS ? 1 : 2;
  for (uint8_t ring = ring_start; ring < (node_number+1)*RINGS_PER_NODE; ring += ring_inc) {
    for (uint16_t pixel = 0; pixel < extended_led_count; pixel++) {
      uint16_t idx;
      if(MID_INTRA_RING_MOTION != SPLIT) {
        idx = (pixel + MID_RING_OFFSET*ring + MID_INTRA_RING_MOTION * MID_INTRA_RING_SPEED * loop_count / THROTTLE) % extended_led_count;
      }
      if(idx >= LEDS_PER_RING) { continue; }

      uint8_t pattern_idx = pixel % period;
      if(pattern_idx < MID_BLACK_THICKNESS) {
        mid_layer[ring][idx] = TRANSPARENT;
      }
      else if(pattern_idx < MID_BLACK_THICKNESS + MID_COLOR_THICKNESS) {
        mid_layer[ring][idx] = get_mid_color(0);
      }
      else if(pattern_idx < MID_BLACK_THICKNESS + 2*MID_COLOR_THICKNESS) {
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
// MID_BLACK_THICKNESS(20-80) == cooling, MID_COLOR_THICKNESS(40-225) == sparking chance, MID_NUM_COLORS(100-255) == minimum spark size
// A cooling wind rolls around the structure: MID_INTER_RING_SPEED == wind speed, MID_RING_OFFSET == wind decay amount in neighboring rings

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
  if(palette_type != FIRE_PALETTE_DISABLED) {
    if(palette_type == FIRE_PALETTE_STANDARD) { set_fire_palette(); }
  }

  uint8_t ring_start = node_number*RINGS_PER_NODE + (ring_mode == ODD_RINGS ? 1 : 0);
  uint8_t ring_inc = ring_mode == ALL_RINGS ? 1 : 2;
  
  // Step 1.  Cool down every 4th cell a little, and add an extra cooling wind that moves around the structure
  uint8_t coldest_ring = (mid_count * MID_INTER_RING_SPEED / THROTTLE) % NUM_RINGS;
  for(uint8_t ring = ring_start; ring < (node_number+1)*RINGS_PER_NODE; ring += ring_inc) {
    uint8_t fire_cooling = MID_BLACK_THICKNESS;
    uint8_t dist = ring > coldest_ring ? ring - coldest_ring : coldest_ring - ring;

    uint8_t wind_cooling = 0;
    if(dist <= MAX_WIND_RANGE) {
      wind_cooling = MAX_WIND_COOLING;
      while(dist-- > 0) {
        wind_cooling = wind_cooling * MID_RING_OFFSET / 127;
      }
    }
    
    
    for(uint16_t pixel = 0; pixel < HALF_RING; pixel+=4) {
      uint8_t extra_cooling = wind_cooling;
      //if((pixel < 2*SPARKING_RANGE) || (pixel > NUM_LEDS-1 - 2*SPARKING_RANGE))
        //extra_cooling = 255;
      //if((pixel < 4*SPARKING_RANGE) || (pixel > NUM_LEDS-1 - 4*SPARKING_RANGE))
        //extra_cooling = wind_cooling * 3;
      mid_layer[ring][pixel] = qsub8(mid_layer[ring][pixel],  random(0, fire_cooling+extra_cooling));
      mid_layer[ring][HALF_RING + pixel] = qsub8(mid_layer[ring][HALF_RING + pixel],  random(0, fire_cooling+extra_cooling));
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
    uint8_t fireSparking = MID_COLOR_THICKNESS;//(i + RINGS_PER_NODE / 2 + loop_count/7) % 230 + 25; // 10% to 100% chance of sparking
    
    if(random(256) < fireSparking) {
      uint8_t pixel = random8(SPARKING_RANGE);
      mid_layer[ring][pixel] = qadd8(mid_layer[ring][pixel], random(MID_NUM_COLORS, 255));
    }

    if(random(256) < fireSparking) {
      uint16_t pixel = LEDS_PER_RING - 1 - random(SPARKING_RANGE);
      mid_layer[ring][pixel] = qadd8(mid_layer[ring][pixel], random(MID_NUM_COLORS, 255));
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
// MID_NUM_COLORS(1-3), MID_COLOR_THICKNESS(1-255), MID_BLACK_THICKNESS(0-6), MID_INTRA_RING_MOTION(-1, 0, 1), MID_RING_OFFSET(-128-127), MID_INTRA_RING_SPEED(0-255)
void mid_scrolling_dim(uint8_t color_mode) {
  uint8_t period = MID_COLOR_THICKNESS + MID_BLACK_THICKNESS + 2*MAX_DIMMING;
  uint16_t extended_led_count = ((LEDS_PER_RING-1)/period+1)*period;

  for(uint8_t ring = node_number * RINGS_PER_NODE; ring < (node_number + 1) * RINGS_PER_NODE; ring++) {
    for(uint16_t pixel = 0; pixel < extended_led_count; pixel++) {
      uint16_t idx;
      if(MID_INTRA_RING_MOTION != SPLIT) {
        idx = (pixel + ring*MID_RING_OFFSET + MID_INTRA_RING_MOTION * MID_INTRA_RING_SPEED * mid_count / THROTTLE) % extended_led_count;
      }

      if(idx >= LEDS_PER_RING) { continue; }
      uint8_t color_index = (color_mode == COLOR_BY_LOCATION ? idx : color_mode == COLOR_BY_PATTERN ? pixel + period/2 : pixel) / period;
      color_index %= MID_NUM_COLORS;

      uint8_t pattern_idx = pixel % period;
      if(pattern_idx < MID_COLOR_THICKNESS) {
        mid_layer[ring][idx] = get_mid_color(color_index);
      }
      else if(pattern_idx < MID_COLOR_THICKNESS + MAX_DIMMING) {
        uint8_t dim_amount = 1 + (pattern_idx - MID_COLOR_THICKNESS);
        mid_layer[ring][idx] = get_mid_color(color_index, dim_amount);
      }
      else if(pattern_idx < MID_COLOR_THICKNESS + MAX_DIMMING + MID_BLACK_THICKNESS) {
        mid_layer[ring][idx] = BLACK;
      }
      else {
        uint8_t dim_amount = MAX_DIMMING - (pattern_idx - MID_COLOR_THICKNESS - MID_BLACK_THICKNESS - MAX_DIMMING);
        mid_layer[ring][idx] = get_mid_color(color_index, dim_amount);
      }
    }
  }
}

