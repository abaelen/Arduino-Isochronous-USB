#pragma once
#include "pch.h"
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <vector>
#include <chrono>

/*Nomen:
	Metadata, Structures, ... : _XXXXXXX
	Instances of Structures: XXXXXXX
	Metadata, classes, ... : _Xxxxxx
	Instances of Metadata: Xxxxx_Xxxxx
	Local instances of variables: xxxxxx
*/


//To use the Class create a _USB_Device USB member. Then call the USB.Initialize(1023,1,1) eg. 1023 in 1 packet & 1 microframe (as only FS) or USB.Initialize(2046,2,1), ...

class _USB_Device
{
	//properties:
		public: DEVICE_DATA Handle;
		public: USB_DEVICE_DESCRIPTOR Descriptor; //Structure holding the device descriptor. Ref. https://www.keil.com/pack/doc/mw/USB/html/_u_s_b__descriptors.html
		public: USB_INTERFACE_DESCRIPTOR Interface; //Structure holding the interface descriptor. Ref. https://www.keil.com/pack/doc/mw/USB/html/_u_s_b__descriptors.html
		public: std::vector<uint8_t> BackBuffer;
		public: std::vector<uint8_t> FrontBuffer;


		public: struct _Status
		{
			BOOL Started=FALSE; //Indicates if the USB Device is in the started state.
			BOOL Stopped=FALSE; //Indicates whether the USB Device is in the stopped state.
			BOOL Initialized=FALSE; //Indicates whether the USB Device is initialized.
			BOOL Errored=FALSE; //Indicates whether the USB Device is in Error state.
			
			struct _Action //Structure that provides insight in which stage(s) the device errored out.
			{
				BOOL DeviceFound = FALSE;			//Returned from the OpenIsochDevice method, indicating whether handle to the device has been retrieved.
				BOOL GetDescriptor = FALSE;			//Returned from the GetDescriptor method, indicating whether the USB descriptor was retrieved.
				BOOL GetInterface = FALSE;			//Returned from the GetInterface method, indicating whether the USB interface description was retrieved.
				BOOL GetIsochPipe = FALSE;			//Returned from the GetIsochPipe method, indicating whether the Pipe to access the Isochronous bus was found.
				BOOL GetInterval = FALSE;			//Returned from the GetInterval method, indicating whether the Interval information was retrieved. For FS this is default 1.
				BOOL SetTransferChars = FALSE;		//Returned from the SetTransferCharacteristics method, indicatign whether the bytes per frame, interval microsize and packet count where set.
				BOOL SetOverlappedStructure = FALSE;//Returned from the SetOverlappedStructure method, indicating whether the overlapped structures, where succesfully setup.
				BOOL SetOverlappedEvents = FALSE;   //Returned from the SetOverlappedEvents method, indicating whether the overlapped events where succesfully setup.
				BOOL SetIsochPackets = FALSE;		//Returned from the SetIsochPackets method, indicating whether the Isoch packets where succesfully setup.
				BOOL RegisterIsochBuffer = FALSE;	//Returned from the RegisterIsochBuffer method, indicating whether the registration of the Isoch buffer was succesfull.
				BOOL ResetPipe = FALSE;				//Returned from the ResetPipe method, indicating whether the pipe was succesfully reset
				BOOL EndAtFrame = FALSE;			//Returned from the SetTransferCahacteristics method, indicating the buffersize is not compliant to the Isosch transfer size.
				BOOL ReadIsochPipe = FALSE;			//Returned from the Start method, indicating the result of the reading of the Isoch Pipe.
				/*
				BOOL ReadTransfer = FALSE;
				
				BOOL AllocateMemory = FALSE;
				BOOL TransferCompleted = FALSE;
				BOOL UnregisterIsochBuffer = FALSE;
				BOOL FoundPipe = FALSE;
				BOOL FailedToStartRead = FALSE;

				HRESULT QueryPipe = 0;*/
			} Action;
		} Status; //Provides the status of the USB Handle.

		private: LPOVERLAPPED Overlapped; //Contains information used in asynchronous (or overlapped) input and output (I/O).
		private: PUSBD_ISO_PACKET_DESCRIPTOR Packets; //The USBD_ISO_PACKET_DESCRIPTOR structure is used by USB client drivers to describe an isochronous transfer packet.
		private: WINUSB_ISOCH_BUFFER_HANDLE IsochBufferHandle; //Receives an opaque handle to the registered buffer. This handle is required by other WinUSB functions that perform isochronous transfers. 
		private: BOOL RequestToStop=FALSE;
		public: BOOL BackBufferLocked=FALSE; //Lock on front buffer during swap operation
		public: BOOL FrontBufferLocked = FALSE; //Lock on back buffer during write operation
		public: uint32_t TotalBytesTransferred = 0; //Holds the total number of bytes transferred
		public: uint64_t AvgTaktTimeMeasured = 0; //Holds the running avg of takt time measured

	//con/de-structor, copy & assignment:
		public: _USB_Device();
		public: ~_USB_Device();
		private: _USB_Device(const _USB_Device& that) = delete;
		private: _USB_Device& operator=(const _USB_Device& that) = delete;


	//methods:
			   /* Initializes the USB Handle. Follows the procedure as provided by the MS link:
			   https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/getting-set-up-to-use-windows-devices-usb
				If the device was already initialized it will first dispose all open handles and frees memory allocations
				If the devicd is in error state, the error state will be elevated and initialization will be attempted.
				*/
		public: VOID Initialize(uint32_t BufferSize, uint32_t PacketSize, uint32_t MicroFrameSize);

			   // Disposing of all open handles and frees memory allocations
		private: VOID Dispose();

			   /*Gives signal to the USB device to start reading. The device will only start if not in error state and initialized. 
			   This method is to be approached via the USB class and is therefore private. 
			   The reading will continue until a Stop is given, via the RequestToStop pointer or the Stop method.
			   The reading follows the TaktTime provided into the function.*/
		public: VOID Start(uint32_t TaktTime, PBOOL pRequestToStop, ULONG FrameNumber);

			  //Enter a request to stop, the device will exit the reading operation after completing the running Isochronous packet.
		public:	VOID Stop();

			  /*Read from the frontend buffer. If the buffer is locked due to short swap operation will return -2.
			  If the size of buffer is NULL will return -1. The Data argument will have a pointer returned to the frontend buffer.
			  The function will return the # of elments in the buffer.
			  The function will only return in case the USB buffer is not in error state and / or not initialized. */
		public: int32_t Read(uint8_t* Data);

			  /* This method will copy the front end buffer values into the destination buffer (for DLL purposes).
				 It will return the size copied */
		public: int32_t Copy(uint8_t* DestinationBuffer);

			  //Saves the back buffer to the front buffer. Will only be succesful if both of the buffers are unlocked and size is greater than zero.
		public: BOOL Swap();

			  //Method to zero out the Backbuffer
		public: BOOL ZeroBackBuffer();

			  //Method to open the USB device
		private: VOID GetHandle();

			   //Method to get the USB descriptor
		private: VOID GetDescriptor();

			   //Method to get the Interface settings
		private: VOID GetInterface();
		
			   /*Initializes the front and backend buffers using the indicated BufferSize argument.
				if the buffers were already initialized they will be disposed off and re-initialized. */
		private: VOID SetBuffers(uint32_t BufferSize);

			   //Method to get the Isochronous pipe
		private: VOID GetIsochPipe();

			   /*Method to get the Interval.
			   The Interval member is used to determine how often the endpoint can send or receive data.
			   For full-speed transmissions, the Interval and polling period values are always 1.*/
		private: VOID CheckInterval();

			   //Method to set transfer characteristics (Bytes per interval, frame, microframe, # of packets) 
		private: VOID SetTransferCharacteristics(uint32_t BufferSize, uint32_t PacketSize, uint32_t MicroFrameSize);

			   /*Method to initialize Overlapped structure. All transfers in the example are sent asynchronously.
			   For this, the app allocates an array of OVERLAPPED structure with three elements, one for each transfer.*/
		private: VOID SetOverlappedStructure();

			   /*Method to create Overlapped event completion.
			   The app provides events so that it can get notified when transfers complete and retrieve the results of the operation.
			   For this, in each OVERLAPPED structure in the array, the app allocates an event and sets the handle in the hEvent member.*/
		private: VOID SetOverlappedEvents();

			   /*Method to setup the Isochronous packets.
			   The USBD_ISO_PACKET_DESCRIPTOR structure is used by USB client drivers to describe an isochronous transfer packet.*/
		private: VOID SetIsochPackets();

			   /*Method to register the Isochronous buffer.
			   The app registers the buffer for a particular pipe by calling WinUsb_RegisterIsochBuffer.
			   The call returns a registration handle which is used to send the transfer.
			   The buffer is reused for subsequent transfers and offset in the buffer is adjusted to send or receive the next set of data.*/
		private: VOID RegisterIsochBuffer();

			   /*Method to get the current framenumber.
				To get the current frame, the app calls WinUsb_GetCurrentFrameNumber.
				At this point, the app must make sure that the start frame of the next transfer is later than the current frame, so that the USB driver stack does not drop late packets.
				To do so, the app calls WinUsb_GetAdjustedFrameNumber to get a realistic current frame number (this is later than the received current frame number).
			   */
		private: ULONG GetCurrentFrameNumber();

			   /*Method to reset the pipe. The WinUsb_ResetPipe function resets the data toggle and clears the stall condition on a pipe.*/
		private: VOID ResetPipe();

		};

