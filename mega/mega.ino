#include <SPI.h>
#include <EEPROM.h>
#include <Ethernet.h>
#include <limits.h>		// provides LONG_MAX
#include "tjp.h"

IPAddress       brain(169,254,136,0);
IPAddress subnet_mask(255,255,0,0);

uint8_t mega_number;
uint8_t node_number;
boolean serial0_is_enabled;	// cannot detect at run time
int led_state;
uint8_t led_program;
unsigned long loop_start_time_msec;
unsigned long next_connect_msec;
unsigned long long epoch_msec;
String network_data;

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
  node_number = 255;		// invalid

  // turn off the LED
  pinMode(LED_BUILTIN, OUTPUT);
  led_state = LOW;		// off
  digitalWrite(LED_BUILTIN, led_state);
  led_program = 8;		// default program selection

  // seed the random number generator with some supposedly unpredictable values
  mega_number = EEPROM.read(TJP_NODE_ID);
  randomSeed(micros() + mega_number);
  for (int pin = 0; pin <= 15; pin++) {
    randomSeed(random(LONG_MAX) + analogRead(pin));
  }

  // establish MAC address and IP address of this device
  byte mac[] = { 0x35, 0x28, 0x35, 0x28, 0x00, mega_number };
  IPAddress self = brain;	// same network
  self[2] = mega_number;	// unique host
  self[3] = mega_number;	// unique host
  // initialize Ethernet shield
  // Ethernet.begin(mac, ip, dns, gateway, subnet);
  Ethernet.begin(mac, self, brain, brain, subnet_mask);
  remote.stop();		// initialize connection state as disconnected
  network_data = "";

  if (serial0_is_enabled) {
    Serial.print("a random 4-digit number is ");
    Serial.println(random(10000), DEC);
    Serial.print("mega ");
    Serial.print(mega_number, DEC);
    print_status(" initialization complete");
  }

  delay_next_network_connection(1);
}

void do_led()
{
  uint8_t new_led_state = led_state;	// default to current state

  if (led_program == 0) {		// steady off
    new_led_state = LOW;
  } else if (led_program == 9) {	// steady on
    new_led_state = HIGH;
  } else if (led_program == 8) {	// flash at 1 Hz, 50% duty cycle
    if (((epoch_msec + loop_start_time_msec) / 500) % 2 == 0) {	// current half second even/odd
      new_led_state = HIGH;
    } else {
      new_led_state = LOW;
    }
  } else if (led_program == 7) {
      if (node_number != 255) {
        led_program = node_number;	// signal the node number
      } else {
        led_program = 8;		// default program
      }
  } else if (led_program < 10) {	// flash a few times and pause
    unsigned long step = ((epoch_msec + loop_start_time_msec) / 200) % ((led_program + 1) * 2);
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

void process_commands()
{
  while (network_data.length() > 0) {
    if (serial0_is_enabled) {
      String s = "looking at " + String(network_data.length(), DEC) + " bytes";
      print_status(s.c_str());
    }
    int size = 1;
    char command = network_data[0];
    switch (command) {
      case 'n':
      case 'p':
        size += 1;	// unsigned 8-bit integer
        if (network_data.length() >= size) {
          if (command == 'n') {
            node_number = network_data[1];
            led_program = node_number;	// signal the node number
          } else if (command == 'p') {
            led_program = network_data[1];
          } else {
            print_status("neither n nor p");
          }
        } else {
          print_status("insufficient node/program data");
        }
        break;
      case 'r':
        print_status("disconnecting from brain");
        remote.stop();
        network_data = "";
        delay_next_network_connection(10);
        break;
      case 't':
        size += 8;	// unsigned 64-bit integer
        if (network_data.length() >= size) {
          int i = 1;
          epoch_msec = (uint8_t)network_data[i++];
          while (i < size) {
            epoch_msec *= 256;
            epoch_msec += (uint8_t)network_data[i++];
          }
          epoch_msec /= 1000;	// convert microseconds to milliseconds
          epoch_msec -= loop_start_time_msec;
          print_status("time set");
        } else {
          print_status("insufficient time data");
        }
        break;
      default:
        print_status("unknown command");
        break;
    }
    network_data = network_data.substring(size);
  }
}

void do_network_input()
{
  if (remote.connected()) {
    int len = remote.available();
    if (len > 0) {
      // pre-allocate the needed space
      network_data.reserve(network_data.length() + len);
      while (len-- > 0) {
        network_data += (char)remote.read();
      }
      process_commands();
    }
  } else {
    if (remote) {
      print_status("disconnected from brain");
      remote.stop();
      network_data = "";
      delay_next_network_connection(10);
    } else if (loop_start_time_msec >= next_connect_msec) {
      if (remote.connect(brain, 3528)) {
        network_data = "";
        print_status("connected to brain");
      } else {
        print_status("connection to brain failed");
        delay_next_network_connection(10);
      }
    }
  }
}

void do_heartbeat()
{
  if (remote.connected()) {
    if (node_number == 255 && loop_start_time_msec % 1000 == 0) {
      char mega_message[3] = {'m', mega_number, '\0'};
      remote.print(mega_message);
      delay(1);	// advance to the next millisecond
      print_status("announcing");
    }
  }
}

void loop()	// up to 13,000 loops per second
{
  loop_start_time_msec = millis();
  do_led();
  do_network_input();
  do_heartbeat();
}
