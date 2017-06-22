// functions in here: snake, sparkle_reset, sparkle_rain, sparkle_giltter, sparkle_warp_speed

//  each time a new sparkle function is called, calling function should reset sparkle_count to 0

//---------------------------------- SPARKLE WARP SPEED ---------------------------
//  In progress
//  Sends sparkles racing around horizontally from ring to ring
//  When someone is inside it is intended to look like flying through space or that old cheesy 
//  animation when star trek goes to "warp speed"  
//
//  1/portion of the leds should be lit

// fixme: doesn't erase previous sparkle 
void sparkle_warp_speed() {

  int ring, pixel;
  int portion = 20; // 1/portion of the structure will be covered in stars  higher # = fewer stars
  
  Serial.println("in warp speed");
    
  sparkle_color = CRGB::LightYellow;

  // if this is the first frame for this sparkle animation, black out old sparkle leds,
  // then choose random pixels to race around structure

 
  if (sparkle_count == 0) {
     sparkle_reset();
     randomSeed(analogRead(0));

     for (int ring = 0; ring < 4; ring++) {
       for (int pixel = 0; pixel < VISIBLE_LEDS_PER_RING; pixel++) {
         if (random(portion) == 0) {
           sparkle_is_set[ring][pixel] = true;
           sparkle[ring][pixel] = sparkle_color; // turn on 1/portion of the leds as stars
         }
       }
     }
  }

  //only move every tenth cycle to slow it down
  if (sparkle_count % 10 == 0) {
    
    // save ring 0 info so it doesn't get overwritten
    for (pixel = 0; pixel < 408; pixel++) {
      temp_is_set[pixel] = sparkle_is_set[0][pixel];
      temp[pixel] = sparkle[0][pixel];

      // clear sparkle[0]
      sparkle_is_set[0][pixel] = false;
      sparkle[0][pixel] = CRGB::Black;
    }

    // shift existing pixels backwards one ring
    for (ring = 1; ring < 4; ring++) {
      for (pixel = 0; pixel < 408; pixel++) {
        sparkle[ring - 1][pixel] = sparkle[ring][pixel];
        sparkle_is_set[ring - 1][pixel] = sparkle_is_set[ring][pixel];

        // clear sparkle[ring]
        sparkle_is_set[ring][pixel] = false;
        sparkle[ring][pixel] = CRGB::Black;
     }
    }

    // move temp info to last ring
    for (pixel = 0; pixel < 408; pixel++) {
      sparkle[4-1][pixel] = temp[pixel];
      sparkle_is_set[4-1][pixel] = temp_is_set[pixel];  
        
      // clear sparkle[ring]
      temp[pixel] = false;
      temp_is_set[pixel] = CRGB::Black;
    }
  }

  sparkle_count++;
  overlay();
}


void scale_usage() {

  CRGB color_to_scale = CRGB::LightYellow;
  
  for (int scale = 0; scale <= 100; scale++) {
    for (int pixel = 0; pixel < 408; pixel++) {
      leds[0][pixel]= scale_intensity( color_to_scale, scale);
    }
    delay(100);
  }
}


//------------------------------------------- SNAKE ---------------------------------------
//  Sends alternating bands of colors rotating around the rings
//
//  Note use of getcolor(), a work-around for not being able to make a 2D array of CRGBs
//  Creates repeated snakes with show_parameters[NUM_EDM_COLORS_INDEX] colors, with each color repeated 
//  show_parameters[COLOR_THICKNESS_INDEX] times, separated by show_parameters[BLACK_THICKNESS_INDEX] 
//  black LEDs.

// offset determines how many steps a ring's lights are rotated to put on the adjacent ring

// good for EDM or mid-layer (maybe with longer black space so background can be seen)


void snake() {

int offset = show_parameters[RING_OFFSET_INDEX];
int rotation_direction = show_parameters[INTRA_RING_MOTION_INDEX];
int num_colors = show_parameters[NUM_EDM_COLORS_INDEX];
int color_length = show_parameters[COLOR_THICKNESS_INDEX];
int strip_length = num_colors * color_length + show_parameters[BLACK_THICKNESS_INDEX];
int palette_num = show_parameters[PALETTE_INDEX];
int throttle = show_parameters[INTRA_RING_SPEED_INDEX];  // the higher the value, the slower the motion

  // slow down the animation without using delay()
  if (loop_count % throttle * 2 == 0) {
    for (int ring = 0; ring < NUM_RINGS; ring++) {
      for (int pixel = 0; pixel < VISIBLE_LEDS_PER_RING; pixel++) {

        // first color for color_length pixels
        if (pixel % strip_length < color_length) {
          leds[ring][(pixel + rotation_direction * loop_count + offset*ring) % VISIBLE_LEDS_PER_RING] = get_color(palette_num, show_colors[0]);
        }

        // second color for color_length pixels
        else if (pixel % strip_length < 2*color_length) {
            leds[ring][(pixel + rotation_direction * loop_count + offset*ring) % VISIBLE_LEDS_PER_RING] = get_color(palette_num, show_colors[1]);
        }

        // third color for color_length pixels
        else if (pixel % strip_length < 3*color_length) {
            leds[ring][(pixel + rotation_direction * loop_count + offset*ring) % VISIBLE_LEDS_PER_RING] = get_color(palette_num, show_colors[2]);
        }
        // black color for color_length pixels
        else {
          leds[ring][(pixel + rotation_direction * loop_count + offset*ring) % VISIBLE_LEDS_PER_RING] = CRGB::Black;
        }
      }
    }
  }
}


//  overwrite current sparkle led array to black 

void sparkle_reset() {
  
  for (int ring = 0; ring < NUM_RINGS; ring++) {
    for (int pixel = 0; pixel < VISIBLE_LEDS_PER_RING; pixel++) {
      leds_node[ring][pixel] = CRGB::Black;
      sparkle_is_set[ring][pixel] = false;
    }
  }  
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
  
  for (int ring = 0; ring < NUM_RINGS; ring++) {
    for (int pixel = 0; pixel < VISIBLE_LEDS_PER_RING; pixel++) {
      if (sparkle_is_set[ring][pixel]) {
        Serial.print("not everything was cleared");
        Serial.print(ring);
        Serial.print(" ");
        Serial.println(pixel);
      }
    }
  }

  Serial.println("in glitter");
  

  for (int ring = 0; ring < NUM_RINGS; ring++) {
    for (int pixel = 0; pixel < VISIBLE_LEDS_PER_RING; pixel++) {
      if (random(portion) == 0) {       
        sparkle_is_set[ring][pixel] = true;
        sparkle[ring][pixel] = sparkle_color; // turn on 1/portion of the leds as stars
      }
    }
  }
  sparkle_count++;
  overlay();
}


//--------------------------------------- SPARKLE RAIN -------------------------------------------
//  Puts raindrops randomly at the top of the structure and runs them down both sides of each ring.  
//  1/portion of the leds should be lit

void sparkle_rain() {

  int ring, pixel;

  // volume:sprinkle = 0, rain = 1; downpour = 2
  int volume = show_parameters[COLOR_THICKNESS_INDEX] % 3;

  // the higher portion is, the fewer raindrops are formed
  //  1600 sprinkle; 400 rain,  100 heavy rain; 
  int portion = 50 * pow(2, (5 - 2 * volume)); 
  Serial.println(volume);
  Serial.println(portion);

  int offset = 40;
  int left_ring_top = HALF_VISIBLE - offset;
  int right_ring_top = HALF_VISIBLE + offset;
 
  // if this is the first frame for this sparkle animation, black out old sparkle leds
  if (sparkle_count == 0) {
     sparkle_reset();
  }

  // create new raindrops every "portion" cycles; may need to change this constant
//  if (sparkle_count % (5 * (3 - volume)) == 0) {
    for (ring = 0; ring < RINGS_PER_NODE; ring++) {
      for (pixel = left_ring_top; pixel < right_ring_top; pixel++) {
        if (random(portion * 2) == 0) {
          sparkle_is_set[ring][pixel] = true;
          sparkle[ring][pixel] = sparkle_color; // turn on 1/portion of the leds as rain
          Serial.println("some are being turned on");
        }
      }
    }
//  }

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




// ********************* IN PROGRESS BELOW HERE ********************


//---------------------------------- SPARKLE 3 CIRCLES ---------------------------
// Highlights the 3 most prominent circles on the torus
// Each of these circles moves around the torus to parallel circles
// 
// fixme: allow speed to change as parameter
// fixme: add 3rd circle

void sparkle_3_circles() {

  int ring, pixel;

  // fixme: these values would have to be sent in by the pi
  int ring_motion_direction = 1;
  int pixel_motion_direction = 1;  
  int coin_motion_direction = 1;

  // if this is the first frame for this sparkle animation,  initialize
  if (sparkle_count == 0) {

     // choose random starting points
     // adapt for usd versus full structure
     current_ring = random(4); 
     current_pixel = random(VISIBLE_LEDS_PER_RING);
     current_coin_bottom = random(4);

     Serial.println(current_ring);
     Serial.println(current_pixel);
    }

    // black out previous sparkle
    sparkle_reset();

  // light 3 circles 
  // fixme: maybe have 3 different colored circles

  // vertical circle
  for (pixel = 0; pixel < VISIBLE_LEDS_PER_RING; pixel++) {
      sparkle[current_ring][pixel] = sparkle_color;
  }

  // horizontal circle
  for (ring = 0; ring < 4; ring++) {
      sparkle[ring][current_pixel] = sparkle_color;
  }

  // third circle has a slope of 3
  // work around rings simultaneously from both sides
  for (ring = 0; ring < 4 / 2; ring++) {
    for (pixel = 0; pixel < HALF_VISIBLE; pixel += 3) {
 
      sparkle[(ring + current_coin_bottom) % 4][pixel] = sparkle_color;
      sparkle[3 - (ring + current_coin_bottom) % 4][pixel] = sparkle_color;
    }
  }

  // move each circle start over one unit every 50 iterations
  if (sparkle_count % 50 == 0) {
    current_ring = (current_ring + 1) % 4;
    current_pixel = (current_pixel + 1) % VISIBLE_LEDS_PER_RING;
    current_coin_bottom = (current_coin_bottom + 1) % 4;
  }

  sparkle_count++;
  overlay();
}



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
        for (int pixel = 0; pixel < VISIBLE_LEDS_PER_RING; pixel++) {

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






//---------------------------------- SPARKLE WARP SPEED OLD---------------------------
//  In progress
//  Sends sparkles racing around horizontally from ring to ring
//  When someone is inside it is intended to look like flying through space or that old cheesy 
//  animation when star trek goes to "warp speed"  
//
//  1/portion of the leds should be lit


  void sparkle_warp_speed_old() {

  int ring, pixel;
  int portion = 30; // 1/portion of the structure will be covered in stars  higher # = fewer stars
  
  sparkle_color = CRGB::LightYellow;

  // if this is the first frame for this sparkle animation, black out old sparkle leds,
  // then choose random pixels to race around structure
  if (sparkle_count == 0) {
     sparkle_reset();
     randomSeed(analogRead(0));

     for (ring = 0; ring < 4; ring++) {
      for (pixel = 0; pixel < 408; pixel++) {
        if (random(portion) == 0) {
//          sparkle_is_set[ring][pixel] = true;
          sparkle[ring][pixel] = sparkle_color; // turn on 1/portion of the leds as stars      
        }
      }
    }
  }

  //only move every tenth cycle
  if (sparkle_count % 10 == 0) {
    
  // push existing sparkles backwards one ring
    
  // save ring 0 info so it doesn't get overwritten, then black ring 0 out
  for (pixel = 0; pixel < 408; pixel++) {
      temp[pixel] = sparkle[0][pixel];
//      temp_is_set[pixel] = sparkle_is_set[0][pixel];

//      sparkle[0][pixel] = CRGB::Black;
//      sparkle_is_set[0][pixel] = false;
/*    if (sparkle_is_set[0][pixel]) {
      temp[pixel] = sparkle[0][pixel];
      temp_is_set[pixel] = true;


    }
    else {
      temp[pixel] = CRGB::Black;
      temp_is_set[pixel] = false;
    }
*/  }

  // shift all pixels backwards one ring
  for (ring = 1; ring < 4; ring++) {
    for (pixel = 0; pixel < 408; pixel++) {
        sparkle[ring - 1][pixel] = sparkle[ring][pixel];
//        sparkle_is_set[ring - 1][pixel] = sparkle_is_set[ring][pixel];
/*      if (sparkle_is_set[ring][pixel]) {
          
        // light sparkle up at next ring
        sparkle[ring - 1][pixel] = sparkle[ring][pixel];
        sparkle_is_set[ring - 1][pixel] = sparkle_is_set[ring][pixel];

        // kill off sparkle on current ring
        sparkle[ring][pixel] = CRGB::Black;
        sparkle_is_set[ring][pixel] = false;
*/
      }
    }

  
  // move temp info to last ring
  for (pixel = 0; pixel < 408; pixel++) {
      sparkle[4-1][pixel] = temp[pixel];
//      sparkle_is_set[4 - 1][pixel] = temp_is_set[pixel];

 /*     
    if (temp_is_set[pixel]) {
      Serial.println("********** move last ring");
      sparkle[4-1][pixel] = temp[pixel];
      sparkle_is_set[4 - 1][pixel] = temp_is_set[pixel];

    }
    temp[pixel] = CRGB::Black;
    temp_is_set[pixel] = false;
    */
  }
  }

  sparkle_count++;
  overlay();
  Serial.println("------------------- pixel dump ------------------");
}



// hasn't been tested
void diane_arrow_1() {

  int ring, pixel;
  int slope = 3;

  for (ring = 0; ring < HALF_VISIBLE / 3; ring++) {
    for (pixel = HALF_VISIBLE - 3 * ring; pixel <= HALF_VISIBLE + 3 * ring; pixel++) {
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
  for (ring = HALF_VISIBLE / 3; ring < NUM_RINGS; ring++) {
    for (pixel = 0; pixel < VISIBLE_LEDS_PER_RING; pixel++) {
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
  for (ring = 0; ring < HALF_VISIBLE / 6; ring++) {
    for (pixel = HALF_VISIBLE - 6 * ring; pixel <= HALF_VISIBLE + 6 * ring; pixel++) {
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
    for (pixel = 0; pixel < VISIBLE_LEDS_PER_RING; pixel++) {
      leds[(34 + loop_count) % NUM_RINGS][pixel] = show_colors[2];
    }
  }

  // arrow 2
  for (ring = HALF_VISIBLE / 6 + 2; ring < 2 * HALF_VISIBLE / 6 + 2; ring++) {
    for (pixel = HALF_VISIBLE - 6 * ring; pixel <= HALF_VISIBLE + 6 * ring; pixel++) {
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
  for (ring = 2 * HALF_VISIBLE / 6 + 2; ring <= NUM_RINGS; ring++) {
    for (pixel = 0; pixel < VISIBLE_LEDS_PER_RING; pixel++) {
      leds[(34 + loop_count) % NUM_RINGS][pixel] = show_colors[2];
    }
  }

}

//  To be written 

//    sparkle_fn[5] = sparkle_warp_speed;
//    sparkle_fn[7] = sparkle_3_circle;
//    sparkle_fn[2] = sparkle_twinkle;

//    sparkle_fn[6] = sparkle_comet;
//    sparkle_fn[3] = sparkle_wind;
//    sparkle_fn[4] = sparkle_wiggle_wind;
// sparkle_collider
// sparkle_fireworks
// sparkle_lightening
//    sparkle_fn[8] = sparkle_torus_knot();
