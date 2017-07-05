/*
Simple test routine, it pulses with music.
 */

void pulse() {
  /*
  for(int i = 0; i < NUM_LEDS; i++)
  {
    leds_all[i].setRGB(0, 0, frequencies_one[3]);
  }
  */

  // A faster way
  for(uint8_t i = 0; i < RINGS_PER_NODE; i++)
    for(uint16_t j = 0; j < LEDS_PER_RING; j++)
    leds_raw[get_idx(i, j)] = CRGB(0, 0, frequencies_one[i%NUM_CHANNELS]);

  // This way works too
  //for(int i = 0; i < NUM_RINGS; i++)
  //  leds[i](0, LEDS_PER_RING) = temp;
}


/*
Simple test routine, it runs a red dot through the strips.
 */

void run_dot() {
  uint16_t pixelOffset = loop_count % LEDS_PER_NODE;
  if(pixelOffset > 0) leds_all(0, pixelOffset-1) = CRGB::Black;
  leds_all[pixelOffset] = CRGB::Red;
  if(pixelOffset < LEDS_PER_NODE - 1) leds_all(pixelOffset+1, LEDS_PER_NODE) = CRGB::Black;
}

void run_dot_fast() {
  uint16_t pixelOffset = (3*loop_count) % LEDS_PER_NODE;
  uint8_t ring = pixelOffset / LEDS_PER_RING;
  uint16_t pixel = pixelOffset - LEDS_PER_RING * ring;
  
  leds_all = CRGB::Black;
  leds_raw[get_idx(ring, pixel)] = CRGB::Red;
}

/*
Simple test routine, it runs a green dot through the strips.
 */

void run_dot_green() {   
  uint16_t pixelOffset = loop_count % NUM_LEDS;
  // One dot for whole structure, instead of one dot per node
  leds_all = CRGB::Black;
  leds_all[pixelOffset] = CRGB::Green;
}

