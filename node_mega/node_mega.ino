#define PI_CONTROLLED
#define DEBUG
#define I_AM_NODE_MEGA
//#define DEBUG_AUDIO_HOOKS

unsigned long long epoch_msec;
uint8_t node_number = 255;
bool do_soft_reset = false;
unsigned long last_channel_message = 0;

void setup()
{
  delay(2000);
  Serial.begin(115200);
  // Slow bring-up, reset due
  pinMode(46, OUTPUT);
  delay(500);
  reset_due();
  delay(2000);
  setup_communication();
  delay(500);
  setup_tsunami();
}

void loop()
{
  do_communication();
  //do_beat_detect();
  do_tsunami();
  //check_for_failure();
  if (do_soft_reset) {
    do_soft_reset = false;
    reset_due();
  }
}

void reset_due() {
  #ifdef DEBUG
    Serial.println("Resetting due");
  #endif
  digitalWrite(46, 1);
  delay(1000);
  digitalWrite(46, 0);
  delay(500);
}


void check_for_failure() {
  unsigned long curMS = millis();
  if((curMS - last_channel_message > 40000) && (curMS > 60000)) {
    Serial.println(curMS);
    Serial.println(last_channel_message);
    delay(1000 * random(10));
    reset_due();
    //asm volatile ("  jmp 0"); // Reset mega and due if communication lost for 40 sec (after running for at least 60 and connecting)
  }
  /*else if(curMS > 45000 && node_number >= 6) {
    asm volatile ("  jmp 0"); // Reset mega and due if no node assignment after 45 sec
  }*/
}

