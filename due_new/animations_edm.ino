// This file is for completed edm animations

//---------------------------- EQUALIZER_FULL ---------------------------
// This isn't actually done yet :o
// to do: does heights[] need to be uint16_t? or can be uint8_t? Is (float) necessary on freq_max[]?
// BASE_BLACK_THICKNESS(3:8)
void equalizer_full(uint8_t display_mode) {
  leds[0] = CRGB::Red;
  uint8_t black_thickness = scale_param(BASE_BLACK_THICKNESS, 3, 8);
  
  uint8_t unlit_pixels = black_thickness * (NUM_CHANNELS-1);
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
    color %= 128; // half brightness
    
    for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
      for(uint16_t pixel = cur_height; pixel <= cur_height + height; pixel++) {
        leds[get_1d_index(ring, pixel)] = color;
        if(display_mode == DISPLAY_SPLIT) { leds[get_1d_index(ring, LEDS_PER_RING - 1 - pixel)] = color; }
      }
    }
    cur_height += height;

    if(chan < NUM_CHANNELS-1) {
      for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
        for(uint16_t pixel = cur_height; pixel < cur_height + black_thickness; pixel++) {
          leds[get_1d_index(ring, pixel)] = CRGB::Black;
          if(display_mode == DISPLAY_SPLIT) { leds[get_1d_index(ring, LEDS_PER_RING - 1 - pixel)] = CRGB::Black; }
        }
      }
      cur_height += black_thickness;
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
// BASE_BLACK_THICKNESS(1:4)
void equalizer_variable(uint8_t display_mode) {
  leds[0] = CRGB::Blue;
  uint8_t black_thickness = scale_param(BASE_BLACK_THICKNESS, 1, 4);
  
  uint8_t max_height = (LEDS_PER_RING - (NUM_CHANNELS-1) * black_thickness) / NUM_CHANNELS;
  
  uint16_t cur_height = 0;
  for(uint8_t chan = 0; chan < NUM_CHANNELS; chan++) {
    uint16_t freq_max = frequencies_one[chan] > frequencies_two[chan] ? frequencies_one[chan] : frequencies_two[chan];
    uint16_t height = freq_max / 6;
    if(height > max_height) {
      #ifdef DEBUG
        Serial.println("equalizer_variable() height capped. Clipped from " + String(height) + " down to " + String(max_height));
      #endif
      height = max_height;
    }
    if(display_mode == DISPLAY_SPLIT) { height /= 2; }
      
    CRGB color = current_palette[chan];
    color.maximizeBrightness();
    color %= 128; // half brightness
    
    for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
      for(uint16_t pixel = cur_height; pixel <= cur_height + height; pixel++) {
        leds[get_1d_index(ring, pixel)] = color;
        if(display_mode == DISPLAY_SPLIT) { leds[get_1d_index(ring, LEDS_PER_RING - 1 - pixel)] = color; }
      }
    }
    
    cur_height += height;

    if(chan < NUM_CHANNELS-1) {
      for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
        for(uint16_t pixel = cur_height; pixel < cur_height + black_thickness; pixel++) {
          leds[get_1d_index(ring, pixel)] = CRGB::Black;
          if(display_mode == DISPLAY_SPLIT) { leds[get_1d_index(ring, LEDS_PER_RING - 1 - pixel)] = CRGB::Black; }
        }
      }
      cur_height += black_thickness;
    }

    //Serial.println("chan " + String(chan) + ", height=" + String(cur_height));
  }

  uint16_t stop_point = display_mode == DISPLAY_SPLIT ? HALF_RING : LEDS_PER_RING;
  while(cur_height < stop_point) {
    //Serial.println("black drawn at height=" + String(cur_height));
    for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
      leds[get_1d_index(ring, cur_height)] = CRGB::Black;
      if(display_mode == DISPLAY_SPLIT) { leds[get_1d_index(ring, LEDS_PER_RING - 1 - cur_height)] = CRGB::Black; }
    }
    cur_height++;
  }
}


//---------------------------- EQUALIZER_PULSE ---------------------------
// Equally sized bands for each channel, brightness goes up and down with channel volume
// BASE_COLOR_THICKNESS(55)
#define BAND_LENGTH LEDS_PER_RING / 7
#define FIRST_BAND_OFFSET BAND_LENGTH/2
void equalizer_pulse() {
  equalizer_variable(SPLIT);
  return;
  
  uint8_t color_thickness = scale_param(BASE_COLOR_THICKNESS, 55, 70);
  
  leds_all = CRGB::Black;
  
  for(uint8_t chan = 0; chan < NUM_CHANNELS; chan++) {
    CRGB color = current_palette[chan];
    color.maximizeBrightness();
    uint16_t scaling = (frequencies_one[chan] + frequencies_two[chan]) / 6;
    if(scaling > 255) { 
      #ifdef DEBUG
        Serial.println("equalizer_pulse() brightness scaling capped, clipped from " + String(scaling) + " down to 255.");
      #endif
      scaling = 255;
    }
    
    uint8_t band_height = color_thickness * scaling / 255;
    int16_t min_pixel = FIRST_BAND_OFFSET + chan * BAND_LENGTH - band_height/2;
    uint16_t max_pixel = FIRST_BAND_OFFSET + chan * BAND_LENGTH + band_height/2 - 1;
    if(min_pixel < 0) { min_pixel = 0; }
    if(max_pixel >= LEDS_PER_RING) { max_pixel = LEDS_PER_RING-1; }
    
    for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
      for(uint16_t pixel = min_pixel; pixel <= max_pixel; pixel++) {
        uint16_t index = get_1d_index(ring, pixel);
        if(leds[index] == CRGB(0,0,0)) {
          leds[index] = color;
        }
        else {
          nblend(&leds[index], &color, 1, 128);
        }
      }
    }
  }
}


//---------------------------- FREQUENCY_PULSE ---------------------------
// Equally sized bands for each channel, brightness goes up and down with channel volume
#define MIN_FREQ_PULSE_BRIGHTNESS 8
// BASE_COLOR_THICKNESS(50:57), To be added: BASE_INTRA_RING_MOTION(-1:1), BASE_RING_OFFSET(-6:6), BASE_INTRA_RING_SPEED(4:32)
void frequency_pulse() {
  uint8_t color_thickness = scale_param(BASE_COLOR_THICKNESS, 48, 55);
  uint8_t intra_speed = scale_param(BASE_INTRA_RING_SPEED, 4, 32);
  int8_t ring_offset = scale_param(BASE_RING_OFFSET, -6, 6);
  
  for(uint8_t chan = 0; chan < NUM_CHANNELS; chan++) {
    uint16_t min_pixel = FIRST_BAND_OFFSET + chan * BAND_LENGTH - color_thickness/2;
    uint16_t max_pixel = FIRST_BAND_OFFSET + chan * BAND_LENGTH + color_thickness/2 - 1;

    CRGB color = current_palette[chan];
    color.maximizeBrightness();
    color %= 128; // half brightness
    
    uint16_t scaling = (frequencies_one[chan] + frequencies_two[chan]) / 4;
    if(scaling > 255) { 
      #ifdef DEBUG
        Serial.println("frequency_pulse() brightness scaling capped, clipped from " + String(scaling) + " down to 255.");
      #endif
      scaling = 255;
    }
    else if(scaling < MIN_FREQ_PULSE_BRIGHTNESS) { scaling = MIN_FREQ_PULSE_BRIGHTNESS; }
    color %= scaling;

    for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
      for(uint16_t pixel = min_pixel; pixel <= max_pixel; pixel++) {
        leds[get_1d_index(ring, pixel)] = color;
      }
    }
  }

  //uint8_t flat_offset = (BASE_INTRA_RING_MOTION * intra_speed * base_count / THROTTLE) % LEDS_PER_RING;
  //rotate_leds(flat_offset, ring_offset, BASE_INTRA_RING_MOTION == CW);
}


