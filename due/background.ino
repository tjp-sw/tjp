
void alex(){
  
}
/*
 * Function takes in a number of leds to color with built in fastled rainbow function
 * Takes in a integer to sparkle, adds blinking white lights, to the array
 */
void rainbow(int num_to_color, int sparkle_on){
  fill_rainbow(leds_all,num_to_color,15,20);
  if(sparkle_on){
    sparklet(95);
  }
}
// end rainbow

/*
 * Function that takes in a probability to compare with random8. Will add "white pop" to random leds
 */
void sparklet(int prob){ 
  if(random8() >= prob){
    leds_all[random16(LEDS_PER_NODE)] += CRGB::White;
  } 
}
// end sparklet

/*
 * Function that takes in CRGB colors and creates a gradient between them, Takes in a start postion, and end postion
 */
void basicGradient(const CRGB color1, const CRGB color2, int start, int fin){
    fill_gradient_RGB(leds_all,start,color1,fin,color2);
  }
// end basic gradient
/*
// eight basic gradient. Will change based on parameter
void gradientEight(const CRGB color1,const CRGB color2,const CRGB color3,const CRGB color 4, const CRGB color5, const CRGB color6, const CRGB color 7, const CRGB color8){
  fill_gradient_RGB(leds_all,0,color1,LEDS_PER_STRIP-1,color2);
  fill_gradient_RGB(leds_all,LEDS_PER_STRIP,color3,(LEDS_PER_STRIP*2)-1,color4;
  fill_gradient_RGB(leds_all,LEDS_PER_STRIP*2,color5,(LEDS_PER_STRIP*3)-1,CRGB:: Black);
  fill_gradient_RGB(leds_all,LEDS_PER_STRIP*3,CRGB::Brown,(LEDS_PER_STRIP*4)-1,CRGB:: Purple);
}*/

//end gradient_eight


