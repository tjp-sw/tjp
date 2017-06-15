
// future enhancements: generalize for more colors? different offsets?
void toms_best()
{
  // an attempt to use the parameter values to choose thickness automatically. could pass this into the function if that's better.
  int thick = show_parameters[COLOR_THICKNESS_INDEX] * 2;
  int color_choice = random(show_parameters[NUM_COLORS_INDEX]);
  CRGB in = show_colors[color_choice];
  
  CHSV color = rgb2hsv(in);
  double brightness = color.v;
  word half = thick/2; //15
  //word dim = 255/half; //17
  double dim = brightness/half; //17
  word i, j, x;
  //word brightness = 255;
  word count = loop_count%420;
  for (i = 0; i <4; i++) // ring count
  {
    for (j = 0; j < (420/thick); j++) //will populate the entire ring
    {
      for (x = 0; x < thick; x++) //dimming and brightening
      {
        word inc = 0;
        if (x < thick/2)
        {
          //word index = (count +x+j +(j*thick))%420;
          //leds[i][(count+x+j +(j*thick))%420] = CHSV(225,255,brightness);
          leds[i][(count+x+j +(j*thick))%420] = CHSV(color.h, color.s, color.v);
          //brightness = brightness - dim;
          color.v = color.v - dim;          
        }
        else
        {
          //leds[i][(count+x+j +(j*thick))%420] = CHSV(225,255,brightness);
          leds[i][(count+x+j +(j*thick))%420] = color;
          //brightness = brightness + dim;
          color.v = color.v + dim;
        }
        
      }
      
    }
    
  }
 // delay(1);
}


// doesn't get the color exactly right, but Erick is working on it.
CHSV rgb2hsv(CRGB in)
{
  CHSV out;
  double mini, maxi, delta;

  mini = in.r < in.g ? in.r: in.g;
  mini = mini < in.b ? mini : in.b;

  maxi = in.r > in.g ? in.r: in.g;
  maxi = maxi > in.b ? maxi : in.b;

  out.v = maxi;
  delta = maxi - mini;
  if (delta < 0.00001)
  {
    out.s = 0;
    out.h = 0;
    out.h = out.h/1.41176471;
    out.s = out.s * 255;
    return out;
  }
  if (maxi > 0.0)
  {
    out.s = (delta/maxi);
    out.s = out.s * 255;
  }
  else
  {
    out.s = 0.0;
    out.s = out.s * 255;
    return out;
  }
  if (in.r >= maxi)
  {
    out.h = (in.g - in.b)/delta;
    
  }
  if (in.g >= maxi)
  {
    out.h = 2.0 + (in.b - in.r)/delta;
  }
  else
  {
    out.h = 4.0 + (in.r-in.g)/delta;
  }
  out.h *= 60.0;

  if(out.h < 0.0)
  {
    out.h += 360.0;
  }
  out.h = out.h/1.41176471;
  return out;
  
}




// fixme: should change from taking a word between 0 and 255 to using a CRGB, because that's
//        how we're going to be given the chosen colors



// fixme: should change from taking a word between 0 and 255 to using a CRGB, because that's
//        how we're going to be given the chosen colors

void toms_best_old()
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
