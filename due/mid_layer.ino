/*
 * Function that creates a line of leds. When they collide in the middle changes it to a rainbow formation.
 * Runs Black led through rainbow until end.
 */
int count = 0;
int finish = 0;
int my_line;
void snake_to_bow(){
 if(finish <3 ){
  if(count <= 210){
    fill_solid(leds_all,LEDS_PER_NODE,CRGB::Black);
    sparklet(80);
    fill_solid(&leds_all[count],5,CRGB::Green);
    fill_solid(&leds_all[count+419],5,CRGB::Blue);
    fill_solid(&leds_all[count+(418*2)],5,CRGB:: Red);
    fill_solid(&leds_all[count+(418*3)+4],5,CRGB::Green);
  
    fill_solid(&leds_all[419-count],5,CRGB:: Green);
    fill_solid(&leds_all[419*2-count],5,CRGB:: Blue);
    fill_solid(&leds_all[419*3-count],5,CRGB:: Red);
    fill_solid(&leds_all[419*4-count],5,CRGB:: Green);
  }
  if(count>=210){
    rainbow(LEDS_PER_NODE,1);
    fill_solid(&leds_all[count],20,CRGB::Blue);
    fill_solid(&leds_all[count+419],20,CRGB::Green);
    fill_solid(&leds_all[count+(418*2)],20,CRGB:: Purple);
    fill_solid(&leds_all[count+(418*3)+4],20,CRGB::Red);
  
    fill_solid(&leds_all[419-count],20,CRGB:: Blue);
    fill_solid(&leds_all[419*2-count],20,CRGB:: Green);
    fill_solid(&leds_all[419*3-count],20,CRGB:: Purple);
    fill_solid(&leds_all[419*4-count],20,CRGB:: Red);
  }
  
  count++;
  count%=420;
  if(count == 419){
    finish++;
  }
}
 else{
  fill_gradient_RGB(leds_all,419*4,CRGB::Green, CRGB:: Purple);
 }
}
// end snake to bow


