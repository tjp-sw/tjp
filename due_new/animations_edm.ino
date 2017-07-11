// This file is for completed edm animations

//---------------------------- EQUALIZER_FULL ---------------------------
// This isn't actually done yet :o
// to do: does heights[] need to be uint16_t? or can be uint8_t? Is (float) necessary on freq_max[]?
void equalizer_full(bool split) {
  uint8_t unlit_pixels = BASE_BLACK_THICKNESS * (NUM_CHANNELS-1);
  uint16_t total_colored_pixels = LEDS_PER_RING - unlit_pixels;
  if(split) { total_colored_pixels /= 2; }

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
        if(split) { leds[get_1d_index(ring, LEDS_PER_RING - 1 - pixel)] = color; }
      }
    }
    cur_height += height;

    if(chan < NUM_CHANNELS-1) {
      for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
        for(uint16_t pixel = cur_height; pixel < cur_height + BASE_BLACK_THICKNESS; pixel++) {
          leds[get_1d_index(ring, pixel)] = CRGB::Black;
          if(split) { leds[get_1d_index(ring, LEDS_PER_RING - 1 - pixel)] = CRGB::Black; }
        }
      }
      cur_height += BASE_BLACK_THICKNESS;
    }
    else {
      // Last channel, write extra pixels until entire ring is full.
      uint8_t max_pixel = split ? LEDS_PER_RING - cur_height : LEDS_PER_RING;
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
void equalizer_variable(bool split) {
  uint16_t cur_height = 0;
  for(uint8_t chan = 0; chan < NUM_CHANNELS; chan++) {
    uint16_t freq_max = frequencies_one[chan] > frequencies_two[chan] ? frequencies_one[chan] : frequencies_two[chan];
    uint16_t height = freq_max > 255 ? 64 : freq_max / 4;
    if(split) { height /= 2; }
      
    CRGB color = current_palette[chan];
    color.maximizeBrightness();
    
    for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
      for(uint16_t pixel = cur_height; pixel <= cur_height + height; pixel++) {
        leds[get_1d_index(ring, pixel)] = color;
        if(split) { leds[get_1d_index(ring, LEDS_PER_RING - 1 - pixel)] = color; }
      }
      
    }
    
    cur_height += height;

    if(chan < NUM_CHANNELS-1) {
      for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
        for(uint16_t pixel = cur_height; pixel < cur_height + BASE_BLACK_THICKNESS; pixel++) {
          leds[get_1d_index(ring, pixel)] = CRGB::Black;
          if(split) { leds[get_1d_index(ring, LEDS_PER_RING - 1 - pixel)] = CRGB::Black; }
        }
      }
      cur_height += BASE_BLACK_THICKNESS;
    }
  }
}


//---------------------------- FREQUENCY_PULSE ---------------------------
// Equally sized bands for each channel, brightness goes up and down with channel volume
#define BAND_DISTANCE LEDS_PER_RING / 8
#define FIRST_BAND_OFFSET BAND_DISTANCE/2
void frequency_pulse() {
  for(uint8_t chan = 0; chan < NUM_CHANNELS; chan++) {
    uint16_t min_pixel = FIRST_BAND_OFFSET + chan * BAND_DISTANCE - BASE_COLOR_THICKNESS;
    uint16_t max_pixel = FIRST_BAND_OFFSET + chan * BAND_DISTANCE + BASE_COLOR_THICKNESS - 1;
    
    CRGB color = current_palette[chan];
    color.maximizeBrightness();
    uint16_t scaling = (frequencies_one[chan] + frequencies_two[chan]) / 4;
    if(scaling > 255) { scaling = 255; }
    color %= scaling;

    for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
      for(uint16_t pixel = min_pixel; pixel <= max_pixel; pixel++) {
        leds[get_1d_index(ring, pixel)] = color;
      }
    }
  }
}

