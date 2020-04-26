/*
 * CDCIso_event.h
 *
 * Created: 25/04/2020 2:50:22
 *  Author: Gebruiker
 */ 



#include <Arduino.h>
#include "CDCIso.h"
#include <SAMD21_USBDevice.h>
#include <USBAPI.h>

#ifndef CDCISO_EVENT_H_
#define CDCISO_EVENT_H_




CDCIso_ USBISO;


void UDD_Handler_Iso(void) {

if (USB->DEVICE.DeviceEndpoint[0].EPINTFLAG.bit.RXSTP) { //USBCore equivalent code:usbd.epBank0IsSetupReceived(0) or usb.DeviceEndpoint[ep].EPINTFLAG.bit.RXSTP; 
	USBISO.EP0_BK0_Addr.b32=(uint32_t *) (USB->DEVICE.DESCADD.reg);
	//DESCADD contains the address (=> cast to 32 bit adddress) to the pointer that contains (=>request content) the address (cast to 32bit address) to the EP0 BK0 content
	memcpy(&(USBISO.EP0_BK0), (uint32_t *) *((uint32_t *) (USB->DEVICE.DESCADD.reg)), sizeof(USBSetup)); 
	if (REQUEST_STANDARD == (USBISO.EP0_BK0.bmRequestType & REQUEST_TYPE)) {
		switch (USBISO.EP0_BK0.bRequest) {
		case SET_CONFIGURATION:
			if (REQUEST_DEVICE == (USBISO.EP0_BK0.bmRequestType & REQUEST_RECIPIENT)) {
				switch (USBISO.iso_pcksize) {  //EP[ep].DeviceDescBank[1].PCKSIZE.bit.SIZE
					case 8:
						USBISO.iso_pckcode = 0x0;
					case 16:
						USBISO.iso_pckcode = 0x01;
					case 32:
						USBISO.iso_pckcode = 0x02;
					case 64:
						USBISO.iso_pckcode = 0x03;
					case 128:
						USBISO.iso_pckcode = 0x04;
					case 256:
						USBISO.iso_pckcode = 0x05;
					case 512:
						USBISO.iso_pckcode = 0x06;
					case 1024:
						USBISO.iso_pckcode = 0x07; //1024 is necessary is the check below is >= and otherwise always AutoZLP!! //TO DO define HS
				}
				USBISO.EPn_BK1_Addr.b8 = USBISO.EP0_BK0_Addr.b8 + 2*0x10*USBISO.epEPNum[1]+0x10; //EPn has offset 2x (0xn0) //BK1 has offset 0x10
				*(USBISO.EPn_BK1_Addr.b32) = (uint32_t) &(USBISO.databuffer); //Set EPn_BK1 32bit content (=> cast to 32bit value) to address of data_iso_buffer; Equivalent to USBCore statement: usbd.epBank1SetAddress(ep, &udd_ep_in_cache_buffer[ep]);
				*(uint32_t *)(USBISO.EPn_BK1_Addr.b8+0x004) = *(uint32_t *) (USBISO.EPn_BK1_Addr.b8+0x004) | ((uint32_t) USBISO.iso_pckcode << 28); //PckSize has offset 0x004; 0x7 on position 28 means 1023bit; Equivalent to USBCore statement: usbd.epBank1SetSize(ep, 1023);
				USB->DEVICE.DeviceEndpoint[USBISO.epEPNum[1]].EPCFG.bit.EPTYPE1 = 2; //0x02 stands for IN Isochronous
				USBISO.usbConfiguration = USBISO.EP0_BK0.wValueL;
				USBISO.enableInterrupt();
				//leave it to the USBCore to handle the other EP's
			}	
		}
	}
}
USBDevice.ISRHandler();
	
}



#endif /* CDCISO_EVENT_H_ */

