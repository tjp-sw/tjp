int ring_id = 0;
int spacing = LEDS_PER_STRIP/7;


void equalizer(){
  //ring_id +=1;
  //ring_id %= 8;//
  ring_id = random(0,7);
              fill_solid(leds_all,LEDS_PER_NODE,CRGB::Black);
              for(int animation_cycle = 0; animation_cycle <1000;animation_cycle++){
                //fill_solid(leds_all,LEDS_PER_NODE,CRGB::Black);
                fill_solid(&leds_all[0],spacing,CRGB::Red);
                fill_solid(&leds_all[420],spacing,CRGB::Green);
                fill_solid(&leds_all[420*2],spacing,CRGB::Purple);
                fill_solid(&leds_all[420*3],spacing,CRGB::Blue);
               
                fill_solid(&leds_all[0+spacing],spacing*ring_id,CRGB::Red);
                fill_solid(&leds_all[420+spacing],spacing*ring_id,CRGB::Green);
                fill_solid(&leds_all[420*2+spacing],spacing*ring_id,CRGB::Purple);
                fill_solid(&leds_all[420*3+spacing],spacing*ring_id,CRGB::Blue);
              }          
  }


void ripple(){
  for(int ripple_effect = 0; ripple_effect < LEDS_PER_STRIP;ripple_effect++){
    fill_solid(&leds_all[ripple_effect],5,CRGB::Black);
    fill_solid(&leds_all[ripple_effect+419],5,CRGB::Black);
    fill_solid(&leds_all[ripple_effect+(418*2)],5,CRGB:: Black);
    fill_solid(&leds_all[ripple_effect+(418*3)+4],5,CRGB::Black);
  
    fill_solid(&leds_all[419-ripple_effect],5,CRGB:: Black);
    fill_solid(&leds_all[419*2-ripple_effect],5,CRGB:: Black);
    fill_solid(&leds_all[419*3-ripple_effect],5,CRGB:: Black);
    fill_solid(&leds_all[419*4-ripple_effect],5,CRGB:: Black);
  }
}

