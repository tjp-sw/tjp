#define NUM_UARTS 4
HardwareSerial* uart[NUM_UARTS] = { &Serial, &Serial1, &Serial2, &Serial3 };
String buffer[NUM_UARTS];
unsigned long last_input_msec[NUM_UARTS];
unsigned long last_output_msec[NUM_UARTS];
uint8_t led_state;

void setup()
{
  for (int s = 0; s < NUM_UARTS; s++) {
    uart[s]->begin(115200);
    buffer[s] = "";
    last_input_msec[s] = 0;
    last_output_msec[s] = 0;
  }

  pinMode(LED_BUILTIN, OUTPUT);
  led_state = LOW;
  digitalWrite(LED_BUILTIN, led_state);

  Serial.print("tester initialized with ");
  Serial.print(NUM_UARTS, DEC);
  Serial.println(" serial ports");
}

void do_led(unsigned long time_msec)
{
  uint8_t new_led_state = led_state;	// default to current state

  // flash at 1 Hz, 50% duty cycle
  if ((time_msec / 500) % 2 == 0) {	// current half second even/odd
    new_led_state = HIGH;
  } else {
    new_led_state = LOW;
  }

  if (led_state != new_led_state) {
    led_state = new_led_state;
    digitalWrite(LED_BUILTIN, led_state);
  }
}


void loop()
{
  const unsigned long loop_start_time_msec = millis();
  for (int s = 0; s < NUM_UARTS; s++) {
    int len = uart[s]->available();
    if (len > 0) {
      last_input_msec[s] = loop_start_time_msec;
      // pre-allocate the needed space
      buffer[s].reserve(buffer[s].length() + len);
      while (len-- > 0) {
        buffer[s] += (char)uart[s]->read();
      }
    }
    if (buffer[s].length() > 0 && loop_start_time_msec >= last_input_msec[s] + 2) {
      Serial.print(millis(), DEC);
      Serial.print(": serial ");
      Serial.print(s, DEC);
      Serial.print(" says ");
      Serial.println(buffer[s]);
      buffer[s] = "";
    }
    if (loop_start_time_msec >= last_output_msec[s] + 5000) {
      last_output_msec[s] = loop_start_time_msec;
      if (s == 0) {
        Serial.println();
        Serial.print(loop_start_time_msec, DEC);
        Serial.print(": ");
      } else {
        const char id = '0' + s;
        uart[s]->write(id);
        Serial.print(id);
      }
    }
  }
  do_led(loop_start_time_msec);
}
