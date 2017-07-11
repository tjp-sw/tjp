#include <FastLED.h>

/* ----- OVERVIEW OF NEW DUE CODE -----
 *  There are now 3 layers of animations. Each has its own loop_count (base_count, mid_count, sparkle_count), LED array, palette, and show_parameters.
 *  The current build will show all 3 layers working at once, plugged into a palette. Turning on CYCLE will walk through everything.
 *  CYCLE_PARAMS will keep the shows you specify but walk parameters. You can set the allowable ranges of params in init_(base|mid|sparkle)_animation()
 *  
 *  For the most part, you want to use the functions in palettes.ino to access allowed colors. You do NOT want to use CRGBs anymore. If you absolutely must use CRGBs,
 *  you can access the current node's raw LED array like this: leds[get_1d_index(ring, pixel)] = CRGB(1,2,3). But keep in mind that the layering logic will also write
 *  to leds[] unless disable_layering = true.
 *  
 *  I've tried to clean up this main file, a lot of globals that don't change often have been moved to globals.h. The stuff that remains is for configuration.
 *  
 *  If you're building a new animation, or want to test multiple layers together, you can set the show parameters below at lines ~80-100.
 *  
 *  Add the function calls for new animations in draw_current_base(), draw_current_mid(), or draw_current_sparkle(). Any initialization or cleanup should be done in init_current_base(), cleanup_current_sparkle(), etc.
 *  
 */

/* to do/thoughts/ideas
 *  Add logic for BASE & MID RING_MOTION = SPLIT
 *  Change base level dimming to be less extreme, since colors will be naturally dim; OR, change use of the colors to (almost) always dim
 *  Consider colorCorrection() when looking at several palettes on the strips.
 *  calling millis() too many times? in do_communication and again in loop()
 *  How to gracefully remove max refresh rate enforcement?
 *  Test shorter delays in read_frequencies
 *  Do frequencies[] arrays need to be ints? Would be better as uint8_t, scaling or capping if necessary.
 *  how random is random8 and random16 really
 *  consider not resetting loop_counts in between animations that are very similar; imagine a snake that's a gradient. would be nice if it didn't jump when we changed animations.
 *  #define vs const scoped variables; is there a difference to RAM?
 *  get_1d_offset() - rings 0-5 seem to be doing rings in reverse order
 *  transparency gradients/variable transparency for each layer defined by the animations
 */

//------------------------ Config -----------------------------------//
// Due controlled versus pi controlled animation choices             //
//#define PI_CONTROLLED                                              //
#ifndef PI_CONTROLLED                                                //
  #define TESTING_NODE_NUMBER 0   // To test diff nodes              //
  //#define CYCLE                                                    //
  //#define CYCLE_RANDOM                                             //
  #define CYCLE_PARAMS    // Locks in show, cycles show_parameters //
#endif                                                               //
                                                                     //
#include "globals.h" // Leave this here, for #define order           //
                                                                     //
// Testing tools                                                     //
#define DEBUG                   // Enables serial output             //
//#define DEBUG_LED_ARRAYS      // Runs unit tests in setup()        //
//#define DEBUG_TIMING          // Times each step in loop()         //
//#define DEBUG_LED_WRITE_DATA 10 // Dumps LED data every X cycles   //
//#define TEST_AVAIL_RAM 3092   // How much RAM we have left         //
                                                                     //
                                                                     //
// Timing settings                                                   //
#define REFRESH_TIME 60         // 16.7 frames per second            //
#define PALETTE_MAX_CHANGES 9   // how quickly palettes transition   //
                                                                     //
                                                                     //
// How fast animations/palettes will cycle                           //                                                                     
#if defined(CYCLE) || defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS) //
  #define BASE_ANIMATION_TIME 13000                                  //
  #define MID_ANIMATION_TIME 11000                                   //
  #define SPARKLE_ANIMATION_TIME 15000                               //
  #define PALETTE_CHANGE_TIME 7000                                   //
#endif                                                               //
                                                                     //
///////////////////////////////////////////////////////////////////////
//-------- Manually set animation parameters here --------//
////////////////////////////////////////////////////////////
void manually_set_animation_params() {                    //
                                                          //
  BASE_ANIMATION = DEBUG_MODE;                                   //
  MID_ANIMATION = OFF;                                    //
  SPARKLE_ANIMATION = OFF;                               //
                                                          //
  BASE_COLOR_THICKNESS = 30;                              //
  BASE_BLACK_THICKNESS = 0;                               //
  show_parameters[BASE_INTRA_RING_MOTION_INDEX] = CW;     //
  BASE_INTRA_RING_SPEED = 16;                             //
  show_parameters[BASE_RING_OFFSET_INDEX] = -8;           //
                                                          //
  MID_NUM_COLORS = 160;                                   //
  MID_COLOR_THICKNESS = 120;                              //
  MID_BLACK_THICKNESS = 30;                               //
  show_parameters[MID_INTRA_RING_MOTION_INDEX] = NONE;    //
  MID_INTRA_RING_SPEED = 0;                               //
  show_parameters[MID_RING_OFFSET_INDEX] = 0;             //
                                                          //
  SPARKLE_COLOR_THICKNESS = 1;                            //
  SPARKLE_PORTION = 20;                                   //
  show_parameters[SPARKLE_INTRA_RING_MOTION_INDEX] = CCW; //
  SPARKLE_INTRA_RING_SPEED = 16;                          //
  SPARKLE_MAX_DIM = 4;                                    //
  SPARKLE_RANGE = 60;                                     //
  SPARKLE_SPAWN_FREQUENCY = 10;                           //
                                                          //
}                                                         //
//--------------------------------------------------------//

// the setup function runs once when you press reset or power the board
void setup() {
  #ifdef TEST_AVAIL_RAM
    static byte test[TEST_AVAIL_RAM];
    test[0] = 0;
  #endif
  
  // Setup Serial port
  #ifdef DEBUG
    Serial.begin(115200);
    Serial.println("Serial port initialized.");
    Serial.flush();
  #endif

  setup_spectrum_shield();

  setup_palettes();

  #ifdef PI_CONTROLLED
    BASE_ANIMATION = DEBUG_MODE; // Debug mode
    MID_ANIMATION = OFF; // off
    SPARKLE_ANIMATION = OFF; // off
  #else
    manually_set_animation_params();
  #endif

  // Initialize digital pin LED_BUILTIN as an output
  pinMode(LED_BUILTIN, OUTPUT);

  // Hold data lines low
  digitalWrite(25, 0);
  digitalWrite(26, 0);
  digitalWrite(27, 0);
  digitalWrite(28, 0);
  delay(500);

  // Initialize FastLED parallel output controller (must be 8, even if we only use 4)
  LEDS.addLeds<WS2811_PORTD, 8>(leds, LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);

  //  Clear all LEDs
  LEDS.clear();
  LEDS.show();
  delay(500); // Hold LEDs off

  #ifdef PI_CONTROLLED
    setup_communication();
  #else
    assign_node(node_number);
  #endif

  // Clear layers
  clear_sparkle_layer();
  clear_mid_layer();
  clear_base_layer();
  
  // Initialize animations
  init_base_animation();
  init_mid_animation();
  init_sparkle_animation();

  // Initialize timers
  unsigned long long temp = millis() + epoch_msec;
  base_start_time = temp;
  mid_start_time = temp;
  sparkle_start_time = temp;
  #if defined(CYCLE) || defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS)
    palette_start_time = temp;
  #endif

  #ifdef DEBUG_LED_ARRAYS
    run_led_array_tests();
  #endif

  #ifdef DEBUG
    Serial.println("setup() complete.");
  #endif
}


// the loop function runs over and over again forever
void loop() {
  #ifdef DEBUG_TIMING
    static uint16_t serial_val[20];
    static unsigned long last_debug_time;
  #endif


  // Update time and counters
  current_time = epoch_msec + millis();
  base_count = (current_time - base_start_time) / REFRESH_TIME;
  mid_count = (current_time - mid_start_time) / REFRESH_TIME;
  sparkle_count = (current_time - sparkle_start_time) / REFRESH_TIME;
  #ifdef DEBUG_TIMING
    serial_val[0] = current_time - last_debug_time; // This also captures the end of loop delay() and any time lost in between loops
    last_debug_time = current_time;
  #endif


  // Read spectrum shield before communicating with Pi
  read_frequencies();
  #ifdef DEBUG_TIMING
    unsigned long now = millis();
    serial_val[1] = now - last_debug_time;
    last_debug_time = now;
  #endif


  // Communicate with pi if available, select animations, palettes, show parameters
  #ifdef PI_CONTROLLED()
    do_communication();
  #elif defined(CYCLE) || defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS)
    cycle_through_animations();
  #endif
  #ifdef DEBUG_TIMING
    now = millis();
    serial_val[2] = now - last_debug_time;
    last_debug_time = now;
  #endif


  //  Draw layers
  draw_current_base();
  #ifdef DEBUG_TIMING
    now = millis();
    serial_val[3] = now - last_debug_time;
    last_debug_time = now;
  #endif

  if(disable_layering) {
    #ifdef DEBUG_TIMING
      serial_val[4] = serial_val[5] = serial_val[6] = serial_val[7] = 0;
    #endif
  }
  else {
    draw_current_mid();
    #ifdef DEBUG_TIMING
      now = millis();
      serial_val[4] = now - last_debug_time;
      last_debug_time = now;
    #endif
  
    draw_current_sparkle();
    #ifdef DEBUG_TIMING
      now = millis();
      serial_val[5] = now - last_debug_time;
      last_debug_time = now;
    #endif


    write_pixel_data();
    #ifdef DEBUG_TIMING
      now = millis();
      serial_val[7] = now - last_debug_time;
      last_debug_time = now;
    #endif
  }


  // Write current node's LEDs
  #ifdef DEBUG_LED_WRITE_DATA
    if(loop_count % DEBUG_LED_WRITE_DATA == 0) {
      for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
        for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
          Serial.print("loop_count " + String(loop_count) + ", Node " + String(node_number) + ", Ring " + String(ring) + ", Pixel " + String(pixel) + " = (");
          CRGB temp = leds[get_1d_index(ring, pixel)];
          Serial.println(String(temp.r) + "," + String(temp.g) + "," + String(temp.b) + ")");
        }
      }
    }
  #endif
  LEDS.show();
  #ifdef DEBUG_TIMING
    now = millis();
    serial_val[8] = now - last_debug_time;
    last_debug_time = now;
  #endif


  // Transition smoothly between palettes; Do this after drawing layers, in case animation wants to disable blending or layering
  if(blend_base_layer) { blend_base_layer = blend_base_palette(PALETTE_MAX_CHANGES); }
  if(blend_mid_layer) { blend_mid_layer = blend_mid_palette(PALETTE_MAX_CHANGES); }
  if(blend_sparkle_layer) { blend_sparkle_layer = blend_sparkle_palette(PALETTE_MAX_CHANGES); }
  #ifdef DEBUG_TIMING
    now = millis();
    serial_val[6] = now - last_debug_time;
    last_debug_time = now;
  #endif

  
  // Serial output for debugging
  #ifdef DEBUG_TIMING
    write_timing_output();
  #endif


  // Enforce max refresh rate, leaving a 1ms buffer
  unsigned long temp = millis();
  if(temp - current_time < REFRESH_TIME - 1) {
    delay(REFRESH_TIME + current_time - temp - 1);
  }
  
  loop_count++;
}

 
#if defined(CYCLE) || defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS)
// Cycles through the animations, running each for ANIMATION_TIME seconds
void cycle_through_animations() {

  if (current_time - palette_start_time >= PALETTE_CHANGE_TIME) {
    palette_start_time = current_time;

    // Re-purposing initial_palette here as current_palette
    #ifdef CYCLE_RANDOM
      uint8_t r = random8(3);
      initial_palette = r == 0 ? icy_bright : r == 1 ? watermelon : fruit_loop;
    #else
           if(initial_palette == fruit_loop) { initial_palette = icy_bright; }
      else if(initial_palette == icy_bright) { initial_palette = watermelon; }
      else if(initial_palette == watermelon) { initial_palette = fruit_loop; }
      else { 
        #ifdef DEBUG
          Serial.println("Undefined palette loaded!");
        #endif
      }
    #endif

    memcpy(target_palette, initial_palette, 3*NUM_COLORS_PER_PALETTE);
    blend_base_layer = blend_mid_layer = blend_sparkle_layer = true;

    #ifdef DEBUG
      Serial.print("New palette loaded: ");
      if(initial_palette == fruit_loop) { Serial.println("fruit_loop"); }
      else if(initial_palette == icy_bright) { Serial.println("icy_bright"); }
      else if(initial_palette == watermelon) { Serial.println("watermelon"); }
      else { Serial.println("INVALID PALETTE LOADED"); }
    #endif
  }

  if (current_time - base_start_time >= BASE_ANIMATION_TIME) {
    base_start_time = current_time;
    base_count = 0;

    #ifdef CYCLE_RANDOM
        BASE_ANIMATION = 1 + random8(0, NUM_BASE_ANIMATIONS);
    #elif defined(CYCLE)
      BASE_ANIMATION = 1 + (++BASE_ANIMATION % NUM_BASE_ANIMATIONS);
    #endif

    init_base_animation();

    #ifdef DEBUG
      Serial.print("New base animation started: ");
      Serial.println(BASE_ANIMATION);
    #endif
  }
  
  
  if (current_time - mid_start_time >= BASE_ANIMATION_TIME) {
    mid_start_time = current_time;
    mid_count = 0;

    #ifdef CYCLE_RANDOM
      MID_ANIMATION = 1 + random8(0, NUM_MID_ANIMATIONS);
    #elif defined(CYCLE)
      MID_ANIMATION = 1 + (++MID_ANIMATION % NUM_MID_ANIMATIONS);
    #endif

    init_mid_animation();

    #ifdef DEBUG
      Serial.print("New mid animation started: ");
      Serial.println(MID_ANIMATION);
    #endif
  }
  
  
  if (current_time - sparkle_start_time >= SPARKLE_ANIMATION_TIME) {
    sparkle_start_time = current_time;
    sparkle_count = 0;

    #ifdef CYCLE_RANDOM
      SPARKLE_ANIMATION = 1 + random8(0, NUM_SPARKLE_ANIMATIONS);
    #elif defined(CYCLE)
      SPARKLE_ANIMATION = 1 + (++SPARKLE_ANIMATION % NUM_SPARKLE_ANIMATIONS);
    #endif

    init_sparkle_animation();

    #ifdef DEBUG
      Serial.print("New sparkle animation started: ");
      Serial.println(SPARKLE_ANIMATION);
    #endif
  }
}
#endif


// Layer-specific drawing functions
void draw_current_base() {
  switch(BASE_ANIMATION) {
    case SCROLLING_DIM:
      base_scrolling_dim();
      break;

    case SCROLLING_GRADIENT:
      base_scrolling_gradient();
      break;

  // ------ Non-layer animations -----
    case DEBUG_MODE:
      test_strands();
      //draw_debug_mode();
      break;
      
    default:
      clear_base_layer();
      break;
  }
}

void draw_current_mid() {
  switch(MID_ANIMATION) {
    case SNAKE:
      snake();
      break;

    case FIRE:
      fire(1);
      break;

    case FIRE_WHOOPS:
      fire(0);
      break;
      
    default:
      clear_mid_layer();
      break;
  }
}

void draw_current_sparkle() {
  switch(SPARKLE_ANIMATION) {
    case GLITTER:
      sparkle_glitter();
      break;

    case RAIN:
      sparkle_rain();
      break;
      
    default:
      clear_sparkle_layer();
      break;
  }
}


// Layer-specific initialization functions (before animation starts)
void init_base_animation() {
  disable_layering = BASE_ANIMATION >= 128; // EDM animations start at 255 and move down.
  
  switch(BASE_ANIMATION) {
    case SCROLLING_DIM:
      #if defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS)
        BASE_COLOR_THICKNESS = 2 + random8(4);
        BASE_BLACK_THICKNESS = random8(4);
        show_parameters[BASE_INTRA_RING_MOTION_INDEX] = random8(2) ? CW : CCW;
        BASE_INTRA_RING_SPEED = 4 << random8(4);
        show_parameters[BASE_RING_OFFSET_INDEX] = random8(12);
        #ifdef DEBUG
          Serial.println("Scrolling Dim params: " + String(BASE_COLOR_THICKNESS) + ", " + String(BASE_BLACK_THICKNESS) + ", " + String(BASE_INTRA_RING_MOTION) + ", " + String(BASE_INTRA_RING_SPEED) + ", " + String(BASE_RING_OFFSET)); 
        #endif
      #endif
      break;

    case SCROLLING_GRADIENT:
      #if defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS)
        BASE_COLOR_THICKNESS = 2 + random8(126);
        show_parameters[BASE_INTRA_RING_MOTION_INDEX] = random8(2) ? -1 : 1;
        BASE_INTRA_RING_SPEED = 4 << (random8(3) + (BASE_COLOR_THICKNESS < 20 ? 0 : BASE_COLOR_THICKNESS < 40 ? 1 : BASE_COLOR_THICKNESS < 70 ? 2 : 3));
        show_parameters[BASE_RING_OFFSET_INDEX] = random8(20);
        #ifdef DEBUG
          Serial.println("Scrolling Gradient params: " + String(BASE_COLOR_THICKNESS) + ", " + String(BASE_INTRA_RING_MOTION) + ", " + String(BASE_INTRA_RING_SPEED) + ", " + String(BASE_RING_OFFSET)); 
        #endif
      #endif
      break;
      
    default:
      break;
  }
}

void init_mid_animation() {
  switch(MID_ANIMATION) {
    case SNAKE:
      #if defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS)
        MID_NUM_COLORS = 2 + random8(2);
        MID_COLOR_THICKNESS = 2 + random8(4);
        MID_BLACK_THICKNESS = 5 + random8(10);
        show_parameters[MID_INTRA_RING_MOTION_INDEX] = random8(2) ? CW : CCW;
        MID_INTRA_RING_SPEED = 8 << random8(3);
        show_parameters[MID_RING_OFFSET_INDEX] = random8(10);
        #ifdef DEBUG
          Serial.println("Snake params: " + String(MID_NUM_COLORS) + ", " + String(MID_COLOR_THICKNESS) + ", " + String(MID_BLACK_THICKNESS) + ", " + String(MID_INTRA_RING_MOTION) + ", " + String(MID_INTRA_RING_SPEED) + ", " + String(MID_RING_OFFSET)); 
        #endif
      #endif
      break;
      
    case FIRE:
    case FIRE_WHOOPS:
      clear_mid_layer(); // Clear old pixels which are now "heat" values
      #if defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS)
        MID_NUM_COLORS = 160;      // Minimum spark size
        MID_COLOR_THICKNESS = 120; // Spark chance
        MID_BLACK_THICKNESS = 21;  // Cooling
        #ifdef DEBUG
          Serial.println("Fire params: " + String(MID_NUM_COLORS) + ", " + String(MID_COLOR_THICKNESS) + ", " + String(MID_BLACK_THICKNESS)); 
        #endif
      #endif
      break;

    default:
      break;
  }
}

void init_sparkle_animation() {
  switch(SPARKLE_ANIMATION) {
    case GLITTER:
      #if defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS)
        SPARKLE_COLOR_THICKNESS = 1 + random8(2);
        SPARKLE_PORTION = 20 + random8(185);
        SPARKLE_MAX_DIM = random8(5);
        { uint16_t temp = 60 + random8(100);
          SPARKLE_RANGE = temp >= 127 ? 127 : temp; }
        #ifdef DEBUG
          Serial.println("Glitter params: " + String(SPARKLE_COLOR_THICKNESS) + ", " + String(SPARKLE_PORTION) + ", " + String(SPARKLE_MAX_DIM) + ", " + String(SPARKLE_RANGE));
        #endif
      #endif
      break;
      
    case RAIN:
      clear_sparkle_layer();
      #if defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS)
        SPARKLE_COLOR_THICKNESS = 1 + random8(2);
        SPARKLE_PORTION = 30 + random8(30);
        show_parameters[SPARKLE_INTRA_RING_MOTION_INDEX] = random8(2) ? CCW : CW;
        SPARKLE_INTRA_RING_SPEED = 8 << random8(3);
        SPARKLE_MAX_DIM = random8(5);
        SPARKLE_RANGE = 20;
        SPARKLE_SPAWN_FREQUENCY = 20;
        #ifdef DEBUG
          Serial.println("Rain params: " + String(SPARKLE_COLOR_THICKNESS) + ", " + String(SPARKLE_PORTION) + ", " + String(SPARKLE_INTRA_RING_MOTION) + ", " + String(SPARKLE_INTRA_RING_SPEED) + ", " + String(SPARKLE_MAX_DIM) + ", " + String(SPARKLE_RANGE) + ", " + String(SPARKLE_SPAWN_FREQUENCY));
        #endif
      #endif
      break;
      
    default:
      break;
  }
}


// Layer-specific cleanup functions (after animation ends)
void cleanup_base_animation(uint8_t animation_index) {
  switch(animation_index) {
    default:
      break;
  }
}

void cleanup_mid_animation(uint8_t animation_index) {
  switch(animation_index) {
    case FIRE:
    case FIRE_WHOOPS:
      cleanup_fire();
      break;

    default:
      break;
  }
}

void cleanup_sparkle_animation(uint8_t animation_index) {
  switch(animation_index) {
    default:
      break;
  }
}

