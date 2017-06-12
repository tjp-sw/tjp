void snake() {

int offset = 3;
int num_colors = show_parameters[NUM_COLORS_INDEX];
int color_length = show_parameters[COLOR_THICKNESS_INDEX];
int strip_length = num_colors * color_length + show_parameters[BLACK_THICKNESS_INDEX];

  for (int ring = 0; ring < RINGS_PER_NODE; ring++) {
    for (int pixel = 0; pixel < VISIBLE_LEDS_PER_RING; pixel++) {

      // first color for color_length pixels
      if (pixel % strip_length < color_length) {
        leds[ring][(pixel + loop_count + offset*ring) % VISIBLE_LEDS_PER_RING] = show_colors[0];
      }

      // second color for color_length pixels
      else if (pixel % strip_length < 2*color_length) {
          leds[ring][(pixel + loop_count + offset*ring) % VISIBLE_LEDS_PER_RING] = show_colors[1];
      }

      // third color for color_length pixels
      else if (pixel % strip_length < 3*color_length) {
          leds[ring][(pixel + loop_count + offset*ring) % VISIBLE_LEDS_PER_RING] = show_colors[2];
      }
      // black color for color_length pixels
      else {
        leds[ring][(pixel + loop_count + offset*ring) % VISIBLE_LEDS_PER_RING] = CRGB::Black;
      }
    }
    delay(2);
  }
}

