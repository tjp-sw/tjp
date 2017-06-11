//  each time a new sparkle function is called, calling function should reset sparkle_count to 0


//  overwrite current sparkle led array to black 

void sparkle_reset() {
  
  for (int ring = 0; ring < RINGS_PER_NODE; ring++) {
    for (int pixel = 0; pixel < VISIBLE_LEDS_PER_RING; pixel++) {
      leds[ring][pixel] = CRGB::Black;
      sparkle_is_set[ring][pixel] = false;
    }
  }  
}


//  Puts raindrops randomly at the top of the structure and runs them down both sides of each ring.  
//  1/portion of the leds should be lit

void sparkle_rain() {

  int ring, pixel;
  int portion = 30; // 1/portion of the structure will be covered in rain
   
  int offset = 40;
  int left_ring_top = HALF_VISIBLE - offset;
  int right_ring_top = HALF_VISIBLE + offset;
 
  // if this is the first frame for this sparkle animation, black out old sparkle leds
  if (sparkle_count == 0) {
     sparkle_reset();
  }

  // create new raindrops every "portion" cycles; may need to change this constant
  if (sparkle_count % portion == 0) {
    for (ring = 0; ring < RINGS_PER_NODE; ring++) {
      for (pixel = left_ring_top; pixel < right_ring_top; pixel++) {
        if (random(portion) == 0) {
          sparkle_is_set[ring][pixel] = true;
          sparkle[ring][pixel] = sparkle_color; // turn on 1/portion of the leds as rain
        }
      }
    }
  }

  // drop existing raindrops down 1 pixel
  for (ring = 0; ring < RINGS_PER_NODE; ring++) {

    // special case so that bottom left pixel dropping down doesn't make array index out of bounds.
    if (sparkle_is_set[ring][0] == true) {
      // black out bottom left pixel
      sparkle[ring][0] = CRGB::Black;
      sparkle_is_set[ring][0] = false;      
    }

    // general case, left side
    for (pixel = 1; pixel <= HALF_VISIBLE; pixel++) { 
      // light up next pixel down
      sparkle[ring][pixel - 1] = sparkle[ring][pixel];
      sparkle_is_set[ring][pixel - 1] = true;

      // black out current pixel
      sparkle[ring][pixel] = CRGB::Black;
      sparkle_is_set[ring][pixel] = false;
    }
    
    // right side
    for (pixel = VISIBLE_LEDS_PER_RING; pixel > HALF_VISIBLE; pixel--) { 
      sparkle[ring][pixel + 1] = sparkle[ring][pixel];
      sparkle_is_set[ring][pixel + 1] = true;
      
      // black out current pixel
      sparkle[ring][pixel] = CRGB::Black;
      sparkle_is_set[ring][pixel] = false;
    }      
  }
  sparkle_count++;
  overlay();
}



