
#include <FastLED.h>
typedef void (*sparkle_f_ptr)();     


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
#define REFRESH_TIME 50 // Max 20 frames per second         //
#define ANIMATION_TIME 30000 // 30 seconds per animation    //
                                                            //
// Node-specific                                            //
#define NODE_ID 0 // Change in code running on each node    //
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
uint16_t bpm_estimate = 0;                                                    //
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

// LEDs ----------------------------------------------------------------------//
// Physical constants                                                         //
#define NUM_NODES 3                                                           //
#define RINGS_PER_NODE 4 // This is set up to be 1 strip/ring                 //
#define STRIPS_PER_NODE 4                                                     //
#define LEDS_PER_STRIP 420                                                    //
#define VISIBLE_LEDS_PER_RING 408                                             //
                                                                              //
#define LEDS_PER_NODE (LEDS_PER_STRIP * STRIPS_PER_NODE)                      //
#define NUM_LEDS (LEDS_PER_NODE * NUM_NODES)                                  //
#define NUM_RINGS (RINGS_PER_NODE * NUM_NODES)                                //
#define LEDS_PER_RING (NUM_LEDS / NUM_RINGS)                                  //
#define HALF_RING (LEDS_PER_RING/2)                                           //
#define HALF_VISIBLE (VISIBLE_LEDS_PER_RING/2)                                //
                                                                              //
// Globals                                                                    //
bool new_animation_triggered;                                                 //
uint8_t current_animation = 0;                                                //
uint32_t loop_count = 0;                                                      //
unsigned long current_time=0, animation_saved_time=0, refresh_saved_time=0;   //
                                                                              //
// LED actual data                                                            //
CRGB leds_raw[NUM_RINGS][LEDS_PER_RING];                                      //
                                                                              //
// Alternative references to LED data; allows for ranged indexing             //
CRGBSet leds_all(*leds_raw, NUM_LEDS);                                        //
CRGBSet leds_node_all(leds_raw[NODE_ID * RINGS_PER_NODE], LEDS_PER_NODE);     //
CRGBSet leds_node[RINGS_PER_NODE] = {                                         //
  CRGBSet(leds_raw[NODE_ID*RINGS_PER_NODE], LEDS_PER_RING),                   //
  CRGBSet(leds_raw[NODE_ID*RINGS_PER_NODE+1], LEDS_PER_RING),                 //
  CRGBSet(leds_raw[NODE_ID*RINGS_PER_NODE+2], LEDS_PER_RING),                 //
  CRGBSet(leds_raw[NODE_ID*RINGS_PER_NODE+3], LEDS_PER_RING)                  //
};                                                                            //
                                                                              //
CRGBSet leds[NUM_RINGS] = {                                                   //
  CRGBSet(leds_raw[0], LEDS_PER_RING), CRGBSet(leds_raw[1], LEDS_PER_RING),   //
  CRGBSet(leds_raw[2], LEDS_PER_RING), CRGBSet(leds_raw[3], LEDS_PER_RING),   //
  CRGBSet(leds_raw[4], LEDS_PER_RING), CRGBSet(leds_raw[5], LEDS_PER_RING),   //
  CRGBSet(leds_raw[6], LEDS_PER_RING), CRGBSet(leds_raw[7], LEDS_PER_RING),   //
  CRGBSet(leds_raw[8], LEDS_PER_RING), CRGBSet(leds_raw[9], LEDS_PER_RING),   //
  CRGBSet(leds_raw[10], LEDS_PER_RING), CRGBSet(leds_raw[11], LEDS_PER_RING)  //
};                                                                            //
//----------------------------------------------------------------------------//
    
//  Show parameters coming from the pi -----------------------------------------------------//
#define NUM_PARAMETERS 9                                                                    //
#define NUM_COLORS_PER_PALETTE 3                                                            //
#define NUM_ANIMATIONS 13                                                                   //
                                                                                            //
//  Indices into show_parameters[] which holds information from the pi                      //
//  Note: These are only *indices*, not values. Don't change these                          //
#define ANIMATION_INDEX 0   // which animation to play                                      //
#define BEAT_EFFECT_INDEX 1   // how to respond to beat                                     //
#define PALETTE_INDEX 2   // which color palette to use                                     //
#define NUM_COLORS_INDEX 3   // how many colors to use out of this palette                  //
#define COLOR_THICKNESS_INDEX 4   // how many consecutive lit LEDs in a row                 //
#define BLACK_THICKNESS_INDEX 5   // how many dark LEDs between lit ones                    //
#define INTRA_RING_MOTION_INDEX 6   // -1 CCW, 0 none, 1 CW, 2 split                        //
#define INTRA_RING_SPEED_INDEX 7   // fixme: still need to decide on units                  //
#define COLOR_CHANGE_STYLE_INDEX 8   // 0 none, 1 cycle thru selected, 2 cycle thru palette //
#define RING_OFFSET_INDEX 9  // how far one ring pattern is rotated from neighbor -10 -> 10 //
                                                                                            //
//  Evolving parameters defining the show                                                   //
int show_parameters[NUM_PARAMETERS];                                                        //
                                                                                            //
// array of show_parameters[NUM_COLORS_INDEX] colors chosen out of given palette            //
int show_colors[NUM_COLORS_PER_PALETTE];                                                    //   
//------------------------------------------------------------------------------------------//



// Color palette choices ------------------------------------------------------//
// Eventually this may be stored in the database if space issues arise
CRGB icy_bright[9] = 
    {CRGB(255,255,255), CRGB(254,207,241),                // light
    CRGB(255,108,189), CRGB(0,172,238), CRGB(44,133,215),    //  medium
    CRGB(114,78,184), CRGB(227,0,141)};                   // dark

CRGB watermelon[9] = 
    {CRGB(47,192,9), CRGB(70,190,31),                     // light
    CRGB(47,192,9), CRGB(72,160,50), CRGB(148,33,137),    //  medium
    CRGB(120,86,103), CRGB(14,139,0)};                 // dark

CRGB fruit_loop[9] = 
{CRGB(255,247,0), CRGB(255,127,14),                   // light
    CRGB(188,0,208), CRGB(255,65,65), CRGB(255,73,0),     //  medium
    CRGB(178,6,88), CRGB(162,80,204)};                // dark


//  Sparkle layer ------------------------------------------------------------//
#define MAX_SPARKLE_INTENSITY 250 // fixme: these should be color dependent to avoid color drift at high intensity
#define MIN_SPARKLE_INTENSITY 50
#define NUM_SPARKLE_FNS 10
 
int current_ring, current_pixel, current_coin_bottom;
  
//  Sparkle layer variables
int sparkle_count = 0;
CRGB sparkle_color = CRGB::Purple;
CRGB sparkle[NUM_RINGS][LEDS_PER_RING];  // Sparkle LED layer as a 2D array.
boolean sparkle_is_set[STRIPS_PER_NODE][LEDS_PER_STRIP];
boolean increasing[STRIPS_PER_NODE][LEDS_PER_STRIP];

// array of sparkle functions to make it easier to choose one randomly
//   sparkle_f_ptr sparkle_fn[] = { sparkle_rain() };
//   sparkle_fn[0] = sparkle_twinkle;
//    sparkle_fn[1] = sparkle_rain;
//    sparkle_fn[2] = sparkle_glitter;
//    sparkle_fn[3] = sparkle_wind;
//    sparkle_fn[4] = sparkle_wiggle_wind;
//    sparkle_fn[5] = sparkle_warp_speed;
//    sparkle_fn[6] = comet;
//    sparkle_fn[7] = sparkle_3_circle;
//    sparkle_fn[8] = sparkle_torus_knot();


// the setup function runs once when you press reset or power the board ------------------------------------//
void setup() {                                                                                              //
  // Setup Serial port                                                                                      //
  #ifdef DEBUG                                                                                              //
    Serial.begin(115200);                                                                                     //
    Serial.println("Serial port initialized.");                                                             //
    Serial.flush();                                                                                         //
  #endif                                                                                                    //
                                                                                                            //
  // Initialize digital pin LED_BUILTIN as an output                                                        //
  pinMode(LED_BUILTIN, OUTPUT);                                                                             //
                                                                                                            //
  // Setup LED output ports.                                                                                //
  // This needs to be declared as 8 strips even though we only use 4                                        //
  LEDS.addLeds<WS2811_PORTD, 8>(leds_node_all, LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);              //
                                                                                                            //                                                                                                            //
  //  Clear all LEDs                                                                                        //
  LEDS.clear();                                                                                             //
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
                                                                                          //
  // read spectrum shield and do beat detection                                           //
  loop_spectrum_shield();                                                                 //
  #ifdef DEBUG_TIMING                                                                     //
    now = millis();                                                                       //
    serial_val[0] = now - current_time;                                                   //
    last_debug_time = now;                                                                //
  #endif                                                                                  //
                                                                                          //
  //  Select animation, other parameters                                                  //                                                   //
  update_parameters();                                                                    //
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
                                                                                          //
  loop_count++;                                                                           //
}                                                                                         //
//----------------------------------------------------------------------------------------//


// Updates show_parameters[] and show_colors[] coming from the pi, or manually
void update_parameters() {

    #ifdef PI_CONTROLLED
      // fixme: Jeff: 
      
//    #elif defined(CYCLE)
  //    cycle_through_animations();
      
    #else
      // set parameters manually for testing                   ********** set parameters manually here **********
      show_parameters[ANIMATION_INDEX] = 12;      // <--- this is where you enter your animation number        //
      show_parameters[BEAT_EFFECT_INDEX] = 0;                                                                  //
      show_parameters[PALETTE_INDEX] = 2;                                                                      //
      show_parameters[NUM_COLORS_INDEX] = 3;                                                                   //
      show_parameters[COLOR_THICKNESS_INDEX] = 3;                                                              //
      show_parameters[BLACK_THICKNESS_INDEX] = 8;                                                              //
      show_parameters[INTRA_RING_MOTION_INDEX] = 1;                                                            //
      show_parameters[INTRA_RING_SPEED_INDEX ] = 2;                                                            //
      show_parameters[COLOR_CHANGE_STYLE_INDEX] = 0;                                                           //
      show_parameters[RING_OFFSET_INDEX] = 6;                                                                  //
                                                                                                               //                                                                           //
      // can choose 0 to 6 as indices into current palette                                                     //
      // 0,1 light, 2,3,4 mid, 5,6 dark                                                                        //
      show_colors[0] = 3;                                                                                      //                                                                                                                   //
      show_colors[1] = 0;                                                                                      //
      show_colors[2] = 5;                                                                                      //
                                                                                                               //  
      // fixme: doesn't yet have its own parameter, so just working with ones we have                          //
      // need colors 0 or 1 from any palette for sparkle color                                                 // 
      sparkle_color = get_color(PALETTE_INDEX, (INTRA_RING_MOTION_INDEX + 1) % 2);                             //
    #endif 
}

 
// Cycles through the animations, running each for ANIMATION_TIME seconds
void cycle_through_animations() {
  //  If an animation has played long enough randomly choose another animation. 
  
  //  Use this global to: SS_PIN_RESET anything that has to do with an animation.
  new_animation_triggered = current_time - animation_saved_time >= ANIMATION_TIME;

  if (new_animation_triggered)
  {
    animation_saved_time = current_time;
    loop_count = 0;
    
    // current_animation = random8(0, 4);  // current_animation is a random number from 0 to (2nd number - 1)
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
  
  //  If pixel refresh time has expired update the LED pattern
  if (current_time - refresh_saved_time > REFRESH_TIME)
  {
    refresh_saved_time = current_time;
  
    // This is where you would add a new animation.
    switch (current_animation)  {   
      case 0:
        toms_best();
        break;
  
      case 1:
        Fire(); // brian
        break;
        
      case 3:
        snake();  // diane
        break; 

      case 4:
        run_dot();
        // equalizer();
        break; 
        
      case 5:
        run_dot();
        break; 

       case 7:
          // fixme: make these last 2 parameters more generic
          basicGradient(show_colors[0], show_colors[1], 2, 4);
          break;

      case 8:
        run_dot();
        // snake_to_bow();
        break; 

      case 9:  
        sparkle_count = 0;
        sparkle_rain();      // diane
        break;

      case 10:
        sparkle_count = 0;
        sparkle_3_circles();  // diane
        break;

      case 11:
        sparkle_count = 0;
        sparkle_warp_speed();  // diane
        break;

      case 12:
        frequency_pulse(); // brian
        break;
      default:
        run_dot();
    }
  }
}



