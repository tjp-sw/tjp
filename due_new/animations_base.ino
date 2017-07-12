// This file is for completed base animations

//-------------------------------- SCROLLING DIM ---------------------------------
// Draws one base color on even rings and the other color on odd rings
// Scrolls bands of darkness over constant background color, offset on each ring
// BASE_COLOR_THICKNESS(1-255), BASE_BLACK_THICKNESS(0-6), BASE_INTRA_RING_MOTION(-1, 0, 1), BASE_RING_OFFSET(-128-127), BASE_INTRA_RING_SPEED(0-255)
void base_scrolling_dim() {
  const uint8_t dim_length = 6;
  const uint8_t max_dim_length = 6;

  uint8_t period = BASE_COLOR_THICKNESS + BASE_BLACK_THICKNESS + 2*dim_length;
  uint16_t extended_led_count = ((LEDS_PER_RING-1)/period+1)*period;

  for(uint8_t ring = node_number * RINGS_PER_NODE; ring < (node_number + 1) * RINGS_PER_NODE; ring++) {
    uint8_t color_index = ring % 2;
    
    for(uint16_t pixel = 0; pixel < extended_led_count; pixel++) {
      uint16_t idx;
      if(BASE_INTRA_RING_MOTION != SPLIT) {
        idx = (pixel + ring*BASE_RING_OFFSET + BASE_INTRA_RING_MOTION * BASE_INTRA_RING_SPEED * base_count / THROTTLE) % extended_led_count;
      }

      if(idx >= LEDS_PER_RING) { continue; }

      uint8_t pattern_idx = pixel % period;
      if(pattern_idx < BASE_COLOR_THICKNESS) {
        base_layer[ring][idx] = get_base_color(color_index);
      }
      else if(pattern_idx < BASE_COLOR_THICKNESS + dim_length) {
        uint8_t dim_amount = 1 + (pattern_idx - BASE_COLOR_THICKNESS) * max_dim_length / dim_length;
        /* Calculating how far into dim cycle and what dim_amount will be:
        // dim_length == 1
        // 0: 1 + 0 = 1
        // dim_length == 2
        // 0: 1 + 0 = 1
        // 1: 1 + 2 = 4
        // dim_length == 3
        // 0: 1 + 0 = 1
        // 1: 1 + 2 = 3
        // 2: 1 + 4 = 5
        // dim_length == 4
        // 0: 1 + 0 = 1
        // 1: 1 + 1 = 2
        // 2: 1 + 3 = 4
        // 3: 1 + 4 = 5
        // dim_length == 5
        // 0: 1 + 0 = 1
        // 1: 1 + 1 = 2
        // 2: 1 + 2 = 3
        // 3: 1 + 3 = 4
        // 3: 1 + 4 = 5
        */
        base_layer[ring][idx] = get_base_color(color_index, dim_amount);
      }
      else if(pattern_idx < BASE_COLOR_THICKNESS + dim_length + BASE_BLACK_THICKNESS) {
        base_layer[ring][idx] = BLACK;
      }
      else {
        uint8_t dim_amount = max_dim_length - (pattern_idx - BASE_COLOR_THICKNESS - BASE_BLACK_THICKNESS - dim_length) * max_dim_length / dim_length;
        base_layer[ring][idx] = get_base_color(color_index, dim_amount);
      }
    }
  }
}


//-------------------------- SCROLLING 2-COLOR GRADIENT --------------------------
// Draws a gradient moving from one base color to the other and back again
// BASE_COLOR_THICKNESS(1-255), BASE_INTRA_RING_MOTION(CCW, NONE, CW, SPLIT), BASE_RING_OFFSET(-128-127), BASE_INTRA_RING_SPEED(0-255)
void base_scrolling_2color_gradient() {
  uint8_t period = 2*BASE_COLOR_THICKNESS;
  uint16_t extended_led_count = ((LEDS_PER_RING-1)/period+1)*period;

  for(uint8_t ring = node_number * RINGS_PER_NODE; ring < (node_number + 1) * RINGS_PER_NODE; ring++) {
    for(uint16_t pixel = 0; pixel < extended_led_count; pixel++) {
      uint16_t idx;
      if(BASE_INTRA_RING_MOTION != SPLIT) {
        idx = (pixel + ring*BASE_RING_OFFSET + BASE_INTRA_RING_MOTION * BASE_INTRA_RING_SPEED * base_count / THROTTLE) % extended_led_count;
      }
      if(idx >= LEDS_PER_RING) { continue; }

      uint8_t pattern_idx = pixel % period;
      if(pattern_idx < period/2) {
        // Gradient from base_color0 -> base_color1
        base_layer[ring][idx] = get_base_color(0, 1, pattern_idx * BASE_MAX_GRADIENT_LENGTH / BASE_COLOR_THICKNESS);
      }
      else {
        // Gradient from base_color1 -> base_color0
        pattern_idx -= period/2;
        base_layer[ring][idx] = get_base_color(1, 0, pattern_idx * BASE_MAX_GRADIENT_LENGTH / BASE_COLOR_THICKNESS);
      }
    }
  }
}

