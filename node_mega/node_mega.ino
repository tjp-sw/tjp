#define PI_CONTROLLED
#define DEBUG
#define I_AM_NODE_MEGA

void setup()
{
  Serial.begin(115200);
  setup_communication();
  setup_tsunami();
}

void loop()
{
  do_communication();
  do_tsunami();
}
