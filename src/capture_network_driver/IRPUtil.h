#pragma once
#include <ntddk.h>

// IRP in/out buffer
// MDL
BOOLEAN IrpMdlIsValid(IN PIRP pIrp);
PVOID   GetIrpMdlAddress(IN PIRP pIrp, OUT ULONG* pnSize);

// buffer
ULONG GetIoControlCode(IN PIRP pIrp);
PVOID GetIrpBuffer(IN PIRP pIrp, OUT ULONG* pnInputSize, OUT ULONG* pnOutputSize);

// info
PVOID GetFsContext(IN PIRP pIrp);

// other
void CompleteIrp(IN OUT PIRP pIrp, IN NTSTATUS status, IN ULONG_PTR information);



