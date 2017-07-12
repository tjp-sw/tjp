// This file is for completed edm animations

//---------------------------- EQUALIZER_FULL ---------------------------
// This isn't actually done yet :o
// to do: does heights[] need to be uint16_t? or can be uint8_t? Is (float) necessary on freq_max[]?
void equalizer_full(uint8_t display_mode) {
  uint8_t unlit_pixels = BASE_BLACK_THICKNESS * (NUM_CHANNELS-1);
  uint16_t total_colored_pixels = LEDS_PER_RING - unlit_pixels;
  if(display_mode == DISPLAY_SPLIT) { total_colored_pixels /= 2; }

  uint16_t freq_max[NUM_CHANNELS];
  uint16_t overall_volume = 0;
  for(uint8_t chan = 0; chan < NUM_CHANNELS; chan++) {
    freq_max[chan] = frequencies_one[chan] > frequencies_two[chan] ? frequencies_one[chan] : frequencies_two[chan];
    overall_volume += freq_max[chan];
  }

  uint16_t cur_height = 0;
  for(uint8_t chan = 0; chan < NUM_CHANNELS; chan++) {
    uint16_t height = total_colored_pixels * ((float)freq_max[chan] / overall_volume);
    CRGB color = current_palette[chan];
    color.maximizeBrightness();
    
    for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
      for(uint16_t pixel = cur_height; pixel <= cur_height + height; pixel++) {
        leds[get_1d_index(ring, pixel)] = color;
        if(display_mode == DISPLAY_SPLIT) { leds[get_1d_index(ring, LEDS_PER_RING - 1 - pixel)] = color; }
      }
    }
    cur_height += height;

    if(chan < NUM_CHANNELS-1) {
      for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
        for(uint16_t pixel = cur_height; pixel < cur_height + BASE_BLACK_THICKNESS; pixel++) {
          leds[get_1d_index(ring, pixel)] = CRGB::Black;
          if(display_mode == DISPLAY_SPLIT) { leds[get_1d_index(ring, LEDS_PER_RING - 1 - pixel)] = CRGB::Black; }
        }
      }
      cur_height += BASE_BLACK_THICKNESS;
    }
    else {
      // Last channel, write extra pixels until entire ring is full.
      uint8_t max_pixel = display_mode == DISPLAY_SPLIT ? LEDS_PER_RING - cur_height : LEDS_PER_RING;
      for(; cur_height < max_pixel; cur_height++) {
        for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
          leds[get_1d_index(ring, cur_height)] = color;
        }
      }
    }
  }
}


//---------------------------- EQUALIZER_VARIABLE ---------------------------
// Stacked bands, one for each channel. Size varies based on volume
// to do: does heights[] need to be uint16_t? or can be uint8_t? Is (float) necessary?
// to do: remove per-channel max_height, instead just reduce heights if total height exceeds LEDS_PER_RING
void equalizer_variable(uint8_t display_mode) {
  uint8_t max_height = (LEDS_PER_RING - (NUM_CHANNELS-1) * BASE_BLACK_THICKNESS) / NUM_CHANNELS;
  
  uint16_t cur_height = 0;
  for(uint8_t chan = 0; chan < NUM_CHANNELS; chan++) {
    uint16_t freq_max = frequencies_one[chan] > frequencies_two[chan] ? frequencies_one[chan] : frequencies_two[chan];
    uint16_t height = freq_max / 10;
    if(height > max_height) {
      #ifdef DEBUG
        Serial.println("equalizer_variable() height capped. Clipped from " + String(height) + " down to " + String(max_height));
      #endif
      height = max_height;
    }
    if(display_mode == DISPLAY_SPLIT) { height /= 2; }
      
    CRGB color = current_palette[chan];
    color.maximizeBrightness();
    
    for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
      for(uint16_t pixel = cur_height; pixel <= cur_height + height; pixel++) {
        leds[get_1d_index(ring, pixel)] = color;
        if(display_mode == DISPLAY_SPLIT) { leds[get_1d_index(ring, LEDS_PER_RING - 1 - pixel)] = color; }
      }
    }
    
    cur_height += height;

    if(chan < NUM_CHANNELS-1) {
      for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
        for(uint16_t pixel = cur_height; pixel < cur_height + BASE_BLACK_THICKNESS; pixel++) {
          leds[get_1d_index(ring, pixel)] = CRGB::Black;
          if(display_mode == DISPLAY_SPLIT) { leds[get_1d_index(ring, LEDS_PER_RING - 1 - pixel)] = CRGB::Black; }
        }
      }
      cur_height += BASE_BLACK_THICKNESS;
    }
  }

  uint16_t stop_point = display_mode == DISPLAY_SPLIT ? HALF_RING : LEDS_PER_RING;
  while(cur_height < stop_point) {
    for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
      leds[get_1d_index(ring, cur_height)] = CRGB::Black;
      if(display_mode == DISPLAY_SPLIT) { leds[get_1d_index(ring, LEDS_PER_RING - 1 - cur_height)] = CRGB::Black; }
    }
    cur_height++;
  }
}


//---------------------------- FREQUENCY_PULSE ---------------------------
// Equally sized bands for each channel, brightness goes up and down with channel volume
#define BAND_LENGTH LEDS_PER_RING / 7
#define FIRST_BAND_OFFSET BAND_LENGTH/2
void frequency_pulse() {
  for(uint8_t chan = 0; chan < NUM_CHANNELS; chan++) {
    uint16_t min_pixel = FIRST_BAND_OFFSET + chan * BAND_LENGTH - BASE_COLOR_THICKNESS/2;
    uint16_t max_pixel = FIRST_BAND_OFFSET + chan * BAND_LENGTH + BASE_COLOR_THICKNESS/2 - 1;

    CRGB color = current_palette[chan];
    color.maximizeBrightness();
    uint16_t scaling = (frequencies_one[chan] + frequencies_two[chan]) / 6;
    if(scaling > 255) { 
      #ifdef DEBUG
        Serial.println("frequency_pulse() brightness scaling capped, clipped from " + String(scaling) + " down to 255.");
      #endif
      scaling = 255;
    }
    else if(scaling < 8) { scaling = 8; }
    color %= scaling;

    for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
      for(uint16_t pixel = min_pixel; pixel <= max_pixel; pixel++) {
        leds[get_1d_index(ring, pixel)] = color;
      }
    }
  }
}

