#include <FastLED.h>
typedef void (*sparkle_f_ptr)();

//Declare Spectrum Shield pin connections -------------------//
// Works don't touch                                         //
#define STROBE 4                                             //
#define RESET 5                                              //
#define DC_One A0                                            //
#define DC_Two A1                                            //
#define down 0                                               //
#define up 1                                                 //
//-----------------------------------------------------------//


// -------------------------- put your animation constants / macros here --------------------------------------------------
//  Number of LEDs per strip.  Their are 4 strips per node
//  #define NUM_LEDS_PER_STRIP 1680

#define NUM_LEDS_PER_STRIP 250
#define NUM_STRIPS_PER_NODE 4
#define NUM_RINGS_PER_STRIP 1
#define NUM_LEDS_PER_RING 250
#define HALF_RING 125
#define MAX_SPARKLE_INTENSITY 250 // fixme: these should be color dependent to avoid color drift at high intensity
#define MIN_SPARKLE_INTENSITY 50
#define NUM_SPARKLE_FNS 10


//This sets up the LED array and output ports -----------//
// Works don't touch                                     //
#define DATA_PIN_STRIP1 10                               //
#define DATA_PIN_STRIP2 11                               //
#define DATA_PIN_STRIP3 12                               //
#define DATA_PIN_STRIP4 13                               //
CRGB leds_1[NUM_LEDS_PER_STRIP];                         //
CRGB leds_2[NUM_LEDS_PER_STRIP];                         //
CRGB leds_3[NUM_LEDS_PER_STRIP];                         //
CRGB leds_4[NUM_LEDS_PER_STRIP];                         //
//-------------------------------------------------------//


// --------------- Nodes LED array -------------------------------------
//  This is the Nodes LED array.  Put what you want the LEDs to do
//  into this array and this array only.  The write_led routine will
//  bit bang it and send it to the LEDs.
CRGB leds_node[NUM_LEDS_PER_STRIP * 4];

// Diane: Trying the same with a 2D array. We'll eventually have 4 long strips, and 3 rings worth of LEDs per strip.
CRGB leds[NUM_STRIPS_PER_NODE][NUM_LEDS_PER_STRIP];
CRGB sparkle[NUM_STRIPS_PER_NODE][NUM_LEDS_PER_STRIP];


//---------------------------------------- Declare your variables here ------------------------------------------------------------

// Tom's
word i;
word cntr_test;
word temp[20];
word playlist;
unsigned long current_time, show_saved_time, show_time, refresh_saved_time, refresh_time, test_time, temp_saved_time, temp_current_time;


// Diane's additions
int sparkle_count = 0;
boolean is_set[NUM_STRIPS_PER_NODE][NUM_LEDS_PER_STRIP];
boolean increasing[NUM_STRIPS_PER_NODE][NUM_LEDS_PER_STRIP];

void sparkle_twinkle_init();
void sparkle_twinkle();

// array of sparkle setup functions
   sparkle_f_ptr sparkle_init[] = { sparkle_twinkle_init };
//      void (*sparkle_init_fn[NUM_SPARKLE_FNS])();
//      sparkle_init_fn[0] = sparkle_twinkle_init;
//      sparkle_init_fn[1] = sparkle_rain_init;
//    sparkle_init_fn[2] = sparkle_glitter_init;
//    sparkle_init_fn[3] = sparkle_wind_init;
//    sparkle_init_fn[4] = sparkle_wiggle_wind_init;
//    sparkle_init_fn[5] = sparkle_warp_speed_init;
//    sparkle_init_fn[6] = comet_init;
//    sparkle_init_fn[7] = sparkle_3_circle_init;
//    sparkle_init_fn[8] = sparkle_torus_knot_init;

// array of sparkle functions to make it easier to choose one randomly
   sparkle_f_ptr sparkle_fn[] = { sparkle_twinkle };
//   sparkle_fn[0] = sparkle_twinkle;
//    sparkle_fn[1] = sparkle_rain;
//    sparkle_fn[2] = sparkle_glitter;
//    sparkle_fn[3] = sparkle_wind;
//    sparkle_fn[4] = sparkle_wiggle_wind;
//    sparkle_fn[5] = sparkle_warp_speed;
//    sparkle_fn[6] = comet;
//    sparkle_fn[7] = sparkle_3_circle;
//    sparkle_fn[8] = sparkle_torus_knot();


//Define spectrum variables ------ don't touch anything in this box ------------//
//  Works don't touch                                                           //
int freq_amp;                                                                   //
int Frequencies_One[7];                                                         //
int Frequencies_Two[7];                                                         //
int Spectrum[7];                                                                //
int beat0;                                                                      //
int beat1;                                                                      //
                                                                                //
// the setup function runs once when you press reset or power the board         //
void setup() {                                                                  //
  // initialize digital pin LED_BUILTIN as an output.                           //
  pinMode(LED_BUILTIN, OUTPUT);                                                 //
                                                                                //
  // Setup LED output ports.                                                    //
  // Works, Don't touch                                                         //
    FastLED.addLeds<NEOPIXEL, DATA_PIN_STRIP1>(leds_1, NUM_LEDS_PER_STRIP);     //
    FastLED.addLeds<NEOPIXEL, DATA_PIN_STRIP2>(leds_2, NUM_LEDS_PER_STRIP);     //
    FastLED.addLeds<NEOPIXEL, DATA_PIN_STRIP3>(leds_3, NUM_LEDS_PER_STRIP);     //
    FastLED.addLeds<NEOPIXEL, DATA_PIN_STRIP4>(leds_4, NUM_LEDS_PER_STRIP);     //

// Diane's attempt to break the rules of not touching and use an array rather than 4 named lists
//  FastLED.addLeds<NEOPIXEL, DATA_PIN_STRIP1>(leds[1], NUM_LEDS_PER_STRIP);         //
//  FastLED.addLeds<NEOPIXEL, DATA_PIN_STRIP2>(leds[2], NUM_LEDS_PER_STRIP);         //
//  FastLED.addLeds<NEOPIXEL, DATA_PIN_STRIP3>(leds[3], NUM_LEDS_PER_STRIP);         //
//  FastLED.addLeds<NEOPIXEL, DATA_PIN_STRIP4>(leds[4], NUM_LEDS_PER_STRIP);         //
                                                                                //
                                                                                //
  //Set spectrum Shield pin configurations                                      //
  // Works, Don't touch                                                         //
  pinMode(STROBE, OUTPUT);                                                      //
  pinMode(RESET, OUTPUT);                                                       //
  pinMode(DC_One, INPUT);                                                       //
  pinMode(DC_Two, INPUT);                                                       //
  digitalWrite(STROBE, HIGH);                                                   //
  digitalWrite(RESET, HIGH);                                                    //
                                                                                //
  //Initialize Spectrum Analyzers                                               //
  // Works, Don't touch                                                         //
  digitalWrite(STROBE, LOW);                                                    //
  delay(1);                                                                     //
  digitalWrite(RESET, HIGH);                                                    //
  delay(1);                                                                     //
  digitalWrite(STROBE, HIGH);                                                   //
  delay(1);                                                                     //
  digitalWrite(STROBE, LOW);                                                    //
  delay(1);                                                                     //
  digitalWrite(RESET, LOW);                                                     //
                                                                                //
  // Setup Serial port                                                          //
  // Works, Don't touch                                                         //
  Serial.begin(9600);                                                           //
                                                                                //
  //  Time between LED refreshes in miliseconds                                 //
  refresh_time = 30;                                                            //
                                                                                //
  //  Time between show refreshes in miliseconds                                //
  show_time = 30000;                                                            //
  //  Clear all LEDs                                                            //
  FastLED.clear();                                                              //
}                                                                               //
//------------------------------------------------------------------------------//






// the loop function runs over and over again forever
void loop() {

  //  how many milliseconds have passed since DUE was started
  current_time = millis();

  //  If a show has played long enough randomly choose another show
  //  You can also force a show by uncommenting the last line of code nested in this for loop.
  if ((current_time - show_saved_time) > show_time) {
    show_saved_time = current_time;

    //  Reset anything that has to do with a show.
    cntr_test = 0;

    //  Change the second number as more shows are added.
    // playlist = random(1, 4); // playlist is a random number from 1 to (2nd number - 1)
    //  uncomment next line if you want to force a playlist
    playlist = 3
    ;
  } // if show played long enough


  //  If pixel refresh time has expired update the LED pattern
  if ((current_time - refresh_saved_time) > refresh_time) {
    temp[1] = current_time - refresh_saved_time;
    refresh_saved_time = current_time;

    //  Reads spectrum analyzer
    // Works, Don't touch
    Read_Frequencies();

    //  This is where you would add a new show.
    switch (playlist)  {

      case 1:
        run_dot();
        break;

      case 2:
        run_dot_green();
        // sparkle_twinkle_setup();
        break;

      case 3:
        pulse();
        break;

      case 4:
//        sample_background();
        break;

      default:
        run_dot();
    }  // end switch

    write_leds();

    // vars to be sent to the serial port for trouble shooting, comment this out if you don't need it.
    temp[1] = Frequencies_One[0];
    temp[2] = Frequencies_One[1];
    temp[3] = Frequencies_One[2];
    temp[4] = Frequencies_One[3];
    temp[5] = Frequencies_One[4];
    print_whatever_serial();

  } // end if pixel refresh time too long

}



/***********   Print whatever to serial port  **********************/
// Works, Don't touch
void print_whatever_serial()  {
  Serial.print("\t");
  Serial.print(temp[1]);
  Serial.print("\t");
  Serial.print(temp[2]);
  Serial.print("\t");
  Serial.print(temp[3]);
  Serial.print("\t");
  Serial.print(temp[4]);
  Serial.print("\t");
  Serial.println(temp[5]);
}
