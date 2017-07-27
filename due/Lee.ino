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
