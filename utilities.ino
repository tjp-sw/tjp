void overlay() {

  // copy mid layer on top of existing background layer
//  for  (int ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
//    for (int pixel = 0; pixel < LEDS_PER_RING; pixel++) {
//      if (mid_is_set[ring][pixel]) {
//        mid[ring][pixel] = mid[ring][pixel];
//      }
//    }
//  }

  
  // copy sparkle layer on top of existing led layer
  for  (int ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
    for (int pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      if (sparkle_is_set[ring][pixel]) {
        leds_raw[get_idx(ring, pixel)] = sparkle[ring][pixel];
      }
    }
  }
}


//  palette_num from 0 to 2 indicates which of the 3 palettes we're going to use for this animation
//  color_index from 0 to 6 indicates which of the 7 colors in that palette to use
CRGB get_color(int palette_num, int color_index) {
  color_index = color_index % show_parameters[NUM_COLORS_INDEX];
  
  switch (palette_num)  {   
  case 0:
    return icy_bright[color_index];

  case 1:
    return watermelon[color_index];

  case 2:
    return fruit_loop[color_index];

  default:
    return fruit_loop[color_index];
  }
}

// rgb2hsv() is slow, if we start using HSV colors more we should switch to HSVs in the palettes
CHSV get_color_hsv(int palette_num, int color_index) {
  return rgb2hsv_approximate(get_color(palette_num, color_index));
}

CRGB get_random_palette_color() {
  return get_color(show_parameters[PALETTE_INDEX], random8(0, 7));
}

uint16_t get_idx(uint8_t ring, uint16_t pixel) {
  // pixel assumes 0 -> 407, lowest being the inside
  // ring starts at 12 o'clock and moves clockwise
  uint8_t node = ring / RINGS_PER_NODE;
  ring -= node*RINGS_PER_NODE;
  
  if(ring < (RINGS_PER_NODE/2)) {
    uint8_t strand = ring % 2;
    uint16_t strandOffset = LEDS_PER_STRIP - 1 - PHYSICAL_LEDS_PER_RING*(ring/2) - pixel;  
    return LEDS_PER_STRIP * strand + strandOffset;
  }
  else {
    uint8_t strand = 2 + ring % 2;
    ring -= (RINGS_PER_NODE/2);
    uint16_t strandOffset = 1 + PHYSICAL_LEDS_PER_RING*(ring/2) + pixel;
    return strand*LEDS_PER_STRIP + strandOffset;
  }
}

