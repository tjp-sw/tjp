#include <EEPROM.h>
#include "tjp.h"

#define	EEPROM_SIZE	4096	// ATmega2560, EEPROM.length() is not available
#define	EEPROM_DEFAULT	255

void scan_eeprom(const boolean write)
{
  // show any EEPROM bytes that are not the default value
  for (int addr = 0; addr < EEPROM_SIZE; addr++) {
    uint8_t val = EEPROM.read(addr);
    if (val != EEPROM_DEFAULT) {
      Serial.print("EEPROM[0x");
      Serial.print(addr, HEX);
      Serial.print("] = 0x");
      Serial.print(val, HEX);
      if (write && addr != TJP_NODE_ID) {	// do not change TJP_NODE_ID
        EEPROM.write(addr, EEPROM_DEFAULT);
        Serial.print(" ... changed");
      }
      Serial.println("");
    }
  }
}

void print_menu()
{
  Serial.println("");

  Serial.print("EEPROM size is assumed to be ");
  Serial.print(EEPROM_SIZE, DEC);
  Serial.println(" bytes");

  Serial.print("TJP node ID at EEPROM address ");
  Serial.print(TJP_NODE_ID, DEC);
  Serial.print(" (decimal) is ");
  Serial.println(EEPROM.read(TJP_NODE_ID), DEC);

  Serial.println("");
  Serial.println("1. display non-default EEPROM data");
  Serial.println("2. set all EEPROM data to default");
  Serial.println("3. change TJP node ID");
  Serial.println("");
  Serial.print("Choose a number: ");
}

String serial_data;
unsigned long last_serial_data_msec;
int input_state;
uint8_t new_tjp_node_id;

void setup()
{
  Serial.begin(115200);
  Serial.println("This is the Mega setup program.");
  serial_data = "";
  last_serial_data_msec = millis();
  input_state = 0;
  new_tjp_node_id = 0;
  print_menu();
}

void check_for_serial_input()
{
  int len = Serial.available();
  if (len > 0) {
    last_serial_data_msec = millis();
    while (len-- > 0) {
      char c = Serial.read();
      Serial.print(c);	// echo to console
      serial_data += c;
    }
  }
}

const String get_serial()
{
  String data = "";
  if (serial_data.length() > 0 && (millis() - last_serial_data_msec) >= 2) {
    data += serial_data;	// copy
    serial_data = "";
    Serial.println();
  }
  return data;
}

void process_serial_input()
{
  const String msg = get_serial();
  if (msg.length() > 0) {
    switch (input_state) {
      case 0: {	// main menu response
        const long number = msg.toInt();
        switch (number) {
          case 1:
            scan_eeprom(false);
            break;
          case 2:
            scan_eeprom(true);
            break;
          case 3:
            input_state = 1;
            Serial.print("Enter new TJP node ID: ");
            break;
          default:
            Serial.print("unexpected number ");
            Serial.println(number, DEC);
            break;
        }
        break;
      }
      case 1:	// new TJP node ID has been entered
        new_tjp_node_id = msg.toInt();
        if (new_tjp_node_id == EEPROM.read(TJP_NODE_ID)) {
          Serial.print(new_tjp_node_id, DEC);
          Serial.println(" is the current value.");
          input_state = 0;
        } else if (0 < new_tjp_node_id && new_tjp_node_id < 255) {
          input_state = 2;
          Serial.print("Really change TJP node ID to ");
          Serial.print(new_tjp_node_id, DEC);
          Serial.print(" [y/n]? ");
        } else {
          Serial.println("TJP node ID must be from 1 to 254");
          input_state = 0;
        }
        break;
      case 2:	// confirmation
        if (msg.equalsIgnoreCase("y")) {
          EEPROM.write(TJP_NODE_ID, new_tjp_node_id);
          Serial.println("Done.");
        } else {
          Serial.println("Canceled.");
        }
        input_state = 0;
        break;
      default:
        Serial.print("unexpected input state ");
        Serial.println(input_state, DEC);
        input_state = 0;
        break;
    }

    if (input_state == 0) {
      print_menu();
    }
  }
}

void loop()
{
  check_for_serial_input();
  process_serial_input();
}
