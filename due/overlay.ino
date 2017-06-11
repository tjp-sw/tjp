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

