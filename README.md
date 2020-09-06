# Arduino Isochronous USB
 Arduino M0 Isochronous USB IN transaction without changing the ArduinoCore


The ArduinoCore for the SAMD21 does not support Isochronous USB datatransfer.
This gitHub provides the code to implement support for Isochronous USB IN transactions at the max of 1023bytes per transfer (ie. streaming data from controller TO laptop). The code implements a speed of around 1MB/S or 10Mb/s. (10,000 x (1023+51)bytes per 10s from Wireshark)
The code provides a library that adds the Isochronous port to Windows keeping the ability to use Serial print accross the common port.

What you find:
1) The Arduino Sketch implementing Isochronous USB IN transactions
2) The Atmel code implementing Isochronous USB IN transactions
3) The Visual studio C++ code implementing a WinUSB based driver for receiving the Isochronous datatransfer in memory databuffer. (new version!)

The VS C++ code is renewed and provides a C DLL library with several functions as:
	Initialize
	Start
	Stop
	Read
	Copy
The library provides the class as well as the DLL exports, exposing it to external applications.

References for reading:
On USB protocol:

				http://www.usbmadesimple.co.uk/
On Arduino's PluggableUSB framework: 

				https://github.com/arduino/Arduino/wiki/PluggableUSB-and-PluggableHID-howto
On WinUSB api for windows receiving end:

				https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/windows-desktop-app-for-a-usb-device
   
				https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/how-to-write-a-windows-desktop-app-that-communicates-with-a-usb-device
   
				https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/getting-set-up-to-use-windows-devices-usb
   
Further detail around some tests I did comparing the BULK, ie. standard data transfer using Serial.print and Isochronous datatransfer.
1. Test description:
  -Run iterative ADC reads for 5 seconds (using RTC clock).
  
  -Keep number of iterations made = Test result
  
  -Write ADC read to Serial.print in 
  
   -package of 2 bytes (serial.print)
   
   -package of 52 bytes (serial.print)
   
   -package of 1023 bytes (isochronous)
   
   -no writing
   
   -Test results:
   
    -2 bytes: 33 000 iterations
    
    -52 bytes: 30 000 iterations
    
    -1023 bytes: 1 293 000 iterations
    
    -no writing: 1 318 000 iterations
    


Hope it can inspire you,

Keep making!
