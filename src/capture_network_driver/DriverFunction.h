#pragma once
#include <ntddk.h>

NTSTATUS DispatchCreate(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS DispatchRead(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS DispatchCleanUp(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS DispatchClose(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);
NTSTATUS DispatchDeviceControl(PDEVICE_OBJECT pDeviceObject, PIRP pIrp);

