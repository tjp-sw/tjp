// Clear layer functions; use these to erase any one layer
inline void clear_mid_layer() {
  memset(mid_layer, 0, NUM_RINGS * LEDS_PER_RING);
}

inline void clear_sparkle_layer() {
  memset(sparkle_layer, 0, NUM_RINGS * LEDS_PER_RING);
}


// Overlays layers and writes to leds[]
inline void write_pixel_data() {
  // Apply changes to base layer
  if((BASE_TRANSITION == TRANSITION_BY_ALPHA) && (transition_progress_base != 0)) { leds_all %= (255 - transition_progress_base); }

  // blacken_node beat effect
  if (blacken_node && (node_number == blacken_node_number)) {
    leds_all = CRGB::Black;
    return;
  }

  uint8_t ring_offset = RINGS_PER_NODE * node_number;

  for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++)
  {
    // blacken_ring beat effect; 2 rings at a time
    if(blacken_ring && (ring_offset + ring == blacken_ring_number || ring_offset + ring == blacken_ring_number + 1)) {
      for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        set_led(ring, pixel, CRGB::Black);
      }
      
      continue;
    }
    
    bool backward_strip = ring < RINGS_PER_NODE/2;
    uint8_t strip = ring % 2 + (backward_strip ? 0 : 2);
    
    uint16_t pixel_offset = LEDS_PER_STRIP * strip;
    if(backward_strip) {
      pixel_offset += LEDS_PER_STRIP - 1 - PHYSICAL_LEDS_PER_RING*(ring/2);
    }
    else {
      pixel_offset += 1 + PHYSICAL_LEDS_PER_RING*((ring - RINGS_PER_NODE/2)/2);
    }

    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      // Blend in mid layer
      uint8_t color_index = mid_layer[ring_offset+ring][pixel];
      if(color_index != 0) {
        CRGB mid_pixel_color = ColorFromPalette(mid_palette, color_index);
        uint8_t blending = get_mid_alpha(ring, pixel);
        if(MID_TRANSITION == TRANSITION_BY_ALPHA) { if(255 - transition_progress_mid < blending) { blending = 255 - transition_progress_mid; } }
        nblend(leds[pixel_offset], mid_pixel_color, blending);
      }

      // Blend in sparkle layer
      color_index = sparkle_layer[ring_offset+ring][pixel];
      if(color_index != 0) {
        CRGB sparkle_pixel_color = ColorFromPalette(sparkle_palette, color_index);
        uint8_t blending = get_sparkle_alpha(ring, pixel);
        if(SPARKLE_TRANSITION == TRANSITION_BY_ALPHA) { if(255 - transition_progress_sparkle < blending) { blending = 255 - transition_progress_sparkle; } }
        nblend(leds[pixel_offset], sparkle_pixel_color, blending);
      }

      if(backward_strip) pixel_offset--;
      else pixel_offset++;
    }
  }
}


// Alpha functions: return an amount to blend each layer based on ring/pixel
inline uint8_t get_sparkle_alpha(uint8_t ring, uint16_t pixel) {
  switch(SPARKLE_ALPHA) {
    case ALPHA_BY_HEIGHT:
      return get_sparkle_alpha_from_height(pixel);

    case ALPHA_BY_BRIGHTNESS:
      return get_sparkle_alpha_from_brightness(ring, pixel);
    
    default:
      return 255;
  }
}

inline uint8_t get_mid_alpha(uint8_t ring, uint16_t pixel) {
  switch(MID_ALPHA) {
    case ALPHA_BY_HEIGHT:
      return get_mid_alpha_from_height(pixel);

    case ALPHA_BY_BRIGHTNESS:
      return get_mid_alpha_from_brightness(ring, pixel);

    case ALPHA_BY_GRADIENT:
      return get_mid_alpha_from_gradient(ring, pixel);

    default:
      return 255;
  }
}


// Height; Top of ring is opaque, bottom is fully transparent
inline uint8_t get_sparkle_alpha_from_height(uint16_t pixel) {
  if(pixel >= HALF_RING) { pixel = LEDS_PER_RING - 1 - pixel; }
  return pixel * 255 / HALF_RING;
}

inline uint8_t get_sparkle_alpha_from_brightness(uint8_t ring, uint16_t pixel) {
  // Only sparkle layer's brightness is considered
  CRGB color = sparkle_palette.entries[sparkle_layer[ring][pixel]];
  uint8_t luma = color.getLuma();
  if(luma >= 64) { return 255; }
  else { return 4 * luma; }
}


inline uint8_t get_mid_alpha_from_height(uint16_t pixel) {
  if(pixel >= HALF_RING) { pixel = LEDS_PER_RING - 1 - pixel; }
  return (HALF_RING - 1 - pixel) * 255 / HALF_RING;
}

inline uint8_t get_mid_alpha_from_brightness(uint8_t ring, uint16_t pixel) {
  // Ratio of mid to base layer's brightness
  CRGB color = mid_palette.entries[mid_layer[ring][pixel]];
  uint8_t col_luma = color.getLuma();
  uint8_t base_luma = get_led(ring, pixel).getLuma();
  return 255 * col_luma / (col_luma + base_luma);
}

inline uint8_t get_mid_alpha_from_gradient(uint8_t ring, uint16_t pixel) {
  const uint8_t ring_offset = 4;
  const uint8_t intra_motion = 0;
  const uint8_t intra_speed = 16;
  
  const uint16_t period = 64;
  
  uint16_t idx = (pixel + ring_offset*ring + intra_motion * intra_speed * mid_count / THROTTLE) % period;

  if(idx < period/2) { return idx * 255 / (period/2); }
  else { return (period - idx) * 255 / (period/2); }
}


// Functions to transition between animations on each layer. Returns true as long as the transition is ongoing. 
inline bool transition_out_base() {
  if(transition_progress_base >= 255 - BASE_TRANSITION_SPEED || BASE_ANIMATION == NONE) {
    transition_progress_base = 255;
    return false;
  }
  else {
    transition_progress_base += BASE_TRANSITION_SPEED;
    return true;
  }
}

inline bool transition_in_base() {
  if(transition_progress_base <= BASE_TRANSITION_SPEED || BASE_ANIMATION == NONE) {
    transition_progress_base = 0;
    return false;
  }
  else {
    transition_progress_base -= BASE_TRANSITION_SPEED;
    return true;
  }
}

inline bool transition_out_mid() {
  if(transition_progress_mid >= 255 - MID_TRANSITION_SPEED || MID_ANIMATION == NONE) {
    transition_progress_mid = 255;
    return false;
  }
  else {
    transition_progress_mid += MID_TRANSITION_SPEED;
    return true;
  }
}

inline bool transition_in_mid() {
  if(transition_progress_mid <= MID_TRANSITION_SPEED || MID_ANIMATION == NONE) {
    transition_progress_mid = 0;
    return false;
  }
  else {
    transition_progress_mid -= MID_TRANSITION_SPEED;
    return true;
  }
}

inline bool transition_out_sparkle() {
  if(transition_progress_sparkle >= 255 - SPARKLE_TRANSITION_SPEED || SPARKLE_ANIMATION == NONE) {
    transition_progress_sparkle = 255;
    return false;
  }
  else {
    transition_progress_sparkle += SPARKLE_TRANSITION_SPEED;
    return true;
  }
}

inline bool transition_in_sparkle() {
  if(transition_progress_sparkle <= SPARKLE_TRANSITION_SPEED || EDM_ANIMATION == NONE) {
    transition_progress_sparkle = 0;
    return false;
  }
  else {
    transition_progress_sparkle -= SPARKLE_TRANSITION_SPEED;
    return true;
  }
}

inline bool transition_out_edm() {
  if(transition_progress_edm >= 255 - EDM_TRANSITION_SPEED || EDM_ANIMATION == NONE) {
    transition_progress_edm = 255;
    return false;
  }
  else {
    transition_progress_edm += EDM_TRANSITION_SPEED;
    return true;
  }
}

inline bool transition_in_edm() {
  if(transition_progress_edm <= EDM_TRANSITION_SPEED || EDM_ANIMATION == NONE) {
    transition_progress_edm = 0;
    return false;
  }
  else {
    transition_progress_edm -= EDM_TRANSITION_SPEED;
    return true;
  }
}

