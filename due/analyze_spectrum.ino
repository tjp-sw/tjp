// ---- This is all under development ----

const uint8_t peakThresholds[NUM_CHANNELS] = { 90, 100, 110, 120, 130, 140, 150 }; // Weighted toward low bands

#define SS_READ_WINDOW_SIZE 5
uint8_t last_read[NUM_CHANNELS][SS_READ_WINDOW_SIZE];
unsigned long last_read_time[SS_READ_WINDOW_SIZE];

#define PEAK_HISTORY_SIZE 8
unsigned long peak_history[NUM_CHANNELS][PEAK_HISTORY_SIZE];
uint8_t peakConsistencies[NUM_CHANNELS];

void update_beat_detection_new() {
  set_peak_history();
  set_peak_consistencies();
}


void set_peak_history() {
  // Update read times; same for all channels
  for(uint8_t j = SS_READ_WINDOW_SIZE-1; j > 0; j--)
  {
    last_read_time[j] = last_read_time[j-1];
  }
  last_read_time[0] = current_time;
  
  for(uint8_t i = 0; i < NUM_CHANNELS; i++)
  {
    // Update read values
    for(uint8_t j = SS_READ_WINDOW_SIZE-1; j > 0 ; j--)
    {
      last_read[i][j] = last_read[i][j-1];
    }
    last_read[i][0] = frequencies_avg[i];
  
    // Check for a peak in the center of the window; using magic numbers for speed; hard-coded to 5
    if(last_read[i][2] > last_read[i][0] && last_read[i][2] > last_read[i][1]
    && last_read[i][2] > last_read[i][3] && last_read[i][2] > last_read[i][4])
    {
      // Before recording it as a peak, verify that it is 33% greater than the average
      uint16_t average = (last_read[i][0] + last_read[i][1] + last_read[i][3] + last_read[i][4]) / 4;
      if(last_read[i][2] > average * 4 / 3) {
        for(uint8_t j = PEAK_HISTORY_SIZE-1; j > 0; j--)
        {
          peak_history[i][j] = peak_history[i][j-1];
        }
        peak_history[i][0] = current_time;
      }
    }
  }
}

#define minBeatDelta 420
#define maxBeatDelta 839
void set_peak_consistencies() {
  for(uint8_t i = 0; i < NUM_CHANNELS; i++)
  {
    // For each channel, predict the beat by:
    //  1. calculate delta between each beat
    //  2. move into min/max BeatDelta range
    //  3. check for values with peaks on most beats

    unsigned long totalDelta = 0;
    uint8_t deltaCount = 0;
    unsigned long beat0 = 0;
    while(beat0 < PEAK_HISTORY_SIZE - 1)
    {
      unsigned long beat1 = beat0+1;
      while(beat1 < PEAK_HISTORY_SIZE)
      {
        unsigned long delta = peak_history[beat1] - peak_history[beat0];
        while(delta < minBeatDelta)
          delta *= 2;
        while(delta > maxBeatDelta)
          delta /= 2;

        
      }
    }



        
    //  1. Compare pairs of peaks (01, 02, 14, ...) to get predicted beat rate
    //  2. If prediction is out of allowable range (~70-140 bpm?), break loop
    //  3. Check if future predictions hold. Make prediction with highest % correct.
    

    
    unsigned long totalLength = peak_history[i][0] - peak_history[i][PEAK_HISTORY_SIZE-1];
    uint16_t avgBeatLength = totalLength / (PEAK_HISTORY_SIZE - 1);
    uint16_t minBeatLength = avgBeatLength * 9/10; // +/- 10%
    uint16_t maxBeatLength = avgBeatLength * 11/10;
    
    for(uint8_t j = 1; j < PEAK_HISTORY_SIZE; j++)
    {
      unsigned long delta = peak_history[i][j] - peak_history[i][j-1];
      if(delta >= minBeatLength && delta <= maxBeatLength)
        peakConsistencies[i]++;
    }
  }
}

/*
// Track levels, capture peak volume, then trigger beat after # samples < peak volume
uint8_t last_peak[NUM_CHANNELS];
unsigned long last_peak_time[NUM_CHANNELS];
uint8_t decreasing_read_count[NUM_CHANNELS] = { 0, 0, 0, 0, 0, 0, 0 };
void update_beat_detection_decreasing() {

  for(uint8_t i = 0; i < NUM_CHANNELS; i++)
  {
    if(frequencies_avg[i] > last_peak[i]) {
      last_peak[i] = frequencies_avg[i];
      last_peak_time[i] = current_time;
      decreasing_read_count[i] = 0;
    }
    else {
      decreasing_read_count[i]++;
      if(decreasing_read_count[i] >= NUM_DECREASING_READS) {
        is_beat[i] = true;
        last_peak[i] = 0;
        decreasing_read_count[i] = 0;
      }
    }
  }
}
*/
/*
// Try to predict the next beat
int space_between_beats[NUM_CHANNELS];
int space_weight = 0;
const int max_space_weight = 6;
const int min_time_between_beats = 250;
void update_beat_detection_Smart() {
  
  update_beat_detection_decreasing();
  for(uint8_t i = 0; i < NUM_CHANNELS; i++)
  {
    if(is_beat[i]) {
      // Beat will be triggered late, but last_peak_time will be accurate.
      if(current_time - last_peak_time[i] > min_time_between_beats) {
        int space = current_time - last_peak_time[i];
        space_between_beats[i] = (space + space_weight * space_between_beats[i]) / (space_weight + 1);
      }
      else {
        // Too soon to be another beat
        is_beat[i] = false;
      }
    }
    
    if(space_weight < max_space_weight) {
      space_weight++;
    }
    else if(!is_beat[i]){
      if(current_time >= last_peak[i] + space_between_beats[i])
        is_beat[i] = true;
    }
  }
}


// Keep track of spectrum reads to track changes
const uint8_t num_spectrum_reads = 10;
uint8_t spectrum_reads[NUM_CHANNELS][num_spectrum_reads];
uint8_t spectrum_read_avg[NUM_CHANNELS];
uint8_t spectrum_read_index = 0;
void update_spectrum_emphasis() {
  
  for(uint8_t i = 0; i < NUM_CHANNELS; i++)
  {
    // Update read and average
    long spectrum_sum = frequencies_avg[i];
    for(uint8_t j = 0; j < num_spectrum_reads; j++)
      spectrum_sum += spectrum_reads[i][j];
    spectrum_read_avg[i] = spectrum_sum / (num_spectrum_reads + 1);
    
    spectrum_reads[i][spectrum_read_index] = frequencies_avg[i];    
  }

  // Set band emphasis based on last several reads
  low_band_emphasis = spectrum_read_avg[0] > spectrum_read_avg[1] ? 0 : 1;
  high_band_emphasis = spectrum_read_avg[5] > spectrum_read_avg[6] ? 0 : 1;
  if(spectrum_read_avg[2] > spectrum_read_avg[3]) {
    mid_band_emphasis = spectrum_read_avg[2] > spectrum_read_avg[4] ? 0 : 2;
  } else {
    mid_band_emphasis = spectrum_read_avg[3] > spectrum_read_avg[4] ? 1 : 2;
  }

  // Update cyclical index
  if(++spectrum_read_index >= num_spectrum_reads)
    spectrum_read_index = 0;
}
*/
