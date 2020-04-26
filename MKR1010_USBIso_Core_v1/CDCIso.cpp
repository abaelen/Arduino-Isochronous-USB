/*
 * CDCIso.cpp
 *
 * Created: 23/04/2020 10:34:21
 *  Author: Gebruiker
 */ 

#include <arduino.h>
#include "CDCIso.h"
#include <USB/USBCore.h>
#include <USB/PluggableUSB.h>
#include <USB/CDC.h>

CDCIso_::CDCIso_(void) : PluggableUSBModule(2,2,epIsoType)
{
	epIsoType[0] = USB_ENDPOINT_TYPE_BULK | USB_ENDPOINT_IN(0);
	epIsoType[1] = USB_ENDPOINT_TYPE_ISOCHRONOUS | USB_ENDPOINT_IN(0);
    PluggableUSB().plug(this);
}

int CDCIso_::getInterface(uint8_t* interfaceNum)
{

	interfaceNum[0] += 1;
	
	epEPNum[0]=pluggedEndpoint;
	epEPNum[1]=pluggedEndpoint+1;
	
	CDCIsoDescriptor _cdcIsoInterface = {

		// CDC Iso interface
		D_INTERFACE(uint8_t(pluggedInterface + 1), 2, 0xFF, 0, 0),
		D_ENDPOINT(USB_ENDPOINT_OUT(pluggedEndpoint), USB_ENDPOINT_TYPE_BULK, EPX_SIZE, 0),
		D_ENDPOINT(USB_ENDPOINT_IN (pluggedEndpoint+1), USB_ENDPOINT_TYPE_ISOCHRONOUS, 1023, 1)
	};
	return USBDevice.sendControl(&_cdcIsoInterface, sizeof(_cdcIsoInterface));		

}

bool CDCIso_::setup(USBSetup& setup)
{
	return false;
}

int CDCIso_::getDescriptor(USBSetup& setup)
{
	return 0;
}

void CDCIso_::enableInterrupt() {
	USB->DEVICE.DeviceEndpoint[epEPNum[0]].EPINTENSET.bit.TRCPT0=1; //TO DO CHECK RIGHT BANK?
	USB->DEVICE.DeviceEndpoint[epEPNum[1]].EPINTENSET.bit.TRCPT1=1;
}

void CDCIso_::handleEndpoint(int ep) {
	if (ep==epEPNum[1]) {
		lenwritebuffer=sizeof(writebuffer);
		write(writebuffer,1023);
	}	
}

size_t CDCIso_::write(const uint8_t *buffer, size_t size)
{
	uint32_t r = send(epEPNum[1], buffer, size);

	if (r > 0) {
		return r;
		} else {
		//setWriteError();
		return 0;
	}
}

uint32_t CDCIso_::send(uint32_t ep, const void *data, uint32_t len)
{
	uint32_t written = 0;
	uint32_t length = 0;

	if (!usbConfiguration)
	return -1;
	if (len > 16384)
	return -1;

	#ifdef PIN_LED_TXL
	if (txLEDPulse == 0)
	digitalWrite(PIN_LED_TXL, LOW);

	txLEDPulse = TX_RX_LED_PULSE_MS;
	#endif

	// Flash area
	while (len != 0)
	{
		if (USB->DEVICE.DeviceEndpoint[epEPNum[1]].EPSTATUS.bit.BK1RDY) { //USBCore equivalent: usbd.epBank1IsReady(ep) or usb.DeviceEndpoint[ep].EPSTATUS.bit.BK1RDY;
			// previous transfer is still not complete
			// convert the timeout from microseconds to a number of times through
			// the wait loop; it takes (roughly) 23 clock cycles per iteration.
			uint32_t timeout = microsecondsToClockCycles(70 * 1000) / 23; //70msec
			// Wait for (previous) transfer to complete
			// inspired by Paul Stoffregen's work on Teensy
			while (!USB->DEVICE.DeviceEndpoint[epEPNum[1]].EPINTFLAG.bit.TRCPT1) { //USBCore equivalent: usbd.epBank1IsTransferComplete(ep) or usb.DeviceEndpoint[ep].EPINTFLAG.bit.TRCPT1;
				if (LastTransmitTimedOut[1] || timeout-- == 0) {
					LastTransmitTimedOut[1] = 1;
					// set byte count to zero, so that ZLP is sent
					// instead of stale data
					*(EPn_BK1_Addr.b8+0x004) = *(EPn_BK1_Addr.b8+0x004) & (0xFFFFFFF0);  //Byte count on position 0 to 7 set to 0 //USBCore equivalent: usbd.epBank1SetByteCount(ep, 0) or EP[ep].DeviceDescBank[1].PCKSIZE.bit.BYTE_COUNT = bc; 
					//return -1;
				}
			}
		}

		LastTransmitTimedOut[1] = 0;
		
		if (len >= iso_pcksize) {
			*(EPn_BK1_Addr.b8+0x004) = *(EPn_BK1_Addr.b8+0x004) | (0x1uL << 32); // Auto ZLP on position 31 //USBCore equivalent code: usbd.epBank1EnableAutoZLP(ep) or EP[ep].DeviceDescBank[1].PCKSIZE.bit.AUTO_ZLP = 1
			length =  iso_pcksize;
			} else {
			length = len;
		}

		/* memcopy could be safer in multi threaded environment */
		memcpy(&databuffer, data, length);

		//set Byte count to length of data
		*(uint32_t *) (EPn_BK1_Addr.b8+0x004) = *(uint32_t *) (EPn_BK1_Addr.b8+0x004) | (length);  //Byte count on position 0 to 13 set to length //USBCore equivalent: usbd.epBank1SetByteCount(ep, 0) or EP[ep].DeviceDescBank[1].PCKSIZE.bit.BYTE_COUNT = bc; 

		// Clear the transfer complete flag
		USB->DEVICE.DeviceEndpoint[epEPNum[1]].EPINTFLAG.reg = ((0x3ul << 0) & (2 << 0));//USBCore equivalent code: usbd.epBank1AckTransferComplete(ep) or usb.DeviceEndpoint[ep].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRCPT(2);

		// RAM buffer is full, we can send data (IN)
		USB->DEVICE.DeviceEndpoint[epEPNum[1]].EPSTATUSSET.bit.BK1RDY=1; //USBCore equivalent code: usbd.epBank1SetReady(ep) or usb.DeviceEndpoint[ep].EPSTATUSSET.bit.BK1RDY = 1
		
		written += length;
		len -= length;
		data = (char *)data + length;

	}

	return written;
}
