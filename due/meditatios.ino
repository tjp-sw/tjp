/*

STEP 1: GO ring by ring and light up
STEP 2: Spacing will be amount of ring light up
Step 3: Adding dimming in and out
loop_counter = 60 ms; 1000ms = 1 second. 16.6 loop_counter per second 
1200 seconds in 20 minutes 1200000 ms per 20 minutes
*/

byte start_flag = 0; 
void bloom(/*byte lock*/){
  short int ring,pixel,spacing,bound;
  spacing = round(1200/(float)LEDS_PER_RING); // should be 3;
  bound = round((loop_count/16.6)*spacing);
  // make sure everything is black first time into function
  if(!start_flag){
    for( ring = 0; ring < NUM_RINGS; ring++){
      for( pixel = 0; pixel < LEDS_PER_RING; pixel++){
        mid_layer[ring][pixel] = BLACK;
        start_flag = 1;
      }
    }
  }

  // go number of rings/2 ligt up pixels moving forward
  // second half of rings light up moving backward

  // forward light
  for(ring = 0; ring < NUM_RINGS/2; ring++){
    for( pixel = 0; pixel < bound; pixel++){
      // activate dimming to give flash sense
      if(bound%2 ==0){
                mid_layer[ring][pixel] = get_mid_color(0);
      }
      else{
        // dimming effect
        mid_layer[ring][pixel] = get_mid_color(0,6);
      }
    }
  }
// backward light
  for(ring = (NUM_RINGS/2) +1; ring < NUM_RINGS; ring++){
    for(pixel = LEDS_PER_RING; pixel > LEDS_PER_RING - bound; pixel--){
       if(bound%2 ==0){
                  mid_layer[ring][pixel] = get_mid_color(0);
        }
        else{
          // dimming effect
          mid_layer[ring][pixel] = get_mid_color(0,6);
        }
    }
  }
}

/*
 *       Step 1. Make entire background one color
 *       Step2. determine timing of the drops. One every 3 seconds.
 *       Step 3. Light up rings on leftside
 *       Step 4. Create moving dot effect from. Light up pixel right to left and all pixels after light up set to black until we reach right side
 *       Step 5. All leds light up at by 20 minte mark;
 */
byte start_flagg = 0;
short int light_pix;
void drip(/*byte lock*/){
  short int ring,pixel, dif;
  light_pix = 0;
  // create blackscreen
  if(!start_flagg){
     for( ring = 0; ring < NUM_RINGS; ring++){
      for( pixel = 0; pixel < LEDS_PER_RING; pixel++){
        mid_layer[ring][pixel] = BLACK;
        start_flagg = 1;
      }
    }
  }
  // determine number of pixels to placed and light them up
  dif = light_pix;
  light_pix = round(loop_count/(float)50);
  for(ring = 0; ring <NUM_RINGS; ring++){
    for(pixel = 0; pixel<light_pix; pixel++){
      mid_layer[ring][pixel] = get_mid_color(0);
    }
  }

  // check if 3 seconds has elapsed dif should only be  less than light pix if that amount of time has passed. 
  //if so add a new pixel from left to right
  if(dif<light_pix){
    for(ring = 0; ring < NUM_RINGS; ring++){
      for(pixel = LEDS_PER_RING;pixel>light_pix; pixel--){
        mid_layer[ring][pixel] = get_mid_color(0);
        mid_layer[ring][pixel-1] = BLACK;
      }
    }
  }
}


