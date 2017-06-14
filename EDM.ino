//  Sends alternating bands of colors rotating around the rings
//
//  Note use of getcolor(), a work-around for not being able to make a 2D array of CRGBs
// fixme: remove delay()

// Creates repeated snakes with show_parameters[NUM_COLORS_INDEX] colors, with each color repeated 
// show_parameters[COLOR_THICKNESS_INDEX] times, separated by show_parameters[BLACK_THICKNESS_INDEX] 
// black LEDs.

// offset determines how many steps a ring's lights are rotated to put on the adjacent ring


void snake() {

int offset = show_parameters[RING_OFFSET_INDEX];
int rotation_direction = show_parameters[INTRA_RING_MOTION_INDEX];
int num_colors = show_parameters[NUM_COLORS_INDEX];
int color_length = show_parameters[COLOR_THICKNESS_INDEX];
int strip_length = num_colors * color_length + show_parameters[BLACK_THICKNESS_INDEX];
int palette_num = show_parameters[PALETTE_INDEX];

  // slow down the animation without using delay()
  if (loop_count % 50 == 0) {
    for (int ring = 0; ring < RINGS_PER_NODE; ring++) {
      for (int pixel = 0; pixel < VISIBLE_LEDS_PER_RING; pixel++) {

        // first color for color_length pixels
        if (pixel % strip_length < color_length) {
          leds[ring][(pixel + rotation_direction * loop_count + offset*ring) % VISIBLE_LEDS_PER_RING] = get_color(palette_num, show_colors[0]);
        }

        // second color for color_length pixels
        else if (pixel % strip_length < 2*color_length) {
            leds[ring][(pixel + rotation_direction * loop_count + offset*ring) % VISIBLE_LEDS_PER_RING] = get_color(palette_num, show_colors[1]);
        }

        // third color for color_length pixels
        else if (pixel % strip_length < 3*color_length) {
            leds[ring][(pixel + rotation_direction * loop_count + offset*ring) % VISIBLE_LEDS_PER_RING] = get_color(palette_num, show_colors[2]);
        }
        // black color for color_length pixels
        else {
          leds[ring][(pixel + rotation_direction * loop_count + offset*ring) % VISIBLE_LEDS_PER_RING] = CRGB::Black;
        }
      }
    }
  }
}


// fixme: should be adapted for CRGB colors input, like animation_color = get_color(PALETTE_INDEX, 3);
// fixme: maybe here it would be fun to have each node pick its own color out of the palette, between indices 2 and 4
// but that proably wouldn't make it all same color, and that's fun too
// this is a generalization we can work on

void toms_best()
{
  word count = loop_count%420;
  word i, j;
  word x = 155; //must fall between 0 and 255; changes color
  for(i = 0; i<12; i++)
  {
    for (j = 0; j<21; j++)
    {
      leds[i][(count +(j*20)+ (i*10))%420] = CHSV(x,255,225);
      
      leds[i][(count + 1 + (j*20)+ (i*10))%420] = CHSV(x,255,225);
      
      leds[i][(count + 2 + (j*20)+ (i*10))%420] = CHSV(x,255,200);
      
      leds[i][(count + 3 + (j*20)+ (i*10))%420] = CHSV(x,255,175);
      
      leds[i][(count + 4 + (j*20)+ (i*10))%420] = CHSV(x,255,150);
      
      leds[i][(count + 5 + (j*20)+ (i*10))%420] = CHSV(x,255,125);
      
      leds[i][(count + 6 + (j*20)+ (i*10))%420] = CHSV(x,255,100);
      
      leds[i][(count + 7 + (j*20)+ (i*10))%420] = CHSV(x,255,75);
      
      leds[i][(count + 8 + (j*20)+ (i*10))%420] = CHSV(x,255,50);
      
      leds[i][(count + 9 + (j*20)+ (i*10))%420] = CHSV(x,255,25);
      
      leds[i][(count + 10 + (j*20)+ (i*10))%420] = CHSV(x,255,25);
      
      leds[i][(count + 11 + (j*20)+ (i*10))%420] = CHSV(x,255,50);
      
      leds[i][(count + 12 + (j*20)+ (i*10))%420] = CHSV(x,255,75);
      
      leds[i][(count + 13+ (j*20)+ (i*10))%420] = CHSV(x,255,100);
      
      leds[i][(count + 14 + (j*20)+ (i*10))%420] = CHSV(x,255,125);
      
      leds[i][(count + 15 + (j*20)+ (i*10))%420] = CHSV(x,255,150);
      
      leds[i][(count + 16 + (j*20)+ (i*10))%420] = CHSV(x,255,175);
      
      leds[i][(count + 17 + (j*20)+ (i*10))%420] = CHSV(x,255,200);
     
      leds[i][(count + 18 + (j*20)+ (i*10))%420] = CHSV(x,255,225);
      leds[i][(count + 19 + (j*20)+ (i*10))%420] = CHSV(x,255,225);
      /*
      leds[i]((count+(j*20))%420, (count+2+(j*20))%420) = CRGB(255,0,0);//CRGB::Red; //brightest
      leds[i]((count+3+(j*20))%420, (count+5+(j*20))%420) = CRGB(150,0,0);//CRGB::Red; //brightest
      leds[i]((count+6+(j*20))%420, (count+8+(j*20))%420) = CRGB(50,0,0);//CRGB::Blue; // 2nd brightest
      leds[i]((count+9+(j*20))%420, (count+11+(j*20))%420) = CRGB(0,0,0);//CRGB::Red; // 3rd brightest
      leds[i]((count+12+(j*20))%420, (count+14+(j*20))%420) = CRGB(50,0,0);//CRGB::Blue; // 2nd brightest
      leds[i]((count+15+(j*20))%420, (count+17+(j*20))%420) = CRGB(150,0,0); //brightest
      leds[i]((count+18+ (j*20))%420, (count+20+(j*20))%420) = CRGB(255,0,0);//CRGB::Red; //brightest
      */
    }
  }
}

