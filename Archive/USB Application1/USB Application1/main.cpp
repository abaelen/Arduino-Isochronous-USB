#include "pch.h"

#include <stdio.h>
#include <time.h>



HRESULT GetIsochPipes(_Inout_ PDEVICE_DATA DeviceData)
{
    BOOL result;
    USB_INTERFACE_DESCRIPTOR usbInterface;
    HRESULT hr = S_OK;
    UCHAR i;

    result = WinUsb_QueryInterfaceSettings(DeviceData->WinusbHandle,
        0,
        &usbInterface);

    if (result == FALSE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        printf("WinUsb_QueryInterfaceSettings failed to get USB interface.\n");
        CloseHandle(DeviceData->DeviceHandle);
        return hr;
    }

    for (i = 0; i < usbInterface.bNumEndpoints; i++)
    {
        result = WinUsb_QueryPipeEx(
            DeviceData->WinusbHandle,
            0,
            (UCHAR)i,
            &(*DeviceData).pipe);

        if (result == FALSE)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            printf("WinUsb_QueryPipeEx failed to get USB pipe.\n");
            CloseHandle(DeviceData->DeviceHandle);
            return hr;
        }

        if (((*DeviceData).pipe.PipeType == UsbdPipeTypeIsochronous) && (!((*DeviceData).pipe.PipeId == 0x82)))
        {
            DeviceData->IsochOutPipe = (*DeviceData).pipe.PipeId;
        }
        else if ((*DeviceData).pipe.PipeType == UsbdPipeTypeIsochronous)
        {
            DeviceData->IsochInPipe = (*DeviceData).pipe.PipeId;
        }
    }

    return hr;
}

#define ISOCH_DATA_SIZE_MS   1

HRESULT GetIntervalInformation(_In_ PDEVICE_DATA DeviceData) {
    HRESULT hr = S_OK;

    if (((*DeviceData).pipe.PipeType == UsbdPipeTypeIsochronous) && (((*DeviceData).pipe.PipeId == 0x82)))
    {
        DeviceData->IsochInPipe = (*DeviceData).pipe.PipeId;

        if ((*DeviceData).pipe.MaximumBytesPerInterval == 0 || ((*DeviceData).pipe.Interval == 0))
        {
            hr = E_INVALIDARG;
            printf("Isoch Out: MaximumBytesPerInterval or Interval value is 0.\n");
            CloseHandle(DeviceData->DeviceHandle);
            return hr;
        }
        else
        {
            DeviceData->IsochInTransferSize =
                ISOCH_DATA_SIZE_MS *
                (*DeviceData).pipe.MaximumBytesPerInterval *
                (8 / (*DeviceData).pipe.Interval);
            
        }
    }
    return NULL;
}

#define ISOCH_TRANSFER_COUNT   10000

VOID SendIsochInTransfer(_Inout_ PDEVICE_DATA DeviceData,_In_ BOOL AsapTransfer) {
    PUCHAR readBuffer;
    LPOVERLAPPED overlapped;
    ULONG numBytes=0;
    ULONG numBytes1;
    BOOL result;
    DWORD lastError;
    WINUSB_ISOCH_BUFFER_HANDLE isochReadBufferHandle;
    PUSBD_ISO_PACKET_DESCRIPTOR isochPackets;
    ULONG i;


    ULONG frameNumber;
    ULONG startFrame;
    LARGE_INTEGER timeStamp;

    ULONG totalTransferSize;

    readBuffer = NULL;
    isochPackets = NULL;
    overlapped = NULL;
    isochReadBufferHandle = INVALID_HANDLE_VALUE;

    printf("\n\nRead transfer.\n");

    DeviceData->IsochInBytesPerInterval = (ULONG)(*DeviceData).pipe.MaximumBytesPerInterval;
    DeviceData->IsochInBytesPerFrame = DeviceData->IsochInBytesPerInterval * (ULONG)(*DeviceData).pipe.Interval;
    DeviceData->IsochInTransferSize = DeviceData->IsochInBytesPerFrame * ISOCH_DATA_SIZE_MS;
    totalTransferSize = DeviceData->IsochInTransferSize * ISOCH_TRANSFER_COUNT;
    DeviceData->IsochInPacketCount = ISOCH_DATA_SIZE_MS * ISOCH_TRANSFER_COUNT;


    if (totalTransferSize % DeviceData->IsochInBytesPerFrame != 0)
    {
        printf("Transfer size must end at a frame boundary.\n");
        goto Error;
    }

    readBuffer = new UCHAR[totalTransferSize];
    if (readBuffer == NULL)
    {
        printf("Unable to allocate memory.\n");
        goto Error;
    }
    ZeroMemory(readBuffer, totalTransferSize);

    overlapped = new OVERLAPPED[ISOCH_TRANSFER_COUNT];
    ZeroMemory(overlapped, ISOCH_TRANSFER_COUNT*(sizeof(OVERLAPPED)));
    for (i = 0; i < ISOCH_TRANSFER_COUNT; i++)
    {
        (*overlapped).hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

        if ((*overlapped).hEvent == NULL)
        {
            printf("Unable to set event for overlapped operation.\n");
            goto Error;
        }
    }

    isochPackets = new USBD_ISO_PACKET_DESCRIPTOR [ISOCH_TRANSFER_COUNT*1] ;
    ZeroMemory(isochPackets, ISOCH_TRANSFER_COUNT*1*sizeof(USBD_ISO_PACKET_DESCRIPTOR));

    result = WinUsb_RegisterIsochBuffer(
        DeviceData->WinusbHandle,
        DeviceData->IsochInPipe,
        readBuffer,
        ISOCH_TRANSFER_COUNT * DeviceData->IsochInTransferSize,
        &isochReadBufferHandle);

    if (!result)
    {
        printf("Isoch buffer registration failed.\n");
        goto Error;
    }

 
/*    result = WinUsb_ResetPipe(DeviceData->WinusbHandle,
        '2');

    if (!result)
    {
        printf("WinUsb_ResetPipe failed.\n");
        goto Error;
    }*/
    result = WinUsb_GetCurrentFrameNumber(
        DeviceData->WinusbHandle,
        &frameNumber,
        &timeStamp);

    if (!result)
    {
        printf("WinUsb_GetCurrentFrameNumber failed.\n");
        goto Error;
    }

    startFrame = frameNumber + 2;


    for (i = 0; i < ISOCH_TRANSFER_COUNT; i++)
    {
        AsapTransfer = TRUE;
        if (AsapTransfer)
        {
            result = WinUsb_ReadIsochPipeAsap(
                isochReadBufferHandle,
                i* DeviceData->IsochInTransferSize,
                DeviceData->IsochInTransferSize,
                //&startFrame,
                (i == 0) ? FALSE : TRUE,
                1,
                &isochPackets[i*1],
                &overlapped[i]);
        }
        else
        {

            printf("Transfer starting at frame %d.\n", startFrame);

            result = WinUsb_ReadIsochPipe(
                isochReadBufferHandle,
                DeviceData->IsochInTransferSize * i,
                DeviceData->IsochInTransferSize,
                &startFrame,
                DeviceData->IsochInPacketCount,
                &isochPackets[i * DeviceData->IsochInPacketCount],
                &overlapped[i]);

            printf("Next transfer frame %d.\n", startFrame);

        }

        if (!result)
        {
            lastError = GetLastError();

            if (lastError != ERROR_IO_PENDING)
            {
                printf("Failed to start a read operation with error %d\n", lastError);
            }
        }
    }
    numBytes = 0; numBytes1 = 0;
    for (i = 0; i < ISOCH_TRANSFER_COUNT; i++) {
    result = WinUsb_GetOverlappedResult(
        DeviceData->WinusbHandle,
        &overlapped[i],
        &numBytes1,
        TRUE);

        if (!result)
        {
            lastError = GetLastError();

            printf("Failed to read with error %d\n", lastError);
        }
    }

        //Allow system time to count the numBytes before killing the thread
        clock_t start_time = clock();
        while (clock() / CLOCKS_PER_SEC < 30+ start_time / CLOCKS_PER_SEC ) {}
        for (i = 0; i < ISOCH_TRANSFER_COUNT; i++) {
            numBytes += isochPackets[i].Length;
        }
        printf("Transfer completed. Read %d bytes. \n\n", numBytes);
        printf("Done\n\n");

Error:
    if (isochReadBufferHandle != INVALID_HANDLE_VALUE)
    {
        result = WinUsb_UnregisterIsochBuffer(isochReadBufferHandle);
        if (!result)
        {
            printf("Failed to unregister isoch read buffer. \n");
        }
    }

    if (readBuffer != NULL)
    {
        delete[] readBuffer;
    }

    if (isochPackets != NULL)
    {
        delete[] isochPackets;
    }

    for (i = 0; i < ISOCH_TRANSFER_COUNT; i++)

    {
        if (overlapped[i].hEvent != NULL)
        {
            CloseHandle(overlapped[i].hEvent);
        }

    }

    if (overlapped != NULL)
    {
        delete[] overlapped;
    }
    return;
}


LONG __cdecl
_tmain(
    LONG     Argc,
    LPTSTR * Argv
    )
/*++

Routine description:

    Sample program that communicates with a USB device using WinUSB

--*/
{
    DEVICE_DATA           deviceData;
    HRESULT               hr;
    USB_DEVICE_DESCRIPTOR deviceDesc;
    BOOL                  bResult;
    BOOL                  noDevice;
    ULONG                 lengthReceived;
    USB_INTERFACE_DESCRIPTOR    interfaceDesc;
    

    UNREFERENCED_PARAMETER(Argc);
    UNREFERENCED_PARAMETER(Argv);

    //
    // Find a device connected to the system that has WinUSB installed using our
    // INF
    //
  //  system ("pause"); 
    hr = OpenDevice(&deviceData, &noDevice);

    if (FAILED(hr)) {

        if (noDevice) {

            wprintf(L"Device not connected or driver not installed\n");

        } else {

            wprintf(L"Failed looking for device, HRESULT 0x%x\n", hr);
        }

        return 0;
    }

    //
    // Get device descriptor
    //
    bResult = WinUsb_GetDescriptor(deviceData.WinusbHandle,
                                   USB_DEVICE_DESCRIPTOR_TYPE,
                                   0,
                                   0,
                                   (PBYTE) &deviceDesc,
                                   sizeof(deviceDesc),
                                   &lengthReceived);

    if (FALSE == bResult || lengthReceived != sizeof(deviceDesc)) {

        wprintf(L"Error among LastError %d or lengthReceived %d\n",
                FALSE == bResult ? GetLastError() : 0,
                lengthReceived);
        CloseDevice(&deviceData);
        return 0;
    }

    bResult = WinUsb_QueryInterfaceSettings(deviceData.WinusbHandle, 0, &interfaceDesc);

    if (FALSE == bResult || lengthReceived != sizeof(deviceDesc)) {

        wprintf(L"Error among LastError %d or lengthReceived %d\n",
            FALSE == bResult ? GetLastError() : 0,
            lengthReceived);
        CloseDevice(&deviceData);
        return 0;
    }


    //
    // Print a few parts of the device descriptor
    //
    wprintf(L"Device found: VID_%04X&PID_%04X; bcdUsb %04X\n",
            deviceDesc.idVendor,
            deviceDesc.idProduct,
            deviceDesc.bcdUSB);

    HRESULT HResult;
    HResult = GetIsochPipes(&deviceData);
        if (FALSE == bResult || lengthReceived != sizeof(deviceDesc)) {

        wprintf(L"Error among LastError %d or lengthReceived %d\n",
            FALSE == bResult ? GetLastError() : 0,
            lengthReceived);
        CloseDevice(&deviceData);
    //    system("pause");
        return 0;
        }
    
        HResult = GetIntervalInformation(&deviceData);
        if (FALSE == bResult || lengthReceived != sizeof(deviceDesc)) {

            wprintf(L"Error among LastError %d or lengthReceived %d\n",
                FALSE == bResult ? GetLastError() : 0,
                lengthReceived);
            CloseDevice(&deviceData);
     //       system("pause");
            return 0;
        }

        SendIsochInTransfer(&deviceData, TRUE);

    system("pause");
    system("pause");
    CloseDevice(&deviceData);
    return 0;
}
