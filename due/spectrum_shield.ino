inline void setup_spectrum_shield() {
  // Configure analogRead() to return 8-bit values
  analogReadResolution(8);
  
  // Set spectrum shield pin configurations
  pinMode(SS_PIN_STROBE, OUTPUT);
  pinMode(SS_PIN_RESET, OUTPUT);
  pinMode(SS_PIN_DC_ONE, INPUT);
  pinMode(SS_PIN_DC_TWO, INPUT);
  
  digitalWrite(SS_PIN_STROBE, HIGH);
  //delayMicroseconds(100);
  digitalWrite(SS_PIN_RESET, HIGH);
  //delayMicroseconds(100);
  
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

// Read sound frequencies for each band and channel. Each channel has 7 frequency "buckets"  0 - 6.
// freq_internal[0] is channel 1 lowest frequency, freq_external[6] is channel 2 highest frequency
#define NOISE_REDUCTION 30
#define FREQ_HISTORY_SIZE 3
uint8_t freq_history[FREQ_HISTORY_SIZE];
inline void read_frequencies() {

  for(uint8_t i = 0; i < NUM_CHANNELS; i++)
  {
    freq_internal[i] = analogRead(SS_PIN_DC_ONE);
    delayMicroseconds(25);
    freq_external[i] = analogRead(SS_PIN_DC_TWO);
    delayMicroseconds(25);
    
    #if NOISE_REDUCTION > 0
      freq_internal[i] = freq_internal[i] <= NOISE_REDUCTION ? 0 : freq_internal[i] - NOISE_REDUCTION;
      freq_external[i] = freq_external[i] <= NOISE_REDUCTION ? 0 : freq_external[i] - NOISE_REDUCTION;
    #endif

    // Smooth out values with a rolling average
    uint8_t max_in_history = 0;
    uint16_t history_sum = 0;
    for(uint8_t j = FREQ_HISTORY_SIZE-1; j > 0; j--) {
      freq_history[j] = freq_history[j-1];
      history_sum += freq_history[j];
      if(freq_history[j] > max_in_history) { max_in_history = freq_history[j]; }
    }
    freq_history[0] = freq_internal[i];

    // If we get a peak, jump upward (so values will be upwardly flexible but decay slower downward)
    if(freq_internal[i] >= max_in_history) { freq_smooth[i] = freq_internal[i]; }
    else { freq_smooth[i] = (history_sum + freq_history[0]) / FREQ_HISTORY_SIZE; }
    
    // Move to next channel
    digitalWrite(SS_PIN_STROBE, HIGH);
    delayMicroseconds(10);
    digitalWrite(SS_PIN_STROBE, LOW);
    delayMicroseconds(10);
  }
}

