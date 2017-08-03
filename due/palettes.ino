// mid_layer[][] and sparkle_layer[][] use 8-bits per color.
// Mid layer dimming: 0:11, 11 is the darkest, 0 is the pure color at full brightness. (Note: this is actually 'transparency'. The CRGBs will technically be kept at full brightness, but it acts like dimming with a black background.)
// Mid layer blending: We have 21 values to use for blending, across 3 colors and 3 ways to blend between them. So 21/3 == 7; 0:6 is allowed.
// Sparkle layer dimming: 0:126, since there is no blending to be done.


// Functions to get the index into each layer's palette. These functions give you the values to put into the layer arrays.
inline uint8_t get_mid_color(uint8_t color0, uint8_t color1, uint8_t blending, uint8_t dimming) {
  #ifdef DEBUG
    if(color0 > 2) Serial.println("Error in get_mid_color(), color0 == " + String(color0));
    if(color1 > 2) Serial.println("Error in get_mid_color(), color1 == " + String(color1));
    if(blending >= MID_GRADIENT_SIZE) Serial.println("Error in get_mid_color(), blending == " + String(blending));
    if(dimming >= NUM_MID_DIMMING_LEVELS) Serial.println("Error in get_mid_color(), dimming == " + String(dimming));
  #endif

  return get_mid_color(color0, color1, blending) + dimming;
}
inline uint8_t get_mid_color(uint8_t color0, uint8_t color1, uint8_t blending) {
  #ifdef DEBUG
    if(color0 > 2) Serial.println("Error in get_mid_color(), color0 == " + String(color0));
    if(color1 > 2) Serial.println("Error in get_mid_color(), color1 == " + String(color1));
    if(blending >= MID_GRADIENT_SIZE) Serial.println("Error in get_mid_color(), blending == " + String(blending));
  #endif
  
  if(color0 == 0) {
    if(color1 == 1) {
      return 4 + NUM_MID_DIMMING_LEVELS*blending;
    }
    else {
      return 4 + NUM_MID_DIMMING_LEVELS*(MID_GRADIENT_SIZE + blending);
    }
  }
  else if(color0 == 1) {
    if(color1 == 0) {
      return 4 + NUM_MID_DIMMING_LEVELS*(MID_GRADIENT_SIZE - 1 - blending);
    }
    else {
      return 4 + NUM_MID_DIMMING_LEVELS*(2*MID_GRADIENT_SIZE + blending);
    }
  }
  else {
    if(color1 == 0) {
      return 4 + NUM_MID_DIMMING_LEVELS*(2*MID_GRADIENT_SIZE - 1 - blending);
    }
    else {
      return 4 + NUM_MID_DIMMING_LEVELS*(3*MID_GRADIENT_SIZE - 1 - blending);
    }
  }
}
inline uint8_t get_mid_color(uint8_t color0, uint8_t dimming) {
  #ifdef DEBUG
    if(color0 > 2) Serial.println("Error in get_mid_color(), color0 == " + String(color0));
    if(dimming >= NUM_MID_DIMMING_LEVELS) Serial.println("Error in get_mid_color(), dimming == " + String(dimming));
  #endif
  return dimming + get_mid_color(color0);
}
inline uint8_t get_mid_color(uint8_t color0) {
  return color0 == 0 ? 4 : color0 == 1 ? 4 + MID_GRADIENT_SIZE*NUM_MID_DIMMING_LEVELS : 4 + (2*MID_GRADIENT_SIZE-1)*NUM_MID_DIMMING_LEVELS;
}

inline uint8_t get_sparkle_color(uint8_t color, uint8_t dimming) {
  #ifdef DEBUG
    if(color > 1) Serial.println("Error in get_sparkle_color(), color == " + String(color));
    if(dimming >= NUM_SPARKLE_DIMMING_LEVELS) Serial.println("Error in get_sparkle_color(), dimming == " + String(dimming));
  #endif

  return get_sparkle_color(color) + dimming;
}
inline uint8_t get_sparkle_color(uint8_t color) {
  #ifdef DEBUG
    if(color > 1) Serial.println("Error in get_sparkle_color(), color == " + String(color));
  #endif
  
  return 2 + NUM_SPARKLE_DIMMING_LEVELS*color;
}


// Getters for info about current values stored in layer arrays. Use these if you need to decipher values that are already there.
// Accepts either the ring & pixel, or the the value that is from the layer array.
inline uint8_t get_sparkle_raw_color(uint8_t color) {
  return (color - 2) / NUM_SPARKLE_DIMMING_LEVELS;
}
inline uint8_t get_sparkle_raw_color(uint8_t ring, uint16_t pixel) {
  return get_sparkle_raw_color(sparkle_layer[ring][pixel]);
}

inline uint8_t get_sparkle_dim_value(uint8_t color) {
  return (color - 2) % NUM_SPARKLE_DIMMING_LEVELS;
}
inline uint8_t get_sparkle_dim_value(uint8_t ring, uint16_t pixel) {
  return get_sparkle_dim_value(sparkle_layer[ring][pixel]);
}

// This will return the 'left-side' color, ignoring blending; the smaller of the two colors that were blended
inline uint8_t get_mid_raw_color0(uint8_t color) {
  uint8_t section = (color - 4) / (NUM_MID_DIMMING_LEVELS * MID_GRADIENT_SIZE);
  return section < 2 ? 0 : 1;
}
inline uint8_t get_mid_raw_color0(uint8_t ring, uint16_t pixel) {
  return get_mid_raw_color0(mid_layer[ring][pixel]);
}

// This will return the 'right-side' color, ignoring blending; the larger of the two colors that were blended
inline uint8_t get_mid_raw_color1(uint8_t color) {
  uint8_t section = (color - 4) / (NUM_MID_DIMMING_LEVELS * MID_GRADIENT_SIZE);
  return section == 0 ? 1 : 2;
}
inline uint8_t get_mid_raw_color1(uint8_t ring, uint16_t pixel) {
  return get_mid_raw_color1(mid_layer[ring][pixel]);
}

inline uint8_t get_mid_dim_value(uint8_t color) {
  return (color - 4) % NUM_MID_DIMMING_LEVELS;
}
inline uint8_t get_mid_dim_value(uint8_t ring, uint16_t pixel) {
  return get_mid_dim_value(mid_layer[ring][pixel]);
}

// This will return the blending amount, always from the left-side color to the right-side color
inline uint8_t get_mid_blending(uint8_t color) {
  return ((color - 4) / NUM_MID_DIMMING_LEVELS) % MID_GRADIENT_SIZE;
}
inline uint8_t get_mid_blending(uint8_t ring, uint16_t pixel) {
  return get_mid_blending(mid_layer[ring][pixel]);
}

// This will return the dominant color; if you blended 60% of the way from color 0 to color 2, it will return 2.
// If blending exactly half way, returns the left-side color (the smaller index of the two)
inline uint8_t get_mid_raw_color(uint8_t color) {
  uint8_t section = (color - 4) / (NUM_MID_DIMMING_LEVELS * MID_GRADIENT_SIZE);
  bool use_color0 = ((color - 4) / NUM_MID_DIMMING_LEVELS) % MID_GRADIENT_SIZE <= MID_GRADIENT_SIZE/2;

  if(section == 0)      { return use_color0 ? 0 : 1; }
  else if(section == 1) { return use_color0 ? 0 : 2; }
  else                  { return use_color0 ? 1 : 2; }
}

inline uint8_t get_mid_raw_color(uint8_t ring, uint16_t pixel) {
  return get_mid_raw_color(mid_layer[ring][pixel]);
}

#ifdef DEBUG
// Tests for new layer utilities (all passing when this is run at the end of setup())
void test_palette_utils() {
  for(uint8_t i = 0; i < 2; i++) {
    for(uint8_t j = 0; j < NUM_SPARKLE_DIMMING_LEVELS; j++) {
      sparkle_layer[0][0] = get_sparkle_color(i, j);
      if(get_sparkle_raw_color(0,0) != i) { Serial.println("Wrong sparkle color at: " + String(i) + ", " + String(j)); Serial.println(get_sparkle_raw_color(0,0));}
      if(get_sparkle_dim_value(0,0) != j) { Serial.println("Wrong sparkle dimming at: " + String(i) + ", " + String(j)); }
    }
  }

  for(uint8_t i = 0; i < 3; i++) {
    for(uint8_t k = 0; k < 3; k++) {
      if (i == k) { continue; }

      for(uint8_t j = 0; j < NUM_MID_DIMMING_LEVELS; j++) {
        for(uint8_t b = 0; b < MID_GRADIENT_SIZE; b++) {
          mid_layer[0][0] = get_mid_color(i, k, b, j);
          uint8_t first_col = i < k ? i : k;
          uint8_t second_col = i < k ? k : i;
          uint8_t dominant_col = b > MID_GRADIENT_SIZE/2 ? k : i;
          uint8_t expected_blending = i == first_col ? b : MID_GRADIENT_SIZE-1-b;

          if(get_mid_raw_color0(0,0) != first_col) { Serial.println("Wrong mid color0 at: " + String(i) + ", " + String(k) + ", " + String(b) + ", " + String(j)); }
          if(get_mid_raw_color1(0,0) != second_col) { Serial.println("Wrong mid color1 at: " + String(i) + ", " + String(k) + ", " + String(b) + ", " + String(j)); }
          if(get_mid_blending(0,0) != expected_blending) { Serial.println("Wrong mid blending at: " + String(i) + ", " + String(k) + ", " + String(b) + ", " + String(j)); Serial.println(expected_blending); Serial.println(get_mid_blending(0,0)); }
          if(get_mid_dim_value(0,0) != j) { Serial.println("Wrong mid dimming at: " + String(i) + ", " + String(k) + ", " + String(b) + ", " + String(j)); }
          if(b == MID_GRADIENT_SIZE/2) { dominant_col = i < k ? i : k; }
          if(get_mid_raw_color(0,0) != dominant_col) { Serial.println("Wrong mid dom color at: " + String(i) + ", " + String(k) + ", " + String(b) + ", " + String(j)); }

        }
      }
    }
  }
}
#endif

// Converts 2||3 colors to a 16||256 color palette with dimming and gradients
inline void create_mid_palette(CRGBPalette256* new_palette, CRGB color0, CRGB color1, CRGB color2) {
  new_palette->entries[TRANSPARENT] = CRGB::Black;
  new_palette->entries[WHITE] = CRGB::White;
  new_palette->entries[BLACK] = CRGB::Black;
  new_palette->entries[GRAY] = CRGB(128, 128, 128);

  CRGB temp;
  for(uint8_t i = 0; i < 3*MID_GRADIENT_SIZE; i++) {
    uint16_t palette_offset = 4 + NUM_MID_DIMMING_LEVELS*i;
    // 36 unique hues/saturations, each with 7 brightness values
    // 12 color0 -> color1, 12 color0 -> color2, 12 color1 -> color2
    if(i < MID_GRADIENT_SIZE) {
      temp = color0;
      tjp_nblend(temp, color1, 255 * i / (MID_GRADIENT_SIZE-1));
    }
    else if(i < 2*MID_GRADIENT_SIZE) {
      temp = color0;
      tjp_nblend(temp, color2, 255 * (i-MID_GRADIENT_SIZE) / (MID_GRADIENT_SIZE-1));
    }
    else {
      temp = color1;
      tjp_nblend(temp, color2, 255 * (i-2*MID_GRADIENT_SIZE) / (MID_GRADIENT_SIZE-1));
    }

    for(uint8_t j = 0; j < NUM_MID_DIMMING_LEVELS; j++) {
      new_palette->entries[palette_offset + j] = temp;
    }


    /*
    new_palette->entries[palette_offset + 1] = CRGB(temp.r / 2, temp.g / 2, temp.b / 2);
    new_palette->entries[palette_offset + 2] = CRGB(temp.r / 4, temp.g / 4, temp.b / 4);
    new_palette->entries[palette_offset + 3] = CRGB(temp.r / 6, temp.g / 6, temp.b / 6);
    new_palette->entries[palette_offset + 4] = CRGB(temp.r / 9, temp.g / 9, temp.b / 9);
    new_palette->entries[palette_offset + 5] = CRGB(temp.r / 12, temp.g / 12, temp.b / 12);
    new_palette->entries[palette_offset + 6] = CRGB(temp.r / 15, temp.g / 15, temp.b / 15);
    */
  }
}

inline void create_sparkle_palette(CRGBPalette256* new_palette, CRGB color0, CRGB color1) {
  new_palette->entries[TRANSPARENT] = CRGB::Black;
  new_palette->entries[WHITE] = CRGB::White;

  CRGB temp;
  for(uint8_t i = 0; i < 2; i++) {
    uint8_t palette_offset = 2 + NUM_SPARKLE_DIMMING_LEVELS*i;
    temp = i == 0 ? color0 : color1;
    for(uint8_t j = 0; j < NUM_SPARKLE_DIMMING_LEVELS; j++) {
      new_palette->entries[palette_offset + j] = temp;
    }
    /*
    new_palette->entries[palette_offset] = temp;
    new_palette->entries[palette_offset + 1] = CRGB(temp.r / 2, temp.g / 2, temp.b / 2);
    new_palette->entries[palette_offset + 2] = CRGB(temp.r / 4, temp.g / 4, temp.b / 4);
    new_palette->entries[palette_offset + 3] = CRGB(temp.r / 8, temp.g / 8, temp.b / 8);
    new_palette->entries[palette_offset + 4] = CRGB(temp.r / 12, temp.g / 12, temp.b / 12);
    new_palette->entries[palette_offset + 5] = CRGB(temp.r / 16, temp.g / 16, temp.b / 16);
    new_palette->entries[palette_offset + 6] = CRGB(temp.r / 20, temp.g / 20, temp.b / 20);
     */
  }
}

// Loads initial colors until brain sends us something
inline void setup_palettes() {
  create_mid_palette(&mid_palette, initial_palette[2], initial_palette[3], initial_palette[4]);
  create_sparkle_palette(&sparkle_palette, initial_palette[5], initial_palette[6]);

  memcpy(current_palette, initial_palette, 3*NUM_COLORS_PER_PALETTE);
  memcpy(target_palette, initial_palette, 3*NUM_COLORS_PER_PALETTE);
}


// Smoothly transitions from current palette into the new target palette received from brain. Returns true as long as blending is ongoing.
inline bool blend_base_palette(uint8_t max_changes, bool use_fast_blend_function) {
  // Bytes 0-5 of current_palette
  if(use_fast_blend_function) { blend_palette_fast(0, 5, max_changes); }
  else { blend_palette(0, 5, max_changes); }

  return current_palette[0] != target_palette[0] || current_palette[1] != target_palette[1];
}

inline bool blend_mid_palette(uint8_t max_changes, bool use_fast_blend_function) {
  // Bytes 6-14 of current_palette
  if(use_fast_blend_function) { blend_palette_fast(6, 14, max_changes); }
  else { blend_palette(6, 14, max_changes); }

  create_mid_palette(&mid_palette, current_palette[2], current_palette[3], current_palette[4]); // Apply changes to CRGBPalette256
  return current_palette[2] != target_palette[2] || current_palette[3] != target_palette[3] || current_palette[4] != target_palette[4];
}

inline bool blend_sparkle_palette(uint8_t max_changes, bool use_fast_blend_function) {
  // Bytes 15-20 of current_palette
  if(use_fast_blend_function) { blend_palette_fast(15, 20, max_changes); }
  else { blend_palette(15, 20, max_changes); }

  create_sparkle_palette(&sparkle_palette, current_palette[5], current_palette[6]); // Apply changes to CRGBPalette16
  return current_palette[5] != target_palette[5] || current_palette[6] != target_palette[6];
}

inline void blend_palette(uint8_t iMin, uint8_t iMax, uint8_t max_changes) {
  uint8_t* p1;
  uint8_t* p2;
  uint8_t changes = 0;

  p1 = (uint8_t*)current_palette;
  p2 = (uint8_t*)target_palette;

  for(uint8_t i = iMin; i <= iMax; i++) {
    if(p1[i] < p2[i]) {
      changes++;
      p1[i]++;
    }
    else if(p1[i] > p2[i]) {
      changes++;
      p1[i]--;
    }

    if(changes >= max_changes) { break; }
  }
}

inline void blend_palette_fast(uint8_t iMin, uint8_t iMax, uint8_t max_changes) {
  uint8_t* p1;
  uint8_t* p2;
  uint8_t changes = 0;

  p1 = (uint8_t*)current_palette;
  p2 = (uint8_t*)target_palette;

  for(uint8_t i = iMin; i <= iMax; i++) {
    if(p1[i] < p2[i]) {
      changes++;
      p1[i]++;
      if(p1[i] < p2[i]) { p1[i]++; }
    }
    else if(p1[i] > p2[i]) {
      changes++;
      p1[i]--;
      if(p1[i] > p2[i]) { p1[i]--; }
    }

    if(changes >= max_changes) { break; }
  }
}
