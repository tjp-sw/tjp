#include <SPI.h>
#include <EEPROM.h>
#include <Ethernet.h>

IPAddress brain(169,254,136,0);

byte node_number = 5;	// get this from eeprom

int led = 13;
int led_state = LOW;	// off

EthernetClient remote;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    delay(10);	// wait for Leonardo serial port to connect
  }

  pinMode(led, OUTPUT);
  digitalWrite(led, led_state);

  for (int addr = 0; addr < 512; addr++) {
    byte val = EEPROM.read(addr);
    if (val != 255) {
      Serial.print("EEPROM[");
      Serial.print(addr, HEX);
      Serial.print("] = ");
      Serial.println(val, HEX);
    }
  }

  // MAC address and IP address of this device
  byte mac[] = { 0x35, 0x28, 0x35, 0x28, 0x00, node_number };
  IPAddress self = brain;	// same network
  self[3] = node_number;	// unique host
  Ethernet.begin(mac, self);	// initialize Ethernet shield

  Serial.print("node ");
  Serial.print(node_number, DEC);
  Serial.print(" initialization complete at ");
  Serial.print(double(millis()) / 1000.0, 2);
  Serial.println(" seconds");
}

void loop()
{
  if (remote.connected()) {
    int len = remote.available();
    if (len > 0) {
      String msg;
      msg.reserve(len + 1);	// pre-allocate the needed space
      for (int i=0; i < len; i++) {
        msg += char(remote.read());
      }
      String editorial = "client received '" + msg + "' at " + String(millis() / 1000, DEC) + " seconds";
      Serial.println(editorial.c_str());
      editorial = "acknowledged: " + msg;
      remote.print(editorial.c_str());

      if (led_state == LOW) {
        led_state = HIGH;
      } else {
        led_state = LOW;
      }
      digitalWrite(led, led_state);
    }
  } else {
    if (remote) {
      Serial.println("disconnected");
    }
    remote.stop();
    delay(10000);	// ten seconds
    if (remote.connect(brain, 3528)) {
      Serial.print("connected, my IP address is ");
      Serial.println(Ethernet.localIP());
      String hello = "I am node " + String(node_number, DEC) + " at " + String(millis() / 1000, DEC) + " seconds";
      remote.print(hello.c_str());
    } else {
      Serial.println("connection failed");
    }
  }
}
