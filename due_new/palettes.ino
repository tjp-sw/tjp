void setup_palettes() {
  create_base_palette(&base_palette, initial_palette[0], initial_palette[1]);
  create_mid_palette(&mid_palette, initial_palette[2], initial_palette[3], initial_palette[4]);
  create_sparkle_palette(&sparkle_palette, initial_palette[5], initial_palette[6]);

  memcpy(current_palette, initial_palette, 3*NUM_COLORS_PER_PALETTE);
  memcpy(target_palette, initial_palette, 3*NUM_COLORS_PER_PALETTE);
}

// Functions to get the index into each layer's palette
uint8_t get_base_color(uint8_t color0, uint8_t color1, uint8_t blending, uint8_t dimming) {
  #ifdef DEBUG
    if(color0 > 1) Serial.println("Error in get_base_color(), color0 == " + String(color0));
    if(color1 > 1) Serial.println("Error in get_base_color(), color1 == " + String(color1));
    if(blending > 35) Serial.println("Error in get_base_color(), blending == " + String(blending));
    if(dimming > MAX_DIMMING) Serial.println("Error in get_base_color(), dimming == " + String(dimming));
  #endif

  if(dimming > MAX_DIMMING) {
    return BLACK;
  }
  else if(color0 == 0) {
    return 4 + 7*blending + dimming;
  }
  else {
    // Blend from color1 to color0
    return 4 + 7*(35-blending) + dimming;
  }
}
uint8_t get_base_color(uint8_t color0, uint8_t color1, uint8_t blending) {
  return get_base_color(color0, color1, blending, 0);
}
uint8_t get_base_color(uint8_t color0, uint8_t dimming) {
  return get_base_color(color0, color0, 0, dimming);
}
uint8_t get_base_color(uint8_t color0) {
  return get_base_color(color0, color0, 0, 0);
}

uint8_t get_mid_color(uint8_t color0, uint8_t color1, uint8_t blending, uint8_t dimming) {
  #ifdef DEBUG
    if(color0 > 2) Serial.println("Error in get_mid_color(), color0 == " + String(color0));
    if(color1 > 2) Serial.println("Error in get_mid_color(), color1 == " + String(color1));
    if(blending > 11) Serial.println("Error in get_mid_color(), blending == " + String(blending));
    if(dimming > 7) Serial.println("Error in get_mid_color(), dimming == " + String(dimming));
  #endif

  if(dimming >= 7) {
    return BLACK;
  }
  else if(color0 == 0) {
    if(color1 == 1) {
      return 4 + 7*blending + dimming;
    }
    else {
      return 4 + 7*(12 + blending) + dimming;
    }
  }
  else if(color0 == 1) {
    if(color1 == 0) {
      return 4 + 7*(11 - blending) + dimming;
    }
    else {
      return 4 + 7*(24 + blending) + dimming;
    }
  }
  else {
    if(color1 == 0) {
      return 4 + 7*(23 - blending) + dimming;
    }
    else {
      return 4 + 7*(35 - blending) + dimming;
    }
  }
}
uint8_t get_mid_color(uint8_t color0, uint8_t color1, uint8_t blending) {
  return get_mid_color(color0, color1, blending, 0);
}
uint8_t get_mid_color(uint8_t color0, uint8_t dimming) {
  return get_mid_color(color0, color0, 0, dimming);
}
uint8_t get_mid_color(uint8_t color0) {
  return get_mid_color(color0, color0, 0, 0);
}

uint8_t get_sparkle_color(uint8_t color, uint8_t dimming) {
  #ifdef DEBUG
    if(color > 1) Serial.println("Error in get_sparkle_color(), color == " + String(color));
    if(dimming > MAX_DIMMING) Serial.println("Error in get_sparkle_color(), dimming == " + String(dimming));
  #endif

  return 2 + 7*color + dimming;
}
uint8_t get_sparkle_color(uint8_t color) {
  return get_sparkle_color(color, 0);
}

uint8_t get_edm_color(uint8_t color, uint8_t dimming) {
    #ifdef DEBUG
    if(color > 6) Serial.println("Error in get_sparkle_color(), color == " + String(color));
    if(dimming > MAX_DIMMING) Serial.println("Error in get_sparkle_color(), dimming == " + String(dimming));
  #endif

  if(color < 2) { return get_base_color(color, dimming); }
  if(color < 4) { return get_mid_color(color-2, dimming); }
  return get_sparkle_color(color-5, dimming);
}
uint8_t get_edm_color(uint8_t color) {
  return get_edm_color(color, 0);
}


// Converts 2||3 colors to a 16||256 color palette with dimming and gradients
void create_base_palette(CRGBPalette256* new_palette, CRGB color0, CRGB color1) {
  new_palette->entries[TRANSPARENT] = CRGB::Black;
  new_palette->entries[WHITE] = CRGB::White;
  new_palette->entries[BLACK] = CRGB::Black;
  new_palette->entries[GRAY] = CRGB(128, 128, 128);

  CRGB temp;
  for(uint8_t i = 0; i < MAX_BASE_GRADIENT; i++) {
    uint16_t palette_offset = 4 + 7*i;
    // 36 unique hues/saturations, each with 7 brightness values
    temp = color0;
    nblend(temp, color1, 255 * i / 35);

    new_palette->entries[palette_offset] = temp;
    new_palette->entries[palette_offset + 1] = CRGB(temp.r * 2/3, temp.g * 2/3, temp.b * 2/3);
    new_palette->entries[palette_offset + 2] = CRGB(temp.r / 2, temp.g / 2, temp.b / 2);
    new_palette->entries[palette_offset + 3] = CRGB(temp.r / 3, temp.g / 3, temp.b / 3);
    new_palette->entries[palette_offset + 4] = CRGB(temp.r / 4, temp.g / 4, temp.b / 4);
    new_palette->entries[palette_offset + 5] = CRGB(temp.r / 5, temp.g / 5, temp.b / 5);
    new_palette->entries[palette_offset + 6] = CRGB(temp.r / 6, temp.g / 6, temp.b / 6);
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

void create_mid_palette(CRGBPalette256* new_palette, CRGB color0, CRGB color1, CRGB color2) {
  new_palette->entries[TRANSPARENT] = CRGB::Black;
  new_palette->entries[WHITE] = CRGB::White;
  new_palette->entries[BLACK] = CRGB::Black;
  new_palette->entries[GRAY] = CRGB(128, 128, 128);

  CRGB temp;
  for(uint8_t i = 0; i < 36; i++) {
    uint16_t palette_offset = 4 + 7*i;
    // 36 unique hues/saturations, each with 7 brightness values
    // 12 color0 -> color1, 12 color0 -> color2, 12 color1 -> color2
    if(i < 12) {
      temp = color0;
      nblend(temp, color1, 255 * i / 11);
    }
    else if(i < 24) {
      temp = color0;
      nblend(temp, color2, 255 * (i-12) / 11);
    }
    else {
      temp = color1;
      nblend(temp, color2, 255 * (i-24) / 11);
    }

    new_palette->entries[palette_offset] = temp;
    new_palette->entries[palette_offset + 1] = CRGB(temp.r / 2, temp.g / 2, temp.b / 2);
    new_palette->entries[palette_offset + 2] = CRGB(temp.r / 4, temp.g / 4, temp.b / 4);
    new_palette->entries[palette_offset + 3] = CRGB(temp.r / 6, temp.g / 6, temp.b / 6);
    new_palette->entries[palette_offset + 4] = CRGB(temp.r / 9, temp.g / 9, temp.b / 9);
    new_palette->entries[palette_offset + 5] = CRGB(temp.r / 12, temp.g / 12, temp.b / 12);
    new_palette->entries[palette_offset + 6] = CRGB(temp.r / 15, temp.g / 15, temp.b / 15);
  }
}

void create_sparkle_palette(CRGBPalette16* new_palette, CRGB color0, CRGB color1) {
  new_palette->entries[TRANSPARENT] = CRGB::Black;
  new_palette->entries[WHITE] = CRGB::White;

  CRGB temp;
  for(uint8_t i = 0; i < 2; i++) {
    uint8_t palette_offset = 2+7*i;
    temp = i == 0 ? color0 : color1;
    new_palette->entries[palette_offset] = temp;
    new_palette->entries[palette_offset + 1] = CRGB(temp.r / 2, temp.g / 2, temp.b / 2);
    new_palette->entries[palette_offset + 2] = CRGB(temp.r / 4, temp.g / 4, temp.b / 4);
    new_palette->entries[palette_offset + 3] = CRGB(temp.r / 6, temp.g / 6, temp.b / 6);
    new_palette->entries[palette_offset + 4] = CRGB(temp.r / 9, temp.g / 9, temp.b / 9);
    new_palette->entries[palette_offset + 5] = CRGB(temp.r / 12, temp.g / 12, temp.b / 12);
    new_palette->entries[palette_offset + 6] = CRGB(temp.r / 15, temp.g / 15, temp.b / 15);
  }
}


// Smoothly transitions from current palette into the new target palette received from Pi
bool blend_base_palette(uint8_t max_changes) {
  blend_palette(0, 5, max_changes); // First 6 bytes of current_palette
    
  create_base_palette(&base_palette, current_palette[0], current_palette[1]); // Apply changes to CRGBPalette256
  return current_palette[0] != target_palette[0] || current_palette[1] != target_palette[1];
}

bool blend_mid_palette(uint8_t max_changes) {
  blend_palette(6, 14, max_changes); // Bytes 6-14 of current_palette
  
  create_mid_palette(&mid_palette, current_palette[2], current_palette[3], current_palette[4]); // Apply changes to CRGBPalette256
  return current_palette[2] != target_palette[2] || current_palette[3] != target_palette[3] || current_palette[4] != target_palette[4];
}

bool blend_sparkle_palette(uint8_t max_changes) {
  blend_palette(15, 20, max_changes); // Bytes 15-20 of current_palette
  
  create_sparkle_palette(&sparkle_palette, current_palette[5], current_palette[6]); // Apply changes to CRGBPalette16
  return current_palette[5] != target_palette[5] || current_palette[6] != target_palette[6];
}

bool blend_palette(uint8_t iMin, uint8_t iMax, uint8_t max_changes) {
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
      //if(p1[i] > p2[i]) { p1[i]--; } // Decrease twice as fast
    }

    if(changes >= max_changes) { break; }
  }
}

