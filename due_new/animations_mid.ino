// This file is for completed mid animations

//-------------------------------- SNAKE --------------------------------
// Sends alternating bands of colors rotating around the rings
// Creates repeated snakes with NUM_COLORS colors, with each color repeated COLOR_THICKNESS times, separated by BLACK_THICKNESS black LEDs.
// MID_NUM_COLORS(1-3), MID_COLOR_THICKNESS(1-20), MID_BLACK_THICKNESS(0-60), MID_INTRA_RING_MOTION(-1, 1), MID_RING_OFFSET(0-20), MID_INTRA_RING_SPEED(8-127)
void snake() {
  uint8_t period = MID_NUM_COLORS * MID_COLOR_THICKNESS + MID_BLACK_THICKNESS;
  uint16_t extended_led_count = ((LEDS_PER_RING-1)/period+1)*period;
  
  for (uint8_t ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
    for (uint16_t pixel = 0; pixel < extended_led_count; pixel++) {
      uint16_t idx = (pixel + MID_RING_OFFSET*ring + MID_INTRA_RING_MOTION * MID_INTRA_RING_SPEED * mid_count / THROTTLE) % extended_led_count;
      if(idx >= LEDS_PER_RING) { continue; }

      uint8_t pattern_idx = pixel % period;
      if(pattern_idx < MID_BLACK_THICKNESS) {
        mid_layer[ring][idx] = TRANSPARENT;
      }
      else if(pattern_idx < MID_BLACK_THICKNESS + MID_COLOR_THICKNESS) {
        mid_layer[ring][idx] = get_mid_color(0);
      }
      else if(pattern_idx < MID_BLACK_THICKNESS + 2*MID_COLOR_THICKNESS) {
        mid_layer[ring][idx] = get_mid_color(1);
      }
      else {
        mid_layer[ring][idx] = get_mid_color(2);
      }
    }
  }
}


//-------------------------------- FIRE ---------------------------------
// Adapted from: Fire2012 by Mark Kriegsman, July 2012, part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
// Changes palette to a heat scale, and values in mid_layer represent the amount of heat in each pixel
// To do: add a cooling wind that rolls around structure: MID_INTER_NODE_SPEED == wind speed, MID_RING_OFFSET == wind decay amount in neighboring rings
// To do: on inner part of structure, allow heat to diffuse from neighboring rings.
// MID_BLACK_THICKNESS(20-80) == cooling, MID_COLOR_THICKNESS(40-225) == sparking chance, MID_NUM_COLORS(100-255) == minimum spark size
// Recommended settings: MID_BLACK_THICKNESS=20, MID_COLOR_THICKNESS=220, NUM_COLORS=60, 
void init_fire() { init_fire(true); }
void init_fire(bool reset_heat) {
  // change mid_palette to heat scale
  CHSV mp0 = rgb2hsv_approximate(mid_palette_target.entries[get_mid_color(0)]);
  CHSV mp1 = rgb2hsv_approximate(mid_palette_target.entries[get_mid_color(1)]);
  CHSV mp2 = rgb2hsv_approximate(mid_palette_target.entries[get_mid_color(2)]);

  // Sort by saturation
  if(mp0.s < mp1.s) {
    CHSV swap = mp0;
    mp0 = mp1;
    mp1 = swap;
  }
  if(mp1.s < mp2.s) {
    CHSV swap = mp1;
    mp1 = mp2;
    mp2 = swap;
  }
  if(mp0.s < mp1.s) {
    CHSV swap = mp0;
    mp0 = mp1;
    mp1 = swap;
  }

  //Serial.println("mp0 = (" + String(mp0.h) + "," + String(mp0.s) + "," + String(mp0.v) + ")");
  //Serial.println("mp1 = (" + String(mp1.h) + "," + String(mp1.s) + "," + String(mp1.v) + ")");
  //Serial.println("mp2 = (" + String(mp2.h) + "," + String(mp2.s) + "," + String(mp2.v) + ")");


  uint16_t i = 0;
  
  // Blend from black to mp0, constant hue, step saturation and value
  for(;i < 75; i++) {
    CHSV temp = CHSV(mp0.h, 255 - i*(255-mp0.s) / 75, i*3/2); // Highest value = 74*3/2 = 111
    mid_palette.entries[i] = temp;
    //Serial.println("hsv[" + String(i) + "] = (" + String(temp.h) + "," + String(temp.s) + "," + String(temp.v) + ")");
  }

  // Blend from mp0 to mp1
  bool increase_hue = increase_hue = mp1.h < (uint8_t)(mp0.h + 128);
  uint8_t distance = increase_hue ? mp1.h - mp0.h : mp0.h - mp1.h;
  
  for(;i < 150; i++) {
    CHSV temp;
    if(increase_hue) {
      temp = CHSV(mp0.h + (i-75)*distance/75, mp1.s + (150-i)*(mp0.s - mp1.s) / 75, 112 + (i-75)); // Highest value = 112 + 74 = 186
    }
    else {
      temp = CHSV(mp0.h - (i-75)*distance/75, mp1.s + (150-i)*(mp0.s - mp1.s) / 75, 112 + (i-75)); // Highest value = 112 + 74 = 186;
    }
    mid_palette.entries[i] = temp;
    //Serial.println("hsv[" + String(i) + "] = (" + String(temp.h) + "," + String(temp.s) + "," + String(temp.v) + ")");
  }

  // Blend from mp1 to mp2
  increase_hue = mp2.h < (uint8_t)(mp1.h + 128);
  distance = increase_hue ? mp2.h - mp1.h : mp1.h - mp2.h;
  
  for(;i < 225; i++) {
    CHSV temp;
    if(increase_hue) {
      temp = CHSV(mp1.h + (i-150)*distance/75, mp2.s + (225-i)*(mp1.s - mp2.s) / 75, 187 + 68*(i-150)/75); // Highest value = 187 + 68 = 255
    }
    else {
      temp = CHSV(mp1.h - (i-150)*distance/75, mp2.s + (225-i)*(mp1.s - mp2.s) / 75, 187 + 68*(i-150)/75); // Highest value = 187 + 68 = 255
    }
    mid_palette.entries[i] = temp;
    //Serial.println("hsv[" + String(i) + "] = (" + String(temp.h) + "," + String(temp.s) + "," + String(temp.v) + ")");
  }
  
  // Step up saturation over last 31 steps, full brightness
  for(;i < 256; i++) {
    CHSV temp = CHSV(mp2.h, (255-i)*mp2.s/31, 255);
    mid_palette.entries[i] = temp;
    //Serial.println("hsv[" + String(i) + "] = (" + String(temp.h) + "," + String(temp.s) + "," + String(temp.v) + ")");
  }
  
  //for(uint16_t i = 0; i < 256; i++) { Serial.println("rgb[" + String(i) + "] = (" + String(mid_palette.entries[i].r) + "," + String(mid_palette.entries[i].g) + "," + String(mid_palette.entries[i].b) + ")"); }
  
  // clear out old "heat" levels
  if(reset_heat) { clear_mid_layer(); }

  // disable automatic blending; palette changes will be immediate
  blend_mid_layer = false;
}

void cleanup_fire() {
  // change mid_palette from heat scale to normal
  //CRGB normal_colors[NUM_COLORS_PER_PALETTE] = { base_palette_target.entries[4], base_palette_target.entries[255 - 6], mid_palette_target.entries[4], mid_palette_target.entries[4 + 11*7], mid_palette_target.entries[255 - 6], sparkle_palette_target.entries[2], sparkle_palette_target.entries[9] };
  //set_palettes((uint8_t*) normal_colors);

  for(uint16_t i = 0; i < 256; i++) {
    mid_palette.entries[i] = mid_palette_target.entries[i];
  }
}

#define SPARKING_RANGE 20
void fire() {
  if(blend_mid_layer) { init_fire(false); } // Recreate fire palette when switching to a new palette
    
  // Step 1.  Cool down every 4th cell a little
  for(uint8_t ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
    // Roll a range of cooling through the structure
    int fireCooling = MID_BLACK_THICKNESS;// + (ring + base_count) % 50;
    
    for(uint16_t pixel = 0; pixel < HALF_RING; pixel+=4) {
      mid_layer[ring][pixel] = qsub8(mid_layer[ring][pixel],  random8(0, fireCooling));
      mid_layer[ring][HALF_RING + pixel] = qsub8(mid_layer[ring][HALF_RING + pixel],  random8(0, fireCooling));
    }
  }

  for(uint8_t ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
    
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for(uint16_t pixel = 1; pixel < HALF_RING - 1; pixel++) {
      // Inner half
      //mid_layer[ring][HALF_RING - pixel] = (mid_layer[ring][HALF_RING - pixel] + mid_layer[ring][HALF_RING - pixel - 1] + mid_layer[ring][HALF_RING - pixel - 2]) / 2;
      mid_layer[ring][HALF_RING - pixel] = (mid_layer[ring][HALF_RING - pixel - 1] + mid_layer[ring][HALF_RING - pixel - 2]) / 2;
      
      // Outer half
      //mid_layer[ring][HALF_RING + pixel - 1] = (mid_layer[ring][HALF_RING + pixel - 1] + mid_layer[ring][HALF_RING + pixel] + mid_layer[ring][HALF_RING + pixel + 1]) / 2;
      mid_layer[ring][HALF_RING + pixel - 1] = (mid_layer[ring][HALF_RING + pixel] + mid_layer[ring][HALF_RING + pixel + 1]) / 2;
    }

  
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    uint8_t fireSparking = MID_COLOR_THICKNESS;//(i + RINGS_PER_NODE / 2 + loop_count/7) % 230 + 25; // 10% to 100% chance of sparking
    
    if(random8() < fireSparking) {
      uint8_t pixel = random8(SPARKING_RANGE);
      mid_layer[ring][pixel] = qadd8(mid_layer[ring][pixel], random8(MID_NUM_COLORS, 255));
    }

    if(random8() < fireSparking) {
      uint16_t pixel = LEDS_PER_RING - 1 - random8(SPARKING_RANGE);
      mid_layer[ring][pixel] = qadd8(mid_layer[ring][pixel], random8(MID_NUM_COLORS, 255));
    }

    if(ring == 4) {
      for(uint16_t pix = 205; pix < 408; pix++) {
        //Serial.println("mid_layer[4][" + String(pix) + "] = " + String(mid_layer[ring][pix]));
      }
    }
  }

  for(uint8_t ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      if(mid_layer[ring][pixel] < 9) { mid_layer[ring][pixel] = TRANSPARENT; }
    }
  }
}

