/*
 * CDCIso.h
 *
 * Created: 23/04/2020 10:34:00
 *  Author: Gebruiker
 */ 


#ifndef CDCISO_H_
#define CDCISO_H_

#include <Arduino.h>
#include <USB/PluggableUSB.h>
#include <USB/USBCore.h>


typedef struct
{
//	Iso
	InterfaceDescriptor			dif;
	EndpointDescriptor			in;
	EndpointDescriptor			out;
} CDCIsoDescriptor;

union MemAddr {
	uint32_t* b32;
	uint8_t* b8;
};

class CDCIso_ : public PluggableUSBModule {
public:
	CDCIso_(void);
	bool begin(void);
	void enableInterrupt();
	size_t write(const uint8_t *buffer, size_t size);
	
	
	uint8_t writebuffer[1023];
	uint8_t lenwritebuffer;
	uint8_t databuffer[1023];
		
	uint32_t iso_pcksize;
	uint8_t iso_pckcode;
	
	uint32_t usbConfiguration = 0;
	
	uint8_t epEPNum[2];	
	USBSetup EP0_BK0;
	uint32_t* ptrEP0_BK0;
	MemAddr EP0_BK0_Addr;
	MemAddr EPn_BK1_Addr;
	
protected:
	int getInterface(uint8_t* interfaceNum);
    bool setup(USBSetup& setup);
	int getDescriptor(USBSetup& setup);
	void handleEndpoint(int ep);
	
private:
	uint32_t epIsoType[2];
	uint32_t send(uint32_t ep, const void *data, uint32_t len);
	
	char LastTransmitTimedOut[2] = {0,0};
};


#endif /* CDCISO_H_ */
