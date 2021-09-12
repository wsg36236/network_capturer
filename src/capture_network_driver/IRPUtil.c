#include "IRPUtil.h"

BOOLEAN IrpMdlIsValid(IN PIRP pIrp)
{
    if (pIrp->MdlAddress == NULL)
    {
        return FALSE;
    }

    if (MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority | MdlMappingNoExecute) == NULL)
    {
        return FALSE;
    }

    return TRUE;
}

PVOID GetIrpMdlAddress(IN PIRP pIrp, OUT ULONG* pnSize)
{
    if (pIrp->MdlAddress == NULL)
    {
        return NULL;
    }

    *pnSize = MmGetMdlByteCount(pIrp->MdlAddress);
    return MmGetSystemAddressForMdlSafe(pIrp->MdlAddress, NormalPagePriority | MdlMappingNoExecute);
}

ULONG GetIoControlCode(IN PIRP pIrp)
{
    PIO_STACK_LOCATION pCurrentStack = IoGetCurrentIrpStackLocation(pIrp);
    return pCurrentStack->Parameters.DeviceIoControl.IoControlCode;
}

PVOID GetIrpBuffer(IN PIRP pIrp, OUT ULONG* pnInputSize, OUT ULONG* pnOutputSize)
{
    PIO_STACK_LOCATION pCurrentStack = IoGetCurrentIrpStackLocation(pIrp);
    *pnInputSize = pCurrentStack->Parameters.DeviceIoControl.InputBufferLength;
    *pnOutputSize = pCurrentStack->Parameters.DeviceIoControl.OutputBufferLength;
    return pIrp->AssociatedIrp.SystemBuffer;
}

PVOID GetFsContext(IN PIRP pIrp)
{
    PIO_STACK_LOCATION pCurrentStack = IoGetCurrentIrpStackLocation(pIrp);
    return pCurrentStack->FileObject->FsContext;
}

void CompleteIrp(IN OUT PIRP pIrp, IN NTSTATUS status, IN ULONG_PTR information)
{
    pIrp->IoStatus.Status = status;
    pIrp->IoStatus.Information = information;
    IoCompleteRequest(pIrp, IO_NO_INCREMENT);
}
