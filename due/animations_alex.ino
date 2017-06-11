// Alex, please sort these into background vs mid-layer vs sparkle
// I'll assume whatever we put into mid-layer for now will be good for EDM music


// Alex testing animations

void simple_gradient(){
  //gradient_eight(Red, CRGB::Green,CRGB:: Blue,CRGB::White , CRGB::Purple, CRGB:: Yellow, CRGB::Pink, CRGB:: Black);
  //box_shuffle();
  //basic_gradient();
}
// built in function to call rainbow
void rainbow(){
  fill_rainbow(leds_all,LEDS_PER_NODE,15,20);
  sparklet(95);
  
}
// add popping sparkle effects to the rainbow or current design
void sparklet(int prob){
  
  if(random8() >= prob){
    leds_all[random16(LEDS_PER_NODE)] += CRGB::White;
  } 

}

// function that fills an aready with a certain gradient.
// void basic_gradient(int start, int end, CRGB start_color, CRGB end_color)
// pass in pallete
void basic_gradient(){
  int roll = loop_count%10;
  if(roll <= 5){
    fill_gradient_RGB(leds_all,0,CRGB::Red,LEDS_PER_NODE,CRGB::Green);
    sparklet(80);
  }
  else{
    fill_gradient_RGB(leds_all,0,CRGB::Green,LEDS_PER_NODE,CRGB::Red);
    sparklet(80);
  }
    // fill_gradient_RGB(leds_all,start,start_color(from pallete),end,end_color(from pallete
}

//void gradient_eight(CRGB color1, CRGB color2,CRGB color3,CRGB color 4, CRGB color5, CRGB color6, CRGB color 7, CRGB color8){
void gradient_eight(){
  fill_gradient_RGB(leds_all,0,CRGB:: Red,LEDS_PER_STRIP-1,CRGB:: Blue);
  blackout(50);
  fill_gradient_RGB(leds_all,LEDS_PER_STRIP,CRGB::Green,(LEDS_PER_STRIP*2)-1,CRGB:: Pink);
  blackout(50);
  fill_gradient_RGB(leds_all,LEDS_PER_STRIP*2,CRGB::White,(LEDS_PER_STRIP*3)-1,CRGB:: Black);
  blackout(50);
  fill_gradient_RGB(leds_all,LEDS_PER_STRIP*3,CRGB::Brown,(LEDS_PER_STRIP*4)-1,CRGB:: Purple);
  blackout(50);
}

// not working fully but mean to turn off colors until all is black and then an animation comes.
void blackout(int prob){
  delay(1);
  if(random8() >= prob){
    int my_rand = random16(LEDS_PER_NODE);
    if(my_rand %6 == 0){
      fill_solid(leds_all,LEDS_PER_NODE,CRGB:: Yellow);
      delay(10000);
    }
    leds_all[my_rand] = CRGB::Black;
    leds_all[my_rand+1] = CRGB::Black;
    leds_all[my_rand+2] = CRGB::Black;
    leds_all[my_rand+3] = CRGB::Black;
    leds_all[my_rand+4] = CRGB::Black;
    leds_all[my_rand+5] = CRGB::Black;
  } 


}


void box_shuffle(){
    uint16_t pixelOffset = loop_count % (LEDS_PER_STRIP);
  // One dot for whole structure, instead of one dot per node
  leds_all = CRGB::Black;
  leds_all[pixelOffset] = CRGB::Pink;
  leds_all[pixelOffset+1] = CRGB::Pink;
  leds_all[pixelOffset+2] = CRGB::Pink;
  leds_all[pixelOffset+3] = CRGB::Red;
  //BOTTOM PART OF BOX 1

  // TOP PART OF BOX
  leds_all[pixelOffset+(420*3)] = CRGB::Pink;
  leds_all[pixelOffset+1+(420*3)] = CRGB::Pink;
  leds_all[pixelOffset+2+(420*3)] = CRGB::Pink;
  leds_all[pixelOffset+3+(420*3)] = CRGB::Pink;
  
}



