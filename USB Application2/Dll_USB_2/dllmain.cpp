// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "USB.h"
#include <vector>

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

_USB_Device USB_Device;
    //To use the Class create a _USB_Device USB_Device member. Then call the USB_Device.Initialize(1023,1,1) eg. 1023 in 1 packet & 1 microframe (as only FS) or USB_Device.Initialize(2046,2,1), ...    

//Below exposes all the public members and methods for external use. Call these from for example C# using
    
    /* [DllImport(@"C:\Users\Gebruiker\Documents\OneDrive\Elec Projects\Github\YetAnother-USB-Oscilloscope\YetAnotherUSBOscilloscope\x64\Debug\Dll_USB_2.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
    public static extern void USB2_Device_Initialize(UInt32 Buffersize, UInt32 PacketSize, UInt32 MicroFrameSize);
    
    [DllImport(@"C:\Users\Gebruiker\Documents\OneDrive\Elec Projects\Github\YetAnother-USB-Oscilloscope\YetAnotherUSBOscilloscope\x64\Debug\Dll_USB_2.dll", CharSet = CharSet.Unicode, CallingConvention = CallingConvention.Cdecl)]
    public static extern void USB2_Device_Start(UInt32 TaktTime, int[] pRequestToStop = null, ulong FrameNumber = 0);*/



    //Keep data out of the dll export functions, as it increases speed of execution
    //To get values, pass by value
    //To send values, pass a pointer and set the value of the memory space of the pointer

extern "C" {
      __declspec(dllexport) void USB2_Device_Initialize(uint32_t BufferSize, uint32_t PacketSize, uint32_t MicroFrameSize)
    {
        USB_Device.Initialize(BufferSize, PacketSize, MicroFrameSize);
    }
    __declspec(dllexport) void USB2_Device_Start(uint32_t TaktTime, PBOOL RequestToStop, ULONG FrameNumber = 0)
    {
        USB_Device.Start(TaktTime,RequestToStop, FrameNumber);
    }
    __declspec(dllexport) void USB2_Device_Stop()
    {
        USB_Device.Stop();
    }
    __declspec(dllexport) void USB2_Device_Copy(uint8_t* DestinationBuffer)
    {
        USB_Device.Copy(DestinationBuffer);
    }
    __declspec(dllexport) uint8_t USB2_Device_Swap()
    {
        return (uint8_t)USB_Device.Swap();
    }
    __declspec(dllexport) uint8_t USB2_Device_ZeroBackBuffer()
    {
        return (uint8_t)USB_Device.ZeroBackBuffer();
    }
    __declspec(dllexport) uint8_t USB2_Device_Status_Started()
    {
        return (uint8_t)USB_Device.Status.Started;
    }
    __declspec(dllexport) uint8_t USB2_Device_Status_Stopped()
    {
        return (uint8_t)USB_Device.Status.Stopped;
    }
    __declspec(dllexport) uint8_t USB2_Device_Status_Initialized()
    {
        return (uint8_t)USB_Device.Status.Initialized;
    }
    __declspec(dllexport) uint8_t USB2_Device_Status_Errored()
    {
        return (uint8_t)USB_Device.Status.Errored;
    }
    __declspec(dllexport) uint8_t USB2_Device_Status_Action_DeviceFound()
    {
        return (uint8_t)USB_Device.Status.Action.DeviceFound;
    }
    __declspec(dllexport) uint8_t USB2_Device_Status_Action_GetDescriptor()
    {
        return (uint8_t)USB_Device.Status.Action.GetDescriptor;
    }
    __declspec(dllexport) uint8_t USB2_Device_Status_Action_GetInterface()
    {
        return (uint8_t)USB_Device.Status.Action.GetInterface;
    }
    __declspec(dllexport) uint8_t USB2_Device_Status_Action_IsocPipe()
    {
        return (uint8_t)USB_Device.Status.Action.GetIsochPipe;
    }
    __declspec(dllexport) uint8_t USB2_Device_Status_Action_GetInterval()
    {
        return (uint8_t)USB_Device.Status.Action.GetInterval;
    }
    __declspec(dllexport) uint8_t USB2_Device_Status_Action_SetTransferChars()
    {
        return (uint8_t)USB_Device.Status.Action.SetTransferChars;
    }
    __declspec(dllexport) uint8_t USB2_Device_Status_Action_SetOverlappedStructure()
    {
        return (uint8_t)USB_Device.Status.Action.SetOverlappedStructure;
    }
    __declspec(dllexport) uint8_t USB2_Device_Status_Action_SetOverlappedEvents()
    {
        return (uint8_t)USB_Device.Status.Action.SetOverlappedEvents;
    }
    __declspec(dllexport) uint8_t USB2_Device_Status_Action_SetIsochPackets()
    {
        return (uint8_t)USB_Device.Status.Action.SetIsochPackets;
    }
    __declspec(dllexport) uint8_t USB2_Device_Status_Action_RegisterIsochBuffer()
    {
        return (uint8_t)USB_Device.Status.Action.RegisterIsochBuffer;
    }
    __declspec(dllexport) uint8_t USB2_Device_Status_Action_ResetPipe()
    {
        return (uint8_t)USB_Device.Status.Action.ResetPipe;
    }
    __declspec(dllexport) uint8_t USB2_Device_Status_Action_EndAtFrame()
    {
        return (uint8_t)USB_Device.Status.Action.EndAtFrame;
    }
    __declspec(dllexport) uint8_t USB2_Device_Status_Action_ReadIsochPipe()
    {
        return (uint8_t)USB_Device.Status.Action.ReadIsochPipe;
    }
}