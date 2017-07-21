void color_swap() {
  
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


void alternate_color_thickness() {
  if (odd_beat) {
    MID_COLOR_THICKNESS += 10;
    odd_beat = false;
  }
  else {
    MID_COLOR_THICKNESS -= 10;
    odd_beat = true;
  }
}


void alternate_black_thickness() {
  if (odd_beat) {
    MID_BLACK_THICKNESS += 20;
    odd_beat = false;
  }
  else {
    MID_BLACK_THICKNESS -= 20;
    odd_beat = true;
  }
}

void do_blacken_node() {
   // fixme: needs testing by diane
  if (beat_proximity <= 5 || beat_proximity >= 95) {
    blacken_node = true;
    
    // move current black ring CW or CCW depending on MID_INTER_RING_MOTION
    uint8_t shift = MID_INTER_RING_MOTION != 0 ? MID_INTER_RING_MOTION : 1;
    blacken_node_number = (blacken_node_number + shift) % NUM_NODES;
  }
  else {
    blacken_node = false;
    // don't change blacken_node_number back to NONE, so that it will start up where it left off next time (eg not always at same place)
  }
}

void do_blacken_ring() {
  // fixme: needs testing by diane
  Serial.print("beat proximity ");
  Serial.println(beat_proximity);
  if (beat_proximity <= 4 || beat_proximity >= 95) {
    blacken_ring = true;

    // move current black ring CW or CCW depending on MID_INTER_RING_MOTION
    uint8_t shift = MID_INTER_RING_MOTION != 0 ? MID_INTER_RING_MOTION : 1;
    blacken_ring_number = (blacken_ring_number + shift) % NUM_RINGS; 
    Serial.print("blacken ring number ");
    Serial.print(blacken_ring_number);
  }
  else {
    blacken_ring = false;
    // don't change blacken_ring_number back to NONE, so that it will start up where it left off next time (eg not always at same place)
  }
}

