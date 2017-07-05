
using namespace std;

#include<set>

/*
   A class to contain info regarding the various animations.
   The most important two attributes are: animation_int & the
   the freqiencies bands that one feels they would fit in best.
*/
class AnimationInfo
{
  private:
    int animation_int;
    set<int> bestFreqBands;

  public:
    AnimationInfo(int ani_num, set<int> bestBands) :
      animation_int(ani_num), bestFreqBands(bestBands) {}

    boolean containsBand(int band)
    {
      set<int>::iterator it = bestFreqBands.find(band);
      return it != bestFreqBands.end();
    }

    void addBand(int band)
    {
      bestFreqBands.insert(band);
    }
};

class Animations
{
  private:
    set<AnimationInfo> lowFreqAnimations;
    set<AnimationInfo> midFreqAnimations;
    set<AnimationInfo> highFreqAnimations;
    set<AnimationInfo> noFreqAnimations;


};

void setup()
{

}

void loop()
{

}

void doAnimationSelectionSetup() {


}

void doAnimatinoSelectionLoop() {


}



