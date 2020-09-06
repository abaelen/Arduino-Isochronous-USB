//
// Define below GUIDs
//
#include <initguid.h>

//
// Device Interface GUID.
// Used by all WinUsb devices that this application talks to.
// Must match "DeviceInterfaceGUIDs" registry value specified in the INF file.
// 82100f7e-09f3-4f64-ab2e-597fe9dba005
//
DEFINE_GUID(GUID_DEVINTERFACE_USBApplication1,
    0x82100f7e, 0x09f3, 0x4f64, 0xab, 0x2e, 0x59, 0x7f, 0xe9, 0xdb, 0xa0, 0x05);

typedef struct _DEVICE_DATA {

    BOOL                    HandlesOpen;
    WINUSB_INTERFACE_HANDLE WinusbHandle;
    HANDLE                  DeviceHandle;
    TCHAR                   DevicePath[MAX_PATH];
    UCHAR                   IsochOutPipe;
    UCHAR                   IsochInPipe;
    ULONG                   IsochInTransferSize;
    ULONG                   IsochOutTransferSize;
    ULONG                   IsochInPacketCount;
    ULONG                   IsochOutPacketCount;
    ULONG                   IsochInBytesPerFrame;
    ULONG                   IsochOutBytesPerFrame;
    ULONG                   IsochInBytesPerInterval;
    ULONG                   IsochOutBytesPerInterval;
    ULONG                   IsochTrfPacketCount;
    WINUSB_PIPE_INFORMATION_EX pipe;
} DEVICE_DATA, * PDEVICE_DATA;

HRESULT
OpenDevice(
    _Out_     PDEVICE_DATA DeviceData,
    _Out_opt_ PBOOL        FailureDeviceNotFound
);

VOID
CloseDevice(
    _Inout_ PDEVICE_DATA DeviceData
);
