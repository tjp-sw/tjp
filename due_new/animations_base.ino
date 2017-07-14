// This file is for completed base animations

//-------------------------------- SCROLLING DIM ---------------------------------
// Draws one base color on even rings and the other color on odd rings
// Scrolls bands of darkness over constant background color, offset on each ring
// To do: if this is expanded like mid_scrolling_dim(), be sure to include additional period (dimming period vs full period with multiple colors)
// BASE_COLOR_THICKNESS(1:6), BASE_BLACK_THICKNESS(0:3), BASE_INTRA_RING_MOTION(-1:1), BASE_RING_OFFSET(-period/2:period/2), BASE_INTRA_RING_SPEED(4:32)
void base_scrolling_dim() {
  uint8_t color_thickness = scale_param(BASE_COLOR_THICKNESS, 1, 5);
  uint8_t black_thickness = scale_param(BASE_BLACK_THICKNESS, 0, 3);
  uint8_t intra_speed = scale_param(BASE_INTRA_RING_SPEED, 4, 32);
  uint8_t period = color_thickness + black_thickness + 2*MAX_DIMMING;
  int8_t ring_offset = scale_param(BASE_RING_OFFSET, -1 * period/2, period/2);
  uint16_t extended_led_count = ((LEDS_PER_RING-1)/period+1)*period;
  int16_t negative_remainder = LEDS_PER_RING - extended_led_count;

  for(uint8_t ring = node_number * RINGS_PER_NODE; ring < (node_number + 1) * RINGS_PER_NODE; ring++) {
    uint8_t color_index = ring % 2;
    
    for(uint16_t pixel = 0; pixel < extended_led_count; pixel++) {
      uint16_t idx = (pixel + ring*ring_offset + BASE_INTRA_RING_MOTION * intra_speed * base_count / THROTTLE) % extended_led_count;
      if(idx >= LEDS_PER_RING) { continue; }

      uint8_t pattern_idx = pixel % period;
      if(pattern_idx < color_thickness) {
        base_layer[ring][idx] = get_base_color(color_index);
      }
      else if(pattern_idx < color_thickness + MAX_DIMMING) {
        uint8_t dim_amount = 1 + (pattern_idx - color_thickness);
        base_layer[ring][idx] = get_base_color(color_index, dim_amount);
      }
      else if(pattern_idx < color_thickness + MAX_DIMMING + black_thickness) {
        base_layer[ring][idx] = BLACK;
      }
      else {
        uint8_t dim_amount = MAX_DIMMING - (pattern_idx - color_thickness - black_thickness - MAX_DIMMING);
        base_layer[ring][idx] = get_base_color(color_index, dim_amount);
      }
    }
  }
}


//-------------------------- SCROLLING 2-COLOR GRADIENT --------------------------
// Draws a gradient moving from one base color to the other and back again
// BASE_COLOR_THICKNESS(8-255), BASE_INTRA_RING_MOTION(-1:1), BASE_RING_OFFSET(-period/2:period/2), BASE_INTRA_RING_SPEED(4:16 if short, *=2 @ 40/70/120)
void base_scrolling_2color_gradient() {
  uint8_t color_thickness = scale_param(BASE_COLOR_THICKNESS, 8, 255);
  uint8_t intra_speed = scale_param(BASE_INTRA_RING_SPEED, 4, 16);
  if(color_thickness >= 120)     { intra_speed <<= 3; }
  else if(color_thickness >= 70) { intra_speed <<= 2; }
  else if(color_thickness >= 40) { intra_speed <<= 1; }
  uint16_t period = 2*color_thickness;
  int8_t ring_offset = scale_param(BASE_RING_OFFSET, -1 * period/2, period/2);
  uint16_t extended_led_count = ((LEDS_PER_RING-1)/period+1)*period;

  for(uint8_t ring = node_number * RINGS_PER_NODE; ring < (node_number + 1) * RINGS_PER_NODE; ring++) {
    for(uint16_t pixel = 0; pixel < extended_led_count; pixel++) {
      uint16_t idx = (pixel + ring*ring_offset + BASE_INTRA_RING_MOTION * intra_speed * base_count / THROTTLE) % extended_led_count;
      if(idx >= LEDS_PER_RING) { continue; }

      uint16_t pattern_idx = pixel % period;
      if(pattern_idx < period/2) {
        // Gradient from base_color0 -> base_color1
        base_layer[ring][idx] = get_base_color(0, 1, pattern_idx * BASE_GRADIENT_SIZE / color_thickness);
      }
      else {
        // Gradient from base_color1 -> base_color0
        pattern_idx -= period/2;
        base_layer[ring][idx] = get_base_color(1, 0, pattern_idx * BASE_GRADIENT_SIZE / color_thickness);
      }
    }
  }
}

