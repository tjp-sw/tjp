//-------------------------------- SCROLLING DIM ---------------------------------
// Draws one base color on even rings and the other color on odd rings
// Scrolls bands of darkness over constant background color, offset on each ring
// To do: if this is expanded like mid_scrolling_dim(), be sure to include additional period (dimming period vs full period with multiple colors)
// BASE_COLOR_THICKNESS(1:6), BASE_BLACK_THICKNESS(0:3), BASE_INTRA_RING_MOTION(-1:1), BASE_RING_OFFSET(-period/2:period/2), BASE_INTRA_RING_SPEED(8:32)
inline void base_scrolling_dim() {
  uint8_t color_thickness = scale_param(BASE_COLOR_THICKNESS, 1, 5);
  uint8_t black_thickness = scale_param(BASE_BLACK_THICKNESS, 0, 3);
  uint8_t intra_speed = 1 << scale_param(BASE_INTRA_RING_SPEED, 3, 5);
  uint8_t period = color_thickness + black_thickness + 2*MAX_DIMMING;
  int8_t ring_offset = scale_param(BASE_RING_OFFSET, -1 * period/2, period/2);
  uint16_t extended_led_count = ((LEDS_PER_RING-1)/period+1)*period;

  CRGB shades[2][MAX_DIMMING];
  for(uint8_t i = 0; i < 2; i++) {
    shades[i][0] = CRGB(current_palette[i].r*2/3, current_palette[i].g*2/3, current_palette[i].b*2/3);
    shades[i][1] = CRGB(current_palette[i].r / 2, current_palette[i].g / 2, current_palette[i].b / 2);
    shades[i][2] = CRGB(current_palette[i].r / 3, current_palette[i].g / 3, current_palette[i].b / 3);
    shades[i][3] = CRGB(current_palette[i].r / 4, current_palette[i].g / 4, current_palette[i].b / 4);
    shades[i][4] = CRGB(current_palette[i].r / 5, current_palette[i].g / 5, current_palette[i].b / 5);
    shades[i][5] = CRGB(current_palette[i].r / 6, current_palette[i].g / 6, current_palette[i].b / 6);
  }

  for(uint8_t ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
    uint8_t color_index = ring % 2;
    #if STRIPS_PER_NODE == 4
      bool backward_strip = ring < RINGS_PER_NODE/2;
      uint8_t strip = ring % 2 + (backward_strip ? 0 : 2);
      
      uint16_t pixel_offset = LEDS_PER_STRIP * strip;
      if(backward_strip) {
        pixel_offset += LEDS_PER_STRIP - 1 - PHYSICAL_LEDS_PER_RING*(ring/2);
      }
      else {
        pixel_offset += 1 + PHYSICAL_LEDS_PER_RING*((ring - RINGS_PER_NODE/2)/2);
      }
    #elif STRIPS_PER_NODE == 6
      bool backward_strip = ring < RINGS_PER_NODE/3;
      uint8_t strip = (ring % 2) + 2*(ring/4);
      uint16_t pixel_offset = LEDS_PER_STRIP * strip;
      if(backward_strip) {
        pixel_offset += LEDS_PER_STRIP - 1 - PHYSICAL_LEDS_PER_RING*(ring/2);
      }
      else {
        pixel_offset += 1 + PHYSICAL_LEDS_PER_RING*((ring % (RINGS_PER_NODE/3))/2);
      }
    #endif
    
    for(uint16_t pixel = 0; pixel < extended_led_count; pixel++) {
      uint16_t idx = (pixel + ring*ring_offset + BASE_INTRA_RING_MOTION * intra_speed * base_count / THROTTLE) % extended_led_count;
      if(idx >= LEDS_PER_RING) { 
        continue;
      }

      uint16_t final_pixel_idx = pixel_offset;
      if(backward_strip) final_pixel_idx -= idx;
      else final_pixel_idx += idx;
        
      uint8_t pattern_idx = pixel % period;
      if(pattern_idx < color_thickness) {
        leds[final_pixel_idx] = current_palette[color_index];
      }
      else if(pattern_idx < color_thickness + MAX_DIMMING) {
        uint8_t dim_amount = pattern_idx - color_thickness;
        leds[final_pixel_idx] = shades[color_index][dim_amount];
      }
      else if(pattern_idx < color_thickness + MAX_DIMMING + black_thickness) {
        leds[final_pixel_idx] = CRGB::Black;
      }
      else {
        uint8_t dim_amount = MAX_DIMMING - 1 - (pattern_idx - color_thickness - black_thickness - MAX_DIMMING);
        leds[final_pixel_idx] = shades[color_index][dim_amount];
      }
    }
  }
}


//-------------------------- SCROLLING 2-COLOR GRADIENT --------------------------
// Draws a gradient moving from one base color to the other and back again
// BASE_COLOR_THICKNESS(8-255), BASE_INTRA_RING_MOTION(-1:1), BASE_RING_OFFSET(-period/2:period/2), BASE_INTRA_RING_SPEED(4:16 if short, *=2 @ 40/70/120)
inline void base_scrolling_2color_gradient() {
  uint8_t color_thickness = scale_param(BASE_COLOR_THICKNESS, 8, 64);
  uint8_t intra_speed = 1 << scale_param(BASE_INTRA_RING_SPEED, 2, 4);
  if(color_thickness >= 120)     { intra_speed <<= 3; }
  else if(color_thickness >= 70) { intra_speed <<= 2; }
  else if(color_thickness >= 40) { intra_speed <<= 1; }
  uint16_t period = 2*color_thickness;
  int16_t ring_offset = scale_param_16(BASE_RING_OFFSET, -1 * period/2, period/2);
  uint16_t extended_led_count = ((LEDS_PER_RING-1)/period+1)*period;

  CRGB values[period];
  fill_gradient_RGB(values, 0, current_palette[0], color_thickness-1, current_palette[1]);
  fill_gradient_RGB(values, color_thickness, current_palette[1], period-1, current_palette[0]);

  for(uint8_t ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
    #if STRIPS_PER_NODE == 4
      bool backward_strip = ring < RINGS_PER_NODE/2;
      uint8_t strip = ring % 2 + (backward_strip ? 0 : 2);
      
      uint16_t pixel_offset = LEDS_PER_STRIP * strip;
      if(backward_strip) {
        pixel_offset += LEDS_PER_STRIP - 1 - PHYSICAL_LEDS_PER_RING*(ring/2);
      }
      else {
        pixel_offset += 1 + PHYSICAL_LEDS_PER_RING*((ring - RINGS_PER_NODE/2)/2);
      }
    #elif STRIPS_PER_NODE == 6
      bool backward_strip = ring < RINGS_PER_NODE/3;
      uint8_t strip = (ring % 2) + 2*(ring/4);
      uint16_t pixel_offset = LEDS_PER_STRIP * strip;
      if(backward_strip) {
        pixel_offset += LEDS_PER_STRIP - 1 - PHYSICAL_LEDS_PER_RING*(ring/2);
      }
      else {
        pixel_offset += 1 + PHYSICAL_LEDS_PER_RING*((ring % (RINGS_PER_NODE/3))/2);
      }
    #endif

    for(uint16_t pixel = 0; pixel < extended_led_count; pixel++) {
      uint16_t idx = (pixel + ring*ring_offset + BASE_INTRA_RING_MOTION * intra_speed * base_count / THROTTLE) % extended_led_count;
      if(idx >= LEDS_PER_RING) { continue; }
      
      uint16_t final_pixel_idx = pixel_offset;
      if(backward_strip) final_pixel_idx -= idx;
      else final_pixel_idx += idx;
      uint16_t pattern_idx = pixel % period;
      leds[final_pixel_idx] = values[pattern_idx];
    }
  }
}

