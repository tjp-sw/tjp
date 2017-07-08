#include <FastLED.h>

/* ----- OVERVIEW OF NEW DUE CODE -----
 *  The current build will show all 3 layers working at once, plugged into a palette. Turning on CYCLE mostly works if you're a little patient and want to see everything changing.
 *  
 *  There are now 3 layers of animations. Each has its own loop_count (base_count, mid_count, sparkle_count), LED array, palette, and show_parameters.
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
 *  CYCLE mostly works, if you want to see palettes and animations changing. It's a little buggy still but gets the point across. In the init_current_xxx() functions,
 *    I set show parameters for each animation. It's not the best method but I don't think anyone else is really concerned with CYCLE right now, and I'll have it cleaned up soon.
 *  
 */

/* to do/thoughts/ideas
 *  define allowable ranges of show parameters, then build random_parameter walk in cycle_through_animations(), and anything else do_communication() does
 *  Consider colorCorrection() when looking at several palettes on the strips.
 *  calling millis() too many times? in do_communication and again in loop()
 *  How to gracefully remove max refresh rate enforcement?
 *  Test shorter delays in read_frequencies
 *  Do frequencies[] arrays need to be ints? Would be better as uint8_t, scaling or capping if necessary.
 *  how random is random8 and random16 really
 *  scrolling_2color_gradient: check that max gradient length and the +2 makes sense
 *  consider not resetting loop_counts in between animations that are very similar; imagine a snake that's a gradient. would be nice if it didn't jump when we changed animations.
 *  #define vs const scoped variables; is there a difference to RAM?
 */

//------------------------ Config --------------------------//
// Due controlled versus pi controlled animation choices    //
//#define PI_CONTROLLED                                     //
#ifdef PI_CONTROLLED                                        //
  #define INITIAL_BASE_ANIMATION -2    // debug mode        //
  #define INITIAL_MID_ANIMATION -1     // off               //
  #define INITIAL_SPARKLE_ANIMATION -1 // off               //
#else                                                       //
  #define TESTING_NODE_NUMBER 0 // To test diff nodes       //
  //#define CYCLE                                           //
  //#define CYCLE_RANDOM                                    //
#endif                                                      //
                                                            //
#include "globals.h" // Leave this here, for #define order  //
                                                            //
// Testing tools                                            //
#define DEBUG // Enables serial output                      //
//#define DEBUG_LED_ARRAYS // Runs unit tests in setup()    //
//#define DEBUG_TIMING // Times each step in loop()         //
//#define DEBUG_LED_WRITE_DATA // Dumps LED data at write() //
//#define TEST_AVAIL_RAM 1567 // How much RAM we have left  //
                                                            //
                                                            //
// Animation settings                                       //
#define PALETTE_MAX_CHANGES 25 // how fast palettes blend   //
#define NUM_BASE_ANIMATIONS 2                               //
#define NUM_MID_ANIMATIONS 2                                //
#define NUM_SPARKLE_ANIMATIONS 2                            //
                                                            //
                                                            //
// Timing settings                                          //
#define REFRESH_TIME 60 // 16.7 frames per second           //
                                                            //
#if defined(CYCLE) || defined(CYCLE_RANDOM)                 //
  #define BASE_ANIMATION_TIME 35000                         //
  #define MID_ANIMATION_TIME 25000                          //
  #define SPARKLE_ANIMATION_TIME 15000                      //
  #define PALETTE_CHANGE_TIME 10000                         //
#endif                                                      //
                                                            //
//////////////////////////////////////////////////////////////
//--- Manually set animation parameters here ---//
//////////////////////////////////////////////////
void manually_set_animation_params() {          //
                                                //
  BASE_ANIMATION = 0;                           //
  MID_ANIMATION = 1;                            //
  SPARKLE_ANIMATION = 0;                        //
                                                //  
  BASE_COLOR_THICKNESS = 8;                     //
  BASE_BLACK_THICKNESS = 0;                     //
  BASE_INTRA_RING_MOTION = 1;                   //
  BASE_INTRA_RING_SPEED = 16;                   //
  BASE_RING_OFFSET = 8;                         //
                                                //
  MID_NUM_COLORS = 160;                         //
  MID_COLOR_THICKNESS = 140;                    //
  MID_BLACK_THICKNESS = 20;                     //
  MID_INTRA_RING_MOTION = 0;                    //
  MID_INTRA_RING_SPEED = 0;                     //
  MID_RING_OFFSET = 0;                          //
                                                //
  SPARKLE_COLOR_THICKNESS = 1;                  //
  SPARKLE_PORTION = 20;                         //
  SPARKLE_INTRA_RING_MOTION = 1;                //
  SPARKLE_INTRA_RING_SPEED = 16;                //
  SPARKLE_MAX_DIM = 4;                          //
  SPARKLE_RANGE = 60;                           //
  SPARKLE_SPAWN_FREQUENCY = 10;                 //
                                                //
}                                               //
//----------------------------------------------//


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
    BASE_ANIMATION = INITIAL_BASE_ANIMATION;
    MID_ANIMATION = INITIAL_MID_ANIMATION;
    SPARKLE_ANIMATION = INITIAL_SPARKLE_ANIMATION;
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

  // Initialize animations
  init_base_animation();
  init_mid_animation();
  init_sparkle_animation();

  // Initialize timers
  unsigned long long temp = millis() + epoch_msec;
  base_start_time = temp;
  mid_start_time = temp;
  sparkle_start_time = temp;
  #if defined(CYCLE) || defined(CYCLE_RANDOM)
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
  #elif defined(CYCLE) || defined(CYCLE_RANDOM)
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


    // Transition smoothly between palettes; Do this after drawing layers, in case animation wants to disable blending or layering
    if(blend_base_layer) { blend_base_layer = blend_palette_256(base_palette, base_palette_target, PALETTE_MAX_CHANGES); }
    if(blend_mid_layer) { blend_mid_layer = blend_palette_256(mid_palette, mid_palette_target, PALETTE_MAX_CHANGES); }
    if(blend_sparkle_layer) { blend_sparkle_layer = blend_palette_16(sparkle_palette, sparkle_palette_target, PALETTE_MAX_CHANGES); }
    #ifdef DEBUG_TIMING
      now = millis();
      serial_val[6] = now - last_debug_time;
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
    for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
      for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
        Serial.print("loop_count " + String(loop_count) + ", Node " + String(node_number) + ", Ring " + String(ring) + ", Pixel " + String(pixel) + " = (");
        CRGB temp = leds[get_1d_index(ring, pixel)];
        Serial.println(String(temp.r) + "," + String(temp.g) + "," + String(temp.b) + ")");
      }
    }
  #endif
  LEDS.show();
  #ifdef DEBUG_TIMING
    now = millis();
    serial_val[8] = now - last_debug_time;
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

 
#if defined(CYCLE) || defined(CYCLE_RANDOM)
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

    set_palettes((uint8_t*) initial_palette);

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
      BASE_ANIMATION = random8(0, NUM_BASE_ANIMATIONS);
    #else
      BASE_ANIMATION = ++BASE_ANIMATION % NUM_BASE_ANIMATIONS;
    #endif

    #ifdef DEBUG
      Serial.print("New base animation started: ");
      Serial.println(BASE_ANIMATION);
    #endif
  }
  
  
  if (current_time - mid_start_time >= BASE_ANIMATION_TIME) {
    mid_start_time = current_time;
    mid_count = 0;
    
    #ifdef CYCLE_RANDOM
      MID_ANIMATION = random8(0, NUM_MID_ANIMATIONS);
    #else
      MID_ANIMATION = ++MID_ANIMATION % NUM_MID_ANIMATIONS;
    #endif

    #ifdef DEBUG
      Serial.print("New mid animation started: ");
      Serial.println(MID_ANIMATION);
    #endif
  }
  
  
  if (current_time - sparkle_start_time >= SPARKLE_ANIMATION_TIME) {
    sparkle_start_time = current_time;
    sparkle_count = 0;
    
    #ifdef CYCLE_RANDOM
      SPARKLE_ANIMATION = random8(0, NUM_SPARKLE_ANIMATIONS);
    #else
      SPARKLE_ANIMATION = ++SPARKLE_ANIMATION % NUM_SPARKLE_ANIMATIONS;
    #endif

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
    case 0:
      base_scrolling_dim();
      break;

    case 1:
      base_scrolling_gradient();
      break;

  // ------ Non-layer animations -----
    case -2:
      draw_debug_mode();
      break;

    case -3:
      draw_equalizer_full();
      break;
      
    default:
      clear_base_layer();
      break;
  }
}

void draw_current_mid() {
  switch(MID_ANIMATION) {
    case 0:
      snake();
      break;

    case 1:
      fire();
      break;
      
    default:
      clear_mid_layer();
      break;
  }
}

void draw_current_sparkle() {
  switch(SPARKLE_ANIMATION) {
    case 0:
      sparkle_glitter();
      break;

    case 1:
      sparkle_rain();
      break;
      
    default:
      clear_sparkle_layer();
      break;
  }
}

// Layer-specific initialization functions (before animation starts)
// Currently using this to store hard-coded show parameters, so cycling through animations can be done and still look good
void init_base_animation() {
  disable_layering = BASE_ANIMATION < -1; // -1 is off, anything below that is an edm animation.
  
  switch(BASE_ANIMATION) {
    case 0: // Scrolling Dim
      #if defined(CYCLE) || defined(CYCLE_RANDOM)
        BASE_COLOR_THICKNESS = 2 + random8(4);
        BASE_BLACK_THICKNESS = random8(4);
        BASE_INTRA_RING_MOTION = random8(2) ? -1 : 1;
        BASE_INTRA_RING_SPEED = 2 << random8(4);
        BASE_RING_OFFSET = random8(12);
      #endif
      break;

    case 1: // Scrolling Gradient
      #if defined(CYCLE) || defined(CYCLE_RANDOM)
        BASE_COLOR_THICKNESS = 20 + random8(20);
        BASE_INTRA_RING_MOTION = random8(2) ? -1 : 1;
        BASE_INTRA_RING_SPEED = 4 << random8(3);
        BASE_RING_OFFSET = random8(20);
      #endif
      break;
      
    default:
      break;
  }
}

void init_mid_animation() {
  switch(MID_ANIMATION) {
    case 0: // Snake
      #if defined(CYCLE) || defined(CYCLE_RANDOM)
        MID_NUM_COLORS = 2 + random8(2);
        MID_COLOR_THICKNESS = 2 + random8(4);
        MID_BLACK_THICKNESS = 5 + random8(10);
        MID_INTRA_RING_MOTION = random8(2) ? -1 : 1;
        MID_INTRA_RING_SPEED = 8 << random8(3);
        MID_RING_OFFSET = random8(10);
      #endif
      break;
      
    case 1: // Fire
      init_fire();
      #if defined(CYCLE) || defined(CYCLE_RANDOM)
        MID_NUM_COLORS = 160;      // Minimum spark size
        MID_COLOR_THICKNESS = 140; // Spark chance
        MID_BLACK_THICKNESS = 20;  // Cooling
      #endif
      break;

    default:
      break;
  }
}

void init_sparkle_animation() {
  switch(SPARKLE_ANIMATION) {
    case 0: // Glitter
      #if defined(CYCLE) || defined(CYCLE_RANDOM)
        SPARKLE_COLOR_THICKNESS = 1 + random8(2);
        SPARKLE_PORTION = 20 + random8(20);
        SPARKLE_MAX_DIM = random8(5);
        SPARKLE_RANGE = 60 + random8(144);
      #endif
      break;
      
    case 1: // Rain
      clear_sparkle_layer();
      #if defined(CYCLE) || defined(CYCLE_RANDOM)
        SPARKLE_COLOR_THICKNESS = 1 + random8(2);
        SPARKLE_PORTION = 30 + random8(30);
        SPARKLE_INTRA_RING_MOTION = random8(2) ? -1 : 1;
        SPARKLE_INTRA_RING_SPEED = 8 << random8(3);
        SPARKLE_MAX_DIM = random8(5);
        SPARKLE_RANGE = 20;
        SPARKLE_SPAWN_FREQUENCY = 20;
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
    case 1:
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

