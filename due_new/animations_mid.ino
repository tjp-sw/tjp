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
      uint16_t idx;
      if(MID_INTRA_RING_MOTION != SPLIT) {
        idx = (pixel + MID_RING_OFFSET*ring + MID_INTRA_RING_MOTION * MID_INTRA_RING_SPEED * loop_count / THROTTLE) % extended_led_count;
      }
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
// The palette_type parameter changes how the heat scale is constructed. Setting to 0 will skip this step and just use the actual palette.
// To do: add a cooling wind that rolls around structure: MID_INTER_NODE_SPEED == wind speed, MID_RING_OFFSET == wind decay amount in neighboring rings
// To do: on inner part of structure, allow heat to diffuse from neighboring rings.
// MID_BLACK_THICKNESS(20-80) == cooling, MID_COLOR_THICKNESS(40-225) == sparking chance, MID_NUM_COLORS(100-255) == minimum spark size
void set_fire_palette() {
  // change mid_palette to heat scale
/*  CHSV tp0 = rgb2hsv_approximate(target_palette[2]);
  CHSV tp1 = rgb2hsv_approximate(target_palette[3]);
  CHSV tp2 = rgb2hsv_approximate(target_palette[4]);

  CHSV cp0 = rgb2hsv_approximate(current_palette[2]);
  CHSV cp1 = rgb2hsv_approximate(current_palette[3]);
  CHSV cp2 = rgb2hsv_approximate(current_palette[4]);

  Serial.print("rgb[0] = (" + String(current_palette[2].r) + "," + String(current_palette[2].g) + "," + String(current_palette[2].b) + ")");
  Serial.println("\t\thsv[0] = (" + String(cp0.h) + "," + String(cp0.s) + "," + String(cp0.v) + ")");
  Serial.print("rgb[1] = (" + String(current_palette[3].r) + "," + String(current_palette[3].g) + "," + String(current_palette[3].b) + ")");
  Serial.println("\t\thsv[1] = (" + String(cp1.h) + "," + String(cp1.s) + "," + String(cp1.v) + ")");
  Serial.print("rgb[2] = (" + String(current_palette[4].r) + "," + String(current_palette[4].g) + "," + String(current_palette[4].b) + ")");
  Serial.println("\t\thsv[2] = (" + String(cp2.h) + "," + String(cp2.s) + "," + String(cp2.v) + ")");
  Serial.println();
  */

  CRGB steps[3];
  steps[0] = current_palette[2];//CHSV(cp0.h, 255, 100);
  steps[1] = current_palette[3];//CHSV(cp1.h, 180, 150);
  steps[2] = current_palette[4];//CHSV(cp2.h, 140, 255);

  steps[0].maximizeBrightness();
  steps[1].maximizeBrightness();

  steps[0] %= 75; // Set to 75/255 % of max brightness
  steps[1] %= 150;// Set to 150/255 % of max brightness

  while(steps[0].r > 5 && steps[0].g > 5 && steps[0].b > 5) { steps[0]--; } // Manually increase saturation
  //while(steps[2].r < 255 && steps[2].g < 255 && steps[3].b < 255) { steps[2]++; }
  steps[2] |= CRGB(40, 40, 40); // Raise each channel to a minimum of 60
  steps[2].maximizeBrightness();

  CRGB start, target;

  // Step 1: Black to dim, saturated color0
  target = steps[0];
  for(uint8_t i = 0; i < 90; i++) {
    start = CRGB::Black;
    nblend(start, target, 255 * i / 90);
    mid_palette.entries[i] = start;
  }

  // Step 2: color0 to color1, up value and drop saturation
  target = steps[1];
  for(uint8_t i = 90; i < 160; i++) {
    start = steps[0];
    nblend(start, target, 255 * (i-90) / 70);
    mid_palette.entries[i] = start;
  }

  // Step 3: color1 to color2, up value and drop saturation
  target = steps[2];
  for(uint8_t i = 160; i < 225; i++) {
    start = steps[1];
    nblend(start, target, 255 * (i-160) / 65);
    mid_palette.entries[i] = start;
  }

  // Step 4: blend into fully bright white
  target = CRGB(255, 255, 255);
  for(uint16_t i = 225; i < 256; i++) {
    start = steps[2];
    nblend(start, target, 255 * (i-225) / 31);
    mid_palette.entries[i] = start;
  }
}

void cleanup_fire() {
  // change mid_palette from heat scale back to normal palette with dimming and gradients
  create_mid_palette(&mid_palette, current_palette[2], current_palette[3], current_palette[4]);
}

#define SPARKING_RANGE 20
void fire(uint8_t palette_type) {
  static bool last_cycle_was_blending = true;
  if(palette_type != 0) {
    if(last_cycle_was_blending || blend_mid_layer) {
      if(palette_type == 1) { set_fire_palette(); }
    }
    
    last_cycle_was_blending = blend_mid_layer;
  }
    
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

