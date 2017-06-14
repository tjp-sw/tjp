// Adapted from:
// Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY

void Fire() {
const uint8_t sparkSize_min = 170;
const uint8_t sparkSize_max = 255;

  
// Array of temperature readings at each simulation cell
static byte heat[RINGS_PER_NODE][VISIBLE_LEDS_PER_RING];

  // Step 1.  Cool down every cell a little
    for(int i = 0; i < RINGS_PER_NODE; i++) {
      // Roll a range of cooling through the structure
      int fireCooling = 2 + (i + loop_count) % 110;
      
      for(uint16_t j = 0; j < HALF_VISIBLE; j++) {
        heat[i][j] = qsub8(heat[i][j],  random8(0, fireCooling));
        heat[i][HALF_VISIBLE + j] = qsub8(heat[i][HALF_VISIBLE + j],  random8(0, fireCooling));
      }
    }

    for(int i = 0; i < RINGS_PER_NODE; i++) {
      // Step 2.  Heat from each cell drifts 'up' and diffuses a little
      for(int k = 1; k < HALF_VISIBLE - 1; k++) {
        // Inner half
        heat[i][HALF_VISIBLE - k] = (heat[i][HALF_VISIBLE - k] + heat[i][HALF_VISIBLE - k - 1] + heat[i][HALF_VISIBLE - k - 2]) / 2;

        // Outer half
        heat[i][HALF_VISIBLE + k - 1] = (heat[i][HALF_VISIBLE + k - 1] + heat[i][HALF_VISIBLE + k] + heat[i][HALF_VISIBLE + k + 1]) / 2;
      }
    
      // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
      // Roll a range of higher sparking percent through the structure, at 1/7 the rate of the cooling
      int fireSparking = 60;//(i + RINGS_PER_NODE / 2 + loop_count/7) % 230 + 25; // 10% to 100% chance of sparking
      
      if(random8() < fireSparking) {
        int y = random8(7);
        heat[i][y] = qadd8(heat[i][y], random8(sparkSize_min, sparkSize_max));
      }
  
      if(random8() < fireSparking) {
        int y = VISIBLE_LEDS_PER_RING - 1 - random8(7);
        heat[i][y] = qadd8(heat[i][y], random8(sparkSize_min, sparkSize_max));
      }
  
      // Step 4.  Map from heat cells to LED colors
      for(int j = 0; j < VISIBLE_LEDS_PER_RING; j++) {
          leds[i][j] = GetHeatColor(heat[i][j]);
      }
    }
}

// CRGB GetHeatColor( uint8_t temperature)
// [to be included in the forthcoming FastLED v2.1]
//
// Approximates a 'black body radiation' spectrum for 
// a given 'heat' level.  This is useful for animations of 'fire'.
// Heat is specified as an arbitrary scale from 0 (cool) to 255 (hot).
// This is NOT a chromatically correct 'black body radiation' 
// spectrum, but it's surprisingly close, and it's extremely fast and small.
//
// On AVR/Arduino, this typically takes around 70 bytes of program memory, 
// versus 768 bytes for a full 256-entry RGB lookup table.

CRGB GetHeatColor(uint8_t temperature) {
  CRGB heatcolor;
  
  // Scale 'heat' down from 0-255 to 0-191,
  // which can then be easily divided into three
  // equal 'thirds' of 64 units each.
  uint8_t t192 = scale8_video(temperature, 192);

  // calculate a value that ramps up from
  // zero to 255 in each 'third' of the scale.
  uint8_t heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2; // scale up to 0..252
 
  // now figure out which third of the spectrum we're in:
  if(t192 & 0x80) {
    // we're in the hottest third
    heatcolor.r = 255; // full red
    heatcolor.g = 255; // full green
    heatcolor.b = heatramp; // ramp up blue
    
  }
  else if(t192 & 0x40 ) {
    // we're in the middle third
    heatcolor.r = 255; // full red
    heatcolor.g = heatramp; // ramp up green
    heatcolor.b = 0; // no blue
    
  }
  else {
    // we're in the coolest third
    heatcolor.r = heatramp; // ramp up red
    heatcolor.g = 0; // no green
    heatcolor.b = 0; // no blue
  }
  
  return heatcolor;
}


