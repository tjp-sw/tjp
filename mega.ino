#include <SPI.h>
#include <EEPROM.h>
#include <Ethernet.h>

IPAddress brain(169,254,136,0);

byte node_number = 5;	// TODO: get this from eeprom

boolean serial0_is_enabled;	// cannot detect at run time
int led_state;
unsigned long loop_start_time_msec;
unsigned long next_connect_msec;

EthernetClient remote;

void print_status(const char* status)
{
  if (serial0_is_enabled) {
    Serial.print(status);
    Serial.print(" at ");
    Serial.print(double(millis()) / 1000.0, 2);
    Serial.println(" seconds");
  }
}

void setup() {
  serial0_is_enabled = false;	// change to true for debugging
  if (serial0_is_enabled) {
    Serial.begin(115200);
  } else {
    Serial.end();
  }

  // turn off the LED
  pinMode(LED_BUILTIN, OUTPUT);
  led_state = LOW;	// off
  digitalWrite(LED_BUILTIN, led_state);

  // show any EEPROM bytes that are not the default value
  if (serial0_is_enabled) {
    for (int addr = 0; addr < 512; addr++) {
      byte val = EEPROM.read(addr);
      if (val != 255) {
        Serial.print("EEPROM[");
        Serial.print(addr, HEX);
        Serial.print("] = ");
        Serial.println(val, HEX);
      }
    }
  }

  // establish MAC address and IP address of this device
  byte mac[] = { 0x35, 0x28, 0x35, 0x28, 0x00, node_number };
  IPAddress self = brain;	// same network
  self[3] = node_number;	// unique host
  Ethernet.begin(mac, self);	// initialize Ethernet shield
  remote.stop();		// initialize connection state as disconnected

  if (serial0_is_enabled) {
    Serial.print("node ");
    Serial.print(node_number, DEC);
    print_status(" initialization complete");
  }

  next_connect_msec = 500 + millis();	// 1/2 second in the future; TODO: randomize
}

void loop()	// up to 13,000 loops per second
{
  loop_start_time_msec = millis();
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
      editorial = "acknowledged: " + msg;
      remote.print(editorial.c_str());

      if (led_state == LOW) {
        led_state = HIGH;
      } else {
        led_state = LOW;
      }
      digitalWrite(LED_BUILTIN, led_state);
    }
  } else {
    if (remote) {
      print_status("disconnected from brain");
      remote.stop();
      next_connect_msec = 10000 + loop_start_time_msec;	// 10 seconds in the future; TODO: randomize
    } else if (loop_start_time_msec >= next_connect_msec) {
      if (remote.connect(brain, 3528)) {
        print_status("connected to brain");
        String hello = "I am node " + String(node_number, DEC) + " at " + String(millis() / 1000, DEC) + " seconds";
        remote.print(hello.c_str());
      } else {
        print_status("connection to brain failed");
        next_connect_msec = 10000 + loop_start_time_msec;	// 10 seconds in the future; TODO: randomize
      }
    }
  }
}
