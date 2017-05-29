#include <FastLED.h>

//////////**** Config area ****/////////////
// Debugging options
#define DEBUG
//#define DEBUG_SPECTRUM
#define DEBUG_TIMING

// SW Config
#define REFRESH_TIME 30
#define SHOW_TIME 30000

// Data pins - Chosen to work with FastLED Parallel output WS2811_PORTD
#define DATA_PIN_STRIP1 25
#define DATA_PIN_STRIP2 26
#define DATA_PIN_STRIP3 27
#define DATA_PIN_STRIP4 28

// Spectrum shield pins
#define SS_PIN_STROBE 4
#define SS_PIN_RESET 5
#define SS_PIN_DC1 A0
#define SS_PIN_DC2 A1 

//////////////////////////////////////////

// LED strip constants
#define NUM_LEDS_PER_STRIP 1260
#define NUM_STRIPS 4
#define NUM_LEDS NUM_LEDS_PER_STRIP * NUM_STRIPS


// Globals
CRGB leds[NUM_LEDS_PER_STRIP*4];
bool newShowTriggered;
word playlist, loopCount;
unsigned long current_time, show_saved_time, refresh_saved_time;

// Debugging globals
#ifdef DEBUG
  word serialVal[20];
#endif


// Spectrum globals
int Frequencies_One[7];
int Frequencies_Two[7];


// the setup function runs once when you press SS_PIN_RESET or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // Setup LED output ports.
  FastLED.addLeds<NEOPIXEL, DATA_PIN_STRIP1>(leds, 0, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, DATA_PIN_STRIP2>(leds, NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, DATA_PIN_STRIP3>(leds, 2*NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);
  FastLED.addLeds<NEOPIXEL, DATA_PIN_STRIP4>(leds, 3*NUM_LEDS_PER_STRIP, NUM_LEDS_PER_STRIP);

  // Set spectrum shield pins
  pinMode(SS_PIN_STROBE, OUTPUT);
  pinMode(SS_PIN_RESET, OUTPUT);
  pinMode(SS_PIN_DC1, INPUT);
  pinMode(SS_PIN_DC2, INPUT);  
  digitalWrite(SS_PIN_STROBE, HIGH);
  digitalWrite(SS_PIN_RESET, HIGH);

  // Initialize spectrum analyzers
  digitalWrite(SS_PIN_STROBE, LOW);
  delay(1);
  digitalWrite(SS_PIN_RESET, HIGH);
  delay(1);
  digitalWrite(SS_PIN_STROBE, HIGH);
  delay(1);
  digitalWrite(SS_PIN_STROBE, LOW);
  delay(1);
  digitalWrite(SS_PIN_RESET, LOW);

  // Setup serial port
  #ifdef DEBUG
    Serial.begin(9600);
  #endif
  
  // Clear all LEDs
  FastLED.clear(); 
}


// the loop function runs over and over again forever
void loop() {

  //  how many milliseconds have passed since DUE was started               
  current_time = millis();

  //  Reads spectrum analyzer - run as often as possible for accurate beat detection
  Read_Frequencies();

  //  Select animation
  UpdateCurShow();

  //  Draw animation
  DrawCurShow();
  FastLED.show();
  #ifdef DEBUG_TIMING
    serialVal[3] = millis() - current_time - serialVal[0] - serialVal[1] - serialVal[2];
  #endif
  
  // Serial debugging
  #ifdef DEBUG
    WriteToSerial();
  #endif

  loopCount++;
}

void UpdateCurShow() {
  //  If a show has played long enough randomly choose another show
  //  You can also force a show by uncommenting the last line of code 
  //  nested in this for loop.
  
  //  Use this global to: SS_PIN_RESET anything that has to do with a show.
  newShowTriggered = current_time - show_saved_time > SHOW_TIME;
  
  if (newShowTriggered)
  {
    show_saved_time = current_time;
    loopCount = 0;
    
    //  Change the second number as more shows are added.
    playlist = random8(1,4);  // playlist is a random number from 1 to (2nd number - 1)
    //  uncomment next line if you want to force a playlist
    //playlist = 3;
  }
  
  #ifdef DEBUG_TIMING
    serialVal[1] = millis() - current_time - serialVal[0];
  #endif
}

void DrawCurShow() {
  //  If pixel refresh time has expired update the LED pattern
  if (current_time - refresh_saved_time > REFRESH_TIME)
  {
    refresh_saved_time = current_time;
  
    // This is where you would add a new show.
    switch (playlist)  {   
      case 1:
        run_dot();
        break;
  
      case 2:
        run_dot_green();
        break;
    
      case 3:
        pulse();
        break; 
  
      default:
        run_dot();
    }
  }

  #ifdef DEBUG_TIMING
    serialVal[2] = millis() - current_time - serialVal[0] - serialVal[1];
  #endif
}

void WriteToSerial() {
  #ifdef DEBUG_SPECTRUM
    Serial.print("Spectrum:");
    int numSerialVals = 9;
  #else
    Serial.print("Timing:");
    int numSerialVals = 3;
  #endif
  
  for(int i = 0; i <= numSerialVals; i++) {
    Serial.print("\t");
    Serial.print(serialVal[i]);
  }  
  Serial.println();
}
