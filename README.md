## The Journey Project ##

Programs currently available:

* **brain.py** runs on the "brain" Raspberry Pi and communicates with
  each node's Arduino Mega via TCP/IP.  Written in Python.

* **due** runs on each node's Arduino Due to control LED strips.

* **node_mega** runs on each node's Arduino Mega and communicates with
  the "brain" via TCP/IP.  Written in the Arduino "Sketch" language,
  which strongly resembles C++.

* **mock_mega.py** runs on the "brain" to lightly simulate the network
  behavior of a node's Arduino Mega.  Written in Python.

* **serial_test** exercises Mega and Due UARTs to demonstrate
  communication.

* **setup_mega** may be run on a Mega to manage its EEPROM.

* **test_due** demonstrates FastLED parallel output.
