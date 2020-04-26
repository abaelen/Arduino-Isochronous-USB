# Arduino Isochronous USB
 Arduino M0 Isochronous USB IN transaction without changing the ArduinoCore


The ArduinoCore for the SAMD21 does not support Isochronous USB datatransfer.
This gitHub provides the code to implement support for Isochronous USB IN transactions at the max of 1023bytes per transfer (ie. streaming data from controller TO laptop). The code implements a speed of around 1MB/S or 10Mb/s. (10,000 x (1023+51)bytes per 10s)
The code provides a library that adds the Isochronous port to Windows keeping the ability to use Serial print accross the common port.

What you find:
1) The Arduino Sketch implementing Isochronous USB IN transactions
2) The Atmel code implementing Isochronous USB IN transactions
3) The Visual studio C++ code implementing a WinUSB based driver for receiving the Isochronous datatransfer in memory databuffer.

References for reading:
On USB protocol:

				http://www.usbmadesimple.co.uk/
On Arduino's PluggableUSB framework: 

				https://github.com/arduino/Arduino/wiki/PluggableUSB-and-PluggableHID-howto
On WinUSB api for windows receiving end:

				https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/windows-desktop-app-for-a-usb-device
   
				https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/how-to-write-a-windows-desktop-app-that-communicates-with-a-usb-device
   
				https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/getting-set-up-to-use-windows-devices-usb
   

Hope it can inspire you,

Keep making!
