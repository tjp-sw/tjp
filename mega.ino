#include <SPI.h>
#include <EEPROM.h>
#include <Ethernet.h>

IPAddress brain(169,254,136,0);

byte node_number = 5;	// TODO: get this from eeprom

int led_state;
unsigned long loop_start_time_msec;
unsigned long next_connect_msec;

EthernetClient remote;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  //while (!Serial) {
  //  delay(10);	// wait for Leonardo serial port to connect
  //}

  pinMode(LED_BUILTIN, OUTPUT);
  led_state = LOW;	// off
  digitalWrite(LED_BUILTIN, led_state);

  if (Serial) {
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

  // MAC address and IP address of this device
  byte mac[] = { 0x35, 0x28, 0x35, 0x28, 0x00, node_number };
  IPAddress self = brain;	// same network
  self[3] = node_number;	// unique host
  Ethernet.begin(mac, self);	// initialize Ethernet shield

  if (Serial) {
    Serial.print("node ");
    Serial.print(node_number, DEC);
    Serial.print(" initialization complete at ");
    Serial.print(double(millis()) / 1000.0, 2);
    Serial.println(" seconds");
  }

  next_connect_msec = 500 + millis();	// 1/2 second in the future; TODO: randomize
}

void loop()
{
  loop_start_time_msec = millis();
  if (remote.connected()) {
    int len = remote.available();
    if (len > 0) {
      String msg;
      msg.reserve(len + 1);	// pre-allocate the needed space
      for (int i=0; i < len; i++) {
        msg += char(remote.read());
      }
      if (Serial) {
        Serial.print("client received '");
        Serial.print(msg);
        Serial.print("' at ");
        Serial.print(millis() / 1000, DEC);
        Serial.println(" seconds");
      }
      String editorial = "acknowledged: " + msg;
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
      if (Serial) {
        Serial.println("disconnected");
      }
      remote.stop();
      next_connect_msec = 10000 + loop_start_time_msec;	// 10 seconds in the future; TODO: randomize
    } else if (loop_start_time_msec >= next_connect_msec) {
      if (remote.connect(brain, 3528)) {
        if (Serial) {
          Serial.print("connected, my IP address is ");
          Serial.println(Ethernet.localIP());
        }
        String hello = "I am node " + String(node_number, DEC) + " at " + String(millis() / 1000, DEC) + " seconds";
        remote.print(hello.c_str());
      } else {
        if (Serial) {
          Serial.println("connection failed");
        }
        next_connect_msec = 10000 + loop_start_time_msec;	// 10 seconds in the future; TODO: randomize
      }
    } else {
      if (Serial) {
        Serial.print(".");
      }
      delay(1000);	// 1 sec
    }
  }
}
