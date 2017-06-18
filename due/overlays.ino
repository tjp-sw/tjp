
void ScrollingDim(uint8_t nDark, uint8_t nBright) {
  const uint8_t period = 7 + nDark + 7 + nBright;
  const uint16_t extendedLEDCount = ((LEDS_PER_RING-1)/period+1)*period;
  
  uint16_t i = 0;
  while(i < extendedLEDCount) {
    // progressively dim the LEDs
    for(uint8_t j = 0; j < period - nBright; j++) {
      uint16_t idx = (i + loop_count) % extendedLEDCount;
      if(i++ >= extendedLEDCount) { return; }
      if(idx >= LEDS_PER_RING) continue;

      if(j <= 7) {
        for(uint8_t ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
          leds[ring][idx].r >>= j;
          leds[ring][idx].g >>= j;
          leds[ring][idx].b >>= j;
        }
      }
      else if(j <= 7 + nDark) {
        for(uint8_t ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
          leds[ring][idx] = CRGB::Black;
        }
      }
      else if(j <= 7 + nDark + 7) {
        for(uint8_t ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
          leds[ring][idx].r >>= 7 + nDark + 7 - j;
          leds[ring][idx].g >>= 7 + nDark + 7 - j;
          leds[ring][idx].b >>= 7 + nDark + 7 - j;
        }
      }
    }
    
    // skip over nBright LEDs to leave them at full brightness
    i += nBright;
  }
}

void ScrollingSaturation(uint8_t nWhite, uint8_t nTrans, uint8_t nPure) {
  const uint8_t maxBlendAmount = 160;
  const uint8_t whiteBrightness = 210;

  uint8_t period = nWhite + nPure + 2*nTrans;
  uint8_t extendedLEDCount = ((LEDS_PER_RING-1)/period+1)*period;
  CRGB myWhite = CRGB(whiteBrightness, whiteBrightness, whiteBrightness);
  uint8_t i = 0;
  while(i < extendedLEDCount) {
    // progressively dim the LEDs
    for(uint16_t j = 0; j < period - nPure; j++) {
      uint16_t idx = (i + loop_count) % extendedLEDCount;
      if(i++ >= extendedLEDCount) { return; }
      if(idx >= LEDS_PER_RING) continue;

      for(uint8_t ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
        if(j < nTrans)
          nblend(&leds[ring][idx], &myWhite, 1, maxBlendAmount * j / nTrans);
        else if(j < nTrans + nWhite)
          nblend(&leds[ring][idx], &myWhite, 1, maxBlendAmount);
        else if(j < 2*nTrans + nWhite)
          nblend(&leds[ring][idx], &myWhite, 1, maxBlendAmount * (2*nTrans + nWhite - j) / nTrans);
      }
    }
    
    // skip over nPure LEDs to leave them at full saturation
    i += nPure;
  }
}

