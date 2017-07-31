#include <FastLED.h>

/* To do:
   Test when peak history is changed to time the read occured, not current_time
   Test shorter delays in read_frequencies
   Change delta from unsigned long to uint16_t?
   Test predictionScore as uint16_t; make sure it doesn't ever overflow in the middle of a calculation
   In set_f): use a rolling index instead of copying all of the arrays each cycle
   Do frequencies[] arrays need to be ints? Would be better as uint8_t, scaling or capping if necessary.
*/
// ----------------------------------------- Peak/Beat detection ----------------------------------------------------
#define NOISE_REDUCT 120 // Fixed amount to reduce each spectrum shield read by
#define PEAK_WINDOW_SIZE 5 // Size of window for determining peaks, center must be maximum for peak to be marked
#define PEAK_HISTORY_SIZE 10 // Number of peaks to remember for beat detection

#define PEAK_AVG_FACTOR 1 // Peaks must be this much greater than the average of nearby points, after noise reduction
#define PEAK_HURDLE_SIZE 70 // Peaks must additionally be this fixed amount greater than average of nearby points
#define PEAK_TOLERANCE 55 // How far peaks can drift (per beat) and still be considered a consistent beat
#define BEAT_PREDICT_TOLERANCE 30 // Max ms a beat will trigger early (should be ~1/2 of refresh time)

#define MIN_BEAT_DELTA 400 // 150 BPM
#define MAX_BEAT_DELTA 799 // >75 BPM

//DUE DEFINTIONS
// Spectrum Shield -----------------------------------------------------------//
// Pin connections                                                            //
#define SS_PIN_STROBE 4                                                       //
#define SS_PIN_RESET 5                                                        //
#define SS_PIN_DC_ONE A0                                                      //
#define SS_PIN_DC_TWO A1                                                      //
#define NUM_CHANNELS 7
#define DEBUG_BPM
#define DEBUG_PEAKS
#define DEBUG_AUDIO_HOOKS
//
//MY GLOBALS
bool isRecording = false;
unsigned long startMs = millis();
unsigned long endMs = millis();
// Globals                                                                    //
int frequencies_one[NUM_CHANNELS];                                            //
int frequencies_two[NUM_CHANNELS];                                            //
int frequencies_max[NUM_CHANNELS];                                            //
//
bool is_beat = false;
bool printInfo = true;                                                   //
uint8_t downbeat_proximity = 0; // Up and down from 0-255 with the beat       //
uint8_t bpm_estimate = 0;                                                    //
uint8_t bpm_confidence = 0; // <10 is weak, 20 is decent, 30+ is really good  //
//
#define AUDIO_HOOK_HISTORY_SIZE 200                                           //
uint16_t overall_volume[AUDIO_HOOK_HISTORY_SIZE];                             //
uint8_t dominant_channel[AUDIO_HOOK_HISTORY_SIZE];                            //
float band_distribution[AUDIO_HOOK_HISTORY_SIZE][3]; // Low=0, mid=1, high=2  //

//RJS variables I will use to see the difference between our sampling code (high vs low sets)
uint16_t overallVolume[AUDIO_HOOK_HISTORY_SIZE];                             //
uint8_t dominantChannel[AUDIO_HOOK_HISTORY_SIZE];                            //
float bandDistribution[AUDIO_HOOK_HISTORY_SIZE][3];
int frequenciesMax[NUM_CHANNELS];
int left[NUM_CHANNELS];
int right[NUM_CHANNELS];

static const int FREQ_BAND_HISTORY_OF_INTEREST = 53;
uint8_t freqbandMode[AUDIO_HOOK_HISTORY_SIZE];
//
uint8_t low_band_emphasis = 0; // 0 or 1                                      //
uint8_t mid_band_emphasis = 0; // 0, 1, or 2                                  //
uint8_t high_band_emphasis = 0; // 0 or 1                                     //
//----------------------------------------------------------------------------//
// Globals                                                                        //
bool new_animation_triggered = false;                                             //
uint8_t current_animation = 0;                                                    //
uint32_t loop_count = 0;                                                          //
unsigned long current_time = 0, animation_start_time = 0;                         //
unsigned long long epoch_msec;
//END DUE DEFINITIONS

// Histories of reads and detected peaks
int read_history[NUM_CHANNELS][PEAK_WINDOW_SIZE];
unsigned long last_read_time[PEAK_WINDOW_SIZE];
unsigned long peak_history[NUM_CHANNELS][PEAK_HISTORY_SIZE];


// ----------------------------------------------- BPM and Phase estimates ------------------------------------------------------------------------------
unsigned long last_beat_trigger = 0, next_beat_prediction = 0;

#define BPM_HISTORY_SIZE 12
uint8_t bpm_history[BPM_HISTORY_SIZE];

#define BEAT_HALF_TIME // If defined, makes downbeat_proximity move up for one beat, then down for one beat, instead of up and down each beat
#define CONFIDENCE_DECAY_RATE 2 // Decay 2 confidence from a read every 3 cycles

#define MIN_SCORE_BPM_ADJUST 250
#define MIN_SCORE_PHASE_ADJUST 100
#define MIN_SCORE_PHASE_OVERRIDE 500
#define BEAT_AVG_FREQUENCY 5 // Once every 5 cycles, to spread out the bpm buffer reads

// -------------------------------------------------------------------------------------------------------------------------------------------------------

void setup_spectrum_shield() {
  // Set spectrum shield pin configurations
  pinMode(SS_PIN_STROBE, OUTPUT);
  pinMode(SS_PIN_RESET, OUTPUT);
  pinMode(SS_PIN_DC_ONE, INPUT);
  pinMode(SS_PIN_DC_TWO, INPUT);
  digitalWrite(SS_PIN_STROBE, HIGH);
  digitalWrite(SS_PIN_RESET, HIGH);

  // Initialize spectrum analyzers
  digitalWrite(SS_PIN_STROBE, LOW);
  delay(1);
  digitalWrite(SS_PIN_RESET, HIGH);
  delay(1);
  digitalWrite(SS_PIN_STROBE, HIGH);
  delay(1);
  digitalWrite(SS_PIN_STROBE, LOW);
  delay(1);
  digitalWrite(SS_PIN_RESET, LOW);
}

void loop_spectrum_shield() {
  read_frequencies();
  //readMSGEQ7();
  //set_peak_history();
  //process_peak_history();
  //trigger_beats();
  set_audio_hooks();
  detect_audio_changes();
  //detect_edm_events();
}

void setup()
{
  Serial.begin(115200);
  setup_spectrum_shield();
  debugToPC("Arduino Ready from ArduinoPC.ino");
}

void sendToPc()
{
  getSerialData();
  dataToPC();
}

void loop()
{
  current_time++;
  loop_spectrum_shield();
  //sendToPc();
  //printFreqs();
  //compareLoop();
}

void compareLoop()
{
  read_frequencies();
  //readMSGEQ7();
  //printFreqs();
  printFreqDiffs();
  //set_audio_hooks();
  //setAudioHooksSimple();
}

void read_frequencies() {
  //Read sound frequencies for each band and channel.
  //  Each channel has 7 frequency "buckets"  0 - 6.
  //  var frequencies_one[0] is channel 1 lowest frequency
  //  var frequencies_two[6] is channel 2 highest frequency

  for (int freq_amp = 0; freq_amp < NUM_CHANNELS; freq_amp++)
  {
    frequencies_one[freq_amp] = analogRead(SS_PIN_DC_ONE);
    delayMicroseconds(100);
    frequencies_two[freq_amp] = analogRead(SS_PIN_DC_TWO);
    delayMicroseconds(100);

    //    Serial.print("band[");
    //    Serial.print(freq_amp);
    //    Serial.print("] left: ");
    //    Serial.print( frequencies_one[freq_amp]);
    //    Serial.print("right: ");
    //    Serial.print(frequencies_two[freq_amp]);
    //    Serial.print("\n");

    frequencies_max[freq_amp] = frequencies_one[freq_amp] > frequencies_two[freq_amp] ? frequencies_one[freq_amp] : frequencies_two[freq_amp];
    //Serial.println(String(frequencies_max[freq_amp]));

    // Calibrate intensity
    if (frequencies_max[freq_amp] > NOISE_REDUCT) {
      frequencies_max[freq_amp] = frequencies_max[freq_amp] - NOISE_REDUCT;
    } else {
      frequencies_max[freq_amp] = 0;
    }

    // Acknowledging the read?
    digitalWrite(SS_PIN_STROBE, HIGH);
    delayMicroseconds(100);
    //delay(10);
    digitalWrite(SS_PIN_STROBE, LOW);
    //delay(10);
  }
}

void set_audio_hooks() {
  static bool downbeat_proximity_increasing = true;
  // downbeat proximity
  if (is_beat) {
    downbeat_proximity = downbeat_proximity_increasing ? 255 : 0;
    downbeat_proximity_increasing = !downbeat_proximity_increasing;
  } else {
    float progress = (float)(current_time - last_beat_trigger) / (float)(next_beat_prediction - last_beat_trigger);

#ifdef BEAT_HALF_TIME
    if (downbeat_proximity_increasing)
      downbeat_proximity = 255 * progress;
    else
      downbeat_proximity = 255 * (1 - progress);
#else
    if (progress <= 0.5)
      downbeat_proximity = 255 * (2 * progress);
    else
      downbeat_proximity = 255 * (1 - 2 * (progress - 0.5));
#endif
  }

  // Calc the loudest of the 7 channels
  for (uint8_t i = AUDIO_HOOK_HISTORY_SIZE - 1; i > 0; i--)
  {
    dominant_channel[i] = dominant_channel[i - 1];
    freqbandMode[i] = freqbandMode[i-1];
    overall_volume[i] = overall_volume[i - 1];
    band_distribution[i][0] = band_distribution[i - 1][0];
    band_distribution[i][2] = band_distribution[i - 1][1];
    band_distribution[i][1] = band_distribution[i - 1][2];
  }
  dominant_channel[0] = get_dominant_channel();
  overall_volume[0] = frequencies_max[0] + frequencies_max[1] + frequencies_max[2] + frequencies_max[3] + frequencies_max[4] + frequencies_max[5] + frequencies_max[6];

  if (overall_volume[0] == 0) {
    band_distribution[0][0] = 0;
    band_distribution[0][1] = 0;
    band_distribution[0][2] = 0;
  }
  else {
    //RJS: I find it weird that the output equals ~.85 - not the expected 1.0. Most of the time. This is why I am using a differnt calcuation in setAudioHooksSimple().
    band_distribution[0][0] = (float)(frequencies_max[0] + frequencies_max[1]) / overall_volume[0];
    band_distribution[0][1] = 0.66667 * (frequencies_max[2] + frequencies_max[3] + frequencies_max[4]) / overall_volume[0];
    band_distribution[0][2] = (float)(frequencies_max[5] + frequencies_max[6]) / overall_volume[0];
  }

  // Show which section of bass/mid/high is loudest
  low_band_emphasis = frequencies_max[0] >= frequencies_max[1] ? 0 : 1;
  high_band_emphasis = frequencies_max[5] >= frequencies_max[6] ? 0 : 1;
  if (frequencies_max[2] >= frequencies_max[3])
    mid_band_emphasis = frequencies_max[2] > frequencies_max[4] ? 0 : 2;
  else
    mid_band_emphasis = frequencies_max[3] > frequencies_max[4] ? 1 : 2;



#ifdef DEBUG_AUDIO_HOOKS
  printInfo = shouldPrint();
  if(printInfo)
  {
    String debugOutput = String(overall_volume[0]) + " | ";
    debugOutput += String(band_distribution[0][0], 2) + " (" + String(low_band_emphasis) + ")  "
                   + String(band_distribution[0][1], 2) + " (" + String(mid_band_emphasis) + ")  "
                   + String(band_distribution[0][2], 2) + " (" + String(high_band_emphasis) + ") | ";

    //for (uint16_t i = 0; i < downbeat_proximity / 10; i++)
      //debugOutput += ' ';
    debugOutput += dominant_channel[0];

    debugOutput += calc_freqband_mode();

    //Serial.println(debugOutput);
    char chars [60];
    debugOutput.toCharArray(chars, 60);
    debugToPC(chars);
    //byte bytes [16];
    //debugOutput.getBytes(bytes, 16);
    //Serial.write(bytes, 16);

    debugOutput = "";
  }
#endif
}

bool shouldPrint()
{
  uint16_t sum = 0;
  for (int i = 0; i < AUDIO_HOOK_HISTORY_SIZE; ++i)
  {
    sum |= overall_volume[i];
  }

  //Serial.println("sum! " + String(sum));

  if (sum < 16) //some noise thresholding...
    return false;

  return true;
}

String calc_freqband_mode()
{
  int history = FREQ_BAND_HISTORY_OF_INTEREST;
  uint8_t last [history];
  memcpy( last, dominant_channel, history*sizeof(uint8_t) );
  /*for(int i = 0; i < 4; i++)
  {
    Serial.println("i: " + String(i) + " " + String(last4[i]));
  }*/
  int mode_last = mode(last, history);
  freqbandMode[0] = mode_last;

  uint8_t last_modes [5];
  memcpy( last_modes, freqbandMode, 5*sizeof(uint8_t) );
  int mode_mode_last = mode(last_modes, 5);

  if(printInfo)
  {
    //return " mode " + String(mode_last) + " mode^2 " + String(mode_mode_last);
    return " mode " + String(mode_last);
  }

  return "";
}

void detect_audio_changes() {
  uint16_t amp_avg2 = (overall_volume[0] + overall_volume[1]) / 2;
  uint16_t amp_avg4 = amp_avg2 / 2 + (overall_volume[2] + overall_volume[3]) / 4;
  uint16_t amp_avg8 = amp_avg4 / 2 + (overall_volume[4] + overall_volume[5] + overall_volume[6] + overall_volume[7]) / 8;
  uint16_t amp_avg8_prior = (overall_volume[8] + overall_volume[9] + overall_volume[10] + overall_volume[11]
                                       + overall_volume[12] + overall_volume[13] + overall_volume[14] + overall_volume[15] ) / 8;
  uint16_t amp_avg16 = amp_avg8 / 2 + amp_avg8_prior / 2;

  //unsigned long endMs = millis();
  if(amp_avg16 > 10) //very low threshold due to high NOISE_REDUCTION value
  {
    if(isRecording == false)
    {
      startMs = millis();
      isRecording = true;
      char chars [20];
      String output = "start"; //[" + String(startMs) + "]";
      output.toCharArray(chars, 20);
      debugToPC(chars);
    }

    doFreqBandDetection();

    //DOING AMPLITUDE CHANGE DETECTION
    int diff = amp_avg8 - amp_avg8_prior;
    Serial.println("vol diff: " + String(diff));
    if(millis() - startMs > 250) //avoid 'event' at new track
    {
      if (abs(diff) > 1500) //major increase
      {
        String output = "@@ at " + String(millis()-startMs) + " " + String(diff) + " overall amplitude change MAJOR event";
        char chars [53];
        output.toCharArray(chars, 53);
        debugToPC(chars);
      }
      else if(abs(diff) > 700) //"minor" increase
      {
        String output = "@ at " + String(millis()-startMs) + " " + String(diff) + " overall amplitude change MINOR event";
        char chars [52];
        output.toCharArray(chars, 52);
        debugToPC(chars);
      }
    }
  }
  else
  {
    if(isRecording == true)
    {
      endMs = millis();
      isRecording = false;
      char chars [30];
      //String output = "end: [" + String(endMs) + "]\ntotalMs: [" + String(endMs-startMs) + "]";
      String output = "end\ntotalMs: [" + String(endMs-startMs) + "]";
      output.toCharArray(chars, 30);
      debugToPC(chars);
    }
  }
}

//RJS: only doing detetion now... maybe down the line add direction detection.
//i.e. if freq band mode goes up signal that for more informed animation param change
void doFreqBandDetection()
{
  int diff = freqbandMode[0] - freqbandMode[1];
  Serial.println("freq diff: " + String(diff));
  if(millis() - startMs > 350) //start of track compensation
  {
    if(abs(diff) > 2)
    {
      String output = "## at " + String(millis()-startMs) + " " + String(diff) + " overall freqband change MAJOR event";
      char chars [53];
      output.toCharArray(chars, 53);
      debugToPC(chars);
    }
    else if(abs(diff) > 0)
    {
      String output = "# at " + String(millis()-startMs) + " " + String(diff) + " overall freqband change MINOR event";
      char chars [52];
      output.toCharArray(chars, 52);
      debugToPC(chars);
    }
  }
}

//mode of vals 0-6
int mode(uint8_t array[], int length) {
    uint8_t modeMap [7];

    for(int i = 0; i < 7; i++)
    {
      modeMap[i] = 0;
    }

    uint8_t maxEl = array[0];
    int maxCount = 0;

    for (int i = 0; i < length; i++) {
        uint8_t el = array[i];
        //if (modeMap[el] == 0) {
          //  modeMap[el] = 1;
        //}
        //else {
            modeMap[el]++;
        //}

        if (modeMap[el] > maxCount) {

            maxEl = el;
            maxCount = modeMap[el];
            //Serial.println("new maxCount "  + String(maxCount) + " for " + String(el));
        }
    }
    return maxEl;
}
