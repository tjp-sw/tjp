//------------ Node assignment ---------------//
#ifdef PI_CONTROLLED                          //
  uint8_t node_number = 255;                  //
#else                                         //
  uint8_t node_number = TESTING_NODE_NUMBER;  //
#endif                                        //
//--------------------------------------------//


//------------------ Built-in color palettes -----------------------//
CRGB fruit_loop[7] = {                                              //
    //CRGB(188,0,208), CRGB(255,73,0),                      // dark   //
    //CRGB(178,6,88), CRGB(162,80,204), CRGB(255,65,65),    // medium //
    //CRGB(255,247,0), CRGB(255,127,14)};                   // light  //
    CRGB(25,0,25), CRGB(25,15,0),                         // dark   //
    CRGB(180,10,70), CRGB(140,60,180), CRGB(180,60,60),    // medium //
    CRGB(255,255,120), CRGB(255,100,180)};                // light  //
                                                                    //
#ifndef PI_CONTROLLED                                               //
  CRGB icy_bright[7] = {                                            //
    CRGB(114,78,184), CRGB(227,0,141),                    // dark   //
    CRGB(255,108,189), CRGB(0,172,238), CRGB(44,133,215), // medium //
    CRGB(255,255,255), CRGB(254,207,241)};                // light  //
                                                                    //
  CRGB watermelon[7] = {                                            //
    CRGB(120,86,103), CRGB(148,33,137),                   // dark   //
    CRGB(47,140,9), CRGB(72,160,5), CRGB(14,139,50),      // medium //
    CRGB(47,192,91), CRGB(70,190,91)};                    // light  //
#endif                                                              //
                                                                    //
CRGB* initial_palette = fruit_loop;                                 //
//------------------------------------------------------------------//


//------------------------------------ Animation parameters --------------------------------------//
//  Indices into show_parameters[] which holds information from the pi                            //
//  Note: These are only *indices*, not values. Don't change these                                //
#define BASE_ANIMATION_INDEX 0              // which base animation to play                       //
#define MID_ANIMATION_INDEX 1               // which mid animation to play                        //
#define SPARKLE_ANIMATION_INDEX 2           // which sparkle animation to play                    //
                                                                                                  //
// Base parameters                                                                                //
#define BASE_COLOR_THICKNESS_INDEX 3        // how many consecutive lit LEDs in a row             //
#define BASE_BLACK_THICKNESS_INDEX 4        // how many dark LEDs between lit ones                //
#define BASE_INTRA_RING_MOTION_INDEX 5      // -1 CCW, 0 none, 1 CW, 2 split                      //
#define BASE_INTRA_RING_SPEED_INDEX 6       // Number of pixels to move per 16 cycles             //
#define BASE_RING_OFFSET_INDEX 7            // rotation of pattern from neighbor                  //
                                                                                                  //
#define MID_NUM_COLORS_INDEX 8              // how many colors to use out of this palette         //
#define MID_COLOR_THICKNESS_INDEX 9         // how many consecutive lit LEDs in a row             //
#define MID_BLACK_THICKNESS_INDEX 10        // how many dark LEDs between lit ones                //
#define MID_INTRA_RING_MOTION_INDEX 11      // -1 CCW, 0 none, 1 CW, 2 split                      //
#define MID_INTRA_RING_SPEED_INDEX 12       // Number of pixels to move per 16 cycles             //
#define MID_RING_OFFSET_INDEX 13            // rotation of pattern from neighbor                  //
                                                                                                  //
#define SPARKLE_COLOR_THICKNESS_INDEX 14    // how many consecutive lit LEDs in a row             //
#define SPARKLE_PORTION_INDEX 15            // percentage of LEDs that will be lit at once        //
#define SPARKLE_INTRA_RING_MOTION_INDEX 16  // -1 Upward, 0 none, 1 Downward                      //
#define SPARKLE_INTRA_RING_SPEED_INDEX 17   // Number of pixels to move per 16 cycles             //
#define SPARKLE_MAX_DIMMING_INDEX 18        // Max amount a sparkle will be dimmed                //
#define SPARKLE_RANGE_INDEX 19              // Range of pixels that new bursts are created in     //
#define SPARKLE_SPAWN_FREQUENCY_INDEX 20    // How often new bursts of sparkles are created       //
                                                                                                  //
#define NUM_SHOW_PARAMETERS 21              // This should be 1 greater than the last index       //
                                                                                                  //
                                                                                                  //
//  Evolving parameters defining the show                                                         //
int8_t show_parameters[NUM_SHOW_PARAMETERS];                                                      //
                                                                                                  //
// Shorthand to access show_parameters                                                            //
#define BASE_ANIMATION            (show_parameters[BASE_ANIMATION_INDEX])                         //
#define MID_ANIMATION             (show_parameters[MID_ANIMATION_INDEX])                          //
#define SPARKLE_ANIMATION         (show_parameters[SPARKLE_ANIMATION_INDEX])                      //
                                                                                                  //
#define BASE_COLOR_THICKNESS      (show_parameters[BASE_COLOR_THICKNESS_INDEX])                   //
#define BASE_BLACK_THICKNESS      (show_parameters[BASE_BLACK_THICKNESS_INDEX])                   //
#define BASE_INTRA_RING_MOTION    (show_parameters[BASE_INTRA_RING_MOTION_INDEX])                 //
#define BASE_INTRA_RING_SPEED     (show_parameters[BASE_INTRA_RING_SPEED_INDEX])                  //
#define BASE_RING_OFFSET          (show_parameters[BASE_RING_OFFSET_INDEX])                       //
                                                                                                  //
#define MID_NUM_COLORS            (show_parameters[MID_NUM_COLORS_INDEX])                         //
#define MID_COLOR_THICKNESS       (show_parameters[MID_COLOR_THICKNESS_INDEX])                    //
#define MID_BLACK_THICKNESS       (show_parameters[MID_BLACK_THICKNESS_INDEX])                    //
#define MID_INTRA_RING_MOTION     (show_parameters[MID_INTRA_RING_MOTION_INDEX])                  //
#define MID_INTRA_RING_SPEED      (show_parameters[MID_INTRA_RING_SPEED_INDEX])                   //
#define MID_RING_OFFSET           (show_parameters[MID_RING_OFFSET_INDEX])                        //
                                                                                                  //
#define SPARKLE_COLOR_THICKNESS   (show_parameters[SPARKLE_COLOR_THICKNESS_INDEX])                //
#define SPARKLE_PORTION           (show_parameters[SPARKLE_PORTION_INDEX])                        //
#define SPARKLE_INTRA_RING_MOTION (show_parameters[SPARKLE_INTRA_RING_MOTION_INDEX])              //
#define SPARKLE_INTRA_RING_SPEED  (show_parameters[SPARKLE_INTRA_RING_SPEED_INDEX])               //
#define SPARKLE_MAX_DIM           (show_parameters[SPARKLE_MAX_DIMMING_INDEX])                    //
#define SPARKLE_RANGE             (show_parameters[SPARKLE_RANGE_INDEX])                          //
#define SPARKLE_SPAWN_FREQUENCY   (show_parameters[SPARKLE_SPAWN_FREQUENCY_INDEX])                //
                                                                                                  //
// Animation constants                                                                            //
#define THROTTLE 16 // Allows INTRA_RING_SPEED to speed up OR slow down an animation              //
#define BASE_MAX_GRADIENT_LENGTH 30 // Plus the pure colors on either side makes 32               //
#define MID_MAX_GRADIENT_LENGTH 10  // Plus the pure colors on either side makes 12               //
//------------------------------------------------------------------------------------------------//


// ------------------ Physical structure ---------------------------------//
#define NUM_NODES 6                                                       //
#define RINGS_PER_NODE 12                                                 //
#define STRIPS_PER_NODE 4                                                 //
#define PHYSICAL_LEDS_PER_RING 420                                        //
#define LEDS_PER_RING 408                                                 //
                                                                          //
#define LEDS_PER_STRIP (2*PHYSICAL_LEDS_PER_RING + LEDS_PER_RING + 1)     //
#define NUM_RINGS (RINGS_PER_NODE * NUM_NODES)                            //
#define LEDS_PER_NODE (LEDS_PER_RING * RINGS_PER_NODE)                    //
#define PHYSICAL_LEDS_PER_NODE (LEDS_PER_STRIP*STRIPS_PER_NODE)           //
#define HALF_RING (LEDS_PER_RING/2)                                       //
#define NUM_LEDS (LEDS_PER_NODE * NUM_NODES)                              //
#define RINGS_PER_STRIP (RINGS_PER_NODE / STRIPS_PER_NODE)                //
//------------------------------------------------------------------------//


//----------------------------- Loop counters/timers -----------------------------//
uint32_t base_count = 0, mid_count = 0, sparkle_count = 0, loop_count = 0;        //
unsigned long current_time = 0;                                                   //
unsigned long long epoch_msec = 0;                                                //
unsigned long base_start_time = 0, mid_start_time = 0, sparkle_start_time = 0;    //
                                                                                  //
#if defined(CYCLE) || defined(CYCLE_RANDOM)                                       //
  unsigned long palette_start_time = 0;                                           //
#endif                                                                            //
//--------------------------------------------------------------------------------//


//  ---------------------------------- LEDs, layers, and palettes ----------------------------//
#define NUM_COLORS_PER_PALETTE 7                                                              //
                                                                                              //
CRGB leds[LEDS_PER_STRIP*STRIPS_PER_NODE]; // 1 - 408 - 12 - 408 - 12 - 408 - 1 - 408 - ...   //
CRGBSet leds_all(leds, LEDS_PER_STRIP*STRIPS_PER_NODE);                                       //
                                                                                              //
uint8_t base_layer[NUM_RINGS][LEDS_PER_RING];                                                 //
uint8_t mid_layer[NUM_RINGS][LEDS_PER_RING];                                                  //
uint8_t sparkle_layer[NUM_RINGS][LEDS_PER_RING/2];                                            //
uint8_t disable_layering = true;                                                              //
                                                                                              //
CRGBPalette256 base_palette;                                                                  //
CRGBPalette256 base_palette_target;                                                           //
CRGBPalette256 mid_palette;                                                                   //
CRGBPalette256 mid_palette_target;                                                            //
CRGBPalette16 sparkle_palette;                                                                //
CRGBPalette16 sparkle_palette_target;                                                         //
                                                                                              //
bool blend_base_layer = true;                                                                 //
bool blend_mid_layer = true;                                                                  //
bool blend_sparkle_layer = true;                                                              //
                                                                                              //
#define TRANSPARENT 0                                                                         //
#define WHITE 1                                                                               //
#define BLACK 2                                                                               //
#define GRAY 3                                                                                //
//--------------------------------------------------------------------------------------------//


// Spectrum Shield -----------------------------------------------------------//
// Pin connections                                                            //
#define SS_PIN_STROBE 4                                                       //
#define SS_PIN_RESET 5                                                        //
#define SS_PIN_DC_ONE A0                                                      //
#define SS_PIN_DC_TWO A1                                                      //
#define NUM_CHANNELS 7                                                        //
#define NUM_BANDS 3 // Bass, Mid, Treble                                      //
                                                                              //
// Outputs to Pi                                                              //
int frequencies_one[NUM_CHANNELS];                                            //
int frequencies_two[NUM_CHANNELS];                                            //
                                                                              //
// Inputs from Pi                                                             //
bool is_beat = false; // This can be inferred from downbeat_proximity         //
uint8_t downbeat_proximity = 0; // Up and down from 0-255 with the beat       //
uint8_t band_distribution[NUM_BANDS]; // bass=0, mid=1, treble=2; sums to 255 //
//----------------------------------------------------------------------------//

