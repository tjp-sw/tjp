#define	NUM_PARAMETERS		30
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
