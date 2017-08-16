//-------------------------------- SCROLLING DIM ---------------------------------
// Draws one base color on even rings and the other color on odd rings
// Scrolls bands of darkness over constant background color, offset on each ring
// To do: if this is expanded like mid_scrolling_dim(), be sure to include additional period (dimming period vs full period with multiple colors)
// BASE_COLOR_THICKNESS(1:8), BASE_BLACK_THICKNESS(0:8), BASE_INTRA_RING_MOTION(-1:1), BASE_RING_OFFSET(-period/2:period/2), BASE_INTRA_RING_SPEED(8:32)
// missing: INTER_RING_OFFSET, INTER_RING_MOTION
inline void base_scrolling_dim(uint8_t min_ring, uint8_t max_ring) {
  uint8_t color_thickness = scale_param(BASE_COLOR_THICKNESS, 8, 16);
  uint8_t black_thickness = scale_param(BASE_BLACK_THICKNESS, 0, 0);
  uint8_t intra_speed = 1 << scale_param(BASE_INTRA_RING_SPEED, 3, 5);
  uint8_t period = color_thickness + black_thickness;
  int8_t ring_offset = scale_param(BASE_RING_OFFSET, -1 * period/2, period/2);
  uint16_t extended_led_count = ((LEDS_PER_RING-1)/period+1)*period;
  int8_t alternating_multiplier = BASE_INTRA_RING_MOTION;

  uint8_t num_dim_levels = color_thickness/2 + (color_thickness % 2);
  CRGB shades[2][num_dim_levels];
  for(uint8_t i = 0; i < 2; i++) {
    CRGB step_size = CRGB(current_palette[i].r / num_dim_levels, current_palette[i].g / num_dim_levels, current_palette[i].b / num_dim_levels);
    CRGB step_size_large = CRGB(step_size.r + 1, step_size.g + 1, step_size.b + 1);
    CRGB step_size_small = step_size;//CRGB(qsub8(step_size.r, 1), qsub8(step_size.g, 1), qsub8(step_size.b, 1));

    shades[i][num_dim_levels-1] = current_palette[i];
    for(int8_t dim_level = num_dim_levels - 2; dim_level >= 0; dim_level--) {
      if(dim_level >= num_dim_levels/2) {
        shades[i][dim_level] = CRGB(1 + qsub8(shades[i][dim_level+1].r, 1 + step_size_large.r), qsub8(1 + shades[i][dim_level+1].g, 1 + step_size_large.g), qsub8(1 + shades[i][dim_level+1].b, 1 + step_size_large.b));
      }
      else {
        shades[i][dim_level] = CRGB(1 + qsub8(shades[i][dim_level+1].r, 1 + step_size_small.r), qsub8(1 + shades[i][dim_level+1].g, 1 + step_size_small.g), qsub8(1 + shades[i][dim_level+1].b, 1 + step_size_small.b));
      }
    }
  }

  if(node_number*RINGS_PER_NODE > min_ring) { min_ring = node_number*RINGS_PER_NODE; }
  if((node_number+1)*RINGS_PER_NODE < max_ring) { max_ring = (node_number+1)*RINGS_PER_NODE; }
  for(uint8_t physical_ring = min_ring; physical_ring < max_ring; physical_ring++) {
    uint8_t ring = physical_ring % RINGS_PER_NODE;
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

    if(BASE_INTRA_RING_MOTION == ALTERNATE) { alternating_multiplier = ring % 2 == 0 ? -1 : 1; }
    int16_t motion_offset = ring*ring_offset + alternating_multiplier * intra_speed * base_count / THROTTLE;
    while(motion_offset < 0) { motion_offset += extended_led_count; }
    
    for(uint16_t pixel = 0; pixel < extended_led_count; pixel++) {
      uint16_t idx = (pixel + motion_offset) % extended_led_count;
      if(idx >= LEDS_PER_RING) { 
        continue;
      }

      uint16_t final_pixel_idx = pixel_offset;
      if(backward_strip) final_pixel_idx -= idx;
      else final_pixel_idx += idx;
        
      uint8_t pattern_idx = pixel % period;
      if(pattern_idx < black_thickness) {
        leds[final_pixel_idx] = CRGB::Black;
      }
      else if(pattern_idx < black_thickness + color_thickness/2) {
        uint8_t dim_amount = pattern_idx - black_thickness;
        leds[final_pixel_idx] = shades[color_index][dim_amount];
      }
      else {
        uint8_t dim_amount = period - pattern_idx - 1;
        leds[final_pixel_idx] = shades[color_index][dim_amount];
      }
    }
  }
}


//-------------------------- SCROLLING 2-COLOR GRADIENT --------------------------
// Draws a gradient moving from one base color to the other and back again
// BASE_COLOR_THICKNESS(8-255), BASE_INTRA_RING_MOTION(-1:1), BASE_RING_OFFSET(-period/2:period/2), BASE_INTRA_RING_SPEED(4:16 if short, *=2 @ 40/70/120)
// missing: INTER_RING_OFFSET, INTER_RING_MOTION, BASE_BLACK_THICKNESS
inline void base_scrolling_2color_gradient(uint8_t min_ring, uint8_t max_ring) {
  uint8_t color_thickness = scale_param(BASE_COLOR_THICKNESS, 8, 64);
  uint8_t intra_speed = 1 << scale_param(BASE_INTRA_RING_SPEED, 2, 4);
  if(color_thickness >= 120)     { intra_speed <<= 3; }
  else if(color_thickness >= 70) { intra_speed <<= 2; }
  else if(color_thickness >= 40) { intra_speed <<= 1; }
  uint16_t period = 2*color_thickness;
  int16_t ring_offset = scale_param_16(BASE_RING_OFFSET, -1 * period/2, period/2);
  uint16_t extended_led_count = ((LEDS_PER_RING-1)/period+1)*period;
  int8_t alternating_multiplier = BASE_INTRA_RING_MOTION;

  CRGB values[period];
  fill_gradient_RGB(values, 0, current_palette[0], color_thickness-1, current_palette[1]);
  fill_gradient_RGB(values, color_thickness, current_palette[1], period-1, current_palette[0]);

  if(node_number*RINGS_PER_NODE > min_ring) { min_ring = node_number*RINGS_PER_NODE; }
  if((node_number+1)*RINGS_PER_NODE < max_ring) { max_ring = (node_number+1)*RINGS_PER_NODE; }
  for(uint8_t physical_ring = min_ring; physical_ring < max_ring; physical_ring++) {
    uint8_t ring = physical_ring % RINGS_PER_NODE;
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

    if(BASE_INTRA_RING_MOTION == ALTERNATE) { alternating_multiplier = ring % 2 == 0 ? -1 : 1; }
    int16_t motion_offset = ring*ring_offset + alternating_multiplier * intra_speed * base_count / THROTTLE;
    while(motion_offset < 0) { motion_offset += extended_led_count; }
    
    for(uint16_t pixel = 0; pixel < extended_led_count; pixel++) {
      int16_t idx = (pixel + motion_offset) % extended_led_count;
      if(idx >= LEDS_PER_RING) { continue; }
      
      uint16_t final_pixel_idx = pixel_offset;
      if(backward_strip) final_pixel_idx -= idx;
      else final_pixel_idx += idx;
      uint16_t pattern_idx = pixel % period;
      leds[final_pixel_idx] = values[pattern_idx];
    }
  }
}


//-------------------------------- SCROLLING HALF DIM ---------------------------------
// Draws one base color on even rings and the other color on odd rings
// Scrolls bands of darkness over constant background color, offset on each ring
// To do: if this is expanded like mid_scrolling_dim(), be sure to include additional period (dimming period vs full period with multiple colors)
// BASE_COLOR_THICKNESS(3:6), BASE_BLACK_THICKNESS(1, 4), BASE_INTRA_RING_MOTION(-1:1), BASE_RING_OFFSET(-period/2:period/2), BASE_INTRA_RING_SPEED(8:32)
// missing: INTER_RING_MOTION, INTER_RING_OFFSET
inline void base_scrolling_half_dim(uint8_t min_ring, uint8_t max_ring) {
  uint8_t color_thickness = scale_param(BASE_COLOR_THICKNESS, 4, 10);
  uint8_t black_thickness = scale_param(BASE_BLACK_THICKNESS, 0, 1);
  uint8_t intra_speed = 1 << scale_param(BASE_INTRA_RING_SPEED, 3, 4);
  uint8_t period = color_thickness + black_thickness;
  int8_t ring_offset = scale_param(BASE_RING_OFFSET, -1 * period/2, period/2);
  uint16_t extended_led_count = ((LEDS_PER_RING-1)/period+1)*period;
  int8_t alternating_multiplier = BASE_INTRA_RING_MOTION;

  CRGB shades[2][color_thickness+1];
  fill_gradient_RGB(shades[0], 0, current_palette[0], color_thickness, CRGB::Black);
  fill_gradient_RGB(shades[1], 0, current_palette[1], color_thickness, CRGB::Black);

  if(node_number*RINGS_PER_NODE > min_ring) { min_ring = node_number*RINGS_PER_NODE; }
  if((node_number+1)*RINGS_PER_NODE < max_ring) { max_ring = (node_number+1)*RINGS_PER_NODE; }
  for(uint8_t physical_ring = min_ring; physical_ring < max_ring; physical_ring++) {
    uint8_t ring = physical_ring % RINGS_PER_NODE;
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

    if(BASE_INTRA_RING_MOTION == ALTERNATE) { alternating_multiplier = ring % 2 == 0 ? -1 : 1; }
    int16_t motion_offset = ring*ring_offset + alternating_multiplier * intra_speed * base_count / THROTTLE;
    while(motion_offset < 0) { motion_offset += extended_led_count; }
    
    for(uint16_t pixel = 0; pixel < extended_led_count; pixel++) {
      uint16_t idx = (pixel + motion_offset) % extended_led_count;
      if(idx >= LEDS_PER_RING) { 
        continue;
      }

      uint16_t final_pixel_idx = pixel_offset;
      if(backward_strip) final_pixel_idx -= idx;
      else final_pixel_idx += idx;
        
      uint8_t pattern_idx = pixel % period;
      if(pattern_idx < black_thickness) {
        leds[final_pixel_idx] = CRGB::Black;
      }
      else {
        uint8_t dim_amount = pattern_idx - black_thickness;
        if(BASE_INTRA_RING_MOTION == DOWN || (BASE_INTRA_RING_MOTION == ALTERNATE && ring % 2 == 1)) { dim_amount = color_thickness - dim_amount; }
        leds[final_pixel_idx] = shades[color_index][dim_amount];
      }
    }
  }
}

//------------------------------------------- UNDER DEVELOPMENT --------------------------------------------
//--------------------- Only move above this line when code has been thoroughly tested ---------------------
//--------------------- Only include in allowable animations when moved above this line --------------------
//----------------------------------------------------------------------------------------------------------


//--------------------------------- HORIZONTAL GRADIENT ------------------------------
inline void base_horizontal_gradient(uint8_t min_ring, uint8_t max_ring) {
  const uint8_t max_inter_speed = 4;
  
  uint8_t color_thickness = 12*scale_param(BASE_COLOR_THICKNESS, 1, 3);
  uint8_t intra_speed = 1 << scale_param(BASE_INTRA_RING_SPEED, 3, 4);
  uint8_t inter_speed = scale_param(BASE_INTRA_RING_SPEED, 1, max_inter_speed);
  int8_t alternating_multiplier = BASE_INTRA_RING_MOTION;
  
  
  CRGB shades[2*color_thickness];
  fill_gradient_RGB(shades, 0, current_palette[0], color_thickness-1, current_palette[1]);
  fill_gradient_RGB(shades, color_thickness, current_palette[1], 2*color_thickness-1, current_palette[0]);

  if(node_number*RINGS_PER_NODE > min_ring) { min_ring = node_number*RINGS_PER_NODE; }
  if((node_number+1)*RINGS_PER_NODE < max_ring) { max_ring = (node_number+1)*RINGS_PER_NODE; }
  
  for(uint8_t physical_ring = min_ring; physical_ring < max_ring; physical_ring++) {
    uint8_t ring = physical_ring % RINGS_PER_NODE;
    
    if(BASE_INTRA_RING_MOTION == ALTERNATE) { alternating_multiplier = ring % 2 == 0 ? -1 : 1; }
    int16_t ring_idx = physical_ring + alternating_multiplier * inter_speed * base_count / 4;
    while(ring_idx < 0) { ring_idx += NUM_RINGS; }
    ring_idx %= NUM_RINGS;
    
    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      set_led(ring, pixel, shades[ring_idx % color_thickness]);
      //int16_t pixel_motion_offset = pixel*intra_ring_motion*
      //set_led(ring, pixel, shades[physical_ring % color_thickness]);
    }
  }
}

