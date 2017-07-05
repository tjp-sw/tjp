/*
   RJS 7/4 I think this should actaully live on the pi in python...
   Would be great if these classes were populated via a non-volitle database.
   Leaving here for now...
*/
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
    //more attributes...?

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

/*
   A class to contian lists of particular animations for various audio inputs.
   Ideally should be populated from the Database as the information is non-volitle.
*/
class Animations
{
  private:
    set<AnimationInfo> lowFreqAnimations;
    set<AnimationInfo> midFreqAnimations;
    set<AnimationInfo> highFreqAnimations;
    set<AnimationInfo> noFreqAnimations;

  public:
    void addToLowFreqAnimations(AnimationInfo ani_info)
    {
      lowFreqAnimations.inset(ani_info);
    }

    void addToMidFreqAnimations(AnimationInfo ani_info)
    {
      midFreqAnimations.inset(ani_info);
    }

    void addToHighFreqAnimations(AnimationInfo ani_info)
    {
      highFreqAnimations.inset(ani_info);
    }

    void addToNoFreqAnimations(AnimationInfo ani_info)
    {
      noFreqAnimations.inset(ani_info);
    }

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



