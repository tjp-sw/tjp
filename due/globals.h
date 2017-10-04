#include "tjp.h"

//------------ Node assignment ---------------//
#ifdef PI_CONTROLLED                          //
  uint8_t node_number = 255;                  //
#else                                         //
  uint8_t node_number = TESTING_NODE_NUMBER;  //
#endif                                        //
//--------------------------------------------//

//------------------ Built-in color palettes -----------------------//
CRGB fruit_loop[7] = {                                              //
    //CRGB(188,0,208), CRGB(255,73,0),                    // dark   //
    //CRGB(178,6,88), CRGB(162,80,204), CRGB(255,65,65),  // medium //
    //CRGB(255,247,0), CRGB(255,127,14)};                 // light  //
    CRGB(25,0,25), CRGB(25,15,0),                         // dark   //
    CRGB(90,5,35), CRGB(70,30,90), CRGB(90,30,30),   // medium      //
    CRGB(128,128,60), CRGB(128,50,90)};                // light     //
                                                                    //
#ifndef PI_CONTROLLED                                               //
  CRGB icy_bright[7] = {                                            //
    CRGB(37,28,60), CRGB(70,0,28),                        // dark   //
    CRGB(128,54,90), CRGB(0,83,115), CRGB(20,64,100), // medium     //
    //CRGB(255,0,0), CRGB(0,255,0), CRGB(0,0,255), // medium     //
    CRGB(128,128,128), CRGB(128,100,120)};                // light  //
                                                                    //
  CRGB watermelon[7] = {                                            //
    CRGB(40,29,35), CRGB(5,45,15),                        // dark   //
    CRGB(47,140,9), CRGB(72,160,5), CRGB(100,22,90),     // medium  //
    CRGB(47,192,91), CRGB(70,190,91)};                    // light  //
#endif                                                              //
/*                                                                  //
CRGB fruit_loop[7] = {                                              //
    //CRGB(188,0,208), CRGB(255,73,0),                    // dark   //
    //CRGB(178,6,88), CRGB(162,80,204), CRGB(255,65,65),  // medium //
    //CRGB(255,247,0), CRGB(255,127,14)};                 // light  //
    CRGB(25,0,25), CRGB(25,15,0),                         // dark   //
    CRGB(180,10,70), CRGB(140,60,180), CRGB(180,60,60),   // medium //
    CRGB(255,255,120), CRGB(255,100,180)};                // light  //
                                                                    //
#ifndef PI_CONTROLLED                                               //
  CRGB icy_bright[7] = {                                            //
    CRGB(37,28,60), CRGB(70,0,28),                        // dark   //
    CRGB(255,108,189), CRGB(0,172,238), CRGB(44,133,215), // medium //
    CRGB(255,255,255), CRGB(254,207,241)};                // light  //
                                                                    //
  CRGB watermelon[7] = {                                            //
    CRGB(40,29,35), CRGB(5,45,15),                        // dark   //
    CRGB(47,140,9), CRGB(72,160,5), CRGB(148,33,137),     // medium //
    CRGB(47,192,91), CRGB(70,190,91)};                    // light  //
#endif                                                              //
*/                                                                  //
                                                                    //
CRGB* initial_palette = fruit_loop;                                 //
//------------------------------------------------------------------//

//----------------- Show modes -----------------------------------------//
//                                                                      //
#define SUNRISE 0;                                                      //
#define DAY 1;                                                          //
#define SUNSET 2;                                                       //
#define NIGHT 3;                                                        //
#define ART_CAR 4;                                                      //
#define NO_ART_CAR 255;                                                 //
//                                                                      //
// set to estimated ring nearest art car if art car is recognized       //
uint8_t art_car = NO_ART_CAR;                                           //
//----------------------------------------------------------------------//

//----------------- Animation indexes ------------------------//
#define OFF 255    // Disables a layer, CYCLE will ignore it  //
#define NONE 0     // An "empty" animation; CYCLE will work   //
                                                              //
// Base layer                                                 //
#define BASE_SCROLLING_DIM 1                                  //
#define BASE_2COLOR_GRADIENT 2                                //
#define BASE_SCROLLING_HALF_DIM 3                             //
#define BASE_HORIZONTAL_GRADIENT 4
#define LEE_COLOR_RANGE 250                                   //
#define LEE_BRIGHTNESS 249                                    //
#define LEE_CHECK 248                                         //
#define LEE_PICK_HSV 247                                      //
#define NUM_BASE_ANIMATIONS 3  // Equal to last animation     //
                                                              //
// Mid layer                                                  //
#define SNAKE 1                                               //
#define FIRE 2                                                //
#define DISCO_FIRE 3                                          //
#define FIRE_ONE_SIDED 254  // Only used with EDM animation   //
#define DISCO_FIRE_ONE_SIDED 253                              //
#define MID_SCROLLING_DIM2 4                                  //
#define MID_SCROLLING_DIM3 5                                  //
#define MID_SCROLLING_DIM4 6                                  //
#define MID_SCROLLING_DIM5 7                                  //
#define ARROW 8                                               //
#define RADIATION 9                                          //
#define SQUARE 10                                             //
#define SQUARE2 11                                            //
#define WAVE 12                                               //
#define NUM_MID_ANIMATIONS 12  // Equal to last animation     //
                                                              //
// Sparkle layer                                              //
#define GLITTER 1                                             //
#define RAIN 2                                                //
#define WARP_SPEED 3                                          //
#define TWINKLE 4                                             //
#define VARIABLE_SPIN 5                                       //
#define THREE_CIRCLES 6                                       //
#define CIRCLE_TRAILS 7                                       //
#define TWO_COINS 8                                           //
#define TORUS_KNOT 9                                          //
#define TORUS_LINK 10                                         //
#define PANIC 11                                              //
#define NUM_SPARKLE_ANIMATIONS 9  // Equal to last animation //
#define STATIC 254  // only used on Sunday before BM start    //
                                                              //
// EDM animations (in the base layer, starting at 128)        //
#define FREQ_PULSE 1                                          //
#define EQ_FULL 2                                             //
#define EQ_FULL_SPLIT 3                                       //
#define EQ_VARIABLE 4                                         //
#define EQ_VARIABLE_SPLIT 5                                   //
#define EQ_VARIABLE_FIRE 6                                    //
#define EQ_PULSE 7                                            //
#define NUM_EDM_ANIMATIONS 7  // Equal to last animation      //
                                                              //
// Hello animations                                           //
#define KILL_ANIMATION 128                                    //
#define CHAKRA_PULSE 129                                      //
#define ANIMATION_PULSE 130                                   //
#define LIGHTNING 131                                         //
#define FIRE_HELLO 132                                        //
#define BLACK_HOLE 133                                        //

// Meditation animations
#define MEDITATION_SUNRISE 192
#define MEDITATION_SUNSET 193
                                                              //
// Debug animations (drawn on EDM layer)                      //
#define DEBUG_MODE 254        // Startup animation            //
#define TEST_STRANDS 253      // Test for burnt out LEDs      //
                                                              //
//------------------------------------------------------------//


//------------------------------------ Animation parameters --------------------------------------//
//  Indices into show_parameters[] which holds information from the pi                            //
//  Note: These are only *indices*, not values. Don't change these                                //
#define BASE_ANIMATION_INDEX 0              // which base animation to play                       //
#define BASE_COLOR_THICKNESS_INDEX 1        // how many consecutive lit LEDs in a row             //
#define BASE_BLACK_THICKNESS_INDEX 2        // how many dark LEDs between lit ones                //
#define BASE_RING_OFFSET_INDEX 3            // rotation of pattern from neighbor                  //
#define BASE_INTRA_RING_MOTION_INDEX 4      // CW, NONE, CW, ALTERNATE, SPLIT                     //
#define BASE_INTRA_RING_SPEED_INDEX 5       // Number of pixels to move per 16 cycles             //
#define BASE_INTER_RING_MOTION_INDEX 6      // CCW, NONE, CW,                                     //
#define BASE_INTER_RING_SPEED_INDEX 7       // Number of pixels to move per 16 cycles             //
                                                                                                  //
#define MID_ANIMATION_INDEX 8               // which mid animation to play                        //
#define MID_NUM_COLORS_INDEX 9              // how many colors to use out of this palette         //
#define MID_COLOR_THICKNESS_INDEX 10        // how many consecutive lit LEDs in a row             //
#define MID_BLACK_THICKNESS_INDEX 11        // how many dark LEDs between lit ones                //
#define MID_RING_OFFSET_INDEX 12            // rotation of pattern from neighbor                  //
#define MID_INTRA_RING_MOTION_INDEX 13      // CW, NONE, CW, ALTERNATE, SPLIT                     //
#define MID_INTRA_RING_SPEED_INDEX 14       // Number of pixels to move per 16 cycles             //
#define MID_INTER_RING_MOTION_INDEX 15      // CCW, NONE, CW,                                     //
#define MID_INTER_RING_SPEED_INDEX 16       // Number of pixels to move per 16 cycles             //
                                                                                                  //
#define SPARKLE_ANIMATION_INDEX 17          // which sparkle animation to play                    //
#define SPARKLE_PORTION_INDEX 18            // percentage of LEDs that will be lit at once        //
#define SPARKLE_COLOR_THICKNESS_INDEX 19    // how many consecutive lit LEDs in a row             //
#define SPARKLE_INTRA_RING_MOTION_INDEX 20  // CW, NONE, CW, ALTERNATE, SPLIT                     //
#define SPARKLE_INTRA_RING_SPEED_INDEX 21   // Number of pixels to move per 16 cycles             //
#define SPARKLE_INTER_RING_MOTION_INDEX 22  // CCW, NONE, CW,                                     //
#define SPARKLE_INTER_RING_SPEED_INDEX 23   // Number of pixels to move per 16 cycles             //
#define SPARKLE_MIN_DIMMING_INDEX 24        // Min amount a sparkle will be dimmed                //
#define SPARKLE_MAX_DIMMING_INDEX 25        // Max amount a sparkle will be dimmed                //
#define SPARKLE_RANGE_INDEX 26              // Range of pixels that new bursts are created in     //
#define SPARKLE_SPAWN_FREQUENCY_INDEX 27    // How often new bursts of sparkles are created       //
                                                                                                  //
#define EDM_ANIMATION_INDEX 28              // Which EDM animation to play                        //
#define ART_CAR_RING_INDEX 29               // Closest ring to the art car                        //
#define BEAT_EFFECT_INDEX 30                // How the beat should affect animations              //
                                                                                                  //
#define PALETTE_CHANGE_INDEX 31             // How to switch into next palette                    //
                                                                                                  //
#define BASE_TRANSITION_INDEX 32             // How to transition base animations                 //
#define BASE_TRANSITION_SPEED_INDEX 33       // How fast to transition base animations            //
#define MID_TRANSITION_INDEX 34              // How to transition mid animations                  //
#define MID_TRANSITION_SPEED_INDEX 35        // How fast to transition mid animations             //
#define SPARKLE_TRANSITION_INDEX 36          // How to transition sparkle animations              //
#define SPARKLE_TRANSITION_SPEED_INDEX 37    // How fast to transition sparkle animations         //
#define EDM_TRANSITION_INDEX 38              // How to transition edm animations                  //
#define EDM_TRANSITION_SPEED_INDEX 39        // How fast to transition edm animations             //
                                                                                                  //
//#define NUM_SHOW_PARAMETERS 40             // Set this in tjp.h                                 //
                                                                                                  //
//  Evolving parameters defining the show, also automatically converts from unsigned/signed       //
uint8_t show_parameters[NUM_SHOW_PARAMETERS];                                                     //
                                                                                                  //
// Shorthand to access show_parameters                                                            //
#define BASE_ANIMATION            (show_parameters[BASE_ANIMATION_INDEX])                         //
#define MID_ANIMATION             (show_parameters[MID_ANIMATION_INDEX])                          //
#define SPARKLE_ANIMATION         (show_parameters[SPARKLE_ANIMATION_INDEX])                      //
#define EDM_ANIMATION             (show_parameters[EDM_ANIMATION_INDEX])                          //
                                                                                                  //
#define PALETTE_CHANGE            (show_parameters[PALETTE_CHANGE_INDEX])                         //
#define BEAT_EFFECT               (show_parameters[BEAT_EFFECT_INDEX])                            //
#define ART_CAR_RING              ((int8_t)show_parameters[ART_CAR_RING_INDEX])                   //
                                                                                                  //
#define BASE_COLOR_THICKNESS      (show_parameters[BASE_COLOR_THICKNESS_INDEX])                   //
#define BASE_BLACK_THICKNESS      (show_parameters[BASE_BLACK_THICKNESS_INDEX])                   //
#define BASE_INTRA_RING_MOTION    ((int8_t)show_parameters[BASE_INTRA_RING_MOTION_INDEX])         //
#define BASE_INTRA_RING_SPEED     (show_parameters[BASE_INTRA_RING_SPEED_INDEX])                  //
#define BASE_INTER_RING_MOTION    ((int8_t)show_parameters[BASE_INTER_RING_MOTION_INDEX])         //
#define BASE_INTER_RING_SPEED     (show_parameters[BASE_INTER_RING_SPEED_INDEX])                  //
#define BASE_RING_OFFSET          ((int8_t)show_parameters[BASE_RING_OFFSET_INDEX])               //
                                                                                                  //
#define MID_NUM_COLORS            (show_parameters[MID_NUM_COLORS_INDEX])                         //
#define MID_COLOR_THICKNESS       (show_parameters[MID_COLOR_THICKNESS_INDEX])                    //
#define MID_BLACK_THICKNESS       (show_parameters[MID_BLACK_THICKNESS_INDEX])                    //
#define MID_INTRA_RING_MOTION     ((int8_t)show_parameters[MID_INTRA_RING_MOTION_INDEX])          //
#define MID_INTRA_RING_SPEED      (show_parameters[MID_INTRA_RING_SPEED_INDEX])                   //
#define MID_INTER_RING_MOTION     ((int8_t)show_parameters[MID_INTER_RING_MOTION_INDEX])          //
#define MID_INTER_RING_SPEED      (show_parameters[MID_INTER_RING_SPEED_INDEX])                   //
#define MID_RING_OFFSET           ((int8_t)show_parameters[MID_RING_OFFSET_INDEX])                //
                                                                                                  //
#define SPARKLE_COLOR_THICKNESS   (show_parameters[SPARKLE_COLOR_THICKNESS_INDEX])                //
#define SPARKLE_PORTION           (show_parameters[SPARKLE_PORTION_INDEX])                        //
#define SPARKLE_INTRA_RING_MOTION ((int8_t)show_parameters[SPARKLE_INTRA_RING_MOTION_INDEX])      //
#define SPARKLE_INTRA_RING_SPEED  (show_parameters[SPARKLE_INTRA_RING_SPEED_INDEX])               //
#define SPARKLE_INTER_RING_MOTION ((int8_t)show_parameters[SPARKLE_INTER_RING_MOTION_INDEX])      //
#define SPARKLE_INTER_RING_SPEED  (show_parameters[SPARKLE_INTER_RING_SPEED_INDEX])               //
#define SPARKLE_MIN_DIM           (show_parameters[SPARKLE_MIN_DIMMING_INDEX])                    //
#define SPARKLE_MAX_DIM           (show_parameters[SPARKLE_MAX_DIMMING_INDEX])                    //
#define SPARKLE_RANGE             (show_parameters[SPARKLE_RANGE_INDEX])                          //
#define SPARKLE_SPAWN_FREQUENCY   (show_parameters[SPARKLE_SPAWN_FREQUENCY_INDEX])                //
                                                                                                  //
#define BASE_TRANSITION           (show_parameters[BASE_TRANSITION_INDEX])                        //
#define BASE_TRANSITION_SPEED     (show_parameters[BASE_TRANSITION_SPEED_INDEX])                  //
#define MID_TRANSITION            (show_parameters[MID_TRANSITION_INDEX])                         //
#define MID_TRANSITION_SPEED      (show_parameters[MID_TRANSITION_SPEED_INDEX])                   //
#define SPARKLE_TRANSITION        (show_parameters[SPARKLE_TRANSITION_INDEX])                     //
#define SPARKLE_TRANSITION_SPEED  (show_parameters[SPARKLE_TRANSITION_SPEED_INDEX])               //
#define EDM_TRANSITION            (show_parameters[EDM_TRANSITION_INDEX])                         //
#define EDM_TRANSITION_SPEED      (show_parameters[EDM_TRANSITION_SPEED_INDEX])                   //
                                                                                                  //
//------------------------------------------------------------------------------------------------//

//----------- Palette change modes -----------//
#define PALETTE_CHANGE_IMMEDIATE 0            //
#define PALETTE_CHANGE_VERY_SLOW_BLEND 1      //
#define PALETTE_CHANGE_SLOW_BLEND 2           //
#define PALETTE_CHANGE_MEDIUM_BLEND 3         //
#define PALETTE_CHANGE_FAST_BLEND 4           //
#define PALETTE_CHANGE_VERY_FAST_BLEND 5      //
#define NUM_PALETTE_CHANGE_TYPES 5            //
//--------------------------------------------//

//------------------ Palette blend rates ---------------------------//
#define PALETTE_CHANGES_VERY_SLOW 1                                 //
#define PALETTE_CHANGES_SLOW 1                                      //
#define PALETTE_CHANGES_MEDIUM 2                                    //
#define PALETTE_CHANGES_FAST 3                                      //
#define PALETTE_CHANGES_VERY_FAST 9                                 //
//------------------------------------------------------------------//

//----------- Alpha blending -------------//
bool override_default_blending = false;   //
//----------------------------------------//

//------- Animation transitions ----------//
#define TRANSITION_BY_ALPHA 1             //
                                          //
#define NUM_BASE_TRANSITION_MODES 1       //
#define NUM_MID_TRANSITION_MODES 1        //
#define NUM_SPARKLE_TRANSITION_MODES 1    //
#define NUM_EDM_TRANSITION_MODES 1        //
//----------------------------------------//

//-------------- Animation transition speeds -----------------------//
#define VERY_SLOW_TRANSITION 1                                      //
#define SLOW_TRANSITION 2                                           //
#define MEDIUM_TRANSITION 3                                         //
// 4 will function as expected but isn't named                      //
#define FAST_TRANSITION 5                                           //
#define VERY_FAST_TRANSITION 6                                      //
#define MAX_TRANSITION_SPEED 6    // Set equal to highest value     //
//------------------------------------------------------------------//

//-------------------- Beat effects --------------------------//
#define COLOR_SWAP 1                                          //
#define ALTERNATE_COLOR_THICKNESS 2                           //
#define ALTERNATE_BLACK_THICKNESS 3                           //
#define PULSE_COLOR_THICKNESS 4                               //
#define PULSE_BLACK_THICKNESS 5                               //
#define JERKY_MOTION 6                                        //
#define BLACKEN_NODE 7                                        //
#define BLACKEN_RING 8                                        //
#define NUM_BEAT_EFFECTS 8 // Should be equal to last number  //
                                                              //
boolean odd_beat = true;                                      //
boolean blacken_ring = false;                                 //
int8_t blacken_ring_number = 0;                               //
boolean blacken_node = false;                                 //
int8_t blacken_node_number = 0;                               //
//------------------------------------------------------------//

//------------------- Physical structure ---------------------------------//
#define NUM_NODES 6                                                       //
#define RINGS_PER_NODE 12                                                 //
#define STRIPS_PER_NODE 6 // Everyone should update this to 6! Leaving it at 4 to not impose, since it may require re-wiring your LED strips. //
#define PHYSICAL_LEDS_PER_RING 420                                        //
#define LEDS_PER_RING 400                                                 //
                                                                          //
#if STRIPS_PER_NODE == 4                                                  //
  #define LEDS_PER_STRIP (2*PHYSICAL_LEDS_PER_RING + LEDS_PER_RING + 1)   //
#elif STRIPS_PER_NODE == 6                                                //
  #define LEDS_PER_STRIP (PHYSICAL_LEDS_PER_RING + LEDS_PER_RING + 1)     //
#endif                                                                    //
                                                                          //
#define NUM_RINGS (RINGS_PER_NODE * NUM_NODES)                            //
#define LEDS_PER_NODE (LEDS_PER_RING * RINGS_PER_NODE)                    //
#define PHYSICAL_LEDS_PER_NODE (LEDS_PER_STRIP*STRIPS_PER_NODE)           //
#define HALF_RING (LEDS_PER_RING/2)                                       //
#define NUM_LEDS (LEDS_PER_NODE * NUM_NODES)                              //
#define RINGS_PER_STRIP (RINGS_PER_NODE / STRIPS_PER_NODE)                //
//------------------------------------------------------------------------//

//----------------------------- Loop counters/timers -----------------------------//
uint32_t base_count = 0, mid_count = 0, sparkle_count = 0, edm_count = 0, loop_count = 0;//
unsigned long current_time = 0;                                                   //
unsigned long long epoch_msec = 0;                                                //
unsigned long base_start_time = 0, mid_start_time = 0, sparkle_start_time = 0;    //
unsigned long edm_start_time = 0;                                                 //
                                                                                  //
#if defined(CYCLE) || defined(CYCLE_RANDOM) || defined(CYCLE_PARAMS)              //
  unsigned long palette_start_time = 0;                                           //
  unsigned long beat_effect_start_time = 0;                                       //
#endif                                                                            //
//--------------------------------------------------------------------------------//


//  ---------------------------------- LEDs, layers, and palettes ----------------------------//
#define NUM_COLORS_PER_PALETTE 7                                                              //
                                                                                              //
CRGB leds[LEDS_PER_STRIP*STRIPS_PER_NODE]; // 1 - 408 - 12 - 408 - 1 - 408 - 12 - 408 - 1 ... //
CRGBSet leds_all(leds, LEDS_PER_STRIP*STRIPS_PER_NODE);                                       //
                                                                                              //
uint8_t mid_layer[NUM_RINGS][LEDS_PER_RING];                                                  //
uint8_t sparkle_layer[NUM_RINGS][LEDS_PER_RING];                                              //
                                                                                              //
CRGBPalette256 mid_palette;                                                                   //
CRGBPalette256 sparkle_palette;                                                                //
CRGB target_palette[NUM_COLORS_PER_PALETTE];  // Used to smoothly blend into next palette     //
CRGB current_palette[NUM_COLORS_PER_PALETTE]; // The current 7-color palette                  //
                                                                                              //
bool blend_base_layer = true;                                                                 //
bool blend_mid_layer = true;                                                                  //
bool blend_sparkle_layer = true;                                                              //
bool transition_out_base_animation = false, transition_in_base_animation = false;             //
bool transition_out_mid_animation = false, transition_in_mid_animation = false;               //
bool transition_out_sparkle_animation = false, transition_in_sparkle_animation = false;       //
bool transition_out_edm_animation = false, transition_in_edm_animation = false;               //
uint8_t next_base_animation = NONE;                                                           //
uint8_t next_mid_animation = NONE;                                                            //
uint8_t next_sparkle_animation = NONE;                                                        //
uint8_t next_edm_animation = NONE;                                                            //
uint8_t transition_progress_base = 0;                                                         //
uint8_t transition_progress_mid = 0;                                                          //
uint8_t transition_progress_sparkle = 0;                                                      //
uint8_t transition_progress_edm = 0;                                                          //
                                                                                              //
//--------------------------------------------------------------------------------------------//


// Spectrum Shield -----------------------------------------------------------//
// Pin connections                                                            //
#define SS_PIN_STROBE 4                                                       //
#define SS_PIN_RESET 5                                                        //
#define SS_PIN_DC_ONE A0                                                      //
#define SS_PIN_DC_TWO A1                                                      //
#define NUM_BANDS 3 // Bass, Mid, Treble                                      //
                                                                              //
// Outputs to Pi                                                              //
uint8_t freq_internal[NUM_CHANNELS];                                          //
uint8_t freq_external[NUM_CHANNELS];                                          //
                                                                              //
// Inputs from Pi                                                             //
unsigned long long next_beat_prediction = 0;                                  //
                                                                              //
// Computed internally                                                        //
uint8_t freq_smooth[NUM_CHANNELS];                                            //
unsigned long long last_beat_prediction = 0;                                  //
uint8_t downbeat_proximity = 0; // 0-255, inferred from beat predictions      //
bool is_beat = false; // Inferred from downbeat_proximity                     //
uint8_t band_distribution[NUM_BANDS]; // bass=0, mid=1, treble=2; sums to 255 //
//----------------------------------------------------------------------------//



//------ Misc enumerations and values --------//
#define THROTTLE 16 // Allows INTRA_RING_SPEED to speed up OR slow down an animation

// Palettes
#define MID_GRADIENT_SIZE 7
#define MAX_MID_DIMMING 11
#define NUM_MID_DIMMING_LEVELS (MAX_MID_DIMMING + 1)
#define MAX_SPARKLE_DIMMING 126
#define NUM_SPARKLE_DIMMING_LEVELS (MAX_SPARKLE_DIMMING + 1)

// Bands
#define BASS_BAND 0
#define MID_BAND 1
#define TREBLE_BAND 2

// Layers
#define BASE_LAYER 1
#define MID_LAYER 2
#define SPARKLE_LAYER 3

// Layer colors
#define TRANSPARENT 0
#define WHITE 1
#define BLACK 2
#define MID_LAYER_SPARKLE 3

// Base and mid INTRA_RING_MOTION values
//#define NONE 0 // Defined above in animations
#define CW 1
#define CCW -1
#define ALTERNATE 2
#define SPLIT 3

// Sparkle INTRA_RING_MOTION values (also used in MID_FIRE animations)
#define DOWN 1
#define UP -1

// EDM animations that can be displayed one-sided or mirrored
#define DISPLAY_FULL 0
#define DISPLAY_SPLIT 1
#define DISPLAY_ONE_SIDED 2

// Fire palettes
#define FIRE_PALETTE_DISABLED 0
#define FIRE_PALETTE_STANDARD 1

// Diane's sparkle globals - if this gets to be too much I'll try to figure out a way to be more space efficient
uint8_t sparkle_current_ring;
uint8_t sparkle_current_pixel;
uint8_t sparkle_current_coin;
