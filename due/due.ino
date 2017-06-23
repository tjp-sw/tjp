
// fixme: Diane - have added and rearranged parameters. update python code to match this change
// fixme: Brian? - animations are served from the center of the strips forward and backwards, though we're writing our code
//        as if it's written strips 0 -> 4. should alter writing out to LEDs to counter this change
// fixme: Brian? - saving all color arrays as CRGB will take too much memory. Instead, we'll save an index into an array / arrays to save space
//        this conversion needs to be handled


#include <FastLED.h>
typedef void (*sparkle_f_ptr)();     


#define I_AM_DUE

// Config --------------------------------------------------//
// Debugging options; comment/uncomment to test diff things //
#define DEBUG                                               //
//#define DEBUG_TIMING                                      //
//#define DEBUG_LED_ARRAYS                                  //
//#define DEBUG_PEAKS                                       //
//#define DEBUG_BPM                                         //
//#define DEBUG_AUDIO_HOOKS                                 //
                                                            //
// Timing settings                                          //
#define REFRESH_TIME 60 // 16.67 frames per second          //
#define ANIMATION_TIME 30000 // 30 seconds per animation    //
                                                            //
// Due controlled versus pi controlled animation choices    //
//#define PI_CONTROLLED                                     //
//#define CYCLE                                             //
//----------------------------------------------------------//

// Debugging globals -------------//
#ifdef DEBUG                      //
  word serial_val[20];            //
  unsigned long last_debug_time;  //
#endif                            //
//--------------------------------//

#ifdef PI_CONTROLLED
uint8_t node_number = 255;
#else
uint8_t node_number = 0; // For testing
#endif

// Spectrum Shield -----------------------------------------------------------//
// Pin connections                                                            //
#define SS_PIN_STROBE 4                                                       //
#define SS_PIN_RESET 5                                                        //
#define SS_PIN_DC_ONE A0                                                      //
#define SS_PIN_DC_TWO A1                                                      //
#define NUM_CHANNELS 7                                                        //
                                                                              //
// Globals                                                                    //
int frequencies_one[NUM_CHANNELS];                                            //
int frequencies_two[NUM_CHANNELS];                                            //
int frequencies_max[NUM_CHANNELS];                                            //
                                                                              //
bool is_beat = false;                                                         //
uint8_t downbeat_proximity = 0; // Up and down from 0-255 with the beat       //
uint8_t bpm_estimate = 0;                                                    //
uint8_t bpm_confidence = 0; // <10 is weak, 20 is decent, 30+ is really good  //
                                                                              //
#define AUDIO_HOOK_HISTORY_SIZE 200                                           //
uint16_t overall_volume[AUDIO_HOOK_HISTORY_SIZE];                             //
uint8_t dominant_channel[AUDIO_HOOK_HISTORY_SIZE];                            //
float band_distribution[AUDIO_HOOK_HISTORY_SIZE][3]; // Low=0, mid=1, high=2  //
                                                                              //
uint8_t low_band_emphasis = 0; // 0 or 1                                      //
uint8_t mid_band_emphasis = 0; // 0, 1, or 2                                  //
uint8_t high_band_emphasis = 0; // 0 or 1                                     //
//----------------------------------------------------------------------------//

// LEDs --------------------------------------------------------------------------//
// Physical constants                                                             //
#ifndef PI_CONTROLLED                                                             //
  #define NUM_NODES 1                                                             //
  #define RINGS_PER_NODE 4                                                        //
  #define STRIPS_PER_NODE 4                                                       //
  #define LEDS_PER_STRIP 420                                                      //
  #define VISIBLE_LEDS_PER_RING 408                                               //
#else                                                                             //
  #define NUM_NODES 3                                                             //
  #define RINGS_PER_NODE 12                                                       //
  #define STRIPS_PER_NODE 4                                                       //
  #define LEDS_PER_STRIP 1260                                                     //
  #define VISIBLE_LEDS_PER_RING 408                                               //
#endif                                                                            //
                                                                                  //
#define LEDS_PER_NODE (LEDS_PER_STRIP * STRIPS_PER_NODE)                          //
#define NUM_LEDS (LEDS_PER_NODE * NUM_NODES)                                      //
#define NUM_RINGS (RINGS_PER_NODE * NUM_NODES)                                    //
#define LEDS_PER_RING (NUM_LEDS / NUM_RINGS)                                      //
#define HALF_RING (LEDS_PER_RING/2)                                               //
#define HALF_VISIBLE (VISIBLE_LEDS_PER_RING/2)                                    //
//                                                                                //
// Globals                                                                        //
bool new_animation_triggered = false;                                             //
uint8_t current_animation = 0;                                                    //
uint32_t loop_count = 0;                                                          //
unsigned long current_time=0, animation_start_time=0;                             //
unsigned long long epoch_msec;                                                    //
                                                                                  //
// LED actual data                                                                //
CRGB leds_raw[NUM_RINGS][LEDS_PER_RING];                                          //
                                                                                  //
// Alternative references to LED data; allows for ranged indexing                 //
#ifndef PI_CONTROLLED                                                             //
  CRGBSet leds_all(*leds_raw, NUM_LEDS);                                          //
  CRGBSet leds_node_all(leds_raw[node_number * RINGS_PER_NODE], LEDS_PER_NODE);   //
  CRGBSet leds_node[RINGS_PER_NODE] = {                                           //
    CRGBSet(leds_raw[node_number*RINGS_PER_NODE], LEDS_PER_RING),                 //
    CRGBSet(leds_raw[node_number*RINGS_PER_NODE+1], LEDS_PER_RING),               //
    CRGBSet(leds_raw[node_number*RINGS_PER_NODE+2], LEDS_PER_RING),               //
    CRGBSet(leds_raw[node_number*RINGS_PER_NODE+3], LEDS_PER_RING)};              //
                                                                                  //
  CRGBSet leds[NUM_RINGS] = {                                                     //
    CRGBSet(leds_raw[0], LEDS_PER_RING), CRGBSet(leds_raw[1], LEDS_PER_RING),     //
    CRGBSet(leds_raw[2], LEDS_PER_RING), CRGBSet(leds_raw[3], LEDS_PER_RING)};    //
#else                                                                             //
  CRGBSet leds_all(*leds_raw, NUM_LEDS);                                          //
  CRGBSet leds_node_all(leds_raw[node_number * RINGS_PER_NODE], LEDS_PER_NODE);   //
  CRGBSet leds_node[RINGS_PER_NODE] = {                                           //
    CRGBSet(leds_raw[node_number*RINGS_PER_NODE], LEDS_PER_RING),                 //
    CRGBSet(leds_raw[node_number*RINGS_PER_NODE+1], LEDS_PER_RING),               //
    CRGBSet(leds_raw[node_number*RINGS_PER_NODE+2], LEDS_PER_RING),               //
    CRGBSet(leds_raw[node_number*RINGS_PER_NODE+3], LEDS_PER_RING),               //
    CRGBSet(leds_raw[node_number*RINGS_PER_NODE+4], LEDS_PER_RING),               //
    CRGBSet(leds_raw[node_number*RINGS_PER_NODE+5], LEDS_PER_RING),               //
    CRGBSet(leds_raw[node_number*RINGS_PER_NODE+6], LEDS_PER_RING),               //
    CRGBSet(leds_raw[node_number*RINGS_PER_NODE+7], LEDS_PER_RING),               //
    CRGBSet(leds_raw[node_number*RINGS_PER_NODE+8], LEDS_PER_RING),               //
    CRGBSet(leds_raw[node_number*RINGS_PER_NODE+9], LEDS_PER_RING),               //
    CRGBSet(leds_raw[node_number*RINGS_PER_NODE+10], LEDS_PER_RING),              //
    CRGBSet(leds_raw[node_number*RINGS_PER_NODE+11], LEDS_PER_RING)               //
  };                                                                              //
                                                                                  //
  CRGBSet leds[NUM_RINGS] = {                                                     //
  CRGBSet(leds_raw[0], LEDS_PER_RING), CRGBSet(leds_raw[1], LEDS_PER_RING),       //
  CRGBSet(leds_raw[2], LEDS_PER_RING), CRGBSet(leds_raw[3], LEDS_PER_RING),       //
  CRGBSet(leds_raw[4], LEDS_PER_RING), CRGBSet(leds_raw[5], LEDS_PER_RING),       //
  CRGBSet(leds_raw[6], LEDS_PER_RING), CRGBSet(leds_raw[7], LEDS_PER_RING),       //
  CRGBSet(leds_raw[8], LEDS_PER_RING), CRGBSet(leds_raw[9], LEDS_PER_RING),       //
  CRGBSet(leds_raw[10], LEDS_PER_RING), CRGBSet(leds_raw[11], LEDS_PER_RING),     //
  CRGBSet(leds_raw[12], LEDS_PER_RING), CRGBSet(leds_raw[13], LEDS_PER_RING),     //
  CRGBSet(leds_raw[14], LEDS_PER_RING), CRGBSet(leds_raw[15], LEDS_PER_RING),     //
  CRGBSet(leds_raw[16], LEDS_PER_RING), CRGBSet(leds_raw[17], LEDS_PER_RING),     //
  CRGBSet(leds_raw[18], LEDS_PER_RING), CRGBSet(leds_raw[19], LEDS_PER_RING),     //
  CRGBSet(leds_raw[20], LEDS_PER_RING), CRGBSet(leds_raw[21], LEDS_PER_RING),     //
  CRGBSet(leds_raw[22], LEDS_PER_RING), CRGBSet(leds_raw[23], LEDS_PER_RING),     //
  CRGBSet(leds_raw[24], LEDS_PER_RING), CRGBSet(leds_raw[25], LEDS_PER_RING),     //
  CRGBSet(leds_raw[26], LEDS_PER_RING), CRGBSet(leds_raw[27], LEDS_PER_RING),     //
  CRGBSet(leds_raw[28], LEDS_PER_RING), CRGBSet(leds_raw[29], LEDS_PER_RING),     //
  CRGBSet(leds_raw[30], LEDS_PER_RING), CRGBSet(leds_raw[31], LEDS_PER_RING),     //
  CRGBSet(leds_raw[32], LEDS_PER_RING), CRGBSet(leds_raw[33], LEDS_PER_RING),     //
  CRGBSet(leds_raw[34], LEDS_PER_RING), CRGBSet(leds_raw[35], LEDS_PER_RING)      //
};                                                                                //
#endif                                                                            //
//--------------------------------------------------------------------------------//
    
//  Show parameters coming from the pi ------------------------------------------------------------------//
#define NUM_PARAMETERS 20                                                                                //
#define NUM_COLORS_PER_PALETTE 7                                                                         //
#define NUM_ANIMATIONS 13                                                                                //
                                                                                                         //
//  Indices into show_parameters[] which holds information from the pi                                   //
//  Note: These are only *indices*, not values. Don't change these                                       //
#define ANIMATION_INDEX 0   // which EDM animation to play - for use when we have music                  //
#define BACKGROUND_INDEX 1  // which background animation to use                                         //
#define MIDLAYER_INDEX 2  // which mid layer animation to use                                            //
#define SPARKLE_INDEX 3  // which sparkle animation to use                                               //
#define BEAT_EFFECT_INDEX 4   // how to respond to beat                                                  //
#define NUM_EDM_COLORS_INDEX 5   // how many colors to use out of the edm palette                               //
#define NUM_BG_COLORS_INDEX 6
#define NUM_ML_COLORS_INDEX 7
#define NUM_SP_COLORS_INDEX 8
#define COLOR_THICKNESS_INDEX 9   // how many consecutive lit LEDs in a row                              //
#define BLACK_THICKNESS_INDEX 10   // how many dark LEDs between lit ones                                 //
#define INTRA_RING_MOTION_INDEX 11   // -1 CCW, 0 none, 1 CW, 2 split                                     //
#define INTRA_RING_SPEED_INDEX 12   // 0 to 4                                                             //
#define COLOR_CHANGE_STYLE_INDEX 13   // 0 none, 1 cycle thru selected, 2 cycle thru palette             //
#define RING_OFFSET_INDEX 14  // how far one ring pattern is rotated from neighbor -10 -> 10             //
#define INTER_RING_MOTION_INDEX 15 // which color palette to use                                         //
#define INTER_RING_SPEED_INDEX 16  // 0 to 4                                                             //
#define COLOR_ROTATION 17 // if true, cycle colors through all currently chosen colors                   //
#define COLOR_RAINBOW_INDEX 18  // if true, spread currently chosen colors around structure like rainbow //
                                                                                                         //
                                                                                                         //
#define PALETTE_INDEX 19   // which color palette to use - may become obsolete                           //
                                                                                                         //                                                                                                       
//  Evolving parameters defining the show                                                                //
int show_parameters[NUM_PARAMETERS];                                                                     //
                                                                                                         //
// array of show_parameters[NUM_COLORS_INDEX] colors chosen out of given palette                         //
int show_colors[NUM_COLORS_PER_PALETTE];                                                                 //   
//-------------------------------------------------------------------------------------------------------//



// Color palette choices ------------------------------------------------------//
// Eventually this may be stored in the database if space issues arise
CRGB icy_bright[9] = 
    {CRGB(255,255,255), CRGB(254,207,241),                   // light
    CRGB(255,108,189), CRGB(0,172,238), CRGB(44,133,215),    //  medium
    CRGB(114,78,184), CRGB(227,0,141)};                      // dark

CRGB watermelon[9] = 
    {CRGB(47,192,9), CRGB(70,190,31),                     // light
    CRGB(47,192,9), CRGB(72,160,50), CRGB(148,33,137),    //  medium
    CRGB(120,86,103), CRGB(14,139,0)};                    // dark

CRGB fruit_loop[9] = 
   {CRGB(255,247,0), CRGB(255,127,14),                    // light
    CRGB(188,0,208), CRGB(255,65,65), CRGB(255,73,0),     //  medium
    CRGB(178,6,88), CRGB(162,80,204)};                    // dark


//  Animation categories function arrays ---------------------------------------------------//
typedef void (* function_ptr)();
   function_ptr edm_fn[10] = { &equalizer, &Fire, &frequency_pulse, &toms_best };
   function_ptr background_fn[10] = { &sparkle_rain, &sparkle_glitter };
   function_ptr midlayer_fn[10] = { &sparkle_rain, &sparkle_glitter };
   function_ptr sparkle_fn[10] = { &sparkle_rain, &sparkle_glitter, &sparkle_warp_speed };
   

//  Sparkle layer ---------------------------------------------------//
#define MAX_SPARKLE_INTENSITY 250 // fixme: these should be color dependent to avoid color drift at high intensity
#define MIN_SPARKLE_INTENSITY 50
#define NUM_SPARKLE_FNS 10
 
int sparkle_count = 0;
int current_sparkle = 0;
int current_ring, current_pixel, current_coin_bottom;
CRGB sparkle[NUM_RINGS][VISIBLE_LEDS_PER_RING];
boolean sparkle_is_set[NUM_RINGS][VISIBLE_LEDS_PER_RING];
boolean increasing[NUM_RINGS][VISIBLE_LEDS_PER_RING];
CRGB temp[VISIBLE_LEDS_PER_RING];
boolean temp_is_set[VISIBLE_LEDS_PER_RING];

// fixme: have this come from parameters
CRGB sparkle_color = CRGB::Purple;




// the setup function runs once when you press reset or power the board ------------------------------------//
void setup() {                                                                                              //
  // Setup Serial port                                                                                      //
  #ifdef DEBUG                                                                                              //
    Serial.begin(115200);                                                                                   //
    Serial.println("Serial port initialized.");                                                             //
    Serial.flush();                                                                                         //
  #endif                                                                                                    //
                                                                                                            //
  setup_spectrum_shield();                                                                                  //
                                                                                                            //
  show_parameters[ANIMATION_INDEX] = 0; // Initialize to debugging mode                                     //
                                                                                                            //
  // Initialize digital pin LED_BUILTIN as an output                                                        //
  pinMode(LED_BUILTIN, OUTPUT);                                                                             //
                                                                                                            //
  // Setup LED output ports.                                                                                //
  // This needs to be declared as 8 strips even though we only use 4                                        //
  LEDS.addLeds<WS2811_PORTD, 8>(leds_node_all, LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);              //
                                                                                                            //
  //  Clear all LEDs                                                                                        //
  LEDS.clear();                                                                                             //
  // fixme: isn't working after changes; putting this in as temporary solution
    for (int ring = 0; ring < NUM_RINGS; ring++) {
    for (int pixel = 0; pixel < VISIBLE_LEDS_PER_RING; pixel++) {
      leds[ring][pixel] = CRGB::Black;
    }
  }
                                                                                                            //
  setup_communication();                                                                                    //
}                                                                                                           //
//----------------------------------------------------------------------------------------------------------//

// the loop function runs over and over again forever ------------------------------------//
void loop() {                                                                             //
  #ifdef DEBUG_LED_ARRAYS                                                                 //
    testLEDs();                                                                           //
    delay(10000);                                                                         //
    return;                                                                               //
  #endif                                                                                  //
                                                                                          //
  //  Using REFRESH_TIME will slow animations, but will be more accurate to final product //
  unsigned long now = millis();                                                           //
  if(now - current_time < REFRESH_TIME)                                                   //
    FastLED.delay(now - current_time);                                                    //
  current_time = now;                                                                     //
  loop_count = (current_time - animation_start_time) / REFRESH_TIME;                      //
                                                                                          //
  // read spectrum shield and do beat detection                                           //
  loop_spectrum_shield();                                                                 //
  #ifdef DEBUG_TIMING                                                                     //
    now = millis();                                                                       //
    serial_val[0] = now - current_time;                                                   //
    last_debug_time = now;                                                                //
  #endif                                                                                  //
                                                                                          //
  //  Communicate with pi if available, select animation, other parameters                //
  do_communication();   
  #ifndef PI_CONTROLLED
    manually_update_parameters();  
  #endif
  
  #ifdef DEBUG_TIMING                                                                     //
    now = millis();                                                                       //
    serial_val[1] = now - last_debug_time;                                                //
    last_debug_time = now;                                                                //
  #endif                                                                                  //
                                                                                          //
  //  Draw animation                                                                      //
  draw_current_animation();                                                               //
                                                                                          //
  // Write LEDs                                                                           //
  LEDS.show();                                                                            //
  #ifdef DEBUG_TIMING                                                                     //
    unsigned long now = millis();                                                         //
    serial_val[3] = now - last_debug_time;                                                //
    last_debug_time = now;                                                                //
  #endif                                                                                  //
                                                                                          //
  // Serial output for debugging                                                          //
  #ifdef DEBUG                                                                            //
    write_to_serial();                                                                    //
  #endif                                                                                  //
}                                                                                         //
//----------------------------------------------------------------------------------------//


// Updates show_parameters[] and show_colors[] manually
void manually_update_parameters() {

    #ifdef CYCLE
      cycle_through_animations();
      
    #else
      // choose single animation manually                                       ********** set parameters manually here **********
      show_parameters[ANIMATION_INDEX] = 8;    // <--- this is where you enter your animation number        //
    #endif 

    // Manually assign values to the rest of the animation parameters
    show_parameters[BACKGROUND_INDEX] = 1;                                                                   //
    show_parameters[MIDLAYER_INDEX] = 2;                                                                     //
    show_parameters[SPARKLE_INDEX] = 1;                                                                      //
    show_parameters[BEAT_EFFECT_INDEX] = 0;                                                                  //
    show_parameters[NUM_EDM_COLORS_INDEX] = 3;                                                                   //
    show_parameters[COLOR_THICKNESS_INDEX] = 3;                                                              //
    show_parameters[BLACK_THICKNESS_INDEX] = 8;                                                              //
    show_parameters[INTRA_RING_MOTION_INDEX] = 1;                                                            //
    show_parameters[INTRA_RING_SPEED_INDEX ] = 2;                                                            //
    show_parameters[COLOR_CHANGE_STYLE_INDEX] = 0;                                                           //
    show_parameters[RING_OFFSET_INDEX] = 10;                                                                 // 
    show_parameters[INTER_RING_MOTION_INDEX] = 1;                                                            //
    show_parameters[INTER_RING_SPEED_INDEX] = 2;                                                             //
    show_parameters[COLOR_ROTATION] = 0;                                                                     //
    show_parameters[COLOR_RAINBOW_INDEX] = 0;                                                                //

    show_parameters[PALETTE_INDEX] = 2;                                                                      //

                                                                                                             //                                                                           //
    // can choose 0 to 6 as indices into current palette                                                     //
    // 0,1 light, 2,3,4 mid, 5,6 dark                                                                        //
    show_colors[0] = 0;                                                                                      //                                                                                                                   //
    show_colors[1] = 1;                                                                                      //
    show_colors[2] = 2;                                                                                      //
                                                                                                             //  
    // fixme: doesn't yet have its own parameter, so just working with ones we have                          //
    // need colors 0 or 1 from any palette for sparkle color                                                 // 
    sparkle_color = get_color(show_parameters[PALETTE_INDEX], (INTRA_RING_MOTION_INDEX + 1) % 2);                          
}

 
// Cycles through the animations, running each for ANIMATION_TIME seconds
void cycle_through_animations() {
  //  If an animation has played long enough randomly choose another animation. 
  
  //  Use this global to: SS_PIN_RESET anything that has to do with an animation.
  new_animation_triggered = current_time - animation_start_time >= ANIMATION_TIME;

  if (new_animation_triggered)
  {
    animation_start_time = current_time;
    loop_count = 0;
    
    // current_animation = random8(0, NUM_ANIMATIONS);  // current_animation is a random number from 0 to (NUM_ANIMATIONS - 1)
    current_animation = ++current_animation % NUM_ANIMATIONS;

    #ifdef DEBUG
      Serial.print("New show started: ");
      Serial.println(current_animation) ;
    #endif
  }
}


// Draws the current animation
void draw_current_animation() {

  current_animation = show_parameters[ANIMATION_INDEX];
  
  // This is where you would add a new animation.
  switch (current_animation)  {   
    case 0:
      draw_debug_mode(); // debugging; This should stay as the default animation for setup purposes
      break;

    case 1:
      toms_best_old(); // erick
      break;

    case 2:
      Fire(); // brian
      break;
      
    case 3:
      snake();  // diane
      break; 

    case 4:
      // has the 7 color bands, but doesn't scale with intensity from spectrum shield yet bc
      // alex couldn't figure out how yet
      equalizer();  // alex
      break; 

    case 5:
      basicGradient(); // alex
      break;

    case 6:  
      // in real use, every time a new sparkle is chosen, sparkle_count should be reset to 0
      sparkle_rain();      // diane
      break;

    case 7:
      sparkle_count = 0;
      sparkle_glitter();  // diane
      break;

    case 8:
      sparkle_warp_speed();  // diane
      break;

    case 9:
      frequency_pulse(); // brian
      break;

    case 10:
      scale_usage();
      break;

    default:
      draw_debug_mode(); // debugging; This should stay as the default animation for setup purposes
  }
  // background_fn[show_parameters[BACKGROUND_INDEX]];
  // midlayer_fn[show_parameters[MIDLAYER_INDEX]];
  //if (current_sparkle != show_parameters[SPARKLE_INDEX]) {
      // new sparkle animation has started
      //sparkle_count = 0;
  //}
  // sparkle_fn[show_parameters[SPARKLE_INDEX]];
  // overlay();
}



