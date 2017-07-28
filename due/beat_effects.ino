inline void color_swap() {
  
   // Rotate colors
   CRGB temp = current_palette[2];
   current_palette[2] = current_palette[3];
   current_palette[3] = current_palette[4];
   current_palette[4] = temp;
    
   // Rotate targets so the same colors continue blending into the same targets
   temp = target_palette[2];
   target_palette[2] = target_palette[3];
   target_palette[3] = target_palette[4];
   target_palette[4] = temp;
    
   // Set all the dimming/gradient values in mid_palette that the animations will reference
   create_mid_palette(&mid_palette, current_palette[2], current_palette[3], current_palette[4]);
}

#define ALTERNATE_COLOR_THICKNESS_AMOUNT 64
inline void alternate_color_thickness() {
  if (odd_beat) {
    if(MID_COLOR_THICKNESS >= 255 - ALTERNATE_COLOR_THICKNESS_AMOUNT) { MID_COLOR_THICKNESS = 255; }
    else { MID_COLOR_THICKNESS += ALTERNATE_COLOR_THICKNESS_AMOUNT; }
    odd_beat = false;
  }
  else {
    if(MID_COLOR_THICKNESS <= ALTERNATE_COLOR_THICKNESS_AMOUNT) { MID_COLOR_THICKNESS = 0; }
    else { MID_COLOR_THICKNESS -= ALTERNATE_COLOR_THICKNESS_AMOUNT; }
    odd_beat = true;
  }
}

#define ALTERNATE_BLACK_THICKNESS_AMOUNT 64
inline void alternate_black_thickness() {
  if (odd_beat) {
    if(MID_BLACK_THICKNESS >= 255 - ALTERNATE_BLACK_THICKNESS_AMOUNT) { MID_BLACK_THICKNESS = 255; }
    else { MID_BLACK_THICKNESS += ALTERNATE_BLACK_THICKNESS_AMOUNT; }
    odd_beat = false;
  }
  else {
    if(MID_BLACK_THICKNESS <= ALTERNATE_BLACK_THICKNESS_AMOUNT) { MID_BLACK_THICKNESS = 0; }
    else { MID_BLACK_THICKNESS -= ALTERNATE_BLACK_THICKNESS_AMOUNT; }
    odd_beat = true;
  }
}


inline void pulse_color_thickness() {
  MID_COLOR_THICKNESS = downbeat_proximity;
}

// Probably only use this when motion is off; otherwise it's impossibly hard to follow
inline void pulse_black_thickness() {
  MID_BLACK_THICKNESS = downbeat_proximity;
}

#define HOLD_NODE_BLACK_LENGTH 6
inline void do_blacken_node() {
  static uint8_t hold_black_count = 0;
  if (is_beat) {
    blacken_node = true;
    
    // move current black ring CW or CCW depending on MID_INTER_RING_MOTION
    int8_t shift = MID_INTER_RING_MOTION == CW ? 1 : -1;
    blacken_node_number = (blacken_node_number + shift) % NUM_NODES;
    hold_black_count = 0;
  }
  else {
    if(++hold_black_count >= HOLD_NODE_BLACK_LENGTH) { blacken_node = false; }
    // don't change blacken_node_number back to NONE, so that it will start up where it left off next time (eg not always at same place)
  }
}

#define HOLD_RING_BLACK_LENGTH 5
inline void do_blacken_ring() {
  static uint8_t hold_black_count = 0;
  
  if (is_beat) {
    blacken_ring = true;

    // move current black ring CW or CCW depending on MID_INTER_RING_MOTION
    int8_t shift = MID_INTER_RING_MOTION == CW ? 1 : -1;
    blacken_ring_number += 2*shift;
    if(blacken_ring_number < 0) { blacken_ring_number += NUM_RINGS; }
    else if(blacken_ring_number >= NUM_RINGS) { blacken_ring_number -= NUM_RINGS; }
    
    hold_black_count = 0;
  }
  else {
    if(++hold_black_count >= HOLD_RING_BLACK_LENGTH) { blacken_ring = false; }
    // don't change blacken_ring_number back to NONE, so that it will start up where it left off next time (eg not always at same place)
  }
}
