#include "pch.h"
#include "USB.h"

_USB_Device::_USB_Device()
{
	//Status.Initialized = Initialize(BufferSize,PacketSize, MicroFrameSize);
}

_USB_Device::~_USB_Device()
{
	Dispose();
}

VOID _USB_Device::GetHandle()
{
	BOOL deviceNotFound = FALSE;
	HRESULT Hresult = OpenDevice(&Handle, &deviceNotFound);
	Status.Action.DeviceFound = !(deviceNotFound);
	if (Hresult < 0 || Status.Action.DeviceFound == FALSE) throw "No device handle retrieved";
}

VOID _USB_Device::GetDescriptor()
{
	ULONG lengthReceived;
	Status.Action.GetDescriptor = WinUsb_GetDescriptor(Handle.WinusbHandle, USB_DEVICE_DESCRIPTOR_TYPE, 0, 0, (PBYTE)&Descriptor, sizeof(Descriptor), &lengthReceived);
	if (lengthReceived != sizeof(Descriptor) || Status.Action.GetDescriptor == FALSE) throw "No Descriptor retrieved";
}

VOID _USB_Device::GetInterface()
{
	Status.Action.GetInterface = WinUsb_QueryInterfaceSettings(Handle.WinusbHandle, 0, &Interface);
}

VOID _USB_Device::SetBuffers(uint32_t BufferSize)
{
	BackBuffer.assign(BufferSize,0);
	FrontBuffer.assign(BufferSize, 0);
	FrontBufferLocked = FALSE;
	BackBufferLocked = FALSE;
}

VOID _USB_Device::GetIsochPipe()
{
	Status.Action.GetIsochPipe = FALSE;
	for (UCHAR i = 0; i < Interface.bNumEndpoints; i++)
	{
		if (WinUsb_QueryPipeEx(Handle.WinusbHandle, 0, i, &Handle.pipe) == TRUE) 
		{
			if (Handle.pipe.PipeType == UsbdPipeTypeIsochronous) 
			{
				Handle.IsochInPipe = Handle.pipe.PipeId;
				Status.Action.GetIsochPipe = TRUE;
				i = Interface.bNumEndpoints;
			}
		}
		else
		{
			//Status.Action.QueryPipe = HRESULT_FROM_WIN32(GetLastError());
			i = Interface.bNumEndpoints;
			throw "No Pipe";
		}
	}
}

VOID _USB_Device::CheckInterval()
{
	if (Handle.pipe.MaximumBytesPerInterval == 0 || Handle.pipe.Interval == 0)
	{
		Status.Action.GetInterval = FALSE;
		throw "Wrong interval or Max Bytes per interval";
	}
	Status.Action.GetInterval = TRUE;
}

VOID _USB_Device::SetTransferCharacteristics(uint32_t BufferSize, uint32_t PacketSize, uint32_t MicroFrameSize)
{
	Handle.IsochInBytesPerInterval = ((ULONG)BufferSize > Handle.pipe.MaximumBytesPerInterval) ? Handle.pipe.MaximumBytesPerInterval : (ULONG)BufferSize;
	Handle.IsochInBytesPerFrame = Handle.IsochInBytesPerInterval * ((ULONG)MicroFrameSize / (ULONG)Handle.pipe.Interval);
	Handle.IsochInTransferSize = Handle.IsochInBytesPerFrame * (ULONG)PacketSize;
	if (Handle.IsochInTransferSize % Handle.IsochInBytesPerFrame == 0) 
	{
		Handle.IsochInPacketCount = max(1,Handle.IsochInTransferSize / Handle.pipe.MaximumBytesPerInterval); //BufferSize / Handle.IsochInTransferSize; //assumption: no re-use of packages accross query
		Status.Action.EndAtFrame=TRUE;
		Status.Action.SetTransferChars = TRUE;
	}
	else
	{
		Status.Action.EndAtFrame = FALSE;
		Status.Action.SetTransferChars = FALSE;
		throw "Buffer not at end of frame";
	}
}

VOID _USB_Device::SetOverlappedStructure()
{
	Overlapped = new OVERLAPPED[Handle.IsochInPacketCount];
	ZeroMemory(Overlapped, Handle.IsochInPacketCount * sizeof(OVERLAPPED));
	if (Overlapped != NULL) Status.Action.SetOverlappedStructure = TRUE; else throw "no Overlapped allocated";
}

VOID _USB_Device::SetOverlappedEvents()
{
	for (ULONG i = 0; i < Handle.IsochInPacketCount; i++)
	{
		Overlapped[i].hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (Overlapped[i].hEvent == NULL) { Status.Action.SetOverlappedEvents = FALSE; throw "Event not created"; }
		else Status.Action.SetOverlappedEvents = TRUE;
	}
}

VOID _USB_Device::SetIsochPackets()
{
	Packets = new USBD_ISO_PACKET_DESCRIPTOR[Handle.IsochInPacketCount];
	ZeroMemory(Packets, Handle.IsochInPacketCount * sizeof(USBD_ISO_PACKET_DESCRIPTOR));
	if (Packets == NULL) {Status.Action.SetIsochPackets = FALSE; throw "No Packets could be created";}
	else Status.Action.SetIsochPackets = TRUE;
}

VOID _USB_Device::RegisterIsochBuffer()
{
	if (BackBuffer.capacity()  < (ULONG) (Handle.IsochInTransferSize))
	{Status.Action.RegisterIsochBuffer = FALSE; throw "No or wrongly sized databuffer"; }
	
	BOOL result = WinUsb_RegisterIsochBuffer(Handle.WinusbHandle, Handle.IsochInPipe, &BackBuffer[0], BackBuffer.size(), &IsochBufferHandle);
	
	if (result == FALSE) { Status.Action.RegisterIsochBuffer = FALSE; throw "No or wrongly sized databuffer"; } 
	else Status.Action.RegisterIsochBuffer = TRUE;
}

ULONG _USB_Device::GetCurrentFrameNumber()
{
	ULONG frameNumber; LARGE_INTEGER timeStamp;
	BOOL result = WinUsb_GetCurrentFrameNumber(Handle.WinusbHandle, &frameNumber, &timeStamp);
	if (result == FALSE)
	{
		Status.Errored = TRUE;
		return -999;
	}
	return frameNumber;
}

VOID _USB_Device::ResetPipe()
{
	BOOL result = WinUsb_ResetPipe(Handle.WinusbHandle, Handle.IsochInPipe);
	if (result == FALSE)
	{
		Status.Errored = TRUE;
		Status.Action.ResetPipe = FALSE;
	}
	Status.Action.ResetPipe = TRUE;
}


//To use the Class create a _USB_Device USB member. Then call the USB.Initialize(1023,1,1) eg. 1023 in 1 packet & 1 microframe (as only FS) or USB.Initialize(2046,2,1), ...

VOID _USB_Device::Initialize(uint32_t BufferSize, uint32_t PacketSize, uint32_t MicroFrameSize)
{
	try {
		if (Status.Initialized==TRUE) Dispose();
		Status.Errored=FALSE;

		SetBuffers(BufferSize);
		GetHandle();
		GetDescriptor();
		GetInterface();
		GetIsochPipe();
		CheckInterval();
		SetTransferCharacteristics(BufferSize, PacketSize, MicroFrameSize);
		SetOverlappedStructure();
		SetOverlappedEvents();
		SetIsochPackets();
		RegisterIsochBuffer();
		ResetPipe();

		Status.Initialized=TRUE;
	}
	catch (...) 
	{
		Status.Initialized = FALSE;
		Status.Errored = TRUE;
		Dispose();
	}
}

VOID _USB_Device::Dispose()
{
	
	if (IsochBufferHandle != 0x0000000000000000) {BOOL Result = WinUsb_UnregisterIsochBuffer(IsochBufferHandle);}
	
	if (Packets != NULL) {delete[] Packets;}
	
	for (ULONG i = 0; i < Handle.IsochInPacketCount; i++)
	{	if (Overlapped != NULL && Overlapped[i].hEvent != NULL) { CloseHandle(Overlapped[i].hEvent); } }
	
	if (Overlapped != NULL) { delete[] Overlapped; }
	
	CloseDevice(&Handle);

	Status.Initialized = FALSE;
	Status.Started = FALSE;
	Status.Stopped = FALSE;
}

VOID _USB_Device::Start(uint32_t TaktTime, PBOOL pRequestToStop, ULONG FrameNumber)
{
	try {
		if (Status.Initialized==FALSE || Status.Errored==TRUE) throw "USB device in Error or not initialized";

		Status.Started = TRUE;
		
		uint64_t previousTime = (uint64_t) TaktTime + 1;
		TotalBytesTransferred = 0;
		AvgTaktTimeMeasured = 0;
		
		while (*pRequestToStop == FALSE && RequestToStop == FALSE)
		{		

			previousTime = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::
				now().time_since_epoch()).count();
			BackBufferLocked = TRUE;
			DWORD numBytes = 0;
			for (ULONG i = 0; i < 1; i++)	//Loop in case more than one transfer is to be made. 
											//https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/getting-set-up-to-use-windows-devices-usb
											//ie. ISOCH_TRANSFER_COUNT in website above. Ie. 3 transfers of 10ms. In our setup only 1 transfer is required.
			{				
				if (FrameNumber == 0)
				{
					Status.Action.ReadIsochPipe = WinUsb_ReadIsochPipeAsap(IsochBufferHandle, i * Handle.IsochInTransferSize, Handle.IsochInTransferSize, /*(i == 0) ? FALSE : TRUE*/ FALSE,
						Handle.IsochInPacketCount, &Packets[i * Handle.IsochInPacketCount], &Overlapped[i]);
				}
				else
				{
					Status.Action.ReadIsochPipe = WinUsb_ReadIsochPipe(IsochBufferHandle, i * Handle.IsochInTransferSize, Handle.IsochInTransferSize, &FrameNumber,
						Handle.IsochInPacketCount, &Packets[i * Handle.IsochInPacketCount], &Overlapped[i]);
				}

				if (Status.Action.ReadIsochPipe == FALSE && GetLastError() != ERROR_IO_PENDING) { throw "Failed to start a read operation"; }

				for (ULONG i = 0; i < 1; i++) //Likewise to the above comment ISOCH_TRANFER_COUNT in our case is 1
				{
					BOOL result = WinUsb_GetOverlappedResult(Handle.WinusbHandle, &Overlapped[i], &numBytes, TRUE);
					if (result == FALSE) {throw "Failed to read";}
					TotalBytesTransferred += numBytes;
				}

			}

				numBytes=0;
				//   Allow system time to count the numBytes before killing the thread
				/*while (numBytes < Handle.IsochInTransferSize) {
					//removed the Getoverlappedresult for performance reasons. This resulted in blue screen WDF-vialoation. 
					//With now includeing a while loop to wait for results to come in, blue screen is not occuring anymore.
					for (ULONG i = 0; i < Handle.IsochInPacketCount; i++) {
						numBytes += Packets[i].Length;
					}
				}*/
				numBytes = 0;
		}
			BackBufferLocked = FALSE;
			
			Swap();
			ZeroBackBuffer();
			
			while (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::
				now().time_since_epoch()).count() - previousTime <= (uint64_t)TaktTime) {uint8_t numBytes=0; };
			
			AvgTaktTimeMeasured = (uint64_t) (AvgTaktTimeMeasured + std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::
				now().time_since_epoch()).count() - previousTime) / 2;
		

		RequestToStop = FALSE;
		Status.Started = FALSE;
		Status.Stopped = TRUE;

	} catch (...)
	{
		Status.Started = FALSE;
		Status.Errored = TRUE;
		Dispose();
	}
}

VOID _USB_Device::Stop()
{
	if(Status.Started == TRUE && Status.Errored==FALSE) {RequestToStop = TRUE;}
}

int32_t _USB_Device::Read(uint8_t* data)
{
	try
	{
		if (Status.Errored == TRUE || Status.Initialized == FALSE) throw "Buffer in Error State or not initialized";

		if (FrontBufferLocked == FALSE)
		{
			FrontBufferLocked = TRUE;
			data = &FrontBuffer[0];
			return (int32_t) FrontBuffer.capacity();
			FrontBufferLocked = FALSE;
		}
		else { data = NULL; return -2; }
	}
	catch (...)
	{
		Status.Errored = TRUE;
		Dispose();
		return FALSE;
	}
}

int32_t _USB_Device::Copy(uint8_t* DestinationBuffer)
{
	try
	{
		if (Status.Errored == TRUE || Status.Initialized == FALSE) throw "Buffer in Error State or not initialized";
		if (FrontBufferLocked == FALSE)
		{
			FrontBufferLocked = TRUE;
			memcpy(DestinationBuffer, &FrontBuffer[0], FrontBuffer.size());
			return FrontBuffer.size();
			FrontBufferLocked = FALSE;
		}
		else { return -2; }
	}
	catch (...)
	{
		Status.Errored = TRUE;
		Dispose();
		return -1;
	}
}

BOOL _USB_Device::Swap()
{
	try
	{
		if (Status.Errored == TRUE || Status.Initialized == FALSE) throw "Buffers not initialized or errored.";

		if (FrontBufferLocked == FALSE && BackBufferLocked == FALSE)
		{
			FrontBufferLocked = TRUE; BackBufferLocked = TRUE;
			FrontBuffer = BackBuffer;
			FrontBufferLocked = FALSE; BackBufferLocked = TRUE;
			return TRUE;
		}
		return FALSE;
	}
	catch (...)
	{
		Status.Errored = TRUE;
		Dispose();
		return FALSE;
	}
}

BOOL _USB_Device::ZeroBackBuffer()
{
	try
	{
		if (Status.Errored == TRUE || Status.Initialized == FALSE) throw "Buffer not initialized or in error state";

		BackBufferLocked = TRUE;
		memset(&BackBuffer[0],0,BackBuffer.capacity());
		BackBufferLocked = FALSE;
		return TRUE;
	}
	catch (...)
	{
		Dispose();
		Status.Errored = TRUE;
		return FALSE;
	}
}