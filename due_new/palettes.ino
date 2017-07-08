void setup_palettes() {
  base_palette.entries[TRANSPARENT] = CRGB::Black;
  base_palette.entries[WHITE] = CRGB::White;
  base_palette.entries[BLACK] = CRGB::Black;
  base_palette.entries[GRAY] = CRGB(128, 128, 128);
  base_palette_target.entries[TRANSPARENT] = CRGB::Black;
  base_palette_target.entries[WHITE] = CRGB::White;
  base_palette_target.entries[BLACK] = CRGB::Black;
  base_palette_target.entries[GRAY] = CRGB(128, 128, 128);

  mid_palette.entries[TRANSPARENT] = CRGB::Black;
  mid_palette.entries[WHITE] = CRGB::White;
  mid_palette.entries[BLACK] = CRGB::Black;
  mid_palette.entries[GRAY] = CRGB(128, 128, 128);
  mid_palette_target.entries[TRANSPARENT] = CRGB::Black;
  mid_palette_target.entries[WHITE] = CRGB::White;
  mid_palette_target.entries[BLACK] = CRGB::Black;
  mid_palette_target.entries[GRAY] = CRGB(128, 128, 128);

  sparkle_palette.entries[TRANSPARENT] = CRGB::Black;
  sparkle_palette.entries[WHITE] = CRGB::White;
  sparkle_palette_target.entries[TRANSPARENT] = CRGB::Black;
  sparkle_palette_target.entries[WHITE] = CRGB::White;

  for(uint8_t ring = 0; ring < NUM_RINGS; ring++) {
    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      set_sparkle(ring, pixel, TRANSPARENT);
      mid_layer[ring][pixel] = TRANSPARENT;
      base_layer[ring][pixel] = TRANSPARENT;
    }
  }

  set_palettes((uint8_t*) initial_palette);
}

// Functions to get the index into each layer's palette
uint8_t get_base_color(uint8_t color0, uint8_t color1, uint8_t blending, uint8_t dimming) {
  #ifdef DEBUG
    if(color0 > 1) Serial.println("Error in get_base_color(), color0 == " + String(color0));
    if(color1 > 1) Serial.println("Error in get_base_color(), color1 == " + String(color1));
    if(blending > 35) Serial.println("Error in get_base_color(), blending == " + String(blending));
    if(dimming > 7) Serial.println("Error in get_base_color(), dimming == " + String(dimming));
  #endif

  if(dimming >= 7) {
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
    if(dimming > 6) Serial.println("Error in get_sparkle_color(), dimming == " + String(dimming));
  #endif

  return 2 + 7*color + dimming;
}
uint8_t get_sparkle_color(uint8_t color) {
  return get_sparkle_color(color, 0);
}

uint8_t get_edm_color(uint8_t color, uint8_t dimming) {
    #ifdef DEBUG
    if(color > 6) Serial.println("Error in get_sparkle_color(), color == " + String(color));
    if(dimming > 6) Serial.println("Error in get_sparkle_color(), dimming == " + String(dimming));
  #endif

  if(color < 2) { return get_base_color(color, dimming); }
  if(color < 4) { return get_mid_color(color-2, dimming); }
  return get_sparkle_color(color-5, dimming);
}
uint8_t get_edm_color(uint8_t color) {
  return get_edm_color(color, 0);
}


// Called when a new palette is received from the Pi
void set_palettes(uint8_t* colors) {
  CRGB temp;
  
  CRGB base0 = CRGB(colors[0], colors[1], colors[2]);
  CRGB base1 = CRGB(colors[3], colors[4], colors[5]);
  if(base0 != base_palette.entries[get_base_color(0)] || base1 != base_palette.entries[get_base_color(1)]) {
    // Change to base palette
    blend_base_layer = true;
    for(uint8_t i = 0; i < 36; i++) {
      uint16_t palette_offset = 4 + 7*i;
      // 36 unique hues/saturations, each with 7 brightness values
      temp = base0;
      nblend(temp, base1, 255 * i / 35);

      base_palette_target.entries[palette_offset] = temp;
      base_palette_target.entries[palette_offset + 1] = CRGB(temp.r / 2, temp.g / 2, temp.b / 2);
      base_palette_target.entries[palette_offset + 2] = CRGB(temp.r / 4, temp.g / 4, temp.b / 4);
      base_palette_target.entries[palette_offset + 3] = CRGB(temp.r / 6, temp.g / 6, temp.b / 6);
      base_palette_target.entries[palette_offset + 4] = CRGB(temp.r / 9, temp.g / 9, temp.b / 9);
      base_palette_target.entries[palette_offset + 5] = CRGB(temp.r / 12, temp.g / 12, temp.b / 12);
      base_palette_target.entries[palette_offset + 6] = CRGB(temp.r / 15, temp.g / 15, temp.b / 15);
    }
  }
  

  CRGB mid0 = CRGB(colors[6], colors[7], colors[8]);
  CRGB mid1 = CRGB(colors[9], colors[10], colors[11]);
  CRGB mid2 = CRGB(colors[12], colors[13], colors[14]);
  if(mid0 != mid_palette.entries[get_mid_color(0)] || mid1 != mid_palette.entries[get_mid_color(1)] || mid2 != mid_palette.entries[get_mid_color(2)]) {
    // Change to mid palette
    blend_mid_layer = true;
    for(uint8_t i = 0; i < 36; i++) {
      uint16_t palette_offset = 4 + 7*i;
      // 36 unique hues/saturations, each with 7 brightness values
      // 12 mid0 -> mid1, 12 mid0 -> mid2, 12 mid1 -> mid2
      if(i < 12) {
        temp = mid0;
        nblend(temp, mid1, 255 * i / 11);
      }
      else if(i < 24) {
        temp = mid0;
        nblend(temp, mid2, 255 * (i-12) / 11);
      }
      else {
        temp = mid1;
        nblend(temp, mid2, 255 * (i-24) / 11);
      }

      mid_palette_target.entries[palette_offset] = temp;
      mid_palette_target.entries[palette_offset + 1] = CRGB(temp.r / 2, temp.g / 2, temp.b / 2);
      mid_palette_target.entries[palette_offset + 2] = CRGB(temp.r / 4, temp.g / 4, temp.b / 4);
      mid_palette_target.entries[palette_offset + 3] = CRGB(temp.r / 6, temp.g / 6, temp.b / 6);
      mid_palette_target.entries[palette_offset + 4] = CRGB(temp.r / 9, temp.g / 9, temp.b / 9);
      mid_palette_target.entries[palette_offset + 5] = CRGB(temp.r / 12, temp.g / 12, temp.b / 12);
      mid_palette_target.entries[palette_offset + 6] = CRGB(temp.r / 15, temp.g / 15, temp.b / 15);
    }
  }


  CRGB sparkle0 = CRGB(colors[15], colors[16], colors[17]);
  CRGB sparkle1 = CRGB(colors[18], colors[19], colors[20]);
  if(sparkle0 != sparkle_palette.entries[get_sparkle_color(0)] || sparkle1 != sparkle_palette.entries[get_sparkle_color(1)]) {
    // Change to sparkle palette
    blend_sparkle_layer = true;

    for(uint8_t i = 0; i < 2; i++) {
      uint8_t palette_offset = 2+7*i;
      temp = i == 0 ? sparkle0 : sparkle1;
      sparkle_palette_target.entries[palette_offset] = temp;
      sparkle_palette_target.entries[palette_offset + 1] = CRGB(temp.r / 2, temp.g / 2, temp.b / 2);
      sparkle_palette_target.entries[palette_offset + 2] = CRGB(temp.r / 4, temp.g / 4, temp.b / 4);
      sparkle_palette_target.entries[palette_offset + 3] = CRGB(temp.r / 6, temp.g / 6, temp.b / 6);
      sparkle_palette_target.entries[palette_offset + 4] = CRGB(temp.r / 9, temp.g / 9, temp.b / 9);
      sparkle_palette_target.entries[palette_offset + 5] = CRGB(temp.r / 12, temp.g / 12, temp.b / 12);
      sparkle_palette_target.entries[palette_offset + 6] = CRGB(temp.r / 15, temp.g / 15, temp.b / 15);
    }
  }
}


// Functions to smoothly blend from one palette into the next one
bool blend_palette_16(CRGBPalette16& current, CRGBPalette16& target, uint8_t maxChanges) {
  uint8_t* p1;
  uint8_t* p2;
  uint8_t  changes = 0;

  p1 = (uint8_t*)current.entries;
  p2 = (uint8_t*)target.entries;

  const uint8_t totalChannels = sizeof(CRGBPalette16);
  //while(changes < maxChanges) {
    uint8_t initChanges = changes;
    for(uint16_t i = 0; i < totalChannels; i++) {
      // if the values are equal, no changes are needed
      if( p1[i] == p2[i] ) { continue; }

      // if the current value is less than the target, increase it by up to 2
      if(p1[i] < p2[i]) {
        changes++;
        p1[i]++;
        if(p1[i] < p2[i]) {
          p1[i]++;
        }
      }

      // if the current value is greater than the target, decrease it by up to 4
      if(p1[i] > p2[i]) {
        changes++;
        p1[i]--;
        if(p1[i] > p2[i]) {
          p1[i]--;
          if(p1[i] > p2[i]) {
            p1[i]--;
            if(p1[i] > p2[i]) {
              p1[i]--;
            }
          }
        }
      }

      if(changes >= maxChanges) {
        // if we've hit the maximum number of changes, exit with true
        return true;
      }
    }

    if(changes == initChanges) {
      // done blending, exit with false
      return false;
    }
  //}
}

bool blend_palette_256(CRGBPalette256& current, CRGBPalette256& target, uint16_t maxChanges) {
  uint8_t* p1;
  uint8_t* p2;
  uint8_t  changes = 0;

  maxChanges *= 16; // Scaled up to fit CRGBPalette256 instead of CRGBPalette16

  p1 = (uint8_t*)current.entries;
  p2 = (uint8_t*)target.entries;

  const uint16_t totalChannels = sizeof(CRGBPalette256);
  //while(changes < maxChanges) {
    uint8_t initChanges = changes;
    for(uint16_t i = 0; i < totalChannels; i++) {
      // if the values are equal, no changes are needed
      if( p1[i] == p2[i] ) { continue; }

      // if the current value is less than the target, increase it by up to 2
      if(p1[i] < p2[i]) {
        changes++;
        p1[i]++;
        if(p1[i] < p2[i]) {
          p1[i]++;
        }
      }

      // if the current value is greater than the target, decrease it by up to 4
      if(p1[i] > p2[i]) {
        changes++;
        p1[i]--;
        if(p1[i] > p2[i]) {
          p1[i]--;
          if(p1[i] > p2[i]) {
            p1[i]--;
            if(p1[i] > p2[i]) {
              p1[i]--;
            }
          }
        }
      }

      if(changes >= maxChanges) {
        // if we've hit the maximum number of changes, exit with true
        return true;
      }
    }

    if(changes == initChanges) {
      // done blending, exit with false
      return false;
    }
  //}
}

