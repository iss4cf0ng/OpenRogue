#include <ntddk.h>

#define DEVICE_NAME_STR L"\\Device\\VulnSim"
#define SYMLINK_NAME_STR L"\\DosDevices\\VulnSim"

#define IOCTL_VULNSIM_PING CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)

VOID VulnSimUnload(_In_ PDRIVER_OBJECT DriverObject)
{

}

NTSTATUS VulnSimCreateClose(_In_ PDEVICE_OBJECT DeviceObject, _Inout_ PIRP Irp)
{

}

NTSTATUS VulnSimDeviceControl(_In_ PDEVICE_OBJECT DeviceObject, _Inout_ PIRP Irp)
{

}

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath)
{

}