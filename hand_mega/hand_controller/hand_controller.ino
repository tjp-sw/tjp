// *to do*
// softer transition
// switch out LED library
// clean up parameter section
// resolve gain?
// communication, (10 samples per sec of each sensor)
// auto change color of the day
// Test under various light conditions
// general code clean up (remove globals and pass args)
// "hand off" 
//
// *bonus points*
// brightness
// make power up fancy

#include <Wire.h>
#include <FastLED.h>

//system constants
int sensorInputStatus = 0x03;
int handAddress = 0x28;
uint8_t CS1 = 0x10; // thumb
uint8_t CS2 = 0x11; // pointer
uint8_t CS3 = 0x12; // middle
uint8_t CS4 = 0x13; // (distance)
uint8_t CS5 = 0x14; // (guard)
uint8_t CS6 = 0x15; // ring
uint8_t CS7 = 0x16; // pinky
uint8_t CS8 = 0x17; // palm

const uint8_t thumbMask   = 0b00000001;
const uint8_t pointerMask = 0b00000010;
const uint8_t middleMask  = 0b00000100;
const uint8_t ringMask    = 0b00001000;
const uint8_t pinkyMask   = 0b00010000;
const uint8_t palmMask    = 0b00100000;

uint8_t sensativityControlRegister = 0x1F;

// LED constants
const int pin = 6;
const int numberOfPixels = 106;
CRGB leds[numberOfPixels];
CRGB red = CRGB::Red;
CRGB orange = CRGB::Orange;
CRGB yellow = CRGB::Yellow;
CRGB green = CRGB::Green;
CRGB blue = CRGB::Blue;
CRGB purple = CRGB::Purple;
CRGB indigo = CRGB::Indigo;
CRGB white = CRGB::White;
CRGB colorOfTheDay[7] = {red, orange, yellow, green, blue, purple, indigo};
CRGB currentColorOfTheDay = colorOfTheDay[0];
CRGB defaultTouchColor = currentColorOfTheDay;

// configuration statements /////////////////////////////////////////
// use the following parameters to change node number, sensativity or default color scheme
uint8_t sensativity = 1; // range is 1 to 128
int handBrightness = 50; // range is 1 to 255 (255 is brightest)
int touchThreshhold = 10;
CRGB defaultStartColor = white;
uint8_t sampleFrequency = 10; // times per sec to look for touches (Hz)
/////////////////////////////////////////////////////////////////////

// setup is automatically called first
void setup() {
  FastLED.addLeds<NEOPIXEL, pin>(leds, numberOfPixels);
  Wire.begin();
  Serial.begin(9600);
  setSensativity(sensativity);
  LEDS.setBrightness(handBrightness);
  startUpColorSequence();
}

// after setup main loop excutes automatically
void loop() {
  uint8_t touchMask = 0b00000000;
  // check what day it is
  //setColorPalette();
  
  // Record Touches
  touchMask = checkForTouch();
  sendTouchMessage(touchMask);
 
}

uint8_t checkForTouch(void) {
  if (wasThumbTouched() && wasPointerTouched() && wasMiddleTouched() && wasRingTouched() && wasPinkyTouched() && wasPalmTouched()) {
      colorSet(defaultTouchColor);    
  }
  
  uint8_t touchMask = 0b00000000;
  if (wasThumbTouched() || wasPointerTouched() || wasMiddleTouched() || wasRingTouched() || wasPinkyTouched() || wasPalmTouched()) {
    if (wasThumbTouched()) {
      drawThumb(defaultTouchColor);  
      touchMask |= thumbMask;
    }
    else {
      drawThumb(defaultStartColor);    
    }
    
    if (wasPointerTouched()) {
      drawPointer(defaultTouchColor);
      touchMask |= pointerMask;
    }
    else {
      drawPointer(defaultStartColor);    
    }
    
    if (wasMiddleTouched()) {
      drawMiddle(defaultTouchColor);
      touchMask |= middleMask;
    }
    else {
      drawMiddle(defaultStartColor);    
    }
    
    if (wasRingTouched()) {
      drawRing(defaultTouchColor);
      touchMask |= ringMask;
    }
    else {
      drawRing(defaultStartColor);    
    }  
    
    if (wasPinkyTouched()) {
      drawPinky(defaultTouchColor);
      touchMask |= pinkyMask;
    } 
    else {
      drawPinky(defaultStartColor);    
    } 
    
    if (wasPalmTouched()) {
      drawPalm(defaultTouchColor);
      touchMask |= palmMask;
    } 
    else {
      drawPalm(defaultStartColor);    
    }
  }
  else {
    proximityDetected();
    //colorSet(defaultStartColor); 
  }
  
  FastLED.show();
  return touchMask;
}
void setColorPalette(void) {
  // todo - set color of the day based on day of the week (0-6)
  currentColorOfTheDay = colorOfTheDay[0];
  defaultTouchColor = currentColorOfTheDay;
}

void sendTouchMessage(uint8_t touchMask) {
  Serial.print("Send touch message -> ");
  Serial.println(touchMask);
  // TODO - put touch received message to pi code here and call from main loop
  // TODO - be sure to incorporate messageFrequency
}

void startUpColorSequence(){
  turnOnHandOutline();
  colorSet(currentColorOfTheDay);
  FastLED.show();
  
  drawThumb(colorOfTheDay[0]);
  drawPointer(colorOfTheDay[1]);  
  drawMiddle(colorOfTheDay[2]); 
  drawRing(colorOfTheDay[3]);
  drawPinky(colorOfTheDay[4]);
  drawPalm(colorOfTheDay[5]);
  FastLED.show();
  delay(2000);
  colorSet(defaultStartColor); 
}

uint8_t proximityDetected(void) {
  uint8_t proximitySensorReading = getRegister(CS4);
  Serial.println(proximitySensorReading);
  if (proximitySensorReading <= 127 && proximitySensorReading >= 32) {
    int bucketSize = 16;
    int red = (((proximitySensorReading * 2)/bucketSize)*bucketSize);
    int green = red;
    int blue = red;
    CRGB newColor = CRGB(red, green, blue);
    colorSet(newColor);
  } 
}

boolean wasThumbTouched(void){
  int pointerSensorReading = getRegister(CS1);
  //Serial.println(pointerSensorReading);
  
  if (wasItTouched(pointerSensorReading)) {
    touched(CS1);
    return true;
  }
  else 
  {
    return false;
  }
}

boolean wasPointerTouched(void){
  int pointerSensorReading = getRegister(CS2);
  //Serial.println(pointerSensorReading);
  
  if (wasItTouched(pointerSensorReading)) {
    touched(CS2);
    return true;
  }
  else 
  {
    return false;
  }
}

boolean wasMiddleTouched(void){
  int pointerSensorReading = getRegister(CS3);
  //Serial.println(pointerSensorReading);
  
  if (wasItTouched(pointerSensorReading)) {
    touched(CS3);
    return true;
  }
  else 
  {
    return false;
  }
}

boolean wasRingTouched(void){
  int pointerSensorReading = getRegister(CS6);
  //Serial.println(pointerSensorReading);
  
  if (wasItTouched(pointerSensorReading)) {
    touched(CS6);
    return true;
  }
  else 
  {
    return false;
  }
}

boolean wasPinkyTouched(void){
  int pointerSensorReading = getRegister(CS7);
  //Serial.println(pointerSensorReading);
  
  if (wasItTouched(pointerSensorReading)) {
    touched(CS7);
    return true;
  }
  else 
  {
    return false;
  }
}

boolean wasPalmTouched(void){
  int pointerSensorReading = getRegister(CS8);
  //Serial.println(pointerSensorReading);
  
  if (wasItTouched(pointerSensorReading)) {
    touched(CS8);
    return true;
  }
  else 
  {
    return false;
  }
}

boolean wasItTouched(int sensorReading){
  return (sensorReading > touchThreshhold && sensorReading < 200); // have to screen out some high values
}

void touched(int sensor){
  //Serial.println("touched:");
}

void setSensativity(int newSensativity) {
  Serial.println("Initial Sensativity Setting (47 is default)");
  printRegister(sensativityControlRegister);
  
  setRegister(sensativityControlRegister, newSensativity);

  Serial.println("New Sensativity Setting (47 is default)");
  printRegister(sensativityControlRegister);   
}

void setRegister(int registerToSet, int registerValue) {
  Wire.beginTransmission(handAddress);
  Wire.write(registerToSet);
  Wire.write(registerValue);
  Wire.endTransmission(); 

  Wire.beginTransmission(handAddress); // Begin transmission to the Sensor 
  Wire.write(sensativityControlRegister); //Ask the particular registers for data
  Wire.endTransmission(); 
}

int getRegister(int registerToGet) {
  int reading = 0;
  Wire.beginTransmission(handAddress); // Begin transmission to the Sensor 
  Wire.write(registerToGet); //Ask the particular registers for data
  Wire.endTransmission();
  
  Wire.requestFrom(handAddress, 1);
  if (Wire.available() >= 1) {
    reading = Wire.read();// & 0x3F;
  }
  else {
    Serial.println("XXXXXXXXXXXXXXX");
  }
  return reading;
}

void printRegister(int registerToPrint) {
  Wire.beginTransmission(handAddress); // Begin transmission to the Sensor 
  Wire.write(registerToPrint); //Ask the particular registers for data
  Wire.endTransmission();
  
  Wire.requestFrom(handAddress, 1);
  if (Wire.available() >= 1) {
    int reading = Wire.read();

    Serial.println(reading);
    Serial.println("---------");
  }
  else {
    Serial.println("XXXXXXXXXXXXXXX");
  }  
}

void turnOnHandOutline(void) {
  colorWipe(currentColorOfTheDay, 10); // white 
}

void drawPalm(CRGB c){
  int start = 0;
  int end = 21;
  drawArea(c, start, end);
}

void drawPinky(CRGB c){
  int start = 22;
  int end = 33;
  drawArea(c, start, end);
}

void drawRing(CRGB c){
  int start = 35;
  int end = 53;
  drawArea(c, start, end);
}

void drawMiddle(CRGB c){
  int start = 51;
  int end = 71;
  drawArea(c, start, end);
}

void drawPointer(CRGB c){
  int start = 71;
  int end = 87;
  drawArea(c, start, end);
}

void drawThumb(CRGB c){
  int start = 93;
  int end = 103;
  drawArea(c, start, end);
}

void drawArea(CRGB c, int start, int end){
  for(uint16_t i = start; i <= end; i++) {
    leds[i] = c;
  } 
}

void colorWipe(CRGB c, uint8_t wait) {
  for(uint16_t i=0; i < numberOfPixels; i++) {
    leds[i] = c;
    FastLED.show();
    delay(wait);
  }
}

void colorSet(CRGB c) {
  for(uint16_t i=0; i < numberOfPixels; i++) {
    leds[i] = c;
  }
}


