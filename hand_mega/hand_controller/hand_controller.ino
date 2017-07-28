// *to do*
// softer transition
// switch out LED library
// clean up parameter section
// resolve gain?
// communication, (10 samples per sec of each sensor)
// stub out color of the day palette 
// auto change color of the day
// Test under various light conditions
// general code clean up (remove globals and pass args)
// "hand off" 
//
// *bonus points*
// brightness
// make power up fancy

#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

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
uint8_t sensativityControlRegister = 0x1F;

// LED constants
int pin = 6;
int numberOfPixels = 106;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(numberOfPixels, pin, NEO_GRB + NEO_KHZ800);
uint32_t red = strip.Color(255, 0, 0);
uint32_t green = strip.Color(0, 255, 0);
uint32_t blue = strip.Color(0, 0, 255);
uint32_t white = strip.Color(255, 255, 255);

// configuration statements
// use the following parameters to change node number, sensativity or default color scheme
uint8_t sensativity = 8; // range is 1 to 128
int handBrightness = 50; // range is 1 to 255 (255 is brightest)
int touchThreshhold = 10;
uint32_t defaultStartColor = white;
uint32_t defaultColorOfTheDay = blue;
uint32_t defaultTouchColor = green;

// setup is automatically called first
void setup() {
  Wire.begin();
  Serial.begin(9600);
  setSensativity(sensativity);
  startUpColorSequence();
}

// after setup main loop excutes automatically
void loop() {
  if (wasThumbTouched() && wasPointerTouched() && wasMiddleTouched() && wasRingTouched() && wasPinkyTouched() && wasPalmTouched()) {
      colorSet(defaultTouchColor);    
  }
  
  if (wasThumbTouched() || wasPointerTouched() || wasMiddleTouched() || wasRingTouched() || wasPinkyTouched() || wasPalmTouched()) {
    if (wasThumbTouched()) {
      drawThumb(defaultTouchColor);    
    }
    else {
      drawThumb(defaultStartColor);    
    }
    
    if (wasPointerTouched()) {
      drawPointer(defaultTouchColor);
    }
    else {
      drawPointer(defaultStartColor);    
    }
    
    if (wasMiddleTouched()) {
      drawMiddle(defaultTouchColor);
    }
    else {
      drawMiddle(defaultStartColor);    
    }
    
    if (wasRingTouched()) {
      drawRing(defaultTouchColor);
    }
    else {
      drawRing(defaultStartColor);    
    }  
    
    if (wasPinkyTouched()) {
      drawPinky(defaultTouchColor);
    } 
    else {
      drawPinky(defaultStartColor);    
    } 
    
    if (wasPalmTouched()) {
      drawPalm(defaultTouchColor);
    } 
    else {
      drawPalm(defaultStartColor);    
    }
   
    sendTouchMessage();
   
  }
  else {
    proximityDetected();
    //colorSet(defaultStartColor); 
  }
  strip.show();
  delay(10);
}

void sendTouchMessage() {
  Serial.println("Send touch received message");
  // TODO - put touch received message to pi code here
}

void startUpColorSequence(){
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  strip.setBrightness(handBrightness);
  turnOnHandOutline();
  colorSet(defaultColorOfTheDay);
  strip.show();
  
  drawPalm(red);
  drawPinky(green);
  drawRing(red);
  drawMiddle(green);
  drawPointer(red);
  drawThumb(green);
  strip.show();
  delay(1000);
  colorSet(defaultStartColor); 
}

uint8_t proximityDetected(void) {
  uint8_t proximitySensorReading = getRegister(CS4);
  Serial.println(proximitySensorReading);
  if (proximitySensorReading <= 127 && proximitySensorReading >= 2) {
    int bucketSize = 32;
    int red = (((proximitySensorReading * 2)/bucketSize)*bucketSize);
    int green = red;
    int blue = red;
    uint32_t newColor = strip.Color(red, green, blue);
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
  Serial.println("touched");
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
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  colorWipe(white, 10); // white 
}

void drawPalm(uint32_t c){
  int start = 0;
  int end = 21;
  drawArea(c, start, end);
}

void drawPinky(uint32_t c){
  int start = 22;
  int end = 33;
  drawArea(c, start, end);
}

void drawRing(uint32_t c){
  int start = 35;
  int end = 53;
  drawArea(c, start, end);
}

void drawMiddle(uint32_t c){
  int start = 51;
  int end = 71;
  drawArea(c, start, end);
}

void drawPointer(uint32_t c){
  int start = 71;
  int end = 87;
  drawArea(c, start, end);
}

void drawThumb(uint32_t c){
  int start = 93;
  int end = 103;
  drawArea(c, start, end);
}

void drawArea(uint32_t c, int start, int end){
  for(uint16_t i = start; i <= end; i++) {
    strip.setPixelColor(i, c);
    //strip.show();
  } 
}

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void colorSet(uint32_t c) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  //strip.show();
}


