void overlay() {

  // copy mid layer on top of existing background layer
//  for  (int ring = 0; ring < RINGS_PER_NODE; ring++) {
//    for (int pixel = 0; pixel < LEDS_PER_RING; pixel++) {
//      if (mid_is_set[ring][pixel]) {
//        leds[ring][pixel] = mid[ring][pixel];
//      }
//    }
//  }

  
  // copy sparkle layer on top of existing led layer
  for  (int ring = 0; ring < RINGS_PER_NODE; ring++) {
    for (int pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      if (sparkle_is_set[ring][pixel]) {
        leds[ring][pixel] = sparkle[ring][pixel];
      }
    }
  }
}


//  palette_num from 0 to 2 indicates which of the 3 palettes we're going to use for this animation
//  color_index from 0 to 6 indicates which of the 7 colors in that palette to use
CRGB get_color(int palette_num, int color_index) {

  switch (palette_num)  {   
  case 0:
    return icy_bright[color_index];

  case 1:
    return watermelon[color_index];

  case 2:
    return fruit_loop[color_index];
  }
}

// rgb2hsv() is slow, if we start using HSV colors more we should switch to HSVs in the palettes
CHSV get_color_hsv(int palette_num, int color_index) {
  return rgb2hsv_approximate(get_color(palette_num, color_index));
}

CRGB get_random_palette_color() {
  // To do
}

