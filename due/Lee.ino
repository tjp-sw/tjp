// run this one first -- will give a lot of color shades
// at fixed cumulative brightness

void normalized_all_colors() {

  int ring = 0;
  int pixel = 0;
  int counter = 0;
  CRGB color;
  
  for (int r = 248; r >= 0; r -= 8) {
    for (int g = 248; g >= 0; g -= 8) {
      for (int b = 248; b >= 0; b -= 8) {
        if (r + g + b == 256) {
          counter++;
          Serial.print("Counter = ");
          Serial.print(counter);
          Serial.print(", r = ");
          Serial.print(r);
          Serial.print(", g = ");
          Serial.print(g);
          Serial.print(", b = ");
          Serial.println(b);
          if (pixel >= 408) {
            ring++;
            pixel = 0;
          }
          color.r = r;
          color.g = g;
          color.b = b;
          leds[get_1d_index(ring, pixel)] = color;
          leds[get_1d_index(ring, pixel+1)] = color;       
          leds[get_1d_index(ring, pixel+2)] = CRGB::Black;    
          pixel+=3;
        }
      }
    }
  }
}


// if you like a shade and want to choose a brightness, then put 
// the rgb values here, and it will show a range of brightnesses

void scale_brightness() {
  // change these values
  int r = 216;
  int g = 0;
  int b = 40;

  int ring = 0;
  int pixel = 0;
  CRGB color;

  int max = (r > g) ? ((r > b) ? r : b) : ((g > b) ? b : b);
  float tenths_max_scale = (10 * 255)/max;

  for (int mult = 0; mult <= tenths_max_scale; mult++) {

          if (pixel >= 408) {
            ring++;
            pixel = 0;
          }
          color.r = r * mult / 10;
          color.g = g * mult / 10;
          color.b = b * mult / 10;
          leds[get_1d_index(ring, pixel)] = color;
          leds[get_1d_index(ring, pixel+1)] = color;       
          leds[get_1d_index(ring, pixel+2)] = CRGB::Black;   
          pixel+=3;
                    
          Serial.print("mult = ");
          Serial.print(mult);
          Serial.print(", r = ");
          Serial.print(color.r);
          Serial.print(", g = ");
          Serial.print(color.g);
          Serial.print(", b = ");
          Serial.println(color.b);
  }
}


// put your final choices here if you want to see them together 
// to see if there's too much overlap

void check_color() {

  // enter up to 20 rgb color values
  CRGB test[] = { 
          {2, 0, 10},
    {10, 20, 54}, 
    {0, 10, 27},
       {40, 80, 216 },
       
    {15, 20, 54}, 
    {7, 10, 27},
    {100, 0, 0},
    
      {10, 20, 44}, 
    {10, 15, 44}, 
    {10, 15, 44}, 
     {10, 15, 44},       
     {15, 30, 81},
           
    {0, 20, 54},
    {20, 40, 108},  
{2, 0, 10},
    {10, 20, 54}, 
    {0, 10, 27},
       {40, 80, 216 },
       
    {15, 20, 54}, 
    {7, 10, 27},
    {100, 0, 0},
    
      {10, 20, 44}, 
    {10, 15, 44}, 
    {10, 15, 44}, 
     {10, 15, 44},       
     {15, 30, 81},
           
    {0, 20, 54},
    {20, 40, 108} 
    };
  for (int ring = 0; ring < NUM_RINGS; ring++) {
    for (int color_num= 0; color_num < 40; color_num++) {
      for (int pixel = 0; pixel < 7; pixel++) {
        leds[get_1d_index(ring, pixel + 10 * color_num)] = test[color_num];
      }
      for (int pixel = 7; pixel < 10; pixel++) {
        leds[get_1d_index(ring, pixel + 10 * color_num)] = CRGB::Black;
      }
    }
  }
}

void pick_hsv_colors() {
  Serial.setTimeout(500);
  uint8_t starting_hue = 0;
  uint8_t hue_increment = 4;
  uint8_t brightness = 255;
  CRGB selected_colors[7] = { CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black, CRGB::Black };

  String text;
  int temp;
  Serial.println();
  Serial.println();
  
  while(true) {
    pick_hsv_colors_draw(starting_hue, hue_increment, brightness, selected_colors);
    Serial.println();
    Serial.println("1. Set starting hue");
    Serial.println("2. Set hue increment");
    Serial.println("3. Set brightness");
    Serial.println("4. Pick a color");

    while(Serial.available() == 0) ;
    String text = Serial.readString();
    switch(text.toInt()) {
      case 1:
        Serial.println("Enter starting hue (0 to 255):");
        while(Serial.available() == 0) ;
        text = Serial.readString();
        temp = text.toInt();
        if(temp < 0 || temp > 255) { Serial.println("Invalid entry"); }
        else { starting_hue = temp; }
        break;
        
      case 2:
        Serial.println("Enter hue increment (1 to 10):");
        while(Serial.available() == 0) ;
        text = Serial.readString();
        temp = text.toInt();
        if(temp < 1 || temp > 10) { Serial.println("Invalid entry"); }
        else { hue_increment = temp; }
        break;

      case 3:
        Serial.println("Enter brightness (1 to 255):");
        while(Serial.available() == 0) ;
        text = Serial.readString();
        temp = text.toInt();
        if(temp < 1 || temp > 255) { Serial.println("Invalid entry"); }
        else { brightness = temp; }
        break;

      case 4:
        Serial.println("Enter strand # (1 to 10):");
        while(Serial.available() == 0) ;
        text = Serial.readString();
        temp = text.toInt();
        if(temp < 1 || temp > 10) { Serial.println("Invalid entry"); }
        else { 
          uint8_t strand = temp;
          Serial.println("Enter pixel # (1 to 256):");
          while(Serial.available() == 0) ;
          text = Serial.readString();
          temp = text.toInt();
          if(temp < 0 || temp > 256) { Serial.println("Invalid entry"); }
          else {
            uint8_t pixel = temp - 1;
            Serial.println("Enter palette color # (1 to 7):");
            while(Serial.available() == 0) ;
            text = Serial.readString();
            temp = text.toInt();
            if(temp < 1 || temp > 7) { Serial.println("Invalid entry"); }
            else {
              selected_colors[temp-1] = CHSV(starting_hue + hue_increment * (strand-1), 256 - pixel, brightness);
              CRGB tempCol = selected_colors[temp-1]; 
              Serial.println();

              for(uint8_t i = 0; i < 7; i++) { Serial.println("Color " + String(i+1) + " = rgb(" + String(selected_colors[i].r) + ", " + String(selected_colors[i].g) + ", " + String(selected_colors[i].b) + ")"); }
              Serial.println();
            }
          }
        }
        break;
        
      default:
        Serial.println("Invalid entry");
    }
  }
}

void pick_hsv_colors_draw(uint8_t starting_hue, uint8_t hue_increment, uint8_t brightness, CRGB selected_colors[7]) {
  for(uint8_t ring = 0; ring < 10; ring++) {
    uint8_t hue = starting_hue + ring*hue_increment;
    uint16_t curPixel = 0;

    for(int16_t saturation = 255; saturation > 84; saturation--) {
      set_led(ring, curPixel, CHSV(hue, saturation, brightness));
      if(++curPixel % 20 == 0) {
        set_led(ring, curPixel++, CRGB::Black);
      }
    }
  }

  
  for(uint8_t col = 0; col < 7; col++) {
    for(uint8_t i = 0; i < 10; i++) {
      set_led(11, i + 12*col, selected_colors[col]);
    }
  }
  
  LEDS.show();
}

