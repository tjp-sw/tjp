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


//--------------------------------------- SPARKLE RAIN -------------------------------------------
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


//---------------------------------- SPARKLE 3 CIRCLES ---------------------------
// Highlights the 3 most prominent circles on the torus
// Each of these circles moves around the torus to parallel circles

void sparkle_3_circles() {

  int ring, pixel;
 
  int current_ring, current_pixel, current_coin_bottom;

  // fixme: these randos would have to be sent in by the pi
  int ring_motion_direction = random(2);
  int pixel_motion_direction = random(2);  
  int coin_motion_direction = random(2);
 
  // if this is the first frame for this sparkle animation,  initialize
  if (sparkle_count == 0) {

     current_ring = random(NUM_RINGS);
     current_pixel = random(VISIBLE_LEDS_PER_RING);
     current_coin_bottom = random(NUM_RINGS);
  }

  // black out previous sparkle
  sparkle_reset();

  // light 3 circles fixme: maybe have 3 different colored circles

  // vertical circle
  for (pixel = 0; pixel < VISIBLE_LEDS_PER_RING; pixel++) {
      sparkle[current_ring][pixel] = sparkle_color;
  }

  // horizontal circle
  for (ring = 0; ring < NUM_RINGS; ring++) {
      sparkle[ring][current_pixel] = sparkle_color;
  }

  // third circle has a slope of 3
  for (ring = 0; ring < NUM_RINGS; ring++) {
    for (pixel = 0; pixel < 216; pixel += 3) {
       sparkle[(ring + current_coin_bottom) % NUM_RINGS][pixel] = sparkle_color;
       sparkle[72 - (ring + current_coin_bottom) % NUM_RINGS][pixel] = sparkle_color;
    }
  }

  // move each circle start over one
  current_ring = (current_ring + 1) % NUM_RINGS;
  current_pixel = (current_pixel + 1) % VISIBLE_LEDS_PER_RING;
  current_coin_bottom = (current_coin_bottom + 1) % NUM_RINGS;

  sparkle_count++;
  overlay();
}



//---------------------------------- SPARKLE WARP SPEED ---------------------------
//  Sends sparkles racing around horizontally from ring to ring
//  When someone is inside it is intended to look like flying through space or that old cheesy 
//  animation when star trek goes to "warp speed"  
//
//  1/portion of the leds should be lit

void sparkle_warp_speed() {

  int ring, pixel, i;
  int portion = 30; // 1/portion of the structure will be covered in stars
  int ring0[VISIBLE_LEDS_PER_RING];
  boolean ring0_is_set[VISIBLE_LEDS_PER_RING];
  
  sparkle_color = CRGB::LightYellow;

  // if this is the first frame for this sparkle animation, black out old sparkle leds,
  // then choose random pixels to race around structure
  if (sparkle_count == 0) {
     sparkle_reset();

     for (ring = 0; ring < RINGS_PER_NODE; ring++) {
      for (pixel = 0; pixel < VISIBLE_LEDS_PER_RING; pixel++) {
        if (random(portion) == 0) {
          sparkle_is_set[ring][pixel] = true;
          sparkle[ring][pixel] = sparkle_color; // turn on 1/portion of the leds as stars
        }
      }
     }
  }

  // push existing sparkles backwards one ring
  // add temp array to avoid over-write
  else {
    
    // save ring 0 info so it doesn't get overwritten
    for (i = 0; i < VISIBLE_LEDS_PER_RING; i++) {
      ring0[i] = sparkle[0][i];
      ring0_is_set[i] = sparkle_is_set[0][i];
    }

    // scoot all pixels backwards one ring
    for (ring = 1; ring <= RINGS_PER_NODE ; ring++) {
      for (pixel = 0; pixel < VISIBLE_LEDS_PER_RING; pixel++) {
        if (sparkle_is_set[ring][pixel]) {
          
          // light sparkle up at next ring
          sparkle[ring - 1][pixel] = sparkle_color;
          sparkle_is_set[ring - 1][pixel] = true;

          // turn off sparkle at this ring
          sparkle[ring][pixel] = CRGB::Black; 
          sparkle_is_set[ring][pixel] = false;
         }
      }
    }
    
    // move ring0 info to last ring
    for (i = 0; i < VISIBLE_LEDS_PER_RING; i++) {
      sparkle[RINGS_PER_NODE - 1][i] = ring0[i];
      sparkle_is_set[RINGS_PER_NODE - 1][i] = ring0_is_set[i];
    }
  }
  sparkle_count++;
  overlay();
}
