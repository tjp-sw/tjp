#include "tjp.h"

#ifdef I_AM_MEGA
#include <SPI.h>
#include <EEPROM.h>
#include <Ethernet.h>
#include <limits.h>		// provides LONG_MAX

IPAddress       brain(169,254,136,0);
IPAddress subnet_mask(255,255,0,0);

uint8_t mega_number;
unsigned long next_connect_msec;
String network_data;

EthernetClient remote;

#define	NodeMate	Serial3
#endif // I_AM_MEGA

#ifdef I_AM_DUE
#define	NodeMate	Serial1
#endif // I_AM_DUE

enum communication_source { network, mate };

bool serial0_is_enabled;	// cannot detect at run time
#ifndef NUM_CHANNELS
uint8_t node_number;		// declare here when not part of due.ino
unsigned long long epoch_msec;
#endif
uint8_t led_state;
uint8_t led_program;
unsigned long loop_start_time_msec;

unsigned long mate_last_input_msec;
String mate_data;

unsigned long now_sec(const unsigned long when_msec)
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

void print_status(const char* status, const long value)
{
  if (serial0_is_enabled) {
    Serial.print(status);
    Serial.print(value, DEC);
    Serial.print(" at ");
    Serial.print(now_sec(millis()), DEC);
    Serial.println(" seconds");
  }
}

#ifdef I_AM_MEGA
void delay_next_network_connection(uint8_t seconds)
{
  // ensure that the minimum delay is 2 msec
  // add up to 1 second of addtional random delay
  next_connect_msec = millis() + (unsigned long)seconds * 1000 + 2 + random(998);
}
#endif // I_AM_MEGA

void setup_communication()
{
#ifdef DEBUG			// defined in due.ino
  serial0_is_enabled = true;
#else
  serial0_is_enabled = false;	// change to true for debugging
  if (serial0_is_enabled) {
    Serial.begin(115200);
  }
#endif // DEBUG

  NodeMate.begin(115200);
  mate_last_input_msec = 0;
  mate_data = "";

  epoch_msec = 0;
  #ifdef PI_CONTROLLED
  node_number = 255;		// invalid
  #endif

  // turn off the LED
  pinMode(LED_BUILTIN, OUTPUT);
  led_state = LOW;		// off
  digitalWrite(LED_BUILTIN, led_state);
  led_program = 8;		// default program selection

#ifdef I_AM_MEGA
  // seed the random number generator with some supposedly unpredictable values
  mega_number = EEPROM.read(TJP_NODE_ID);
  randomSeed(micros() + mega_number);
  for (int pin = 0; pin <= 15; pin++) {
    randomSeed(random(LONG_MAX) + analogRead(pin));
  }

  delay(50);			// extra time for Ethernet shield to power on
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

  print_status("a random 4-digit number is ", random(10000));
  print_status("initialization is complete for mega ", (long)mega_number);

  delay_next_network_connection(1);
#endif // I_AM_MEGA

#ifdef I_AM_DUE
  print_status("initialization is complete for due");
#endif // I_AM_DUE
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
        led_program = node_number == 0 ? 6 : node_number;	// signal the node number
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

void process_commands(const int source, String& input)
{
  while (input.length() > 0) {
    print_status("bytes available: ", (long)input.length());
    size_t size = 1;
    char command = input[0];
    switch (command) {
#ifdef I_AM_MEGA
      case 'b':
        size += 55;
        if (input.length() >= size) {
          // pass the beat message through in both directions
          if (source == mate && remote.connected()) {
            remote.write((uint8_t *)input.c_str(), size);
          } else if (source == network) {
            NodeMate.write((uint8_t *)input.c_str(), size);
          }
        } else {
          print_status("insufficient beat data");
        }
        break;
#endif // I_AM_MEGA
#ifdef I_AM_MEGA
      case 'd': {
        const uint8_t node_message[2] = { 'n', node_number };
        NodeMate.write(node_message, 2);
        break;
        }
#endif // I_AM_MEGA
      case 'n':
      case 'p':
        size += 1;	// unsigned 8-bit integer
        if (input.length() >= size) {
          if (command == 'n') {
            assign_node(input[1]);
            led_program = node_number == 0 ? 6 : node_number;	// signal the node number
#ifdef I_AM_MEGA
            NodeMate.write((uint8_t *)input.c_str(), size);
#endif // I_AM_MEGA
          } else if (command == 'p') {
            led_program = input[1];
#ifdef I_AM_MEGA
            NodeMate.write((uint8_t *)input.c_str(), size);
#endif // I_AM_MEGA
          } else {
            print_status("neither n nor p");
          }
        } else {
          print_status("insufficient node/program data");
        }
        break;
#ifdef I_AM_MEGA
      case 'r':
        print_status("disconnecting from brain");
        remote.stop();
        network_data = "";
        delay_next_network_connection(10);
        break;
#endif // I_AM_MEGA
      case 's':
        size += NUM_PARAMETERS;
        if (input.length() >= size) {
            uint8_t number_of_colors = input[1 + NUM_COLORS_INDEX];
            size += number_of_colors;
            if (input.length() >= size) {
#ifdef I_AM_DUE
              uint8_t params[NUM_PARAMETERS], colors[6];

              size_t i = 0;
              size_t j = 1;
              while (i < sizeof params) {
                params[i++] = input[j++];
              }
              i = 0;	// restart at the beginning of the colors array
              while (j < size) {
                colors[i++] = input[j++];
              }
              while (i < sizeof colors) {
                colors[i++] = 255;	// fill with invalid values
              }
              if (serial0_is_enabled) {
                Serial.print("params");
                for (i = 0; i < sizeof params; i++) {
                  Serial.print(' ');
                  Serial.print(params[i], DEC);
                }
                Serial.print(" colors");
                for (i = 0; i < sizeof colors; i++) {
                  Serial.print(' ');
                  Serial.print(colors[i], DEC);
                }
                Serial.println();
              }
#ifdef NUM_CHANNELS
              memcpy(show_parameters, params, NUM_PARAMETERS);
              memcpy(show_colors, colors, NUM_COLORS_PER_PALETTE);
#endif // NUM_CHANNELS
#endif // I_AM_DUE
#ifdef I_AM_MEGA
              NodeMate.write((uint8_t *)input.c_str(), size);
#endif // I_AM_MEGA
            } else {
              print_status("insufficient show data");
            }
        } else {
          print_status("insufficient show data");
        }
        break;
      case 't':
        size += 8;	// unsigned 64-bit integer
        if (input.length() >= size) {
          const unsigned long long old_epoch_msec = epoch_msec;
          unsigned int i = 1;
          epoch_msec = (uint8_t)input[i++];
          while (i < size) {
            epoch_msec *= 256;
            epoch_msec += (uint8_t)input[i++];
          }
          epoch_msec /= 1000;	// convert microseconds to milliseconds
          epoch_msec -= loop_start_time_msec;
          if (epoch_msec > old_epoch_msec) {
            print_status("time advanced ", long(epoch_msec - old_epoch_msec));
          } else if (epoch_msec < old_epoch_msec) {
            print_status("time set back ", long(old_epoch_msec - epoch_msec));
          } else {
            print_status("time unchanged!");
          }
#ifdef I_AM_MEGA
          NodeMate.write((uint8_t *)input.c_str(), size);
#endif // I_AM_MEGA
        } else {
          print_status("insufficient time data");
        }
        break;
      default:
        print_status("unknown command");
        break;
    }
    input = input.substring(size);	// discard processed message
  }
}

void do_mate_input()
{
  int len = NodeMate.available();
  if (len > 0) {
    mate_last_input_msec = loop_start_time_msec;
    // pre-allocate the needed space
    mate_data.reserve(mate_data.length() + len);
    while (len-- > 0) {
      mate_data += (char)NodeMate.read();
    }
  }
  if (mate_data.length() > 0 && loop_start_time_msec >= mate_last_input_msec + 2) {
    process_commands(mate, mate_data);
  }
}

#ifdef I_AM_MEGA
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
      process_commands(network, network_data);
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
#endif // I_AM_MEGA

void do_heartbeat()
{
  if (node_number == 255 && loop_start_time_msec % 1000 == 0) {
#ifdef I_AM_MEGA
    if (remote.connected()) {
      const char mega_message[3] = {'m', (char)mega_number, '\0'};
      remote.print(mega_message);
#endif // I_AM_MEGA
#ifdef I_AM_DUE
      NodeMate.write('d');
#endif // I_AM_DUE
      delay(1);	// advance to the next millisecond
      print_status("announcing");
#ifdef I_AM_MEGA
    }
#endif // I_AM_MEGA
  }
}

void do_communication()
{
  loop_start_time_msec = millis();
#ifdef I_AM_MEGA
  do_network_input();
#endif // I_AM_MEGA
  do_mate_input();

  if(node_number == 0) {
    send_audio_packet();
  }
  
  do_heartbeat();
  do_led();
}

#ifdef NUM_CHANNELS
// Send this from the Due that has audio (node_number == 0?), up to the Pi, then back out to all of the nodes
void send_audio_packet()
{
  uint8_t audioData[55];
  audioData[0] = 'b';
  audioData[1] = downbeat_proximity;
  audioData[2] = bpm_estimate;
  audioData[3] = bpm_confidence;
  audioData[4] = low_band_emphasis;
  audioData[5] = mid_band_emphasis;
  audioData[6] = high_band_emphasis;
  audioData[7] = dominant_channel[0];
  audioData[8] = overall_volume[0] >> 8;
  audioData[9] = overall_volume[0] & 0xFF;
  audioData[10] = band_distribution[0][0] * 255;
  audioData[11] = band_distribution[0][1] * 255;
  audioData[12] = band_distribution[0][2] * 255;
  for(uint8_t i = 0; i < NUM_CHANNELS; i++) {
    audioData[13 + 2*i] = frequencies_max[i] >> 8;
    audioData[13 + 2*i + 1] = frequencies_max[i] & 0xFF;
    audioData[27 + 2*i] = frequencies_one[i] >> 8;
    audioData[27 + 2*i + 1] = frequencies_one[i] & 0xFF;
    audioData[41 + 2*i] = frequencies_two[i] >> 8;
    audioData[41 + 2*i + 1] = frequencies_two[i] & 0xFF;
  }
  NodeMate.write(audioData, sizeof audioData);
}
#endif

void assign_node(uint8_t node_num) {
  // Needs to call setLeds() for each strip?
  // Needs to re-call .setCorrection(TypicalLEDStrip)?
  node_number = node_num;
  LEDS[0].setLeds(&leds_raw[node_num * PHYSICAL_LEDS_PER_NODE], LEDS_PER_STRIP);
  
  #ifdef DEBUG
    Serial.println("Assigned node #" + String(node_number));
    for(int i = 0; i < 4; i ++)
      leds_raw[node_num * PHYSICAL_LEDS_PER_NODE + LEDS_PER_STRIP*i] = CRGB::Green;
    LEDS.show();
    delay(1000);
    for(int i = 0; i < 4; i ++)
      leds_all(node_num * PHYSICAL_LEDS_PER_NODE + LEDS_PER_STRIP*i, node_num * PHYSICAL_LEDS_PER_NODE + 5 + LEDS_PER_STRIP*i) = CRGB::Red;
    LEDS.show();
    delay(1000);
    for(int i = 0; i < 4; i ++)
      leds[node_num * RINGS_PER_NODE + 3*i][-1] = CRGB::Blue;
    LEDS.show();
    delay(1000);
  #endif
}


