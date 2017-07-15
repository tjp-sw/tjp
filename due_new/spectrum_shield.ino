void setup_spectrum_shield() {
  // Set spectrum shield pin configurations
  pinMode(SS_PIN_STROBE, OUTPUT);
  pinMode(SS_PIN_RESET, OUTPUT);
  pinMode(SS_PIN_DC_ONE, INPUT);
  pinMode(SS_PIN_DC_TWO, INPUT);

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
// frequencies_one[0] is channel 1 lowest frequency, frequencies_two[6] is channel 2 highest frequency
#define NOISE_REDUCTION 70 
void read_frequencies() {

  for(uint8_t i = 0; i < NUM_CHANNELS; i++)
  {
    int16_t temp = analogRead(SS_PIN_DC_ONE) - NOISE_REDUCTION;
    frequencies_one[i] = temp < 0 ? 0 : temp;
    delayMicroseconds(100);
    temp = analogRead(SS_PIN_DC_TWO) - NOISE_REDUCTION;
    frequencies_two[i] = temp < 0 ? 0 : temp;
    delayMicroseconds(100);

    // Acknowledging the read?
    digitalWrite(SS_PIN_STROBE, HIGH);
    delayMicroseconds(100);
    digitalWrite(SS_PIN_STROBE, LOW);
  }
}

