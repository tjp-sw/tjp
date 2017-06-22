CRGB scale_intensity( CRGB my_color, int increase){
  CRGB to_return;
  // get the max value from old rgb
  int my_max = find_max(my_color.red,my_color.blue,my_color.green);
  // calculate scalar using largest rgb value
  int scalar = 255/my_max;
  // calculate new color based on increase
  to_return.red = scalar*(increase/100)*my_color.red;
  to_return.green = scalar*(increase/100)*my_color.green;
  to_return.blue = scalar*(increase/100)*my_color.blue;
  return to_return;
}

// helper function used to calculate max of three numbers
int find_max( int a, int b, int c){
  int biggest = a;
  (biggest < b) && ( biggest = b); // set biggest to b if and only if biggest is less than b. do same for c.
  (biggest < c) && ( biggest = c);
  return biggest;
}

/*
 * Function takes in a number of leds to color with built in fastled rainbow function
 * Takes in a integer to sparkle, adds blinking white lights, to the array
 */
void rainbow(int sparkle_on){
  int num_to_color = show_parameters[COLOR_THICKNESS_INDEX] *show_parameters[NUM_EDM_COLORS_INDEX];
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
    int num_colors = show_parameters[NUM_EDM_COLORS_INDEX];
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
  int color_thickness = show_parameters[COLOR_THICKNESS_INDEX];
  int palette = show_parameters[PALETTE_INDEX];
  int spacing = LEDS_PER_NODE/RINGS_PER_NODE;
  int bound = spacing/14;
  int my_rand = random(0,bound);
  CRGB my_colors[7];
  CRGB temp;
  // setup colors 
  for(int i = 0; i<7; i++){
    temp = get_color(palette,i);
    my_colors[i] = temp;
  }
  // FIRST HALF OF RING
  
  // bottom 1/7
    fill_solid(&leds_all[0],my_rand,my_colors[0]);
    fill_solid(&leds_all[spacing],my_rand,my_colors[0]);
    fill_solid(&leds_all[spacing*2],my_rand,my_colors[0]);
    fill_solid(&leds_all[spacing*3],my_rand,my_colors[0]);

  // 2/7
    fill_solid(&leds_all[bound],my_rand,my_colors[1]);
    fill_solid(&leds_all[spacing +bound],my_rand,my_colors[1]);
    fill_solid(&leds_all[spacing*2 +bound],my_rand,my_colors[1]);
    fill_solid(&leds_all[spacing*3 +bound],my_rand,my_colors[1]);
  
  // 3/7
    fill_solid(&leds_all[bound*2],my_rand,my_colors[2]);
    fill_solid(&leds_all[spacing +bound*2],my_rand,my_colors[2]);
    fill_solid(&leds_all[spacing*2 +bound*2],my_rand,my_colors[2]);
    fill_solid(&leds_all[spacing*3 +bound*2],my_rand,my_colors[2]);
    
  // 4/7
    fill_solid(&leds_all[bound*3],my_rand,my_colors[3]);
    fill_solid(&leds_all[spacing +bound*3],my_rand,my_colors[3]);
    fill_solid(&leds_all[spacing*2 +bound*3],my_rand,my_colors[3]);
    fill_solid(&leds_all[spacing*3 +bound*3],my_rand,my_colors[3]);
  
  // 5/7
    fill_solid(&leds_all[bound*4],my_rand,my_colors[4]);
    fill_solid(&leds_all[spacing +bound*4],my_rand,my_colors[4]);
    fill_solid(&leds_all[spacing*2 +bound*4],my_rand,my_colors[4]);
    fill_solid(&leds_all[spacing*3 +bound*4],my_rand,my_colors[4]);
  
  // 6/7
    fill_solid(&leds_all[bound*5],my_rand,my_colors[5]);
    fill_solid(&leds_all[spacing +bound*5],my_rand,my_colors[5]);
    fill_solid(&leds_all[spacing*2 +bound*5],my_rand,my_colors[5]);
    fill_solid(&leds_all[spacing*3 +bound*5],my_rand,my_colors[5]);
  
  // 7/7
    fill_solid(&leds_all[bound*6],my_rand,my_colors[6]);
    fill_solid(&leds_all[spacing +bound*6],my_rand,my_colors[6]);
    fill_solid(&leds_all[spacing*2 +bound*6],my_rand,my_colors[6]);
    fill_solid(&leds_all[spacing*3 +bound*6],my_rand,my_colors[6]); 


 // SECOND HALF OF RING
 // 7/7
    fill_solid(&leds_all[bound*7],my_rand,my_colors[6]);
    fill_solid(&leds_all[spacing +bound*7],my_rand,my_colors[6]);
    fill_solid(&leds_all[spacing*2 +bound*7],my_rand,my_colors[6]);
    fill_solid(&leds_all[spacing*3 +bound*7],my_rand,my_colors[6]);
 // 6/7   
    fill_solid(&leds_all[bound*8],my_rand,my_colors[5]);
    fill_solid(&leds_all[spacing +bound*8],my_rand,my_colors[5]);
    fill_solid(&leds_all[spacing*2 +bound*8],my_rand,my_colors[5]);
    fill_solid(&leds_all[spacing*3 +bound*8],my_rand,my_colors[5]);
// 5/7
    fill_solid(&leds_all[bound*9],my_rand,my_colors[4]);
    fill_solid(&leds_all[spacing +bound*9],my_rand,my_colors[4]);
    fill_solid(&leds_all[spacing*2 +bound*9],my_rand,my_colors[4]);
    fill_solid(&leds_all[spacing*3 +bound*9],my_rand,my_colors[4]);

// 4/7
    fill_solid(&leds_all[bound*10],my_rand,my_colors[3]);
    fill_solid(&leds_all[spacing +bound*10],my_rand,my_colors[3]);
    fill_solid(&leds_all[spacing*2 +bound*10],my_rand,my_colors[3]);
    fill_solid(&leds_all[spacing*3 +bound*10],my_rand,my_colors[3]);

// 3/7
    fill_solid(&leds_all[bound*11],my_rand,my_colors[2]);
    fill_solid(&leds_all[spacing +bound*11],my_rand,my_colors[2]);
    fill_solid(&leds_all[spacing*2 +bound*11],my_rand,my_colors[2]);
    fill_solid(&leds_all[spacing*3 +bound*11],my_rand,my_colors[2]);

// 2/7
    fill_solid(&leds_all[bound*12],my_rand,my_colors[1]);
    fill_solid(&leds_all[spacing +bound*12],my_rand,my_colors[1]);
    fill_solid(&leds_all[spacing*2 +bound*12],my_rand,my_colors[1]);
    fill_solid(&leds_all[spacing*3 +bound*12],my_rand,my_colors[1]);

// 1/7
    fill_solid(&leds_all[bound*13],my_rand,my_colors[0]);
    fill_solid(&leds_all[spacing +bound*13],my_rand,my_colors[0]);
    fill_solid(&leds_all[spacing*2 +bound*13],my_rand,my_colors[0]);
    fill_solid(&leds_all[spacing*3 + bound*13],my_rand,my_colors[0]);


  
    
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





