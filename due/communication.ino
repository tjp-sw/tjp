#include "tjp.h"

#ifdef I_AM_MEGA
  #include <SPI.h>
  #include <EEPROM.h>
  #include <Ethernet.h>
  #include <limits.h>    // provides LONG_MAX
  
  IPAddress brain(169,254,136,0);
  IPAddress subnet_mask(255,255,0,0);
  
  uint8_t mega_number;
  unsigned long next_connect_msec;
  String network_data;
  
  EthernetClient remote;
  
  #define NodeMate  Serial3

  // declare here when not part of due.ino
  unsigned long long epoch_msec;
  uint8_t node_number; 
  
#elif defined(I_AM_DUE)
  #define  NodeMate  Serial1

#endif // I_AM_MEGA / I_AM_DUE


enum communication_source { network, mate };

uint8_t led_state;
uint8_t led_program;
unsigned long loop_start_time_msec;
unsigned long last_announcement_msec;
unsigned long mate_last_input_msec;
String mate_data;


#ifdef DEBUG
unsigned long now_sec(const unsigned long when_msec) {
  return (unsigned long)((epoch_msec + when_msec) / 1000);
}

void print_status(const char* status) {
  Serial.print(status);
  Serial.print(" at ");
  Serial.print(now_sec(millis()), DEC);
  Serial.println(" seconds");
}

void print_status(const char* status, const long value) {
  Serial.print(status);
  Serial.print(value, DEC);
  Serial.print(" at ");
  Serial.print(now_sec(millis()), DEC);
  Serial.println(" seconds");
}

void do_led() {
  uint8_t new_led_state = led_state;  // default to current state

  if (led_program == 0) {   // steady off
    new_led_state = LOW;
  } else if (led_program == 9) {  // steady on
    new_led_state = HIGH;
  } else if (led_program == 8) {  // flash at 1 Hz, 50% duty cycle
    if (((epoch_msec + loop_start_time_msec) / 500) % 2 == 0) { // current half second even/odd
      new_led_state = HIGH;
    } else {
      new_led_state = LOW;
    }
  } else if (led_program == 7) {
      if (node_number != 255) {
        led_program = node_number == 0 ? 6 : node_number; // signal the node number
      } else {
        led_program = 8;    // default program
      }
  } else if (led_program < 10) {  // flash a few times and pause
    unsigned long step = ((epoch_msec + loop_start_time_msec) / 200) % ((led_program + 1) * 2);
    if (step == 1 || step % 2 == 0) { // leave LED off once per cycle
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
#endif


#ifdef I_AM_MEGA
void delay_next_network_connection(uint8_t seconds) {
  // ensure that the minimum delay is 2 msec
  // add up to 1 second of addtional random delay
  next_connect_msec = millis() + (unsigned long)seconds * 1000 + 2 + random(998);
}

void do_network_input() {
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
  }
  else {
    if (remote) {
      #ifdef DEBUG
        print_status("disconnected from brain");
      #endif
      remote.stop();
      network_data = "";
      delay_next_network_connection(10);
    }
    else if (loop_start_time_msec >= next_connect_msec) {
      if (remote.connect(brain, 3528)) {
        network_data = "";
        #ifdef DEBUG
          print_status("connected to brain");
        #endif
      }
      else {
        #ifdef DEBUG
          print_status("connection to brain failed");
        #endif
        delay_next_network_connection(10);
      }
    }
  }
}
#endif // I_AM_MEGA


#ifdef I_AM_DUE
void send_audio_packet() {
  uint8_t audioData[2] = { is_beat, downbeat_proximity };
  NodeMate.write(audioData, sizeof audioData);
}

// Sets up LED array after being assigned a node by the Pi.
void assign_node(uint8_t node_num) {
  node_number = node_num;
  
  #ifdef DEBUG
    Serial.println("Assigned node #" + String(node_number));
    for(int i = 0; i < 4; i ++)
      leds[LEDS_PER_STRIP*i] = CRGB::Red;
    LEDS.show();
    delay(500);
    for(int i = 0; i < 4; i ++)
      leds[LEDS_PER_STRIP*i] = CRGB::Green;
    LEDS.show();
    delay(500);
    for(int i = 0; i < 4; i ++)
      leds[LEDS_PER_STRIP*i] = CRGB::Blue;
    LEDS.show();
    delay(500);
  #endif
}
#endif


void setup_communication() {
  #ifdef DEBUG
    #ifdef I_AM_MEGA
      Serial.begin(115200); // Already called for due in setup()
    #endif


  // turn off the LED
  pinMode(LED_BUILTIN, OUTPUT);
  led_state = LOW;    // off
  digitalWrite(LED_BUILTIN, led_state);
  led_program = 8;    // default program selection
  #endif // DEBUG && I_AM_MEGA


  last_announcement_msec = 0;

  NodeMate.begin(115200);
  mate_last_input_msec = 0;
  mate_data = "";

  epoch_msec = 0;

  #ifdef I_AM_MEGA
    // seed the random number generator with some supposedly unpredictable values
    mega_number = EEPROM.read(TJP_NODE_ID);
    randomSeed(micros() + mega_number);
    for (uint8_t pin = 0; pin <= 15; pin++) {
      randomSeed(random(LONG_MAX) + analogRead(pin));
    }

    delay(50);      // extra time for Ethernet shield to power on
    // establish MAC address and IP address of this device
    byte mac[] = { 0x35, 0x28, 0x35, 0x28, 0x00, mega_number };
    IPAddress self = brain; // same network
    self[2] = mega_number;  // unique host
    self[3] = mega_number;  // unique host
    // initialize Ethernet shield
    // Ethernet.begin(mac, ip, dns, gateway, subnet);
    Ethernet.begin(mac, self, brain, brain, subnet_mask);
    remote.stop();    // initialize connection state as disconnected
    network_data = "";

    #ifdef DEBUG
      print_status("a random 4-digit number is ", random(10000));
      print_status("initialization is complete for mega ", (long)mega_number);
    #endif
    
    delay_next_network_connection(1);
  #endif // I_AM_MEGA

  #if defined(I_AM_DUE) && defined(DEBUG)
    print_status("initialization is complete for due");
  #endif // I_AM_DUE && DEBUG
}

void process_commands(const int source, String& input) {
  while (input.length() > 0) {
    #ifdef DEBUG
      print_status("bytes available: ", (long)input.length());
    #endif
    
    size_t size = 1;
    char command = input[0];
    switch (command) {
      #ifdef I_AM_MEGA
      case 'a':
        handle_commands(command);
      case 'b':
        size += AUDIO_PACKET_SIZE;
        if (input.length() >= size) {
          // pass the beat message through in both directions
          if (source == mate && remote.connected()) {
            remote.write((uint8_t *)input.c_str(), size);
          }
          else if (source == network) {
            NodeMate.write((uint8_t *)input.c_str(), size);
          }
        }
        else {
          #ifdef DEBUG
            print_status("insufficient beat data");
          #endif
        }
        break;

      case 'd':
        const uint8_t node_message[2] = { 'n', node_number };
        NodeMate.write(node_message, 2);
        break;

      case 'r':
        #ifdef DEBUG
          print_status("disconnecting from brain");
        #endif
        remote.stop();
        network_data = "";
        delay_next_network_connection(10);
        break;
      
      #endif // I_AM_MEGA
 
      case 'n':
      case 'p':
        size += 1;  // unsigned 8-bit integer
        if (input.length() >= size) {
          if(command == 'n') {
            #ifdef I_AM_DUE
              assign_node(input[1]);
            #endif
  
            led_program = node_number == 0 ? 6 : node_number;  // signal the node number

            #ifdef I_AM_MEGA
              NodeMate.write((uint8_t *)input.c_str(), size);
            #endif // I_AM_MEGA
            
          }
          else if (command == 'p') {
            led_program = input[1];
            #ifdef I_AM_MEGA
              NodeMate.write((uint8_t *)input.c_str(), size);
            #endif // I_AM_MEGA
          }
          else {
            #ifdef DEBUG
              print_status("neither n nor p");
            #endif
          }
        }
        else {
          #ifdef DEBUG
            print_status("insufficient node/program data");
          #endif
        }
        break;
 
      case 's':
        size += NUM_SHOW_PARAMETERS + 3*NUM_COLORS_PER_PALETTE;
        if (input.length() >= size) {
          #ifdef I_AM_MEGA
            NodeMate.write((uint8_t *)input.c_str(), size);
          #endif // I_AM_MEGA
      
          #ifdef I_AM_DUE
            uint8_t params[NUM_SHOW_PARAMETERS], colors[3*NUM_COLORS_PER_PALETTE];
            size_t i = 0;
            size_t j = 1;
            while (i < sizeof params) {
              params[i++] = input[j++];
            }
            i = 0;  // restart at the beginning of the colors array
            while (j < size) {
              colors[i++] = input[j++];
            }

            if(node_number < NUM_NODES) {
              uint8_t last_base_animation = BASE_ANIMATION;
              uint8_t last_mid_animation = MID_ANIMATION;
              uint8_t last_sparkle_animation = SPARKLE_ANIMATION;
              
              memcpy(show_parameters, params, NUM_SHOW_PARAMETERS);
              memcpy(target_palette, colors, 3*NUM_COLORS_PER_PALETTE);
              blend_base_layer = current_palette[0] != target_palette[0] || current_palette[1] != target_palette[1];
              blend_mid_layer = current_palette[2] != target_palette[2] || current_palette[3] != target_palette[3] || current_palette[4] != target_palette[4];
              blend_sparkle_layer = current_palette[5] != target_palette[5] || current_palette[6] != target_palette[6];
    
              if(BASE_ANIMATION != last_base_animation) {
                cleanup_base_animation(last_base_animation);
                init_base_animation();
              }
              
              if(MID_ANIMATION != last_mid_animation) {
                cleanup_mid_animation(last_mid_animation);
                init_mid_animation();
              }
    
              if(SPARKLE_ANIMATION != last_sparkle_animation) {
                cleanup_sparkle_animation(last_sparkle_animation);
                init_sparkle_animation();
              }
              
              #ifdef DEBUG
                Serial.print("params");
                for (i = 0; i < sizeof params; i++)
                {
                  Serial.print(' ');
                  Serial.print(params[i], DEC);
                }
                Serial.print(" colors");
                for (i = 0; i < sizeof colors; i++) {
                  Serial.print(' ');
                  Serial.print(colors[i], DEC);
                }
    
                Serial.println();
              #endif // DEBUG
            }
            else {
              #ifdef DEBUG
                Serial.print("Ignoring message. node_number = " + String(node_number));
              #endif
            }
          #endif // I_AM_DUE
        }
        else {
          #ifdef DEBUG
            print_status("insufficient show data");
          #endif
        }
        break;
  
      
      case 't':
        size += 8;  // unsigned 64-bit integer
        if (input.length() >= size) {
          const unsigned long long old_epoch_msec = epoch_msec;
          unsigned int i = 1;
          epoch_msec = (uint8_t)input[i++];
          while (i < size) {
            epoch_msec *= 256;
            epoch_msec += (uint8_t)input[i++];
          }
      
          epoch_msec /= 1000; // convert microseconds to milliseconds
          epoch_msec -= loop_start_time_msec;

          #ifdef DEBUG
            if (epoch_msec > old_epoch_msec) {
              print_status("time advanced ", long(epoch_msec - old_epoch_msec));
            } else if (epoch_msec < old_epoch_msec) {
              print_status("time set back ", long(old_epoch_msec - epoch_msec));
            } else {
              print_status("time unchanged!");
            }
          #endif // DEBUG
          
          #ifdef I_AM_MEGA
            NodeMate.write((uint8_t *)input.c_str(), size);
          #endif // I_AM_MEGA
        }
        else {
          #ifdef DEBUG
            print_status("insufficient time data");
          #endif
        }
        break;
    
      default:
        #ifdef DEBUG
          print_status("unknown command: ");
          Serial.println(command);
        #endif
        break;
    }

    input = input.substring(size);  // discard processed message
  }
}

void do_mate_input() {
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

void do_heartbeat() {
  if (node_number == 255 && loop_start_time_msec > last_announcement_msec + 1000) {
    #ifdef I_AM_MEGA
      if (remote.connected()) {
        const char mega_message[3] = {'m', (char)mega_number, '\0'};
        remote.print(mega_message);
    #endif // I_AM_MEGA
    #ifdef I_AM_DUE
      NodeMate.write('d');
    #endif // I_AM_DUE
    
    #ifdef DEBUG
      print_status("announcing");
    #endif
    
    #ifdef I_AM_MEGA
      }
    #endif // I_AM_MEGA
      last_announcement_msec = millis();
  }
}

void do_communication() {
  loop_start_time_msec = millis();
  #ifdef I_AM_MEGA
    do_network_input();
  #endif // I_AM_MEGA
  
  do_mate_input();

  #ifdef I_AM_THE_BEAT_DUE
    send_audio_packet();
  #endif
  
  do_heartbeat();
  #ifdef DEBUG
    do_led();
  #endif
}
