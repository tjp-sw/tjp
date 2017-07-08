// Overlays layers and writes to leds[]
void write_pixel_data() {
  uint8_t ring_offset = RINGS_PER_NODE * node_number;

  for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++)
  {
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
      uint8_t color_index = get_sparkle(ring_offset+ring, pixel);
      CRGB pixel_color;

      if(color_index != 0) {
        pixel_color = ColorFromPalette(sparkle_palette, color_index);
      }
      else {
        color_index = mid_layer[ring_offset+ring][pixel];
        if(color_index != 0) {
          pixel_color = ColorFromPalette(mid_palette, color_index);
          /*if(ring == 4 && pixel > 205 && pixel < 408) {
            Serial.print("mid_layer[4][" + String(pixel) + "] = palette[" + String(mid_layer[ring][pixel]) + "] = (");
            Serial.println(String(pixel_color.r) + "," + String(pixel_color.g) + "," + String(pixel_color.b) + ")");
          }*/
        }
        else {
          color_index = base_layer[ring_offset+ring][pixel];
          pixel_color = ColorFromPalette(base_palette, color_index);
        }
      }

      leds[pixel_offset] = pixel_color;
      if(backward_strip) pixel_offset--;
      else pixel_offset++;
    }
  }
}


// Clear layer functions
void clear_base_layer() {
  for(uint8_t ring = 0; ring < NUM_RINGS; ring++) {
    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      base_layer[ring][pixel] = TRANSPARENT;
    }
  }
}

void clear_mid_layer() {
  for(uint8_t ring = 0; ring < NUM_RINGS; ring++) {
    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      mid_layer[ring][pixel] = TRANSPARENT;
    }
  }
}

void clear_sparkle_layer() {
  for(uint8_t ring = 0; ring < NUM_RINGS; ring++) {
    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      set_sparkle(ring, pixel, TRANSPARENT);
    }
  }
}


// Takes a ring 0-11, pixel 0-407, returns 1d offset into leds[]. Not to be used when drawing layers.
uint16_t get_1d_index(uint8_t ring, uint16_t pixel) {
  #ifdef DEBUG
    if(ring >= RINGS_PER_NODE) Serial.println("Error in get_1d_index(" + String(ring) + ", " + String(pixel) + "). Ring out of range.");
    if(pixel >= LEDS_PER_RING) Serial.println("Error in get_1d_index(" + String(ring) + ", " + String(pixel) + "). Pixel out of range.");
  #endif
  
  bool backward_strip = ring < RINGS_PER_NODE/2;
  uint8_t strip = ring % 2 + (backward_strip ? 0 : 2);
  
  uint16_t pixel_offset = LEDS_PER_STRIP * strip;
  if(backward_strip) {
    return pixel_offset + LEDS_PER_STRIP - 1 - PHYSICAL_LEDS_PER_RING*(ring/2) - pixel;
  }
  else {
    return pixel_offset + 1 + PHYSICAL_LEDS_PER_RING*((ring - RINGS_PER_NODE/2)/2) + pixel;
  }
}


// Bit-bashing functions for reading/writing nybbles in sparkle layer
void set_sparkle(uint8_t ring, uint16_t pixel, uint8_t value) {
  uint8_t pixel_index = pixel >> 1;
  uint8_t curVal = sparkle_layer[ring][pixel_index];
  if(pixel & 0x01) {
    sparkle_layer[ring][pixel_index] = (curVal & 0xF0) | (value & 0x0F);
  }
  else {
    sparkle_layer[ring][pixel_index] = (curVal & 0x0F) | ((value<<4) & 0xF0);
  }
}

uint8_t get_sparkle(uint8_t ring, uint16_t pixel) {
  uint8_t pixel_index = pixel >> 1;
  if(pixel & 0x01) {
    return sparkle_layer[ring][pixel_index] & 0x0F;
  }
  else {
    return (sparkle_layer[ring][pixel_index] & 0xF0) >> 4;
  }
}

