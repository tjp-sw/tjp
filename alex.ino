
/*
 * Function takes in a number of leds to color with built in fastled rainbow function
 * Takes in a integer to sparkle, adds blinking white lights, to the array
 */
void rainbow(int sparkle_on){
  int num_to_color = show_parameters[COLOR_THICKNESS_INDEX] *show_parameters[NUM_COLORS_INDEX];
  fill_rainbow(leds_all,num_to_color,15,20);
  if(sparkle_on){
    sparklet(50,num_to_color);
  }
}
// end rainbow

/*
 * Function that takes in a probability to compare with random8. Will add "white pop" to random leds
 */
void sparklet(int prob,int num_to_color){ 
  if(random8() >= prob){
    leds_all[random16(num_to_color)] += CRGB::White;
  } 
}
// end sparklet

/*
 * Function that takes in CRGB colors and creates a gradient between them, Takes in a start postion, and end postion
 */
void basicGradient(){
    int num_colors = show_parameters[NUM_COLORS_INDEX];
    int color_length = show_parameters[COLOR_THICKNESS_INDEX];
    int spacing = LEDS_PER_NODE/num_colors;
    int palette = show_parameters[PALETTE_INDEX];
    int strip_size = num_colors * color_length;
    CRGB color1 = get_color(palette,0);
    CRGB color2 = get_color(palette,1);
    CRGB color3 = get_color(palette,2);
    CRGB color4 = get_color(palette,3);
    CRGB color5 = get_color(palette,4);
    CRGB color6 = get_color(palette,5);
    CRGB color7 = get_color(palette,6);
    switch(num_colors){
      case 2:
        fill_gradient_RGB(leds_all,spacing*2,color1,color2);
        break;
      case 3:
        fill_gradient_RGB(leds_all,spacing*3,color1,color2,color3);      
        break;
      case 4:
        fill_gradient_RGB(leds_all,spacing*4,color1,color2,color3,color4);
        break;
      case 6:
        fill_gradient_RGB(leds_all,spacing*3,color1,color2,color3);
        fill_gradient_RGB(&leds_all[spacing*3],spacing*3,color4,color5,color6);
        break; 
      case 8:
        fill_gradient_RGB(leds_all,spacing*4,color1,color2,color3,color4);
        fill_gradient_RGB(&leds_all[spacing*4],spacing*4,color5,color6,color7,color6);
        break;
      case 9:
        fill_gradient_RGB(leds_all,spacing*3,color1,color2,color3);
        fill_gradient_RGB(&leds_all[spacing*3],spacing*3,color4,color5,color6);
        fill_gradient_RGB(&leds_all[spacing*6],spacing*3,color7,color6,color5);
        break;
      case 12:
        fill_gradient_RGB(leds_all,spacing*4,color1,color2,color3,color4);
        fill_gradient_RGB(&leds_all[spacing*4],spacing*4,color5,color6,color7,color6);
        fill_gradient_RGB(&leds_all[spacing*8],spacing*4,color5,color4,color3,color2);
        break;
      default:
        fill_gradient_RGB(leds_all,LEDS_PER_NODE,CRGB::White,CRGB::Black);
        
    }
}
// end basic gradient

// Testing equalizer
void equalizer(){
  leds_all = CRGB::Black;
  
  int palette = show_parameters[PALETTE_INDEX];
  int bound = HALF_RING/NUM_CHANNELS - 1; // -1 for empty pixel between channels

  CRGB my_colors[7];  
  // setup colors
  for(int i = 0; i < NUM_CHANNELS; i++){
    my_colors[i] = get_color(palette, i);
  }

  // setup levels
  uint16_t heights[NUM_CHANNELS][2];
  for(uint8_t i = 0; i < NUM_CHANNELS; i++) {
    heights[i][0] = frequencies_max[i];
    if(heights[i][0] >= 255)
      heights[i][0] = bound;
    else
      heights[i][0] = heights[i][0] * bound / 255;

    if(heights[i][0] > 0)
      heights[i][0]--;
    
    heights[i][1] = HALF_RING * ((float)frequencies_max[i] / overall_volume[0]);
    if(heights[i][1] > 0)
      heights[i][1]--;
  }
  
  for(uint8_t ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++)
  {
    uint16_t curHeight1 = 0;
    uint16_t curHeight2 = LEDS_PER_RING - 1;
    for(uint8_t i = 0; i < NUM_CHANNELS; i++) {
      for(uint16_t j = curHeight1; j <= curHeight1 + heights[i][0]; j++)
        leds_raw[get_idx(ring, j)] = my_colors[i];
      curHeight1 += (heights[i][0] + 2);
      for(uint16_t j = curHeight2; j >= curHeight2 - heights[i][1]; j--)
      leds_raw[get_idx(ring, j)] = my_colors[i];
      curHeight2 -= (heights[i][1] + 2);
    }
  }    
}

/*
 * Function that creates a line of leds. When they collide in the middle changes it to a rainbow formation.
 * Runs Black led through rainbow until end.

int count = 0;
int finish = 0;
int my_line;
void snake_to_bow(){
  if(count <= 210){
    fill_solid(leds_all,LEDS_PER_NODE,CRGB::Black);
    sparklet(80);
    fill_solid(&leds_all[count],5+count,CRGB::Green);
    fill_solid(&leds_all[count+419],5+count,CRGB::Blue);
    fill_solid(&leds_all[count+(418*2)],5+count,CRGB:: Red);
    fill_solid(&leds_all[count+(418*3)+4],5+count,CRGB::Green);
  
    fill_solid(&leds_all[419-count],5+count,CRGB:: Green);
    fill_solid(&leds_all[419*2-count],5+count,CRGB:: Blue);
    fill_solid(&leds_all[419*3-count],5+count,CRGB:: Red);
    fill_solid(&leds_all[419*4-count],5+count,CRGB:: Green);
  }
  if(count>=210){
    rainbow(1);
    fill_solid(&leds_all[count],20+count,CRGB::Blue);
    fill_solid(&leds_all[count+419],20+count,CRGB::Green);
    fill_solid(&leds_all[count+(418*2)],20+count,CRGB:: Purple);
    fill_solid(&leds_all[count+(418*3)+4],20+count,CRGB::Red);
  
    fill_solid(&leds_all[419-count],20+count,CRGB:: Blue);
    fill_solid(&leds_all[419*2-count],20+count,CRGB:: Green);
    fill_solid(&leds_all[419*3-count],20+count,CRGB:: Purple);
    fill_solid(&leds_all[419*4-count],20+count,CRGB:: Red);
  }
  
  count++;
  count%=420;
}
// end snake to bow
*/





