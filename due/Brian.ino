// Adapted from:
// Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
void Fire() {
  const uint8_t sparkSize_min = 170;
  const uint8_t sparkSize_max = 255;
  
    
  // Array of temperature readings at each simulation cell
  static byte heat[RINGS_PER_NODE][LEDS_PER_RING];

  // Step 1.  Cool down every cell a little
    for(int i = 0; i < RINGS_PER_NODE; i++) {
      // Roll a range of cooling through the structure
      int fireCooling = 15 + (i + loop_count) % 72;
      
      for(uint16_t j = 0; j < HALF_RING; j++) {
        heat[i][j] = qsub8(heat[i][j],  random8(0, fireCooling));
        heat[i][HALF_RING + j] = qsub8(heat[i][HALF_RING + j],  random8(0, fireCooling));
      }
    }

    for(int i = 0; i < RINGS_PER_NODE; i++) {
      // Step 2.  Heat from each cell drifts 'up' and diffuses a little
      for(int k = 1; k < HALF_RING - 1; k++) {
        // Inner half
        heat[i][HALF_RING - k] = (heat[i][HALF_RING - k] + heat[i][HALF_RING - k - 1] + heat[i][HALF_RING - k - 2]) / 2;

        // Outer half
        heat[i][HALF_RING + k - 1] = (heat[i][HALF_RING + k - 1] + heat[i][HALF_RING + k] + heat[i][HALF_RING + k + 1]) / 2;
      }
    
      // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
      // Roll a range of higher sparking percent through the structure, at 1/7 the rate of the cooling
      int fireSparking = 70;//(i + RINGS_PER_NODE / 2 + loop_count/7) % 230 + 25; // 10% to 100% chance of sparking
      
      if(random8() < fireSparking) {
        int y = random8(7);
        heat[i][y] = qadd8(heat[i][y], random8(sparkSize_min, sparkSize_max));
      }
  
      if(random8() < fireSparking) {
        int y = LEDS_PER_RING - 1 - random8(7);
        heat[i][y] = qadd8(heat[i][y], random8(sparkSize_min, sparkSize_max));
      }
  
      // Step 4.  Map from heat cells to LED colors
      for(int j = 0; j < LEDS_PER_RING; j++) {
          leds[i][j] = GetHeatColor(heat[i][j]);
      }
    }
}

// Approximates a 'black body radiation' spectrum for a given 'heat' level.
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


// Frequency Pulse
// 7 bands of LEDs, mirrored on each side of the ring. Value of each band scales from 30% to 100% brightness based on the spectrum shield's outputs.
// Waves of whiteness move along the first dimension and bounce off each other along with the beat
//
#define PIXELS_PER_BAND 30
#define MIN_VAL_FP 64 // Scaled from 0-255, currently 25%

void frequency_pulse() {
  uint8_t white_ring = RINGS_PER_NODE * downbeat_proximity / 255;
  if(node_number % 2 == 1)
    white_ring = RINGS_PER_NODE - 1 - white_ring;

  
  uint8_t inner_start, inner_end, outer_start, outer_end;
  
  for(uint8_t j = 0; j < NUM_CHANNELS-1; j++) {
    inner_start = PIXELS_PER_BAND * j;
    inner_end = PIXELS_PER_BAND * (j+1) - 1;
    outer_start = LEDS_PER_RING - PIXELS_PER_BAND * j;
    outer_end = LEDS_PER_RING - PIXELS_PER_BAND * (j+1) - 1;

    CHSV tempColor = get_color_hsv(show_parameters[PALETTE_INDEX], j % show_parameters[NUM_COLORS_INDEX]);
    tempColor.value = MIN_VAL_FP + frequencies_max[j] * (255-MIN_VAL_FP) / 255;

    CHSV whiteColor = tempColor;
    whiteColor.saturation = qsub8(whiteColor.saturation, 128);

    CHSV midColor = tempColor;
    midColor.saturation = qsub8(whiteColor.saturation, 64);
    
    for(uint8_t i = 0; i < RINGS_PER_NODE; i++) {
      if(i == white_ring) {
        leds[i](inner_start, inner_end) = whiteColor;
        leds[i](outer_start, outer_end) = whiteColor;
      }
      else if(i == white_ring - 1 || i == white_ring + 1 || (i == 0 && white_ring == RINGS_PER_NODE-1) || (i == RINGS_PER_NODE-1 && white_ring == 0)){
        leds[i](inner_start, inner_end) = midColor;
        leds[i](outer_start, outer_end) = midColor;
      }
      else {
        leds[i](inner_start, inner_end) = tempColor;
        leds[i](outer_start, outer_end) = tempColor;
      }
    }
  }
  
  // Draw center band (using indexes from last iteration)
  CHSV tempColor = get_color_hsv(show_parameters[PALETTE_INDEX], (NUM_CHANNELS-1) % show_parameters[NUM_COLORS_INDEX]);
  tempColor.value = MIN_VAL_FP + frequencies_max[NUM_CHANNELS-1] * (255-MIN_VAL_FP) / 255;
  
  for(uint8_t i = 0; i < RINGS_PER_NODE; i++) {
    leds[i](inner_end+1, outer_start-1) = tempColor;
  }
}


// To do: increase velocity every 30 pixels or so, to get an acceleration effect
// Collision
// Comets fly from base of each ring and collide in center with a flash of white
// Comets trigger every NUM_RINGS cycles, 1 cycle delayed for each ring. This will result in a single ring of white moving through the structure
void collision()
{
  const uint8_t fadeRate = 48;
  const uint8_t fadeCycles = 6; // How many cycles to fade from white to black after collision; will also be the tail length
  const uint8_t patternTime = HALF_RING + 1 + fadeCycles;
  
  static CRGB curColor[NUM_RINGS][patternTime/NUM_RINGS];  // colors of the current comets

  leds_all.fadeToBlackBy(fadeRate);
  for(uint8_t ring = node_number * RINGS_PER_NODE; ring < (node_number + 1) * RINGS_PER_NODE; ring++)
  {
    // Start up with only comets from bottom
    if(loop_count < ring)
      break;
      
    uint8_t ring_offset = (loop_count - ring) % NUM_RINGS; // Each comet on this ring has moved ring_offset pixels from its starting point
    
    if(ring_offset == 0) {
      // Move comets to correct indexes, spawn new comet at [0]
      for(uint8_t i = patternTime/NUM_RINGS - 1; i > 0; i++)
      {
          curColor[ring][i] = curColor[ring][i-1];
      }
      curColor[ring][0] = curColor[ring][0] = get_random_palette_color();
    }
    

    bool explodeRing = false;
    for(uint8_t iComet = patternTime/NUM_RINGS - 1; iComet >= 0; iComet--) // From highest numbered comet to lowest
    {
      // Start up with only comets from bottom
      if(loop_count < ring + iComet*NUM_RINGS)
        continue;

      uint8_t comet_location = NUM_RINGS*iComet + ring_offset; // The position of the current comet

      if(comet_location >= HALF_RING) {
          explodeRing = true;
          break; // No need to draw further comets on this ring; they will be overwritten
      }


      leds[ring][comet_location] = CRGB::White;
      leds[ring][LEDS_PER_RING-1 - comet_location] = CRGB::White;

      if(comet_location >= 1) {
        leds[ring][comet_location - 1] = CRGB::White;
        leds[ring][LEDS_PER_RING-1 - comet_location + 1] = CRGB::White;
      }
      
      if(comet_location >= 2) {
        leds[ring][comet_location - 2] = curColor[ring][iComet];
        leds[ring][LEDS_PER_RING-1 - comet_location + 2] = curColor[ring][iComet];
      }
    }
    
    if(explodeRing) {
      leds[ring] = CRGB::White; //flash white
    }
  }
}

// Variable spin
// Each ring rotates one block of 3 pixels at different rates.
// Goal is to find cool periods that result in several unique alignments forming
//
void variable_spin() {
  // 6 spin rates, centered at the middle of each node, so node edges line up with each other.
  // First half of node spins one way, second half the other way. Reverse direction on odd numbered nodes so node edges line up.
  const uint8_t spinRates[6] = { 2, 3, 4, 6, 8, 12 };
  static uint16_t centerPoints[12] = { HALF_RING, HALF_RING, HALF_RING, HALF_RING, HALF_RING, HALF_RING,
                                       HALF_RING, HALF_RING, HALF_RING, HALF_RING, HALF_RING, HALF_RING };
  CRGB colors[6];

  for(uint8_t i = 0; i < 6; i++)
  {
    colors[i] = get_color(show_parameters[PALETTE_INDEX], i);
    
    if(loop_count % spinRates[i] == 0) {
      if(centerPoints[i] == 0) centerPoints[i] = LEDS_PER_RING - 1;
      else centerPoints[i]--;

      if(centerPoints[11 - i] == LEDS_PER_RING - 1) centerPoints[11 - i] = 0;
      else centerPoints[11 - i]++;
    }
  }

  leds_all = CRGB::Black;
  for(uint8_t ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
    uint16_t centerPoint = centerPoints[ring];

    leds[ring][centerPoint] = colors[ring < 6 ? ring : 11 - ring];
    if(centerPoint == 0)
      leds[ring][LEDS_PER_RING - 1] = colors[ring < 6 ? ring : 11 - ring];
    else
      leds[ring][centerPoint-1] = colors[ring < 6 ? ring : 11 - ring];

    if(centerPoint == LEDS_PER_RING - 1)
      leds[ring][0] = colors[ring < 6 ? ring : 11 - ring];
    else
      leds[ring][centerPoint+1] = colors[ring < 6 ? ring : 11 - ring];
  }
}


// Spinning dots illusion
// Fixed color (pink), rotate a black pixel through each ring. It will appear green when you stare straight ahead?
// 

// My take on the equalizer
// Inner ring is left audio channel, outer ring is right audio channel
// Use a global fade to smoothly dim after sudden spikes
// Center of ring is the lowest band, other 6 bands occupy 2 rings each
void equalizer3() {
  const uint8_t centerBandMinWidth = 10; // Keep this value even
  const uint8_t centerBandMaxWidth = 40; // Keep this value even
  const uint8_t minHeight = 20;
  const uint8_t maxHeight = HALF_RING - centerBandMaxWidth/2;
  const uint8_t fadeRate = 64;

/*
  // Moving average stuff
  const uint8_t windowSize = 4;  
  static int innerReads[NUM_CHANNELS][windowSize];
  static int outerReads[NUM_CHANNELS][windowSize];
*/

  leds_all.fadeToBlackBy(fadeRate);

  CHSV centerColor = get_color_hsv(show_parameters[PALETTE_INDEX], 6);
  centerColor.saturation = 255 - frequencies_max[6];

  uint8_t centerBandWidth = centerBandMinWidth + (centerBandMaxWidth - centerBandMinWidth) * downbeat_proximity / 255;
  if(centerBandWidth % 2 == 1)
    centerBandWidth++;
  
  for(uint8_t ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++)
  {
    uint8_t channel = ring/2;
    CRGB thisColor = get_color(show_parameters[PALETTE_INDEX], channel);
    uint8_t innerHeight, outerHeight;
    innerHeight = minHeight + (maxHeight - minHeight) * frequencies_one[channel] / 255;
    outerHeight = minHeight + (maxHeight - minHeight) * frequencies_two[channel] / 255;

    leds[ring](0, innerHeight) = thisColor;
    leds[ring](LEDS_PER_RING - 1, LEDS_PER_RING - 1 - outerHeight) = thisColor;
    leds[ring](HALF_RING - 1 - (centerBandWidth-1)/2, HALF_RING - 1 + centerBandWidth/2) = centerColor;
  }
}

// Gradient from one color to the next, overlay=1 adds scrolling dim, 2 adds scrolling saturation
void ScrollingGradient_TwoColor(uint8_t overlay) {
  const uint8_t period = 32;
  const uint8_t paletteWidth = 17;
  const uint8_t extendedLEDCount = ((NUM_LEDS-1)/period+1)*period;
  const uint8_t dim_fullDarkLEDs = 8;
  const uint8_t dim_fullBrightLEDs = 1;
  const uint8_t sat_fullWhiteLEDs = 1;
  const uint8_t sat_transitionLength = 6;
  const uint8_t sat_fullSatLEDs = 12;

  CRGB col1 = get_color(show_parameters[PALETTE_INDEX], 0);
  CRGB col2 = get_color(show_parameters[PALETTE_INDEX], show_parameters[NUM_COLORS_INDEX]-1);
  
  for(uint16_t i = 0; i < extendedLEDCount; i++) {
    uint8_t idx = (loop_count/2 + i) % extendedLEDCount;
    if(idx >= NUM_LEDS) continue;
    
    uint8_t blendAmount;
    if(i % period < period/2)
      blendAmount = (i % (period/2)) * 255 / (period/2);
    else
      blendAmount = 255 - (i % (period/2)) * 255 / (period/2);

    CRGB temp = col1;
    nblend(&temp, &col2, 1, blendAmount);
    for(uint8_t ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
      leds[ring][idx] = temp;
    }
  }

  if(overlay == 1) ScrollingDim(dim_fullDarkLEDs, dim_fullBrightLEDs);
  else if(overlay == 2) ScrollingSaturation(sat_fullWhiteLEDs, sat_transitionLength, sat_fullSatLEDs);
}

