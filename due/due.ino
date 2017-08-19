#include <FastLED.h>

//------------------------ Config -----------------------------------//
// Due controlled versus pi controlled animation choices             //
#define PI_CONTROLLED                                              //
#define AUDIO_PACKET_THROTTLE 100 // min ms between sending audio packets
#ifndef PI_CONTROLLED                                                //
  #define TESTING_NODE_NUMBER 0   // To test diff nodes              //
  //#define CYCLE           // Cycles through shows in order         //
  //#define CYCLE_RANDOM    // Cycles through shows randomly         //
  //#define CYCLE_PARAMS    // Locks in show, cycles show_parameters //
#endif                                                               //
                                                                     //
#include "globals.h"        // Leave this here, for #define order    //
                                                                     //
// Testing tools                                                     //
#define DEBUG                     // Enables serial output         //
#ifdef DEBUG                                                         //
  //#define DEBUG_TIMING            // Times each step in loop()     //
  //#define DEBUG_SPECTRUM_SHIELD     // Output all read values        //
  //#define DEBUG_LED_WRITE_DATA 10 // Dumps LED data every X cycles //
  //#define TEST_AVAIL_RAM 17600    // How much RAM we have left     //
#endif                                                               //
                                                                     //
                                                                     //
// How fast animations/palettes will cycle                           //
#if defined(CYCLE) || defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS) //
  #define BASE_ANIMATION_TIME 13000                                  //
  #define MID_ANIMATION_TIME 9000                                    //
  #define SPARKLE_ANIMATION_TIME 7000                                //
  #define PALETTE_CHANGE_TIME 5000                                   //
                                                                     //
  #define EDM_ANIMATION_TIME 10000                                   //
  #define BEAT_EFFECT_TIME 6000                                      //
#endif                                                               //
                                                                     //
///////////////////////////////////////////////////////////////////////
//-------- Manually set animation parameters here --------//
////////////////////////////////////////////////////////////
inline void manually_set_animation_params() {             //
                                                          //
  // Set to OFF to disable a layer during CYCLE'ing.      //
  // Use NONE to signify a layer that is off temporarily  //  //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
  BASE_ANIMATION = NONE;                                  //  // Lee: use LEE_COLOR_RANGE, LEE_BRIGHTNESS, LEE_CHECK, LEE_PICK_HSV
  MID_ANIMATION = NONE;                                   //  //@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
  SPARKLE_ANIMATION = NONE;                               //
  EDM_ANIMATION = EQ_PULSE;                            //
                                                          //
  BASE_COLOR_THICKNESS = 255;                             //
  BASE_BLACK_THICKNESS = 255;                             //
  show_parameters[BASE_INTRA_RING_MOTION_INDEX] = CCW;    //
  BASE_INTRA_RING_SPEED = 255;                            //
  show_parameters[BASE_INTER_RING_MOTION_INDEX] = NONE;   //
  BASE_INTER_RING_SPEED = 0;                              //
  show_parameters[BASE_RING_OFFSET_INDEX] = 16;           //
                                                          //
  MID_NUM_COLORS = 3;                                     //
  MID_COLOR_THICKNESS = 100;                              //
  MID_BLACK_THICKNESS = 255;                              //
  show_parameters[MID_INTRA_RING_MOTION_INDEX] = DOWN;    //
  MID_INTRA_RING_SPEED = 120;                             //
  show_parameters[MID_INTER_RING_MOTION_INDEX] = NONE;    //
  MID_INTER_RING_SPEED = 0;                               //
  show_parameters[MID_RING_OFFSET_INDEX] = 32;            //
                                                          //
  SPARKLE_PORTION = 100;                                   //
  SPARKLE_COLOR_THICKNESS = 100;                            //
  show_parameters[SPARKLE_INTRA_RING_MOTION_INDEX] = UP;  //
  SPARKLE_INTRA_RING_SPEED = 128;                         //
  show_parameters[SPARKLE_INTER_RING_MOTION_INDEX] = DOWN;//
  SPARKLE_INTER_RING_SPEED = 0;                           //
  SPARKLE_MIN_DIM = 0;                                    //
  SPARKLE_MAX_DIM = 255;                                  //
  SPARKLE_RANGE = 0;                                      //
  SPARKLE_SPAWN_FREQUENCY = 0;                            //
                                                          //
  PALETTE_CHANGE = PALETTE_CHANGE_IMMEDIATE;              //
  BEAT_EFFECT = NONE;                                     //
  show_parameters[ART_CAR_RING_INDEX] = NO_ART_CAR;       //
                                                          //
  BASE_TRANSITION = TRANSITION_BY_ALPHA;                  //
  BASE_TRANSITION_SPEED = FAST_TRANSITION;                //
                                                          //
  MID_TRANSITION = TRANSITION_BY_ALPHA;                   //
  MID_TRANSITION_SPEED = MEDIUM_TRANSITION;               //
                                                          //
  SPARKLE_TRANSITION = TRANSITION_BY_ALPHA;               //
  SPARKLE_TRANSITION_SPEED = SLOW_TRANSITION;             //
                                                          //
  EDM_TRANSITION = NONE;                                  //
  EDM_TRANSITION_SPEED = VERY_SLOW_TRANSITION;            //
}                                                         //
//--------------------------------------------------------//

// the setup function runs once when you press reset or power the board
void setup() {
  delay(1000);
  
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


  // Hold data lines low
  digitalWrite(25, 0);
  digitalWrite(26, 0);
  digitalWrite(27, 0);
  digitalWrite(28, 0);
  digitalWrite(14, 0);
  digitalWrite(15, 0);
  delay(1000);

  // Initialize FastLED parallel output controller (must be 8, even if we use fewer)
  LEDS.addLeds<WS2811_PORTD, 8>(leds, LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);

  //  Clear all LEDs
  LEDS.clear(true);
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
    BASE_ANIMATION = NONE;
    MID_ANIMATION = NONE;
    SPARKLE_ANIMATION = NONE;
    EDM_ANIMATION = DEBUG_MODE; // Debug mode
  #else
    manually_set_animation_params();
  #endif

  // Clear layers
  clear_sparkle_layer();
  clear_mid_layer();

  // Init layers
  init_base_animation();
  init_mid_animation();
  init_sparkle_animation(0, NUM_RINGS);
  init_edm_animation();


  // Initialize timers
  unsigned long long temp = millis() + epoch_msec;
  base_start_time = temp;
  mid_start_time = temp;
  sparkle_start_time = temp;
  edm_start_time = temp;
  #if defined(CYCLE) || defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS)
    palette_start_time = temp;
    beat_effect_start_time = temp;
  #endif
}


// the loop function runs over and over again forever
void loop() {
  //delay(700);
  #ifdef DEBUG_TIMING
    static uint16_t serial_val[20];
    static unsigned long last_debug_time;
  #endif


  // Update time and counters
  current_time = epoch_msec + millis();
  #ifdef DEBUG_TIMING
    unsigned long now = millis();
    serial_val[0] = now - last_debug_time; // This also captures any time lost in between loops
    last_debug_time = now;
  #endif


  // Read spectrum shield before communicating with Pi
  read_frequencies();
  #ifdef DEBUG_TIMING
    now = millis();
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


  // Blend smoothly between palettes
  transition_palette();
  #ifdef DEBUG_TIMING
    now = millis();
    serial_val[3] = now - last_debug_time;
    last_debug_time = now;
  #endif


  // Apply beat effects
  if(EDM_ANIMATION != OFF && EDM_ANIMATION != NONE) {
    #ifndef PI_CONTROLLED
      // Simulate a beat
      is_beat = loop_count % 10 == 0;
      uint16_t step = loop_count % 20;
      if(step > 10) { step = 20 - step; }
      downbeat_proximity = step * 255 / 9; // Using full range of downbeat_proximity (0-255)
    #endif
  
    //do_beat_effects();
  }
  #ifdef DEBUG_TIMING
    now = millis();
    serial_val[4] = now - last_debug_time;
    last_debug_time = now;
  #endif


  // Transition smoothly between animations
  transition_animations();
  #ifdef DEBUG_TIMING
    now = millis();
    serial_val[5] = now - last_debug_time;
    last_debug_time = now;
  #endif


  if(EDM_ANIMATION == 0 || EDM_ANIMATION >= 128) {
    // Draw layers
    draw_current_base(0, NUM_RINGS);
    #ifdef DEBUG_TIMING
      now = millis();
      serial_val[6] = now - last_debug_time;
      last_debug_time = now;
    #endif
  
    draw_current_mid(0, NUM_RINGS);
    #ifdef DEBUG_TIMING
      now = millis();
      serial_val[7] = now - last_debug_time;
      last_debug_time = now;
    #endif
  
  
    draw_current_sparkle(0, NUM_RINGS);
    #ifdef DEBUG_TIMING
      now = millis();
      serial_val[8] = now - last_debug_time;
      last_debug_time = now;
    #endif
  
  
    // Overlay layers
    write_pixel_data();
    #ifdef DEBUG_TIMING
      now = millis();
      serial_val[9] = now - last_debug_time;
      last_debug_time = now;
    #endif
  }


  // Overlay EDM layer
  draw_current_edm(0, NUM_RINGS);
  #ifdef DEBUG_TIMING
    now = millis();
    serial_val[10] = now - last_debug_time;
    last_debug_time = now;
  #endif


  // Output LED data (debugging tool)
  #ifdef DEBUG_LED_WRITE_DATA
    if(loop_count % DEBUG_LED_WRITE_DATA == 0) {
      for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++) {
        for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++) {
          Serial.print("loop_count " + String(loop_count) + ", Node " + String(node_number) + ", Ring " + String(ring) + ", Pixel " + String(pixel) + " = (");
          CRGB temp = get_led(ring, pixel);
            Serial.println(String(temp.r) + "," + String(temp.g) + "," + String(temp.b) + ")");
        }
      }
    }
  #endif


  // Write current node's LEDs
  if ((BEAT_EFFECT != JERKY_MOTION) || is_beat) { LEDS.show(); }
  #ifdef DEBUG_TIMING
    now = millis();
    serial_val[11] = now - last_debug_time;
    last_debug_time = now;
  #endif


  // Serial output for debugging
  #ifdef DEBUG_TIMING
    Serial.print("Timing:");

    for(int i = 0; i < 12; i++) {
      Serial.print("\t" + String(serial_val[i]));
    }

    Serial.println();
    Serial.flush(); // Guarantees buffer will write if future code hangs
  #endif


  // Increment loop counters
  loop_count++;
  base_count++;
  mid_count++;
  sparkle_count++;
  edm_count++;
}


inline void transition_animations() {
  if(transition_out_base_animation) {
    transition_out_base_animation = transition_out_base();
    if(!transition_out_base_animation) {
      cleanup_base_animation(BASE_ANIMATION);
      BASE_ANIMATION = next_base_animation;
      next_base_animation = NONE;
      init_base_animation();
      transition_in_base_animation = true;
    }
  }
  if(transition_in_base_animation) {
    transition_in_base_animation = transition_in_base();
    #if defined(CYCLE) || defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS)
      if(!transition_in_base_animation) { base_start_time = current_time; }
    #endif
  }

  if(transition_out_mid_animation) {
    transition_out_mid_animation = transition_out_mid();
    transition_in_mid_animation = false;
    if(!transition_out_mid_animation) {
      cleanup_mid_animation(MID_ANIMATION);
      MID_ANIMATION = next_mid_animation;
      next_mid_animation = NONE;
      init_mid_animation();
      transition_in_mid_animation = true;
    }
  }
  if(transition_in_mid_animation) {
    transition_in_mid_animation = transition_in_mid();
    #if defined(CYCLE) || defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS)
      if(!transition_in_mid_animation) { mid_start_time = current_time; }
    #endif
  }


  if(transition_out_sparkle_animation) {
    transition_out_sparkle_animation = transition_out_sparkle();
    transition_in_sparkle_animation = false;
    if(!transition_out_sparkle_animation) {
      cleanup_sparkle_animation(SPARKLE_ANIMATION);
      SPARKLE_ANIMATION = next_sparkle_animation;
      next_sparkle_animation = NONE;
      init_sparkle_animation(0, NUM_RINGS);
      transition_in_sparkle_animation = true;
    }
  }
  if(transition_in_sparkle_animation) {
    transition_in_sparkle_animation = transition_in_sparkle();
    #if defined(CYCLE) || defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS)
      if(!transition_in_sparkle_animation) { sparkle_start_time = current_time; }
    #endif
  }


  if(transition_out_edm_animation) {
    transition_out_edm_animation = transition_out_edm();
    transition_in_edm_animation = false;
    if(!transition_out_edm_animation) {
      cleanup_edm_animation(EDM_ANIMATION);
      EDM_ANIMATION = next_edm_animation;
      next_edm_animation = NONE;
      init_edm_animation();
      transition_in_edm_animation = true;
    }
  }
  if(transition_in_edm_animation) {
    transition_in_edm_animation = transition_in_edm();
    #if defined(CYCLE) || defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS)
      if(!transition_in_edm_animation) { edm_start_time = current_time; }
    #endif
  }
}

inline void do_beat_effects() {
  switch(BEAT_EFFECT) {
    case COLOR_SWAP: {
      if(is_beat) { color_swap(); }
      break;
    }

    case ALTERNATE_COLOR_THICKNESS: {
      if(is_beat) { alternate_color_thickness(); }
      break;
    }

    case ALTERNATE_BLACK_THICKNESS: {
      if(is_beat) { alternate_black_thickness(); }
      break;
    }

    case PULSE_COLOR_THICKNESS: {
      pulse_color_thickness();
      break;
    }

    case PULSE_BLACK_THICKNESS: {
      pulse_black_thickness();
      break;
    }

    case JERKY_MOTION: {
      // code for this is incorporated into write_pixel_data() in "layers"
      break;
    }

    case BLACKEN_NODE: {
      do_blacken_node();
      break;
    }

    case BLACKEN_RING: {
      do_blacken_ring();
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

inline void transition_palette() {
  switch(PALETTE_CHANGE) {
    case PALETTE_CHANGE_VERY_SLOW_BLEND:
      if(loop_count % 2 == 0) { return; }
      if(blend_base_layer) { blend_base_layer = blend_base_palette(PALETTE_CHANGES_VERY_SLOW, false); }
      if(blend_mid_layer) { blend_mid_layer = blend_mid_palette(PALETTE_CHANGES_VERY_SLOW, false); }
      if(blend_sparkle_layer) { blend_sparkle_layer = blend_sparkle_palette(PALETTE_CHANGES_VERY_SLOW, false); }
      break;

    case PALETTE_CHANGE_SLOW_BLEND:
      if(blend_base_layer) { blend_base_layer = blend_base_palette(PALETTE_CHANGES_SLOW, false); }
      if(blend_mid_layer) { blend_mid_layer = blend_mid_palette(PALETTE_CHANGES_SLOW, false); }
      if(blend_sparkle_layer) { blend_sparkle_layer = blend_sparkle_palette(PALETTE_CHANGES_SLOW, false); }
      break;

    case PALETTE_CHANGE_MEDIUM_BLEND:
      if(blend_base_layer) { blend_base_layer = blend_base_palette(PALETTE_CHANGES_MEDIUM, false); }
      if(blend_mid_layer) { blend_mid_layer = blend_mid_palette(PALETTE_CHANGES_MEDIUM, false); }
      if(blend_sparkle_layer) { blend_sparkle_layer = blend_sparkle_palette(PALETTE_CHANGES_MEDIUM, false); }
      break;

    case PALETTE_CHANGE_FAST_BLEND:
      if(blend_base_layer) { blend_base_layer = blend_base_palette(PALETTE_CHANGES_FAST, true); }
      if(blend_mid_layer) { blend_mid_layer = blend_mid_palette(PALETTE_CHANGES_FAST, true); }
      if(blend_sparkle_layer) { blend_sparkle_layer = blend_sparkle_palette(PALETTE_CHANGES_FAST, true); }
      break;

    case PALETTE_CHANGE_VERY_FAST_BLEND:
      if(blend_base_layer) { blend_base_layer = blend_base_palette(PALETTE_CHANGES_VERY_FAST, true); }
      if(blend_mid_layer) { blend_mid_layer = blend_mid_palette(PALETTE_CHANGES_VERY_FAST, true); }
      if(blend_sparkle_layer) { blend_sparkle_layer = blend_sparkle_palette(PALETTE_CHANGES_VERY_FAST, true); }
      break;

    case PALETTE_CHANGE_IMMEDIATE:
      if(blend_base_layer) {
         blend_base_layer = false;
         current_palette[0] = target_palette[0];
         current_palette[1] = target_palette[1];
      }
      if(blend_mid_layer) {
        blend_mid_layer = false;
        current_palette[2] = target_palette[2];
        current_palette[3] = target_palette[3];
        current_palette[4] = target_palette[4];
      }
      if(blend_sparkle_layer) {
        blend_sparkle_layer = false;
        current_palette[5] = target_palette[5];
        current_palette[6] = target_palette[6];
      }
      break;

    default:
      break;
  }
}


#if defined(CYCLE) || defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS)
// Cycles through the animations, running each for ANIMATION_TIME seconds
inline void cycle_through_animations() {

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
    #ifdef CYCLE_RANDOM
      PALETTE_CHANGE = 1 + random8(NUM_PALETTE_CHANGE_TYPES);
    #elif defined(CYCLE) || defined(CYCLE_PARAMS)
      PALETTE_CHANGE = (PALETTE_CHANGE+1) % (NUM_PALETTE_CHANGE_TYPES+1);
    #endif

    #ifdef DEBUG
      Serial.print("New palette loaded: ");
      if(initial_palette == fruit_loop) { Serial.print("fruit_loop, "); }
      else if(initial_palette == icy_bright) { Serial.print("icy_bright, "); }
      else if(initial_palette == watermelon) { Serial.print("watermelon, "); }
      else { Serial.print("INVALID PALETTE LOADED, "); }
      Serial.println("Palette change style = " + String(PALETTE_CHANGE));
    #endif
  }


  if(BASE_ANIMATION <= NUM_BASE_ANIMATIONS) {
    if ((current_time - base_start_time >= BASE_ANIMATION_TIME) && !(transition_out_base_animation || transition_in_base_animation)) {
      transition_out_base_animation = true;
      #ifdef CYCLE_RANDOM
        next_base_animation = 1 + random8(NUM_BASE_ANIMATIONS);
        BASE_TRANSITION = 1 + random8(NUM_BASE_TRANSITION_MODES);
        BASE_TRANSITION_SPEED = 1 + random8(MAX_TRANSITION_SPEED);
      #elif defined(CYCLE)
        next_base_animation = (BASE_ANIMATION+1) % (NUM_BASE_ANIMATIONS+1);
        BASE_TRANSITION = (BASE_TRANSITION+1) % (NUM_BASE_TRANSITION_MODES+1);
        BASE_TRANSITION_SPEED = 1 + (BASE_TRANSITION_SPEED % MAX_TRANSITION_SPEED);
      #else
        next_base_animation = BASE_ANIMATION;
      #endif
    }
  }

  if(MID_ANIMATION <= NUM_MID_ANIMATIONS) {
    if ((current_time - mid_start_time >= MID_ANIMATION_TIME) && !(transition_out_mid_animation || transition_in_mid_animation)) {
      transition_out_mid_animation = true;

      #ifdef CYCLE_RANDOM
        next_mid_animation = 1 + random8(NUM_MID_ANIMATIONS);
        MID_TRANSITION = 1 + random8(NUM_MID_TRANSITION_MODES);
        MID_TRANSITION_SPEED = 1 + random8(MAX_TRANSITION_SPEED);
      #elif defined(CYCLE)
        next_mid_animation = (MID_ANIMATION+1) % (NUM_MID_ANIMATIONS+1);
        MID_TRANSITION = (MID_TRANSITION+1) % (NUM_MID_TRANSITION_MODES+1);
        MID_TRANSITION_SPEED = 1 + (MID_TRANSITION_SPEED % MAX_TRANSITION_SPEED);
      #else
        next_mid_animation = MID_ANIMATION;
      #endif
    }
  }

  if(SPARKLE_ANIMATION <= NUM_SPARKLE_ANIMATIONS) {
    if ((current_time - sparkle_start_time >= SPARKLE_ANIMATION_TIME) && !(transition_out_sparkle_animation || transition_in_sparkle_animation)) {
      transition_out_sparkle_animation = true;

      #ifdef CYCLE_RANDOM
        next_sparkle_animation = 1 + random8(NUM_SPARKLE_ANIMATIONS);
        SPARKLE_TRANSITION = 1 + random8(NUM_SPARKLE_TRANSITION_MODES);
        SPARKLE_TRANSITION_SPEED = 1 + random8(MAX_TRANSITION_SPEED);
      #elif defined(CYCLE)
        next_sparkle_animation = (SPARKLE_ANIMATION+1) % (NUM_SPARKLE_ANIMATIONS+1);
        SPARKLE_TRANSITION = (SPARKLE_TRANSITION+1) % (NUM_SPARKLE_TRANSITION_MODES+1);
        SPARKLE_TRANSITION_SPEED = 1 + (SPARKLE_TRANSITION_SPEED % MAX_TRANSITION_SPEED);
      #else
        next_sparkle_animation = SPARKLE_ANIMATION;
      #endif
    }
  }

  if(EDM_ANIMATION != OFF) {
    if ((current_time - edm_start_time >= EDM_ANIMATION_TIME) && !(transition_out_edm_animation || transition_in_edm_animation)) {
      transition_out_edm_animation = true;

      #ifdef CYCLE_RANDOM
        next_edm_animation = 1 + random8(NUM_EDM_ANIMATIONS);
        EDM_TRANSITION = 1 + random8(NUM_EDM_TRANSITION_MODES);
        EDM_TRANSITION_SPEED = 1 + random8(MAX_TRANSITION_SPEED);
      #elif defined(CYCLE)
        next_edm_animation = (EDM_ANIMATION+1) % (NUM_EDM_ANIMATIONS+1);
        EDM_TRANSITION = (EDM_TRANSITION+1) % (NUM_EDM_TRANSITION_MODES+1);
        EDM_TRANSITION_SPEED = (EDM_TRANSITION_SPEED % MAX_TRANSITION_SPEED);
      #else
        next_edm_animation = EDM_ANIMATION;
      #endif
    }
  
    if(current_time - beat_effect_start_time >= BEAT_EFFECT_TIME) {
      beat_effect_start_time = current_time;
      #ifdef CYCLE_RANDOM
        BEAT_EFFECT = 1 + random8(NUM_BEAT_EFFECTS);
      #elif defined(CYCLE)
        BEAT_EFFECT = (BEAT_EFFECT+1) % (NUM_BEAT_EFFECTS+1);
      #endif

      #ifdef DEBUG
        Serial.println("New beat effect: " + String(BEAT_EFFECT));
      #endif
    }
  }
}
#endif


// Layer-specific drawing functions
inline void draw_current_base(uint8_t min_ring, uint8_t max_ring) {
  switch(BASE_ANIMATION) {
    case BASE_SCROLLING_DIM:
      base_scrolling_dim(min_ring, max_ring);
      break;

    case BASE_SCROLLING_HALF_DIM:
      base_scrolling_half_dim(min_ring, max_ring);
      break;

    case BASE_2COLOR_GRADIENT:
      base_scrolling_2color_gradient(min_ring, max_ring);
      break;

    case BASE_HORIZONTAL_GRADIENT:
      base_horizontal_gradient(min_ring, max_ring);
      break;

    case LEE_COLOR_RANGE:
      normalized_all_colors();
      break;

    case LEE_BRIGHTNESS:
       scale_brightness();
      break;

    case LEE_CHECK:
      check_color();
      break;

    case LEE_PICK_HSV:
      pick_hsv_colors();
      break;

    default:
      leds_all = CRGB::Black;
      break;
  }
}

inline void draw_current_mid(uint8_t min_ring, uint8_t max_ring) {
  switch(MID_ANIMATION) {
    case SNAKE:
      snake(min_ring, max_ring);
      break;

    case FIRE:
      fire(FIRE_PALETTE_STANDARD, true, min_ring, max_ring);
      break;

    case FIRE_ONE_SIDED:
      fire(FIRE_PALETTE_STANDARD, false, min_ring, max_ring);
      break;

    case DISCO_FIRE:
      fire(FIRE_PALETTE_DISABLED, true, min_ring, max_ring);
      break;

    case DISCO_FIRE_ONE_SIDED:
      fire(FIRE_PALETTE_DISABLED, false, min_ring, max_ring);
      break;

    case MID_SCROLLING_DIM2:
      mid_scrolling_dim2(min_ring, max_ring);
      break;

    case MID_SCROLLING_DIM3:
      mid_scrolling_dim3(min_ring, max_ring);
      break;

    case MID_SCROLLING_DIM4:
      mid_scrolling_dim4(min_ring, max_ring);
      break;

    case MID_SCROLLING_DIM5:
      mid_scrolling_dim5(min_ring, max_ring);
      break;

    case ARROW:
      arrow();
      break;

    case RADIATION:
      radiation_symbol();
      break;

    case SQUARE:
      square_pattern();
      break;

    case SQUARE2:
      square_pattern2();
      break;

    case WAVE:
      wave();
      break;

    default:
      break;
  }
}

inline void draw_current_sparkle(uint8_t min_ring, uint8_t max_ring) {
  switch(SPARKLE_ANIMATION) {
    case STATIC:
      sparkle_glitter(1, false, min_ring, max_ring);
      
    case GLITTER:
      sparkle_glitter(2, false, min_ring, max_ring);
      break;

    case RAIN:
      sparkle_rain(min_ring, max_ring);
      break;

    case WARP_SPEED:
      sparkle_warp_speed();
      break;

    case THREE_CIRCLES:
      clear_sparkle_layer();
      sparkle_three_circles();
      break;

    case CIRCLE_TRAILS:
      sparkle_circle_trails();
      break;

    case TWO_COINS:
      sparkle_two_coins();
      break;

    case TWINKLE:
      sparkle_twinkle(min_ring, max_ring);
      break;

    case VARIABLE_SPIN:
      variable_spin(min_ring, max_ring);
      break;

    case TORUS_KNOT:
      sparkle_torus_knot();
      break;

    case PANIC:
      sparkle_panic();
      break;

    case TORUS_LINK:
      sparkle_torus_link();
      break;
      
    default:
      break;
  }
}

inline void draw_current_edm(uint8_t min_ring, uint8_t max_ring) {
  switch(EDM_ANIMATION) {
    case TEST_STRANDS:
      test_strands_2strands_only();
      break;

    case DEBUG_MODE:
      draw_debug_mode();
      break;

    case FREQ_PULSE:
      frequency_pulse(min_ring, max_ring);
      break;

    case EQ_FULL:
      equalizer_full(DISPLAY_FULL, min_ring, max_ring);
      break;

    case EQ_FULL_SPLIT:
      equalizer_full(DISPLAY_SPLIT, min_ring, max_ring);
      break;

    case EQ_VARIABLE_FIRE:
      MID_ANIMATION = FIRE_ONE_SIDED;
      show_parameters[MID_INTRA_RING_MOTION_INDEX] = UP;
      draw_current_mid(min_ring, max_ring);
      write_pixel_data();
    case EQ_VARIABLE:
      equalizer_variable(DISPLAY_FULL, min_ring, max_ring);
      break;

    case EQ_VARIABLE_SPLIT:
      equalizer_variable(DISPLAY_SPLIT, min_ring, max_ring);
      break;

    case EQ_PULSE:
      equalizer_pulse(min_ring, max_ring);
      break;

    case KILL_ANIMATION:
      kill_animation();
      break;

    case CHAKRA_PULSE:
      chakra_pulse();
      break;

    case ANIMATION_PULSE:
      animation_pulse();
      break;

    case LIGHTNING:
      lightning();
      break;
      
    case FIRE_HELLO:
      fire_hello();
      break;

    case BLACK_HOLE:
      black_hole();
      break;

    case MEDITATION_SUNRISE:
      bloom();
      break;

    case MEDITATION_SUNSET:
      drip();
      break;

    default:
      break;
  }
}


// Layer-specific initialization functions (before animation starts)
inline void init_base_animation() {
  base_start_time = current_time;
  base_count = 0;

  #if defined(CYCLE) || defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS)
    BASE_COLOR_THICKNESS = random8();
    BASE_BLACK_THICKNESS = random8();
    show_parameters[BASE_INTRA_RING_MOTION_INDEX] = random8(3);
    if(BASE_INTRA_RING_MOTION == 0) { show_parameters[BASE_INTRA_RING_MOTION_INDEX] = -1; }
    BASE_INTRA_RING_SPEED = random8();
    show_parameters[BASE_INTER_RING_MOTION_INDEX] = random8(2) ? CW : CCW;
    BASE_INTER_RING_SPEED = random8();
    show_parameters[BASE_RING_OFFSET_INDEX] = random8() - 128;
  #endif

  switch(BASE_ANIMATION) {
    default:
      break;
  }

  #ifdef DEBUG
    Serial.println();
    Serial.print("New base animation started: ");
    Serial.println(BASE_ANIMATION);
    if(BASE_ANIMATION != OFF && BASE_ANIMATION != NONE) {
      Serial.println("Params: " + String(BASE_COLOR_THICKNESS) + ", " + String(BASE_BLACK_THICKNESS) + ", " + String(BASE_INTRA_RING_MOTION) + ", " + String(BASE_INTRA_RING_SPEED) + ", " + String(BASE_INTER_RING_MOTION) + ", " + String(BASE_INTER_RING_SPEED) + ", " + String(BASE_RING_OFFSET));
      Serial.println("Base transition style: " + String(BASE_TRANSITION) + ", speed: " + String(BASE_TRANSITION_SPEED));
      Serial.println();
    }
  #endif
}

inline void init_mid_animation() {
  mid_start_time = current_time;
  mid_count = 0;

  #if defined(CYCLE) || defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS)
    MID_NUM_COLORS = random8(1, 4);
    MID_COLOR_THICKNESS = random8();
    MID_BLACK_THICKNESS = random8();
    show_parameters[MID_INTRA_RING_MOTION_INDEX] = random8(3);
    if(MID_INTRA_RING_MOTION == 0) { show_parameters[MID_INTRA_RING_MOTION_INDEX] = -1; }
    MID_INTRA_RING_SPEED = random8();
    show_parameters[MID_INTER_RING_MOTION_INDEX] = random8(2) ? CW : CCW;
    MID_INTER_RING_SPEED = random8();
    show_parameters[MID_RING_OFFSET_INDEX] = random8() - 128;
    //MID_ALPHA = random8(NUM_MID_ALPHA_MODES);
  #endif

  switch(MID_ANIMATION) {
    case FIRE:
    case FIRE_ONE_SIDED:
      override_default_blending = true;
    case DISCO_FIRE:
    case DISCO_FIRE_ONE_SIDED:
      clear_mid_layer(); // Clear old pixels which are now "heat" values
      break;

    case ARROW:
      init_arrow();
      break;

    case NONE:
      clear_mid_layer();
      break;

    default:
      break;
  }

  #ifdef DEBUG
    Serial.println();
    Serial.print("New mid animation started: ");
    Serial.println(MID_ANIMATION);
    if(MID_ANIMATION != OFF && MID_ANIMATION != NONE) {
      Serial.println("Params: " + String(MID_NUM_COLORS) + ", " + String(MID_COLOR_THICKNESS) + ", " + String(MID_BLACK_THICKNESS) + ", " + String(MID_INTRA_RING_MOTION) + ", " + String(MID_INTRA_RING_SPEED) + ", " + String(MID_INTER_RING_MOTION) + ", " + String(MID_INTER_RING_SPEED) + ", " + String(MID_RING_OFFSET));
      Serial.println("Mid transition style: " + String(MID_TRANSITION) + ", speed: " + String(MID_TRANSITION_SPEED));
      Serial.println();
    }
  #endif
}

inline void init_sparkle_animation(uint8_t min_ring, uint8_t max_ring) {
  sparkle_start_time = current_time;
  sparkle_count = 0;

  #if defined(CYCLE) || defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS)
    SPARKLE_COLOR_THICKNESS = random8();
    SPARKLE_PORTION = random8();
    show_parameters[SPARKLE_INTRA_RING_MOTION_INDEX] = random8(3);
    if(SPARKLE_INTRA_RING_MOTION == 0) { show_parameters[SPARKLE_INTRA_RING_MOTION_INDEX] = -1; }
    SPARKLE_INTRA_RING_SPEED = random8();
    show_parameters[SPARKLE_INTER_RING_MOTION_INDEX] = random8(2) ? CW : CCW;
    SPARKLE_INTER_RING_SPEED = random8();
    SPARKLE_MIN_DIM = random8();
    SPARKLE_MAX_DIM = random8();
    SPARKLE_RANGE = random8();
    SPARKLE_SPAWN_FREQUENCY = random8();
    //SPARKLE_ALPHA = random8(NUM_SPARKLE_ALPHA_MODES);
  #endif


  switch(SPARKLE_ANIMATION) {
    case GLITTER:
    case RAIN:
      // Don't clear sparkles when starting these animations
      break;

    case STATIC:
      SPARKLE_PORTION = 255;
      SPARKLE_MIN_DIM = 255; // Start everything at minimum brightness
      SPARKLE_MAX_DIM = 255;

    case WARP_SPEED:
      random16_set_seed(0); // Synchronize RNG on different nodes
      SPARKLE_PORTION = 100;
      SPARKLE_MIN_DIM = 255; // Start everything at minimum brightness
      SPARKLE_MAX_DIM = 255;
      SPARKLE_RANGE = 255;

      sparkle_glitter(2, true, min_ring, max_ring); // Generate a set of spots
      break;

    case TWINKLE:
      random16_set_seed(0); // Synchronize RNG on different nodes
      sparkle_glitter(2, true, min_ring, max_ring); // Generate a set of spots
      break;

    case THREE_CIRCLES:
    case CIRCLE_TRAILS:
    case TWO_COINS:
      random16_set_seed(0); // Synchronize RNG on different nodes
      sparkle_current_ring = random8(NUM_RINGS);
      sparkle_current_pixel = random16(LEDS_PER_RING);
      sparkle_current_coin = random8(NUM_RINGS);
      break;

    case TORUS_KNOT:
    case TORUS_LINK:
    case PANIC:
    case NONE:
      clear_sparkle_layer();

    default:
      break;
  }

  #ifdef DEBUG
    Serial.println();
    Serial.print("New sparkle animation started: ");
    Serial.println(SPARKLE_ANIMATION);
    if(SPARKLE_ANIMATION != OFF && SPARKLE_ANIMATION != NONE) {
      Serial.println("Params: " + String(SPARKLE_COLOR_THICKNESS) + ", " + String(SPARKLE_PORTION) + ", " + String(SPARKLE_INTRA_RING_MOTION) + ", " + String(SPARKLE_INTRA_RING_SPEED) + ", " + String(SPARKLE_INTER_RING_MOTION) + ", " + String(SPARKLE_INTER_RING_SPEED) + ", " + String(SPARKLE_MIN_DIM) + ", " + String(SPARKLE_MAX_DIM) + ", " + String(SPARKLE_RANGE) + ", " + String(SPARKLE_SPAWN_FREQUENCY));
      Serial.println("Sparkle transition style: " + String(SPARKLE_TRANSITION) + ", speed: " + String(SPARKLE_TRANSITION_SPEED));
      Serial.println();
    }
  #endif
}

inline void init_edm_animation() {
  edm_start_time = current_time;
  edm_count = 0;
  leds_all = CRGB::Black;

  #if defined(CYCLE) || defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS)
    if(EDM_ANIMATION != NONE) {
      BASE_COLOR_THICKNESS = random8();
      BASE_BLACK_THICKNESS = random8();
      show_parameters[BASE_INTRA_RING_MOTION_INDEX] = random8(3);
      if(BASE_INTRA_RING_MOTION == 0) { show_parameters[BASE_INTRA_RING_MOTION_INDEX] = -1; }
      BASE_INTRA_RING_SPEED = random8();
      show_parameters[BASE_INTER_RING_MOTION_INDEX] = random8(2) ? CW : CCW;
      BASE_INTER_RING_SPEED = random8();
      show_parameters[BASE_RING_OFFSET_INDEX] = random8() - 128;
    }
  #endif

  switch(EDM_ANIMATION) {
    case NONE:
    case OFF:
      break; // Don't disable layers

    case EQ_VARIABLE_FIRE:
      MID_ANIMATION = FIRE_ONE_SIDED;
      init_mid_animation();
      clear_sparkle_layer();
      break;

    case FIRE_HELLO:
      if(node_number == ART_CAR_RING/RINGS_PER_NODE) { clear_mid_layer(); }
      break;

    default:
      break;
  }

  #ifdef DEBUG
    Serial.println();
    Serial.print("New edm animation started: ");
    Serial.println(EDM_ANIMATION);
    if(EDM_ANIMATION != OFF && EDM_ANIMATION != NONE) {
      Serial.println("Params: " + String(BASE_COLOR_THICKNESS) + ", " + String(BASE_BLACK_THICKNESS) + ", " + String(BASE_INTRA_RING_MOTION) + ", " + String(BASE_INTRA_RING_SPEED) + ", " + String(BASE_INTER_RING_MOTION) + ", " + String(BASE_INTER_RING_SPEED) + ", " + String(BASE_RING_OFFSET));
      Serial.println("EDM transition style: " + String(EDM_TRANSITION) + ", speed: " + String(EDM_TRANSITION_SPEED));
      Serial.println();
    }
  #endif
}


// Layer-specific cleanup functions (after animation ends)
inline void cleanup_base_animation(uint8_t animation_index) {
  switch(animation_index) {
    default:
      break;
  }
}

inline void cleanup_mid_animation(uint8_t animation_index) {
  switch(animation_index) {
    case FIRE:
    case FIRE_ONE_SIDED:
      override_default_blending = false;
    case DISCO_FIRE:
    case DISCO_FIRE_ONE_SIDED:
      cleanup_fire();
      break;

    default:
      break;
  }
}

inline void cleanup_sparkle_animation(uint8_t animation_index) {
  switch(animation_index) {
    default:
      break;
  }
}

inline void cleanup_edm_animation(uint8_t animation_index) {
  switch(animation_index) {
    default:
      break;
  }
}
