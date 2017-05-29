#include <SPI.h>
#include <EEPROM.h>
#include <Ethernet.h>
#include <limits.h>		// provides LONG_MAX
#include "tjp.h"

#define	MAX_PARAMS	2	// allow up to 2 command parameters

IPAddress brain(169,254,136,0);

uint8_t node_number;
boolean serial0_is_enabled;	// cannot detect at run time
int led_state;
unsigned int led_program;
unsigned long loop_start_time_msec;
unsigned long next_connect_msec;
unsigned long long epoch_msec;

EthernetClient remote;

const unsigned long now_sec(const unsigned long when_msec)
{
  return (unsigned long)((epoch_msec + when_msec) / 1000);
}

void print_status(const char* status)
{
  if (serial0_is_enabled) {
    Serial.print(status);
    Serial.print(" at ");
    Serial.print(now_sec(millis()), DEC);
    Serial.println(" seconds");
  }
}

void delay_next_network_connection(uint8_t seconds)
{
  // ensure that the minimum delay is 2 msec
  // add up to 1 second of addtional random delay
  next_connect_msec = millis() + (unsigned long)seconds * 1000 + 2 + random(998);
}

void setup()
{
  serial0_is_enabled = false;	// change to true for debugging
  if (serial0_is_enabled) {
    Serial.begin(115200);
  } else {
    Serial.end();
  }

  epoch_msec = 0;

  // turn off the LED
  pinMode(LED_BUILTIN, OUTPUT);
  led_state = LOW;		// off
  digitalWrite(LED_BUILTIN, led_state);
  led_program = 2;		// default program selection

  // seed the random number generator with some supposedly unpredictable values
  node_number = EEPROM.read(TJP_NODE_ID);
  randomSeed(micros() + node_number);
  for (int pin = 0; pin <= 15; pin++) {
    randomSeed(random(LONG_MAX) + analogRead(pin));
  }

  // establish MAC address and IP address of this device
  byte mac[] = { 0x35, 0x28, 0x35, 0x28, 0x00, node_number };
  IPAddress self = brain;	// same network
  self[3] = node_number;	// unique host
  Ethernet.begin(mac, self);	// initialize Ethernet shield
  remote.stop();		// initialize connection state as disconnected

  if (serial0_is_enabled) {
    Serial.print("a random 4-digit number is ");
    Serial.println(random(10000), DEC);
    Serial.print("node ");
    Serial.print(node_number, DEC);
    print_status(" initialization complete");
  }

  delay_next_network_connection(1);
}

void do_led()
{
  int new_led_state = led_state;	// default to current state

  if (led_program == 0) {		// steady off
    new_led_state = LOW;
  } else if (led_program == 1) {	// steady on
    new_led_state = HIGH;
  } else if (led_program == 2) {	// flash at 1 Hz, 50% duty cycle
    if ((loop_start_time_msec / 500) % 2 == 0) {	// current half second even/odd
      new_led_state = HIGH;
    } else {
      new_led_state = LOW;
    }
  } else if (led_program < 10) {	// flash a few times and pause
    unsigned long step = (loop_start_time_msec / 200) % ((led_program + 1) * 2);
    if (step == 1 || step % 2 == 0) {	// leave LED off once per cycle
      new_led_state = LOW;
    } else {
      new_led_state = HIGH;
    }
  }

  if (led_state != new_led_state) {
    led_state = new_led_state;
    digitalWrite(LED_BUILTIN, led_state);
  }
}

const unsigned long long atoull(const char* string)	// ascii to unsigned long long
{
  char c;
  unsigned long long value = 0;
  while (c = *string++) {
    if (c < '0' or '9' < c) {
      print_status("bad integer string");
      break;
    }
    value *= 10;
    value += (c - '0');
  }
  return value;
}

void do_command(const String message)
{
  // parse out a command and an array of up to MAX_PARAMS parameters
  String command, parameters[MAX_PARAMS];
  int parameter_count = 0;
  int separator = message.indexOf(' ');	// find the space separator, if any
  if (separator >= 0) {
    command = message.substring(0, separator);
    String remaining = message.substring(separator + 1);
    int limit = MAX_PARAMS - 1;
    while (parameter_count < limit) {
      separator = remaining.indexOf(' ');
      if (separator < 0) {
	break;
      }
      parameters[parameter_count] = remaining.substring(0, separator);
      parameter_count += 1;
      remaining = remaining.substring(separator + 1);
    }
    parameters[parameter_count] = remaining;
    parameter_count += 1;
  } else {
    command = message;
  }

  if (serial0_is_enabled) {
    String s = "command parsed as '" + command + "' [";
    for (int i=0; i < parameter_count; i++) {
      s += " '" + parameters[i] + "'";
    }
    s += "]";
    print_status(s.c_str());
  }

  if (command == "time" && parameter_count == 1) {
    epoch_msec = atoull(parameters[0].c_str()) / 1000 - loop_start_time_msec;
    print_status("time set");
  } else if (command == "AllLEDoff") {
    led_program = 0;
  } else if (command == "SetAnimation") {
    led_program = 1;
  } else {
    long number = command.toInt();
    if (2 <= number && number < 10) {
      led_program = number;
    }
  }
}

void do_network()
{
  if (remote.connected()) {
    int len = remote.available();
    if (len > 0) {
      String msg = "";
      msg.reserve(len + 1);	// pre-allocate the needed space
      for (int i=0; i < len; i++) {
        msg += char(remote.read());
      }
      String editorial;
      if (serial0_is_enabled) {
        editorial = "received '" + msg + "'";
        print_status(editorial.c_str());
      }
      if (msg == "reconnect") {
        print_status("disconnecting from brain");
        remote.stop();
        delay_next_network_connection(10);
      } else {
        do_command(msg);
        editorial = "acknowledged: " + msg + " at " + String(now_sec(loop_start_time_msec), DEC);
        remote.print(editorial.c_str());
      }
    }
  } else {
    if (remote) {
      print_status("disconnected from brain");
      remote.stop();
      delay_next_network_connection(10);
    } else if (loop_start_time_msec >= next_connect_msec) {
      if (remote.connect(brain, 3528)) {
        print_status("connected to brain");
        String hello = "I am node " + String(node_number, DEC) + " at " + String(now_sec(millis()), DEC) + " seconds running LED program " + String(led_program, DEC);
        remote.print(hello.c_str());
      } else {
        print_status("connection to brain failed");
        delay_next_network_connection(10);
      }
    }
  }
}

void loop()	// up to 13,000 loops per second
{
  loop_start_time_msec = millis();
  do_led();
  do_network();
}
