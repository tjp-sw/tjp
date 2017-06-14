// fixme: generalize for: more colors? different lengths of fade snakes, different offsets
// fixme: should change from taking a word between 0 and 255 to using a CRGB, because that's
//        how we're going to be given the chosen colors

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
