// EDM animations don't use layers. They are overlayed on top of old layers, which will usually be disabled while the EDM animation plays.
// They directly access the leds array, which means they can use CRGBs, but don't know the last value of all 30k LEDs. They only store the current node's data.

#define BAND_LENGTH LEDS_PER_RING / 7
#define FIRST_BAND_OFFSET BAND_LENGTH/2

//---------------------------- FREQUENCY_PULSE ---------------------------
// Equally sized bands for each channel, brightness goes up and down with channel volume
#define MIN_FREQ_PULSE_BRIGHTNESS 10
// BASE_COLOR_THICKNESS(50:57), BASE_INTRA_RING_MOTION(-1:1), BASE_RING_OFFSET(-6:6), BASE_INTRA_RING_SPEED(4:32)
inline void frequency_pulse(uint8_t min_ring, uint8_t max_ring) {
  uint8_t color_thickness = scale_param(BASE_COLOR_THICKNESS, 48, 55);
  //uint8_t intra_speed = 1 << scale_param(BASE_INTRA_RING_SPEED, 2, 5);
  //int8_t ring_offset = scale_param(BASE_RING_OFFSET, -6, 6);
  
  for(uint8_t chan = 0; chan < NUM_CHANNELS; chan++) {
    uint16_t min_pixel = FIRST_BAND_OFFSET + chan * BAND_LENGTH - color_thickness/2;
    uint16_t max_pixel = FIRST_BAND_OFFSET + chan * BAND_LENGTH + color_thickness/2 - 1;

    CRGB color = current_palette[chan];
    color.maximizeBrightness();
    color %= 128; // half brightness
    
    uint16_t scaling = freq_smooth[chan];
    
    if(scaling < MIN_FREQ_PULSE_BRIGHTNESS) { scaling = MIN_FREQ_PULSE_BRIGHTNESS; }
    color %= scaling;

    for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
      if(node_number*RINGS_PER_NODE + ring < min_ring) { continue; }
      if(node_number*RINGS_PER_NODE + ring >= max_ring) { continue; }
      
      for(uint16_t pixel = min_pixel; pixel <= max_pixel; pixel++) {
        set_led(ring, pixel, color);
      }
    }
  }

  //uint8_t flat_offset = (BASE_INTRA_RING_MOTION * intra_speed * base_count / THROTTLE) % LEDS_PER_RING;
  //rotate_leds(flat_offset, ring_offset, BASE_INTRA_RING_MOTION == CW);
}



//---------------------------- EQUALIZER_VARIABLE ---------------------------
// Stacked bands, one for each channel. Size varies based on volume
// to do: does heights[] need to be uint16_t? or can be uint8_t?
// to do: remove per-channel max_height, instead just reduce heights if total height exceeds LEDS_PER_RING
// BASE_BLACK_THICKNESS(1:4)
inline void equalizer_variable(uint8_t display_mode, uint8_t min_ring, uint8_t max_ring) {
  uint8_t black_thickness = scale_param(BASE_BLACK_THICKNESS, 1, 4);
  
  uint8_t max_height = (LEDS_PER_RING - (NUM_CHANNELS-1) * black_thickness) / NUM_CHANNELS;
  
  uint16_t cur_height = 0;
  for(uint8_t chan = 0; chan < NUM_CHANNELS; chan++) {
    uint16_t height = freq_smooth[chan] * max_height / 255;
    if(display_mode == DISPLAY_SPLIT) { height /= 2; }
      
    CRGB color = current_palette[chan];
    color.maximizeBrightness();
    color %= 128; // half brightness
    
    for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
      if(node_number*RINGS_PER_NODE + ring < min_ring) { continue; }
      if(node_number*RINGS_PER_NODE + ring >= max_ring) { continue; }
      
      for(uint16_t pixel = cur_height; pixel <= cur_height + height; pixel++) {
        set_led(ring, LEDS_PER_RING - 1 - pixel, color);
        if(display_mode == DISPLAY_SPLIT) { set_led(ring, pixel, color); }
      }
    }
    
    cur_height += height;

    if(chan < NUM_CHANNELS-1) {
      for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
        for(uint16_t pixel = cur_height; pixel < cur_height + black_thickness; pixel++) {
          set_led(ring, LEDS_PER_RING - 1 - pixel, CRGB::Black);
          if(display_mode == DISPLAY_SPLIT) { set_led(ring, pixel, CRGB::Black); }
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
      leds[get_1d_index(ring, LEDS_PER_RING - 1 - cur_height)].fadeToBlackBy(32);
      if(display_mode == DISPLAY_SPLIT) { leds[get_1d_index(ring, cur_height)].fadeToBlackBy(32); }
    }
    cur_height++;
  }
}


//---------------------------- EQUALIZER_FULL ---------------------------
// This isn't actually done yet :o
// to do: does heights[] need to be uint16_t? or can be uint8_t?
// BASE_BLACK_THICKNESS(3:8)
inline void equalizer_full(uint8_t display_mode, uint8_t min_ring, uint8_t max_ring) {
  uint8_t black_thickness = scale_param(BASE_BLACK_THICKNESS, 3, 8);
  
  uint8_t unlit_pixels = black_thickness * (NUM_CHANNELS-1);
  uint16_t total_colored_pixels = LEDS_PER_RING - unlit_pixels;
  if(display_mode == DISPLAY_SPLIT) { total_colored_pixels /= 2; }

  uint16_t overall_volume = 0;
  for(uint8_t chan = 0; chan < NUM_CHANNELS; chan++) {
    overall_volume += freq_smooth[chan];
  }

  uint16_t cur_height = 0;
  for(uint8_t chan = 0; chan < NUM_CHANNELS; chan++) {
    uint16_t height = total_colored_pixels * freq_smooth[chan] / overall_volume;

    CRGB color = current_palette[chan];
    color.maximizeBrightness();
    color %= 128; // half brightness
    
    for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
      if(node_number*RINGS_PER_NODE + ring < min_ring) { continue; }
      if(node_number*RINGS_PER_NODE + ring >= max_ring) { continue; }
      
      for(uint16_t pixel = cur_height; pixel <= cur_height + height; pixel++) {
        set_led(ring, pixel, color);
        if(display_mode == DISPLAY_SPLIT) { set_led(ring, LEDS_PER_RING - 1 - pixel, color); }
      }
    }
    cur_height += height;

    if(chan < NUM_CHANNELS-1) {
      for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
        if(node_number*RINGS_PER_NODE + ring < min_ring) { continue; }
        if(node_number*RINGS_PER_NODE + ring >= max_ring) { continue; }
        
        for(uint16_t pixel = cur_height; pixel < cur_height + black_thickness; pixel++) {
          set_led(ring, pixel, CRGB::Black);
          if(display_mode == DISPLAY_SPLIT) { set_led(ring, LEDS_PER_RING - 1 - pixel, CRGB::Black); }
        }
      }
      cur_height += black_thickness;
    }
    else {
      // Last channel, write extra pixels until entire ring is full.
      uint8_t max_pixel = display_mode == DISPLAY_SPLIT ? LEDS_PER_RING - cur_height : LEDS_PER_RING;
      for(; cur_height < max_pixel; cur_height++) {
        for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
          if(node_number*RINGS_PER_NODE + ring < min_ring) { continue; }
          if(node_number*RINGS_PER_NODE + ring >= max_ring) { continue; }
          set_led(ring, cur_height, color);
        }
      }
    }
  }
}



//---------------------------- EQUALIZER_PULSE ---------------------------
// Equally sized bands for each channel, brightness goes up and down with channel volume
// BASE_COLOR_THICKNESS(55)
inline void equalizer_pulse(uint8_t min_ring, uint8_t max_ring) {
  uint8_t color_thickness = scale_param(BASE_COLOR_THICKNESS, 60, 80);
  
  for(CRGB & pixel : leds_all) { pixel.fadeToBlackBy(32); }
  
  for(uint8_t chan = 0; chan < NUM_CHANNELS; chan++) {
    CRGB color = current_palette[chan];
    color.maximizeBrightness();
    color %= 128; // half brightness
    uint16_t scaling = freq_smooth[chan];
    
    uint8_t band_height = color_thickness * scaling / 255;
    int16_t min_pixel = FIRST_BAND_OFFSET + chan * BAND_LENGTH - band_height/2;
    uint16_t max_pixel = FIRST_BAND_OFFSET + chan * BAND_LENGTH + band_height/2 - 1;
    if(min_pixel < 0) { min_pixel = 0; }
    if(max_pixel >= LEDS_PER_RING) { max_pixel = LEDS_PER_RING-1; }
    
    for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
      if(node_number*RINGS_PER_NODE + ring < min_ring) { continue; }
      if(node_number*RINGS_PER_NODE + ring >= max_ring) { continue; }
      
      for(uint16_t pixel = min_pixel; pixel <= max_pixel; pixel++) {
        uint16_t index = get_1d_index(ring, pixel);
        if(leds[index] == CRGB(0,0,0)) {
          //if(ring == 0) { Serial.println(String(pixel) + " = color"); }
          leds[index] = color;
        }
        else {
          tjp_nblend(leds[index], color, 128);
          //if(ring == 0) { Serial.println(String(pixel) + " = blend"); }
        }
      }
    }
  }
}


//------------------------------------------- UNDER DEVELOPMENT --------------------------------------------
//--------------------- Only move above this line when code has been thoroughly tested ---------------------
//--------------------- Only include in allowable animations when moved above this line --------------------
//----------------------------------------------------------------------------------------------------------


//---------- KILL_ANIMATION ------------
inline void kill_animation() {
  const uint8_t max_kill_range = 6;
  const uint8_t throttle = 3;
  
  uint8_t kill_range = (loop_count/throttle) % (max_kill_range+1);

  for(int8_t ring = ART_CAR_RING - kill_range; ring <= ART_CAR_RING + kill_range; ring++) {
    if(ring < 0) { ring += NUM_RINGS; }
    if(ring < node_number*RINGS_PER_NODE) { continue; }
    if(ring >= (node_number+1)*RINGS_PER_NODE) { continue; }
    
    uint8_t node_ring = ring - node_number*RINGS_PER_NODE;
    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      set_led(node_ring, pixel, CRGB::Black);
    }
  }
}

//---------- CHAKRA_PULSE ------------
inline void chakra_pulse() {
  const uint8_t max_pulse_range = 6;
  const uint8_t throttle = 3;
  const uint8_t max_blend_amount = 128;
  
  uint8_t pulse_range = (loop_count/throttle) % (2*max_pulse_range);
  if(pulse_range >= max_pulse_range) { pulse_range = 2*max_pulse_range - pulse_range; }

  for(int8_t ring = ART_CAR_RING - pulse_range; ring <= ART_CAR_RING + pulse_range; ring++) {
    if(ring < 0) { ring += NUM_RINGS; }
    if(ring < node_number*RINGS_PER_NODE) { continue; }
    if(ring >= (node_number+1)*RINGS_PER_NODE) { continue; }

    uint8_t node_ring = ring - node_number*RINGS_PER_NODE;
    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      tjp_nblend(leds[get_1d_index(node_ring, pixel)], current_palette[2], max_blend_amount*(1+pulse_range)/(1+max_pulse_range));
    }
  }
}


//---------- ANIMATION_PULSE ------------
inline void animation_pulse() {
  const uint8_t range = 6;

  uint8_t fadeAmount = (freq_external[0] / 2) + (freq_external[1] / 2);

  for(int8_t ring = ART_CAR_RING - range; ring <= ART_CAR_RING + range; ring++) {
    if(ring < 0) { ring += NUM_RINGS; }
    if(ring < node_number*RINGS_PER_NODE) { continue; }
    if(ring >= (node_number+1)*RINGS_PER_NODE) { continue; }

    uint8_t node_ring = ring - node_number*RINGS_PER_NODE;
    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      leds[get_1d_index(node_ring, pixel)].fadeToBlackBy(fadeAmount);
    }
  }
}


//---------- LIGHTNING ------------
inline void lightning() {
  const uint8_t spark_range = 40;
  const uint8_t min_length = 20;
  const uint8_t range = 6;
  const uint8_t portion = 9;
  const uint8_t blend_amount = 160;

  for(int8_t ring = ART_CAR_RING - range; ring <= ART_CAR_RING + range; ring++) {
    if(ring < 0) { ring += NUM_RINGS; }
    if(ring < node_number*RINGS_PER_NODE) { continue; }
    if(ring >= (node_number+1)*RINGS_PER_NODE) { continue; }
    uint8_t node_ring = ring - node_number*RINGS_PER_NODE;

    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      leds[get_1d_index(node_ring, pixel)].fadeToBlackBy(32); // Dim by 1/8
    }

    if(random8(portion) == 0) {
      uint8_t start_point = (HALF_RING - 1 - random8(spark_range));
      uint8_t length = min_length + random8(start_point - min_length + 1);
      uint8_t bolt_ring = node_ring;
      for(int16_t pixel = start_point; pixel >= start_point - length; pixel--) {
        uint8_t change_ring = random(8);
        if(change_ring == 0) { if(bolt_ring != RINGS_PER_NODE-1) { bolt_ring++; } }
        else if(change_ring == 1) { if(bolt_ring != 0) { bolt_ring--; } }

        uint8_t brightness = 255 * length / HALF_RING;
        tjp_nblend(leds[get_1d_index(bolt_ring, pixel)], CRGB(brightness, brightness, brightness), blend_amount);
      }
    }

    if(random8(portion) == 0) {
      uint8_t start_point = (HALF_RING + random8(spark_range));
      uint8_t length = min_length + random8(LEDS_PER_RING - start_point - min_length);
      uint8_t bolt_ring = node_ring;
      for(int16_t pixel = start_point; pixel < start_point + length; pixel++) {
        uint8_t change_ring = random(8);
        if(change_ring == 0) { if(bolt_ring != RINGS_PER_NODE-1) { bolt_ring++; } }
        else if(change_ring == 1) { if(bolt_ring != 0) { bolt_ring--; } }

        uint8_t brightness = 255 * length / HALF_RING;
        tjp_nblend(leds[get_1d_index(bolt_ring, pixel)], CRGB(brightness, brightness, brightness), blend_amount);
      }
    }
  }
}


//---------- FIRE_HELLO ------------
inline void fire_hello() {
  if(node_number == ART_CAR_RING / RINGS_PER_NODE) {
    fire(FIRE_PALETTE_STANDARD, false, node_number*RINGS_PER_NODE, (node_number+1)*RINGS_PER_NODE);
  }
}



