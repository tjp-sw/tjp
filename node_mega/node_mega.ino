#define NUM_SHOW_PARAMETERS 30
#define NUM_COLORS_PER_PALETTE 7
#define PI_CONTROLLED
#define DEBUG

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
