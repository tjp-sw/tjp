#include <FastLED.h>
typedef void (*sparkle_f_ptr)();     


// Config --------------------------------------------------//
// Debugging options; comment/uncomment to test diff things //
//#define DEBUG                                               //
//#define DEBUG_TIMING                                      //
//#define DEBUG_FREQUENCIES                                 //
//#define DEBUG_PEAKS                                       //
//#define DEBUG_BPM                                         //
//#define DEBUG_AUDIO_HOOKS                                 //
                                                            //
// Timing settings                                          //
//#define REFRESH_TIME 55 // 18.2 frames per second         //
#define REFRESH_TIME 60 // 16.67 frames per second          //
#define ANIMATION_TIME 60000 // 30 seconds per animation    //
                                                            //
// Due controlled versus pi controlled animation choices    //
//#define PI_CONTROLLED                                       //
#define CYCLE                                             //
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
uint8_t bpm_estimate = 0;                                                     //
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
#define NUM_NODES 1                                                           //
#define RINGS_PER_NODE 12                                                     //
#define STRIPS_PER_NODE 4                                                     //
#define PHYSICAL_LEDS_PER_RING 420                                            //
#define LEDS_PER_RING 408                                                     //
                                                                              //
#define LEDS_PER_STRIP (2*PHYSICAL_LEDS_PER_RING + LEDS_PER_RING + 1)         //
#define NUM_RINGS (RINGS_PER_NODE * NUM_NODES)                                //
#define LEDS_PER_NODE (LEDS_PER_RING * RINGS_PER_NODE)                        //
#define PHYSICAL_LEDS_PER_NODE (LEDS_PER_STRIP*STRIPS_PER_NODE)               //
#define HALF_RING (LEDS_PER_RING/2)                                           //
#define NUM_LEDS (LEDS_PER_NODE * NUM_NODES)                                  //
#define RINGS_PER_STRIP (RINGS_PER_NODE / STRIPS_PER_NODE)                    //
                                                                              //
// Globals                                                                    //
bool new_animation_triggered = false;                                         //
uint8_t current_animation = 0;                                                //
uint32_t loop_count = 0;                                                      //
unsigned long current_time=0, animation_start_time=0;                         //
unsigned long long epoch_msec;                                                //
byte heat[RINGS_PER_NODE][LEDS_PER_RING];

//----------------------------------------------------------------------------//

//  ----------------------------------------------LED arrays ---------------------------------------------------------------------
CRGB leds_raw[LEDS_PER_STRIP*STRIPS_PER_NODE*NUM_NODES]; // 1 - 408 - 12 - 408 - 12 - 408 - 1 - 408 - 12 - 408 - 12 - 408 ...
CRGBSet leds_all(leds_raw, LEDS_PER_STRIP*STRIPS_PER_NODE*NUM_NODES);

//  Show parameters coming from the pi -----------------------------------------------------//
#define NUM_PARAMETERS 10                                                                   //
#define NUM_COLORS_PER_PALETTE 7                                                            //
#define NUM_ANIMATIONS 9                                                                   //
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
#define INTRA_RING_SPEED_INDEX 7   // 0 to 4                                                //
#define COLOR_CHANGE_STYLE_INDEX 8   // 0 none, 1 cycle thru selected, 2 cycle thru palette //
#define RING_OFFSET_INDEX 9  // how far one ring pattern is rotated from neighbor -10 -> 10 //
                                                                                            //
//  Evolving parameters defining the show                                                   //
uint8_t show_parameters[NUM_PARAMETERS];                                                    //
                                                                                            //
// array of show_parameters[NUM_COLORS_INDEX] colors chosen out of given palette            //
uint8_t show_colors[7];                                                //   
//------------------------------------------------------------------------------------------//


// Color palette choices ------------------------------------------------------//
// Eventually this may be stored in the database if space issues arise
CRGB icy_bright[9] = 
    {CRGB(255,255,255), CRGB(254,207,241),                // light
    CRGB(255,108,189), CRGB(0,172,238), CRGB(44,133,215),    //  medium
    CRGB(114,78,184), CRGB(227,0,141)};                   // dark

CRGB watermelon[9] = 
    {CRGB(47,192,91), CRGB(70,190,91),                     // light
    CRGB(47,140,9), CRGB(72,160,5), CRGB(14,139,50),    //  medium
    CRGB(120,86,103), CRGB(148,33,137)};                 // dark

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
CRGB sparkle[RINGS_PER_NODE][LEDS_PER_RING];  // Sparkle LED layer as a 2D array. Currently only enough RAM for # per node. Could change this from CRGB to a byte that indexes into palette.
boolean sparkle_is_set[RINGS_PER_NODE][LEDS_PER_RING];
boolean increasing[RINGS_PER_NODE][LEDS_PER_RING];

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
    Serial.begin(115200);                                                                                   //
    Serial.println("Serial port initialized.");                                                             //
    Serial.flush();                                                                                         //
  #endif                                                                                                    //
                                                                                                            //
  setup_spectrum_shield();                                                                                  //
                                                                                                            //
  show_parameters[ANIMATION_INDEX] = 6; // Initialize to debugging mode                                     //
                                                                                                            //
  // Initialize digital pin LED_BUILTIN as an output                                                        //
  pinMode(LED_BUILTIN, OUTPUT);                                                                             //
                                                                                                            //
  // Initialize LEDS controller to node 0's data. Will be moved when node is assigned from brain            //
  LEDS.addLeds<WS2811_PORTD, 8>(leds_raw, LEDS_PER_STRIP).setCorrection(TypicalLEDStrip);                   //
                                                                                                            //
  //  Clear all LEDs                                                                                        //
  LEDS.clear();                                                                                             //
                                                                                                            //
  #ifdef PI_CONTROLLED                                                                                      //
    setup_communication();                                                                                  //
    animation_start_time = millis() + epoch_msec;
  #else                                                                                                     //
   assign_node(node_number);                                                                                //
  #endif                                                                                                    //

  int temp = random8(0, 10);
}                                                                                                           //
//----------------------------------------------------------------------------------------------------------//


// the loop function runs over and over again forever ------------------------------------//
void loop() {                                                             
  current_time = epoch_msec + millis(); // epoch_msec == 0 ? millis() : epoch_msec;
  
  loop_count = (current_time - animation_start_time) / REFRESH_TIME;   
       
  #ifdef DEBUG_TIMING                                                     
    serial_val[0] = current_time - last_debug_time;                       
    last_debug_time = current_time;                                       
  #endif                                                                  
                                                                          
  // read spectrum shield and do beat detection                           
  loop_spectrum_shield();                                                 
  #ifdef DEBUG_TIMING                                                     
    unsigned long now = millis();                                         
    serial_val[1] = now - last_debug_time;                                
    last_debug_time = now;                                                
  #endif                                                                  
                                                                          
  //  Communicate with pi if available, select animation, other parameters
  #ifdef PI_CONTROLLED()
    do_communication();
    if(epoch_msec > 0)
      current_time = epoch_msec;
    current_animation = show_parameters[ANIMATION_INDEX];
  #else
    manually_update_parameters();  
  #endif
  
  #ifdef DEBUG_TIMING                                                     
    now = millis();                                                       
    serial_val[2] = now - last_debug_time;                                
    last_debug_time = now;                                                
  #endif                                                                  
                                                                          
  //  Draw animation                                                      
  draw_current_animation();                                               
  #ifdef DEBUG_TIMING                                                     
    now = millis();                                                       
    serial_val[3] = now - last_debug_time;                                
    last_debug_time = now;                                                
  #endif                                                                  

  #ifdef DEBUG
    if(node_number < NUM_NODES) {
      for(int i = 0; i < STRIPS_PER_NODE; i++) {
        //Serial.println("Setting debug LED " + String(i) + " to palette " + String(show_parameters[PALETTE_INDEX]) + ", color " + String(i));
        leds_raw[node_number*PHYSICAL_LEDS_PER_NODE + i*LEDS_PER_STRIP] = get_color(show_parameters[PALETTE_INDEX], i);//show_colors[i % 3]);
      }
    }
  #endif

  // Write LEDs                                                           
  LEDS.show();                                                            
  #ifdef DEBUG_TIMING                                                     
    now = millis();                                                       
    serial_val[4] = now - last_debug_time;                                
    last_debug_time = now;                                                
  #endif                                                                  
                                                                          
                                                                          
  // Serial output for debugging                                          
  #ifdef DEBUG                                                            
    write_to_serial();                                                    
  #endif                                                                  
                                                                          
  // Enforce max refresh rate                                             
  unsigned long temp = millis();                                          
  if(temp - current_time < REFRESH_TIME - 1) // Leave a 1ms buffer        
    delay(REFRESH_TIME + current_time - temp - 1);                        
}                                                                         


// Updates show_parameters[] and show_colors[] coming from the pi, or manually
void manually_update_parameters() {

    #ifdef CYCLE
      cycle_through_animations();
      show_parameters[BEAT_EFFECT_INDEX] = 0;                                                                  //
      show_parameters[NUM_COLORS_INDEX] = 7;                                                                   //
      show_parameters[COLOR_THICKNESS_INDEX] = 3;                                                              //
      show_parameters[BLACK_THICKNESS_INDEX] = 8;                                                              //
      show_parameters[INTRA_RING_MOTION_INDEX] = 1;                                                            //
      show_parameters[INTRA_RING_SPEED_INDEX ] = 2;                                                            //
      show_parameters[COLOR_CHANGE_STYLE_INDEX] = 0;                                                           //
      show_parameters[RING_OFFSET_INDEX] = 10;                                                                 //      
    #else
      // choose single animation manually         ********** set parameters manually here **********
      show_parameters[ANIMATION_INDEX] = 6;    // <--- this is where you enter your animation number         //
    
      // Manually assign values to the rest of the animation parameters
      
                                                                                                               //                                                                           //
      // can choose 0 to 6 as indices into current palette                                                     //
      // 0,1 light, 2,3,4 mid, 5,6 dark                                                                        //
      setShowColors();
    #endif 
                                                                                                           //  
    // fixme: doesn't yet have its own parameter, so just working with ones we have                          //
    // need colors 0 or 1 from any palette for sparkle color                                                 // 
    sparkle_color = get_color(random8(0, 4), (INTRA_RING_MOTION_INDEX + 1) % 6);                          
}

void setShowColors()
{
  for(uint8_t i = 0; i < 7; i++)
  {
    show_colors[0] = 0;//random8(0, 7);                                                                                      //                                                                                                                   //
    show_colors[1] = (show_colors[0] + 1) % 7;                                                                                      //
    show_colors[2] = (show_colors[0] + 2) % 7;                                                                                      //
    show_colors[3] = (show_colors[0] + 3) % 7;                                                                                      //
    show_colors[4] = (show_colors[0] + 4) % 7;                                                                                      //
    show_colors[5] = (show_colors[0] + 5) % 7;                                                                                      //
    show_colors[6] = (show_colors[0] + 6) % 7;                                                                                      //
  }
  
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

    
    current_animation = random8(0, NUM_ANIMATIONS);  // current_animation is a random number from 0 to (NUM_ANIMATIONS - 1) 
    //current_animation = ++current_animation % NUM_ANIMATIONS;
    show_parameters[PALETTE_INDEX] = random8(0, 3);
    setShowColors();

    #ifdef DEBUG
      Serial.print("New show started: ");
      Serial.println(current_animation) ;
    #endif
  }
}

bool last_animation_fire = false;
// Draws the current animation
void draw_current_animation() {
  // This is where you would add a new animation.
  Serial.println(current_animation);
  switch (current_animation)  {   
    case 0:
      last_animation_fire = false;
      draw_debug_mode(); // debugging; This should stay as the default animation for setup purposes
      break;

    case 1:
      last_animation_fire = false;
      toms_best_old(); // erick
      break;

    case 2:
    case 3:
      if(!last_animation_fire) {
        for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++)
          for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++)
            heat[ring][pixel] = 0;
      }
      last_animation_fire = true;
      Fire(); // brian
      break;
      
    case 4:
    last_animation_fire = false;
      snake();  // diane
      break; 

    case 5:  
    last_animation_fire = false;
      sparkle_count = 0;
      sparkle_rain();      // diane
      break;

    case 6:
      last_animation_fire = false;
      sparkle_count = 0;
      leds_all = CRGB::Black;
      sparkle_glitter();  // diane
      break;
      
    case 7:
      if(!last_animation_fire) {
        for(uint8_t ring = 0; ring < RINGS_PER_NODE; ring++)
          for(uint16_t pixel = 0; pixel < LEDS_PER_RING; pixel++)
            heat[ring][pixel] = 0;
      }
      last_animation_fire = true;
      sparkle_count = 0;
      Fire();
      sparkle_glitter();
      break;
  
    case 8:
    case 9:
    last_animation_fire = false;
      // has the 7 color bands, but doesn't scale with intensity from spectrum shield yet bc
      // alex couldn't figure out how yet
      equalizer();  // alex
      break; 


    default:
    last_animation_fire = false;
      draw_debug_mode(); // debugging; This should stay as the default animation for setup purposes
  }
}

