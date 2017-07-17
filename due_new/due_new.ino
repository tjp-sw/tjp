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
 *  EQ_VARIABLE(one sided) + Fire on the other side; requires mixing mid and edm animation
 *  smooth out frequencies[] values cycle to cycle
 *  add edm layer
 *  all speeds must be a factor of 2 to prevent jumping
 *  SPARKLE_PORTION uses an exponential scale, so most values end up looking very sparse
 *  optimize and inline small functions like scale_param()
 *  add sparkle layer parameter MIN_DIMMING
 *  using random8() in fire() is faster, but leads to artifacts. look into adding entropy to fire and using random8
 *  For fire: on inner part of structure, allow heat to diffuse from neighboring rings, since the rings are physically closer together
 *  Scale frequencies_...[] to be 0-255
 *  Add logic for *_RING_MOTION = SPLIT
 *  Consider colorCorrection() when looking at several palettes on the strips.
 *  calling millis() too many times? in do_communication and again in loop()
 *  How to gracefully remove max refresh rate enforcement?
 *  Test shorter delays in read_frequencies
 *  consider not resetting loop_counts in between animations that are very similar; imagine a snake that's a gradient. would be nice if it didn't jump when we changed animations.
 *  #define vs const scoped variables; is there a difference to RAM?
 *  get_1d_offset() - rings 0-5 seem to be doing rings in reverse order
 *  transparency gradients/variable transparency for each layer defined by the animations
 */

//------------------------ Config -----------------------------------//
//#define I_AM_THE_BEAT_DUE // Enable this on the 4th due for pride  //
// Due controlled versus pi controlled animation choices             //
#define PI_CONTROLLED                                              //
#ifndef PI_CONTROLLED                                                //
  #define TESTING_NODE_NUMBER 0   // To test diff nodes              //
  //#define CYCLE                                                    //
  //#define CYCLE_RANDOM                                             //
  //#define CYCLE_PARAMS    // Locks in show, cycles show_parameters //
#endif                                                               //
                                                                     //
#include "globals.h" // Leave this here, for #define order           //
                                                                     //
// Testing tools                                                     //
//#define DEBUG                   // Enables serial output             //
//#define DEBUG_TIMING          // Times each step in loop()         //
//#define DEBUG_LED_WRITE_DATA 10 // Dumps LED data every X cycles   //
//#define TEST_AVAIL_RAM 3092   // How much RAM we have left         //
                                                                     //
                                                                     //
// Timing settings                                                   //
#define REFRESH_TIME 0         // 16.7 frames per second            //
#define PALETTE_MAX_CHANGES 9   // how quickly palettes transition   //
                                                                     //
                                                                     //
// How fast animations/palettes will cycle                           //                                                                     
#if defined(CYCLE) || defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS) //
  #define BASE_ANIMATION_TIME 12000                                  //
  #define MID_ANIMATION_TIME 90000                                   //
  #define SPARKLE_ANIMATION_TIME 12000                               //
  #define PALETTE_CHANGE_TIME 7000                                   //
#endif                                                               //
                                                                     //
///////////////////////////////////////////////////////////////////////
//-------- Manually set animation parameters here --------//
////////////////////////////////////////////////////////////
void manually_set_animation_params() {                    //
                                                          //
  BASE_ANIMATION = 128;                                  //
  MID_ANIMATION = NONE;                                   //
  SPARKLE_ANIMATION = NONE;                               //
                                                          //
  BEAT_EFFECT = NONE;                                     //
                                                          //
  BASE_COLOR_THICKNESS = 0;                             //
  BASE_BLACK_THICKNESS = 255;                             //
  show_parameters[BASE_INTRA_RING_MOTION_INDEX] = CW;     //
  BASE_INTRA_RING_SPEED = 255;                            //
  show_parameters[BASE_INTER_RING_MOTION_INDEX] = NONE;   //
  BASE_INTER_RING_SPEED = 0;                              //
  show_parameters[BASE_RING_OFFSET_INDEX] = 127;          //
                                                          //
  MID_NUM_COLORS = 3;                                     //
  MID_COLOR_THICKNESS = 0;                              //
  MID_BLACK_THICKNESS = 0;                              //
  show_parameters[MID_INTRA_RING_MOTION_INDEX] = CW;      //
  MID_INTRA_RING_SPEED = 255;                             //
  show_parameters[MID_INTER_RING_MOTION_INDEX] = NONE;    //
  MID_INTER_RING_SPEED = 0;                               //
  show_parameters[MID_RING_OFFSET_INDEX] = 32;            //
                                                          //
  SPARKLE_COLOR_THICKNESS = 255;                          //
  SPARKLE_PORTION = 32;                                   //
  show_parameters[SPARKLE_INTRA_RING_MOTION_INDEX] = UP;  //
  SPARKLE_INTRA_RING_SPEED = 128;                         //
  show_parameters[SPARKLE_INTER_RING_MOTION_INDEX] = NONE;//
  SPARKLE_INTER_RING_SPEED = 0;                           //
  SPARKLE_MAX_DIM = 255;                                  //
  SPARKLE_RANGE = 0;                                      //
  SPARKLE_SPAWN_FREQUENCY = 0;                            //
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


  

  // Initialize FastLED parallel output controller (must be 8, even if we only use 4)
  LEDS.addLeds<WS2811_PORTD, 8>(leds, LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);

// Hold data lines low
  digitalWrite(25, 0);
  digitalWrite(26, 0);
  digitalWrite(27, 0);
  digitalWrite(28, 0);
  delay(500);
  
  //  Clear all LEDs
  LEDS.clear();
  LEDS.show();
  delay(500); // Hold LEDs off

  
  #ifdef PI_CONTROLLED
    setup_communication();
  #else
    assign_node(node_number);
  #endif

  setup_spectrum_shield();

  setup_palettes();


  // Initialize animations
  #ifdef PI_CONTROLLED
    BASE_ANIMATION = DEBUG_MODE; // Debug mode
    MID_ANIMATION = NONE; // off
    SPARKLE_ANIMATION = NONE; // off
  #else
    manually_set_animation_params();
  #endif
    
  init_base_animation();
  init_mid_animation();
  init_sparkle_animation();



  // Clear layers
  clear_sparkle_layer();
  clear_mid_layer();
  clear_base_layer();

  
  // Initialize timers
  unsigned long long temp = millis() + epoch_msec;
  base_start_time = temp;
  mid_start_time = temp;
  sparkle_start_time = temp;
  #if defined(CYCLE) || defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS)
    palette_start_time = temp;
  #endif
}


// the loop function runs over and over again forever
void loop() {
if(loop_count % 100 == 0) {
  // Hold data lines low
  digitalWrite(25, 0);
  digitalWrite(26, 0);
  digitalWrite(27, 0);
  digitalWrite(28, 0);
  delay(1);
}
  
  #ifdef DEBUG_TIMING
    static uint16_t serial_val[20];
    static unsigned long last_debug_time;
  #endif


  // Update time and counters
  current_time = epoch_msec + millis();
  
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
  #ifdef PI_CONTROLLED
    do_communication();
  #elif defined(CYCLE) || defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS)
    cycle_through_animations();
  #endif
  #ifdef DEBUG_TIMING
    now = millis();
    serial_val[2] = now - last_debug_time;
    last_debug_time = now;
  #endif


  // Apply beat effects
  #ifndef PI_CONTROLLED
    is_beat = loop_count % 10 == 0; // Generates a fake beat
  #endif
  if(is_beat) { do_beat_effects(); }


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


  
  loop_count++;
    base_count++;
    mid_count++;
    sparkle_count++;
}


void do_beat_effects() {
  return;
  switch(BEAT_EFFECT) {
    case COLOR_SWAP:
    {
      // Rotate colors
      CRGB temp = current_palette[2];
      current_palette[2] = current_palette[3];
      current_palette[3] = current_palette[4];
      current_palette[4] = temp;
    
      // Rotate targets so the same colors continue blending into the same targets
      temp = target_palette[2];
      target_palette[2] = target_palette[3];
      target_palette[3] = target_palette[4];
      target_palette[4] = temp;
    
      // Set all the dimming/gradient values in mid_palette that the animations will reference
      create_mid_palette(&mid_palette, current_palette[2], current_palette[3], current_palette[4]);
      break;
    }

    case NONE:
      break;

    default:
      #ifdef DEBUG
        Serial.println("Error! Undefined BEAT EFFECT: " + String(BEAT_EFFECT));
      #endif
      break;
  }
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
  
  
  if (current_time - mid_start_time >= MID_ANIMATION_TIME) {
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
    case BASE_SCROLLING_DIM:
      base_scrolling_dim();
      break;

    case BASE_2COLOR_GRADIENT:
      base_scrolling_2color_gradient();
      break;

  // ------ Non-layer animations -----
    case TEST_STRANDS:
      test_strands();
      break;
      
    case DEBUG_MODE:
      draw_debug_mode();
      break;

    case FREQ_PULSE:
      frequency_pulse();
      break;

    case EQ_FULL:
      equalizer_full(DISPLAY_FULL);
      break;

    case EQ_FULL_SPLIT:
      equalizer_full(DISPLAY_SPLIT);
      break;

    case EQ_VARIABLE:
      equalizer_variable(DISPLAY_FULL);
      break;
      
    case EQ_VARIABLE_SPLIT:
      equalizer_variable(DISPLAY_SPLIT);
      break;
      
    default:
      clear_base_layer();
      break;
  }
}

void draw_current_mid() {
  switch(MID_ANIMATION) {
    case SNAKE:
      snake(ALL_RINGS);
      break;

    case FIRE:
      fire(ALL_RINGS, FIRE_PALETTE_STANDARD);
      break;

    case FIRE_WHOOPS:
      fire(ALL_RINGS, FIRE_PALETTE_DISABLED);
      break;
      
    case MID_SCROLLING_DIM:
      mid_scrolling_dim(COLOR_BY_LOCATION);
      break;

    case MID_SCROLLING_DIM2:
      mid_scrolling_dim(COLOR_BY_PATTERN);
      break;
      
    case MID_SCROLLING_DIM3:
      mid_scrolling_dim(COLOR_BY_PATTERN_OFFSET);
      break;

    case FIRE_SNAKE:
      fire(ODD_RINGS, FIRE_PALETTE_STANDARD);
      snake(EVEN_RINGS);
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
    case BASE_SCROLLING_DIM:
      #if defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS)
        BASE_COLOR_THICKNESS = random8();
        BASE_BLACK_THICKNESS = random8();
        show_parameters[BASE_INTRA_RING_MOTION_INDEX] = random8(2) ? CW : CCW;
        BASE_INTRA_RING_SPEED = random8();
        show_parameters[BASE_RING_OFFSET_INDEX] = random8(21) - 10;
        #ifdef DEBUG
          Serial.println("Scrolling Dim params: " + String(BASE_COLOR_THICKNESS) + ", " + String(BASE_BLACK_THICKNESS) + ", " + String(BASE_INTRA_RING_MOTION) + ", " + String(BASE_INTRA_RING_SPEED) + ", " + String(BASE_RING_OFFSET)); 
        #endif
      #endif
      break;

    case BASE_2COLOR_GRADIENT:
      #if defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS)
        BASE_COLOR_THICKNESS = random8();
        show_parameters[BASE_INTRA_RING_MOTION_INDEX] = random8(2) ? CW : CCW;
        BASE_INTRA_RING_SPEED = random8();//4 << (random8(3) + (BASE_COLOR_THICKNESS < 20 ? 0 : BASE_COLOR_THICKNESS < 40 ? 1 : BASE_COLOR_THICKNESS < 70 ? 2 : BASE_COLOR_THICKNESS < 128 ? 3 : 4));
        show_parameters[BASE_RING_OFFSET_INDEX] = random8(21) - 10;
        #ifdef DEBUG
          Serial.println("Scrolling Gradient params: " + String(BASE_COLOR_THICKNESS) + ", " + String(BASE_INTRA_RING_MOTION) + ", " + String(BASE_INTRA_RING_SPEED) + ", " + String(BASE_RING_OFFSET)); 
        #endif
      #endif
      break;

    case SOUND_RESPONSIVE:
      if(BASE_ANIMATION < 128) {
        BASE_ANIMATION = random8(128, 133);
        disable_layering = BASE_ANIMATION >= 128; // EDM animations start at 255 and move down.
        
      }
      break;
      
    default:
      break;
  }
}

void init_mid_animation() {
  switch(MID_ANIMATION) {
    case FIRE_SNAKE:
      clear_mid_layer(); // Clear old pixels which are now "heat" values
    case SNAKE:
      #if defined(CYCLE) || defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS)
        MID_NUM_COLORS = random8();
        MID_COLOR_THICKNESS = random8();
        MID_BLACK_THICKNESS = random8();
        show_parameters[MID_INTRA_RING_MOTION_INDEX] = random8(2) ? CW : CCW;
        MID_INTRA_RING_SPEED = random8();
        show_parameters[MID_RING_OFFSET_INDEX] = random8(21) - 10;
        #ifdef DEBUG
          Serial.println("Snake params: " + String(MID_NUM_COLORS) + ", " + String(MID_COLOR_THICKNESS) + ", " + String(MID_BLACK_THICKNESS) + ", " + String(MID_INTRA_RING_MOTION) + ", " + String(MID_INTRA_RING_SPEED) + ", " + String(MID_RING_OFFSET)); 
        #endif
      #endif
      break;
      
    case FIRE:
    case FIRE_WHOOPS:
      clear_mid_layer(); // Clear old pixels which are now "heat" values
      #if defined(CYCLE) || defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS)
        // Everything is hard-coded in fire();
      #endif
      break;
      
    case MID_SCROLLING_DIM:
    case MID_SCROLLING_DIM2:
    case MID_SCROLLING_DIM3:
      #if defined(CYCLE) || defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS)
        MID_NUM_COLORS = random8();
        MID_COLOR_THICKNESS = random8();
        MID_BLACK_THICKNESS = random8();
        show_parameters[MID_INTRA_RING_MOTION_INDEX] = random8(2) ? CW : CCW;
        show_parameters[MID_RING_OFFSET_INDEX] = random8(21) - 10;
        MID_INTRA_RING_SPEED = random8();
        #ifdef DEBUG
          Serial.println("Scrolling dim params: " + String(MID_NUM_COLORS) + ", " + String(MID_COLOR_THICKNESS) + ", " + String(MID_BLACK_THICKNESS) + ", " + String(MID_INTRA_RING_MOTION) + ", " + String(MID_RING_OFFSET) + ", " + String(MID_INTRA_RING_SPEED));
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
      #if defined(CYCLE) || defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS)
        SPARKLE_COLOR_THICKNESS = random8();
        SPARKLE_PORTION = random8();
        SPARKLE_MAX_DIM = random8();
        SPARKLE_RANGE = random8();
        #ifdef DEBUG
          Serial.println("Glitter params: " + String(SPARKLE_COLOR_THICKNESS) + ", " + String(SPARKLE_PORTION) + ", " + String(SPARKLE_MAX_DIM) + ", " + String(SPARKLE_RANGE));
        #endif
      #endif
      break;
      
    case RAIN:
      clear_sparkle_layer();
      #if defined(CYCLE) || defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS)
        SPARKLE_COLOR_THICKNESS = random8();
        SPARKLE_PORTION = random8();
        show_parameters[SPARKLE_INTRA_RING_MOTION_INDEX] = random8(2) ? DOWN : UP;
        SPARKLE_INTRA_RING_SPEED = random8();
        SPARKLE_MAX_DIM = random8();
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

