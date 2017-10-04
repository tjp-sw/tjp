/*

STEP 1: GO ring by ring and light up
STEP 2: Spacing will be amount of ring light up
Step 3: Adding dimming in and out
loop_counter = 60 ms; 1000ms = 1 second. 16.6 loop_counter per second 
1200 seconds in 20 minutes 1200000 ms per 20 minutes
*/

byte start_flag = 0; 
void bloom(){
  leds_all = CRGB::Black;
  short int ring,pixel,spacing;
  spacing = round(1200/(float)LEDS_PER_RING); // should be 3;
  int bound = 30 + edm_count/500;
  if(bound >= LEDS_PER_RING/2) { bound = LEDS_PER_RING/2; }
  // make sure everything is black first time into function
  if(!start_flag){
    for( ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++){
      for( pixel = 0; pixel < LEDS_PER_RING; pixel++){
        leds[get_1d_index(ring % RINGS_PER_NODE, pixel % LEDS_PER_RING)] = CRGB::Black;  // leds[get_1d_index(ring, pixel)] = BLACK;
        start_flag = 1;
      }
    }
  }

  // go number of rings/2 ligt up pixels moving forward
  // second half of rings light up moving backward

  // forward light
  for(ring = 0; ring < NUM_RINGS; ring++){
    if(ring < node_number*RINGS_PER_NODE || ring >= (node_number+1)*RINGS_PER_NODE) { continue; }
    uint8_t offset = ring % 12;
    if(offset >= 6) { offset = 12 - offset; }
    uint8_t this_bound = bound + 6 * offset;
    if(this_bound > LEDS_PER_RING/2) { this_bound = LEDS_PER_RING/2; }
    CRGB this_color = ColorFromPalette(mid_palette, get_mid_color(ring % 3, (edm_count/4 + ring) % NUM_MID_DIMMING_LEVELS));
    this_color.fadeToBlackBy(255 * ((edm_count/4 + ring) % 12) / 12);
    for( pixel = 0; pixel < this_bound; pixel++){
      leds[get_1d_index(ring % RINGS_PER_NODE, pixel % LEDS_PER_RING)] = this_color;
      leds[get_1d_index(ring % RINGS_PER_NODE, (LEDS_PER_RING - 1 - pixel) % LEDS_PER_RING)] = this_color;
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
void drip(){
  leds_all = CRGB::Black;
  short int ring,pixel, dif;
  light_pix = 0;
  // create blackscreen
  if(!start_flagg){
     for( ring = 0; ring < NUM_RINGS; ring++){
      if(ring < node_number*RINGS_PER_NODE || ring >= (node_number+1)*RINGS_PER_NODE) { continue; }
      for( pixel = 0; pixel < LEDS_PER_RING; pixel++){
        leds[get_1d_index(ring % RINGS_PER_NODE, pixel % LEDS_PER_RING)] = BLACK;
        start_flagg = 1;
      }
    }
  }
  // determine number of pixels to placed and light them up
  dif = light_pix;
  light_pix = edm_count/600;
  for(ring = 0; ring <NUM_RINGS; ring++){
    if(ring < node_number*RINGS_PER_NODE || ring >= (node_number+1)*RINGS_PER_NODE) { continue; }
    for(pixel = 0; pixel<light_pix; pixel++){
      leds[get_1d_index(ring % RINGS_PER_NODE, pixel % LEDS_PER_RING)] = get_mid_color(0);
    }
  }

  // check if 3 seconds has elapsed dif should only be  less than light pix if that amount of time has passed. 
  //if so add a new pixel from left to right
  if(dif<light_pix){
    for(ring = 0; ring < NUM_RINGS; ring++){
      if(ring < node_number*RINGS_PER_NODE || ring >= (node_number+1)*RINGS_PER_NODE) { continue; }
      for(pixel = LEDS_PER_RING;pixel>light_pix; pixel--){
        leds[get_1d_index(ring % RINGS_PER_NODE, pixel % LEDS_PER_RING)] = get_mid_color(0);
        leds[get_1d_index(ring % RINGS_PER_NODE, (pixel-1) % LEDS_PER_RING)] = BLACK;  //mid_layer[ring][pixel-1] = BLACK;
      }
    }
  }
}

void fire_chakra() {
  leds_all = CRGB::Black;
  clear_sparkle_layer();
  show_parameters[MID_INTRA_RING_MOTION_INDEX] = UP;
  int16_t chakra_cooling = 33 - edm_count / 500;
  if(chakra_cooling < 0) { chakra_cooling = 0; }
  fire(FIRE_PALETTE_STANDARD, true, 0, NUM_RINGS-1, chakra_cooling);
  write_pixel_data();
}

