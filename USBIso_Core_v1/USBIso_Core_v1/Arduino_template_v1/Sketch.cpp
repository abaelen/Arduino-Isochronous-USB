/*Begining of Auto generated code by Atmel studio */
#include <Arduino.h>

#include "CDCIso.h"
#include "CDCIso_event.h"

/*End of auto generated code by Atmel studio */


//Beginning of Auto generated function prototypes by Atmel Studio
//End of Auto generated function prototypes by Atmel Studio


void setup() {
	// put your setup code here, to run once:

	USBISO.iso_pcksize=1024;
	USB_SetHandler(UDD_Handler_Iso);
	SerialUSB.print("test");
	delay(1000);
	SerialUSB.print("test");
	delay(1000);
	for (int i=0; i<1023;i++) {
		USBISO.writebuffer[i]=0xAA;
	}
	USBISO.write(USBISO.writebuffer,1023);

}

void loop() {
	// put your main code here, to run repeatedly:

}
