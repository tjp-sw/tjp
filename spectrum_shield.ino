/* To do:
 * Test when peak history is changed to time the read occured, not current_time
 * Test shorter delays in read_frequencies
 * Change delta from unsigned long to uint16_t?
 * Test predictionScore as uint16_t; make sure it doesn't ever overflow in the middle of a calculation
 * In set_audio_hooks(): use a rolling index instead of copying all of the arrays each cycle
 * Do frequencies[] arrays need to be ints? Would be better as uint8_t, scaling or capping if necessary.
 */

// ----------------------------------------- Peak/Beat detection ----------------------------------------------------
#define NOISE_REDUCT 40 // Fixed amount to reduce each spectrum shield read by
#define PEAK_WINDOW_SIZE 5 // Size of window for determining peaks, center must be maximum for peak to be marked
#define PEAK_HISTORY_SIZE 10 // Number of peaks to remember for beat detection

#define PEAK_AVG_FACTOR 1 // Peaks must be this much greater than the average of nearby points, after noise reduction
#define PEAK_HURDLE_SIZE 70 // Peaks must additionally be this fixed amount greater than average of nearby points
#define PEAK_TOLERANCE 55 // How far peaks can drift (per beat) and still be considered a consistent beat
#define BEAT_PREDICT_TOLERANCE 30 // Max ms a beat will trigger early (should be ~1/2 of refresh time)

#define MIN_BEAT_DELTA 400 // 150 BPM
#define MAX_BEAT_DELTA 799 // >75 BPM

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

  // Initialize spectrum analyzers
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
  set_peak_history();
  process_peak_history();
  trigger_beats();
  set_audio_hooks();
  //detect_edm_events();
}
  
void read_frequencies() {
  //Read sound frequencies for each band and channel.
  //  Each channel has 7 frequency "buckets"  0 - 6.
  //  var frequencies_one[0] is channel 1 lowest frequency
  //  var frequencies_two[6] is channel 2 highest frequency
  
  for(uint8_t i = 0; i < NUM_CHANNELS; i++)
  {
    frequencies_one[i] = analogRead(SS_PIN_DC_ONE);
    delayMicroseconds(100);
    frequencies_two[i] = analogRead(SS_PIN_DC_TWO);
    delayMicroseconds(100);

    // Acknowledging the read?
    digitalWrite(SS_PIN_STROBE, HIGH);
    delayMicroseconds(100);
    //delay(10);
    digitalWrite(SS_PIN_STROBE, LOW);
    //delay(10);
  }

  for(uint8_t i = 0; i < NUM_CHANNELS; i++) {
    frequencies_max[i] = frequencies_one[i] > frequencies_two[i] ? frequencies_one[i] : frequencies_two[i];
      
    // Calibrate intensity
    if (frequencies_max[i] > NOISE_REDUCT){
      frequencies_max[i] = frequencies_max[i] - NOISE_REDUCT;
    } else {
     frequencies_max[i] = 0;
    }
  }

  scale_frequencies();
}

void set_peak_history() {
  #ifdef DEBUG_PEAKS
    String debugOutput = String(current_time) + "\t";
  #endif

  // Update read times; same for all channels
  for(uint8_t i = PEAK_WINDOW_SIZE-1; i > 0; i--)
  {
    last_read_time[i] = last_read_time[i-1];
  }
  last_read_time[0] = current_time;
  
  // Update read values and mark peaks for each channel, peaks are detected a couple cycles after they happen
  for(uint8_t i = 0; i < NUM_CHANNELS; i++)
  {
    // Update read values
    for(uint8_t j = PEAK_WINDOW_SIZE-1; j > 0 ; j--)
    {
      read_history[i][j] = read_history[i][j-1];
    }
    read_history[i][0] = frequencies_max[i];


    // Check for a peak in the center of the window; using magic numbers for speed
    // Additionally require the read be some level above the average of the other samples
    #if PEAK_WINDOW_SIZE == 5
      uint16_t average = (read_history[i][0] + read_history[i][1] + read_history[i][3] + read_history[i][4]) / 4;
      if(read_history[i][PEAK_WINDOW_SIZE/2] > PEAK_AVG_FACTOR*average + PEAK_HURDLE_SIZE
      && read_history[i][2] >= read_history[i][0] && read_history[i][2] >= read_history[i][1]
      && read_history[i][2] >= read_history[i][3] && read_history[i][2] >= read_history[i][4])
    #elif PEAK_WINDOW_SIZE == 7
      uint16_t average = (read_history[i][0] + read_history[i][1] + read_history[i][2] + read_history[i][4] + read_history[i][5] + read_history[i][6]) / 6;
      if(read_history[i][PEAK_WINDOW_SIZE/2] > PEAK_AVG_FACTOR*average + PEAK_HURDLE_SIZE
      && read_history[i][3] >= read_history[i][0] && read_history[i][3] >= read_history[i][1]
      && read_history[i][3] >= read_history[i][2] && read_history[i][3] >= read_history[i][4]
      && read_history[i][3] >= read_history[i][5] && read_history[i][3] >= read_history[i][6])
    #endif
    {    
      for(uint8_t j = PEAK_HISTORY_SIZE-1; j > 0; j--)
      {
        peak_history[i][j] = peak_history[i][j-1];
      }
      // This SHOULD be last_read_time[2], but since these tend to fire a little early anyway, this is fine.
      peak_history[i][0] = current_time;//last_read_time[PEAK_WINDOW_SIZE/2];
        
      #ifdef DEBUG_PEAKS
        debugOutput += "X ";
      #endif
    }
    else {
      #ifdef DEBUG_PEAKS
        debugOutput += "- ";
      #endif
    }
  }

  #ifdef DEBUG_PEAKS
    Serial.println(debugOutput);
  #endif
}

void process_peak_history() {
  // For each channel, calculate delta between each peak then check if future predictions hold.
  // Create a score for how well a pair of points predicts, and pass the best prediction to bpm/phase prediction functions
  
  static uint8_t best_channel;
  static uint16_t best_delta;
  static uint32_t best_score;
  static unsigned long best_last_beat;

  for(uint8_t i = 0; i < NUM_CHANNELS; i++)
  {
    for(uint8_t beat0 = PEAK_HISTORY_SIZE - 1; beat0 > 1; beat0--)
    {
      if(peak_history[i][beat0] == 0)
        continue;

      for(uint8_t beat1 = beat0 - 1; beat1 > 0; beat1--)
      {
        // For each pair of beats (starting at oldest/highest indexes), shift into allowable range, but max once
        unsigned long delta = peak_history[i][beat1] - peak_history[i][beat0];
        if(delta < MIN_BEAT_DELTA) delta *= 2;
        if(delta < MIN_BEAT_DELTA) continue;
        if(delta > MAX_BEAT_DELTA) delta /= 2;
        if(delta > MAX_BEAT_DELTA) continue;

        uint8_t last_beat = 0;
        uint32_t predictionScore = score_prediction(i, beat1, delta, &last_beat);

        // Store best score from any channel for this cycle
        if(predictionScore > best_score) {
          best_score = predictionScore;
          best_last_beat = peak_history[i][last_beat];
          best_delta = delta;
          best_channel = i;
        }
      }
    }
  }

  // Update with best prediction across BEAT_AVG_FREQUENCY cycles
  if(loop_count % BEAT_AVG_FREQUENCY == BEAT_AVG_FREQUENCY-1) {
    if(best_score > MIN_SCORE_BPM_ADJUST)
      update_bpm_prediction(60000 / best_delta, best_score);
    else
      bpm_confidence = qsub8(bpm_confidence, 1);
       
    if(best_score > MIN_SCORE_PHASE_ADJUST)
      update_beat_phase(best_last_beat, best_delta, best_score);
  }
}

uint32_t score_prediction(uint8_t chan, uint8_t beat1, unsigned long delta, uint8_t *last_beat) {
  unsigned long prediction = peak_history[chan][beat1] + delta;
  uint8_t curPeak = beat1 - 1;
  uint8_t hitCount = 0, maxCount = 0;

  // Make predictions until prediction is later than the last peak
  while(prediction <= peak_history[chan][0] + PEAK_TOLERANCE)
  {
    // Move through beats until prediction is in range or earlier than beat
    while(prediction > peak_history[chan][curPeak] + PEAK_TOLERANCE)
    {
      curPeak--;
    }

    // Check if prediction = peak_history +/- PEAK_TOLERANCE
    if(prediction >= peak_history[chan][curPeak] - PEAK_TOLERANCE && prediction <= peak_history[chan][curPeak] + PEAK_TOLERANCE) {
      hitCount++;
      *last_beat = curPeak;
    }
      
    // Move to next prediction (because prediction's time < current peak's time)
    maxCount++;
    prediction = peak_history[chan][curPeak] + delta;
  }
  
  // Increment maxCount for each beat that was expected after the latest peak and definitely missed (this will cause old peaks to decay even if no new ones appear)
  while(prediction < current_time - PEAK_TOLERANCE)
  {
    maxCount++;
    prediction += delta;
  }

  if(maxCount == 0) return 0;
  else return 100*(hitCount*hitCount*hitCount) / (maxCount*maxCount);
  // Example scores:
  // (No miss) = 1/1 = 100; 2/2 = 200; 3/3 = 300;
  // (1 miss) =  1/2 = 25;  2/3 = 88;  3/4 = 168; 4/5 = 256
  // (2 miss) =  1/3 = 11;   2/4 = 50;  3/5 = 108; 4/6 = 177; 5/7 = 255
}

// This function is more art than science. Magic numbers galore!
void update_bpm_prediction(uint8_t bpm, uint32_t predictionScore) {
  static uint8_t bpm_weight_history[BPM_HISTORY_SIZE];
/*
  // bpm clamping; 2 thresholds, 2 methods. Quickly decay the confidence when doing this, since it should only be used
  // to weed out a small percentage of repeated inaccurate reads (e.g. a melody with a consistent beat at 3/4 bpm)
  if(bpm_confidence >= 12) {
    // Exclude extraneous readings but ding the beat_confidence
    float maxDiff;
    if(bpm_confidence >= 30)
      maxDiff = 0.06;
    else
      maxDiff = 0.35 - 0.01*bpm_confidence;
    
    if(bpm < bpm_estimate * (1 - maxDiff) || bpm > bpm_estimate * (1 + maxDiff)) {
      bpm_confidence -= bpm_confidence/10;
      return;
    }
  }
  else if(bpm_confidence > 5){
    // Hedge against incorrect predictions, results in a gradual change in bpm
    if(bpm > bpm_estimate + 15)
      bpm = bpm_estimate + 15;
    else if(bpm < bpm_estimate - 15)
      bpm = bpm_estimate - 15;
  }
*/
  uint32_t bpm_sum = 0;
  uint16_t count = 0;

  // Shift bpm history. Sum and decrease old weights at the same time
  for(uint8_t i = BPM_HISTORY_SIZE-1; i > 0; i--) {
    bpm_history[i] = bpm_history[i-1];
    if(i % CONFIDENCE_DECAY_RATE == 0 && bpm_weight_history[i-1] > 1)
      bpm_weight_history[i] = bpm_weight_history[i-1] - 1;
    else
      bpm_weight_history[i] = bpm_weight_history[i-1];
    
    if(bpm_history[i] > 0) {
      bpm_sum += bpm_history[i] * bpm_weight_history[i];
      count += bpm_weight_history[i];
    }
  }
  bpm_history[0] = bpm;
  bpm_weight_history[0] = predictionScore/100 - 1;
  
  bpm_sum += bpm_weight_history[0] * bpm;
  count += bpm_weight_history[0];

  bpm_estimate = (float)bpm_sum / (float)count;

  // Haven't tried this yet but should. Would prevent thrashing from 0 to 10+ and triggering bpm clamping.
  //if(bpm_confidence < 5)
  //  bpm_confidence = count/2;
  //else
    bpm_confidence = count;

  #ifdef DEBUG_BPM
    Serial.print("BPM: " + String(bpm_estimate) + " Score: " + String(count));
    Serial.println(" | Added: " + String(bpm) + " Val:" + String(predictionScore));
  #endif
  
/*
  if(bpm_confidence < 2) {
    bpm_estimate = bpm;
    bpm_confidence = predictionScore / 100;
    is_beat = true;
  }
  else {
    if(bpm < bpm_estimate * 6/10)
      bpm *= 2;
    if(bpm > bpm_estimate * 6/5)
      bpm /= 2;
    
    if(bpm > bpm_estimate*9/10 && bpm < bpm_estimate*11/10) {
      // Matches the current estimate within 20%
      bpm_estimate = (bpm + bpm_estimate * bpm_confidence) / (bpm_confidence + 1);
      if(bpm_confidence < 10)
        bpm_confidence++;
    }
    else {
      // Doesn't match estimate very well
      if(predictionScore >= 200) {
        // New prediction seems credible
        bpm_confidence--;

        bpm_estimate = (bpm + bpm_estimate * bpm_confidence) / (bpm_confidence + 1);
        if(bpm < bpm_estimate)
          bpm_estimate--;
        else
          bpm_estimate++;
      }
    }

    #if defined(DEBUG_BPM) || defined(DEBUG_AUDIO_HOOKS)
      Serial.print("BPM: " + String(bpm_estimate));
      Serial.println(" (Confidence " + String(bpm_confidence) + ")");
    #endif
  }*/

  next_beat_prediction = last_beat_trigger + 60000/bpm_estimate;
}

void update_beat_phase(unsigned long last_beat, uint16_t delta, uint32_t predictionScore) {
  if(next_beat_prediction == 0) {
    if(bpm_estimate > 0)
      next_beat_prediction = current_time + 60000 / bpm_estimate;
  }
  else if(predictionScore >= MIN_SCORE_PHASE_OVERRIDE) {
    next_beat_prediction = last_beat_trigger + delta;
  }

  // Nudge beat phase
  uint16_t changeAmount = predictionScore / 200;
  unsigned long prediction = last_beat;
  while(prediction < current_time - BEAT_PREDICT_TOLERANCE)
    prediction += delta;
    
  if(prediction + changeAmount < next_beat_prediction) {
    next_beat_prediction -= changeAmount;
    #ifdef DEBUG_BPM
      Serial.println("Phase adjust: -" + String(changeAmount));
    #endif
  }
  else if(prediction > next_beat_prediction + changeAmount) {
    next_beat_prediction += changeAmount;
    #ifdef DEBUG_BPM
      Serial.println("Phase adjust: +" + String(changeAmount));
    #endif
  }
}

void trigger_beats() {
  is_beat = current_time >= next_beat_prediction - BEAT_PREDICT_TOLERANCE && next_beat_prediction > 0;
  if(is_beat) {
    last_beat_trigger = current_time;
    next_beat_prediction = current_time + 60000 / bpm_estimate;
  
    #if defined(DEBUG_BPM) || defined(DEBUG_AUDIO_HOOKS)
      Serial.println("------------------ BEAT (bpm:" + String(bpm_estimate) + " confidence:" + String(bpm_confidence) + ") ------------------");
    #endif
  }
}

void set_audio_hooks() {
  static bool downbeat_proximity_increasing = true;
  // downbeat proximity
  if(is_beat) {
    downbeat_proximity = downbeat_proximity_increasing ? 255 : 0;
    downbeat_proximity_increasing = !downbeat_proximity_increasing;
  } else {
    float progress = (float)(current_time - last_beat_trigger) / (float)(next_beat_prediction - last_beat_trigger);

    #ifdef BEAT_HALF_TIME
      if(downbeat_proximity_increasing)
        downbeat_proximity = 255 * progress;
      else
        downbeat_proximity = 255 * (1-progress);
    #else
      if(progress <= 0.5)
        downbeat_proximity = 255 * (2 * progress);
      else
        downbeat_proximity = 255 * (1 - 2*(progress - 0.5));
    #endif  
  }

  // Calc the loudest of the 7 channels
  for(uint8_t i = AUDIO_HOOK_HISTORY_SIZE - 1; i > 0; i--)
  {
    dominant_channel[i] = dominant_channel[i-1];
    overall_volume[i] = overall_volume[i-1];
    band_distribution[i][0] = band_distribution[i-1][0];
    band_distribution[i][2] = band_distribution[i-1][1];
    band_distribution[i][1] = band_distribution[i-1][2];
  }
  dominant_channel[0] = get_dominant_channel();
  overall_volume[0] = frequencies_max[0] + frequencies_max[1] + frequencies_max[2] + frequencies_max[3] + frequencies_max[4] + frequencies_max[5] + frequencies_max[6];

  if(overall_volume[0] == 0) {
    band_distribution[0][0] = 0;
    band_distribution[0][1] = 0;
    band_distribution[0][2] = 0;
  }
  else {
    band_distribution[0][0] = (float)(frequencies_max[0] + frequencies_max[1]) / overall_volume[0];
    band_distribution[0][1] = 0.66667*(frequencies_max[2] + frequencies_max[3] + frequencies_max[4]) / overall_volume[0];
    band_distribution[0][2] = (float)(frequencies_max[5] + frequencies_max[6]) / overall_volume[0];
  }

  // Show which section of bass/mid/high is loudest
  low_band_emphasis = frequencies_max[0] >= frequencies_max[1] ? 0 : 1;
  high_band_emphasis = frequencies_max[5] >= frequencies_max[6] ? 0 : 1;
  if(frequencies_max[2] >= frequencies_max[3])
    mid_band_emphasis = frequencies_max[2] > frequencies_max[4] ? 0 : 2;
  else
    mid_band_emphasis = frequencies_max[3] > frequencies_max[4] ? 1 : 2;
  


  #ifdef DEBUG_AUDIO_HOOKS
    String debugOutput = String(overall_volume[0]) + " | ";
    debugOutput += String(band_distribution[0][0], 2) + " (" + String(low_band_emphasis) + ")  " 
                 + String(band_distribution[0][1], 2) + " (" + String(mid_band_emphasis) + ")  "
                 + String(band_distribution[0][2], 2) + " (" + String(high_band_emphasis) + ")\t";

    for(uint16_t i = 0; i < downbeat_proximity/10; i++)
      debugOutput += ' ';
    debugOutput += dominant_channel[0];
    
    Serial.println(debugOutput);
    debugOutput = "";
  #endif
}

uint8_t get_dominant_channel() {
  uint8_t retVal = 0;
  for(uint8_t i = 1; i < NUM_CHANNELS; i++)
  {
    if(frequencies_max[i] > frequencies_max[retVal])
      retVal = i;
  }
  return retVal;
}

// This is 100% untested, and more just a way to put down my thoughts on how it could be done.
// At some point soon I'll run this and see what it does, edits/suggestions welcomed!
void detect_edm_events() {
  const uint8_t MAX_CUTOFF_CYCLES = 50;
  static uint8_t cutoff_cycles = 0;
  static uint16_t fade_only_cutoff = 0;

  static uint16_t drop_cutoff = 0xFFFF;
  static bool drop_active = false;
  static bool here_comes_a_big_drop = false;

  static uint8_t cycles_with_low_beat_confidence = 0;
  static uint8_t cycles_building = 0;
  
  uint16_t amp_avg2 = (overall_volume[0] + overall_volume[1]) / 2;
  uint16_t amp_avg4 = amp_avg2 / 2 + (overall_volume[2] + overall_volume[3]) / 4;
  uint16_t amp_avg8 = amp_avg4 / 2 + (overall_volume[4] + overall_volume[5] + overall_volume[6] + overall_volume[7]) / 8;
  uint16_t amp_avg16 = amp_avg8 / 2 + (overall_volume[8] + overall_volume[9] + overall_volume[10] + overall_volume[11]
                                    + overall_volume[12] + overall_volume[13] + overall_volume[14] + overall_volume[15] ) / 16;

  // Watch for sudden decreases in overall volume
  if(overall_volume[0] <= fade_only_cutoff) {
    // Activate fade only mode
    if(++cutoff_cycles < MAX_CUTOFF_CYCLES) {
      if(cycles_building > 20) {
        here_comes_a_big_drop = true;
        Serial.println("------------------------------------------------------------");
        Serial.println("---------------------- Build --> cutoff --------------------");
        Serial.println("------------------------------------------------------------");
      }
      else {
        Serial.println("------------------------------------------------------------");
        Serial.println("------------------------ Sudden cutoff ---------------------");
        Serial.println("------------------------------------------------------------");
      }
    }
    else {
      // Slowly fade in a new animation
      Serial.println("------------------------------------------------------------");
      Serial.println("------------------- Bring in new animation -----------------");
      Serial.println("------------------------------------------------------------");
      cutoff_cycles = 0;
      fade_only_cutoff = amp_avg16 / 3;
    }
  }
  else {
    fade_only_cutoff = amp_avg16 / 3;
  }


  // Watch for sudden increases in overall volume
  if(amp_avg2 >= drop_cutoff || amp_avg4 >= drop_cutoff) {
    // Drop detected
    drop_active = true;
    drop_cutoff = amp_avg16 * 2;

    if(dominant_channel[0] == dominant_channel[1]) {
      if(dominant_channel[0] == 0) {
        // Draw an exciting overlay effect as long as this is hit
        if(here_comes_a_big_drop) {
          Serial.println("------------------------------------------------------------");
          Serial.println("------------------- Build -> cutoff -> drop ----------------");
          Serial.println("------------------------------------------------------------");
        }
        
        if(bpm_confidence >= 15) {
          // Bass drop with good tempo
          Serial.println("------------------------------------------------------------");
          Serial.println("-------------------- Bass drop, good tempo -----------------");
          Serial.println("------------------------------------------------------------");
        }
        else {
          // Bass drop weak tempo estimate
          Serial.println("------------------------------------------------------------");
          Serial.println("-------------------- Bass drop, weak tempo -----------------");
          Serial.println("------------------------------------------------------------");
        }
      }
      else if(dominant_channel[0] == 1) {
        if(bpm_confidence >= 15) {
          // Mid drop with good tempo
          Serial.println("------------------------------------------------------------");
          Serial.println("--------------------- Mid drop, good tempo -----------------");
          Serial.println("------------------------------------------------------------");
        }
        else {
          // Mid drop weak tempo estimate
          Serial.println("------------------------------------------------------------");
          Serial.println("-------------------- Mid drop, weak tempo ------------------");
          Serial.println("------------------------------------------------------------");
        }
      }
      else {
        if(bpm_confidence >= 15) {
          // High band drop with good tempo
          Serial.println("------------------------------------------------------------");
          Serial.println("-------------------- High drop, good tempo -----------------");
          Serial.println("------------------------------------------------------------");
        }
        else {
          // High band drop weak tempo estimate
          Serial.println("------------------------------------------------------------");
          Serial.println("-------------------- High drop, weak tempo -----------------");
          Serial.println("------------------------------------------------------------");
        }
      }
    }
    else {
      // No clear dominant channel
      Serial.println("------------------------------------------------------------");
      Serial.println("--------------------- Multi-channel drop -------------------");
      Serial.println("------------------------------------------------------------");
    }
  }
  else {
    drop_active = false;
    drop_cutoff = amp_avg16 < amp_avg8 ? amp_avg16 * 2.5 : amp_avg8 * 2.5;
  }


  // Detect "tempoless" state (might just be hard to hear the beat)
  if(bpm_confidence < 10) {
    if(++cycles_with_low_beat_confidence == 20) {
      Serial.println("------------------------------------------------------------");
      Serial.println("----------------- Begin tempo-less animation ---------------");
      Serial.println("------------------------------------------------------------");
    }
  }
  else if(bpm_confidence >= 16) {
    cycles_with_low_beat_confidence = 0;
    Serial.println("------------------------------------------------------------");
    Serial.println("--------------- Resume tempo-sync'd animations -------------");
    Serial.println("------------------------------------------------------------");
  }

  
  // Detect high-pass filter
  if(band_distribution[0][2] + band_distribution[1][2] + band_distribution[2][2] + band_distribution[3][2] + band_distribution[4][2] + 
     band_distribution[5][2] + band_distribution[6][2] + band_distribution[7][2] + band_distribution[8][2] + band_distribution[9][2] > 5) {

     // At least 50% of volume coming from high band over the last 10 cycles (.5-.6 seconds)
     Serial.println("------------------------------------------------------------");
     Serial.println("---------------------- High pass filter --------------------");
     Serial.println("------------------------------------------------------------");
  }

  // Detect low-pass filter
  if(band_distribution[0][0] + band_distribution[1][0] + band_distribution[2][0] + band_distribution[3][0] + band_distribution[4][0] + 
     band_distribution[5][0] + band_distribution[6][0] + band_distribution[7][0] + band_distribution[8][0] + band_distribution[9][0] > 5) {

     // At least 50% of volume coming from low band over the last 10 cycles (.5-.6 seconds)
     Serial.println("------------------------------------------------------------");
     Serial.println("----------------------- Low pass filter --------------------");
     Serial.println("------------------------------------------------------------");
  }
  

  // Detect long builds with a consistent beat
  if(bpm_confidence >= 14 && band_distribution[0][0] < .2) {
    if(++cycles_building > 30) {
      Serial.println("------------------------------------------------------------");
      Serial.println("------------------------ Build active ----------------------");
      Serial.println("------------------------------------------------------------");
    }
  }
  else {
    cycles_building = 0;
  }
}



// Max out each frequency at 255
void scale_frequencies() {
  /*
  if(overall_volume[AUDIO_HOOK_HISTORY_SIZE-1] == 0) {
    for(uint8_t i = 0; i < NUM_CHANNELS; i++) {
      if(frequencies_one[i] > 255)
        frequencies_one[i] = 255;
      if(frequencies_two[i] > 255)
        frequencies_two[i] = 255;
      if(frequencies_max[i] > 255)
        frequencies_max[i] = 255;
    }
    return;
  }
    
  uint16_t avg_volume = 0;
  for(uint16_t i = 0; i < AUDIO_HOOK_HISTORY_SIZE; i++) {
    avg_volume += overall_volume[i];
  }
  avg_volume /= AUDIO_HOOK_HISTORY_SIZE;

  
  uint16_t fullBlast = 2 * (avg_volume / NUM_CHANNELS);
  for(uint8_t i = 0; i < NUM_CHANNELS; i++) {
    if(frequencies_one[i] > fullBlast)
    frequencies_one[i] = scalingFactor * frequencies_one[i];
    frequencies_two[i] = scalingFactor * frequencies_two[i];
    frequencies_max[i] = scalingFactor * frequencies_max[i];
  }
*/

  for(uint8_t i = 0; i < NUM_CHANNELS; i++) {
    frequencies_max[i] /= 2;
    if(frequencies_max[i] > 255)
      frequencies_max[i] = 255;
  }
  
  #ifdef DEBUG_FREQUENCIES
    String debugOutput = "";
    for(uint8_t i = 0; i < NUM_CHANNELS; i++) {
      debugOutput += String(frequencies_max[i]) + ", ";
    }
    Serial.println(debugOutput);
  #endif
}

