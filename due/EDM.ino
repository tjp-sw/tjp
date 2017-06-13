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

