#define PI_CONTROLLED
#define DEBUG
#define I_AM_NODE_MEGA

void setup()
{
  setup_communication();
  setup_tsunami();
}

void loop()
{
  do_communication();
  do_tsunami();
}
