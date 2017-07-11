
#ifdef DEBUG_TIMING
// For debugging only, writes all saved variables
void write_timing_output() {
  int num_serial_vals = 0;
  
  #ifdef DEBUG_TIMING
    Serial.print("Timing:");
    num_serial_vals = 9;
  #endif
  
  for(int i = 0; i < num_serial_vals; i++) {
    Serial.print("\t" + String(serial_val[i]));
  }

  if(num_serial_vals > 0)
  {
    Serial.println();
    Serial.flush(); // Guarantees buffer will write if future code hangs
  }
}
#endif

#ifdef DEBUG_LED_ARRAYS
// "Unit tests" to verify LEDs arrays and palettes work correctly
void run_led_array_tests() {
  // Test 1: Confirms arrays don't overwrite each other and that get_sparkle() works
  Serial.println("----Test 1-----");
  Serial.flush();
  for(uint8_t ring = 0; ring < NUM_RINGS; ring++) {
    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      if(pixel < LEDS_PER_RING/2) {
        sparkle_layer[ring][pixel] = 0x12;
        mid_layer[ring][pixel] = 0x34;
        base_layer[ring][pixel] = 0x56;
      }
      else {
        mid_layer[ring][pixel] = 0x78;
        base_layer[ring][pixel] = 0x91;
      }
    }
  }

  for(uint8_t ring = 0; ring < NUM_RINGS; ring++) {
    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      if(pixel < LEDS_PER_RING/2) {
        if(sparkle_layer[ring][pixel] != 0x12)
          Serial.println("sparkle_layer[" + String(ring) + "][" + String(pixel) + "] = " + String(sparkle_layer[ring][pixel]));

        if(mid_layer[ring][pixel] != 0x34)
          Serial.println("mid_layer[" + String(ring) + "][" + String(pixel) + "] = " + String(mid_layer[ring][pixel]));

        if(base_layer[ring][pixel] != 0x56)
          Serial.println("base_layer[" + String(ring) + "][" + String(pixel) + "] = " + String(base_layer[ring][pixel]));
      }
      else {
        if(mid_layer[ring][pixel] != 0x78)
          Serial.println("mid_layer[" + String(ring) + "][" + String(pixel) + "] = " + String(mid_layer[ring][pixel]));
          
        if(base_layer[ring][pixel] != 0x91)
          Serial.println("base_layer[" + String(ring) + "][" + String(pixel) + "] = " + String(base_layer[ring][pixel]));
      }
      
      if(pixel % 2 == 0) {
        if(get_sparkle(ring, pixel) != 0x1)
          Serial.println("get_sparkle(" + String(ring) + ", " + String(pixel) + ") = " + String(get_sparkle(ring, pixel)));
      }
      else {
        if(get_sparkle(ring, pixel) != 0x2)
          Serial.println("get_sparkle(" + String(ring) + ", " + String(pixel) + ") = " + String(get_sparkle(ring, pixel)));
      }
    }
  }


  // Test 2: Confirms set_sparkle() writes to correct location; relies on get_sparkle() working correctly
  Serial.println("----Test 2-----");
  uint8_t counter = 0;
  for(uint8_t ring = 0; ring < NUM_RINGS; ring++) {
    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      uint8_t valToWrite = 1 + (counter++ % 15);
      set_sparkle(ring, pixel, valToWrite);
      if(get_sparkle(ring, pixel) != valToWrite)
        Serial.println("set_sparkle to " + String(valToWrite) + ", get_sparkle(" + String(ring) + ", " + String(pixel) + ") = " + String(get_sparkle(ring, pixel)));
    }
  }


  // --- Init simple test color palette ---
  CRGB test_col0 = CRGB(255, 200, 128);
  CRGB test_col1 = CRGB(230, 95, 180);
  CRGB test_colors[NUM_COLORS_PER_PALETTE] = { CRGB(0,0,0), CRGB(0,0,0), CRGB(0,0,0), CRGB(0,0,0), CRGB(0,0,0), test_col0, test_col1 };
  set_palettes((uint8_t*) test_colors);

  
  // Test 3: Confirms sparkle_palette_target is initialized as intended
  Serial.println("----Test 3-----");
  if(sparkle_palette_target.entries[0] != CRGB(0,0,0))
    Serial.println("sparkle_palette_target.entries[0] = (" + String(sparkle_palette_target.entries[0].r) + "," + String(sparkle_palette_target.entries[0].g) + "," + String(sparkle_palette_target.entries[0].b) + ")");
  if(sparkle_palette_target.entries[1] != CRGB(255,255,255))
    Serial.println("sparkle_palette_target.entries[1] = (" + String(sparkle_palette_target.entries[1].r) + "," + String(sparkle_palette_target.entries[1].g) + "," + String(sparkle_palette_target.entries[1].b) + ")");
  if(sparkle_palette.entries[0] != CRGB(0,0,0))
    Serial.println("sparkle_palette.entries[0] = (" + String(sparkle_palette.entries[0].r) + "," + String(sparkle_palette.entries[0].g) + "," + String(sparkle_palette.entries[0].b) + ")");
  if(sparkle_palette.entries[1] != CRGB(255,255,255))
    Serial.println("sparkle_palette.entries[1] = (" + String(sparkle_palette.entries[1].r) + "," + String(sparkle_palette.entries[1].g) + "," + String(sparkle_palette.entries[1].b) + ")");
  /*  
  for(uint8_t i = 0; i < 7; i++) {
    CRGB exp_col0 = CRGB(test_col0.r >> i, test_col0.g >> i, test_col0.b >> i);
    CRGB exp_col1 = CRGB(test_col1.r >> i, test_col1.g >> i, test_col1.b >> i);
    
    if(sparkle_palette_target.entries[2 + i] != exp_col0)
      Serial.println("sparkle_palette_target.entries[" + String(2 + i) + "] = " + String(sparkle_palette_target.entries[2 + i].r) + "," + String(sparkle_palette_target.entries[2 + i].g) + "," + String(sparkle_palette_target.entries[2 + i].b) + ")");
    if(sparkle_palette_target.entries[9 + i] != exp_col1)
      Serial.println("sparkle_palette_target.entries[" + String(9 + i) + "] = " + String(sparkle_palette_target.entries[9 + i].r) + "," + String(sparkle_palette_target.entries[9 + i].g) + "," + String(sparkle_palette_target.entries[9 + i].b) + ")");
  }*/


  // Test 4: Confirm sparkle_palette blends upward into sparkle_palette_target, max 128 cycles (255/2)
  Serial.println("----Test 4-----");
  for(uint8_t i = 2; i < 16; i++) {
    if(sparkle_palette.entries[i] != CRGB(0,0,0)) {
      Serial.println("sparkle_palette.entries[" + String(i) + "] initialized to (" + String(sparkle_palette.entries[i].r) + "," + String(sparkle_palette.entries[i].g) + "," + String(sparkle_palette.entries[i].b) + ")");
    }
  }

  for(uint8_t i = 0; i < 128; i++) {
    blend_palette_16(sparkle_palette, sparkle_palette_target, PALETTE_MAX_CHANGES);
  }

  for(uint8_t i = 0; i < 16; i++) {
    if(sparkle_palette.entries[i] != sparkle_palette_target.entries[i]) {
      Serial.print("sparkle_palette.entries[" + String(i) + "] = (" + String(sparkle_palette.entries[i].r) + "," + String(sparkle_palette.entries[i].g) + "," + String(sparkle_palette.entries[i].b) + "), ");
      Serial.println("sparkle_palette_target.entries[" + String(i) + "] = (" + String(sparkle_palette_target.entries[i].r) + "," + String(sparkle_palette_target.entries[i].g) + "," + String(sparkle_palette_target.entries[i].b) + ")");
    }
  }


  // Test 5: Confirm sparkle_palette blends downward into sparkle_palette_target, max 64 cycles (255/4)
  Serial.println("----Test 5-----");
  for(uint8_t i = 0; i < 16; i++) {
    sparkle_palette_target.entries[i] = CRGB(0,0,0);
  }

  for(uint8_t i = 0; i < 64; i++) {
    blend_palette_16(sparkle_palette, sparkle_palette_target, PALETTE_MAX_CHANGES);
  }

  for(uint8_t i = 0; i < 16; i++) {
    if(sparkle_palette.entries[i] != sparkle_palette_target.entries[i]) {
      Serial.print("sparkle_palette.entries[" + String(i) + "] = (" + String(sparkle_palette.entries[i].r) + "," + String(sparkle_palette.entries[i].g) + "," + String(sparkle_palette.entries[i].b) + "), ");
      Serial.println("sparkle_palette_target.entries[" + String(i) + "] = (" + String(sparkle_palette_target.entries[i].r) + "," + String(sparkle_palette_target.entries[i].g) + "," + String(sparkle_palette_target.entries[i].b) + ")");
    }
  }


  // Reset sparkle test colors
  test_colors[5] = test_col0;
  test_colors[6] = test_col1;
  set_palettes((uint8_t*) test_colors);
  for(uint8_t i = 0; i < 128; i++) {
    blend_palette_16(sparkle_palette, sparkle_palette_target, PALETTE_MAX_CHANGES);
  }

/*
  // Test 6: Sparkle layer dimming parameter
  Serial.println("----Test 6-----");
  for(uint8_t dim = 0; dim < 6; dim++) {
    if(sparkle_palette.entries[get_sparkle_color(0, dim)] != CRGB(test_col0.r >> dim, test_col0.g >> dim, test_col0.b >> dim)) {
      Serial.println("sparkle_palette.entries[get_sparkle_color(0, " + String(dim) + ") = (" + String(sparkle_palette.entries[get_sparkle_color(0, dim)].r) + "," + String(sparkle_palette.entries[get_sparkle_color(0, dim)].g) + "," + String(sparkle_palette.entries[get_sparkle_color(0, dim)].b) + ")");
    }
    if(sparkle_palette.entries[get_sparkle_color(1, dim)] != CRGB(test_col1.r >> dim, test_col1.g >> dim, test_col1.b >> dim)) {
      Serial.println("sparkle_palette.entries[get_sparkle_color(1, " + String(dim) + ") = (" + String(sparkle_palette.entries[get_sparkle_color(1, dim)].r) + "," + String(sparkle_palette.entries[get_sparkle_color(1, dim)].g) + "," + String(sparkle_palette.entries[get_sparkle_color(1, dim)].b) + ")");
    }
  }*/
  
  
  // Test 7: Confirm sparkle layer writes correct values to LED array
  Serial.println("----Test 7-----");

  for(uint8_t node = 0; node < NUM_NODES; node++) {
    node_number = node;

    for(uint16_t i = 0; i < PHYSICAL_LEDS_PER_NODE; i++) {
      leds[i] = CRGB(0,0,0);
    }
    
    for(uint8_t ring = 0; ring < NUM_RINGS; ring++) {
      for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        if(ring >= node*RINGS_PER_NODE && ring < (node+1)*RINGS_PER_NODE) {
          set_sparkle(ring, pixel, get_sparkle_color((pixel + ring - node*RINGS_PER_NODE + node) % 2, (pixel + ring - node*RINGS_PER_NODE + node) % 7));
        }
        else {
          // Clear all other nodes
          set_sparkle(ring, pixel, BLACK);
        }
      }
    }
    
    write_pixel_data();

    // Strip 1
    uint16_t curPixel = LEDS_PER_STRIP - 1;
    for(uint8_t ring = 0; ring < 2*RINGS_PER_STRIP; ring+=2) {
      for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        CRGB exp_col = sparkle_palette.entries[get_sparkle_color((pixel+ring+node) % 2, (pixel+ring+node) % 7)];
        if(leds[curPixel] != exp_col) {
          Serial.print("node " + String(node) + ", leds[" + String(curPixel) + "] = (" + String(leds[curPixel].r) + ", " + String(leds[curPixel].g) + ", " + String(leds[curPixel].b) + "), ");
          Serial.println("sparkle_palette.entries[get_sparkle_color(" + String((pixel+ring+node) % 2) + ", " + String((pixel+ring+node) % 7) + ")], ring " + String(ring) + ", pixel " + String(pixel) + " = (" + String(exp_col.r) + ", " + String(exp_col.g) + ", " + String(exp_col.b) + ")");
        }
        curPixel--;
      }
      curPixel -= (PHYSICAL_LEDS_PER_RING - LEDS_PER_RING); // Skip over hidden pixels
    }

    // Strip 2
    curPixel = 2*LEDS_PER_STRIP - 1;
    for(uint8_t ring = 1; ring < 1+2*RINGS_PER_STRIP; ring+=2) {
      for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        CRGB exp_col = sparkle_palette.entries[get_sparkle_color((pixel+ring+node) % 2, (pixel+ring+node) % 7)];
        if(leds[curPixel] != exp_col) {
          Serial.print("node " + String(node) + ", leds[" + String(curPixel) + "] = (" + String(leds[curPixel].r) + ", " + String(leds[curPixel].g) + ", " + String(leds[curPixel].b) + "), ");
          Serial.println("sparkle_palette.entries[get_sparkle_color(" + String((pixel+ring+node) % 2) + ", " + String((pixel+ring+node) % 7) + ")], ring " + String(ring) + ", pixel " + String(pixel) + " = (" + String(exp_col.r) + ", " + String(exp_col.g) + ", " + String(exp_col.b) + ")");
        }
        curPixel--;
      }
      curPixel -= (PHYSICAL_LEDS_PER_RING - LEDS_PER_RING); // Skip over hidden pixels
    }

    // Strip 3
    curPixel = 2*LEDS_PER_STRIP + 1;
    for(uint8_t ring = 2*RINGS_PER_STRIP; ring < 4*RINGS_PER_STRIP; ring+=2) {
      for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        CRGB exp_col = sparkle_palette.entries[get_sparkle_color((pixel+ring+node) % 2, (pixel+ring+node) % 7)];
        if(leds[curPixel] != exp_col) {
          Serial.print("node " + String(node) + ", leds[" + String(curPixel) + "] = (" + String(leds[curPixel].r) + ", " + String(leds[curPixel].g) + ", " + String(leds[curPixel].b) + "), ");
          Serial.println("sparkle_palette.entries[get_sparkle_color(" + String((pixel+ring+node) % 2) + ", " + String((pixel+ring+node) % 7) + ")], ring " + String(ring) + ", pixel " + String(pixel) + " = (" + String(exp_col.r) + ", " + String(exp_col.g) + ", " + String(exp_col.b) + ")");
        }
        curPixel++;
      }
      curPixel += (PHYSICAL_LEDS_PER_RING - LEDS_PER_RING); // Skip over hidden pixels
    }

    // Strip 4
    curPixel = 3*LEDS_PER_STRIP + 1;
    for(uint8_t ring = 1+2*RINGS_PER_STRIP; ring < 1+4*RINGS_PER_STRIP; ring+=2) {
      for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        CRGB exp_col = sparkle_palette.entries[get_sparkle_color((pixel+ring+node) % 2, (pixel+ring+node) % 7)];
        if(leds[curPixel] != exp_col) {
          Serial.print("node " + String(node) + ", leds[" + String(curPixel) + "] = (" + String(leds[curPixel].r) + ", " + String(leds[curPixel].g) + ", " + String(leds[curPixel].b) + "), ");
          Serial.println("sparkle_palette.entries[get_sparkle_color(" + String((pixel+ring+node) % 2) + ", " + String((pixel+ring+node) % 7) + ")], ring " + String(ring) + ", pixel " + String(pixel) + " = (" + String(exp_col.r) + ", " + String(exp_col.g) + ", " + String(exp_col.b) + ")");
        }
        curPixel++;
      }
      curPixel += (PHYSICAL_LEDS_PER_RING - LEDS_PER_RING); // Skip over hidden pixels
    }
  }


  // Init base layer test colors
  test_col0 = CRGB(1, 255, 128);
  test_col1 = CRGB(128, 2, 254);
  test_colors[0] = test_col0;
  test_colors[1] = test_col1;
  set_palettes((uint8_t*) test_colors);


  // Test 8: Verify base layer initialization, gradients and dimming in the base_palette_target array
  Serial.println("----Test 8-----");
  if(base_palette_target.entries[0] != CRGB(0,0,0))
    Serial.println("base_palette_target.entries[0] = (" + String(base_palette_target.entries[0].r) + "," + String(base_palette_target.entries[0].g) + "," + String(base_palette_target.entries[0].b) + ")");
  if(base_palette_target.entries[1] != CRGB(255,255,255))
    Serial.println("base_palette_target.entries[1] = (" + String(base_palette_target.entries[1].r) + "," + String(base_palette_target.entries[1].g) + "," + String(base_palette_target.entries[1].b) + ")");
  if(base_palette_target.entries[2] != CRGB(0,0,0))
    Serial.println("base_palette_target.entries[2] = (" + String(base_palette_target.entries[2].r) + "," + String(base_palette_target.entries[2].g) + "," + String(base_palette_target.entries[2].b) + ")");
  if(base_palette_target.entries[3] != CRGB(128,128,128))
    Serial.println("base_palette_target.entries[3] = (" + String(base_palette_target.entries[3].r) + "," + String(base_palette_target.entries[3].g) + "," + String(base_palette_target.entries[3].b) + ")");
  if(base_palette.entries[0] != CRGB(0,0,0))
    Serial.println("base_palette.entries[0] = (" + String(base_palette.entries[0].r) + "," + String(base_palette.entries[0].g) + "," + String(base_palette.entries[0].b) + ")");
  if(base_palette.entries[1] != CRGB(255,255,255))
    Serial.println("base_palette.entries[1] = (" + String(base_palette.entries[1].r) + "," + String(base_palette.entries[1].g) + "," + String(base_palette.entries[1].b) + ")");
  if(base_palette.entries[2] != CRGB(0,0,0))
    Serial.println("base_palette.entries[2] = (" + String(base_palette.entries[2].r) + "," + String(base_palette.entries[2].g) + "," + String(base_palette.entries[2].b) + ")");
  if(base_palette.entries[3] != CRGB(128,128,128))
    Serial.println("base_palette.entries[3] = (" + String(base_palette.entries[3].r) + "," + String(base_palette.entries[3].g) + "," + String(base_palette.entries[3].b) + ")");
    
  for(uint16_t i = 4; i < 255; i+=7) {
    CRGB exp_col_gradient = test_col0;
    nblend(exp_col_gradient, test_col1, 255 * ((i-4)/7) / 35);
    if(base_palette_target.entries[i] != exp_col_gradient) {
      Serial.println("gradient: base_palette_target.entries[" + String(i) + "] = (" + String(base_palette_target.entries[i].r) + "," + String(base_palette_target.entries[i].g) + "," + String(base_palette_target.entries[i].b) + ")");
    }
    /*
    for(uint8_t j = 0; j < 7; j++) {
      CRGB exp_col_dim = CRGB(base_palette_target.entries[i].r >> j, base_palette_target.entries[i].g >> j, base_palette_target.entries[i].b >> j);
      if(base_palette_target.entries[j + i] != exp_col_dim) {
        Serial.println("dim: base_palette_target.entries[" + String(i+j) + "] = (" + String(base_palette_target.entries[i+j].r) + "," + String(base_palette_target.entries[i+j].g) + "," + String(base_palette_target.entries[i+j].b) + ")");
      }
    }*/
  }

/*
  // Test 9: Check base layer dimming parameter against expected color
  Serial.println("----Test 9-----");
  for(uint8_t dim = 0; dim < 7; dim++) {
    if(base_palette_target.entries[get_base_color(0, dim)] != CRGB(test_col0.r >> dim, test_col0.g >> dim, test_col0.b >> dim)) {
      Serial.println("base_palette_target.entries[get_base_color(0, " + String(dim) + ") = base_palette_target.entries[" + String(get_base_color(0, dim)) + "] = (" + String(base_palette_target.entries[get_base_color(0, dim)].r) + "," + String(base_palette_target.entries[get_base_color(0, dim)].g) + "," + String(base_palette_target.entries[get_base_color(0, dim)].b) + ")");
    }
    if(base_palette_target.entries[get_base_color(1, dim)] != CRGB(test_col1.r >> dim, test_col1.g >> dim, test_col1.b >> dim)) {
      Serial.println("base_palette_target.entries[get_base_color(1, " + String(dim) + ") = base_palette_target.entries[" + String(get_base_color(1, dim)) + "] = (" + String(base_palette_target.entries[get_base_color(1, dim)].r) + "," + String(base_palette_target.entries[get_base_color(1, dim)].g) + "," + String(base_palette_target.entries[get_base_color(1, dim)].b) + ")");
    }
  }
*/

  // Test 10: Check base layer forward gradients
  Serial.println("----Test 10----");
  for(uint8_t gradient = 0; gradient < 36; gradient++) {
    CRGB exp_col_grad = test_col0;
    nblend(exp_col_grad, test_col1, 255*gradient/35);
    if(base_palette_target.entries[get_base_color(0, 1, gradient)] != exp_col_grad) {
      Serial.println("base_palette_target.entries[get_base_color(0, 1, " + String(gradient) + ")] = base_palette_target.entries[" + String(get_base_color(0, 1, gradient)) + "] = (" + String(base_palette_target.entries[get_base_color(0, 1, gradient)].r) + "," + String(base_palette_target.entries[get_base_color(0, 1, gradient)].g) + "," + String(base_palette_target.entries[get_base_color(0, 1, gradient)].b) + ")");
    }
  }

  
  // Test 11: Check base layer backward gradients
  Serial.println("----Test 11----");
  for(uint8_t gradient = 0; gradient < 36; gradient++) {
    if(get_base_color(1, 0, gradient) != get_base_color(0, 1, 35-gradient)) {
      Serial.println("get_base_color(1, 0, " + String(gradient) + ") = " + String(get_base_color(1, 0, gradient)) + ", get_base_color(0, 1, " + String(35-gradient) + ") = " + String(get_base_color(0, 1, 35-gradient)));
    }
  }
  
  
  // Test 12: Confirm base_palette blends upward into base_palette_target, currently over 210 cycles (could be higher if several LEDs moving a long distance)
  Serial.println("----Test 12----");
  for(uint16_t i = 4; i < 256; i++) {
    if(base_palette.entries[i] != CRGB(0,0,0)) {
      Serial.println("base_palette.entries[" + String(i) + "] initialized to (" + String(base_palette.entries[i].r) + "," + String(base_palette.entries[i].g) + "," + String(base_palette.entries[i].b) + ")");
    }
  }

  for(uint8_t i = 0; i < 210; i++) {
    blend_palette_256(base_palette, base_palette_target, PALETTE_MAX_CHANGES);
  }

  for(uint16_t i = 4; i < 256; i++) {
    if(base_palette.entries[i] != base_palette_target.entries[i]) {
      Serial.print("base_palette.entries[" + String(i) + "] = (" + String(base_palette.entries[i].r) + "," + String(base_palette.entries[i].g) + "," + String(base_palette.entries[i].b) + "), ");
      Serial.println("base_palette_target.entries[" + String(i) + "] = (" + String(base_palette_target.entries[i].r) + "," + String(base_palette_target.entries[i].g) + "," + String(base_palette_target.entries[i].b) + ")");
    }
  }


  // Test 13: Confirm base_palette blends downward into base_palette_target, currently 110 cycles (changes based on # pixels traveling long distances)
  Serial.println("----Test 13----");
  for(uint16_t i = 0; i < 256; i++) {
    base_palette_target.entries[i] = CRGB(0,0,0);
  }

  for(uint8_t i = 0; i < 110; i++) {
    blend_palette_256(base_palette, base_palette_target, PALETTE_MAX_CHANGES);
  }

  for(uint16_t i = 0; i < 256; i++) {
    if(base_palette.entries[i] != base_palette_target.entries[i]) {
      Serial.print("base_palette.entries[" + String(i) + "] = (" + String(base_palette.entries[i].r) + "," + String(base_palette.entries[i].g) + "," + String(base_palette.entries[i].b) + "), ");
      Serial.println("base_palette_target.entries[" + String(i) + "] = (" + String(base_palette_target.entries[i].r) + "," + String(base_palette_target.entries[i].g) + "," + String(base_palette_target.entries[i].b) + ")");
    }
  }


  // Reset base test colors
  test_colors[0] = test_col0;
  test_colors[1] = test_col1;
  set_palettes((uint8_t*) test_colors);
  for(uint8_t i = 0; i < 255; i++) {
    blend_palette_256(base_palette, base_palette_target, PALETTE_MAX_CHANGES);
  }

  
  // Test 14: Confirm base layer correctly writes node's data
  Serial.println("----Test 14----");

  for(uint8_t ring = 0; ring < NUM_RINGS; ring++) {
    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      set_sparkle(ring, pixel, TRANSPARENT);
      mid_layer[ring][pixel] = TRANSPARENT;
    }
  }
  
  for(uint8_t node = 0; node < NUM_NODES; node++) {
    node_number = node;

    for(uint16_t i = 0; i < PHYSICAL_LEDS_PER_NODE; i++) {
      leds[i] = CRGB(0,0,0);
    }
    
    for(uint8_t ring = 0; ring < NUM_RINGS; ring++) {
      for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        if(ring >= node*RINGS_PER_NODE && ring < (node+1)*RINGS_PER_NODE) {
          base_layer[ring][pixel] = get_base_color((pixel + ring - node*RINGS_PER_NODE + node) % 2, (pixel + ring - node*RINGS_PER_NODE + node + 1) % 2, (pixel + ring - node*RINGS_PER_NODE + node) % 36, (pixel + ring - node*RINGS_PER_NODE + node) % 7);
        }
        else {
          // Clear all other nodes
          base_layer[ring][pixel] = BLACK;
        }
      }
    }
    
    write_pixel_data();

    // Strip 1
    uint16_t curPixel = LEDS_PER_STRIP - 1;
    for(uint8_t ring = 0; ring < 2*RINGS_PER_STRIP; ring+=2) {
      for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        CRGB exp_col = base_palette.entries[get_base_color((pixel+ring+node) % 2, (pixel+ring+node+1) % 2, (pixel+ring+node) % 36, (pixel+ring+node) % 7)];
        if(leds[curPixel] != exp_col) {
          Serial.print("node " + String(node) + ", leds[" + String(curPixel) + "] = (" + String(leds[curPixel].r) + ", " + String(leds[curPixel].g) + ", " + String(leds[curPixel].b) + "), ");
          Serial.println("base_palette.entries[get_base_color(" + String((pixel+ring+node) % 2) + ", " + String((pixel+ring+node) % 36) + ", " + String((pixel+ring+node) % 7) + ")], ring " + String(ring) + ", pixel " + String(pixel) + " = (" + String(exp_col.r) + ", " + String(exp_col.g) + ", " + String(exp_col.b) + ")");
        }
        curPixel--;
      }
      curPixel -= (PHYSICAL_LEDS_PER_RING - LEDS_PER_RING); // Skip over hidden pixels
    }

    // Strip 2
    curPixel = 2*LEDS_PER_STRIP - 1;
    for(uint8_t ring = 1; ring < 1+2*RINGS_PER_STRIP; ring+=2) {
      for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        CRGB exp_col = base_palette.entries[get_base_color((pixel+ring+node) % 2, (pixel+ring+node+1) % 2, (pixel+ring+node) % 36, (pixel+ring+node) % 7)];
        if(leds[curPixel] != exp_col) {
          Serial.print("node " + String(node) + ", leds[" + String(curPixel) + "] = (" + String(leds[curPixel].r) + ", " + String(leds[curPixel].g) + ", " + String(leds[curPixel].b) + "), ");
          Serial.println("base_palette.entries[get_base_color(" + String((pixel+ring+node) % 2) + ", " + String((pixel+ring+node) % 36) + ", " + String((pixel+ring+node) % 7) + ")], ring " + String(ring) + ", pixel " + String(pixel) + " = (" + String(exp_col.r) + ", " + String(exp_col.g) + ", " + String(exp_col.b) + ")");
        }
        curPixel--;
      }
      curPixel -= (PHYSICAL_LEDS_PER_RING - LEDS_PER_RING); // Skip over hidden pixels
    }

    // Strip 3
    curPixel = 2*LEDS_PER_STRIP + 1;
    for(uint8_t ring = 2*RINGS_PER_STRIP; ring < 4*RINGS_PER_STRIP; ring+=2) {
      for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        CRGB exp_col = base_palette.entries[get_base_color((pixel+ring+node) % 2, (pixel+ring+node+1) % 2, (pixel+ring+node) % 36, (pixel+ring+node) % 7)];
        if(leds[curPixel] != exp_col) {
          Serial.print("node " + String(node) + ", leds[" + String(curPixel) + "] = (" + String(leds[curPixel].r) + ", " + String(leds[curPixel].g) + ", " + String(leds[curPixel].b) + "), ");
          Serial.println("base_palette.entries[get_base_color(" + String((pixel+ring+node) % 2) + ", " + String((pixel+ring+node) % 36) + ", " + String((pixel+ring+node) % 7) + ")], ring " + String(ring) + ", pixel " + String(pixel) + " = (" + String(exp_col.r) + ", " + String(exp_col.g) + ", " + String(exp_col.b) + ")");
        }
        curPixel++;
      }
      curPixel += (PHYSICAL_LEDS_PER_RING - LEDS_PER_RING); // Skip over hidden pixels
    }

    // Strip 4
    curPixel = 3*LEDS_PER_STRIP + 1;
    for(uint8_t ring = 1+2*RINGS_PER_STRIP; ring < 1+4*RINGS_PER_STRIP; ring+=2) {
      for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        CRGB exp_col = base_palette.entries[get_base_color((pixel+ring+node) % 2, (pixel+ring+node+1) % 2, (pixel+ring+node) % 36, (pixel+ring+node) % 7)];
        if(leds[curPixel] != exp_col) {
          Serial.print("node " + String(node) + ", leds[" + String(curPixel) + "] = (" + String(leds[curPixel].r) + ", " + String(leds[curPixel].g) + ", " + String(leds[curPixel].b) + "), ");
          Serial.println("base_palette.entries[get_base_color(" + String((pixel+ring+node) % 2) + ", " + String((pixel+ring+node) % 36) + ", " + String((pixel+ring+node) % 7) + ")], ring " + String(ring) + ", pixel " + String(pixel) + " = (" + String(exp_col.r) + ", " + String(exp_col.g) + ", " + String(exp_col.b) + ")");
        }
        curPixel++;
      }
      curPixel += (PHYSICAL_LEDS_PER_RING - LEDS_PER_RING); // Skip over hidden pixels
    }
  }
  

  // Init mid layer test colors
  test_col0 = CRGB(190, 210, 255);
  test_col1 = CRGB(150, 255, 222);
  CRGB test_col2 = CRGB(255, 0, 127);
  test_colors[2] = test_col0;
  test_colors[3] = test_col1;
  test_colors[4] = test_col2;
  set_palettes((uint8_t*) test_colors);

  
  // Test 15: Verify mid layer initialization, gradients and dimming in the mid_palette_target array
  Serial.println("----Test 15----");
  if(mid_palette_target.entries[0] != CRGB(0,0,0))
    Serial.println("mid_palette_target.entries[0] = (" + String(mid_palette_target.entries[0].r) + "," + String(mid_palette_target.entries[0].g) + "," + String(mid_palette_target.entries[0].b) + ")");
  if(mid_palette_target.entries[1] != CRGB(255,255,255))
    Serial.println("mid_palette_target.entries[1] = (" + String(mid_palette_target.entries[1].r) + "," + String(mid_palette_target.entries[1].g) + "," + String(mid_palette_target.entries[1].b) + ")");
  if(mid_palette_target.entries[2] != CRGB(0,0,0))
    Serial.println("mid_palette_target.entries[2] = (" + String(mid_palette_target.entries[2].r) + "," + String(mid_palette_target.entries[2].g) + "," + String(mid_palette_target.entries[2].b) + ")");
  if(mid_palette_target.entries[3] != CRGB(128,128,128))
    Serial.println("mid_palette_target.entries[3] = (" + String(mid_palette_target.entries[3].r) + "," + String(mid_palette_target.entries[3].g) + "," + String(mid_palette_target.entries[3].b) + ")");
  if(mid_palette.entries[0] != CRGB(0,0,0))
    Serial.println("mid_palette.entries[0] = (" + String(mid_palette.entries[0].r) + "," + String(mid_palette.entries[0].g) + "," + String(mid_palette.entries[0].b) + ")");
  if(mid_palette.entries[1] != CRGB(255,255,255))
    Serial.println("mid_palette.entries[1] = (" + String(mid_palette.entries[1].r) + "," + String(mid_palette.entries[1].g) + "," + String(mid_palette.entries[1].b) + ")");
  if(mid_palette.entries[2] != CRGB(0,0,0))
    Serial.println("mid_palette.entries[2] = (" + String(mid_palette.entries[2].r) + "," + String(mid_palette.entries[2].g) + "," + String(mid_palette.entries[2].b) + ")");
  if(mid_palette.entries[3] != CRGB(128,128,128))
    Serial.println("mid_palette.entries[3] = (" + String(mid_palette.entries[3].r) + "," + String(mid_palette.entries[3].g) + "," + String(mid_palette.entries[3].b) + ")");
    
  for(uint16_t i = 0; i < 12; i++) {
    CRGB exp_col_gradient = test_col0;
    nblend(exp_col_gradient, test_col1, 255 * i / 11);
    if(mid_palette_target.entries[4+7*i] != exp_col_gradient) {
      Serial.println("gradient: mid_palette_target.entries[" + String(4+7*i) + "] = (" + String(mid_palette_target.entries[4+7*i].r) + "," + String(mid_palette_target.entries[4+7*i].g) + "," + String(mid_palette_target.entries[4+7*i].b) + ")");
    }/*
    for(uint8_t j = 0; j < 7; j++) {
      CRGB exp_col_dim = CRGB(mid_palette_target.entries[4+7*i].r >> j, mid_palette_target.entries[4+7*i].g >> j, mid_palette_target.entries[4+7*i].b >> j);
      if(mid_palette_target.entries[4+7*i + j] != exp_col_dim) {
        Serial.println("dim: mid_palette_target.entries[" + String(4+7*i+j) + "] = (" + String(mid_palette_target.entries[4+7*i+j].r) + "," + String(mid_palette_target.entries[4+7*i+j].g) + "," + String(mid_palette_target.entries[4+7*i+j].b) + ")");
      }
    }*/

    exp_col_gradient = test_col0;
    nblend(exp_col_gradient, test_col2, 255 * i / 11);
    if(mid_palette_target.entries[88+7*i] != exp_col_gradient) {
      Serial.println("gradient: mid_palette_target.entries[" + String(88+7*i) + "] = (" + String(mid_palette_target.entries[88+7*i].r) + "," + String(mid_palette_target.entries[88+7*i].g) + "," + String(mid_palette_target.entries[88+7*i].b) + ")");
    }/*
    for(uint8_t j = 0; j < 7; j++) {
      CRGB exp_col_dim = CRGB(mid_palette_target.entries[88+7*i].r >> j, mid_palette_target.entries[88+7*i].g >> j, mid_palette_target.entries[88+7*i].b >> j);
      if(mid_palette_target.entries[88+7*i + j] != exp_col_dim) {
        Serial.println("dim: mid_palette_target.entries[" + String(88+7*i+j) + "] = (" + String(mid_palette_target.entries[88+7*i+j].r) + "," + String(mid_palette_target.entries[88+7*i+j].g) + "," + String(mid_palette_target.entries[88+7*i+j].b) + ")");
      }
    }*/

    exp_col_gradient = test_col1;
    nblend(exp_col_gradient, test_col2, 255 * i / 11);
    if(mid_palette_target.entries[172+7*i] != exp_col_gradient) {
      Serial.println("gradient: mid_palette_target.entries[" + String(172+7*i) + "] = (" + String(mid_palette_target.entries[172+7*i].r) + "," + String(mid_palette_target.entries[172+7*i].g) + "," + String(mid_palette_target.entries[172+7*i].b) + ")");
    }/*
    for(uint8_t j = 0; j < 7; j++) {
      CRGB exp_col_dim = CRGB(mid_palette_target.entries[172+7*i].r >> j, mid_palette_target.entries[172+7*i].g >> j, mid_palette_target.entries[172+7*i].b >> j);
      if(mid_palette_target.entries[172+7*i + j] != exp_col_dim) {
        Serial.println("dim: mid_palette_target.entries[" + String(172+7*i+j) + "] = (" + String(mid_palette_target.entries[172+7*i+j].r) + "," + String(mid_palette_target.entries[172+7*i+j].g) + "," + String(mid_palette_target.entries[172+7*i+j].b) + ")");
      }
    }*/
  }


/*
  // Test 16: Check mid layer dimming parameter against expected color
  Serial.println("----Test 16----");
  for(uint8_t dim = 0; dim < 7; dim++) {
    if(mid_palette_target.entries[get_mid_color(0, 0, 0, dim)] != CRGB(test_col0.r >> dim, test_col0.g >> dim, test_col0.b >> dim)) {
      Serial.println("mid_palette_target.entries[get_mid_color(0, 0, 0, " + String(dim) + ") = mid_palette_target.entries[" + String(get_mid_color(0, 0, 0, dim)) + "] = (" + String(mid_palette_target.entries[get_mid_color(0, 0, 0, dim)].r) + "," + String(mid_palette_target.entries[get_mid_color(0, 0, 0, dim)].g) + "," + String(mid_palette_target.entries[get_mid_color(0, 0, 0, dim)].b) + ")");
    }
    if(mid_palette_target.entries[get_mid_color(1, 0, 0, dim)] != CRGB(test_col1.r >> dim, test_col1.g >> dim, test_col1.b >> dim)) {
      Serial.println("mid_palette_target.entries[get_mid_color(1, 0, 0, " + String(dim) + ") = mid_palette_target.entries[" + String(get_mid_color(1, 0, 0, dim)) + "] = (" + String(mid_palette_target.entries[get_mid_color(1, 0, 0, dim)].r) + "," + String(mid_palette_target.entries[get_mid_color(1, 0, 0, dim)].g) + "," + String(mid_palette_target.entries[get_mid_color(1, 0, 0, dim)].b) + ")");
    }
    if(mid_palette_target.entries[get_mid_color(2, 0, 0, dim)] != CRGB(test_col2.r >> dim, test_col2.g >> dim, test_col2.b >> dim)) {
      Serial.println("mid_palette_target.entries[get_mid_color(2, 0, 0, " + String(dim) + ") = mid_palette_target.entries[" + String(get_mid_color(2, 0, 0, dim)) + "] = (" + String(mid_palette_target.entries[get_mid_color(2, 0, 0, dim)].r) + "," + String(mid_palette_target.entries[get_mid_color(2, 0, 0, dim)].g) + "," + String(mid_palette_target.entries[get_mid_color(2, 0, 0, dim)].b) + ")");
    }
  }
*/

  // Test 17: Check mid layer forward gradients
  Serial.println("----Test 17----");
  for(uint8_t gradient = 0; gradient < 12; gradient++) {
    CRGB exp_col_grad = test_col0;
    nblend(exp_col_grad, test_col1, 255*gradient/11);
    if(mid_palette_target.entries[get_mid_color(0, 1, gradient, 0)] != exp_col_grad) {
      Serial.println("mid_palette_target.entries[get_mid_color(0, 1, " + String(gradient) + ", 0)] = mid_palette_target.entries[" + String(get_mid_color(0, 1, gradient, 0)) + "] = (" + String(mid_palette_target.entries[get_mid_color(0, 1, gradient, 0)].r) + "," + String(mid_palette_target.entries[get_mid_color(0, 1, gradient, 0)].g) + "," + String(mid_palette_target.entries[get_mid_color(0, 1, gradient, 0)].b) + ")");
    }

    exp_col_grad = test_col0;
    nblend(exp_col_grad, test_col2, 255*gradient/11);
    if(mid_palette_target.entries[get_mid_color(0, 2, gradient, 0)] != exp_col_grad) {
      Serial.println("mid_palette_target.entries[get_mid_color(0, 2, " + String(gradient) + ", 0)] = mid_palette_target.entries[" + String(get_mid_color(0, 2, gradient, 0)) + "] = (" + String(mid_palette_target.entries[get_mid_color(0, 2, gradient, 0)].r) + "," + String(mid_palette_target.entries[get_mid_color(0, 2, gradient, 0)].g) + "," + String(mid_palette_target.entries[get_mid_color(0, 2, gradient, 0)].b) + ")");
    }

    exp_col_grad = test_col1;
    nblend(exp_col_grad, test_col2, 255*gradient/11);
    if(mid_palette_target.entries[get_mid_color(1, 2, gradient, 0)] != exp_col_grad) {
      Serial.println("mid_palette_target.entries[get_mid_color(1, 2, " + String(gradient) + ", 0)] = mid_palette_target.entries[" + String(get_mid_color(0, 1, gradient, 0)) + "] = (" + String(mid_palette_target.entries[get_mid_color(1, 2, gradient, 0)].r) + "," + String(mid_palette_target.entries[get_mid_color(1, 2, gradient, 0)].g) + "," + String(mid_palette_target.entries[get_mid_color(1, 2, gradient, 0)].b) + ")");
    }
  }

  
  // Test 18: Check mid layer backward gradients
  Serial.println("----Test 18----");
  for(uint8_t gradient = 0; gradient < 12; gradient++) {
    if(get_mid_color(1, 0, gradient, 0) != get_mid_color(0, 1, 11-gradient, 0)) {
      Serial.println("get_mid_color(1, 0, " + String(gradient) + ", 0) = " + String(get_mid_color(1, 0, gradient, 0)) + ", get_mid_color(0, 1, " + String(12-gradient) + ", 0) = " + String(get_mid_color(0, 1, 12-gradient, 0)));
    }

    if(get_mid_color(2, 0, gradient, 0) != get_mid_color(0, 2, 11-gradient, 0)) {
      Serial.println("get_mid_color(2, 0, " + String(gradient) + ", 0) = " + String(get_mid_color(2, 0, gradient, 0)) + ", get_mid_color(0, 2, " + String(12-gradient) + ", 0) = " + String(get_mid_color(0, 2, 12-gradient, 0)));
    }

    if(get_mid_color(2, 1, gradient, 0) != get_mid_color(1, 2, 11-gradient, 0)) {
      Serial.println("get_mid_color(2, 1, " + String(gradient) + ", 0) = " + String(get_mid_color(2, 1, gradient, 0)) + ", get_mid_color(1, 2, " + String(12-gradient) + ", 0) = " + String(get_mid_color(1, 2, 12-gradient, 0)));
    }
  }

  
  // Test 19: Confirm mid_palette blends upward into mid_palette_target, currently over 255 cycles (could be higher if several LEDs moving a long distance)
  Serial.println("----Test 19----");
  for(uint16_t i = 4; i < 256; i++) {
    if(mid_palette.entries[i] != CRGB(0,0,0)) {
      Serial.println("mid_palette.entries[" + String(i) + "] initialized to (" + String(mid_palette.entries[i].r) + "," + String(mid_palette.entries[i].g) + "," + String(mid_palette.entries[i].b) + ")");
    }
  }

  for(uint16_t i = 0; i < 255; i++) {
    blend_palette_256(mid_palette, mid_palette_target, PALETTE_MAX_CHANGES);
  }

  for(uint16_t i = 4; i < 256; i++) {
    if(mid_palette.entries[i] != mid_palette_target.entries[i]) {
      Serial.print("mid_palette.entries[" + String(i) + "] = (" + String(mid_palette.entries[i].r) + "," + String(mid_palette.entries[i].g) + "," + String(mid_palette.entries[i].b) + "), ");
      Serial.println("mid_palette_target.entries[" + String(i) + "] = (" + String(mid_palette_target.entries[i].r) + "," + String(mid_palette_target.entries[i].g) + "," + String(mid_palette_target.entries[i].b) + ")");
    }
  }


  // Test 20: Confirm mid_palette blends downward into mid_palette_target, currently 150 cycles (changes based on # pixels traveling long distances)
  Serial.println("----Test 20----");
  for(uint16_t i = 0; i < 256; i++) {
    mid_palette_target.entries[i] = CRGB(0,0,0);
  }

  for(uint8_t i = 0; i < 150; i++) {
    blend_palette_256(mid_palette, mid_palette_target, PALETTE_MAX_CHANGES);
  }

  for(uint16_t i = 0; i < 256; i++) {
    if(mid_palette.entries[i] != mid_palette_target.entries[i]) {
      Serial.print("mid_palette.entries[" + String(i) + "] = (" + String(mid_palette.entries[i].r) + "," + String(mid_palette.entries[i].g) + "," + String(mid_palette.entries[i].b) + "), ");
      Serial.println("mid_palette_target.entries[" + String(i) + "] = (" + String(mid_palette_target.entries[i].r) + "," + String(mid_palette_target.entries[i].g) + "," + String(mid_palette_target.entries[i].b) + ")");
    }
  }


  // Reset mid test colors
  test_colors[2] = test_col0;
  test_colors[3] = test_col1;
  test_colors[4] = test_col2;
  set_palettes((uint8_t*) test_colors);
  for(uint8_t i = 0; i < 255; i++) {
    blend_palette_256(mid_palette, mid_palette_target, PALETTE_MAX_CHANGES);
  }

  
  // Test 21: Confirm mid layer correctly writes node's data
  Serial.println("----Test 21----");

  for(uint8_t ring = 0; ring < NUM_RINGS; ring++) {
    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      set_sparkle(ring, pixel, TRANSPARENT);
    }
  }
  
  for(uint8_t node = 0; node < NUM_NODES; node++) {
    node_number = node;

    for(uint16_t i = 0; i < PHYSICAL_LEDS_PER_NODE; i++) {
      leds[i] = CRGB(0,0,0);
    }
    
    for(uint8_t ring = 0; ring < NUM_RINGS; ring++) {
      for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        if(ring >= node*RINGS_PER_NODE && ring < (node+1)*RINGS_PER_NODE) {
          mid_layer[ring][pixel] = get_mid_color((pixel + ring - node*RINGS_PER_NODE + node) % 3, (pixel + ring - node*RINGS_PER_NODE + 2*node) % 3, (pixel + ring - node*RINGS_PER_NODE + node) % 12, (pixel + ring - node*RINGS_PER_NODE + node) % 7);
        }
        else {
          // Clear all other nodes
          mid_layer[ring][pixel] = BLACK;
        }
      }
    }
    
    write_pixel_data();

    // Strip 1
    uint16_t curPixel = LEDS_PER_STRIP - 1;
    for(uint8_t ring = 0; ring < 2*RINGS_PER_STRIP; ring+=2) {
      for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        CRGB exp_col = mid_palette.entries[get_mid_color((pixel+ring+node) % 3, (pixel+ring+2*node) % 3, (pixel+ring+node) % 12, (pixel+ring+node) % 7)];
        if(leds[curPixel] != exp_col) {
          Serial.print("node " + String(node) + ", leds[" + String(curPixel) + "] = (" + String(leds[curPixel].r) + ", " + String(leds[curPixel].g) + ", " + String(leds[curPixel].b) + "), ");
          Serial.println("mid_palette.entries[get_mid_color(" + String((pixel+ring+node) % 3) + ", " + String((pixel+ring+2*node) % 3) + ", " + String((pixel+ring+node) % 12) + ", " + String((pixel+ring+node) % 7) + ")], ring " + String(ring) + ", pixel " + String(pixel) + " = (" + String(exp_col.r) + ", " + String(exp_col.g) + ", " + String(exp_col.b) + ")");
        }
        curPixel--;
      }
      curPixel -= (PHYSICAL_LEDS_PER_RING - LEDS_PER_RING); // Skip over hidden pixels
    }

    // Strip 2
    curPixel = 2*LEDS_PER_STRIP - 1;
    for(uint8_t ring = 1; ring < 1+2*RINGS_PER_STRIP; ring+=2) {
      for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        CRGB exp_col = mid_palette.entries[get_mid_color((pixel+ring+node) % 3, (pixel+ring+2*node) % 3, (pixel+ring+node) % 12, (pixel+ring+node) % 7)];
        if(leds[curPixel] != exp_col) {
          Serial.print("node " + String(node) + ", leds[" + String(curPixel) + "] = (" + String(leds[curPixel].r) + ", " + String(leds[curPixel].g) + ", " + String(leds[curPixel].b) + "), ");
          Serial.println("mid_palette.entries[get_mid_color(" + String((pixel+ring+node) % 3) + ", " + String((pixel+ring+2*node) % 3) + ", " + String((pixel+ring+node) % 12) + ", " + String((pixel+ring+node) % 7) + ")], ring " + String(ring) + ", pixel " + String(pixel) + " = (" + String(exp_col.r) + ", " + String(exp_col.g) + ", " + String(exp_col.b) + ")");
        }
        curPixel--;
      }
      curPixel -= (PHYSICAL_LEDS_PER_RING - LEDS_PER_RING); // Skip over hidden pixels
    }

    // Strip 3
    curPixel = 2*LEDS_PER_STRIP + 1;
    for(uint8_t ring = 2*RINGS_PER_STRIP; ring < 4*RINGS_PER_STRIP; ring+=2) {
      for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        CRGB exp_col = mid_palette.entries[get_mid_color((pixel+ring+node) % 3, (pixel+ring+2*node) % 3, (pixel+ring+node) % 12, (pixel+ring+node) % 7)];
        if(leds[curPixel] != exp_col) {
          Serial.print("node " + String(node) + ", leds[" + String(curPixel) + "] = (" + String(leds[curPixel].r) + ", " + String(leds[curPixel].g) + ", " + String(leds[curPixel].b) + "), ");
          Serial.println("mid_palette.entries[get_mid_color(" + String((pixel+ring+node) % 3) + ", " + String((pixel+ring+2*node) % 3) + ", " + String((pixel+ring+node) % 12) + ", " + String((pixel+ring+node) % 7) + ")], ring " + String(ring) + ", pixel " + String(pixel) + " = (" + String(exp_col.r) + ", " + String(exp_col.g) + ", " + String(exp_col.b) + ")");
        }
        curPixel++;
      }
      curPixel += (PHYSICAL_LEDS_PER_RING - LEDS_PER_RING); // Skip over hidden pixels
    }

    // Strip 4
    curPixel = 3*LEDS_PER_STRIP + 1;
    for(uint8_t ring = 1+2*RINGS_PER_STRIP; ring < 1+4*RINGS_PER_STRIP; ring+=2) {
      for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        CRGB exp_col = mid_palette.entries[get_mid_color((pixel+ring+node) % 3, (pixel+ring+2*node) % 3, (pixel+ring+node) % 12, (pixel+ring+node) % 7)];
        if(leds[curPixel] != exp_col) {
          Serial.print("node " + String(node) + ", leds[" + String(curPixel) + "] = (" + String(leds[curPixel].r) + ", " + String(leds[curPixel].g) + ", " + String(leds[curPixel].b) + "), ");
          Serial.println("mid_palette.entries[get_mid_color(" + String((pixel+ring+node) % 3) + ", " + String((pixel+ring+2*node) % 3) + ", " + String((pixel+ring+node) % 12) + ", " + String((pixel+ring+node) % 7) + ")], ring " + String(ring) + ", pixel " + String(pixel) + " = (" + String(exp_col.r) + ", " + String(exp_col.g) + ", " + String(exp_col.b) + ")");
        }
        curPixel++;
      }
      curPixel += (PHYSICAL_LEDS_PER_RING - LEDS_PER_RING); // Skip over hidden pixels
    }
  }

  
  // Test 22: overlaying works as intended
  Serial.println("----Test 22----");
  setup_palettes();
  
  for(uint8_t ring = 0; ring < NUM_RINGS; ring++) {
    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      set_sparkle(ring, pixel, WHITE);
      mid_layer[ring][pixel] = GRAY;
      base_layer[ring][pixel] = GRAY;
    }
  }

  write_pixel_data();
  
  for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      uint16_t idx = get_1d_index(ring, pixel);
      if(leds[idx] != CRGB(255, 255, 255)) {
        Serial.println("Expected white, leds[" + String(idx) + "] = (" + String(leds[idx].r) + "," + String(leds[idx].g) + "," + String(leds[idx].b) + ")");
      }
    }
  }

  for(uint8_t ring = 0; ring < NUM_RINGS; ring++) {
    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      set_sparkle(ring, pixel, TRANSPARENT);
      mid_layer[ring][pixel] = GRAY;
      base_layer[ring][pixel] = BLACK;
    }
  }

  write_pixel_data();

  for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      uint16_t idx = get_1d_index(ring, pixel);
      if(leds[idx] != CRGB(128, 128, 128)) {
        Serial.println("Expected gray from mid, leds[" + String(idx) + "] = (" + String(leds[idx].r) + "," + String(leds[idx].g) + "," + String(leds[idx].b) + ")");
      }
    }
  }

  for(uint8_t ring = 0; ring < NUM_RINGS; ring++) {
    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      set_sparkle(ring, pixel, TRANSPARENT);
      mid_layer[ring][pixel] = TRANSPARENT;
      base_layer[ring][pixel] = GRAY;
    }
  }

  write_pixel_data();

  for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      uint16_t idx = get_1d_index(ring, pixel);
      if(leds[idx] != CRGB(128, 128, 128)) {
        Serial.println("Expected gray from base, leds[" + String(idx) + "] = (" + String(leds[idx].r) + "," + String(leds[idx].g) + "," + String(leds[idx].b) + ")");
      }
    }
  }

  for(uint8_t ring = 0; ring < NUM_RINGS; ring++) {
    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      set_sparkle(ring, pixel, TRANSPARENT);
      mid_layer[ring][pixel] = TRANSPARENT;
      base_layer[ring][pixel] = TRANSPARENT;
    }
  }

  write_pixel_data();

  for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
    for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
      uint16_t idx = get_1d_index(ring, pixel);
      if(leds[idx] != CRGB(0, 0, 0)) {
        Serial.println("Expected black, leds[" + String(idx) + "] = (" + String(leds[idx].r) + "," + String(leds[idx].g) + "," + String(leds[idx].b) + ")");
      }
    }
  }


  Serial.println("All tests done.");
}
#endif

// To help with plugging in strips and troubleshooting. This does not use layers or palettes.
// Scrolls blocks of pixels around each ring. Rings will be colored RGBWRGBW...
// Number of lit pixels signals which strand it is for the current node
// Number of empty pixels singals which node number it is
void draw_debug_mode() {  
  uint8_t ringOffset, unlitPixels, throttle;
  
  if(node_number < NUM_NODES) {
    // Node number has been assigned
    ringOffset = node_number*RINGS_PER_NODE;
    unlitPixels = 1 + node_number;
    throttle = 8;
  }
  else {
    // Node number has NOT been assigned
    ringOffset = 0;
    unlitPixels = 8;
    throttle = 16;
  }

  // Clear all LEDs
  leds_all = CRGB::Black;

  
  for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++)
  {
    // Pick color based on ring
    uint8_t temp = (ring + ringOffset) % 4;
    CRGB ringColor = temp == 0 ? CRGB::Red : temp == 1 ? CRGB::Green : temp == 2 ? CRGB::Blue : CRGB::White;

    
    // Determine strand number and number of pixels to light up
    uint8_t strand;
    if(ring < RINGS_PER_NODE/2) {
      strand = ring % 2;
    }
    else {
      strand = 2 + (ring % 2);
    }
    
    uint8_t litPixels = 1 + strand;


    // Determine period of the repeating pattern and get an extended LED count that is a multiple of the period
    uint8_t period = litPixels + unlitPixels;
    uint16_t extended_led_count = ((LEDS_PER_RING-1)/period+1)*period;

    // Loop over extended LED count, but don't write the ones that are outside the range of the physical LEDs
    for(uint16_t curPixel = 0; curPixel < extended_led_count; curPixel++) {
      uint16_t idx = (curPixel + loop_count/throttle) % extended_led_count; // Add scrolling based off loop_count
      if(idx >= LEDS_PER_RING) continue;

      if(curPixel % period < litPixels) { // This line references curPixel since we are talking about the pattern
        leds[get_1d_index(ring, idx)] = ringColor; // This line references idx since we are talking about where the pattern gets written
      }
    }
  }
}

void test_strands() {
  const uint16_t delay_inc = 500;
  static uint16_t delay_factor = 0;
  for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
    if(ring == (loop_count % RINGS_PER_NODE)) {
      if(ring == 0) {
        delay_factor++;
        Serial.println("delay(" + String(delay_inc * delay_factor) + ")");
      }
      for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        leds[get_1d_index(ring, pixel)] = CRGB::White;//CRGB(128,128,128);
      }
    }
    else {
      for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        leds[get_1d_index(ring, pixel)] = CRGB::Black;
      }
    }
  }
  
  LEDS.show();
  delay(delay_inc * delay_factor);
}

