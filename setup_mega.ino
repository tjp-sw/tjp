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

void setup()
{
  Serial.begin(115200);
  Serial.println("This is the Mega setup program.");
  print_menu();
}

void loop()
{
  int len = Serial.available();
  if (len > 0) {
    // wait briefly for more data
    while (true) {
      delay(10);
      int newlen = Serial.available();
      if (newlen > len) {
        len = newlen;
      } else {
        break;
      }
    }

    String msg = "";
    msg.reserve(len + 1);		// pre-allocate the needed space
    for (int i=0; i < len; i++) {
      msg += char(Serial.read());
    }
    Serial.println(msg.c_str());	// echo to console

    const long number = msg.toInt();
    switch (number) {
      case 1:
        scan_eeprom(false);
        break;
      case 2:
        scan_eeprom(true);
        break;
      case 3:
        Serial.println("3 is not implemented yet");
        break;
      default:
        Serial.print("unexpected number ");
        Serial.println(number, DEC);
        break;
    }

    print_menu();
  }
}
