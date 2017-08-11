#define PI_CONTROLLED
#define DEBUG
#define I_AM_NODE_MEGA
//#define DEBUG_AUDIO_HOOKS

unsigned long long epoch_msec;
uint8_t node_number = 255;
  
void setup()
{
  Serial.begin(115200);
  setup_communication();
  setup_tsunami();
}

void loop()
{
  do_communication();
  //do_beat_detect();
  do_tsunami();
}
