// This file is for completed edm animations

//---------------------------- EQUALIZER_FULL ---------------------------
// This isn't actually done yet :o
// to do: does heights[] need to be uint16_t? or can be uint8_t? Is (float) necessary on freq_max[]?
void draw_equalizer_full() {
  uint8_t unlit_pixels = MID_BLACK_THICKNESS;
  uint16_t total_colored_pixels = LEDS_PER_RING - 2 * (NUM_CHANNELS-1) * unlit_pixels;
  

  uint8_t my_colors[7] = { get_mid_color(0), get_mid_color(0, 1, 3), get_mid_color(0, 1, 9), get_mid_color(1), get_mid_color(1, 2, 3), get_mid_color(1, 2, 9), get_mid_color(2) };
  //uint8_t my_colors[3] = { get_mid_color(0), get_mid_color(1), get_mid_color(2) };

  uint16_t freq_max[NUM_CHANNELS];
  uint16_t overall_volume = 0;
  for(uint8_t chan = 0; chan < NUM_CHANNELS; chan++) {
    freq_max[chan] = frequencies_one[chan] > frequencies_two[chan] ? frequencies_one[chan] : frequencies_two[chan];
    overall_volume += freq_max[chan];
  }

  // setup levels
  uint16_t heights[NUM_CHANNELS];
  for(uint8_t i = 0; i < NUM_CHANNELS; i++) {
    heights[i] = total_colored_pixels * ((float)freq_max[i] / overall_volume);
  }
  
  for(uint8_t ring = node_number*RINGS_PER_NODE; ring < (node_number+1)*RINGS_PER_NODE; ring++)
  {
    uint16_t cur_height = 0;

    for(uint8_t chan = 0; chan < NUM_CHANNELS; chan++) {
      for(uint16_t pixel = cur_height; pixel <= cur_height + heights[chan]; pixel++) {
        mid_layer[ring][pixel] = my_colors[chan];
      }
      cur_height += heights[chan];

      if(chan < NUM_CHANNELS-1) {
        for(uint16_t pixel = cur_height; pixel < cur_height + unlit_pixels; pixel++) {
          mid_layer[ring][pixel] = TRANSPARENT;
        }
        cur_height += unlit_pixels;
      }
      else {
        // Last channel, write extra pixels until entire ring is full.
        while(cur_height < LEDS_PER_RING) {
          mid_layer[ring][cur_height++] = my_colors[NUM_CHANNELS-1];
        }
      }
    }
  }
}


