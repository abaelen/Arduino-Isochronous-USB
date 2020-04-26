# Arduino Isochronous USB
 Arduino M0 Isochronous USB IN transaction without changing the ArduinoCore


The ArduinoCore for the SAMD21 does not support Isochronous USB datatransfer.
This gitHub provides the code to implement support for Isochronous USB IN transactions (ie. streaming data from controller TO laptop).
The code will implement a speed of around 1MB/S or 10Mb/s. (10,000 x (1023+51)bytes per 10s)
The code provides a library that adds the Isochronous port to Windows keeping the ability to use Serial print accross the common port.

What to find:
1) The Arduino Sketch implementing Isochronous USB IN transactions
2) The Atmel code implementing Isochronous USB IN transactions
3) The Visual studio C++ code implementing a WinUSB based driver for receiving the Isochronous datatransfer in memory databuffer.

Keep making!
