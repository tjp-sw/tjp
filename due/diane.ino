//  each time a new sparkle function is called, calling function should reset sparkle_count to 0


//  overwrite current sparkle led array to black 

void sparkle_reset() {
  
  for (int ring = 0; ring < RINGS_PER_NODE; ring++) {
    for (int pixel = 0; pixel < LEDS_PER_RING; pixel++) {
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
  
  // enforce reasonable bounds for rain, 2 <= show_parameters[COLOR_THICKNESS_INDEX] <= 8
  // fixme: for use as sparkle layer, would want to tamp this down significantly, maybe to 3 or 4
  if (show_parameters[COLOR_THICKNESS_INDEX] > 8) {
    show_parameters[COLOR_THICKNESS_INDEX] = 8;
  }
  if (show_parameters[COLOR_THICKNESS_INDEX] < 2) {
    show_parameters[COLOR_THICKNESS_INDEX] = 2;
  }

  // the higher portion is, the fewer raindrops are formed
  // 3200 is barely trickle; 1600 sprinkle; 400 rain,  100 heavy rain;  50 deluge; 
  int portion = 50 * pow(2, (8 - show_parameters[COLOR_THICKNESS_INDEX])); 
  
  int offset = 40;
  int left_ring_top = HALF_RING - offset;
  int right_ring_top = HALF_RING + offset;
 
  // if this is the first frame for this sparkle animation, black out old sparkle leds
  if (sparkle_count == 0) {
     sparkle_reset();
  }

  // create new raindrops every "portion" cycles; may need to change this constant
  if (sparkle_count % (portion) == 0) {
    for (ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
      for (pixel = left_ring_top; pixel < right_ring_top; pixel++) {
        if (random(portion) == 0) {
          sparkle_is_set[ring][pixel] = true;
          sparkle[ring][pixel] = sparkle_color; // turn on 1/portion of the leds as rain
        }
      }
    }
  }

  // drop existing raindrops down 1 pixel
  for (ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {

    // special case so that bottom left pixel dropping down doesn't make array index out of bounds.
    if (sparkle_is_set[ring][0] == true) {
      // black out bottom left pixel
      sparkle[ring][0] = CRGB::Black;
      sparkle_is_set[ring][0] = false;      
    }

    // general case, left side
    for (pixel = 1; pixel <= HALF_RING; pixel++) { 
      // light up next pixel down
      sparkle[ring][pixel - 1] = sparkle[ring][pixel];
      sparkle_is_set[ring][pixel - 1] = true;

      // black out current pixel
      sparkle[ring][pixel] = CRGB::Black;
      sparkle_is_set[ring][pixel] = false;
    }
    
    // right side
    for (pixel = LEDS_PER_RING; pixel > HALF_RING; pixel--) { 
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

//---------------------------------- SPARKLE GLITTER ---------------------------
//  Creates sparkles of glitter randomly all over the structure 
//
//  1/portion of the leds should be lit

void sparkle_glitter() {

  int portion = 30; // 1/portion of the structure will be covered in glitter

  sparkle_color =  CRGB::LightYellow; //show_colors[0];

  // if this is the first frame for this sparkle animation, black out old sparkle leds,
  // then choose random pixels to race around structure

  sparkle_reset();

  for (int ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
    for (int pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      if (random(portion) == 0) {
        sparkle_is_set[ring][pixel] = true;
        sparkle[ring][pixel] = sparkle_color; // turn on 1/portion of the leds as stars
      }
    }
  }
  sparkle_count++;
  overlay();
}


//---------------------------------- SPARKLE 3 CIRCLES ---------------------------
// Highlights the 3 most prominent circles on the torus
// Each of these circles moves around the torus to parallel circles
// 
// fixme: allow speed to change as parameter
// fixme: add 3rd circle

/* void sparkle_3_circles() {

  int ring, pixel;

  // fixme: these randos would have to be sent in by the pi
  int ring_motion_direction = random(2);
  int pixel_motion_direction = random(2);  
  int coin_motion_direction = random(2);

  // if this is the first frame for this sparkle animation,  initialize
  if (sparkle_count == 0) {

     // choose random starting points
     current_ring = random(NUM_RINGS); 
     current_pixel = random(LEDS_PER_RING);
     current_coin_bottom = random(NUM_RINGS);

     Serial.println(current_ring);
     Serial.println(current_pixel);
    }

    // black out previous sparkle
    sparkle_reset();

  // light 3 circles 
  //fixme: maybe have 3 different colored circles

  // vertical circle
  for (pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      sparkle[current_ring][pixel] = sparkle_color;
      sparkle_is_set[current_ring][pixel] = true;
  }

  // horizontal circle
  for (ring = 0; ring < RINGS_PER_NODE; ring++) {
      sparkle[ring][current_pixel] = sparkle_color;
      sparkle_is_set[ring][current_pixel] = true;
  }

  // third circle has a slope of 3
  // work around ring simultaneously from both sides
 // for (ring = 0; ring < NUM_RINGS / 2; ring++) {
 //   for (pixel = 0; pixel < HALF_RING; pixel += 3) {
 //     sparkle[(ring + current_coin_bottom) % NUM_RINGS][pixel] = sparkle_color;
 //     sparkle_is_set[(ring + current_coin_bottom) % NUM_RINGS][pixel] = true;
      
 //     sparkle[NUM_RINGS - (ring + current_coin_bottom) % NUM_RINGS][pixel] = sparkle_color;
 //     sparkle_is_set[NUM_RINGS - (ring + current_coin_bottom) % NUM_RINGS][pixel] = true;
 //   }
 // }

  // move each circle start over one unit
  if (sparkle_count % 50 == 0) {
    current_ring = (current_ring + 1) % NUM_RINGS;
    current_pixel = (current_pixel + 1) % LEDS_PER_RING;
    current_coin_bottom = (current_coin_bottom + 1) % NUM_RINGS;
  }

  sparkle_count++;
  overlay();
}
*/


// Sparkle_twinkle
//
// Doesn't work yet
// Chooses random location for stars, with random starting intensities, 
// and then randomly and continuously increases or decreases their intensity over time

/*
 * 
 *void sparkle_twinkle(){

  int max_twinkle_intensity = 255; 
  int min_twinkle_intensity = 10; 
  int portion = 5; // then stars will appear on approx 1/5 of the leds
  int brightness;

  // if this is the first time through this sparkle, clear the sparkles, 
  // and choose places and relative intensities for stars
  if (sparkle_count == 0) {
    sparkle_reset();

    for (int ring = 0; ring < RINGS_PER_NODE; ring++) {
      for (int pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        if (random(portion) == 0) {
          is_set[strip][pixel] = true;

          // intensity[ring][pixel] = random(MIN_SPARKLE_INTENSITY, MAX_SPARKLE_INTENSITY);
          brightness = random(0, (int) max_intensity_scalar(sparkle_color) * 100);
          sparkle[ring][pixel] = scale_color(sparkle_color, start_brightness);  // turn on 1/5 of the leds as stars

          increasing[ring][pixel] = (random(2) == 1);  // randomly choose to increase or decrease intensity
        }
      }
    }

    for (int ring = 0; ring < NUM_RINGS; ring++) {
        for (int pixel = 0; pixel < LEDS_PER_RING; pixel++) {

            // only change intensity if pixel is chosen as star
            if (is_set[ring][pixel]) {
   
                if (increasing[ring][pixel]) {
                    if ((current_intensity(sparkle[ring][pixel]) * 1.1) >= max_twinkle_intensity) {
                        increasing[ring][pixel] = false;
                    }
                    else {
                        sparkle[ring][pixel].red *= 1.1;
                        sparkle[ring][pixel].blue *= 1.1;
                        sparkle[ring][pixel].green *= 1.1; 
                   }
                }

                else  { // decreasing
                    if ((current_intensity(sparkle[ring][pixel]) * .9) <= min_twinkle_intensity) {
                        increasing[ring][pixel] = true;
                    }
                    else {
                       sparkle[ring][pixel].red *= .9;
                       sparkle[ring][pixel].blue *= .9;
                       sparkle[ring][pixel].green *= .9; 
                    }
                } // end else decreasing

            } // end if pixel chosen as star
        }
    }
}
*/



//---------------------------------- SPARKLE WARP SPEED ---------------------------
//  In progress
//  Sends sparkles racing around horizontally from ring to ring
//  When someone is inside it is intended to look like flying through space or that old cheesy 
//  animation when star trek goes to "warp speed"  
//
//  1/portion of the leds should be lit

/*
 * 
  void sparkle_warp_speed() {

  int ring, pixel;
  int portion = 30; // 1/portion of the structure will be covered in stars
  int ring0[LEDS_PER_RING];
  boolean ring0_is_set[LEDS_PER_RING];
  
  sparkle_color = CRGB::LightYellow;

  // if this is the first frame for this sparkle animation, black out old sparkle leds,
  // then choose random pixels to race around structure
  if (sparkle_count == 0) {
     sparkle_reset();

     for (ring = 0; ring < RINGS_PER_NODE; ring++) {
      for (pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        if (random(portion) == 0) {
          sparkle_is_set[ring][pixel] = true;
          sparkle[ring][pixel] = sparkle_color; // turn on 1/portion of the leds as stars
        }
      }
    }
  }

  // push existing sparkles backwards one ring
  // add temp array to avoid over-write
  // slow down the motion
//  else if (loop_count % 10 == 0) {
    
  // save ring 0 info so it doesn't get overwritten
  for (pixel = 0; pixel < LEDS_PER_RING; pixel++) {
    if (sparkle_is_set[0][pixel]) {
      ring0[pixel] = sparkle[0][pixel];
      ring0_is_set[pixel] = sparkle_is_set[0][pixel];
    }
  }

  // shift all pixels backwards one ring
  for (ring = 1; ring <= RINGS_PER_NODE ; ring++) {
    for (pixel = 0; pixel < LEDS_PER_RING; pixel++) {
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
  for (pixel = 0; pixel < LEDS_PER_RING; pixel++) {
    if (sparkle_is_set[3][pixel]) {
      sparkle[3][pixel] = ring0[pixel];
      sparkle_is_set[RINGS_PER_NODE - 1][pixel] = ring0_is_set[pixel];
    }
  }

  sparkle_count++;
  overlay();
  delay(1000);
}
*/


//  Sends alternating bands of colors rotating around the rings
//
//  Note use of getcolor(), a work-around for not being able to make a 2D array of CRGBs
// fixme: remove delay()

// Creates repeated snakes with show_parameters[NUM_COLORS_INDEX] colors, with each color repeated 
// show_parameters[COLOR_THICKNESS_INDEX] times, separated by show_parameters[BLACK_THICKNESS_INDEX] 
// black LEDs.

// offset determines how many steps a ring's lights are rotated to put on the adjacent ring

// good for EDM or mid-layer (maybe with longer black space so background can be seen)


void snake() {

int offset = show_parameters[RING_OFFSET_INDEX];
int rotation_direction = show_parameters[INTRA_RING_MOTION_INDEX];
int num_colors = show_parameters[NUM_COLORS_INDEX];
int color_length = show_parameters[COLOR_THICKNESS_INDEX];
int strip_length = num_colors * color_length + show_parameters[BLACK_THICKNESS_INDEX];
int palette_num = show_parameters[PALETTE_INDEX];

  // slow down the animation without using delay()
  if (loop_count % 1 == 0) {
    for (int ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++) {
      for (int pixel = 0; pixel < LEDS_PER_RING; pixel++) {

        // first color for color_length pixels
        if (pixel % strip_length < color_length) {
          leds[ring][(pixel + rotation_direction * loop_count + offset*ring) % LEDS_PER_RING] = get_color(palette_num, show_colors[0]);
        }

        // second color for color_length pixels
        else if (pixel % strip_length < 2*color_length) {
            leds[ring][(pixel + rotation_direction * loop_count + offset*ring) % LEDS_PER_RING] = get_color(palette_num, show_colors[1]);
        }

        // third color for color_length pixels
        else if (pixel % strip_length < 3*color_length) {
            leds[ring][(pixel + rotation_direction * loop_count + offset*ring) % LEDS_PER_RING] = get_color(palette_num, show_colors[2]);
        }
        // black color for color_length pixels
        else {
          leds[ring][(pixel + rotation_direction * loop_count + offset*ring) % LEDS_PER_RING] = CRGB::Black;
        }
      }
    }
  }
}


// hasn't been tested
void diane_arrow_1() {

  int ring, pixel;
  int slope = 3;

  for (ring = 0; ring < HALF_RING / 3; ring++) {
    for (pixel = HALF_RING - 3 * ring; pixel <= HALF_RING + 3 * ring; pixel++) {
      if (pixel >= ring) {
        // inside the arrow
        leds[(ring + loop_count) % NUM_RINGS][pixel] = show_colors[0];
      }
      else {
        // background of arow
        leds[(ring + loop_count) % NUM_RINGS][pixel] = show_colors[1];
      }
    }
  }
  for (ring = HALF_RING / 3; ring < NUM_RINGS; ring++) {
    for (pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      leds[(ring + loop_count) % NUM_RINGS][pixel] = show_colors[1];

    }
  }
}

// 2 copies of same color arrows, with alternating backgrounds
// hasn't been tested
void diane_arrow_2() {

  int ring, pixel;
  int slope = 6;

  // arrow 1
  for (ring = 0; ring < HALF_RING / 6; ring++) {
    for (pixel = HALF_RING - 6 * ring; pixel <= HALF_RING + 6 * ring; pixel++) {
      if (pixel >= ring) {
        // inside the arrow
        leds[(ring + loop_count) % NUM_RINGS][pixel] = show_colors[0];
      }
      else {
        // background of arrow
        leds[(ring + loop_count) % NUM_RINGS][pixel] = show_colors[1];
      }
    }
  }

  // ring 34 & 35 solid of background color #1
  for (ring = 34; ring <= 35; ring++) {
    for (pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      leds[(34 + loop_count) % NUM_RINGS][pixel] = show_colors[2];
    }
  }

  // arrow 2
  for (ring = HALF_RING / 6 + 2; ring < 2 * HALF_RING / 6 + 2; ring++) {
    for (pixel = HALF_RING - 6 * ring; pixel <= HALF_RING + 6 * ring; pixel++) {
      if (pixel >= ring) {
        // inside the arrow
        leds[(ring + loop_count) % NUM_RINGS][pixel] = show_colors[0];
      }
      else {
        // background of arrow
        leds[(ring + loop_count) % NUM_RINGS][pixel] = show_colors[2];
      }
    }
  }

  // ring 70, 71, 72 solid of background color #2
  for (ring = 2 * HALF_RING / 6 + 2; ring <= NUM_RINGS; ring++) {
    for (pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      leds[(34 + loop_count) % NUM_RINGS][pixel] = show_colors[2];
    }
  }

}


